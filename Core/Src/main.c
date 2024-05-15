/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

//BH1750
#include "BH1750GY302_1.h"
//OLED
#include "oled.h"
//DHT11
#include "dht11.h"
//����Э���
#include "onenet.h"
//�����豸
#include "esp8266.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    //�����ĸ����ڿ��Ը���huart1Ϊ��������
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1 , 0xffff);
    return ch;
}

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define busytime 10

uint8_t rxBuf=0,txData[100],oled_Data[16],marks_LED,marks_BEEP,alarm_free=busytime;
uint8_t temperature=0,humidity=0;
uint8_t timeCount=0,timeCount_3=0;
float light=0;
/*
����devSubTopic: /YLdemosmart/sub
����devPubTopic: /YLdemosmart/pub
*/
const char *topicsub[]={"/YLdemosmart/sub"};	//�豸���ĵ�Topic(С���򷢲����������)
const char topicpub[]="/YLdemosmart/pub"; 		//�豸������Topic(С���������ݵ�����)
unsigned char *dataPtr = NULL; 					//��ָ��

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
	memset(txData,0,sizeof(txData)/sizeof(uint8_t));
	memset(oled_Data,0,sizeof(oled_Data)/sizeof(uint8_t));
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	HAL_Delay(100);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart2,&rxBuf,1);//��������2�Ľ����ж�
  	while(dht11_init()); 				/* ��ʼ��DHT11 */
	BH1750_Init();			//BH1750��ʼ��	
	ESP8266_Init();				//��ʼ��ESP8266
	OLED_Init();			//OLED��ʼ��
	OLED_ColorTurn(0);		//0������ʾ��1 ��ɫ��ʾ
	OLED_DisplayTurn(0);	//0������ʾ 1 ��Ļ��ת��ʾ

	printf("Hardware init OK\r\n");
	ESP8266_Clear();
	while(OneNet_DevLink())		//���������
	HAL_Delay(500);
	//������ʾ����ɹ�
	BEEP_1;
	HAL_Delay(250);
	BEEP_0;
	LED_0;
	marks_LED=0;
	marks_BEEP=0;
	OneNet_Subscribe(topicsub, 1); 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//1s
		if(timeCount==50)
		{
			Sending();
			clamp_values(&temperature,&humidity,&light,&marks_LED,&marks_BEEP);
			/*�Դ������ж�*/
			if(alarm_free!=busytime)
			{
				alarm_free++;//�ֶ����Ƴ���busytime
			}else{
				/*�Զ�����LED*/
				if(light<100)
				{
					LED_1;
					marks_LED=1;
				}else
				{
					LED_0;
					marks_LED=0;
				}
				/*�Զ�����BEEP*/
				if(humidity>70||temperature>35){
					BEEP_1;
					marks_BEEP=1;
				}else{
					BEEP_0;
					marks_BEEP=0;
				}
			} 
			/*oled*/
			memset(oled_Data,0,16);
			sprintf((char*)oled_Data,"tem:%dc",temperature);
			OLED_ShowString(0,0,oled_Data,16,1);//8*16 
			memset(oled_Data,0,16);
			sprintf((char*)oled_Data,"hum:%d%%",humidity);
			OLED_ShowString(0,16,oled_Data,16,1);//8*16 
			memset(oled_Data,0,16);
			sprintf((char*)oled_Data,"light:%.1F lx",light);
			OLED_ShowString(0,32,oled_Data,16,1);//8*16 
			OLED_Refresh();
			/*��ʱ*/
			timeCount = 0;
			timeCount_3++;
		}
		//���ͼ��3s
		if(timeCount_3 >= 3)									
		{
			printf("OneNet_Publish\r\n");
			memset(txData,0,sizeof(txData)/sizeof(uint8_t));
			sprintf((char*)txData,"{\"Tem\":%d,\"Hum\":%d,\"Light\":%.1f,\"LED\":%d,\"BEEP\":%d}",temperature,humidity,light,marks_LED,marks_BEEP);
			OneNet_Publish(topicpub,(char *)txData);
			ESP8266_Clear();
			
			timeCount = 0;
			timeCount_3=0;
		}
		dataPtr = ESP8266_GetIPD(3);				//�ȴ�3*5ms or 5ms
		if(dataPtr != NULL)OneNet_RevPro(dataPtr);	//��������
		HAL_Delay(10);						
		/*25ms*/
		timeCount++;
		if(timeCount>=250)timeCount=0;
		if(timeCount_3>=250)timeCount_3=0;
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void Sending()
{
	/*��ȡһ����ʪ�ȹ��ն�Ȼ���͸�uartdebug��oled*/
	light=Light_Intensity()/10.0;
	while(dht11_read_data(&temperature, &humidity));   /* ��ȡ��ʪ��ֵ */
	/*uartdebug*/
	printf("tem:%d��,hum:%d%%,lig:%.1flx\r\n",temperature,humidity,light);
}

void clamp_values(uint8_t *temperature, uint8_t *humidity, float *light, uint8_t *marks_LED, uint8_t *marks_BEEP) 
{  
    if (*temperature > UINT8_MAX) {  
        *temperature = UINT8_MAX;  
    }  
    if (*humidity > UINT8_MAX) {  
        *humidity = UINT8_MAX;  
    }  
    if (*light > INT8_MAX) {  
        *light = INT8_MAX;  
    }  
    if (*marks_LED > UINT8_MAX) {  
        *marks_LED = UINT8_MAX;  
    }  
    if (*marks_BEEP > UINT8_MAX) {  
        *marks_BEEP = UINT8_MAX;  
    }  
}


uint8_t Json_getTarget(char *str,char* toBuf)
{
	char* target_start = strstr(str, "\"target\":\"");
	if (target_start == NULL) {
			//"δ�ҵ�\"target\":"
			return 10;
	}
	target_start += strlen("\"target\":\"");
	char* target_end = strchr(target_start, '\"');
	if (target_end == NULL) {
			//"δ�ҵ�'\"'"
			return 20;
	}
	char target[100];
	strncpy(target, target_start, target_end - target_start);
	target[target_end - target_start] = '\0';
	
	strcpy(toBuf,target);
	
	return 0;
}

uint8_t Json_getValue(char *str)
{
	char* value_start = strstr(str, "\"value\":");
	if (value_start == NULL) {
			//"δ�ҵ�\"value\":"
			return 10;
	}
	int value;
	if (sscanf(value_start, "\"value\":%d", &value) != 1) {
			//"δ�ҵ�ֵ"
			return 20;
	}
	return value;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin==SW1_Pin){
		memset(txData,0,sizeof(txData)/sizeof(uint8_t));
		sprintf((char*)txData,"LED�л��ɹ�\r\n");
		HAL_UART_Transmit(&huart1,txData,strlen((char *)txData),1000);
		HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
		marks_LED=!marks_LED;
		alarm_free=0;
	}
	if(GPIO_Pin==SW2_Pin){
		memset(txData,0,sizeof(txData)/sizeof(uint8_t));
		sprintf((char*)txData,"BEEP�л��ɹ�\r\n");
		HAL_UART_Transmit(&huart1,txData,strlen((char *)txData),1000);
		HAL_GPIO_TogglePin(BEEP_GPIO_Port,BEEP_Pin);
		marks_BEEP=!marks_BEEP;
		alarm_free=0;
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
