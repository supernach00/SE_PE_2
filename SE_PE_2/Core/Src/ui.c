#include <string.h>
#include "ui.h"
#include "graphics.h"
#include "ssd1306.h"

void ui_init(UI_t *ui, MonitorData_t *data_monitor){
	SSD1306_Init();
	SSD1306_Clear();
	ui_update_oled(ui, data_monitor);
}

//void ui_update_data(char *data_buffer, MonitorData_t *data_monitor){
//
//    strncpy(data_monitor->data_system, data_buffer, sizeof(data_monitor->data_system));
//
//}

void ui_update_oled(UI_t *ui, MonitorData_t *data_monitor){

		SSD1306_Clear();

		switch (ui->ui_estado){
		case ESTADO_INICIO:
			switch(ui->ui_seleccion){
			case SEL_MEDIDA:
				SSD1306_DrawBitmap(0, 0, inicio_sel_medida_bmp, 128, 64, 1);
				break;
			case SEL_CONFIG:
				SSD1306_DrawBitmap(0, 0, inicio_sel_config_bmp, 128, 64, 1);
				break;
			case SEL_DIAG:
				SSD1306_DrawBitmap(0, 0, inicio_sel_diagnostico_bmp, 128, 64, 1);
				break;
			default:
				break;
			}
		break;

		case ESTADO_DIAG:
			SSD1306_DrawBitmap(0, 0, diag_sistema_bmp, 128, 64, 1);
			break;

		case ESTADO_MEDIDA:
			SSD1306_DrawBitmap(0, 0, pepe_bmp, 128, 64, 1);
			break;

		case ESTADO_CONFIG:
			SSD1306_DrawBitmap(0, 0, pepe_bmp, 128, 64, 1);
			break;

		default:
			break;
		}

		SSD1306_UpdateScreen(); // update screen

	}

//    SSD1306_DrawBitmap(18, 7, medida_bmp, 42, 11, 1);
//    SSD1306_DrawBitmap(5, 25, config_bmp, 40, 11, 1);
//    SSD1306_DrawBitmap(5, 44, diag_bmp, 37, 11, 1);

//    SSD1306_DrawBitmap(0, 0, grid_bmp, 128, 64, 1);


void ui_FSM_switch(UI_t *ui, Evento_e evento){

	switch(ui->ui_estado){
	case ESTADO_INICIO:

		if (evento == EV_UP) {
			ui_up(ui, 2);
			ui->ui_update = 1;
			break;
		}

		if (evento == EV_DOWN) {
			ui_down(ui);
			ui->ui_update = 1;
			break;
		}

		if (evento == EV_BOTON_ENCODER){
			switch (ui->ui_seleccion){
			case SEL_CONFIG:
				ui->ui_estado = ESTADO_CONFIG;
				ui->ui_update = 1;
				break;
			case SEL_MEDIDA:
				ui->ui_estado = ESTADO_MEDIDA;
				ui->ui_update = 1;
				break;
			case SEL_DIAG:
				ui->ui_estado = ESTADO_DIAG;
				ui->ui_update = 1;
				break;
			default:
				break;

		}

		}

		break;

	case ESTADO_DIAG:
		if (evento == EV_BOTON_ENCODER){
			ui->ui_estado = ESTADO_INICIO;
			ui->ui_update = 1;
		}
	break;

	case ESTADO_CONFIG:
		if (evento == EV_BOTON_ENCODER){
			ui->ui_estado = ESTADO_INICIO;
			ui->ui_update = 1;
		}
	break;

	case ESTADO_MEDIDA:
		if (evento == EV_BOTON_ENCODER){
			ui->ui_estado = ESTADO_INICIO;
			ui->ui_update = 1;
		}
	break;

	default:
		break;
	}



}
