//功能 ：用于测试定时器T4的文件
#include "T4_Time.h"
static unsigned char T4_Count=0;//T4中断计数器
static unsigned char T4_Count1=0;//T4中断计数器
unsigned char flag1 =0;   //KEY1按下的标志位
//初始化定时器T4
void Init_T4(void)//25ms  11.0592Mhz
{
	//初始化T4作为MCULED指示灯
  T4T3M &= 0xDF;		//12T模式
	T4L = 0x00;		//定时器初值
	T4H = 0xA6;		//定时器初值
	T4T3M |= 0x80;		//T4开始计时
	IE2 |= 0X40;//允许T4中断
	T4T3M |= 0x80;//打开T4定时器
}
//定时器T4中断服务程序 25Ms中断一次
void T4_INT(void) interrupt 20
{
	T4_Count++;
	T4_Count1++;
	if(T4_Count > 20)   //led2  500ms闪烁
	{
		T4_Count = 0;
		led2 = ~led2;     //检测T4 工作正常即LED2以500ms亮灭		
	}
	if(T4_Count1 >1)   //按键检测
	{
		T4_Count1 = 0;
	if(KEY1 == 0)
	{
		flag1 =1;
	}
			
	}
}
