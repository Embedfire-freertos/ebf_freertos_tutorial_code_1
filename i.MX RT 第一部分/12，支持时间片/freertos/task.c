#include "FreeRTOS.h"
#include "task.h"


/*
*************************************************************************
*                               任务控制块
*************************************************************************
*/
//typedef struct tskTaskControlBlock
//{
//	volatile StackType_t    *pxTopOfStack;    /* 栈顶 */

//	ListItem_t			    xStateListItem;   /* 任务节点 */
//    
//    StackType_t             *pxStack;         /* 任务栈起始地址 */
//	                                          /* 任务名称，字符串形式 */
//	char                    pcTaskName[ configMAX_TASK_NAME_LEN ];  
//} tskTCB;
//typedef tskTCB TCB_t;

/* 当前正在运行的任务的任务控制块指针，默认初始化为NULL */
TCB_t * volatile pxCurrentTCB = NULL;

/* 任务就绪列表 */
//static List_t pxReadyTasksLists[ configMAX_PRIORITIES ];
List_t pxReadyTasksLists[ configMAX_PRIORITIES ];

static volatile UBaseType_t uxCurrentNumberOfTasks 	= ( UBaseType_t ) 0U;
static UBaseType_t uxTaskNumber 					= ( UBaseType_t ) 0U;
static TaskHandle_t xIdleTaskHandle					= NULL;
static volatile TickType_t xTickCount 				= ( TickType_t ) 0U;
static volatile UBaseType_t uxTopReadyPriority 		= tskIDLE_PRIORITY;

static List_t xDelayedTaskList1;
static List_t xDelayedTaskList2;
static List_t * volatile pxDelayedTaskList;
static List_t * volatile pxOverflowDelayedTaskList;

static volatile TickType_t xNextTaskUnblockTime		= ( TickType_t ) 0U;
static volatile BaseType_t xNumOfOverflows 			= ( BaseType_t ) 0;

/*
*************************************************************************
*                               函数声明
*************************************************************************
*/

static void prvInitialiseNewTask( 	TaskFunction_t pxTaskCode,              /* 任务入口 */
									const char * const pcName,              /* 任务名称，字符串形式 */
									const uint32_t ulStackDepth,            /* 任务栈大小，单位为字 */
									void * const pvParameters,              /* 任务形参 */
                                    UBaseType_t uxPriority,                 /* 任务优先级，数值越大，优先级越高 */
									TaskHandle_t * const pxCreatedTask,     /* 任务句柄 */
									TCB_t *pxNewTCB );                       /* 任务控制块 */
                                    
static void prvAddNewTaskToReadyList( TCB_t *pxNewTCB );
void prvInitialiseTaskLists( void );
                                    
static portTASK_FUNCTION( prvIdleTask, pvParameters );
void vTaskSwitchContext( void );                                    
static void prvAddCurrentTaskToDelayedList( TickType_t xTicksToWait ); 
static void prvResetNextTaskUnblockTime( void );                                    
/*
*************************************************************************
*                               宏定义
*************************************************************************
*/

/* 将任务添加到就绪列表 */                                    
#define prvAddTaskToReadyList( pxTCB )																   \
	taskRECORD_READY_PRIORITY( ( pxTCB )->uxPriority );												   \
	vListInsertEnd( &( pxReadyTasksLists[ ( pxTCB )->uxPriority ] ), &( ( pxTCB )->xStateListItem ) ); \


/* 查找最高优先级的就绪任务：通用方法 */                                    
#if ( configUSE_PORT_OPTIMISED_TASK_SELECTION == 0 )
	/* uxTopReadyPriority 存的是就绪任务的最高优先级 */
	#define taskRECORD_READY_PRIORITY( uxPriority )														\
	{																									\
		if( ( uxPriority ) > uxTopReadyPriority )														\
		{																								\
			uxTopReadyPriority = ( uxPriority );														\
		}																								\
	} /* taskRECORD_READY_PRIORITY */

	/*-----------------------------------------------------------*/

	#define taskSELECT_HIGHEST_PRIORITY_TASK()															\
	{																									\
	UBaseType_t uxTopPriority = uxTopReadyPriority;														\
																										\
		/* 寻找包含就绪任务的最高优先级的队列 */                                                          \
		while( listLIST_IS_EMPTY( &( pxReadyTasksLists[ uxTopPriority ] ) ) )							\
		{																								\
			--uxTopPriority;																			\
		}																								\
																										\
		/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */							            \
		listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( pxReadyTasksLists[ uxTopPriority ] ) );			\
		/* 更新uxTopReadyPriority */                                                                    \
		uxTopReadyPriority = uxTopPriority;																\
	} /* taskSELECT_HIGHEST_PRIORITY_TASK */

	/*-----------------------------------------------------------*/

    /* 这两个宏定义只有在选择优化方法时才用，这里定义为空 */
	#define taskRESET_READY_PRIORITY( uxPriority )
	#define portRESET_READY_PRIORITY( uxPriority, uxTopReadyPriority )
    
/* 查找最高优先级的就绪任务：根据处理器架构优化后的方法 */
#else /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

	#define taskRECORD_READY_PRIORITY( uxPriority )	portRECORD_READY_PRIORITY( uxPriority, uxTopReadyPriority )

	/*-----------------------------------------------------------*/

	#define taskSELECT_HIGHEST_PRIORITY_TASK()														    \
	{																								    \
	UBaseType_t uxTopPriority;																		    \
																									    \
		/* 寻找包含就绪任务的最高优先级的队列 */								                            \
		portGET_HIGHEST_PRIORITY( uxTopPriority, uxTopReadyPriority );								    \
		/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */                                       \
		listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB, &( pxReadyTasksLists[ uxTopPriority ] ) );		    \
	} /* taskSELECT_HIGHEST_PRIORITY_TASK() */

	/*-----------------------------------------------------------*/
#if 1
	#define taskRESET_READY_PRIORITY( uxPriority )														\
	{																									\
		if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ ( uxPriority ) ] ) ) == ( UBaseType_t ) 0 )	\
		{																								\
			portRESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );							\
		}																								\
	}
#else
    #define taskRESET_READY_PRIORITY( uxPriority )											    \
    {																							\
            portRESET_READY_PRIORITY( ( uxPriority ), ( uxTopReadyPriority ) );					\
    }
#endif
    
#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */
                                   

    
/* 
 * 当系统时基计数器溢出的时候，延时列表pxDelayedTaskList 和
 * pxOverflowDelayedTaskList要互相切换
 */
#define taskSWITCH_DELAYED_LISTS()\
{\
	List_t *pxTemp;\
	pxTemp = pxDelayedTaskList;\
	pxDelayedTaskList = pxOverflowDelayedTaskList;\
	pxOverflowDelayedTaskList = pxTemp;\
	xNumOfOverflows++;\
	prvResetNextTaskUnblockTime();\
}


/*
*************************************************************************
*                               静态任务创建函数
*************************************************************************
*/
#if( configSUPPORT_STATIC_ALLOCATION == 1 )

TaskHandle_t xTaskCreateStatic(	TaskFunction_t pxTaskCode,           /* 任务入口 */
					            const char * const pcName,           /* 任务名称，字符串形式 */
					            const uint32_t ulStackDepth,         /* 任务栈大小，单位为字 */
					            void * const pvParameters,           /* 任务形参 */
                                UBaseType_t uxPriority,              /* 任务优先级，数值越大，优先级越高 */
					            StackType_t * const puxStackBuffer,  /* 任务栈起始地址 */
					            TCB_t * const pxTaskBuffer )         /* 任务控制块 */
{
	TCB_t *pxNewTCB;
	TaskHandle_t xReturn;

	if( ( pxTaskBuffer != NULL ) && ( puxStackBuffer != NULL ) )
	{		
		pxNewTCB = ( TCB_t * ) pxTaskBuffer; 
		pxNewTCB->pxStack = ( StackType_t * ) puxStackBuffer;

		/* 创建新的任务 */
		prvInitialiseNewTask( pxTaskCode, pcName, ulStackDepth, pvParameters,uxPriority, &xReturn, pxNewTCB);
        
		/* 将任务添加到就绪列表 */
		prvAddNewTaskToReadyList( pxNewTCB );

	}
	else
	{
		xReturn = NULL;
	}

	return xReturn;
}

#endif /* configSUPPORT_STATIC_ALLOCATION */


static void prvInitialiseNewTask( 	TaskFunction_t pxTaskCode,              /* 任务入口 */
									const char * const pcName,              /* 任务名称，字符串形式 */
									const uint32_t ulStackDepth,            /* 任务栈大小，单位为字 */
									void * const pvParameters,              /* 任务形参 */
                                    UBaseType_t uxPriority,                 /* 任务优先级，数值越大，优先级越高 */
									TaskHandle_t * const pxCreatedTask,     /* 任务句柄 */
									TCB_t *pxNewTCB )                       /* 任务控制块 */

{
	StackType_t *pxTopOfStack;
	UBaseType_t x;	
	
	/* 获取栈顶地址 */
	pxTopOfStack = pxNewTCB->pxStack + ( ulStackDepth - ( uint32_t ) 1 );
	//pxTopOfStack = ( StackType_t * ) ( ( ( portPOINTER_SIZE_TYPE ) pxTopOfStack ) & ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) );
	/* 向下做8字节对齐 */
	pxTopOfStack = ( StackType_t * ) ( ( ( uint32_t ) pxTopOfStack ) & ( ~( ( uint32_t ) 0x0007 ) ) );	

	/* 将任务的名字存储在TCB中 */
	for( x = ( UBaseType_t ) 0; x < ( UBaseType_t ) configMAX_TASK_NAME_LEN; x++ )
	{
		pxNewTCB->pcTaskName[ x ] = pcName[ x ];

		if( pcName[ x ] == 0x00 )
		{
			break;
		}
	}
	/* 任务名字的长度不能超过configMAX_TASK_NAME_LEN */
	pxNewTCB->pcTaskName[ configMAX_TASK_NAME_LEN - 1 ] = '\0';
    
    vListInitialiseItem( &( pxNewTCB->xStateListItem ) );
	listSET_LIST_ITEM_OWNER( &( pxNewTCB->xStateListItem ), pxNewTCB );
	
    /* 初始化优先级 */
	if( uxPriority >= ( UBaseType_t ) configMAX_PRIORITIES )
	{
		uxPriority = ( UBaseType_t ) configMAX_PRIORITIES - ( UBaseType_t ) 1U;
	}
	pxNewTCB->uxPriority = uxPriority;
    
	/* 初始化任务栈 */
	pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack, pxTaskCode, pvParameters ); 
    
    /* 让任务句柄指向任务控制块 */
    if( ( void * ) pxCreatedTask != NULL )
	{		
		*pxCreatedTask = ( TaskHandle_t ) pxNewTCB;
	}
}

/* 初始化任务相关的列表 */
void prvInitialiseTaskLists( void )
{
    UBaseType_t uxPriority;
    
    /* 初始化就绪列表 */
    for( uxPriority = ( UBaseType_t ) 0U; uxPriority < ( UBaseType_t ) configMAX_PRIORITIES; uxPriority++ )
	{
		vListInitialise( &( pxReadyTasksLists[ uxPriority ] ) );
	}
    
    vListInitialise( &xDelayedTaskList1 );
	vListInitialise( &xDelayedTaskList2 );
    
    pxDelayedTaskList = &xDelayedTaskList1;
	pxOverflowDelayedTaskList = &xDelayedTaskList2;
}

static void prvAddNewTaskToReadyList( TCB_t *pxNewTCB )
{
	/* 进入临界段 */
	taskENTER_CRITICAL();
	{
		uxCurrentNumberOfTasks++;
        
        /* 如果pxCurrentTCB为空，则将pxCurrentTCB指向新创建的任务 */
		if( pxCurrentTCB == NULL )
		{
			pxCurrentTCB = pxNewTCB;

			/* 如果是第一次创建任务，则需要初始化任务相关的列表 */
            if( uxCurrentNumberOfTasks == ( UBaseType_t ) 1 )
			{
				/* 初始化任务相关的列表 */
                prvInitialiseTaskLists();
			}
		}
		else /* 如果pxCurrentTCB不为空，则根据任务的优先级将pxCurrentTCB指向最高优先级任务的TCB */
		{
				if( pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority )
				{
					pxCurrentTCB = pxNewTCB;
				}
		}
		uxTaskNumber++;
        
		/* 将任务添加到就绪列表 */
        prvAddTaskToReadyList( pxNewTCB );

	}
	/* 退出临界段 */
	taskEXIT_CRITICAL();
}



extern TCB_t Task1TCB;
extern TCB_t Task2TCB;
extern TCB_t IdleTaskTCB;
void vApplicationGetIdleTaskMemory( TCB_t **ppxIdleTaskTCBBuffer, 
                                    StackType_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize );
void vTaskStartScheduler( void )
{
/*======================================创建空闲任务start==============================================*/     
    TCB_t *pxIdleTaskTCBBuffer = NULL;
    StackType_t *pxIdleTaskStackBuffer = NULL;
    uint32_t ulIdleTaskStackSize;
    
    /* 获取空闲任务的内存：任务栈和任务TCB */
    vApplicationGetIdleTaskMemory( &pxIdleTaskTCBBuffer, 
                                   &pxIdleTaskStackBuffer, 
                                   &ulIdleTaskStackSize );    
    
    xIdleTaskHandle = xTaskCreateStatic( (TaskFunction_t)prvIdleTask,              /* 任务入口 */
					                     (char *)"IDLE",                           /* 任务名称，字符串形式 */
					                     (uint32_t)ulIdleTaskStackSize ,           /* 任务栈大小，单位为字 */
					                     (void *) NULL,                            /* 任务形参 */
                                         (UBaseType_t) tskIDLE_PRIORITY,           /* 任务优先级，数值越大，优先级越高 */
					                     (StackType_t *)pxIdleTaskStackBuffer,     /* 任务栈起始地址 */
					                     (TCB_t *)pxIdleTaskTCBBuffer );           /* 任务控制块 */
/*======================================创建空闲任务end================================================*/ 
    xNextTaskUnblockTime = portMAX_DELAY;
    xTickCount = ( TickType_t ) 0U;
    //xTickCount = ( TickType_t ) 0xfffffffcUL;
    /* 启动调度器 */
    if( xPortStartScheduler() != pdFALSE )
    {
        /* 调度器启动成功，则不会返回，即不会来到这里 */
    }
}



static portTASK_FUNCTION( prvIdleTask, pvParameters )
{
	/* 防止编译器的警告 */
	( void ) pvParameters;
    
    for(;;)
    {
        /* 空闲任务暂时什么都不做 */
    }
}


/* 任务切换，即寻找优先级最高的就绪任务 */
void vTaskSwitchContext( void )
{
	/* 获取优先级最高的就绪任务的TCB，然后更新到pxCurrentTCB */
    taskSELECT_HIGHEST_PRIORITY_TASK();
}


void vTaskDelay( const TickType_t xTicksToDelay )
{
    TCB_t *pxTCB = NULL;
    
    /* 获取当前任务的TCB */
    pxTCB = pxCurrentTCB;
    
    /* 将任务插入到延时列表 */
    prvAddCurrentTaskToDelayedList( xTicksToDelay );
    
    /* 任务切换 */
    taskYIELD();
}


//void xTaskIncrementTick( void )
BaseType_t xTaskIncrementTick( void )
{
	TCB_t * pxTCB;
	TickType_t xItemValue;    
    BaseType_t xSwitchRequired = pdFALSE;
    
	const TickType_t xConstTickCount = xTickCount + 1;
	xTickCount = xConstTickCount;

	/* 如果xConstTickCount溢出，则切换延时列表 */
	if( xConstTickCount == ( TickType_t ) 0U )
	{
		taskSWITCH_DELAYED_LISTS();
	}

	/* 最近的延时任务延时到期 */
	if( xConstTickCount >= xNextTaskUnblockTime )
	{
		for( ;; )
		{
			if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
			{
				/* 延时列表为空，设置xNextTaskUnblockTime为可能的最大值 */
				xNextTaskUnblockTime = portMAX_DELAY;
				break;
			}
			else /* 延时列表不为空 */
			{
				pxTCB = ( TCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList );
				xItemValue = listGET_LIST_ITEM_VALUE( &( pxTCB->xStateListItem ) );

				/* 直到将延时列表中所有延时到期的任务移除才跳出for循环 */
                if( xConstTickCount < xItemValue )
				{
					xNextTaskUnblockTime = xItemValue;
					break;
				}

				/* 将任务从延时列表移除，消除等待状态 */
				( void ) uxListRemove( &( pxTCB->xStateListItem ) );

				/* 将解除等待的任务添加到就绪列表 */
				prvAddTaskToReadyList( pxTCB );
                

                #if (  configUSE_PREEMPTION == 1 )
                {
                    if( pxTCB->uxPriority >= pxCurrentTCB->uxPriority )
                    {
                        xSwitchRequired = pdTRUE;
                    }
                }
                #endif /* configUSE_PREEMPTION */
			}
		}
	}/* xConstTickCount >= xNextTaskUnblockTime */
    
    #if ( ( configUSE_PREEMPTION == 1 ) && ( configUSE_TIME_SLICING == 1 ) )
    {
        if( listCURRENT_LIST_LENGTH( &( pxReadyTasksLists[ pxCurrentTCB->uxPriority ] ) ) 
                                     > ( UBaseType_t ) 1 )
        {
            xSwitchRequired = pdTRUE;
        }
    }
    #endif /* ( ( configUSE_PREEMPTION == 1 ) && ( configUSE_TIME_SLICING == 1 ) ) */
    
    
    /* 任务切换 */
    //portYIELD();
}


static void prvAddCurrentTaskToDelayedList( TickType_t xTicksToWait )
{
    TickType_t xTimeToWake;
    
    /* 获取系统时基计数器xTickCount的值 */
    const TickType_t xConstTickCount = xTickCount;

    /* 将任务从就绪列表中移除 */
	if( uxListRemove( &( pxCurrentTCB->xStateListItem ) ) == ( UBaseType_t ) 0 )
	{
		/* 将任务在优先级位图中对应的位清除 */
        portRESET_READY_PRIORITY( pxCurrentTCB->uxPriority, uxTopReadyPriority );
	}

    /* 计算延时到期时，系统时基计数器xTickCount的值是多少 */
    xTimeToWake = xConstTickCount + xTicksToWait;

    /* 将延时到期的值设置为节点的排序值 */
    listSET_LIST_ITEM_VALUE( &( pxCurrentTCB->xStateListItem ), xTimeToWake );

    /* 溢出 */
    if( xTimeToWake < xConstTickCount )
    {
        vListInsert( pxOverflowDelayedTaskList, &( pxCurrentTCB->xStateListItem ) );
    }
    else /* 没有溢出 */
    {

        vListInsert( pxDelayedTaskList, &( pxCurrentTCB->xStateListItem ) );

        /* 更新下一个任务解锁时刻变量xNextTaskUnblockTime的值 */
        if( xTimeToWake < xNextTaskUnblockTime )
        {
            xNextTaskUnblockTime = xTimeToWake;
        }
    }	
}


static void prvResetNextTaskUnblockTime( void )
{
    TCB_t *pxTCB;

	if( listLIST_IS_EMPTY( pxDelayedTaskList ) != pdFALSE )
	{
		/* The new current delayed list is empty.  Set xNextTaskUnblockTime to
		the maximum possible value so it is	extremely unlikely that the
		if( xTickCount >= xNextTaskUnblockTime ) test will pass until
		there is an item in the delayed list. */
		xNextTaskUnblockTime = portMAX_DELAY;
	}
	else
	{
		/* The new current delayed list is not empty, get the value of
		the item at the head of the delayed list.  This is the time at
		which the task at the head of the delayed list should be removed
		from the Blocked state. */
		( pxTCB ) = ( TCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxDelayedTaskList );
		xNextTaskUnblockTime = listGET_LIST_ITEM_VALUE( &( ( pxTCB )->xStateListItem ) );
	}
}


