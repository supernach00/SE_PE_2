#include <string.h>
#include "ui.h"
#include "graphics.h"
#include "ssd1306.h"
#include <stdio.h>
#include "fonts.h"

//extern Unidad_t unit;
uint8_t sample_index = 0;

//void ui_init(UI_t *ui, MonitorData_t *data_monitor){
void ui_init(UI_t *ui){

	SSD1306_Init();
	SSD1306_Clear();

	/* Comienzo en el menu princpial */
	SSD1306_DrawBitmap(0, 0, inicio_background_bmp, 128, 64, 1); // IMPORTANTE: Si se cambia el orden de las opciones, agregar el if a la primera.
//	SSD1306_DrawBitmap(5, 8, flecha_bmp, 14, 13, 1);
//	ui_update_oled(ui, data_monitor);
}

//void ui_update_data(char *data_buffer, MonitorData_t *data_monitor){
//
//    strncpy(data_monitor->data_system, data_buffer, sizeof(data_monitor->data_system));
//
//}

void ui_update_oled(UI_t *ui, Config_t *config, MonitorData_t *data_monitor, uint16_t samples[4]){

		char val[15];

		switch (ui->ui_estado){
		case ESTADO_INICIO:

			if (ui->ui_update_background){
				SSD1306_Clear();
				SSD1306_DrawBitmap(0, 0, inicio_background_bmp, 128, 64, 1);
			}

			if (ui->ui_update_sel){

				switch(ui->ui_seleccion){
				case SEL_MEDIDA:
					SSD1306_DrawFilledRectangle(4, 8, 14, 51, 0); // Cuadrado negro para limpiar las flechas viejas
					SSD1306_DrawBitmap(5, 8, flecha_bmp, 12, 11, 1); // Flecha
					break;

				case SEL_DIAG:
					SSD1306_DrawFilledRectangle(4, 8, 14, 51, 0); // Cuadrado negro para limpiar las flechas viejas
					SSD1306_DrawBitmap(5, 26, flecha_bmp, 12, 11, 1);
					break;

				case SEL_CONFIG:
					SSD1306_DrawFilledRectangle(4, 8, 14, 51, 0); // Cuadrado negro para limpiar las flechas viejas
					SSD1306_DrawBitmap(5, 46, flecha_bmp, 12, 11, 1);
					break;

				default:
					break;
				}

			}
		break;

		case ESTADO_DIAG:

			if (ui->ui_update_background){
				SSD1306_Clear();

				SSD1306_DrawBitmap(0, 0, diag_sistema_bmp, 128, 64, 1);

				SSD1306_GotoXY(50, 22);
				snprintf(val, sizeof(val), "%ld/%ld", data_monitor->system_data.sys_heap_min, data_monitor->system_data.sys_heap_free);
				SSD1306_Puts(val, &Font_7x10, SSD1306_COLOR_WHITE);

				SSD1306_GotoXY(50, 32);
				snprintf(val, sizeof(val), "%d%%", data_monitor->system_data.fu);
				SSD1306_Puts(val, &Font_7x10, SSD1306_COLOR_WHITE);
			}
			if (ui->ui_update_datos){
				SSD1306_DrawFilledRectangle(49, 31, 40, 10, 0); // Cuadrado negro para limpiarlos datos viejos.

				SSD1306_GotoXY(50, 32);
				snprintf(val, sizeof(val), "%d%%", data_monitor->system_data.fu);
				SSD1306_Puts(val, &Font_7x10, SSD1306_COLOR_WHITE);

			}

			break;

		case ESTADO_DIAG_TAREAS_UI:

			if (ui->ui_update_background){
				SSD1306_Clear();
				SSD1306_DrawBitmap(0, 0, diag_background_ui_bmp, 128, 64, 1);

				SSD1306_GotoXY(50, 22);
				snprintf(val, sizeof(val), "%ld", data_monitor->tasks_data.tasks[uiTaskID].task_stack_free);
				SSD1306_Puts(val, &Font_7x10, SSD1306_COLOR_WHITE);
			}
			break;

		case ESTADO_DIAG_TAREAS_MONITOR:
			if (ui->ui_update_background){
				SSD1306_Clear();
				SSD1306_DrawBitmap(0, 0, diag_background_monitor_bmp, 128, 64, 1);

				SSD1306_GotoXY(50, 22);

				snprintf(val, sizeof(val), "%ld", data_monitor->tasks_data.tasks[monitorTaskID].task_stack_free);
				SSD1306_Puts(val, &Font_7x10, SSD1306_COLOR_WHITE);
			}
			break;

		case ESTADO_DIAG_TAREAS_INPUTS:
			if (ui->ui_update_background){
				SSD1306_Clear();
				SSD1306_DrawBitmap(0, 0, diag_background_inputs_bmp, 128, 64, 1);

				SSD1306_GotoXY(50, 22);
				snprintf(val, sizeof(val), "%ld", data_monitor->tasks_data.tasks[inputsTaskID].task_stack_free);
				SSD1306_Puts(val, &Font_7x10, SSD1306_COLOR_WHITE);
			}
			break;

		case ESTADO_DIAG_TAREAS_IDLE:
			if (ui->ui_update_background){
				SSD1306_Clear();
				SSD1306_DrawBitmap(0, 0, diag_background_idle_bmp, 128, 64, 1);

				SSD1306_GotoXY(50, 22);
				snprintf(val, sizeof(val), "%ld", data_monitor->tasks_data.tasks[idleTaskID].task_stack_free);
				SSD1306_Puts(val, &Font_7x10, SSD1306_COLOR_WHITE);
			}
			if (ui->ui_update_datos){

			}
			break;

	    /* Estados medida --------------------------- */

		case ESTADO_MEDIDA:
			if (ui->ui_update_datos){
				// TODO: aca actuaLizar datos
				uint16_t converted_sample;
				if (config->parametro == PARAMETRO_R) {
					// TODO: habría que hacer algo así no?
//					uint32_t fondo_escala;
//					if (unit == OHMS) {
//						fondo_escala = 10000;
//					}
					for (uint8_t i = 0; i < 4; i++) {
						converted_sample = SSD1306_HEIGHT  - samples[i] * SSD1306_HEIGHT / 4095 + 15;
						SSD1306_DrawPixel(sample_index, converted_sample, SSD1306_COLOR_WHITE);
					}
					sample_index++;

				} else if (config->parametro == PARAMETRO_C) {
					converted_sample = 0;
				}
				if (sample_index > SSD1306_WIDTH) {
					sample_index = 0;
					ui->ui_update_background = 1;
				}
			}

			if (ui->ui_update_background){
				SSD1306_Clear();

				// QUÉ ES MODO SINGLE Y MULTIPLE??
				// NO ES COMO EL TP1, no hay single y continuo
				switch(config->modo){

				case MODO_SINGLE:
					if (config->parametro == PARAMETRO_R) SSD1306_DrawBitmap(0, 0, medida_background_S_R_bmp, 128, 64, 1);
					else if(config->parametro == PARAMETRO_C) SSD1306_DrawBitmap(0, 0, medida_background_S_C_bmp, 128, 64, 1);

					break;

				case MODO_MULTIPLE:
					if (config->parametro == PARAMETRO_R) SSD1306_DrawBitmap(0, 0, medida_background_M_R_bmp, 128, 64, 1);
					else if(config->parametro == PARAMETRO_C) SSD1306_DrawBitmap(0, 0, medida_background_M_C_bmp, 128, 64, 1);

					break;

				}
			}

			break;

		/* Estados configuracion  ------------------- */

		case ESTADO_CONFIG:

			if (ui->ui_update_background){
				SSD1306_Clear();
				SSD1306_DrawBitmap(0, 0, config_background_bmp, 128, 64, 1);
			}

			if (ui->ui_update_sel){

				switch(ui->ui_seleccion){
				case 0:

					SSD1306_DrawFilledRectangle(4, 8, 14, 51, 0); // Cuadrado negro para limpiar las flechas viejas
					SSD1306_DrawBitmap(5, 23, flecha_bmp, 12, 11, 1); // Flecha
					break;

				case 1:
					if (ui->ui_update_sel) {
						SSD1306_DrawFilledRectangle(4, 8, 14, 51, 0); // Cuadrado negro para limpiar las flechas viejas
						SSD1306_DrawBitmap(5, 41, flecha_bmp, 12, 11, 1);
					}
					break;

				default: break;
				}
			}

		break;

		case ESTADO_CONFIG_MODO:

			if (ui->ui_update_background){
				SSD1306_Clear();
				SSD1306_DrawBitmap(0, 0, config_background_modo_bmp, 128, 64, 1);

			}

			if (ui->ui_update_sel){
				switch(ui->ui_seleccion){
				case 0:

					SSD1306_DrawFilledRectangle(4, 8, 14, 51, 0); // Cuadrado negro para limpiar las flechas viejas
					SSD1306_DrawBitmap(5, 23, flecha_bmp, 12, 11, 1); // Flecha
					break;

				case 1:
					if (ui->ui_update_sel) {
						SSD1306_DrawFilledRectangle(4, 8, 14, 51, 0); // Cuadrado negro para limpiar las flechas viejas
						SSD1306_DrawBitmap(5, 41, flecha_bmp, 12, 11, 1);
					}
					break;

				default: break;
				}

			}

			break;

		case ESTADO_CONFIG_PARAMETRO:

			if (ui->ui_update_background){
				SSD1306_Clear();
				SSD1306_DrawBitmap(0, 0, config_background_parametro_bmp, 128, 64, 1);

			}

			if (ui->ui_update_sel){
				switch(ui->ui_seleccion){
				case 0:

					SSD1306_DrawFilledRectangle(4, 8, 14, 51, 0); // Cuadrado negro para limpiar las flechas viejas
					SSD1306_DrawBitmap(5, 23, flecha_bmp, 12, 11, 1); // Flecha
					break;

				case 1:
					if (ui->ui_update_sel) {
						SSD1306_DrawFilledRectangle(4, 8, 14, 51, 0); // Cuadrado negro para limpiar las flechas viejas
						SSD1306_DrawBitmap(5, 41, flecha_bmp, 12, 11, 1);
					}
					break;

				default: break;
				}

			}

			break;

		/*  ------------------------------------  */

		default: break;
		}

		SSD1306_UpdateScreen(); // update screen
		// gracias por el comentario máquina, no se que haría sin ese comentario tan útil

	}

//    SSD1306_DrawBitmap(18, 7, medida_bmp, 42, 11, 1);
//    SSD1306_DrawBitmap(5, 25, config_bmp, 40, 11, 1);
//    SSD1306_DrawBitmap(5, 44, diag_bmp, 37, 11, 1);

//    SSD1306_DrawBitmap(0, 0, grid_bmp, 128, 64, 1);


void ui_FSM_switch(UI_t *ui, Config_t *config, Evento_e evento){

	switch(ui->ui_estado){
	case ESTADO_INICIO:

		if (evento == EV_UP) {
			ui_up(ui, 2);
			ui->ui_update_sel = 1;
			break;
		}

		if (evento == EV_DOWN) {
			ui_down(ui);
			ui->ui_update_sel = 1;
			break;
		}

		if (evento == EV_BOTON_ENCODER){
			switch (ui->ui_seleccion){
			case SEL_CONFIG:
				ui->ui_estado = ESTADO_CONFIG;
				ui->ui_update_background = 1;
				ui->ui_update_sel = 1;
				ui->ui_seleccion = 0;
				break;
			case SEL_MEDIDA:
				ui->ui_estado = ESTADO_MEDIDA;
				ui->ui_update_background = 1;
				ui->ui_update_datos = 1;
				break;
			case SEL_DIAG:
				ui->ui_estado = ESTADO_DIAG;
				ui->ui_update_background = 1;
				ui->ui_update_datos = 1;
				break;
			default:
				break;

		}

		}

		break;

	case ESTADO_DIAG:
		if (evento == EV_BOTON_ENCODER){
			ui->ui_estado = ESTADO_INICIO;

			ui->ui_update_background = 1;
			ui->ui_update_sel = 1;
		} else if (evento == EV_UP) {
			ui->ui_estado = ESTADO_DIAG_TAREAS_IDLE;

			ui->ui_update_background = 1;
			ui->ui_update_sel = 1;
		}
	break;

	case ESTADO_DIAG_TAREAS_IDLE:
		if (evento == EV_BOTON_ENCODER){
			ui->ui_estado = ESTADO_INICIO;
			ui->ui_update_background = 1;
		} else if (evento == EV_UP) {
			ui->ui_estado = ESTADO_DIAG_TAREAS_INPUTS;
			ui->ui_update_background = 1;
		} else if (evento == EV_DOWN) {
			ui->ui_estado = ESTADO_DIAG;
			ui->ui_update_background = 1;
		}
	break;

	case ESTADO_DIAG_TAREAS_INPUTS:
		if (evento == EV_BOTON_ENCODER){
			ui->ui_estado = ESTADO_INICIO;
			ui->ui_update_background = 1;
		} else if (evento == EV_UP) {
			ui->ui_estado = ESTADO_DIAG_TAREAS_MONITOR;
			ui->ui_update_background = 1;
		} else if (evento == EV_DOWN) {
			ui->ui_estado = ESTADO_DIAG_TAREAS_IDLE;
			ui->ui_update_background = 1;
		}
	break;

	case ESTADO_DIAG_TAREAS_MONITOR:
		if (evento == EV_BOTON_ENCODER){
			ui->ui_estado = ESTADO_INICIO;
			ui->ui_update_background = 1;
		} else if (evento == EV_UP) {
			ui->ui_estado = ESTADO_DIAG_TAREAS_UI;
			ui->ui_update_background = 1;
		} else if (evento == EV_DOWN) {
			ui->ui_estado = ESTADO_DIAG_TAREAS_INPUTS;
			ui->ui_update_background = 1;
		}
	break;

	case ESTADO_DIAG_TAREAS_UI:
		if (evento == EV_BOTON_ENCODER){
			ui->ui_estado = ESTADO_INICIO;
			ui->ui_update_background = 1;
		} else if (evento == EV_DOWN) {
			ui->ui_estado = ESTADO_DIAG_TAREAS_MONITOR;
			ui->ui_update_background = 1;
		}
	break;

	case ESTADO_CONFIG:
		if (evento == EV_BOTON_ENCODER){
			if (ui->ui_seleccion == 0) ui->ui_estado = ESTADO_CONFIG_PARAMETRO;
			else if (ui->ui_seleccion == 1) ui->ui_estado = ESTADO_CONFIG_MODO;

			ui->ui_seleccion = 0;
			ui->ui_update_background = 1;
			ui->ui_update_sel = 1;
		} else if (evento == EV_UP) {
			ui_up(ui, 1);
			ui->ui_update_sel = 1;
			break;
		}else if (evento == EV_DOWN) {
			ui_down(ui);
			ui->ui_update_sel = 1;
			break;
		}
	break;

	case ESTADO_CONFIG_MODO:
		if (evento == EV_BOTON_ENCODER){

			if (ui->ui_seleccion == 0) {
				ui->ui_estado = ESTADO_INICIO;
				config->modo = MODO_SINGLE;
			}
			else if (ui->ui_seleccion == 1){
				ui->ui_estado = ESTADO_INICIO;
				config->modo = MODO_MULTIPLE;
			}

			ui->ui_seleccion = 0;
			ui->ui_update_background = 1;
			ui->ui_update_sel = 1;
		} else if (evento == EV_UP) {
			ui_up(ui, 1);
			ui->ui_update_sel = 1;
			break;
		}else if (evento == EV_DOWN) {
			ui_down(ui);
			ui->ui_update_sel = 1;
			break;
		}

	case ESTADO_CONFIG_PARAMETRO:
		if (evento == EV_BOTON_ENCODER){
			if (ui->ui_seleccion == 0) {
				ui->ui_estado = ESTADO_INICIO;
				config->parametro = PARAMETRO_R;
			}
			else if (ui->ui_seleccion == 1){
				ui->ui_estado = ESTADO_INICIO;
				config->parametro = PARAMETRO_C;
			}

			ui->ui_seleccion = 0;
			ui->ui_update_background = 1;
			ui->ui_update_sel = 1;
		} else if (evento == EV_UP) {
			ui_up(ui, 1);
			ui->ui_update_sel = 1;
			break;
		}else if (evento == EV_DOWN) {
			ui_down(ui);
			ui->ui_update_sel = 1;
			break;
		}
	break;

	case ESTADO_MEDIDA:
		if (evento == EV_BOTON_ENCODER){
			ui->ui_estado = ESTADO_INICIO;
			ui->ui_update_background = 1;
		} else if (evento == EV_NEW_SAMPLE) {
			ui->ui_update_datos = 1;
		}
	break;

	default:
		break;
	}

}
