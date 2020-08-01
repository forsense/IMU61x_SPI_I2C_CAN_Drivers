#include "main.h"
#include "imu_drv.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"

#include "FreeRTOS.h"
#include "task.h"

/**
 * I2C帧字节数
 */
#define I2C_DATA_SIZE_USER 8*4

/**
 * I2C帧定义
 */
struct i2c_slave_frame_publish_user
{
  float accel[3];
  float gyro[3];
  float temp;
  uint32_t check_crc;
} __attribute__((packed));

struct i2c_slave_frame_publish_user i2c_rev_publish_user;
uint32_t i2c_errorCount=0;
uint8_t i2c_id[8];
uint8_t lpf_set,lpf_value;

/**
 * 初始化读取ID和配置滤波器
 */
void I2C_Master_Init(void)
{

//id
    HAL_I2C_Mem_Read(&hi2c1, 0x30, 0x6A, 1, i2c_id, 8, 1000);
    lpf_set = 0x55;

//lpf
    HAL_I2C_Mem_Write(&hi2c1, 0x30, 0x06, 1, &lpf_set, 1, 1000);
    HAL_I2C_Mem_Read(&hi2c1, 0x30, 0x06, 1, &lpf_value, 1, 1000);
}


/**
 * I2C Burst读取
 */
void I2C_Master_Read(void)
{
    uint8_t i2c_data_user[I2C_DATA_SIZE_USER];

    HAL_I2C_Mem_Read(&hi2c1, 0x30, 0x12, 1, i2c_data_user, I2C_DATA_SIZE_USER, 1000);

    uint32_t check_crc=1;
    uint32_t check_sum_i2c=crc_crc32(check_crc,(uint8_t *)(i2c_data_user), I2C_DATA_SIZE_USER-4);

    if(check_sum_i2c== *(uint32_t*)&i2c_data_user[I2C_DATA_SIZE_USER-4])
    {   
         memcpy((uint8_t *)(&i2c_rev_publish_user), (uint8_t *)(i2c_data_user),I2C_DATA_SIZE_USER); 
    }
    else
    {
        i2c_errorCount++;
    }

}

