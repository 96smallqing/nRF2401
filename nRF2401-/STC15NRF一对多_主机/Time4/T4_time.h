//���� �����ڲ��Զ�ʱ��T4���ļ�
#ifndef __T4_Test_H__
#define __T4_Test_H__
#include<STC15W4K.h>

sbit KEY1 = P4^4;//����ԭ��ͼ��KEY1��Ӧ��IO��
sbit KEY2 = P4^5;//����ԭ��ͼ��KEY2��Ӧ��IO��
sbit KEY3 = P4^6;//����ԭ��ͼ��KEY3��Ӧ��IO��
sbit KEY4 = P4^7;//����ԭ��ͼ��KEY3��Ӧ��IO��
sbit led2 = P4^2;//����ԭ��ͼ��led2��Ӧ��IO��
sbit speak = P5^5;//��������Ӧ��IO��
extern unsigned char flag1;
extern unsigned char flag2;
extern unsigned char Fasong1;
void Init_T4(void);//��ʼ����ʱ��T4




#endif
