#include "mbed.h"                    
#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <hal.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <nrk_stack_check.h>
#include <nrk_stats.h>

NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);

NRK_STK Stack2[NRK_APP_STACKSIZE];
nrk_task_type TaskTwo;
void Task2(void);

NRK_STK Stack3[NRK_APP_STACKSIZE];
nrk_task_type TaskThree;
void Task3(void);

void nrk_create_taskset();

// You do not need to modify this function
struct __FILE { int handle; };

int main(void) {
    nrk_setup_ports();
    nrk_init();
    nrk_create_taskset();
	  printf("\r\n\r\n========== START ==========\r\n\r\n");
    nrk_start();
    return 0;
}


void Task1() {
    while (1) {
        wait_ms(50);
			  nrk_wait_until_next_period();
    }
}

void Task2() {
    while (1) {
        wait_ms(50);
			  nrk_wait_until_next_period();
    }
}

void Task3() {
    while (1) {
        wait_ms(50);
			  nrk_wait_until_next_period();
    }
}

void nrk_create_taskset() {
    nrk_task_set_entry_function( &TaskOne, Task1);
    nrk_task_set_stk( &TaskOne, Stack1, NRK_APP_STACKSIZE);
    TaskOne.prio = 3;
    TaskOne.FirstActivation = TRUE;
    TaskOne.Type = BASIC_TASK;
    TaskOne.SchType = PREEMPTIVE;
    TaskOne.period.secs = 0;
    TaskOne.period.nano_secs = 300*NANOS_PER_MS;
    TaskOne.cpu_reserve.secs = 0;
    TaskOne.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
    TaskOne.offset.secs = 0;
    TaskOne.offset.nano_secs = 0;
    nrk_activate_task (&TaskOne);
    
    nrk_task_set_entry_function( &TaskTwo, Task2);
    nrk_task_set_stk( &TaskTwo, Stack2, NRK_APP_STACKSIZE);
    TaskTwo.prio = 2;
    TaskTwo.FirstActivation = TRUE;
    TaskTwo.Type = BASIC_TASK;
    TaskTwo.SchType = PREEMPTIVE;
    TaskTwo.period.secs = 1;
    TaskTwo.period.nano_secs = 000*NANOS_PER_MS;
    TaskTwo.cpu_reserve.secs = 0;
    TaskTwo.cpu_reserve.nano_secs = 300*NANOS_PER_MS;
    TaskTwo.offset.secs = 0;
    TaskTwo.offset.nano_secs = 0;
    nrk_activate_task (&TaskTwo);
    
    nrk_task_set_entry_function( &TaskThree, Task3);
    nrk_task_set_stk( &TaskThree, Stack3, NRK_APP_STACKSIZE);
    TaskThree.prio = 1;
    TaskThree.FirstActivation = TRUE;
    TaskThree.Type = BASIC_TASK;
    TaskThree.SchType = PREEMPTIVE;
    TaskThree.period.secs = 1;
    TaskThree.period.nano_secs = 500*NANOS_PER_MS;
    TaskThree.cpu_reserve.secs = 0;
    TaskThree.cpu_reserve.nano_secs = 400*NANOS_PER_MS;
    TaskThree.offset.secs = 0;
    TaskThree.offset.nano_secs = 0;
    nrk_activate_task (&TaskThree);
}
