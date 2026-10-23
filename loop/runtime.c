/*
 * vmpinix Loop Subsystem - runtime.c
 * Task Execution Runtime Engine and Context Dispatcher
 */

#include "loop.h"

extern kernel_task_t* loop_get_task(int id);

int runtime_execute_task(int id) {
    kernel_task_t *task = loop_get_task(id);
    if (!task || task->state == TASK_STATE_TERMINATED) {
        return -1;
    }

    task->state = TASK_STATE_RUNNING;

    if (task->entry) {
        task->entry(task->arg);
    }

    task->state = TASK_STATE_WAITING;
    task->cpu_ticks_used++;

    return 0;
}

void runtime_dispatch_tick(void) {
    // Çalışma zamanı zamanlayıcı işleyicisi: Hazır görevleri sırayla yürüt
    for (int i = 1; i <= MAX_TASKS; i++) {
        kernel_task_t *t = loop_get_task(i);
        if (t && t->state == TASK_STATE_READY) {
            runtime_execute_task(t->id);
        }
    }
}