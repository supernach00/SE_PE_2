#ifndef MONITOR_H
#define MONITOR_H

#define CANTIDAD_TASKS 4

#include "medida.h"

/* Enums */
typedef enum {

	idleTaskID,
	uiTaskID,
	monitorTaskID,
	inputsTaskID,
	muestreoTaskID

}TaskId_e;

/* Structs */
typedef struct {
    uint32_t task_stack_free;
    uint32_t task_stack_min;
}Task_t;

typedef struct {
	Task_t tasks[CANTIDAD_TASKS];
}TasksData_t;

typedef struct {
    uint32_t sys_heap_free;
    uint32_t sys_heap_min;
    uint8_t fu;
} SystemData_t;

/*Este es el struct que se envia por monitorQueue */
typedef struct {
    SystemData_t system_data;
    TasksData_t tasks_data;
	MuestreoQueue_t muestreo_data[4];
} MonitorData_t;

typedef struct {
	uint32_t tiempo1;
	uint32_t tiempo2;

} MonitorCounter_t;

#endif
