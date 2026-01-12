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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "API_func.h"
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

/* USER CODE BEGIN PV */
input_vars input;

volatile char container[1024];
volatile int temp;
volatile int key;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  UB_VGA_Screen_Init(); // Init VGA-Screen

  int i;

  for(i = 0; i < LINE_BUFLEN; i++)
	  input.line_rx_buffer[i] = 0;

  // Reset some stuff
  input.byte_buffer_rx[0] = 0;
  input.char_counter = 0;
  input.command_execute_flag = FALSE;

  // HAl wants a memory location to store the charachter it receives from the UART
  // We will pass it an array, but we will not use it. We declare our own variable in the interupt handler
  // See stm32f4xx_it.c
  HAL_UART_Receive_IT(&huart2, input.byte_buffer_rx, BYTE_BUFLEN);

  // Test to see if the screen reacts to UART
  unsigned char colorTest = TRUE;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_Delay(1501);
  while (1)
  {
	HAL_Delay(1500);
    API_clearscreen(VGA_COL_BLACK);

    for(int i = 10; i < 240; i++)
    {
      API_draw_line(1, i, 319, i, i, 1, 0);
    }
    API_draw_bitmap(130, 90, 1);
    HAL_Delay(1000);

    API_clearscreen(VGA_COL_WHITE);

    API_draw_text(30, 60, VGA_COL_RED, "Test VoOr dEfAulT aA bB cC", "default", 8, 0, 32);
    API_draw_text(30, 80, VGA_COL_BLUE, "Test VoOr minecraft", "Minecraft", 8, 1, 34);
    API_draw_text(30, 100, VGA_COL_BLUE, "Test VoOr minecraft 2", "Minecraft", 16, 1, 36);
    API_draw_text(30, 120, VGA_COL_BLUE, "Test VoOr Arial", "Arial", 8, 1, 30);
    API_draw_text(30, 140, VGA_COL_BLUE, "Test VoOr Arial 2", "Arial", 16, 1, 32);
    API_draw_bitmap(220, 10, 60);
    for(int i = 0; i < 100; i++)
    {
      API_draw_text(30, 160, VGA_COL_RED, "Gratis koffie", "SGA", 16, 1, 0);
      HAL_Delay(200);
      API_draw_text(30, 160, VGA_COL_GREEN, "Gratis koffie", "SGA", 16, 1, 0);
      HAL_Delay(200);
      API_draw_text(30, 160, VGA_COL_BLUE, "Gratis koffie", "SGA", 16, 1, 0);
      HAL_Delay(200);
    }
    HAL_Delay(200);
  
   for(int j = 0; j < 1000; j++)
   {
     API_clearscreen(VGA_COL_BLACK);
     for(int i = 0; i < 14; i++)
     {
       API_draw_bitmap(0, 0, i+3);
       HAL_Delay(100);
     }
     API_clearscreen(VGA_COL_BLACK);
     for(int i = 0; i < 42; i++)
     {
       API_draw_bitmap(0, 0, i+17);
       HAL_Delay(100);
     }
     API_clearscreen(VGA_COL_BLACK);
     HAL_Delay(100);
     API_draw_bitmap(70, 20, 59);
     HAL_Delay(100);
   }
    API_clearscreen(VGA_COL_WHITE);

    API_draw_line(5, 5, 5, 100, VGA_COL_BLUE, 1, 0);
    UB_VGA_SetPixel(5, 5, VGA_COL_BLACK);
    UB_VGA_SetPixel(5, 100, VGA_COL_BLACK);

    API_draw_line(5, 5, 100, 5, VGA_COL_BLUE, 1, 0);
    UB_VGA_SetPixel(5, 5, VGA_COL_BLACK);
    UB_VGA_SetPixel(100, 5, VGA_COL_BLACK);

    API_draw_line(10, 10, 100, 100, VGA_COL_BLACK, 1, 0);
    UB_VGA_SetPixel(10, 10, VGA_COL_RED);
    UB_VGA_SetPixel(100, 100, VGA_COL_RED);

    API_draw_line(50, 10, 140, 100, VGA_COL_RED, 3, 0);
    UB_VGA_SetPixel(50, 10, VGA_COL_BLACK);
    UB_VGA_SetPixel(140, 100, VGA_COL_BLACK);

    API_draw_line(90, 50, 20, 60, VGA_COL_MAGENTA, 5, 0);
    UB_VGA_SetPixel(90, 50, VGA_COL_BLACK);
    UB_VGA_SetPixel(20, 60, VGA_COL_BLACK);

    API_draw_line(120, 30, 250, 10, VGA_COL_GREEN, 7, 0);
    UB_VGA_SetPixel(120, 30, VGA_COL_BLACK);
    UB_VGA_SetPixel(250, 10, VGA_COL_BLACK);

    API_draw_line(200, 200, 200, 200, VGA_COL_GREEN, 7, 0);
    UB_VGA_SetPixel(200, 200, VGA_COL_BLACK);

    HAL_Delay(5000);

    API_clearscreen(VGA_COL_WHITE);
    API_draw_bitmap(0, 0, 1);
    API_draw_bitmap(70, 0, 2);

    HAL_Delay(5000);

    API_clearscreen(VGA_COL_WHITE);


    API_draw_rectangle(50, 50, 150, 90, VGA_COL_BLUE, 0, 0, 0 );

    UB_VGA_SetPixel(50, 50, VGA_COL_BLACK);
    UB_VGA_SetPixel(200, 50, VGA_COL_BLACK);
    UB_VGA_SetPixel(200, 140, VGA_COL_BLACK);
    UB_VGA_SetPixel(50, 140, VGA_COL_BLACK);

    API_draw_rectangle(50, 150, 130, 80, VGA_COL_RED, 1, 0, 0 );

    UB_VGA_SetPixel(50, 150, VGA_COL_BLACK);
    UB_VGA_SetPixel(180, 150, VGA_COL_BLACK);
    UB_VGA_SetPixel(180, 230, VGA_COL_BLACK);
    UB_VGA_SetPixel(50, 230, VGA_COL_BLACK);

    HAL_Delay(5000);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

#ifdef __GNUC__
	#define USART_PRINTF int __io_putchar(int ch)		//With GCC/RAISONANCE printf calls __io_putchar()
#else
	#define USART_PRINTF int fputc(int ch, FILE *f)		//With other compiler printf calls fputc()
#endif /* __GNUC__ */

//Retargets the C library printf function to the USART
USART_PRINTF
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);	//Write character to UART2
	return ch;												//Return the character
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
