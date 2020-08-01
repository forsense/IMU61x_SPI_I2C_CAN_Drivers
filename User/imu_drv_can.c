#include "FreeRTOS.h"
#include "task.h"
#include "imu_drv.h"
#include "can.h"

typedef struct  
{
float roll;
float pitch;
}CAN_STRUCT1;

typedef struct   
{
float yaw;
float gx;
}CAN_STRUCT2;

typedef struct   
{
float gy;
float gz;
}CAN_STRUCT3;

typedef struct   
{
float ax;
float ay;
}CAN_STRUCT4;

typedef struct   
{
float az;
uint16_t temp;
uint16_t index;
}CAN_STRUCT5;

CAN_STRUCT1 can1_data;
CAN_STRUCT2 can2_data;
CAN_STRUCT3 can3_data;
CAN_STRUCT4 can4_data;
CAN_STRUCT5 can5_data;
CAN_RxHeaderTypeDef     RxMeg;

/**
 * CAN 过滤器配置
 */
void CAN_User_Init(CAN_HandleTypeDef* hcan ) 
{
   CAN_FilterTypeDef  sFilterConfig;
   HAL_StatusTypeDef  HAL_Status;
  
  sFilterConfig.FilterBank = 0;                       
  sFilterConfig.FilterMode =  CAN_FILTERMODE_IDMASK;  
  sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;    
    
  sFilterConfig.FilterIdHigh = 0;  
  sFilterConfig.FilterIdLow  = 0;    
    
  sFilterConfig.FilterMaskIdHigh =0;
  sFilterConfig.FilterMaskIdLow  =0; 
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;  
 
  sFilterConfig.FilterActivation = ENABLE;    
  sFilterConfig.SlaveStartFilterBank  = 0; 
 
  HAL_Status=HAL_CAN_ConfigFilter(hcan, &sFilterConfig);
  HAL_Status=HAL_CAN_Start(hcan);  
  if(HAL_Status!=HAL_OK){
    Error_Handler();
  }  
  HAL_Status=HAL_CAN_ActivateNotification(hcan,   CAN_IT_RX_FIFO0_MSG_PENDING);
  if(HAL_Status!=HAL_OK){
    Error_Handler();
  }
}

/**
 * 接收回调函数
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)  
{
  uint8_t  Data[8];
  HAL_StatusTypeDef HAL_RetVal;
  if(hcan ==&hcan1){  
    HAL_RetVal=HAL_CAN_GetRxMessage(hcan,  CAN_RX_FIFO0, &RxMeg,  Data);
    if ( HAL_OK==HAL_RetVal){                                   

		if(RxMeg.StdId==101)
		{
		memcpy((uint8_t*)&can1_data,Data,8);
		}

		if(RxMeg.StdId==102)
		{
		memcpy((uint8_t*)&can2_data,Data,8);
		}

		if(RxMeg.StdId==103)
		{
		memcpy((uint8_t*)&can3_data,Data,8);
		}

		if(RxMeg.StdId==104)
		{
		memcpy((uint8_t*)&can4_data,Data,8);
		}

		if(RxMeg.StdId==105)
		{
		memcpy((uint8_t*)&can5_data,Data,8);
		}
    }
  }
}
 