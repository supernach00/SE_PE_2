#include "ui.h"

void UI_FSM_Switch(UI_t *ui, Evento_t evento){

	switch(ui->ui_estado){
	case ESTADO_INICIO:

		if (evento == EV_UP) {
			ui_up(ui, 2);
			break;
		}

		if (evento == EV_DOWN) {
			ui_down(ui);
			break;
		}

		if (evento == EV_BOTON_ENCODER){
			switch (ui->ui_seleccion){
			case 0:
				ui->ui_estado = ESTADO_CONFIG;
				break;
			case 1:
				ui->ui_estado = ESTADO_MEDIDA;
				break;
			case 2:
				ui->ui_estado = ESTADO_DIAG;
				break;

		}

		}

		break;

	default:
		break;
	}



}
