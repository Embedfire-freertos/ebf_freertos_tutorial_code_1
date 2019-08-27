/**
  ************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2018-xx-xx
  * @brief   《FreeRTOS内核实现与应用开发实战指南》书籍例程
  *           列表与列表项
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
#include "list.h"

/*
*************************************************************************
*                              全局变量
*************************************************************************
*/

/* 定义链表根节点 */
struct xLIST       List_Test;

/* 定义节点 */
struct xLIST_ITEM  List_Item1;
struct xLIST_ITEM  List_Item2;
struct xLIST_ITEM  List_Item3;



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
	
    /* 链表根节点初始化 */
    vListInitialise( &List_Test );
    
    /* 节点1初始化 */
    vListInitialiseItem( &List_Item1 );
    List_Item1.xItemValue = 1;
    
    /* 节点2初始化 */    
    vListInitialiseItem( &List_Item2 );
    List_Item2.xItemValue = 2;
    
    /* 节点3初始化 */
    vListInitialiseItem( &List_Item3 );
    List_Item3.xItemValue = 3;
    
    /* 将节点插入链表，按照升序排列 */
    vListInsert( &List_Test, &List_Item2 );    
    vListInsert( &List_Test, &List_Item1 );
    vListInsert( &List_Test, &List_Item3 );    
    
    for(;;)
	{
		/* 啥事不干 */
	}
}
