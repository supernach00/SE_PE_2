/*
 * medida.h
 *
 *  Created on: 10 jul 2026
 *      Author: Chantiri Ignacio y Tomás Vidal
 */

#ifndef INC_MEDIDA_H_
#define INC_MEDIDA_H_

#include "main.h"

typedef enum {
	OHMS,
	KILO_OHMS,
	MEGA_OHMS,
	MICRO_FARADIOS,
	NANO_FARADIOS,
	PICO_FARADIOS
} Unidad_t;

typedef enum {
	RESISTOR_330,
	RESISTOR_10K,
	RESISTOR_1M,
} OutputResistor_Type;

#define VCC_MV (3270)
#define VALOR_RESISTOR_330_OHMS (331)
#define VALOR_RESISTOR_10K_OHMS (9920)
#define VALOR_RESISTOR_1M_KOHMS (999) // TODO: como manejamos esta proporcion?
#define VALOR_RESISTOR_1M_OHMS (VALOR_RESISTOR_1M_KOHMS * 1000) // TODO: como manejamos esta proporcion?

#define VCC_AL_95_PORCIENTO (0.95*VCC_MV)
#define VCC_AL_63_PORCIENTO (0.63*VCC_MV)
#define VCC_AL_2_PORCIENTO (0.02*VCC_MV)

#define MAX_CUENTAS_DESCARGA (10 * 1000 * 1000) // 10 segundos?
#define MAX_CUENTAS_CARGA (10 * 1000) // 10 segundos?

#define FALLO_MAX_TIEMPO_CARGA (-1)
#define FALLO_MAX_TIEMPO_DESCARGA (-2)
#define FALLO_R_FUERA_DE_ESCALA (-3)

void set_resistencia(OutputResistor_Type resistorType);

void configurar_carga();
void configurar_descarga();

#endif /* INC_MEDIDA_H_ */
