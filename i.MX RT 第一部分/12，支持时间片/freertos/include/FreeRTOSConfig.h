#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_16_BIT_TICKS		              0
#define configMAX_TASK_NAME_LEN		            ( 16 )
#define configSUPPORT_STATIC_ALLOCATION           1
#define configMAX_PRIORITIES		            ( 5 )

#define configKERNEL_INTERRUPT_PRIORITY 		255   /* 高四位有效，即等于0xff，或者是15 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	191   /* 高四位有效，即等于0xb0，或者是11 */

#define configMINIMAL_STACK_SIZE	( ( unsigned short ) 128 )

#define configCPU_CLOCK_HZ			( ( unsigned long ) 25000000 )	
#define configTICK_RATE_HZ			( ( TickType_t ) 1000 )

#define configUSE_PREEMPTION            1

#define xPortPendSVHandler   PendSV_Handler
#define xPortSysTickHandler  SysTick_Handler
#define vPortSVCHandler      SVC_Handler


#endif /* FREERTOS_CONFIG_H */
