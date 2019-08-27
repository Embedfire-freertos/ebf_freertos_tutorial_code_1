#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H

#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portable.h"
#include "list.h"


//#if 0
//typedef struct xSTATIC_TCB
//{
//	void				*pxDummy1;      /* 任务栈顶 */

//	StaticListItem_t	xDummy3[ 2 ];   /* 两个列表节点 */
//	UBaseType_t			uxDummy5;       /* 任务优先级，0表示最低优先级 */
//	void				*pxDummy6;      /* 任务栈起始地址 */
//	                                    /* 任务名称，字符串形式 */
//	uint8_t				ucDummy7[ configMAX_TASK_NAME_LEN ];                                    	

//} StaticTask_t;
//#endif 

//struct xSTATIC_LIST_ITEM
//{
//	TickType_t xDummy1;
//	void *pvDummy2[ 4 ];
//};
//typedef struct xSTATIC_LIST_ITEM StaticListItem_t;

//typedef struct xSTATIC_TCB
//{
//	void				*pxDummy1;      /* 任务栈顶 */
//	StaticListItem_t	xDummy3[ 2 ];   /* 两个列表节点 */
//    
//	void				*pxDummy6;      /* 任务栈起始地址 */
//	                                    /* 任务名称，字符串形式 */
//	uint8_t				ucDummy7[ configMAX_TASK_NAME_LEN ];                                    	

//} StaticTask_t;


typedef struct tskTaskControlBlock
{
	volatile StackType_t    *pxTopOfStack;    /* 栈顶 */

	ListItem_t			    xStateListItem;   /* 任务节点 */
    
    StackType_t             *pxStack;         /* 任务栈起始地址 */
	                                          /* 任务名称，字符串形式 */
	char                    pcTaskName[ configMAX_TASK_NAME_LEN ];

    TickType_t xTicksToDelay;
    UBaseType_t			uxPriority;    
} tskTCB;
typedef tskTCB TCB_t;

















#endif /* INC_FREERTOS_H */
