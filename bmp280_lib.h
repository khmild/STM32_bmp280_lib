/**
 * @file bmp280_lib.h
 * @author Denys Khmil
 * @brief This file contents the bmp280 class
 */
#ifndef BMP280_LIB
#define BMP280_LIB

#include "main.h"
#include <math.h>

class bmp280{
public:
    /*CONSTRUCTORS*/
    bmp280(I2C_HandleTypeDef _i2c, uint8_t _address);
    bmp280(I2C_HandleTypeDef _i2c);
    
    /*UTILITY FUNCTIONS*/
    void settings(uint8_t osrs_p, uint8_t osrs_t, uint8_t mode);
    void setConfig(uint8_t t_sb);
    uint8_t conversionRunning();
    uint8_t dataCopying();
    void Reset();
    uint8_t read_id(); 

    /*MEASURINGS*/
    void getTempPressure(double* temperature, double* pressure);
    double getTemperature();
    double getPressure();

private:
    /*READ FUNCTIONS*/
    void readAll(int32_t *temperature_raw, int32_t *pressure_raw);
    int32_t readTemp();
    int32_t readPressure();
    uint16_t readU16bit(uint8_t addr);
    int16_t readS16bit(uint8_t addr);
    void readCalibration();
    
    /*CONVERT FUNCTIONS*/
    double convertPressure(int32_t pres_raw);
    double convertTemp(int32_t temp_raw);
    
    /*SENSOR PARAMETERS*/
    I2C_HandleTypeDef i2c;
    uint8_t address;

    /*TEMPERATURE CALIBRATION CONSTANTS*/
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;

    /*PRESSURE CALIBRATION CONSTANTS*/
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;

    int32_t t_fine;
};

#endif