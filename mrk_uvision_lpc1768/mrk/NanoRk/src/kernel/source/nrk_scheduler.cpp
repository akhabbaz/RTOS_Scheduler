/******************************************************************************
 *  Nano-RK, a real-time operating system for sensor networks.
 *  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
 *  All rights reserved.
 *
 *  This is the Open Source Version of Nano-RK included as part of a Dual
 *  Licensing Model. If you are unsure which license to use please refer to:
 *  http://www.nanork.org/nano-RK/wiki/Licensing
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.0 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Contributing Authors (specific to this file):
 *  Anthony Rowe
 *  Zane Starr
 *  Anand Eswaren
 *******************************************************************************/

#include <nrk.h>
#include <nrk_task.h>
#include <nrk_error.h>
#include <nrk_events.h>
#include <nrk_scheduler.h>
#include <include.h>
#include <ulib.h>
#include <nrk_timer.h>
#include <nrk_time.h>
#include <nrk_cfg.h>
#include <nrk_cpu.h>
#include <nrk_watchdog.h>
#include <nrk_platform_time.h>
#include <nrk_stats.h>
#include <stdio.h>

uint8_t    _nrk_cpu_state;
uint32_t   task_addr;
uint8_t    t;

// These LEDs allow us to visualize which task is currently executing
// on the mbed.
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

static void set_leds(int task_id, bool state) {
	  switch (task_id) {
			  case 1:
					  led1 = state;
				    led2 = false;
				    led3 = false;
				    break;
			  case 2:
					  led1 = false;
					  led2 = state;
				    led3 = false;
				    break;
				case 3:
					  led1 = false;
				    led2 = false;
					  led3 = state;
				    break;
			  default:
					  led1 = false;
				    led2 = false;
				    led3 = false;
		}
}

void _nrk_scheduler() {
    int8_t   task_ID;
    uint16_t next_wake;
    uint16_t start_time_stamp;
	
    nrk_int_disable();
	
    
#ifndef NRK_NO_BOUNDED_CONTEXT_SWAP
    _nrk_high_speed_timer_reset();
    start_time_stamp = _nrk_high_speed_timer_get();
#endif
    
	  // Set to huge number which will later get set to min
    _nrk_set_next_wakeup(MAX_SCHED_WAKEUP_TIME);
    next_wake = 60000;
	
    // Safety zone starts here....
    
    nrk_system_time.nano_secs += ((uint32_t)_nrk_prev_timer_val * NANOS_PER_TICK);
    nrk_system_time.nano_secs -= (nrk_system_time.nano_secs % (uint32_t)NANOS_PER_TICK);
    
    while (nrk_system_time.nano_secs >= NANOS_PER_SEC) {
        nrk_system_time.nano_secs -= NANOS_PER_SEC;
        nrk_system_time.secs++;
        nrk_system_time.nano_secs -= (nrk_system_time.nano_secs % (uint32_t)NANOS_PER_TICK);
    }
    
    if (nrk_cur_task_TCB->suspend_flag == 1 && nrk_cur_task_TCB->task_state != FINISHED) {
        if (nrk_cur_task_TCB->event_suspend == RSRC_EVENT_SUSPENDED) {
            nrk_cur_task_TCB->task_state = EVENT_SUSPENDED;
        } else if (nrk_cur_task_TCB->event_suspend > 0 && nrk_cur_task_TCB->nw_flag == 0) {
            nrk_cur_task_TCB->task_state = EVENT_SUSPENDED;
        } else if (nrk_cur_task_TCB->event_suspend > 0 && nrk_cur_task_TCB->nw_flag == 1) {
            nrk_cur_task_TCB->task_state = SUSPENDED;
        } else {
            nrk_cur_task_TCB->task_state = SUSPENDED;
            nrk_cur_task_TCB->event_suspend = 0;
            nrk_cur_task_TCB->nw_flag = 0;
        }
        
        nrk_rem_from_readyQ(nrk_cur_task_TCB->task_ID);
    }
    
    // Update cpu used value for ended task
    // If the task has used its reserve, suspend task
    // Don't disable IdleTask which is 0
    // Don't decrease cpu_remaining if reserve is 0 and hence disabled
    if (nrk_cur_task_TCB->cpu_reserve != 0 && nrk_cur_task_TCB->task_ID != NRK_IDLE_TASK_ID && 
			      nrk_cur_task_TCB->task_state != SUSPENDED && nrk_cur_task_TCB->task_state != FINISHED) {
        if (nrk_cur_task_TCB->cpu_remaining < _nrk_prev_timer_val) {
            nrk_kernel_error_add(NRK_RESERVE_ERROR, nrk_cur_task_TCB->task_ID);
					  printf("Reserve error! Task %d\r\n", nrk_cur_task_TCB->task_ID);
            nrk_cur_task_TCB->cpu_remaining = 0;
        } else {
            nrk_cur_task_TCB->cpu_remaining -= _nrk_prev_timer_val;
				}
        
        task_ID = nrk_cur_task_TCB->task_ID;
				
				if (nrk_cur_task_TCB->cpu_remaining == 0) {
						nrk_cur_task_TCB->task_state = SUSPENDED;
						nrk_rem_from_readyQ(nrk_cur_task_TCB->task_ID);
				}
    }
    
    // Add eligible tasks back to the ready queue
    // At the same time find the next earliest wakeup
    
    // For each task up to NRK_MAX_TASKS...
    // TODO: The contents of this loop is what you need to implement
    for (task_ID = 0; task_ID < NRK_MAX_TASKS; task_ID++) {
			
        // TODO: Replace the if condition below. If the task is disabled, skip it.
        // Hint: a task ID of -1 in an nrk_task_TCB entry indicates a task is disabled
        if (true) {
						
				}
        
        // TODO: Reset the current task's suspend flag
        // Note, this step changes nothing about the task's state -- just its suspend flag changes.
				
        
        // TODO: Replace the if condition below. If the task is NOT the IDLE task and is NOT finished
        // then we want to consider the task, otherwise we can ignore it
        if (true) {
            
            // TODO: Implement Code Block 1 here
            // You need to update the value of next_wakeup for each task, keeping in mind
            // there are two situations that can arise.
            // Hint: next_wakeup is the number of ticks before the task will be woken up again

            
            // TODO: Implement Code Block 2 here
            // You need to update the value of next_period for each task, keeping in mind
            // there are three situations that can arise.
            // Hint: next_period is the number of ticks until the next period for a task starts

            
        }
        
        // Here is where we add tasks to the ready queue
        if (nrk_task_TCB[task_ID].task_state == SUSPENDED) {
            // TODO: Recall Situation 1B from the lab appendix and replace the condition in the if statement below
            if (true) {
                // TODO: set the cpu_remaining, task_state, and next_wakeup for the current task
                // Hints:
                // For cpu_remaining, think about how long a task needs to execute
                // For task_state, think about the state in which a task in the ready queue should be
                // For next_wakeup, think about the next time we'd want to add this task to the ready queue
            
                //nrk_task_TCB[task_ID].cpu_remaining = TODO
                //nrk_task_TCB[task_ID].task_state    = TODO
                //nrk_task_TCB[task_ID].next_wakeup   = TODO
            
                // TODO: uncomment this line once you've implemented the rest of the for loop
                //nrk_add_to_readyQ(task_ID);
            
            }
            
            // TODO: Here you should set the value of next_wake (already declared for you)
            // to find the minimum next_wakeup within the tasks (excluding a next_wake value of 0)

						
        }
    }
   
    nrk_high_ready_prio = nrk_task_TCB[task_ID].task_prio;
    nrk_high_ready_TCB = &nrk_task_TCB[task_ID];
    
    // next_wake should hold next time when a suspended task might get run
    // task_ID holds the highest priority READY task ID
    // So nrk_task_TCB[task_ID].cpu_remaining holds the READY task's end time
    
    // Now we pick the next wakeup (either the end of the current task, or the possible resume
    // of a suspended task)
    if (task_ID != NRK_IDLE_TASK_ID) {
        // You are a non-Idle Task
        if (nrk_task_TCB[task_ID].cpu_reserve != 0 && nrk_task_TCB[task_ID].cpu_remaining < MAX_SCHED_WAKEUP_TIME) {
            // TODO: We need to check one more condition here, and if it's true, update the value of next_wake
            // Hint: Consult Figure 4 in the lab document
            
					
        } else {
            if (next_wake > MAX_SCHED_WAKEUP_TIME) {
						    next_wake = MAX_SCHED_WAKEUP_TIME;
						}
        }
    } else {
#ifndef NRK_NO_POWER_DOWN
        if (next_wake > NRK_SLEEP_WAKEUP_TIME) {
            if (next_wake - NRK_SLEEP_WAKEUP_TIME < MAX_SCHED_WAKEUP_TIME) {
                if (next_wake - NRK_SLEEP_WAKEUP_TIME < NRK_SLEEP_WAKEUP_TIME) {
                    next_wake = NRK_SLEEP_WAKEUP_TIME - 1;
                } else {
                    next_wake = next_wake - NRK_SLEEP_WAKEUP_TIME;
                }
            } else if (next_wake > NRK_SLEEP_WAKEUP_TIME + MAX_SCHED_WAKEUP_TIME) {
                next_wake = MAX_SCHED_WAKEUP_TIME;
            } else {
                next_wake = MAX_SCHED_WAKEUP_TIME - NRK_SLEEP_WAKEUP_TIME;
            }
        }
#endif
    }
    //  If we disable power down, we still need to wakeup before the overflow
#ifdef NRK_NO_POWER_DOWN
    if (next_wake > MAX_SCHED_WAKEUP_TIME) {
		    next_wake = MAX_SCHED_WAKEUP_TIME;
		}
#endif
		
		// The code block below is used to test your scheduler.
		// DO NOT MODIFY!
		if (nrk_cur_task_TCB->task_ID != nrk_high_ready_TCB->task_ID) {
				printf("Scheduling in task %d\r\n", nrk_high_ready_TCB->task_ID);
				set_leds(nrk_cur_task_TCB->task_ID, false);
				set_leds(nrk_high_ready_TCB->task_ID, true);
		}
		
    nrk_cur_task_prio = nrk_high_ready_prio;
    nrk_cur_task_TCB  = nrk_high_ready_TCB;
    
    _nrk_prev_timer_val = next_wake;
    
    if ((_nrk_os_timer_get() + 1) >= next_wake) {
        // +2 just in case we are on the edge of the last tick
        next_wake = _nrk_os_timer_get() + 2;
        _nrk_prev_timer_val = next_wake;
    }
    
    if (task_ID != NRK_IDLE_TASK_ID) {
		    _nrk_cpu_state = CPU_ACTIVE;
		}
    
    _nrk_set_next_wakeup(next_wake);
#ifndef NRK_NO_BOUNDED_CONTEXT_SWAP
    // Bound Context Swap to 100us
    nrk_high_speed_timer_wait(start_time_stamp, CONTEXT_SWAP_TIME_BOUND);
#endif
    nrk_stack_pointer_restore();
    
    task_addr = (unsigned int) nrk_task_TCB[task_ID].task_addr ;
    
    
    Switch();
}
