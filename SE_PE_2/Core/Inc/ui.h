#ifndef UI_H
#define UI_H

#include <stdint.h>

typedef enum {

		ESTADO_INICIO,
		ESTADO_CONFIG,
		ESTADO_DIAG,
		ESTADO_MEDIDA

}Estado_t;

typedef enum {

		EV_UP,
		EV_DOWN,
		EV_BOTON,
		EV_BOTON_ENCODER

}Evento_t;

typedef enum {

	SEL_MEDIDA,
	SEL_CONFIG,
	SEL_DIAG

}Seleccion_t;

typedef struct {

		Estado_t ui_estado;
		Seleccion_t ui_seleccion;
		uint8_t ui_actualizar_oled;

}UI_t;


void UI_FSM_Switch(UI_t *ui, Evento_t evento);

static inline void ui_up(UI_t *ui, int MAX)
{
    if (ui->ui_seleccion < MAX) {
        ui->ui_seleccion++;
        ui->ui_actualizar_oled = 1;
    }
}

static inline void ui_down(UI_t *ui)
{
    if (ui->ui_seleccion > 0) {
        ui->ui_seleccion--;
        ui->ui_actualizar_oled = 1;
    }
}


#endif
