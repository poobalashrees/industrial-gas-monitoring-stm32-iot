/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "lcd_i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE BEGIN PD */
#define DHT11_PIN GPIO_PIN_1
#define DHT11_GPIO_Port GPIOA
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// ===== SYSTEM STATE =====
uint8_t alarm_type = 0;
uint8_t gas_state = 0;

// ===== SERVO =====
uint16_t servo_pos = 1500;

// ===== TIMERS =====
uint32_t last_sensor_time = 0;
uint32_t last_display_time = 0;
uint32_t last_uart_time = 0;
#define GAS_THRESHOLD      350
#define PRESSURE_LOW       960
#define FLOW_MIN           0.5
#define GAS_ON_THRESHOLD   350
#define GAS_OFF_THRESHOLD  300
volatile uint32_t flow_pulse_count = 0;
float flow_rate = 0;
uint32_t last_flow_time = 0;
uint32_t threshold = 350;
uint32_t gas_value = 0;
uint8_t vibration = 0;
uint32_t buzzer_timer = 0;
uint8_t buzzer_state = 0;
float dht_temp = 0;
long hx_raw = 0;
float pressure = 0;

// FLAGS
uint8_t gas_leak = 0;
uint8_t pressure_issue = 0;
uint8_t flow_issue = 0;
uint8_t vibration_alert = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void delay_us(uint16_t us);
uint8_t DHT11_Read(float *temp);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

void Alarm_On()
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}

void Alarm_Off()
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
}
// ===== HX710B =====
#define HX_DT_PORT GPIOB
#define HX_DT_PIN  GPIO_PIN_1

#define HX_SCK_PORT GPIOB
#define HX_SCK_PIN  GPIO_PIN_4

long HX710B_Read(void)
{
    long data = 0;

    // Wait for data ready (DT LOW)
    uint32_t timeout = HAL_GetTick();

    while(HAL_GPIO_ReadPin(HX_DT_PORT, HX_DT_PIN))
    {
        if(HAL_GetTick() - timeout > 100)
            return 0; // prevent hang
    }

    for(int i = 0; i < 24; i++)
    {
        HAL_GPIO_WritePin(HX_SCK_PORT, HX_SCK_PIN, GPIO_PIN_SET);
        delay_us(1);

        data = data << 1;

        HAL_GPIO_WritePin(HX_SCK_PORT, HX_SCK_PIN, GPIO_PIN_RESET);
        delay_us(1);

        if(HAL_GPIO_ReadPin(HX_DT_PORT, HX_DT_PIN))
            data++;
    }

    // 25th pulse
    HAL_GPIO_WritePin(HX_SCK_PORT, HX_SCK_PIN, GPIO_PIN_SET);
    delay_us(1);
    HAL_GPIO_WritePin(HX_SCK_PORT, HX_SCK_PIN, GPIO_PIN_RESET);
    delay_us(1);

    // Convert to signed
    if(data & 0x800000)
        data |= 0xFF000000;

    return data;
}
// ===== DHT22 =====
uint8_t DHT11_Read(float *temp)
{
    uint8_t data[5] = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // ---------- STEP 1: OUTPUT LOW ----------
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(18);

    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_PIN, GPIO_PIN_SET);
    delay_us(30);

    // ---------- STEP 2: INPUT ----------
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);

    // ---------- STEP 3: WAIT RESPONSE ----------
    if(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_PIN))
        return 0;

    uint32_t timeout = HAL_GetTick();
    while(!HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_PIN))
    {
        if(HAL_GetTick() - timeout > 2) return 0;
    }
    while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_PIN));

    // ---------- STEP 4: READ 40 BITS ----------
    for(int i = 0; i < 40; i++)
    {
        while(!HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_PIN));
        delay_us(40);

        if(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_PIN))
            data[i/8] |= (1 << (7 - (i % 8)));

        while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_PIN));
    }

    // ---------- STEP 5: BACK TO OUTPUT ----------
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_PIN, GPIO_PIN_SET);

    // ---------- STEP 6: CHECKSUM ----------
    if(data[4] != (data[0] + data[1] + data[2] + data[3]))
        return 0;

    // ---------- STEP 7: TEMPERATURE ----------
    *temp = data[2];   // DHT11 gives integer temp

    return 1;
}
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
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_Base_Start(&htim2);




  LCD_Init(&hi2c1);
  LCD_Clear();
  LCD_Set_Cursor(0,0);
  LCD_Send_String("SYSTEM START");
  HAL_Delay(1000);
  LCD_Clear();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /* USER CODE BEGIN WHILE */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      uint32_t now = HAL_GetTick();

      // ================= SENSOR READ (EVERY 200 ms) =================
      // ================= SENSOR READ =================
      if(now - last_sensor_time >= 1000)   // DHT22 needs slow read
      {
          last_sensor_time = now;

          // ---- GAS ----
          HAL_ADC_Start(&hadc1);
          HAL_ADC_PollForConversion(&hadc1, 10);
          gas_value = HAL_ADC_GetValue(&hadc1);

          // ---- DHT22 ----

          if(DHT11_Read(&dht_temp) == 0)
          {
              HAL_Delay(10);  // small retry delay

              if(DHT11_Read(&dht_temp) == 0)
              {
                  dht_temp = -1; // still failed
              }
          }
          // ---- HX710B ----
          long sum = 0;
          int valid = 0;

          for(int i=0;i<5;i++)
          {
              long val = HX710B_Read();

              if(val != 0)   // ignore timeout values
              {
                  sum += val;
                  valid++;
              }
          }


        	  static long hx_filtered = 0;

        	  if(valid > 0)
        	  {
        	      long new_val = sum / valid;

        	      // simple low-pass filter
        	      hx_filtered = (hx_filtered * 0.8) + (new_val * 0.2);

        	      hx_raw = hx_filtered;
        	  }
          else
              hx_raw = 0;  // sensor fail

          // Convert to pressure (CALIBRATE THIS)
          pressure = (hx_raw + 857600) / 1000.0;// adjust later

          // ---- VIBRATION ----
          vibration = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);

          // ---- FLOW ----
          if(now - last_flow_time >= 1000)
          {
              flow_rate = flow_pulse_count / 7.5;
              flow_pulse_count = 0;
              last_flow_time = now;
          }
      }
      // ================= PROCESS LOGIC =================
      // GAS hysteresis using calibrated threshold
      if(gas_value > threshold)
          gas_state = 1;
      else if(gas_value < (threshold - 50))
          gas_state = 0;

      pressure_issue = (pressure < PRESSURE_LOW);
      uint8_t flow_issue = (flow_rate < FLOW_MIN);
      uint8_t vibration_alert = (vibration == 1);

      // PRIORITY LOGIC
      if (gas_state && pressure_issue)
          alarm_type = 3;
      else if (gas_state)
          alarm_type = 2;
      else if (!gas_state && flow_issue)
          alarm_type = 1;
      else if (vibration_alert)
          alarm_type = 4;
      else
          alarm_type = 0;

      // ================= ACTUATORS =================
      uint16_t new_pos = servo_pos;

      switch(alarm_type)
      {
          case 3: new_pos = 1000; break;
          case 2: new_pos = 1200; break;
          default: new_pos = 2000; break;
      }

      if(new_pos != servo_pos)
      {
          servo_pos = new_pos;
          __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, servo_pos);
      }

      // Buzzer (non-blocking)
      // ================= BUZZER PATTERN =================
      uint32_t now_buzz = HAL_GetTick();

      switch(alarm_type)
      {
          case 3: // 🔴 CRITICAL (FAST BEEP)
              if(now_buzz - buzzer_timer >= 100)
              {
                  buzzer_timer = now_buzz;
                  buzzer_state ^= 1;
                  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, buzzer_state);
              }
              break;

          case 2: // 🟠 GAS (MEDIUM BEEP)
              if(now_buzz - buzzer_timer >= 300)
              {
                  buzzer_timer = now_buzz;
                  buzzer_state ^= 1;
                  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, buzzer_state);
              }
              break;

          case 1: // 🟡 FLOW (SLOW BEEP)
              if(now_buzz - buzzer_timer >= 700)
              {
                  buzzer_timer = now_buzz;
                  buzzer_state ^= 1;
                  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, buzzer_state);
              }
              break;

          case 4: // 🟣 VIBRATION (DOUBLE TAP)
              if(now_buzz - buzzer_timer >= 200)
              {
                  buzzer_timer = now_buzz;
                  buzzer_state ^= 1;
                  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, buzzer_state);
              }
              break;

          default: // 🟢 SAFE
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
              buzzer_state = 0;
              break;
      }

      // ================= DISPLAY (EVERY 500 ms) =================
      if(now - last_display_time >= 500)
      {
          last_display_time = now;

          char line1[17];
          char line2[17];
          sprintf(line1, "G:%lu P:%.1f", gas_value, pressure);

          if(alarm_type == 3)
              sprintf(line2, "CRITICAL!");
          else if(alarm_type == 2)
              sprintf(line2, "GAS ALERT");
          else if(alarm_type == 1)
              sprintf(line2, "FLOW LOW");
          else if(alarm_type == 4)
              sprintf(line2, "VIBRATION");
          else
              sprintf(line2, "FLOW:%.1f OK", flow_rate);

          LCD_Clear();

          LCD_Set_Cursor(0,0);
          LCD_Send_String(line1);

          LCD_Set_Cursor(1,0);
          LCD_Send_String(line2);
      }
      int _write(int file, char *ptr, int len)
      {
          HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
          return len;
      }

      // ================= UART DEBUG (EVERY 1 sec) =================
      if(now - last_uart_time >= 1000)
      {
          last_uart_time = now;

          char msg[200];

          sprintf(msg,
          "%lu,%.1f,%.2f,%.2f,%d,%d\r\n",
          gas_value,
          dht_temp,
          pressure,
          flow_rate,
          vibration,
          alarm_type);



          HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
      }  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 89;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 19999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 84-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA6 */
  /*Configure GPIO pins : PA1 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE BEGIN 4 */
/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // ===== FLOW SENSOR (PA7) =====
    if(GPIO_Pin == GPIO_PIN_7)
    {
        flow_pulse_count++;   // count pulses
    }

    // ===== BUTTON (PC13) =====
    if(GPIO_Pin == GPIO_PIN_13)
    {
        uint32_t sum = 0;

        for(int i = 0; i < 10; i++)
        {
            HAL_ADC_Start(&hadc1);
            HAL_ADC_PollForConversion(&hadc1, 10);
            sum += HAL_ADC_GetValue(&hadc1);
            HAL_Delay(50);
        }

        threshold = (sum / 10) + 100;
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
#ifdef USE_FULL_ASSERT
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
