#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"
#include "imu_drv.h"

#define DUMMY 0x00FF
uint32_t spi_error=0;
uint16_t filter_ctrl;
uint16_t imu_id[4];
uint16_t burst_mode=0;

typedef struct {
    float  gyro[3];
    float  accel[3];
    float  temperature;
    float  attitude[3];
    uint32_t status;
    uint32_t check_crc;
}spi_burst_data_ext_t __attribute__((packed));

typedef struct {
    float  gyro[3];
    float  accel[3];
    uint32_t checksum;
}spi_burst_data_normal_t __attribute__((packed));


spi_burst_data_ext_t     spi_burst_data_ext;
spi_burst_data_normal_t  spi_burst_data_normal;

/**
 * IMU的SPI主机接口初始化
 */
void IMU_Reboot(void)
{
    HAL_GPIO_WritePin(RST_IMU_GPIO_Port, RST_IMU_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(RST_IMU_GPIO_Port, RST_IMU_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(RST_IMU_GPIO_Port, RST_IMU_Pin, GPIO_PIN_SET); 
    HAL_Delay(3000);
}


/**
 * SPI双向传输数据
 */
uint16_t SPI1_ReadWriteByte16(uint16_t cmd)
{
    uint16_t rev;
    HAL_SPI_TransmitReceive(&hspi1,(uint8_t* )&cmd,(uint8_t* )&rev,1,0xFFFFFF); 

    return rev;
}

/**
 * SPI读取单个寄存器
 */
uint16_t SPI1_ReadReg(uint16_t addr)
{
    SPI1_ReadWriteByte16(addr);
    SPI1_ReadWriteByte16(DUMMY);
    return SPI1_ReadWriteByte16(DUMMY);
}


/**
 * 初始化SPI寄存器，扩展帧
 */
void SPI_Master_Init_Ext()
{
    IMU_Reboot();
    SPI1_ReadWriteByte16(0xFE01);//选择配置窗口
    SPI1_ReadWriteByte16(0x8655);//滤波器设置,15hz,数值参考手册
	SPI1_ReadWriteByte16(0x8D01);//配置burst为扩展输出类型

    filter_ctrl = SPI1_ReadReg(0x0600);
    burst_mode = SPI1_ReadReg(0x0D00);

    imu_id[0] = SPI1_ReadReg(0x6A00);
    imu_id[1] = SPI1_ReadReg(0x6C00);
    imu_id[2] = SPI1_ReadReg(0x6E00);
    imu_id[3] = SPI1_ReadReg(0x7000);

    SPI1_ReadWriteByte16(0xFE00);//切换为数据窗口

}

/**
 * SPI读取扩展帧数据
 */
void SPI_Master_Read_Ext()
{
   uint16_t imu_data[30]={0}; 
    /**
     * 偏移2位读取数据
     */
   SPI1_ReadWriteByte16(0x8000);
   SPI1_ReadWriteByte16(0x0000);

    /**
     * 读取整帧共24个uint16
     */
   for (int i = 0; i < sizeof(spi_burst_data_ext_t)/2; i++) 
   imu_data[i] = SPI1_ReadWriteByte16(0x0000);
	
   spi_burst_data_ext_t *pt = (spi_burst_data_ext_t *)imu_data;
   /**
    * CRC按照uint8进行校验，初值是1
    */
   uint32_t check_crc=1;
   if(pt->check_crc==crc_crc32(check_crc,(uint8_t *)pt, sizeof(spi_burst_data_ext_t)-4))
		memcpy(&spi_burst_data_ext, pt, sizeof(spi_burst_data_ext_t));
   else
		spi_error++;
}


/**
 * 初始化SPI寄存器，标准帧
 */
void SPI_Master_Init_Normal()
{
    IMU_Reboot();
    SPI1_ReadWriteByte16(0xFE01);//选择配置窗口
    SPI1_ReadWriteByte16(0x8655);//滤波器设置,15hz,数值参考手册
    SPI1_ReadWriteByte16(0x8D00);//配置burst为标准输出类型

    filter_ctrl = SPI1_ReadReg(0x0600);
    burst_mode = SPI1_ReadReg(0x0D00);

    imu_id[0] = SPI1_ReadReg(0x6A00);
    imu_id[1] = SPI1_ReadReg(0x6C00);
    imu_id[2] = SPI1_ReadReg(0x6E00);
    imu_id[3] = SPI1_ReadReg(0x7000);

    SPI1_ReadWriteByte16(0xFE00);//切换为数据窗口

}

/**
 * SPI读取标准帧数据
 */
void SPI_Master_Read_Normal()
{
    uint16_t imu_data[30]={0}; 
    /**
     * 偏移2位读取数据
     */
    SPI1_ReadWriteByte16(0x8000);
    SPI1_ReadWriteByte16(0x0000);

    /**
     * 读取整帧共14个uint16
     */
    for (int i = 0; i < sizeof(spi_burst_data_normal_t)/2; i++) 
    imu_data[i] = SPI1_ReadWriteByte16(0x0000);

    /**
     * 和校验
     */
    uint32_t checksum_spi_calc = 0;
    for(int i=0;i<sizeof(spi_burst_data_normal_t)/2-2;i++)
    checksum_spi_calc+=imu_data[i];
    
    /**
     * 小端模式
     */
    if(((imu_data[13]<<16) | imu_data[12])==checksum_spi_calc)
    {
        spi_burst_data_normal.gyro[0] = (float)((int32_t)(imu_data[1]<<16) | imu_data[0])/4096000.0f;
        spi_burst_data_normal.gyro[1] = (float)((int32_t)(imu_data[3]<<16) | imu_data[2])/4096000.0f;
        spi_burst_data_normal.gyro[2] = (float)((int32_t)(imu_data[5]<<16) | imu_data[4])/4096000.0f;

        spi_burst_data_normal.accel[0] = (float)((int32_t)(imu_data[7]<<16) | imu_data[6])/327680.0f;
        spi_burst_data_normal.accel[1] = (float)((int32_t)(imu_data[9]<<16) | imu_data[8])/327680.0f;
        spi_burst_data_normal.accel[2] = (float)((int32_t)(imu_data[11]<<16) | imu_data[10])/327680.0f;
    }
    else
    {
        spi_error++;
    }
 
}

