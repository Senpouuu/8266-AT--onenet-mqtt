#ifndef _DELAY_H_
#define _DELAY_H_


#include "rtthread.h"




#define RTOS_TimeDly(ticks) 						rt_thread_delay(ticks)

#define RTOS_EnterInt()								rt_interrupt_enter()
#define RTOS_ExitInt()								rt_interrupt_leave()

#define RTOS_ENTER_CRITICAL()						rt_enter_critical()
#define RTOS_EXIT_CRITICAL()						rt_exit_critical()

#define RTOS_GetTicks()								rt_tick_get()

#define RTOS_TICK_PER_SEC							RT_TICK_PER_SECOND




void Delay_Init(void);

void DelayUs(unsigned short us);

void DelayXms(unsigned short ms);

#endif
