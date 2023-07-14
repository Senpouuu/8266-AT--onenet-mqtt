#ifndef _MCU_EXTI_H_
#define _MCU_EXTI_H_


#include "stm32f10x.h"


_Bool MCU_EXTI_Init(GPIO_TypeDef *GPIO_Group, unsigned short GPIO_Pin,
						EXTIMode_TypeDef EXTI_Mode, EXTITrigger_TypeDef EXTI_Trigger, FunctionalState EXTI_LineCmd);


#endif
