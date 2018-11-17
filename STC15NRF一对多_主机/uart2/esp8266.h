//功能 ：用于测试定时器T4的文件
#ifndef __ESP8266_H__
#define __ESP8266_H__
#include "WIFI.h"
#include "delay.h"
#include <STC15W4K.h>
u8	xdata	RX2_Buffer[];	//接收缓冲
void clear();
//void WIFI();
void PrintString2(u8 *puts);   //注意申明函数时，函数有参数要写上
void UART2_config(u8 brt);
//u8 flag_xintiao ;
#endif