/*
 * vmpinix Loop Subsystem - loop.c
 * Task Manager, Allocation, and Lifecycle Management Implementation
 */

#include "loop.h"

typedef struct {
    kernel_task_t tasks[MAX_TASKS];
    int count;
    int initialized;
} task_manager_t;

static task_manager_t t_manager;

int loop_subsystem_init(void) {
    if (t_manager.initialized) {
        return 0;
    }

    t_manager.count = 0;
    for (int i = 0; i < MAX_TASKS; i++) {
        t_manager.tasks[i].id = 0;
        t_manager.tasks[i].state = TASK_STATE_TERMINATED;
        t_manager.tasks[i].entry = (task_entry_t)0;
        t_manager.tasks[i].arg = (void*)0;
        t_manager.tasks[i].priority = 0;
        t_manager.tasks[i].cpu_ticks_used = 0;
        t_manager.tasks[i].name[0] = '\0';
    }

    t_manager.initialized = 1;
    return 0;
}

int loop_create_task(const char *name, task_entry_t entry, void *arg, unsigned long priority) {
    if (!entry || t_manager.count >= MAX_TASKS) {
        return -1;
    }

    int task_id = t_manager.count + 1;
    kernel_task_t *task = &t_manager.tasks[t_manager.count++];

    task->id = task_id;
    task->state = TASK_STATE_READY;
    task->entry = entry;
    task->arg = arg;
    task->priority = priority;
    task->cpu_ticks_used = 0;

    if (name) {
        int i = 0;
        while (name[i] != '\0' && i < TASK_NAME_LEN - 1) {
            task->name[i] = name[i];
            i++;
        }
        task->name[i] = '\0';
    }

    return task_id;
}

int loop_terminate_task(int id) {
    int found = 0;
    for (int i = 0; i < t_manager.count; i++) {
        if (t_manager.tasks[i].id == id) {
            t_manager.tasks[i].state = TASK_STATE_TERMINATED;
            found = 1;
            break;
        }
    }
    return found ? 0 : -1;
}

kernel_task_t* loop_get_task(int id) {
    for (int i = 0; i < t_manager.count; i++) {
        if (t_manager.tasks[i].id == id) {
            return &t_manager.tasks[i];
        }
    }
    return (kernel_task_t*)0;
}

unsigned int loop_get_active_task_count(void) {
    unsigned int active = 0;
    for (int i = 0; i < t_manager.count; i++) {
        if (t_manager.tasks[i].state != TASK_STATE_TERMINATED) {
            active++;
        }
    }
    return active;
}