//功能 ：用于测试定时器T4的文件
#ifndef __T4_Test_H__
#define __T4_Test_H__
#include<STC15W4K.h>

sbit KEY1 = P4^4;//根据原理图，KEY1对应的IO口
sbit KEY2 = P4^5;//根据原理图，KEY2对应的IO口
sbit KEY3 = P4^6;//根据原理图，KEY3对应的IO口
sbit KEY4 = P4^7;//根据原理图，KEY3对应的IO口
sbit led2 = P4^2;//根据原理图，led2对应的IO口
sbit speak = P5^5;//蜂鸣器对应的IO口
extern unsigned char flag1;
void Init_T4(void);//初始化定时器T4




#endif
