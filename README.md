# IMU61x SPI/I2C/CAN驱动示例程序

基于STM32Cube自动生成的STM32F405处理器的HAL程序，引脚见F4-dev.ioc和F4-dev.jpg。

晶振12MHz，FreeRTOS频率400Hz，SPI驱动分为标准帧格式和扩展帧格式。

1）标准帧SPI读取
``` c++
SPI_Master_Init_Normal();
SPI_Master_Read_Normal();
```
2）扩展帧SPI读取
``` c++
SPI_Master_Init_Ext();
SPI_Master_Read_Ext();
```
3）I2C读取
``` c++
I2C_Master_Init();
I2C_Master_Read();
```
4）CAN读取
``` c++
CAN_User_Init();
```

注意：需要设置IMU为对应模式，方法见说明书手册。