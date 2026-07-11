/*
 * medida.c
 *
 *  Created on: 10 jul 2026
 *      Author: Chantiri Ignacio y Tomás Vidal
 */

#include "medida.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;

void configurar_carga() {

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Bajo los 3 pines
	HAL_GPIO_WritePin(GPIO330R_GPIO_Port, GPIO330R_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO10K_GPIO_Port, GPIO10K_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_RESET);

	//1M como salida en bajo (no pull up ni pull down)
	GPIO_InitStruct.Pin = GPIO1M_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIO1M_GPIO_Port, &GPIO_InitStruct);

	// El resto (10k y 330ohm) en alta impedancia
	GPIO_InitStruct.Pin = GPIO330R_Pin|GPIO10K_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//Reinicio timer
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	HAL_TIM_Base_Start_IT(&htim1);
	// 1M en alto
	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_SET);

//	c_medida = contador_de_muestras * (1000 / VALOR_RESISTOR_1M_KOHMS);

//	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_RESET);
}

void configurar_descarga() {
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// TODO: preguntar orden de WritePin
	HAL_GPIO_WritePin(GPIO330R_GPIO_Port, GPIO330R_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO10K_GPIO_Port, GPIO10K_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_RESET);

	GPIO_InitStruct.Pin = GPIO330R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO1M_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void set_resistencia(OutputResistor_Type resistorType){


	/*
	Toma como input el define del pin, y lo setea en alto
	mientras que setea los otros dos pines en alta Z.
	*/

	  GPIO_InitTypeDef GPIO_InitStruct = {0}; //Esto habria que ver si hace falta llamarlo siempre, capaz
	  	  	  	  	  	  	  	  	  	  	 	 //podemos evitarnos tambien inicializar gpio initstruct cada vez

	  switch (resistorType){

	  case RESISTOR_330:
          //330r como salida en alto
		  GPIO_InitStruct.Pin = GPIO330R_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);
		  HAL_GPIO_WritePin(GPIO330R_GPIO_Port, GPIO330R_Pin, GPIO_PIN_SET);

		  GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO1M_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  // Hay que tener cuidado que justo GPIO10K y GPIO1M
		  // son del puerto GPIOA, pero si no fuera así
		  // hay que inicializarlos separados
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		  break;

	  case RESISTOR_10K:
		  // GPIO10K en alto
		  GPIO_InitStruct.Pin = GPIO10K_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIO10K_GPIO_Port, &GPIO_InitStruct);
		  HAL_GPIO_WritePin(GPIO10K_GPIO_Port, GPIO10K_Pin, GPIO_PIN_SET); //High

		  // Y las demas en Z
		  GPIO_InitStruct.Pin = GPIO330R_Pin|GPIO1M_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  // Hay que tener cuidado que justo GPIO10K y GPIO1M
		  // son del puerto GPIOA, pero si no fuera así
		  // hay que inicializarlos separados
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		  break;

	  case RESISTOR_1M:
		  //GPIO1M en alto
		  GPIO_InitStruct.Pin = GPIO1M_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIO1M_GPIO_Port, &GPIO_InitStruct);
		  HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_SET); //High

		  // Y las demas en Z
		  GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO330R_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		  break;

	  default:
		  // TODO habría que tener un fallback para el error
		  break;

	  }
}
