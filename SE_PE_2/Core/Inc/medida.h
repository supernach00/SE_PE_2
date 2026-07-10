/*
 * medida.h
 *
 *  Created on: 10 jul 2026
 *      Author: Ignacio Chantiri y Tomás Vidal
 */



#ifndef INC_MEDIDA_H_
#define INC_MEDIDA_H_

#include "main.h"
#include "ui.h"

//Macros
#define CALCULAR_CAPACIDAD(cuentas) ((cuentas * 1000) / VALOR_RESISTOR_1M_KOHMS)

#define DEFAULT_MODO UNICO
#define DEFAULT_PARAMETRO RESISTENCIA
#define DEFUALT_COMANDO OPCION_1

#define VCC_MV (3270)
#define VALOR_RESISTOR_330_OHMS (331)
#define VALOR_RESISTOR_10K_OHMS (9920)
#define VALOR_RESISTOR_1M_KOHMS (999) // TODO: como manejamos esta proporcion?
#define VALOR_RESISTOR_1M_OHMS (VALOR_RESISTOR_1M_KOHMS * 1000) // TODO: como manejamos esta proporcion?

#define VCC_AL_95_PORCIENTO (0.95*VCC_MV)
#define VCC_AL_63_PORCIENTO (0.63*VCC_MV)
#define VCC_AL_2_PORCIENTO (0.02*VCC_MV)

#define DEBOUNCER_BTN_TIMEOUT_MS (200)

#define MAX_CUENTAS_DESCARGA (10 * 1000 * 1000) // 10 segundos?
#define MAX_CUENTAS_CARGA (10 * 1000) // 10 segundos?

#define FALLO_MAX_TIEMPO_CARGA (-1)
#define FALLO_MAX_TIEMPO_DESCARGA (-2)
#define FALLO_R_FUERA_DE_ESCALA (-3)

typedef enum {
	OHMS,
	KILO_OHMS,
	MEGA_OHMS,
	MICRO_FARADIOS,
	NANO_FARADIOS,
	PICO_FARADIOS
} Unidad_t;

typedef enum {
	FSM_C_CARGA,
	FSM_C_DESCARGA,
	FSM_MOSTRAR_C,
	FSM_MOSTRAR_R,
	FSM_R330,
	FSM_R10K,
	FSM_R1M,
	//FUERA_DE_ESCALA,
} FSM_State;

typedef enum {
	RESISTOR_330,
	RESISTOR_10K,
	RESISTOR_1M,
} OutputResistor_Type;

FSM_State FSM_General(FSM_State state, Unidad_t *unit, Modo_e mode, uint16_t *medida);
uint32_t ADC_muestrear(uint32_t cantidad_muestras);
int procesar_comando(void);
void set_resistencia(OutputResistor_Type resistorType);

void configurar_carga();
void configurar_descarga();
void configurar_resistencia330();


#endif /* INC_MEDIDA_H_ */
