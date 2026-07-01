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
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
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
const osMessageQueueAttr_t uiQueue_attributes = {
  .name = "uiQueue"
};
/* Definitions for monitorQueue */
osMessageQueueId_t monitorQueueHandle;
const osMessageQueueAttr_t monitorQueue_attributes = {
  .name = "monitorQueue"
};
/* USER CODE BEGIN PV */
volatile int16_t encoder = 0;
volatile int16_t encoder_prev = 0;

volatile uint32_t FU_acc = 0;
volatile uint32_t FU_time_delta = 0;

UI_t ui1 = {
	ESTADO_INICIO,
	1,
	1
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
void StartDefaultTask(void *argument);
void oledEntry(void *argument);
void encoderEntry(void *argument);
void monitorEntry(void *argument);

/* USER CODE BEGIN PFP */
void UI_FSM_Switch(UI_t *ui, Evento_e evento);
void callback_in(int tag);
void callback_out(int tag);

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
  MX_I2C1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

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
  HAL_GPIO_WritePin(GPIOB, HOOK_UI_Pin|HOOK_INPUTS_Pin|HOOK5_Pin, GPIO_PIN_RESET);

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

  /*Configure GPIO pins : HOOK_UI_Pin HOOK_INPUTS_Pin HOOK5_Pin */
  GPIO_InitStruct.Pin = HOOK_UI_Pin|HOOK_INPUTS_Pin|HOOK5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
//	case TAG_TASK_MUESTREO: HAL_GPIO_WritePin(HOOK_MONITOR_GPIO_Port, HOOK5_Pin, GPIO_PIN_SET); break;
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
//	case TAG_TASK_MUESTREO:    HAL_GPIO_WritePin(HOOK5_GPIO_Port, HOOK5_Pin, GPIO_PIN_RESET);break;
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
  /* Infinite loop */
  for(;;)
  {
	  osDelay(1);
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

  /* Infinite loop */
  for(;;)
  {

	/* Leo todas las colas */
	/*uiQueue, cola de eventos, actualiza estado del sistema*/
	while (osMessageQueueGet(uiQueueHandle, &evt, NULL, 0) == osOK){

			ui_FSM_switch(&ui1, evt);

	}

	/* Leo monitorQueue, actualiza datos de diagnostico*/
	osMessageQueueGet(monitorQueueHandle, &data_monitor_buffer, NULL, 0);

	/* Actualizo pantalla si es necesario*/
	if (ui1.ui_update_sel || ui1.ui_update_background || ui1.ui_update_datos){

		ui_update_oled(&ui1, &data_monitor_buffer);
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

  /* Buffer para el uiQueue */
  Evento_e evt;

  TickType_t last_tick_type = xTaskGetTickCount();

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

	vTaskDelayUntil(&last_tick_type, 20);
//	osDelay(20);

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

  TickType_t last_tick_type = xTaskGetTickCount();

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


     vTaskDelayUntil(&last_tick_type, 50);
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
