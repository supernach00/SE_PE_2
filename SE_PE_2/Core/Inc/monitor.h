#ifndef MONITOR_H
#define MONITOR_H

typedef struct {
    uint16_t sys_heap_free;
    uint16_t sys_heap_min;
} MonitorDataSistema_t;

typedef enum {

	uiTaskID,
	monitorTaskID,
	inputsTaskID

}TaskId_t;

typedef struct {
    uint16_t task_stack_free;
    uint16_t task_stack_peor;
    TaskId_t ID;
} MonitorDataTasks_t;

#endif
