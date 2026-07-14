#ifndef UI_H
#define UI_H

#include <stdint.h>
#include "monitor.h"

/* Estados y eventos */
typedef enum {

	ESTADO_INICIO,

	ESTADO_DIAG,
	ESTADO_DIAG_TAREAS_UI,
	ESTADO_DIAG_TAREAS_MONITOR,
	ESTADO_DIAG_TAREAS_INPUTS,
	ESTADO_DIAG_TAREAS_IDLE,

	ESTADO_MEDIDA,

	ESTADO_CONFIG,
	ESTADO_CONFIG_MODO,
	ESTADO_CONFIG_PARAMETRO

}Estado_e;

typedef enum {

	EV_UP,
	EV_DOWN,
	EV_BOTON,
	EV_BOTON_ENCODER,
	EV_HEAP_ADVERTENCIA,
	EV_NEW_SAMPLE,

}Evento_e;

typedef enum {

	SEL_MEDIDA,
	SEL_DIAG,
	SEL_CONFIG

}Seleccion_t;

typedef struct {

	Estado_e ui_estado;
	Seleccion_t ui_seleccion;
	uint8_t ui_update_background;
	uint8_t ui_update_sel;
	uint8_t ui_update_datos;

}UI_t;

/* Configuracion */
typedef enum {

	MODO_SINGLE,
	MODO_MULTIPLE,

}Modo_e;

typedef enum {

	PARAMETRO_R,
	PARAMETRO_C

}Parametro_e;

typedef struct {

	Modo_e modo;
	Parametro_e parametro;

}Config_t;

void ui_FSM_switch(UI_t *ui, Config_t *config, Evento_e evento);
void ui_update_oled(UI_t *ui, Config_t *config, MonitorData_t *data_monitor);
void ui_update_data(char *data_buffer, MonitorData_t *data_monitor);
void ui_init(UI_t *ui);

/* Funciones inline */
static inline void ui_up(UI_t *ui, int MAX)
{
    if (ui->ui_seleccion < MAX) {
        ui->ui_seleccion++;

    }
}

static inline void ui_down(UI_t *ui)
{
    if (ui->ui_seleccion > 0) {
        ui->ui_seleccion--;
    }
}

#endif
