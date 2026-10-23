/*
 * vmpinix Loop Subsystem - loop.h
 * Task Definitions, Constants, and Function Prototypes
 */

#ifndef LOOP_H
#define LOOP_H

#define MAX_TASKS 64
#define TASK_NAME_LEN 32

/* Görev (Task) Durumları */
#define TASK_STATE_TERMINATED 0
#define TASK_STATE_READY      1
#define TASK_STATE_RUNNING    2
#define TASK_STATE_WAITING    3

/* Görev Girdi Fonksiyonu Tipi */
typedef void (*task_entry_t)(void *arg);

/* Çekirdek Görev Yapısı */
typedef struct {
    int id;
    int state;
    task_entry_t entry;
    void *arg;
    unsigned long priority;
    unsigned long cpu_ticks_used;
    char name[TASK_NAME_LEN];
} kernel_task_t;

/* loop.c Fonksiyonları */
int loop_subsystem_init(void);
int loop_create_task(const char *name, task_entry_t entry, void *arg, unsigned long priority);
int loop_terminate_task(int id);
kernel_task_t* loop_get_task(int id);
unsigned int loop_get_active_task_count(void);

/* runtime.c Fonksiyonları */
int runtime_execute_task(int id);
void runtime_dispatch_tick(void);

/* while.c Fonksiyonları */
void while_polling_worker_init(void);
void while_polling_worker_loop(void);
void while_polling_worker_stop(void);

#endif