/*
 * drv_BMP280.c
 *
 *  Created on: 10/11/2023
 *      Author: Coutouillat
 */

#include "stdio.h"
#include "stdlib.h"

#include "main.h"
#include "drv_BMP280.h"

extern I2C_HandleTypeDef hi2c1;

uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3;
uint16_t dig_P1;
int16_t dig_P2;
int16_t dig_P3;
int16_t dig_P4;
int16_t dig_P5;
int16_t dig_P6;
int16_t dig_P7;
int16_t dig_P8;
int16_t dig_P9;

BMP280_S32_t t_fine;

int BMP280_check() {
	uint8_t buf[1];
	HAL_StatusTypeDef ret;
	buf[0] = BMP280_ID_REG;

	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, buf, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with check (I2C Transmit)\r\n");
	}

	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buf, BMP280_ID_LEN,
			HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with check (I2C Receive) \r\n");
	}

	printf("Id: 0x%x...", buf[0]);
	if (buf[0] == BMP280_ID_VAL) {
		printf("Ok\r\n");
		return 0;
	} else {
		printf("not Ok!\r\n");
		return 1;
	}
}

int BMP280_init() {
	HAL_StatusTypeDef ret;
	uint8_t ctrl = (0b010 << 5) | (0b101 << 2) | (0b11);
	/* 				osr_t x2       osr_p x16       normal mode   */

    uint8_t *calibration_data = BMP280_Read_Reg(BMP280_TRIM_REG_MSB, BMP280_TRIM_LEN);

	dig_T1 = (uint16_t)(((uint16_t)calibration_data[1] << 8) | calibration_data[0]);
	dig_T2 = (int16_t)(((int16_t)calibration_data[3] << 8) | calibration_data[2]);
	dig_T3 = (int16_t)(((int16_t)calibration_data[5] << 8) | calibration_data[4]);

    dig_P1 = (uint16_t)(((uint16_t)calibration_data[7] << 8) | calibration_data[6]);
    dig_P2 = (int16_t)(((int16_t)calibration_data[9] << 8) | calibration_data[8]);
    dig_P3 = (int16_t)(((int16_t)calibration_data[11] << 8) | calibration_data[10]);
    dig_P4 = (int16_t)(((int16_t)calibration_data[13] << 8) | calibration_data[12]);
    dig_P5 = (int16_t)(((int16_t)calibration_data[15] << 8) | calibration_data[14]);
    dig_P6 = (int16_t)(((int16_t)calibration_data[17] << 8) | calibration_data[16]);
    dig_P7 = (int16_t)(((int16_t)calibration_data[19] << 8) | calibration_data[18]);
    dig_P8 = (int16_t)(((int16_t)calibration_data[21] << 8) | calibration_data[20]);
    dig_P9 = (int16_t)(((int16_t)calibration_data[23] << 8) | calibration_data[22]);

	free(calibration_data);

	printf("Configure...\r\n");
	ret = BMP280_Write_Reg(BMP280_CTRL_MEAS_REG, ctrl);
	if (ret == 0) {
		printf("Config Ok\r\n");
	} else {
		printf("Config not Ok!\r\n");
		return 1;
	}
	//BMP280_get_trimming();
	return 0;
}

int BMP280_Write_Reg(uint8_t reg, uint8_t value) {
	uint8_t buf[3];
	HAL_StatusTypeDef ret;

	buf[0] = reg;
	buf[1] = value;
	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, buf, 2, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Transmit\r\n");
	}

	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buf, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Receive\r\n");
	}

	if (buf[0] == value) {
		return 0;
	} else {
		return 1;
	}
}

uint8_t* BMP280_Read_Reg(uint8_t reg, uint8_t length) {
	uint8_t *buf;
	HAL_StatusTypeDef ret;

	ret = HAL_I2C_Master_Transmit(&hi2c1, BMP280_ADDR, &reg, 1, HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Transmit\r\n");
	}

	buf = (uint8_t*) malloc(length);
	ret = HAL_I2C_Master_Receive(&hi2c1, BMP280_ADDR, buf, length,
			HAL_MAX_DELAY);
	if (ret != 0) {
		printf("Problem with I2C Receive\r\n");
	}

	return buf;
}

BMP280_S32_t BMP280_get_temperature() {
	uint8_t *buf;
	BMP280_S32_t adc_T;

	buf = BMP280_Read_Reg(BMP280_TEMP_REG_MSB, BMP280_TEMP_LEN);

	adc_T = ((BMP280_S32_t) (buf[0]) << 12) | ((BMP280_S32_t) (buf[1]) << 4)
			| ((BMP280_S32_t) (buf[2]) >> 4);

	free(buf);

	printf("Temperature: ");
	printf("0X%05lX", adc_T);
	printf("\r\n");

	return adc_T;
}

int BMP280_get_pressure() {
	uint8_t *buf;
	BMP280_S32_t adc_P;

	buf = BMP280_Read_Reg(BMP280_PRES_REG_MSB, BMP280_PRES_LEN);

	adc_P = ((BMP280_S32_t) (buf[0]) << 12) | ((BMP280_S32_t) (buf[1]) << 4)
			| ((BMP280_S32_t) (buf[2]) >> 4);

	free(buf);

	printf("Pressure:    0x");
	printf("%05lX", adc_P);
	printf("\r\n");

	return 0;
}

BMP280_S32_t BMP280_compensate_temp(BMP280_S32_t adc_T) {
    // Compensation de la température
    BMP280_S32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((BMP280_S32_t)dig_T1 << 1))) * ((BMP280_S32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((BMP280_S32_t)dig_T1)) * ((adc_T >> 4) - ((BMP280_S32_t)dig_T1))) >> 12) * ((BMP280_S32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

BMP280_S32_t BMP280_compensate_press(BMP280_S32_t adc_P) {
// Compensation de la pression
    BMP280_S64_t var3, var4, p;
    var3 = ((BMP280_S64_t)t_fine) - 128000;
    var4 = var3 * var3 * (BMP280_S64_t)dig_P6;
    var4 = var4 + ((var3 * (BMP280_S64_t)dig_P5) << 17);
    var4 = var4 + (((BMP280_S64_t)dig_P4) << 35);
    var3 = ((var3 * var3 * (BMP280_S64_t)dig_P3) >> 8) + ((var3 * (BMP280_S64_t)dig_P2) << 12);
    var3 = (((((BMP280_S64_t)1) << 47) + var3)) * ((BMP280_S64_t)dig_P1) >> 33;
    if (var3 == 0) {
        return 0; // Pour éviter la division par zéro
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var4) * 3125) / var3;
    var3 = (((BMP280_S64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var4 = (((BMP280_S64_t)dig_P8) * p) >> 19;
    p = ((p + var3 + var4) >> 8) + (((BMP280_S64_t)dig_P7) << 4);

    return (BMP280_S32_t)p; // Vous pouvez également renvoyer la pression si nécessaire
}


