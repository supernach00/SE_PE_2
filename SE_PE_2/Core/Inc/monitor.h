#ifndef MONITOR_H
#define MONITOR_H

#define CANTIDAD_TASKS 4
/* Enums */
typedef enum {

	uiTaskID,
	monitorTaskID,
	inputsTaskID,
	muestreoTaskID

}TaskId_e;

/* Structs */
typedef struct {
    TaskId_e ID;
    uint32_t task_stack_free;
    uint32_t task_stack_min;
}Task_t;

typedef struct {
	Task_t tasks[CANTIDAD_TASKS];
}TasksData_t;

typedef struct {
    uint32_t sys_heap_free;
    uint32_t sys_heap_min;
} SystemData_t;

/*Este es el struct que se envia por monitorQueue */
typedef struct {
    SystemData_t system_data;
    TasksData_t tasks_data;
} MonitorData_t;

#endif
