/*
 * medida.c
 *
 *  Created on: 10 jul 2026
 *      Author: Ignacio Chantiri y Tomás Vidal
 */

#include "medida.h"
#include "ui.h"

extern ADC_HandleTypeDef hadc1;
volatile uint32_t contador_capacitor=0;
volatile uint32_t ultima_muestra = 0;

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

uint32_t ADC_muestrear(uint32_t cantidad_muestras) {

	//Realiza una lectura de ADC_N_MUESTRAS muestras y devuelve el promedio
	uint32_t acc = 0;

	for (uint32_t i=0; i<cantidad_muestras; i++) {

		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 1000);
		acc += HAL_ADC_GetValue(&hadc1);

	}

	//Convierto a mV, calculo promedio y retorno
	return (acc * 3300) / (4095 * cantidad_muestras);
}

FSM_State FSM_General(FSM_State state, Unidad_t *unit, Modo_e mode, uint16_t *medida) {
	switch (state) {

	case FSM_C_DESCARGA:

		int i = ADC_muestrear(2);
		if (i >= VCC_AL_2_PORCIENTO) {

			contador_capacitor++;

			if (contador_capacitor >= MAX_CUENTAS_DESCARGA) {
				//				c_medida = FALLO_MAX_TIEMPO_DESCARGA;
				*medida = FALLO_MAX_TIEMPO_DESCARGA;
				//				UART_mostrar_menu(MENU_C);
				return FSM_MOSTRAR_C;
			}

			return FSM_C_DESCARGA;
		}

		// TODO: y acá?
		// ya descarga
		//			configurar_carga();
		//			return FSM_C_CARGA;

		//		configurar_descarga();
		//		return FSM_C_DESCARGA;

		break;
	case FSM_C_CARGA:
		contador_capacitor++;

		if ((ADC_muestrear(1) >= VCC_AL_63_PORCIENTO) && contador_capacitor < MAX_CUENTAS_CARGA) {

			*unit = NANO_FARADIOS;
			//				c_medida = CALCULAR_CAPACIDAD(contador_capacitor); // Solucion a: qué pasa si 1M no es 1M exactamente
			//			c_medida = contador_capacitor;
			*medida = contador_capacitor / 5; // porque las muestras se hacen cada 5ms ahora
			//			UART_mostrar_menu(MENU_C);
			HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_RESET);
			return FSM_MOSTRAR_C;

		} else if(contador_capacitor <= MAX_CUENTAS_CARGA){
			return FSM_C_CARGA;

		} else {
			contador_capacitor = 0;
			//			c_medida = FALLO_MAX_TIEMPO_CARGA;
			*medida = FALLO_MAX_TIEMPO_CARGA;
			//			return FUERA_DE_ESCALA;

			//			UART_mostrar_menu(MENU_C);
			return FSM_MOSTRAR_C;

		}

		break;
	case FSM_MOSTRAR_C:

		if (mode == MODO_MULTIPLE) {
			configurar_descarga();
			return FSM_C_DESCARGA;
		}

		break;
	case FSM_R330:
		ultima_muestra = ADC_muestrear(32);
		if (ultima_muestra > VCC_AL_95_PORCIENTO) {
			set_resistencia(RESISTOR_10K);
			return FSM_R10K;
		}

		//		r_medida = (VALOR_RESISTOR_330_OHMS * ultima_muestra) / (VCC_MV - ultima_muestra);
		*medida = (VALOR_RESISTOR_330_OHMS * ultima_muestra) / (VCC_MV - ultima_muestra);
		*unit = OHMS;
		return FSM_MOSTRAR_R;

	case FSM_R10K:
		ultima_muestra = ADC_muestrear(32);
		if (ultima_muestra > VCC_AL_95_PORCIENTO) {
			set_resistencia(RESISTOR_1M);
			return FSM_R1M;
		}

		//		r_medida = (VALOR_RESISTOR_10K_OHMS * ultima_muestra) / (VCC_MV - ultima_muestra);
		*medida = (VALOR_RESISTOR_10K_OHMS * ultima_muestra) / (VCC_MV - ultima_muestra);
		*unit = OHMS;

		return FSM_MOSTRAR_R;

	case FSM_R1M:
		ultima_muestra = ADC_muestrear(32);
		if (ultima_muestra < VCC_AL_95_PORCIENTO) {
			*medida = ( (VALOR_RESISTOR_1M_OHMS * ultima_muestra) / (VCC_MV - ultima_muestra) ) / 1000;
			//			r_medida = ( (VALOR_RESISTOR_1M_OHMS * ultima_muestra) / (VCC_MV - ultima_muestra) ) / 1000;
			*unit = KILO_OHMS;
		} else {
			//			r_medida = FALLO_R_FUERA_DE_ESCALA;
			*medida = FALLO_R_FUERA_DE_ESCALA;
			*unit = MEGA_OHMS;
			//			return FUERA_DE_ESCALA;
		}
		return FSM_MOSTRAR_R;

	case FSM_MOSTRAR_R:
		return FSM_MOSTRAR_R;

	}
	return state;
}

void configurar_descarga() {
	contador_capacitor = 0;
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

void configurar_carga() {
	contador_capacitor = 0;

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
//	__HAL_TIM_SET_COUNTER(&htim1, 0);
//	HAL_TIM_Base_Start_IT(&htim1);
//	contador_timer_1ms = 0;
	// 1M en alto
	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_SET);

//	c_medida = contador_de_muestras * (1000 / VALOR_RESISTOR_1M_KOHMS);

//	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_RESET);
}
