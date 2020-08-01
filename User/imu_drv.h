#ifndef  _IMU_DRIVER_H_
#define  _IMU_DRIVER_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "main.h"
#include "string.h"
#include "stdio.h"

void SPI_Master_Init_Ext(void);
void SPI_Master_Read_Ext(void);
void SPI_Master_Init_Normal(void);
void SPI_Master_Read_Normal(void);
void I2C_Master_Init(void);
void I2C_Master_Read(void);
void CAN_User_Init(CAN_HandleTypeDef* hcan);

uint32_t crc_crc32(uint32_t crc, const uint8_t *buf, uint32_t size);

#ifdef __cplusplus
}
#endif


#endif
