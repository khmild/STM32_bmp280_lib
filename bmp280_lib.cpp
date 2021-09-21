/**
 * @file bmp280_lib.cpp
 * @author Denys Khmil
 * @brief This file contents all the bmp280 library functions
 */
#include "bmp280_lib.h"

#define TEMP_MSB_ADDR   0xfa
#define TEMP_LSB_ADDR   0xfb
#define TEMP_XLSB_ADDR  0xfc
#define PRESS_MSB_ADDR  0xf7
#define press_LSB_ADDR  0xf8
#define press_XLSB_ADDR 0xf9

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
    this->settings(0b001, 0b011, 0b11);
    this->readCalibration();
}

/**
 * @brief Changes sensor settings
 * @param osrs_p: Pressure measurement settings register.
 * @param osrs_t: Temperature measurement settings register.
 * @param mode: Sensor mode.
 */
void bmp280::settings(uint8_t osrs_p, uint8_t osrs_t, uint8_t mode){
    uint8_t buffer[2];
    buffer[0] = 0xf4;
    buffer[1] = (osrs_t << 5)|(osrs_p << 2)|mode;
    HAL_I2C_Master_Transmit(&this->i2c, (this->address << 1), buffer, 2, 100);
}

/**
 * @brief Get temperature from sensor
 * @retval Temperature (double)
 */
void bmp280::getTemperature(){
    this->readAll();
}

/**
 * @brief Get pressure from sensor
 * @retval Pressure (double)
 */
void bmp280::getPressure(){

}

/**
 * @brief Software reset for sensor
 */
void bmp280::Reset(){

}

/**
 * @brief Read all the data registers
 */
void bmp280::readAll(){
    uint8_t buffer[6];
    buffer[0] = 0xf7;
    HAL_I2C_Master_Transmit(&this->i2c, (this->address << 1), &buffer[0], 1, 100);
    HAL_I2C_Master_Receive(&this->i2c, (this->address << 1), buffer, 6, 100);
    buffer[1] = 0xd0;
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
 * @brief Read sensor id (default 0x58)
 * @retval id uint8_t
 */
uint8_t bmp280::read_id(){
    uint8_t buffer;
    HAL_I2C_Mem_Read(&this->i2c, (uint16_t)(this->address << 1), 0xD0, 1, &buffer, 1, 1000);
    return buffer;
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

void bmp280::convertPressure(){

}

void bmp280::convertTemp(){

}