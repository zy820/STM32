#include "sys.h"
#include "usart.h"
#include "led.h"
#include "task_msgq_lux.h"
#include "task_msgq_gprs.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
/************************************************

************************************************/

int main(void)
{	
    HAL_Init();                     //initialize HAL library   
    Stm32_Clock_Init(360,25,2,8);   //configure the clock,180Mhz
    
    USART1_Init(19200);            //initialize USART
    USART6_Init(19200);
    //UART4_Init(19200);
    USART3_Init(19200);             //initialize GPRS TIM7
    
    
    LED_Init();                     //initialize LED 
    LED0=!LED0;  //
    
    //init_lux_task();
    init_gprs_task();
    
    osKernelStart();
    
    while(1)
    {
      
    } 
}


