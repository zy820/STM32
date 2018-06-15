/******************************************************************************/
/*                                                                            */
/*    $Workfile::   task_gprs.c                                                */
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
#include "task_msgq_gprs.h"
#include "task_gprs.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"
#include "usart.h"
#include "tim.h"

//#ifdef SYS_GPRS_MODULE_ENABLED

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
extern UART_HandleTypeDef USART3_Handler;

/******************************************************************************/
/*                          PRIVATE DATA DEFINITIONS                          */
/******************************************************************************/
#define ATBUFF_SIZE 400
#define GPRS_RX_EN 1       //1:receive
#define TESTDATABUF_LEN      10

uint8_t GPRS_RX_BUF[ATBUFF_SIZE];    //Receive buffer
uint8_t GPRS_TX_BUF[ATBUFF_SIZE];
uint16_t GPRS_RX_STA=0;                //Receiving data state

uint8_t AT_CMD_AT[] = "AT\r\n"; 
uint8_t AT_CMD_CSQ[] = "AT+CSQ\r\n";
uint8_t AT_CMD_CGCLASS[] = "AT+CGCLASS=\"B\"\r\n";
uint8_t AT_CMD_CGDCONT[] = "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n";
uint8_t AT_CMD_CGATT[] = "AT+CGATT?\r\n";
uint8_t AT_CMD_CSTT[] = "AT+CSTT=\"CMNET\"\r\n";   
uint8_t AT_CMD_CIICR[] = "AT+CIICR\r\n"; 
uint8_t AT_CMD_CIFSR[] = "AT+CIFSR\r\n";   
uint8_t AT_CMD_CREG[] = "AT+CREG?\r\n";  
uint8_t AT_CMD_CIPCSGP[] = "AT+CIPCSGP=1,\"CMNET\"\r\n";
uint8_t AT_CMD_CIPHEAD[] = "AT+CIPHEAD=1\r\n";
uint8_t AT_CMD_CIPSTART[] = "AT+CIPSTART=\"TCP\",\"120.76.100.197\",\"10002\"\r\n";   
uint8_t AT_CMD_CIPSEND[] = "AT+CIPSEND\r\n";
uint8_t AT_CMD_CIPCLOSE[] = "AT+CIPCLOSE=1\r\n";  
uint8_t AT_CMD_CIPSHUT[] = "AT+CIPSHUT\r\n";
uint8_t AT_CMD_CIPSTATUS[] = "AT+CIPSTATUS\r\n";
uint8_t AT_CMD_SENDDATA[] = {0x1a};  //ctrl+z
uint8_t AT_CMD_CANCELSEND[] = {0x1b};  //esc

uint8_t TEST_DATA[] = "test:hello";

/******************************************************************************/
/*                          PRIVATE FUNCTION IMPLEMENTATIONS                  */
/******************************************************************************/
void gprsIRQHandler(void)
{
  uint8_t data;
  //uint16_t bytesum = 0;

  if((USART3_Handler.Instance->SR) & UART_FLAG_RXNE)
  {
      CLEAR_BIT(USART3_Handler.Instance->SR,UART_FLAG_RXNE|UART_FLAG_ORE);
      data = USART3_Handler.Instance->DR;
      
//      if(data == '+' && GPRS_RX_BUF[GPRS_RX_STA-1] == 'T' && GPRS_RX_BUF[GPRS_RX_STA-2] == 'A')
//      {
//        GPRS_RX_BUF[0]='A';
//        GPRS_RX_BUF[1]='T';
//        GPRS_RX_BUF[2]='+';
//        GPRS_RX_STA=3;
//      }
//      else
        GPRS_RX_BUF[GPRS_RX_STA++] = data;
      
      HAL_UART_Receive_IT(&USART3_Handler,(uint8_t *)GPRS_RX_BUF,1);
      if(GPRS_RX_STA >= ATBUFF_SIZE)
      {
        GPRS_RX_STA = 0;
      }
      
  }
  /*
  uint8_t val;
  if((__HAL_UART_GET_FLAG(&USART3_Handler,UART_FLAG_RXNE)!=RESET))
  {
     HAL_UART_Receive_IT(&USART3_Handler,&val,1);
     if((GPRS_RX_STA&0x8000)==0)                  //The data is received, but it is not processed
     {
       if(GPRS_RX_STA<ATBUFF_SIZE)
       {
         TIM7->CNT=0;
         if(GPRS_RX_STA==0)
           HAL_TIM_Base_Start(&TIM7_Handler);
         GPRS_RX_BUF[GPRS_RX_STA++]=val;
         printf("GPRS_RX_BUF:~~~%c~~~",GPRS_TX_BUF[GPRS_RX_STA]);
       }
       else
         GPRS_RX_STA|=1<<15;
     }
  }
  HAL_UART_IRQHandler(&USART3_Handler);	
  */
}

/**
  * @brief  After send commands,detect the received response(ack).
  * @param  ack: Required return value
  *         
  * @retval 0: Unexpected response result   other: Received expecting reply
  */
uint8_t check_at_ack(uint8_t *ack)
{
  char *str=0;
  uint8_t val=0;
  if((USART3_Handler.Instance->SR) & UART_FLAG_RXNE)
  {
    if( HAL_UART_Receive_IT(&USART3_Handler,(uint8_t *)GPRS_RX_BUF,1) != HAL_OK )
    {
      printf("HAL_UART_Receive fail!");
    }
  }
  HAL_Delay(1000);     //Ensure data reception
  for(size_t i=0;i<=sizeof(GPRS_RX_BUF)-1;i++)
  {
    printf("%c",GPRS_RX_BUF[i]);
  } 

//  uint8_t GPRS_BUF[ATBUFF_SIZE]="AT+CSQ\r\nOK\r\n";
//  for(size_t i=0;i<=sizeof(GPRS_BUF)-1;i++)
//  {
//    printf("%c",GPRS_BUF[i]);
//  }

  str=strstr((const char*)GPRS_RX_BUF,(const char*)ack);
  if(str!=NULL)
    val=str-GPRS_RX_BUF;
  //val=*str;
  printf("\n\n~~~%d~~~\n\n",val);
  
  //After reading the data,put rxbuff zero
  for(size_t i=0;i<=sizeof(GPRS_RX_BUF)-1;i++)
  {
    GPRS_RX_BUF[i]=0;
  } 
  //memset(GPRS_RX_BUF,0,sizeof(char)*ATBUFF_SIZE);
  return val;
}

/**
  * @brief  Send commands to the GSM
  * @param  cmd: AT command  ack: The expected response,if it is empty,it means there is no need to wait for a reply
  *         waittime: 
  *         
  * @retval 0: correct   1: error
  */
uint8_t SendCmd(uint8_t *cmd,uint8_t *ack)
{
  uint8_t val;
  uint8_t i = 0,len = 1;
  uint8_t *pSend;
  pSend = cmd;
  GPRS_RX_STA=0;           //Make sure the data starts from [0]
  while(*(cmd++)!=0x0a)
  {
    len++;
  }
  //printf("%d \r\n", len);
  HAL_Delay(500);                     //Ensure the query instructions are not disturbed
  while(i++ < len)
  {
     if(HAL_UART_Transmit(&USART3_Handler,(uint8_t *)(pSend++),1,5000) != HAL_OK)
    {
      printf("HAL_UART_Transmit fail!");
    }
  }
  if(ack)
  {
    if(check_at_ack(ack))
      val=0;
    else
      val=1;
    GPRS_RX_STA=0;   //Make sure the data starts from [0]
  }
  return val;
}

/**
  * @brief  Send data
  * @param  
  *         
  * @retval 0: correct   1: error
  */
void SendData(uint8_t *databuff)
{
    uint8_t i = 0;
    uint8_t *pSend;
    pSend = databuff;
    HAL_Delay(500);                     //Ensure the query instructions are not disturbed
    while(i++ < TESTDATABUF_LEN)
  {
      if(HAL_UART_Transmit(&USART3_Handler,(uint8_t *)(pSend++),1,5000) != HAL_OK)
    {
      printf("HAL_UART_Transmit fail!");
    }
  }
  HAL_Delay(100);                      //Ensure the query instruction is valid
}


/**
  * @brief  Send commands(string) to the GSM
  * @param  at: commands(string)
  *         
  * @retval no return value
  */
void SendAT_cmd(char* at,...)  
{  
 // printf("%s %d  success\r\n", __func__, __LINE__);
	uint16_t i,j;
	va_list ap;
	va_start(ap,at);
	vsprintf((char*)GPRS_TX_BUF,at,ap);
	va_end(ap);
	i=strlen((const char*)GPRS_TX_BUF);
	for(j=0;j<i;j++)
	{
		while((USART3->SR&0X40)==0);      
		USART3->DR=GPRS_TX_BUF[j];  
                //printf("GPRS_TX_BUF:~~~%c~~~",GPRS_TX_BUF[j]);
	}
}

/**
  * @brief  After send commands,detect the received response(ack).
  * @param  ack: Required return value
  *         
  * @retval 0: Unexpected response result   other: Received expecting reply
  */
uint8_t* CheckAT_ack(uint8_t *ack)
{
  printf("%s %d  success\r\n", __func__, __LINE__);
  char *str=0;
  if(GPRS_RX_STA&0X8000)
  {
    printf("%s %d  success\r\n", __func__, __LINE__);
    GPRS_RX_BUF[GPRS_RX_STA&0X7FFF]=0;
    for(size_t i=0;i<sizeof(GPRS_RX_BUF)-1;i++)
    {
      printf("@%d %x@",GPRS_RX_BUF[i],GPRS_RX_BUF[i]);
    }
    str=strstr((const char*)GPRS_RX_BUF,(const char*)ack);
  }
  return (uint8_t*)str;
}

/**
  * @brief  Send commands to the GSM
  * @param  cmd: AT command  ack: The expected response,if it is empty,it means there is no need to wait for a reply
  *         waittime: 
  *         
  * @retval 0: correct   1: error
  */
uint8_t SendAT(uint8_t *cmd,uint8_t *ack,uint16_t waittime)
{
  uint8_t val=0;
  GPRS_RX_STA=0;
  if((int )cmd<=0XFF)          //When the cmd is like 0x1a
  {
       USART3->DR = (uint32_t)cmd;
    while((USART3->SR & USART_FLAG_TC)==1);
  }
  else
  {
    printf("cmd_aT");
    SendAT_cmd("%s\r\n",cmd);
  }
    
  if(ack&&waittime)              //Need to wait for reply
  {
    while(--waittime)
    {
      HAL_Delay(10);
      if(GPRS_RX_STA&0X8000)
      {
        for(size_t i=0;i<sizeof(GPRS_RX_BUF)-1;i++)
    {
      printf("@%d %x@",GPRS_RX_BUF[i],GPRS_RX_BUF[i]);
    }
        if(CheckAT_ack(ack))break;   //Get valid data(ack)
        GPRS_RX_STA=0;
      }
    }
    if(waittime==0)
      val=1;
  }
  
  return val;
}

/**
  * @brief  Query network registration status and activate wireless connection
  * @param  no param
  *         
  * @retval 0: correct   other: error
  */
uint8_t GPRSConnect(void)
{
  
  uint8_t err=2,sim_ready=1;
  
  if(SendCmd(AT_CMD_AT,"OK"))
  {
    printf("!!!fail!!!");
  }
  
  if(SendCmd(AT_CMD_CSQ,"+CSQ:"))
  {
    sim_ready=0;
    return err;
  }
  if(sim_ready)
  {
    if(SendCmd(AT_CMD_CREG,"OK")) return 3;
    //Attach or detach from GPRS service
    if(SendCmd(AT_CMD_CGATT,"OK")) return 3;
    
    //Set GPRS for Connection Mode
    //if(SendCmd(AT_CMD_CIPCSGP,"OK")) return 4;
    
    SendCmd(AT_CMD_CIPSHUT,"OK");
    
    //Start Task and Set APN, USER NAME, PASSWORD
    while(SendCmd(AT_CMD_CIPSTATUS,"IP STATUS"))
    {
      SendCmd(AT_CMD_CIPSHUT,"OK");
      SendCmd(AT_CMD_CSTT,"OK");
      SendCmd(AT_CMD_CIICR,"OK");
      SendCmd(AT_CMD_CIFSR,"");
    }
      
    //Bring up wireless connection with GPRS or CSD
    //SendCmd(AT_CMD_CIICR,"OK");
    
    //Get Local IP Address
      //SendCmd(AT_CMD_CIFSR,"OK");
    
  }
  HAL_Delay(2000);
  return 0;
}

/**
  * @brief  Start Up TCP or UDP Connection and send data
  * @param  no param
  *         
  * @retval 0: correct   other: error
  */
uint8_t GPRSSendData_test(void)
{
  if(SendCmd(AT_CMD_CIPSTART,"OK"))
  {
    printf("Connection Fail!");
    return 1;
  }
  printf("Data transmission...");
  if(SendCmd(AT_CMD_CIPSEND,">")==0)   //send data
  {
    SendData(TEST_DATA);
    HAL_Delay(10);
    if(SendCmd(AT_CMD_SENDDATA,"SEND OK")==0)    //ctrl+z(0x1a) send data
      printf("Data sent successfully!");
    else
      printf("Data sent failure!");
  }
  return 0;
}

/**
  * @brief  Start Up TCP or UDP Connection and send data
  * @param  no param
  *         
  * @retval 0: correct   other: error
  */
uint8_t GPRSSendData(void)
{
  //uint8_t i,j;
  TS_GPRS_SENSOR tsSensorData;
  uint8_t connectstatus=0;   //0:connecting 1:Connection successful 2:Connection is closed
  uint8_t heartbeatcnt=0;
  uint8_t count=0;
  uint16_t times=0;
  
  //Start up TCP or UDP connection
  if(SendCmd(AT_CMD_CIPSTART,"OK"))
  {
    printf("Connection Fail!");
    return 1;
  }
  while(1)
  {
    if(heartbeatcnt==0)       //Send data when the heartbeat is normal
    {
      printf("Data transmission...");
      if(SendCmd(AT_CMD_CIPSEND,">")==0)   //send data
      {
        //SendData(&tsSensorData);
        HAL_Delay(10);
        if(SendCmd(AT_CMD_SENDDATA,"SEND OK")==0)    //ctrl+z(0x1a) send data
          printf("Data sent successfully!");
        else
          printf("Data sent failure!");
      }
     else
       SendCmd(AT_CMD_CANCELSEND,0);     //ESC Cancel sending
    }
    if((times%20)==0)
    {
      count++;
      if(connectstatus==2||heartbeatcnt>8)  //Connection is closed or failed to send heartbeat packets eight times in a row,reconnect
      {
        SendCmd(AT_CMD_CIPCLOSE,"CLOSE OK");   //Close TCP or UDP Connection
        SendCmd(AT_CMD_CIPSHUT,"SHUT OK");       //Deactivate GPRS PDP Context
        SendCmd(AT_CMD_CIPSTART,"OK"); //reconnect
        connectstatus=0;
        heartbeatcnt=0;
      }
    }//if(times%20)
    if(connectstatus==0&&(times%200)==0)
    {
      SendCmd(AT_CMD_CIPSTATUS,"OK");  //Query Current Connection Status
      if(strstr((const char*)GPRS_RX_BUF,"CLOSED"))connectstatus=2;
      if(strstr((const char*)GPRS_RX_BUF,"CONNECT OK"))connectstatus=1;
    }
    if(connectstatus==1&&times>=600)  //Connection successful,send a Heartbeat packet every 6 seconds
    {
      times=0;
      if(SendCmd(AT_CMD_CIPSEND,">")==0)
      {
        //SendAT((uint8_t*)0x00,0,0);  //Send data 0x00
        HAL_Delay(20);
        SendCmd(AT_CMD_SENDDATA,0);  //ctrl+z
      }
      else
        SendCmd(AT_CMD_CANCELSEND,0);
      heartbeatcnt++;
      printf("heartbeatcnt:%d\r\n",heartbeatcnt);
    }
    HAL_Delay(20);
    if(GPRS_RX_STA&0X8000)
    {
      GPRS_RX_BUF[GPRS_RX_STA&0X7FFF]=0;   //add end mark [0]
      printf("%s",GPRS_RX_BUF);
      if(heartbeatcnt)
      {
        if(strstr((const char*)GPRS_RX_BUF,"SEND OK"))heartbeatcnt=0;
      }
      GPRS_RX_STA=0;
    }
    times++;
  }// end while
  
}//GPRSSendData

/**
  * @brief  gprs task main loop
  * @param  no param
  *         
  * @retval no return value
  */
static void GprsTaskMain(void const *arg)
{
  uint8_t i=0;
  for(;;)
  {
    i=GPRSConnect();
    if(i)
    {
      printf("Connect error:%d\n",i);
    }
    else
    {
      printf("Connect successful:%d",i);
    }
    GPRSSendData_test();
    //GPRSSendData();
  }
}

/**
  * @brief  start gprs task
  * @param  no param
  *         
  * @retval no return value
  */
void init_gprs_task(void)
{
    //enable_lightSensor();
//    initSensorReport();
    osThreadDef(gprsTask, GprsTaskMain, osPriorityNormal, 0, GPRS_STACK_SIZE);   //128 is stack size (in bytes) Configuration in sys_config.h
    osThreadCreate(osThread(gprsTask), NULL);
}

//#endif // SYS_GPRS_MODULE_ENABLED


/******************************************************************************/
/*                                 END OF FILE                                */
/******************************************************************************/

