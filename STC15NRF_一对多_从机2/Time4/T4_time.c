//���� �����ڲ��Զ�ʱ��T4���ļ�
#include "T4_Time.h"
static unsigned char T4_Count=0;//T4�жϼ�����
static unsigned char T4_Count1=0;//T4�жϼ�����
unsigned char flag1 =0;   //KEY1���µı�־λ
//��ʼ����ʱ��T4
void Init_T4(void)//25ms  11.0592Mhz
{
	//��ʼ��T4��ΪMCULEDָʾ��
  T4T3M &= 0xDF;		//12Tģʽ
	T4L = 0x00;		//��ʱ����ֵ
	T4H = 0xA6;		//��ʱ����ֵ
	T4T3M |= 0x80;		//T4��ʼ��ʱ
	IE2 |= 0X40;//����T4�ж�
	T4T3M |= 0x80;//��T4��ʱ��
}
//��ʱ��T4�жϷ������ 25Ms�ж�һ��
void T4_INT(void) interrupt 20
{
	T4_Count++;
	T4_Count1++;
	if(T4_Count > 20)   //led2  500ms��˸
	{
		T4_Count = 0;
		led2 = ~led2;     //���T4 ����������LED2��500ms����		
	}
	if(T4_Count1 >1)   //�������
	{
		T4_Count1 = 0;
	if(KEY1 == 0)
	{
		flag1 =1;
	}
			
	}
}
