/******************************************************************************/
/*                                                                            */
/*    $Workfile::   usart.c                                                */
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

#include "usart.h"
#include "task_gprs.h"
#include "tim.h"
////////////////////////////////////////////////////////////////////////////////// 	 
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 	  
#endif

//////////////////////////////////////////////////////////////////////////////////	 

////////////////////////////////////////////////////////////////////////////////// 	  	  
//#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)	

FILE __stdout;       
   
void _sys_exit(int x) 
{ 
	x = x; 
} 
//Redefine fputc function 
int fputc(int ch, FILE *f)
{ 	
	while((USART6->SR&0X40)==0);//Cycle to send   
	USART6->DR = (uint8_t) ch;      
	return ch;
}	
////////////////////////////////////////////////////////////////////////////////// 	  	  
UART_HandleTypeDef USART1_Handler;     //sensor
UART_HandleTypeDef USART2_Handler;
UART_HandleTypeDef USART3_Handler;    //GPRS
UART_HandleTypeDef UART4_Handler;    //
UART_HandleTypeDef USART6_Handler;   //debug_printf

//todo: add USARTx Handle

//Initialize usart1
//bound:Band Rate
void USART1_Init(uint32_t bound)
{	
	USART1_Handler.Instance=USART1;					
	USART1_Handler.Init.BaudRate=bound;				    
	USART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   
	USART1_Handler.Init.StopBits=UART_STOPBITS_1;	    
	USART1_Handler.Init.Parity=UART_PARITY_NONE;		    
	USART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   
	USART1_Handler.Init.Mode=UART_MODE_TX_RX;		    
	HAL_UART_Init(&USART1_Handler);					   
	
//	HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE);
  
}

//Initialize usart6
//bound:Band Rate
void USART6_Init(uint32_t bound)
{	
	USART6_Handler.Instance=USART6;					
	USART6_Handler.Init.BaudRate=bound;				    
	USART6_Handler.Init.WordLength=UART_WORDLENGTH_8B;   
	USART6_Handler.Init.StopBits=UART_STOPBITS_1;	    
	USART6_Handler.Init.Parity=UART_PARITY_NONE;		    
	USART6_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   
	USART6_Handler.Init.Mode=UART_MODE_TX_RX;		    
	HAL_UART_Init(&USART6_Handler);					   
	
//	HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE);
  
}

//Initialize usart2
//bound:Band Rate
void USART2_Init(uint32_t bound)
{	
	USART2_Handler.Instance=USART2;					
	USART2_Handler.Init.BaudRate=bound;				    
	USART2_Handler.Init.WordLength=UART_WORDLENGTH_8B;   
	USART2_Handler.Init.StopBits=UART_STOPBITS_1;	    
	USART2_Handler.Init.Parity=UART_PARITY_NONE;		    
	USART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   
	USART2_Handler.Init.Mode=UART_MODE_TX_RX;		    
	HAL_UART_Init(&USART2_Handler);					   
	
//	HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE);
  
}

//Initialize uart5
//bound:Band Rate
void UART4_Init(uint32_t bound)
{	
	UART4_Handler.Instance=UART4;					   
	UART4_Handler.Init.BaudRate=bound;				
	UART4_Handler.Init.WordLength=UART_WORDLENGTH_8B;   
	UART4_Handler.Init.StopBits=UART_STOPBITS_1;	   
	UART4_Handler.Init.Parity=UART_PARITY_NONE;		
	UART4_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE; 
	UART4_Handler.Init.Mode=UART_MODE_TX_RX;		 
	HAL_UART_Init(&UART4_Handler);					   
  
}

//Initialize usart3
//bound:Band Rate
void USART3_Init(uint32_t bound)
{	
	USART3_Handler.Instance=USART3;					   
	USART3_Handler.Init.BaudRate=bound;				
	USART3_Handler.Init.WordLength=UART_WORDLENGTH_8B;   
	USART3_Handler.Init.StopBits=UART_STOPBITS_1;	   
	USART3_Handler.Init.Parity=UART_PARITY_NONE;		
	USART3_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE; 
	USART3_Handler.Init.Mode=UART_MODE_TX_RX;		 
	HAL_UART_Init(&USART3_Handler);					   
	
//	HAL_UART_Receive_IT(&UART4_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE);
  
}
//todo: add USARTx_Init()

//UART MspInit,enable clock,pin configuration,interrupt configuration
//This function is called by HAL_UART_Init()
//huart:UART handle
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)
	{
            __HAL_RCC_GPIOA_CLK_ENABLE();			
            __HAL_RCC_USART1_CLK_ENABLE();			
            
            /**USART1 GPIO Configuration    
            USART1_TX ------> PA9
            USART1_RX ------> PA10
            */
            GPIO_Initure.Pin=GPIO_PIN_9;			
            GPIO_Initure.Mode=GPIO_MODE_AF_PP;		
            GPIO_Initure.Pull=GPIO_PULLUP;			
            GPIO_Initure.Speed=GPIO_SPEED_FAST;		
            GPIO_Initure.Alternate=GPIO_AF7_USART1;	
            HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	

            GPIO_Initure.Pin=GPIO_PIN_10;			
            HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	
            
            HAL_NVIC_EnableIRQ(USART1_IRQn);		
            HAL_NVIC_SetPriority(USART1_IRQn,3,3);	
    
	}
        else if(huart->Instance==USART6)
	{
            __HAL_RCC_GPIOC_CLK_ENABLE();			
            __HAL_RCC_USART6_CLK_ENABLE();			
            
            /**USART6 GPIO Configuration    
            USART6_TX ------> PC6
            USART6_RX ------> PC7
            */
            GPIO_Initure.Pin=GPIO_PIN_6;			
            GPIO_Initure.Mode=GPIO_MODE_AF_PP;		
            GPIO_Initure.Pull=GPIO_PULLUP;			
            GPIO_Initure.Speed=GPIO_SPEED_FAST;	
            GPIO_Initure.Alternate=GPIO_AF8_USART6;
            HAL_GPIO_Init(GPIOC,&GPIO_Initure);	   	

            GPIO_Initure.Pin=GPIO_PIN_7;			
            HAL_GPIO_Init(GPIOC,&GPIO_Initure);	   	
            
            HAL_NVIC_EnableIRQ(USART6_IRQn);		
            HAL_NVIC_SetPriority(USART6_IRQn,3,3);	
    
	}
        else if(huart->Instance==USART2)
	{
            __HAL_RCC_GPIOA_CLK_ENABLE();			
            __HAL_RCC_USART2_CLK_ENABLE();			
            
            /**USART2 GPIO Configuration    
            USART2_TX ------> PA2
            USART2_RX ------> PA3
            */
            GPIO_Initure.Pin=GPIO_PIN_2;			
            GPIO_Initure.Mode=GPIO_MODE_AF_PP;		
            GPIO_Initure.Pull=GPIO_NOPULL;			
            GPIO_Initure.Speed=GPIO_SPEED_FAST;		
            //GPIO_Initure.Alternate=GPIO_AF7_USART2;	
            HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	

            GPIO_Initure.Pin=GPIO_PIN_3;
            HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	
            
            HAL_NVIC_EnableIRQ(USART2_IRQn);		
            HAL_NVIC_SetPriority(USART2_IRQn,3,3);	
    
	}
        else if(huart->Instance==UART4)
        {
            __HAL_RCC_GPIOC_CLK_ENABLE();			
            __HAL_RCC_UART4_CLK_ENABLE();
            
            /**UART4 GPIO Configuration    
            UART4_TX ------> PC10
            UART4_RX ------> PC11
            */
            GPIO_Initure.Pin=GPIO_PIN_10;			
            GPIO_Initure.Mode=GPIO_MODE_AF_PP;		
            GPIO_Initure.Pull=GPIO_PULLUP;			
            GPIO_Initure.Speed=GPIO_SPEED_FAST;		
            GPIO_Initure.Alternate=GPIO_AF8_UART4;	
            HAL_GPIO_Init(GPIOC,&GPIO_Initure);	   	

            GPIO_Initure.Pin=GPIO_PIN_11;			
            HAL_GPIO_Init(GPIOC,&GPIO_Initure);	   	
            
            //Enable the UART Data Register not empty Interrupt
            __HAL_UART_ENABLE_IT(&UART4_Handler,UART_IT_RXNE);  
            HAL_NVIC_EnableIRQ(UART4_IRQn);		
            HAL_NVIC_SetPriority(UART4_IRQn,3,3);
            
            TIM7_Init(100,9000);
            HAL_TIM_Base_Stop(&TIM7_Handler);
            GPRS_RX_STA=0;
    
        }
        else if(huart->Instance==USART3)
        {
            __HAL_RCC_GPIOB_CLK_ENABLE();			
            __HAL_RCC_USART3_CLK_ENABLE();
            
            /**USART3 GPIO Configuration    
            USART3_TX ------> PB10
            USART3_RX ------> PB11
            */
            GPIO_Initure.Pin=GPIO_PIN_10;
            GPIO_Initure.Mode=GPIO_MODE_AF_PP;
            GPIO_Initure.Pull=GPIO_PULLUP;
            GPIO_Initure.Speed=GPIO_SPEED_FAST;
            GPIO_Initure.Alternate=GPIO_AF7_USART3;
            HAL_GPIO_Init(GPIOB,&GPIO_Initure);
            
            GPIO_Initure.Pin=GPIO_PIN_11;
            HAL_GPIO_Init(GPIOB,&GPIO_Initure);
            
            //Enable the UART Data Register not empty Interrupt
            __HAL_UART_ENABLE_IT(&USART3_Handler,UART_IT_RXNE);  
            HAL_NVIC_EnableIRQ(USART3_IRQn);		
            HAL_NVIC_SetPriority(USART3_IRQn,3,3);
            
            //TIM7_Init(100,9000);
            //HAL_TIM_Base_Stop(&TIM7_Handler);
            //GPRS_RX_STA=0;
        }
        else if(huart->Instance==UART5)
	{
            __HAL_RCC_GPIOC_CLK_ENABLE();
            __HAL_RCC_GPIOD_CLK_ENABLE();
            __HAL_RCC_UART5_CLK_ENABLE();			
            
            /**USART5 GPIO Configuration    
            USART5_TX ------> PC12
            USART5_RX ------> PD2
            */
            GPIO_Initure.Pin=GPIO_PIN_12;			
            GPIO_Initure.Mode=GPIO_MODE_AF_PP;		
            GPIO_Initure.Pull=GPIO_PULLUP;			
            GPIO_Initure.Speed=GPIO_SPEED_FAST;		
            //GPIO_Initure.Alternate=GPIO_AF8_UART5;	
            HAL_GPIO_Init(GPIOC,&GPIO_Initure);	   	

            GPIO_Initure.Pin=GPIO_PIN_2;			
            HAL_GPIO_Init(GPIOD,&GPIO_Initure);	   	
            
            HAL_NVIC_EnableIRQ(UART5_IRQn);		
            HAL_NVIC_SetPriority(UART5_IRQn,3,3);	
    
	}
        //else if()
        //todo: add USARTx GPIO init
}




