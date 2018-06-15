/******************************************************************************/
/*                                                                            */
/*    $Workfile::   task_lux.c                                                */
/*                                                                            */
/*    $Revision::   0.1                                                  $    */
/*                                                                            */
/*     $Modtime::   02/05/2018                                          $    */
/*                                                                            */
/*      $Author::                                                        $    */
/*                                                                            */
/*        Owner::                                                             */
/*                                                                            */
/*  Description::                                                             */
/*                                                                            */
/*   Company   ::                                                             */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                               INCLUDE FILES                                */
/******************************************************************************/
#include <stdlib.h>

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "sys.h"
#include "task_msgq_lux.h"

//#ifdef SYS_LUX_MODULE_ENABLED

/******************************************************************************/
/*                            CONSTANT DEFINITIONS                            */
/****************************************************************************/

/******************************************************************************/
/*                              MACRO DEFINITIONS                             */
/******************************************************************************/

/******************************************************************************/
/*                              TYPE DEFINITIONS                              */
/******************************************************************************/

/******************************************************************************/
/*                          PUBLIC DATA DEFINITIONS                           */
/******************************************************************************/
extern UART_HandleTypeDef USART1_Handler;

#define usartHandle USART1_Handler

#define upper_computer
//#define report_sensor
//#define uart_print

//#define sensor_data_type     int32_t
//#define sensor_data_accuracy   0
#define sensor_data_type     float
#define sensor_data_accuracy   1

void sensorIRQHandler(void);
void sensor_SendAndRecBytes(uint8_t* txbuff);            
sensor_data_type sensor_BytesToRealval(uint8_t* rxbuff);
sensor_data_type sensor_WindDirection_BytesToRealval(uint8_t* rxbuff);

//static osMutexId MyDebugLock;

/******************************************************************************/
/*                          PRIVATE DATA DEFINITIONS                          */
/******************************************************************************/
#define MAX_SENSOR_BUF_LEN      64
#define SENSOR_RXBUF_LEN      7
#define SENSOR_TXBUF_LEN      8
#define SENSOR_DATA_NUM      7
static uint8_t sensorUartBuffer[MAX_SENSOR_BUF_LEN];
//static uint8_t sensorUartBufferIndex = 0;
//static uint8_t sensorReportEnabled = 0;
static uint8_t prxbuff = 0;
//static uint8_t ptxbuff = 0;
static uint8_t rxbuff[SENSOR_RXBUF_LEN];
static uint8_t WindDirection_txbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x01,0x00,0x01,0xD1,0xEB};      //The average wind direction
static uint8_t WindSpeed_txbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x04,0x00,0x01,0xC1,0xEA};          //average wind speed
static uint8_t Temperature_txbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x06,0x00,0x01,0x60,0x2A};        //temperature
static uint8_t Humidity_txbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x07,0x00,0x01,0x31,0xEA};           //humidity
static uint8_t AirPressuretxbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x08,0x00,0x01,0x01,0xE9};         //Atmospheric pressure
//static uint8_t Rainfall_txbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x09,0x00,0x01,0x50,0x29};           //rainfall
//static uint8_t Radiation_txbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x0A,0x00,0x01,0xA0,0x29};          //global radiation
//static uint8_t Ultraviolet_txbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x0B,0x00,0x01,0xF1,0xE9};        //ultraviolet
static uint8_t Noise_txbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x0C,0x00,0x01,0x40,0x28};              //noise
static uint8_t Pm25_txbuff[SENSOR_TXBUF_LEN] = {0x30,0x03,0x00,0x0D,0x00,0x01,0x11,0xE8};               //PM2.5


/******************************************************************************/
/*                          PRIVATE FUNCTION IMPLEMENTATIONS                  */
/******************************************************************************/
void sensorIRQHandler(void)
{
  uint8_t data;
  //uint16_t bytesum = 0;

  if((usartHandle.Instance->SR) & UART_FLAG_RXNE)
  {
      CLEAR_BIT(usartHandle.Instance->SR,UART_FLAG_RXNE|UART_FLAG_ORE);
      data = usartHandle.Instance->DR;
      /*
      sensorUartBuffer[sensorUartBufferIndex++] = data;
      if(sensorUartBufferIndex >= MAX_SENSOR_BUF_LEN)
      {
        sensorUartBufferIndex = 0;
      }
      */
      
      //Accept 7 bytes,determines whether the next byte is 0x30(put bytesum zero)
      //bytesum++;
      //if(bytesum == SENSOR_RXBUF_LEN)
      //{
      
        if(data == 0x30)
        {
          //Continue to accept
          prxbuff = 0;
        }/*
        else
        {
          //No, resend
        }
      }
      else
      {
        //No, resend
      }
      */
      rxbuff[prxbuff++] = data;
      HAL_UART_Receive_IT(&usartHandle,(uint8_t *)sensorUartBuffer,1);
      if(prxbuff >= MAX_SENSOR_BUF_LEN)
      {
        prxbuff = 0;
      }
      
  }
}

/**
  * @brief  lux task main loop
  * @param  no param
  *         
  * @retval no return value
  */
static void LuxTaskMain(void const *arg)
{
    for(;;)
    {
     
      HAL_Delay(5000);
      sensor_data_type sensorbuff[SENSOR_DATA_NUM];
      sensor_data_type sensor = 0;
      uint8_t i = 0;
      uint8_t psensorbuff = 0;
      
      while(++i)
      {
        switch(i)
        {
        case 1:
          sensor_SendAndRecBytes(Pm25_txbuff);
          sensor = sensor_BytesToRealval(rxbuff);
#ifdef upper_computer
          //Put sensor in sensorbuff,after saving, print together
          sensorbuff[psensorbuff++] = sensor;
#endif

#ifdef report_sensor
          //Pass the data to the upload data interface
#endif
          
#ifdef uart_print
          if(sensor_data_accuracy)
            printf("Pm2.5 : %.1f\n",sensor);
          else
            printf("Pm2.5 : %d\n",sensor);
#endif
          break;
        case 2:
          sensor_SendAndRecBytes(Noise_txbuff);
          sensor = sensor_BytesToRealval(rxbuff);
#ifdef upper_computer
          //Put sensor in sensorbuff,after saving, print together
          sensorbuff[psensorbuff++] = sensor;
#endif

#ifdef report_sensor
          //Pass the data to the upload data interface
#endif

#ifdef uart_print
          if(sensor_data_accuracy)
            printf("Noise : %.1f\n",sensor);
          else
            printf("Noise : %d\n",sensor);
#endif
          break;
        case 3:
          sensor_SendAndRecBytes(Temperature_txbuff);
          sensor = sensor_BytesToRealval(rxbuff);
#ifdef upper_computer
          //Put sensor in sensorbuff,after saving, print together
          sensorbuff[psensorbuff++] = sensor;
#endif

#ifdef report_sensor
          //Pass the data to the upload data interface
#endif

#ifdef uart_print
          if(sensor_data_accuracy)
            printf("Temperature : %.1f\n",sensor);
          else
            printf("Temperature : %d\n",sensor);
#endif 
          break;
        case 4:
          sensor_SendAndRecBytes(Humidity_txbuff);
          sensor = sensor_BytesToRealval(rxbuff);
#ifdef upper_computer
          //Put sensor in sensorbuff,after saving, print together
          sensorbuff[psensorbuff++] = sensor;
#endif

#ifdef report_sensor
          //Pass the data to the upload data interface
#endif

#ifdef uart_print
          if(sensor_data_accuracy)
            printf("Humidity : %.1f\n",sensor);
          else
            printf("Humidity : %d\n",sensor);
#endif  
          break;
        case 5:
          sensor_SendAndRecBytes(AirPressuretxbuff);
          sensor = sensor_BytesToRealval(rxbuff);
#ifdef upper_computer
          //Put sensor in sensorbuff,after saving, print together
          sensorbuff[psensorbuff++] = sensor;
#endif

#ifdef report_sensor
          //Pass the data to the upload data interface
#endif

#ifdef uart_print
          if(sensor_data_accuracy)
            printf("AirPressure : %.1f\n",sensor);
          else
            printf("AirPressure : %d\n",sensor);
#endif 
          break;
        case 6:
          sensor_SendAndRecBytes(WindDirection_txbuff);
          sensor = sensor_WindDirection_BytesToRealval(rxbuff);
#ifdef upper_computer
          //Put sensor in sensorbuff,after saving, print together
          sensorbuff[psensorbuff++] = sensor;
#endif

#ifdef report_sensor
          //Pass the data to the upload data interface
#endif

#ifdef uart_print
          if(sensor_data_accuracy)
            printf("WindDirection : %.1f\n",sensor);
          else
            printf("WindDirection : %d\n",sensor);
#endif 
          break;
        case 7:
          sensor_SendAndRecBytes(WindSpeed_txbuff);
          sensor = sensor_BytesToRealval(rxbuff);
#ifdef upper_computer
          //Put sensor in sensorbuff,after saving, print together
          sensorbuff[psensorbuff++] = sensor;
#endif

#ifdef report_sensor
          //Pass the data to the upload data interface
#endif

#ifdef uart_print
          if(sensor_data_accuracy)
            printf("WindSpeed : %.1f\n",sensor);
          else
            printf("WindSpeed : %d\n",sensor);
#endif  
        default:
          break;
        }
        if(i == 8)
        {
          i = 0;
          psensorbuff = 0;
          break;
          //HAL_Delay(5000);        //Query cycle
        }
        HAL_Delay(1000);
      }
      
#ifdef upper_computer                                   //close task_iot
//      osMutexWait(MyDebugLock, osWaitForever);
      printf("\n\n");
      for(int8_t j = 0;j<8;j++)
      {
        if(sensor_data_accuracy)
        {
          if(j==7)
            printf("##\n");                       //Valid data mark bits,convenient upper computer serial port reading
          else
            printf("%.1f_",sensorbuff[j]);
        }
         else
         {
           if(j==7)
            printf("##\n");
           else
             printf("%d_",sensor);
         }
      }
      HAL_Delay(5000); 
//      osMutexRelease(MyDebugLock);
#endif
      /*
      sensor_SendAndRecBytes(txbuff);
      noise = sensor_BytesToRealval(rxbuff);
      printf("noise : %d\n",noise);
      */
    }
}


/**
  * @brief  send sensor command and receive data(bytes)
  * @param  sensor query command
  *         
  * @retval no return value
  */
void sensor_SendAndRecBytes(uint8_t* txbuff)
{
        while(1)
        {
          uint8_t i = 0;
          uint8_t *pSend;
          pSend = txbuff;
          HAL_Delay(500);                     //Ensure the query instructions are not disturbed
          while(i++ < SENSOR_TXBUF_LEN)
        {
            if(HAL_UART_Transmit(&usartHandle,(uint8_t *)(pSend++),1,5000) != HAL_OK)
          {
            printf("HAL_UART_Transmit fail!");
          }
        }
        HAL_Delay(100);                      //Ensure the query instruction is valid
        
        if((usartHandle.Instance->SR) & UART_FLAG_RXNE)
        {
          //HAL_UART_Receive_IT(&MyDebugUart,(uint8_t *)sensorUartBuffer,1);
          if( HAL_UART_Receive_IT(&usartHandle,(uint8_t *)sensorUartBuffer,1) != HAL_OK )
          {
            printf("HAL_UART_Receive fail!");
          }
        }
        
        //Verify acceptable data,if the first byte accepted is 0x30,break while;accept data incorrectly and resend instructions
        //HAL_Delay(500);
        if(0x30 == rxbuff[0])
          break;
        else
        {
          ;
//          printf("###Receive fail!###");
//          HAL_Delay(500);
        }
        
        //HAL_Delay(500);
        /*
        for(size_t i=0;i<=sizeof(rxbuff)-1;i++)
        {
        printf("@%d %x@",rxbuff[i],rxbuff[i]);
        } 
        */
        }
}

/**
  * @brief  converts the byte data returned by sensor to real value
  * @param  byte data returned by sensor
  *         
  * @retval int32_t value
  */
sensor_data_type sensor_BytesToRealval(uint8_t* rxbuff)
{
        int32_t high = rxbuff[3];             //The data is in the fourth (high), 5 (low) byte in the return frame
        int32_t low = rxbuff[4];
        high = high&0x000000ff;               //Read low 8 bits
        low = low&0x000000ff;
        high = high<<8;                       //Combine two bytes in one word(Convert to decimal value)
        low = low+high;
        sensor_data_type val = (sensor_data_type)low/10;
        //After reading the data,put rxbuff zero
        for(uint8_t i = 0;i<7;i++)
        {
          rxbuff[i] = 0;
        }
        return val;
        //If the actual value is less than 1,return 0
        /*
        sensor_data_type val = (sensor_data_type)low/10;
        if(val >= 1.0)
          return (sensor_data_type)val;
        else
          */
}

/**
  * @brief  converts the WindDirection data returned by sensor to real value
  * @param  WindDirection byte data returned by sensor
  *         
  * @retval int32_t value
  */
sensor_data_type sensor_WindDirection_BytesToRealval(uint8_t* rxbuff)
{
        int32_t high = rxbuff[3];             
        int32_t low = rxbuff[4];
        high = high&0x000000ff;               
        low = low&0x000000ff;
        high = high<<8;                    
        low = low+high;
        sensor_data_type val = (sensor_data_type)low;
        return val;
}

/******************************************************************************/
/*                      PUBLIC FUNCTION IMPLEMENTATIONS                       */
/******************************************************************************/
/**
  * @brief  start lux task
  * @param  no param
  *         
  * @retval no return value
  */
void init_lux_task(void)
{
    //enable_lightSensor();
    
//    initSensorReport();
    osThreadDef(luxTask, LuxTaskMain, osPriorityNormal, 0, LUX_STACK_SIZE);   //128 is stack size (in bytes) Configuration in sys_config.h
    osThreadCreate(osThread(luxTask), NULL);
}

//#endif // SYS_LUX_MODULE_ENABLED


/******************************************************************************/
/*                                 END OF FILE                                */
/******************************************************************************/
