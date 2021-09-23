/**
 * @file bmp280_lib.cpp
 * @author Denys Khmil
 * @brief This file contents all the bmp280 library functions
 */
#include "bmp280_lib.h"

/**
 * @brief bmp280 constructor with specified i2c address
 * @param _i2c: bmp280 i2c port.
 * @param _address bmp280 address.
 * @note Saves i2c port and adress, sets bmp280 to default mode (16bit temp, oversampling*4, normal mode)
 */
bmp280::bmp280(I2C_HandleTypeDef _i2c, uint8_t _address){
    this->i2c = _i2c;
    this->address = _address;
    this->settings(0b001, 0b011, 0b11);
    this->setConfig(0);
    this->readCalibration();
}


/**
 * @brief bmp280 constructor with defauld i2c address
 * @param _i2c: bmp280 i2c port.
 * @note Saves i2c port and adress, sets bmp280 to default mode (16bit temp, oversampling*4, normal mode)
 */
bmp280::bmp280(I2C_HandleTypeDef _i2c){
    this->i2c = _i2c;
    this->address = 0b1110110;
    this->settings(0b010, 0b011, 0b11);
    this->setConfig(0);
    this->readCalibration();
}


/**
 * @brief Changes sensor settings
 * @param osrs_p: Pressure measurement settings register.
 * @param osrs_t: Temperature measurement settings register.
 * @param mode: Sensor mode.
 */
void bmp280::settings(uint8_t osrs_p, uint8_t osrs_t, uint8_t mode){    
    uint8_t reg = (osrs_t << 5)|(osrs_p << 2)|(mode);
    HAL_I2C_Mem_Write(&this->i2c, (this->address << 1), 0xf4, I2C_MEMADD_SIZE_8BIT, &reg, 1, 1000);
}


/**
 * @brief Set sensor configuration
 */
void bmp280::setConfig(uint8_t t_sb){
    HAL_I2C_Mem_Write(&this->i2c, (this->address << 1), 0xf5, I2C_MEMADD_SIZE_8BIT, &t_sb, 1, 1000);
}


/**
 * @brief Software reset for sensor
 */
void bmp280::Reset(){
    uint8_t value = 0xb6;
    HAL_I2C_Mem_Write(&this->i2c, (this->address << 1), 0xe0, I2C_MEMADD_SIZE_8BIT, &value, 1, 1000);
}


/**
 * @brief Check if conversion is running
 */
uint8_t bmp280::conversionRunning(){
    uint8_t reg;
    HAL_I2C_Mem_Read(&this->i2c, (this->address << 1), 0xf3, I2C_MEMADD_SIZE_8BIT, &reg, 1, 1000);
    return reg & 0x08;
}


/**
 * @brief Check if data is copying
 */
uint8_t bmp280::dataCopying(){
    uint8_t reg;
    HAL_I2C_Mem_Read(&this->i2c, (this->address << 1), 0xf3, I2C_MEMADD_SIZE_8BIT, &reg, 1, 1000);
    return reg & 0x08;
}


/**
 * @brief Read sensor id (default 0x58)
 * @retval id uint8_t
 */
uint8_t bmp280::read_id(){
    uint8_t id;
    HAL_I2C_Mem_Read(&this->i2c, (uint16_t)(this->address << 1), 0xD0, I2C_MEMADD_SIZE_8BIT, &id, 1, 1000);
    return id;
}


/**
 * @brief Read calibration constants from sensor
 */
void bmp280::readCalibration(){
    this->dig_T1 = this->readU16bit(0x88);
    this->dig_T2 = this->readS16bit(0x8a);
    this->dig_T3 = this->readS16bit(0x8c);
    this->dig_P1 = this->readU16bit(0x8e);
    this->dig_P2 = this->readS16bit(0x90);
    this->dig_P3 = this->readS16bit(0x92);
    this->dig_P4 = this->readS16bit(0x94);
    this->dig_P5 = this->readS16bit(0x96);
    this->dig_P6 = this->readS16bit(0x98);
    this->dig_P7 = this->readS16bit(0x9a);
    this->dig_P8 = this->readS16bit(0x9c);
    this->dig_P9 = this->readS16bit(0x9e);
}


/**
 * @brief Read unsigned 16bit
 * @retval uint16_t
 */
uint16_t bmp280::readU16bit(uint8_t addr){
    uint16_t data = 0;
    uint8_t buffer[2];
    HAL_I2C_Master_Transmit(&this->i2c,(this->address << 1), &addr, 1, 100);
    HAL_I2C_Master_Receive(&this->i2c,(this->address << 1), buffer, 2, 100);
    data = uint16_t(buffer[1] << 8)|uint16_t(buffer[0]);
    return data;
}


/**
 * @brief Read signed 16bit
 * @retval int16_t
 */
int16_t bmp280::readS16bit(uint8_t addr){
    uint16_t data = this->readU16bit(addr);
    return int16_t(data);
}


/**
 * @brief Get temperature and pressure from sensor
 */
void bmp280::getTempPressure(double* temperature, double* pressure){
    int32_t temperature_raw, pressure_raw;
    this->readAll(&temperature_raw, &pressure_raw);
    *temperature = this->convertTemp(temperature_raw);
    *pressure = this->convertPressure(pressure_raw);
}


/**
 * @brief Read all the data registers
 */
void bmp280::readAll(int32_t *temperature_raw, int32_t *pressure_raw){
    uint8_t buffer[6];
    HAL_I2C_Mem_Read(&this->i2c, (uint16_t)(this->address << 1), 0xF7, I2C_MEMADD_SIZE_8BIT, buffer, 6, 1000);
    *temperature_raw = (buffer[3] << 12)|(buffer[4] << 4)|(buffer[5] >> 4);
    *pressure_raw = (buffer[0] << 12)|(buffer[1] << 4)|(buffer[2] >> 4);
}


/**
 * @brief Get temperature from sensor
 * @retval Temperature (double)
 */
double bmp280::getTemperature(){
    int32_t temp_raw = this->readTemp();
    double temperature = this->convertTemp(temp_raw);
    return temperature;
}


/**
 * @brief Read temperature data registers
 */
int32_t bmp280::readTemp(){
    int32_t temperature_raw = 0;
    uint8_t buffer[3];
    HAL_I2C_Mem_Read(&this->i2c, (uint16_t)(this->address << 1), 0xFA, I2C_MEMADD_SIZE_8BIT, buffer, 3, 1000);
    temperature_raw = (buffer[0] << 12)|(buffer[1] << 4)|(buffer[2] >> 4);
    return temperature_raw;
}

/**
 * @brief Convert values from sensor to celsius
 */
double bmp280::convertTemp(int32_t temp_raw){
    int32_t var1 = ((((temp_raw>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
    int32_t var2 = (((((temp_raw>>4) - ((int32_t)dig_T1)) * ((temp_raw>>4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    int32_t temperature = (t_fine*5 + 128) >> 8;

    return temperature/100.0;
}


/**
 * @brief Get pressure from sensor
 * @retval Pressure (double)
 */
double bmp280::getPressure(){
    int32_t pressure_raw = this->readPressure();
    double pressure = this->convertPressure(pressure_raw);
    return pressure;
}


/**
 * @brief Read pressure data registers
 */
int32_t bmp280::readPressure(){
    int32_t pressure_raw;
    uint8_t buffer[3];
    HAL_I2C_Mem_Read(&this->i2c, (uint16_t)(this->address << 1), 0xF7, 1, buffer, 3, 1000);
    pressure_raw = (int32_t)((buffer[0] << 12)|(buffer[1] << 4)|(buffer[2] >> 4));
    return pressure_raw;
}


/**
 * @brief Convert pressure values from sensor to Pa
 */
double bmp280::convertPressure(int32_t pres_raw){
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
    var2 = var2 + (((int64_t)dig_P4)<<35);
    var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
    var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;

    if (var1 == 0) return 0.0; // avoid exception caused by division by zero

    p = 1048576-pres_raw;
    p = (((p<<31)-var2)*3125)/var1;
    var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);

    return p/256.0;
}