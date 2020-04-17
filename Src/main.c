/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "stdbool.h"
#include "usbh_def.h"
#include "usbh_cdc.h"
#include "gps.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_TimeTypeDef sTime1;
RTC_DateTypeDef sDate1;
RTC_HandleTypeDef hrtc;
SD_HandleTypeDef hsd;
UART_HandleTypeDef huart1;
extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_RTC_Init(void);
void MX_USART1_UART_Init(void);
void MX_SDIO_SD_Init(void);
void MX_USB_HOST_Process(void);
void Test_SD(void);
void Print_DateTime(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	char buff[1024];
	bool bProcessing = true;
	/* GPS handle  */
	gps_t hgps;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_USB_HOST_Init();
  
	/* USER CODE BEGIN 2 */
	
	printf("\033[2J"); // Clear terminal display (or scroll out on PuTTy)
	printf("Uart up and running\r\n");

//	Test_SD();
//	HAL_Delay(5000);
	
	printf("\033[2J");  // Clear terminal display
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

		if(bProcessing) {
    	if(USBH_CDC_Receive(&hUsbHostFS, (uint8_t *)buff, 1024) == USBH_OK ) {
				printf("buff - %s\r\n", buff);

				/* Process all input data */
				gps_process(&hgps, buff, strlen((char *)buff));  // minimal gps/NMEA parsing by Tilen MAJERLE

				/* Print messages */
				printf("Latitude: %f degrees\r\n", hgps.latitude);
				printf("Longitude: %f degrees\r\n", hgps.longitude);
				printf("Altitude: %f meters\r\n", hgps.altitude);
				printf("Date: %02d/%02d/%02d\r\n", hgps.date, hgps.month, hgps.year);
				printf("Time: %02d:%02d:%02d\r\n", hgps.hours, hgps.minutes, hgps.seconds);
//				bProcessing = false;      //Uncomment to stop after each USB read
			}
		} else {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);       //Toggle the onboard LEDS
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
			HAL_Delay(500);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
			HAL_Delay(500);
		
			Print_DateTime();
	}

		//Check the two onboard buttons
		if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == GPIO_PIN_RESET) { //Stop USB Processing
			USBH_CDC_Stop(&hUsbHostFS);
			bProcessing = false;
		}
		
		if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4) == GPIO_PIN_RESET) { //Reset USB Processing
			bProcessing = true;
		}
		
		/* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

//void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
//{
//}

static void fault_err (FRESULT rc)
{
  const char *str =
                    "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
                    "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
                    "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
                    "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
  FRESULT i;

  for (i = (FRESULT)0; i != rc && *str; i++) {
    while (*str++) ;
  }
  printf("rc=%u FR_%s\n\r", (UINT)rc, str);
  while(1);
}

void Print_DateTime(void)
{
	uint8_t buffer[32];

	HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BCD);

	buffer[0] = (sDate1.Date / 16) + 48;
	buffer[1] = (sDate1.Date % 16) + 48;
	buffer[2] = '.';
	buffer[3] = (sDate1.Month / 16) + 48;
	buffer[4] = (sDate1.Month % 16) + 48;
	buffer[5] = '.';
	buffer[6] = '2';
	buffer[7] = '0';
	buffer[8] = (sDate1.Year / 16) + 48;
	buffer[9] = (sDate1.Year % 16) + 48;
	buffer[10] =' ';
	buffer[11] = (sTime1.Hours / 16) + 48;
	buffer[12] = (sTime1.Hours % 16) + 48;
	buffer[13] = ':';
	buffer[14] = (sTime1.Minutes / 16) + 48;
	buffer[15] = (sTime1.Minutes % 16) + 48;
	buffer[16] = ':';
	buffer[17] = (sTime1.Seconds / 16) + 48;
	buffer[18] = (sTime1.Seconds % 16) + 48;
	buffer[19] = '\r';
	buffer[20] = '\n';
	buffer[21] = 0;
	printf("%s", buffer);
}

void Test_SD(void)
{
	uint8_t buffer[64];
	uint8_t buffer2[64];
	FATFS filesystem;
	FIL   file;
	UINT byteCount;
	DIR dir;				    /* Directory object */
	FILINFO fno;			  /* File information object */
	UINT bw, br;
	FRESULT ret;
	HAL_SD_CardInfoTypeDef CardInfo;
	HAL_SD_CardCIDTypeDef pCID;
	
	printf("path: '%s'\r\n", SDPath);
  
	ret = f_mount(&filesystem, SDPath, 1);
	if(ret == FR_OK){
		printf("Card Mounted\r\n");
		
		printf("Open a test file (message.txt) \n\r");
		ret = f_open(&file, "MESSAGE.TXT", FA_READ);
		if (ret) {
			printf("Test file doesn't exist (message.txt)\n\r");
		} else {
			printf("Type the file content\n\r");
			for (;;) {
				ret = f_read(&file, buffer, sizeof(buffer), &br);	/* Read a chunk of file */
				if (ret || !br) {
					break;			/* Error or end of file */
				}
				buffer[br] = 0;
				printf("%s\r\n",buffer);
			}
			if (ret) {
				printf("Read the file error\n\r");
				fault_err(ret);
			}

			printf("Close the file\n\r");
			ret = f_close(&file);
			if (ret) {
				printf("Close the file error\n\r");
			}
		}

		/*  hello.txt write test*/
		HAL_Delay(50);	
		printf("Create a new file (hello.txt)\n\r");
		ret = f_open(&file, "HELLO.TXT", FA_WRITE | FA_CREATE_ALWAYS);
		if (ret) {
			printf("Create a new file error\n\r");
			fault_err(ret);
		} else {
			printf("Write a text data. (hello.txt)\n\r");
			ret = f_write(&file, "Hello world!", 14, &bw);
			if (ret) {
				printf("Write a text data to file error\n\r");
			} else {
				printf("%u bytes written\n\r", bw);
			}
			HAL_Delay(50);
			printf("Close the file\n\r");
			ret = f_close(&file);
			if (ret) {
				printf("Close the hello.txt file error\n\r");			
			}
		}

		/*  hello.txt read test*/
		HAL_Delay(50);	
		printf("read the file (hello.txt)\n\r");
		ret = f_open(&file, "HELLO.TXT", FA_READ);
		if (ret) {
			printf("open hello.txt file error\n\r");
		} else {
			printf("Type the file content(hello.txt)\n\r");
			for (;;) {
				ret = f_read(&file, buffer, sizeof(buffer), &br);	/* Read a chunk of file */
				if (ret || !br) {
					break;			/* Error or end of file */
				}
				buffer[br] = 0;
				printf("%s\r\n",buffer);
			}
			if (ret) {
				printf("Read file (hello.txt) error\n\r");
				fault_err(ret);
			}

			printf("Close the file (hello.txt)\n\r");
			ret = f_close(&file);
			if (ret) {
				printf("Close the file (hello.txt) error\n\r");
			}
		}

		/*  directory display test*/
		HAL_Delay(50);	
		printf("Open root directory\n\r");
		ret = f_opendir(&dir, "");
		if (ret) {
			printf("Open root directory error\n\r");	
		} else {
			printf("Directory listing...\n\r");
			for (;;) {
				ret = f_readdir(&dir, &fno);		/* Read a directory item */
				if (ret || !fno.fname[0]) {
					break;	/* Error or end of dir */
				}
				if (fno.fattrib & AM_DIR) {
					printf("  <dir>  %s\n\r", fno.fname);
				} else {
					printf("%8lu  %s\n\r", fno.fsize, fno.fname);
				}
			}
			if (ret) {
				printf("Read a directory error\n\r");
				fault_err(ret);
			}
		}
		printf("Test completed\n\r");
		
		BSP_SD_GetCardInfo(&CardInfo);

		printf("SD Card Type: %d\r\n", CardInfo.CardType);
		printf("SD Card Version: %d\r\n",CardInfo.CardVersion);
		printf("SD Card Class: %d\r\n", CardInfo.Class);
		printf("SD Card Relative Address: %d\r\n", CardInfo.RelCardAdd);
		printf("SD Card Capacity in blocks: %d\r\n", CardInfo.BlockNbr);
		printf("SD Card Block size in bytes: %d\r\n", CardInfo.BlockSize);
		printf("SD Card Logical Capacity in blocks: %d\r\n", CardInfo.LogBlockNbr);
		printf("SD Card Logical Block size in bytes: %d\r\n", CardInfo.LogBlockSize);

		ret = f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
		printf("File Open:(%d)\r\n", ret);
		ret = f_write(&file, buffer, 30, &byteCount);
		printf("File Written:(%d)\r\n", ret);
		ret = f_close(&file);
		printf("File Closed:(%d)\r\n", ret);
 
		ret = f_open(&file, "test2.txt", FA_WRITE | FA_CREATE_ALWAYS);
		printf("File Open:(%d)\r\n", ret);
		ret = f_write(&file, buffer, 30, &byteCount);
		printf("File Written:(%d)\r\n", ret);
		ret = f_close(&file);
		printf("File Closed:(%d)\r\n", ret);

		f_open(&file, "test.txt", FA_READ);
		f_read(&file, buffer2, 5, &byteCount);
		f_close(&file);
	}else{
		printf("mount fail :(%d)\r\n", ret);
	}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
    
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date 
  */
  sTime.Hours = 0x13;
  sTime.Minutes = 0x30;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = RTC_MONTH_MARCH;
  sDate.Date = 0x22;
  sDate.Year = 0x20;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PE4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

//  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_SET);

  /*Configure GPIO pins : PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f) {
  /* Place your implementation of fputc here */
  /* e.g. write a character to the UART and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	
  return ch;
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
