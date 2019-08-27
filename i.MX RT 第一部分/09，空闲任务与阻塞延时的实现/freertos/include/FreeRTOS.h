#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H

#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portable.h"
#include "list.h"

typedef struct tskTaskControlBlock
{
	volatile StackType_t    *pxTopOfStack;    /* 栈顶 */

	ListItem_t			    xStateListItem;   /* 任务节点 */
    
    StackType_t             *pxStack;         /* 任务栈起始地址 */
	                                          /* 任务名称，字符串形式 */
	char                    pcTaskName[ configMAX_TASK_NAME_LEN ];

    TickType_t xTicksToDelay; /* 用于延时 */    
} tskTCB;
typedef tskTCB TCB_t;

















#endif /* INC_FREERTOS_H */
