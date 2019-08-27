/**
  ************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   《FreeRTOS内核实现与应用开发实战指南》书籍例程
  *           新建FreeRTOS工程—软件仿真
  ************************************************************************
  * @attention
  *
  * 实验平台:野火 STM32 系列 开发板
  * 
  * 官网    :www.embedfire.com  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ************************************************************************
  */
  
/*
*************************************************************************
*                             包含的头文件
*************************************************************************
*/
#include "FreeRTOS.h"
#include "task.h"

/*
*************************************************************************
*                              全局变量
*************************************************************************
*/
portCHAR flag1;
portCHAR flag2;

extern List_t pxReadyTasksLists[ configMAX_PRIORITIES ];


/*
*************************************************************************
*                        任务控制块 & STACK 
*************************************************************************
*/
TaskHandle_t Task1_Handle;
#define TASK1_STACK_SIZE                    20
StackType_t Task1Stack[TASK1_STACK_SIZE];
TCB_t Task1TCB;

TaskHandle_t Task2_Handle;
#define TASK2_STACK_SIZE                    20
StackType_t Task2Stack[TASK2_STACK_SIZE];
TCB_t Task2TCB;


/*
*************************************************************************
*                               函数声明 
*************************************************************************
*/
void delay (uint32_t count);
void Task1_Entry( void *p_arg );
void Task2_Entry( void *p_arg );

/*
************************************************************************
*                                main函数
************************************************************************
*/
/*
* 注意事项：1、该工程使用软件仿真，debug需选择 Ude Simulator
*           2、在Target选项卡里面把晶振Xtal(Mhz)的值改为25，默认是12，
*              改成25是为了跟system_ARMCM3.c中定义的__SYSTEM_CLOCK相同，确保仿真的时候时钟一致
*/
int main(void)
{	
    /* 硬件初始化 */
	/* 将硬件相关的初始化放在这里，如果是软件仿真则没有相关初始化代码 */
    
    /* 初始化与任务相关的列表，如就绪列表 */
    prvInitialiseTaskLists();
    
    /* 创建任务 */
    Task1_Handle = xTaskCreateStatic( (TaskFunction_t)Task1_Entry,   /* 任务入口 */
					                  (char *)"Task1",               /* 任务名称，字符串形式 */
					                  (uint32_t)TASK1_STACK_SIZE ,   /* 任务栈大小，单位为字 */
					                  (void *) NULL,                 /* 任务形参 */
					                  (StackType_t *)Task1Stack,     /* 任务栈起始地址 */
					                  (TCB_t *)&Task1TCB );          /* 任务控制块 */
    /* 将任务添加到就绪列表 */                                 
    vListInsertEnd( &( pxReadyTasksLists[1] ), &( ((TCB_t *)(&Task1TCB))->xStateListItem ) );
                                
    Task2_Handle = xTaskCreateStatic( (TaskFunction_t)Task2_Entry,   /* 任务入口 */
					                  (char *)"Task2",               /* 任务名称，字符串形式 */
					                  (uint32_t)TASK2_STACK_SIZE ,   /* 任务栈大小，单位为字 */
					                  (void *) NULL,                 /* 任务形参 */
					                  (StackType_t *)Task2Stack,     /* 任务栈起始地址 */
					                  (TCB_t *)&Task2TCB );          /* 任务控制块 */
    /* 将任务添加到就绪列表 */                                 
    vListInsertEnd( &( pxReadyTasksLists[2] ), &( ((TCB_t *)(&Task2TCB))->xStateListItem ) );
                                      
    /* 启动调度器，开始多任务调度，启动成功则不返回 */
    vTaskStartScheduler();                                      
    
    for(;;)
	{
		/* 系统启动成功不会到达这里 */
	}
}

/*
*************************************************************************
*                               函数实现
*************************************************************************
*/
/* 软件延时 */
void delay (uint32_t count)
{
	for(; count!=0; count--);
}
/* 任务1 */
void Task1_Entry( void *p_arg )
{
	for( ;; )
	{
		flag1 = 1;
		delay( 100 );		
		flag1 = 0;
		delay( 100 );
		
		/* 任务切换，这里是手动切换 */
        taskYIELD();
	}
}

/* 任务2 */
void Task2_Entry( void *p_arg )
{
	for( ;; )
	{
		flag2 = 1;
		delay( 100 );		
		flag2 = 0;
		delay( 100 );
		
		/* 任务切换，这里是手动切换 */
        taskYIELD();
	}
}



























