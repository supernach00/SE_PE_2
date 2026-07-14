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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdint.h>
#include "ui.h"
#include "monitor.h"
#include "ssd1306.h"

#include "medida.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_BUFFER_SIZE (256)

// -----------------------------------------------------
// TODO: factorizar esto a donde corresponde
// Valores de ARR para el TIM3 según la escala de tiempo requerida
#define TIM3_ARR_RAPIDO   (100 - 1)    // Muestreo muy rápido (~100 microsegundos por muestra)
#define TIM3_ARR_MEDIO    (1000 - 1)  // Muestreo medio (~1 milisegundo por muestra)
#define TIM3_ARR_LENTO    (10000 - 1) // Muestreo lento (~10 milisegundos por muestra)

typedef enum {
    RANGE_330R = 0,
    RANGE_10K,
    RANGE_1M
} RangeState_t;

typedef enum {
    TIME_BASE_RAPIDA = 0,
    TIME_BASE_MEDIA,
    TIME_BASE_LENTA
} TimeBase_t;

// Prototipos de funciones auxiliares
void set_hardware_range(RangeState_t range);
uint32_t calcular_capacidad_final(int16_t indice_tau, TimeBase_t base_tiempo, RangeState_t rango_R);
// TODO -----------------------------------------------------

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for uiTask */
osThreadId_t uiTaskHandle;
const osThreadAttr_t uiTask_attributes = {
  .name = "uiTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for inputsTask */
osThreadId_t inputsTaskHandle;
const osThreadAttr_t inputsTask_attributes = {
  .name = "inputsTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal3,
};
/* Definitions for monitorTask */
osThreadId_t monitorTaskHandle;
const osThreadAttr_t monitorTask_attributes = {
  .name = "monitorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for uiQueue */
osMessageQueueId_t uiQueueHandle;
uint8_t uiQueueBuffer[ 10 * 4 ];
osStaticMessageQDef_t uiQueueControlBlock;
const osMessageQueueAttr_t uiQueue_attributes = {
  .name = "uiQueue",
  .cb_mem = &uiQueueControlBlock,
  .cb_size = sizeof(uiQueueControlBlock),
  .mq_mem = &uiQueueBuffer,
  .mq_size = sizeof(uiQueueBuffer)
};
/* Definitions for monitorQueue */
osMessageQueueId_t monitorQueueHandle;
uint8_t monitorQueueBuffer[ 1 * 56 ];
osStaticMessageQDef_t monitorQueueControlBlock;
const osMessageQueueAttr_t monitorQueue_attributes = {
  .name = "monitorQueue",
  .cb_mem = &monitorQueueControlBlock,
  .cb_size = sizeof(monitorQueueControlBlock),
  .mq_mem = &monitorQueueBuffer,
  .mq_size = sizeof(monitorQueueBuffer)
};
/* Definitions for muestreoQueue */
osMessageQueueId_t muestreoQueueHandle;
uint8_t samplesQueueBuffer[ 4 * sizeof( MuestreoQueue_t ) ];
osStaticMessageQDef_t samplesQueueControlBlock;
const osMessageQueueAttr_t muestreoQueue_attributes = {
  .name = "muestreoQueue",
  .cb_mem = &samplesQueueControlBlock,
  .cb_size = sizeof(samplesQueueControlBlock),
  .mq_mem = &samplesQueueBuffer,
  .mq_size = sizeof(samplesQueueBuffer)
};
/* USER CODE BEGIN PV */

/* Globables para monitor */
volatile uint32_t FU_acc = 0;
volatile uint32_t FU_time_delta = 0;

/* Globales para UI */
volatile UI_t ui1 = {
	ESTADO_INICIO,
	0, // Seleccion inicial
	1, // Flag update background
	1, // Flag update seleccion
	0, // Flag update datos
};

Config_t config1 = { // Configuracion default
	.modo = MODO_MULTIPLE,
	.parametro = PARAMETRO_R,
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
void StartDefaultTask(void *argument);
void oledEntry(void *argument);
void encoderEntry(void *argument);
void monitorEntry(void *argument);

/* USER CODE BEGIN PFP */
void UI_FSM_Switch(UI_t *ui, Evento_e evento);
void callback_in(int tag);
void callback_out(int tag);

/**
 * Ajustan los GPIOs para el fondo de escala más adecuado
 * de para medir resistencias
 */
//MuestreoQueue_t configureAutoRangeResistor();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint16_t adc_buffer[ADC_BUFFER_SIZE] = {0};
volatile Bool buffer_ready = FALSE;

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
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

//  HAL_ADC_Start_DMA(&hadc1,
//		  (uint32_t *)adc_buffer,
//		  ADC_BUFFER_SIZE);

  // Se comienza el Timer 3 que da la base de tiempo
  // para la conversión del ADC 1
  HAL_TIM_Base_Start(&htim3);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of uiQueue */
  uiQueueHandle = osMessageQueueNew (10, 4, &uiQueue_attributes);

  /* creation of monitorQueue */
  monitorQueueHandle = osMessageQueueNew (1, 56, &monitorQueue_attributes);

  /* creation of muestreoQueue */
  muestreoQueueHandle = osMessageQueueNew (4, sizeof(MuestreoQueue_t), &muestreoQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of uiTask */
  uiTaskHandle = osThreadNew(oledEntry, NULL, &uiTask_attributes);

  /* creation of inputsTask */
  inputsTaskHandle = osThreadNew(encoderEntry, NULL, &inputsTask_attributes);

  /* creation of monitorTask */
  monitorTaskHandle = osThreadNew(monitorEntry, NULL, &monitorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T3_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
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
  hi2c1.Init.ClockSpeed = 400000;
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

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 72-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DEBUG_PIN_Pin|HOOK_IDLE_Pin|HOOK_MONITOR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, HOOK_UI_Pin|HOOK_INPUTS_Pin|HOOK5_Pin|GPIO330R_Pin
                          |GPIO10K_Pin|GPIO1M_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BOTON_ENCODER_Pin */
  GPIO_InitStruct.Pin = BOTON_ENCODER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BOTON_ENCODER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DEBUG_PIN_Pin HOOK_IDLE_Pin HOOK_MONITOR_Pin */
  GPIO_InitStruct.Pin = DEBUG_PIN_Pin|HOOK_IDLE_Pin|HOOK_MONITOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : HOOK_UI_Pin HOOK_INPUTS_Pin HOOK5_Pin GPIO330R_Pin
                           GPIO10K_Pin GPIO1M_Pin */
  GPIO_InitStruct.Pin = HOOK_UI_Pin|HOOK_INPUTS_Pin|HOOK5_Pin|GPIO330R_Pin
                          |GPIO10K_Pin|GPIO1M_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
 * @brief Configura los GPIOs para activar la resistencia de carga/descarga correcta.
 */
void set_hardware_range(RangeState_t range) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Todos se configuran como Entrada (Alta impedancia / Flotante / Z) por defecto
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // También puedes usar GPIO_MODE_ANALOG para menor ruido
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    switch (range){

    case RANGE_330R:
        // 1. Ponemos en Z las resistencias de 10K y 1M
        GPIO_InitStruct.Pin = GPIO10K_Pin;
        HAL_GPIO_Init(GPIO10K_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO1M_Pin;
        HAL_GPIO_Init(GPIO1M_GPIO_Port, &GPIO_InitStruct);

        // 2. Activamos la de 330R como salida en ALTO
        GPIO_InitStruct.Pin = GPIO330R_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(GPIO330R_GPIO_Port, GPIO330R_Pin, GPIO_PIN_SET);
        break;

    case RANGE_10K:
        // 1. Ponemos en Z las resistencias de 330R y 1M
        GPIO_InitStruct.Pin = GPIO330R_Pin;
        HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO1M_Pin;
        HAL_GPIO_Init(GPIO1M_GPIO_Port, &GPIO_InitStruct);

        // 2. Activamos la de 10K como salida en ALTO
        GPIO_InitStruct.Pin = GPIO10K_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIO10K_GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(GPIO10K_GPIO_Port, GPIO10K_Pin, GPIO_PIN_SET);
        break;

    case RANGE_1M:
        // 1. Ponemos en Z las resistencias de 330R y 10K
        GPIO_InitStruct.Pin = GPIO330R_Pin;
        HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO10K_Pin;
        HAL_GPIO_Init(GPIO10K_GPIO_Port, &GPIO_InitStruct);

        // 2. Activamos la de 1M como salida en ALTO
        GPIO_InitStruct.Pin = GPIO1M_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        HAL_GPIO_Init(GPIO1M_GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_SET);
        break;

    default:
        // Fallback preventivo: Apagamos todo a Z para proteger el hardware
        GPIO_InitStruct.Pin = GPIO330R_Pin;
        HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO10K_Pin;
        HAL_GPIO_Init(GPIO10K_GPIO_Port, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO1M_Pin;
        HAL_GPIO_Init(GPIO1M_GPIO_Port, &GPIO_InitStruct);
        break;
    }
}

/**
 * @brief Calcula el valor final de la capacidad basado en el índice de Tau y el hardware.
 * Formula: C = t / R.
 * t = indice_tau * Periodo_Muestreo
 */
uint32_t calcular_capacidad_final(int16_t indice_tau, TimeBase_t base_tiempo, RangeState_t rango_R) {
	// TODO: cambiar esto para no usar double, y hacer todo con enteros
    double tiempo_por_muestra_seg = 0.0;
    double resistencia_ohms = 0.0;

    // 1. Determinar el tiempo de paso del TIM3 (Asumiendo Clock del timer a 72MHz y Prescaler = 72-1 -> 1 microsegundo por tick)
    switch(base_tiempo) {
        case TIME_BASE_RAPIDA: tiempo_por_muestra_seg = (double)TIM3_ARR_RAPIDO / 1000000.0; break;
        case TIME_BASE_MEDIA:  tiempo_por_muestra_seg = (double)TIM3_ARR_MEDIO / 1000000.0; break;
        case TIME_BASE_LENTA:  tiempo_por_muestra_seg = (double)TIM3_ARR_LENTO / 1000000.0; break;
    }

    // 2. Determinar la resistencia utilizada para la carga
    switch(rango_R) {
        case RANGE_330R: resistencia_ohms = 330.0; break;
        case RANGE_10K:  resistencia_ohms = 10000.0; break;
        case RANGE_1M:   resistencia_ohms = 1000000.0; break;
    }

    // Tiempo total hasta llegar a Tau (63.2%)
    double t_tau = (double)indice_tau * tiempo_por_muestra_seg;

    // C = Tau / R
    double capacidad_faradios = t_tau / resistencia_ohms;

    // Lo convertimos a NanoFaradios (nF) para devolver un entero cómodo para la UI
    uint32_t capacidad_nf = (uint32_t)(capacidad_faradios * 1000000000.0);

    return capacidad_nf;
}

//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
//{
//    buffer_ready = TRUE;
//}

/**
 * @brief Hice que cuando se llene el buffer el callback
 * levante a la tarea default, así podemos muestrear
 * con un tiempo fijo.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Despierta a la tarea de medición inmediatamente cuando el buffer de 256 muestras está lleno
    vTaskNotifyGiveFromISR(defaultTaskHandle, &xHigherPriorityTaskWoken);

    // Fuerza el cambio de contexto si la prioridad de defaultTask es mayor que la interrumpida
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Funciones para el analizador logico */
void callback_in(int tag)
{
	switch (tag) {
	case TAG_TASK_IDLE:
		FU_time_delta = HAL_GetTick();
		HAL_GPIO_WritePin(HOOK_IDLE_GPIO_Port, HOOK_IDLE_Pin, GPIO_PIN_SET);
		break;
	case TAG_TASK_INPUTS:   HAL_GPIO_WritePin(HOOK_INPUTS_GPIO_Port, HOOK_INPUTS_Pin, GPIO_PIN_SET); break;
	case TAG_TASK_UI:       HAL_GPIO_WritePin(HOOK_UI_GPIO_Port, HOOK_UI_Pin, GPIO_PIN_SET); break;
	case TAG_TASK_MONITOR:  HAL_GPIO_WritePin(HOOK_MONITOR_GPIO_Port, HOOK_MONITOR_Pin, GPIO_PIN_SET); break;
	case TAG_TASK_MUESTREO: HAL_GPIO_WritePin(HOOK5_GPIO_Port, HOOK5_Pin, GPIO_PIN_SET); break;
	default: break;
	}
}

void callback_out(int tag){
	switch (tag) {
	case TAG_TASK_IDLE:
		uint32_t time_delta = HAL_GetTick() - FU_time_delta;
		FU_acc += time_delta;
		HAL_GPIO_WritePin(HOOK_IDLE_GPIO_Port, HOOK_IDLE_Pin, GPIO_PIN_RESET);
		break;
	case TAG_TASK_INPUTS:      HAL_GPIO_WritePin(HOOK_INPUTS_GPIO_Port, HOOK_INPUTS_Pin, GPIO_PIN_RESET);break;
	case TAG_TASK_UI:          HAL_GPIO_WritePin(HOOK_UI_GPIO_Port, HOOK_UI_Pin, GPIO_PIN_RESET);break;
	case TAG_TASK_MONITOR:     HAL_GPIO_WritePin(HOOK_MONITOR_GPIO_Port, HOOK_MONITOR_Pin, GPIO_PIN_RESET);break;
	case TAG_TASK_MUESTREO:    HAL_GPIO_WritePin(HOOK5_GPIO_Port, HOOK5_Pin, GPIO_PIN_RESET);break;
	default: break;
	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
	/* USER CODE BEGIN 5 */
	vTaskSetApplicationTaskTag( NULL, (void*) TAG_TASK_MUESTREO);

	RangeState_t current_range = RANGE_10K;
	TimeBase_t current_time_base = TIME_BASE_MEDIA;
	Parametro_e parametro_anterior = config1.parametro;

	set_hardware_range(current_range);
	HAL_ADCEx_Calibration_Start(&hadc1);

	// Inicialización rápida de arranque para modo Resistencia (100 us por muestra en TIM3)
	__HAL_TIM_SET_AUTORELOAD(&htim3, TIM3_ARR_RAPIDO);
	HAL_TIM_Base_Start(&htim3);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_BUFFER_SIZE);

	/* Infinite loop */
	for(;;)
	{
		// 1. SI NO ESTAMOS EN MEDIDA: Apagamos hardware y esperamos de forma pasiva
		if (ui1.ui_estado != ESTADO_MEDIDA) {
			HAL_ADC_Stop_DMA(&hadc1);
			HAL_TIM_Base_Stop(&htim3);

			osDelay(200); // Espera pasiva que no traba al scheduler

			// Fuerza la sincronización total del hardware al regresar a medir
			parametro_anterior = (config1.parametro == PARAMETRO_R) ? PARAMETRO_C : PARAMETRO_R;
			continue;
		}

		// 2. CONTROL DE TRANSICIÓN (Solo actúa en el momento exacto del cambio R <-> C)
		if (config1.parametro != parametro_anterior)
		{
			HAL_TIM_Base_Stop(&htim3);
			HAL_ADC_Stop_DMA(&hadc1);
			__HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC | ADC_FLAG_AWD);
			ulTaskNotifyTake(pdTRUE, 0); // Flush de notificaciones viejas

			if (config1.parametro == PARAMETRO_R)
			{
				current_range = RANGE_10K;
				set_hardware_range(current_range);
				__HAL_TIM_SET_AUTORELOAD(&htim3, TIM3_ARR_RAPIDO); // 100us para R
				__HAL_TIM_SET_COUNTER(&htim3, 0);

				HAL_TIM_Base_Start(&htim3);
				HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_BUFFER_SIZE);
			}
			else if (config1.parametro == PARAMETRO_C)
			{
				current_range = RANGE_10K;
				current_time_base = TIME_BASE_MEDIA;
				set_hardware_range(current_range);
			}

			parametro_anterior = config1.parametro;
		}

		// ====================================================================
		// MODO RESISTENCIA (Lazo Continuo por Interrupciones)
		// ====================================================================
		if (config1.parametro == PARAMETRO_R)
		{
			if (hadc1.State == HAL_ADC_STATE_READY) {
				HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_BUFFER_SIZE);
				HAL_TIM_Base_Start(&htim3);
			}

			// La tarea se bloquea liberando el procesador hasta que el DMA se llene (25.6 ms)
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

			uint32_t acc = 0;
			for (uint16_t i = 0; i < ADC_BUFFER_SIZE; i++) {
				acc += adc_buffer[i];
			}

#define VCC_AL_95_PORCIENTO (62259)
#define VCC_AL_2_PORCIENTO (1310)

			uint16_t adc_16bit = acc / 16; // Decimación a 16 bits
			Bool range_changed = FALSE;

			// Evaluar autorango de Resistencia
			if (adc_16bit >= VCC_AL_95_PORCIENTO) {
				if (current_range == RANGE_330R) { current_range = RANGE_10K; range_changed = TRUE; }
				else if (current_range == RANGE_10K) { current_range = RANGE_1M; range_changed = TRUE; }
			}
			else if (adc_16bit <= VCC_AL_2_PORCIENTO) {
				if (current_range == RANGE_1M) { current_range = RANGE_10K; range_changed = TRUE; }
				else if (current_range == RANGE_10K) { current_range = RANGE_330R; range_changed = TRUE; }
			}

			if (range_changed) {
				set_hardware_range(current_range);
				osDelay(5); // Tiempo de asentamiento del transitorio eléctrico

				// Limpieza y reinicio inmediato del DMA sin saltear el flujo
				HAL_ADC_Stop_DMA(&hadc1);
				__HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC | ADC_FLAG_AWD);
				ulTaskNotifyTake(pdTRUE, 0);

				HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_BUFFER_SIZE);
			}
			else
			{
				MuestreoQueue_t muestreoQueueSample;
				muestreoQueueSample.raw = adc_16bit;
#define VCC_16_BITS (65000)

				if (current_range == RANGE_330R) {
					muestreoQueueSample.processed = (VALOR_RESISTOR_330_OHMS * adc_16bit) / (VCC_16_BITS - adc_16bit);
					muestreoQueueSample.unit = OHMS;
				} else if (current_range == RANGE_10K) {
					muestreoQueueSample.processed = (VALOR_RESISTOR_10K_OHMS * adc_16bit) / (VCC_16_BITS - adc_16bit);
					muestreoQueueSample.unit = OHMS;
				} else if (current_range == RANGE_1M) {
					muestreoQueueSample.processed = ((VALOR_RESISTOR_1M_OHMS * adc_16bit) / (VCC_16_BITS - adc_16bit)) / 1000;
					muestreoQueueSample.unit = KILO_OHMS;
				}

				osMessageQueuePut(muestreoQueueHandle, &muestreoQueueSample, 0, 0);

				// Volvemos a disparar para el próximo lote continuo
				HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_BUFFER_SIZE);
			}
		}

		// ====================================================================
		// MODO CAPACIDAD (Lazo por Ráfagas / Burst)
		// ====================================================================
		else if (config1.parametro == PARAMETRO_C)
		{
			// Apagar temporizadores de ráfagas anteriores
			HAL_TIM_Base_Stop(&htim3);
			HAL_ADC_Stop_DMA(&hadc1);
			if (hadc1.State == HAL_ADC_STATE_REG_BUSY || hadc1.State == HAL_ADC_STATE_BUSY_INTERNAL) {
				hadc1.State = HAL_ADC_STATE_READY;
			}
			__HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC | ADC_FLAG_AWD);

			// FASE DE DESCARGA
			HAL_GPIO_WritePin(GPIOB, GPIO330R_Pin | GPIO10K_Pin | GPIO1M_Pin, GPIO_PIN_RESET);

			uint16_t adc_val = 4095;
			uint32_t descarga_timeout = HAL_GetTick();

			// El bucle romperá por descarga (<30) o por timeout (max 100ms)
			while(adc_val > 30 && (HAL_GetTick() - descarga_timeout < 100)) {
				HAL_ADC_Start(&hadc1);
				if (HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK) {
					adc_val = HAL_ADC_GetValue(&hadc1);
				}
				HAL_ADC_Stop(&hadc1);
				osDelay(2); // Cede CPU a la UI mientras descarga para mantener el encoder responsivo
			}

			// CONFIGURAR NUEVA BASE DE TIEMPO
			switch(current_time_base) {
			case TIME_BASE_RAPIDA: __HAL_TIM_SET_AUTORELOAD(&htim3, TIM3_ARR_RAPIDO); break;
			case TIME_BASE_MEDIA:  __HAL_TIM_SET_AUTORELOAD(&htim3, TIM3_ARR_MEDIO); break;
			case TIME_BASE_LENTA:  __HAL_TIM_SET_AUTORELOAD(&htim3, TIM3_ARR_LENTO); break;
			}
			__HAL_TIM_SET_COUNTER(&htim3, 0);

			ulTaskNotifyTake(pdTRUE, 0); // Limpieza de notificaciones residuales

			if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_BUFFER_SIZE) != HAL_OK) {
				hadc1.State = HAL_ADC_STATE_READY;
				HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, ADC_BUFFER_SIZE);
			}

			HAL_TIM_Base_Start(&htim3); // El timer comienza a disparar el ADC

			// INICIAR CARGA ELÉCTRICA
			if (current_range == RANGE_330R) HAL_GPIO_WritePin(GPIOB, GPIO330R_Pin, GPIO_PIN_SET);
			else if (current_range == RANGE_10K) HAL_GPIO_WritePin(GPIOB, GPIO10K_Pin, GPIO_PIN_SET);
			else if (current_range == RANGE_1M) HAL_GPIO_WritePin(GPIOB, GPIO1M_Pin, GPIO_PIN_SET);

			// Espera pasiva de buffer lleno (Con timeout de respaldo de 500ms para evitar trabar el RTOS)
			uint32_t notificado = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(500));

			if (notificado == 0) {
				continue; // Si se trabó la ráfaga, reintenta de forma limpia en el próximo ciclo
			}

			// ANALIZAR DINÁMICA DE LA CURVA RC
#define UMBRAL_TAU (2589)
			int16_t indice_tau = -1;

			for (uint16_t i = 0; i < ADC_BUFFER_SIZE; i++) {
				if (adc_buffer[i] >= UMBRAL_TAU) {
					indice_tau = i;
					break;
				}
			}

			// EVALUAR AUTORANGO DE CAPACIDAD
			Bool rango_valido = TRUE;

			if (indice_tau == -1) {
				rango_valido = FALSE;
				if (current_time_base == TIME_BASE_RAPIDA) current_time_base = TIME_BASE_MEDIA;
				else if (current_time_base == TIME_BASE_MEDIA) current_time_base = TIME_BASE_LENTA;
				else current_range = RANGE_330R;
			}
			else if (indice_tau < 12) {
				rango_valido = FALSE;
				if (current_time_base == TIME_BASE_LENTA) current_time_base = TIME_BASE_MEDIA;
				else if (current_time_base == TIME_BASE_MEDIA) current_time_base = TIME_BASE_RAPIDA;
				else current_range = RANGE_1M;
			}

			if (!rango_valido) {
				set_hardware_range(current_range);
				osDelay(50); // Evitamos bucle a máxima velocidad para no saturar la cola de la UI
			}
			else
			{
				MuestreoQueue_t muestreoQueueSample;
				muestreoQueueSample.raw = adc_buffer[indice_tau];
				muestreoQueueSample.processed = calcular_capacidad_final(indice_tau, current_time_base, current_range);
				muestreoQueueSample.unit = NANO_FARADIOS;

				osMessageQueuePut(muestreoQueueHandle, &muestreoQueueSample, 0, 0);

				osDelay(200); // Pausa recomendada entre ciclos estables de capacidad
			}
		}
	}
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_oledEntry */
/**
* @brief Function implementing the oledTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_oledEntry */
void oledEntry(void *argument)
{
  /* USER CODE BEGIN oledEntry */

  /* Seteo el tag para el analizador logico */
    vTaskSetApplicationTaskTag( NULL, (void*) TAG_TASK_UI);
//	ui_init(&ui1, &data_monitor);
	ui_init(&ui1);

  /* Buffer para uiQueue */
  Evento_e evt;

  /* Buffer para monitorQueue */
  MonitorData_t data_monitor_buffer;

  TickType_t last_tick_type = xTaskGetTickCount();

  MuestreoQueue_t queueBuffer = {0};

  uint8_t samples_index = 0;

  /* Infinite loop */
  for(;;)
  {

	/* Leo todas las colas */
	/*uiQueue, cola de eventos, actualiza estado del sistema*/
	while (osMessageQueueGet(uiQueueHandle, &evt, NULL, 0) == osOK){
			ui_FSM_switch(&ui1, &config1, evt);
	}

	/* Leo monitorQueue, actualiza datos de diagnostico*/
	osMessageQueueGet(monitorQueueHandle, &data_monitor_buffer, NULL, 0);


	/* Se adquiere la última muestra*/
	samples_index = 0;
	data_monitor_buffer.muestreo_data[0].raw = 0;
	data_monitor_buffer.muestreo_data[1].raw = 0;
	data_monitor_buffer.muestreo_data[2].raw = 0;
	data_monitor_buffer.muestreo_data[3].raw = 0;
	while (osMessageQueueGet(muestreoQueueHandle, &queueBuffer, NULL, 0) == osOK){
		ui_FSM_switch(&ui1, &config1, EV_NEW_SAMPLE);
		data_monitor_buffer.muestreo_data[samples_index++] = queueBuffer;
	}


	/* Actualizo pantalla si es necesario*/
	if (ui1.ui_update_sel || ui1.ui_update_background || ui1.ui_update_datos){
		ui_update_oled(&ui1, &config1, &data_monitor_buffer);
		ui1.ui_update_sel = 0;
		ui1.ui_update_background = 0;
		ui1.ui_update_datos = 0;
	}

	/* A mimir */
     vTaskDelayUntil(&last_tick_type, 100);

  }


  /* USER CODE END oledEntry */
}

/* USER CODE BEGIN Header_encoderEntry */
/**
* @brief Function implementing the encoderTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_encoderEntry */
void encoderEntry(void *argument)
{
  /* USER CODE BEGIN encoderEntry */

  /* Parametros debouncer boton */
  #define DEBOUNCER_MAX 6
  uint8_t contador_debouncer = 0;

  /* Seteo el tag para el analizador logico */
  vTaskSetApplicationTaskTag( NULL, (void*) TAG_TASK_INPUTS);

  /* Inicializacion cosas del encoder */
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  __HAL_TIM_SET_COUNTER(&htim1, 0);

  /* Buffers para el encoder */
  volatile int16_t encoder = 0;
  volatile int16_t encoder_prev = 0;

  /* Buffer para el uiQueue */
  Evento_e evt;

  TickType_t last_tick = xTaskGetTickCount();

  /* Infinite loop */
  for(;;)
  {
	// Check encoder
	encoder = __HAL_TIM_GET_COUNTER(&htim1);

	if (encoder > encoder_prev + 2) {
		encoder_prev = encoder;
		evt = EV_DOWN;
		osMessageQueuePut(uiQueueHandle, &evt, 0, 0);
	}
	else if (encoder < encoder_prev - 2) {
		encoder_prev = encoder;
		evt = EV_UP;
		osMessageQueuePut(uiQueueHandle, &evt, 0, 0);
	}

	// Check boton TODO: mejorar el debounce, hice uno basicon. No complicarla igual xq queda feo.
	if (!HAL_GPIO_ReadPin(BOTON_ENCODER_GPIO_Port, BOTON_ENCODER_Pin)){

		contador_debouncer++;

		if (contador_debouncer > DEBOUNCER_MAX){
			contador_debouncer = 0;
			evt = EV_BOTON_ENCODER;
			osMessageQueuePut(uiQueueHandle, &evt, 0, 0);
		}
	}

	vTaskDelayUntil(&last_tick, 20);

  }
  /* USER CODE END encoderEntry */
}

/* USER CODE BEGIN Header_monitorEntry */
/**
* @brief Function implementing the monitorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_monitorEntry */
void monitorEntry(void *argument)
{
  /* USER CODE BEGIN monitorEntry */
  /* Seteo el tag para el analizador logico */
  vTaskSetApplicationTaskTag( NULL, (void*) TAG_TASK_MONITOR);

  /* Cosas para advertir cuando se esta llenando mucho el heap.
   * La advertencia se pasa a uiTask mediante la cola de eventos (uiQueue)
   */
  #define LIMITE_ADVERTENCIA 30
  Evento_e evt = EV_HEAP_ADVERTENCIA;

  /* Buffers para cargar datos de monitoreo */
  MonitorData_t data_monitor_buffer;
  uint32_t heap_free = xPortGetFreeHeapSize();
  uint32_t heap_min = xPortGetMinimumEverFreeHeapSize();

  uint32_t FU_average_timer = HAL_GetTick();

//  uint8_t should_update_ui = 0;

  TickType_t last_tick = xTaskGetTickCount();

  /* Infinite loop */
  for(;;)
  {
	 /* Recopilacion y carga de datos del sistema en data_monitor */

//const osThreadAttr_t defaultTask_attributes = {
//  .name = "defaultTask",
//  .stack_size = 128 * 4,
//  .priority = (osPriority_t) osPriorityNormal,
//};

	 /* Check de si estoy pasando los limites */
	 heap_free = xPortGetFreeHeapSize();
	 if ( heap_free < LIMITE_ADVERTENCIA){
			osMessageQueuePut(uiQueueHandle, &evt, 0, 0); /* Si estamos complicados de heap, envio evento de alerta al ui*/
	 }

	 heap_min = xPortGetMinimumEverFreeHeapSize();

	 data_monitor_buffer.system_data.sys_heap_free = heap_free;
	 data_monitor_buffer.system_data.sys_heap_min = heap_min;

	 /* Recopilacion de datos de tareas*/
	 uint32_t free_stack = osThreadGetStackSpace(uiTaskHandle);
//	 if (free_stack != data_monitor_buffer.tasks_data.tasks[uiTaskID].task_stack_free) should_update_ui = 1;
	 data_monitor_buffer.tasks_data.tasks[uiTaskID].task_stack_free = free_stack;

	// Estamos usando la defaultTask como la task de idle
	 free_stack = osThreadGetStackSpace(defaultTaskHandle);
//	 if (free_stack != data_monitor_buffer.tasks_data.tasks[idleTaskID].task_stack_free) should_update_ui = 1;
	 data_monitor_buffer.tasks_data.tasks[idleTaskID].task_stack_free = free_stack;

	 free_stack = osThreadGetStackSpace(inputsTaskHandle);
//	 if (free_stack != data_monitor_buffer.tasks_data.tasks[inputsTaskID].task_stack_free) should_update_ui = 1;
	 data_monitor_buffer.tasks_data.tasks[inputsTaskID].task_stack_free = free_stack;

	 free_stack = osThreadGetStackSpace(monitorTaskHandle);
//	 if (data_monitor_buffer.tasks_data.tasks[monitorTaskID].task_stack_free) should_update_ui = 1;
	 data_monitor_buffer.tasks_data.tasks[monitorTaskID].task_stack_free = free_stack;

//	 // TODO: esto va si se quiere actualizar en tiempo real el UI, pero se ve feo
//		 if (	should_update_ui &&
//				 (ui1.ui_estado == ESTADO_DIAG ||
//				 ui1.ui_estado == ESTADO_DIAG_TAREAS_UI ||
//				 ui1.ui_estado == ESTADO_DIAG_TAREAS_MONITOR ||
//				 ui1.ui_estado == ESTADO_DIAG_TAREAS_INPUTS ||
//				 ui1.ui_estado == ESTADO_DIAG_TAREAS_IDLE) )
//			 ui1.ui_update = 1;

#define PERIODO_FU_MS (1000) // TODO: revisar
	 if (HAL_GetTick() - FU_average_timer >= PERIODO_FU_MS) {
		 FU_average_timer = HAL_GetTick();
		 HAL_GPIO_TogglePin(DEBUG_PIN_GPIO_Port, DEBUG_PIN_Pin);

		 // se calcula el FU con un periodod de PERIODO_FU_MS en porcentaje
		 uint32_t fu = ( FU_acc * 100 ) / PERIODO_FU_MS;
		 data_monitor_buffer.system_data.fu = 100 - fu;
		 FU_acc = 0;
		 if (ui1.ui_estado == ESTADO_DIAG )
			 ui1.ui_update_datos = 1;
	 }

	 /* Envio el paquete a ui a traves de monitorQueue */
     osMessageQueuePut(monitorQueueHandle, &data_monitor_buffer, 0, 0);


     vTaskDelayUntil(&last_tick, 50);
//     osDelay(100); // a mimir
  }
  /* USER CODE END monitorEntry */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
