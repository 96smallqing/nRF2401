//实验晶振位11.0592MHz，波特率位9600
//NRF从机2
#include "stc15w4k.h"
#include "T4_Time.h"
#include "oled.h"
#include "T4_Time.h"
#include <intrins.h>
typedef unsigned char uchar;
typedef unsigned char uint;

/******************************STC15串口1定时器1模式0定义************************************/
typedef unsigned char BYTE;
typedef unsigned int WORD;

#define FOSC 11059200L          //系统频率
#define BAUD 9600             //串口波特率

#define NONE_PARITY     0       //无校验
#define ODD_PARITY      1       //奇校验
#define EVEN_PARITY     2       //偶校验
#define MARK_PARITY     3       //标记校验
#define SPACE_PARITY    4       //空白校验

#define PARITYBIT EVEN_PARITY   //定义校验位

#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7
bit busy;

void SendData(BYTE dat);
void SendString(char *s);

//****************************************IO端口定义***************************************

sbit  MISO = P3^2;
sbit  MOSI = P3^5;
sbit  SCK  = P3^4;
sbit  CE   = P3^6;
sbit  CSN  = P3^7;
sbit  IRQ  = P3^3;

sbit led0 = P4^3;
sbit led1 = P4^2;
//sbit led2 = P4^1;
sbit led3 = P4^0;

//*********************************************NRF24L01*************************************
#define TX_ADR_WIDTH    5    // 5 uints TX address width
#define RX_ADR_WIDTH    5    // 5 uints RX address width
#define TX_PLOAD_WIDTH  32  // 20 uints TX payload
#define RX_PLOAD_WIDTH  32   // 20 uints TX payload
uint const TX_ADDRESS_2[TX_ADR_WIDTH]= {0x10,0x10,0x10,0x10,0x20}; //本地地址从机2	  0x10,0x10,0x10,0x10,0x20
uint const RX_ADDRESS[RX_ADR_WIDTH]= {0x01,0x01,0x01,0x01,0x01}; //接收主机地址	  0x01,0x01,0x01,0x01,0x01
//***************************************NRF24L01寄存器指令*******************************************************
#define READ_REG        0x00   //读配置寄存器,低5位为寄存器地址
#define WRITE_REG       0x20  //写配置寄存器,低5位为寄存器地址
#define RD_RX_PLOAD     0x61   // 读取接收数据指令	   读RX有效数据,1~32字节
#define WR_TX_PLOAD     0xA0   // 写待发数据指令	   写TX有效数据,1~32字节
#define FLUSH_TX        0xE1  // 冲洗发送 FIFO指令	   清除TX FIFO寄存器.发射模式下用
#define FLUSH_RX        0xE2   // 冲洗接收 FIFO指令	   清除RX FIFO寄存器.接收模式下用
#define REUSE_TX_PL     0xE3   // 定义重复装载数据指令	重新使用上一包数据,CE为高,数据包被不断发送.
#define NOP             0xFF   // 保留				   空操作,可以用来读状态寄存器
//*************************************SPI(nRF24L01)寄存器地址****************************************************
#define CONFIG          0x00  // 配置收发状态，CRC校验模式以及收发状态响应方式
#define EN_AA           0x01  // 自动应答功能设置
#define EN_RXADDR       0x02  // 可用信道设置
#define SETUP_AW        0x03  // 收发地址宽度设置
#define SETUP_RETR      0x04  // 自动重发功能设置
#define RF_CH           0x05  // 工作频率设置
#define RF_SETUP        0x06  // 发射速率、功耗功能设置	  RF寄存器
#define STATUS          0x07  // 状态寄存器
#define OBSERVE_TX      0x08  // 发送监测功能
#define CD              0x09  // 地址检测           	 载波检测寄存器
#define RX_ADDR_P0      0x0A  // 频道0接收数据地址
#define RX_ADDR_P1      0x0B  // 频道1接收数据地址
#define RX_ADDR_P2      0x0C  // 频道2接收数据地址
#define RX_ADDR_P3      0x0D  // 频道3接收数据地址
#define RX_ADDR_P4      0x0E  // 频道4接收数据地址
#define RX_ADDR_P5      0x0F  // 频道5接收数据地址
#define TX_ADDR         0x10  // 发送地址寄存器
#define RX_PW_P0        0x11  // 接收频道0接收数据长度	   (1~32字节)
#define RX_PW_P1        0x12  // 接收频道0接收数据长度
#define RX_PW_P2        0x13  // 接收频道0接收数据长度
#define RX_PW_P3        0x14  // 接收频道0接收数据长度
#define RX_PW_P4        0x15  // 接收频道0接收数据长度
#define RX_PW_P5        0x16  // 接收频道0接收数据长度
#define FIFO_STATUS     0x17  // FIFO栈入栈出状态寄存器设置
//**************************************************************************************
void Delay(unsigned int s);
void inerDelay_us(unsigned char n);
void init_NRF24L01(void);
uint SPI_RW(uint uchar);
uchar SPI_Read(uchar reg);
void SetRX_Mode(void);
uint SPI_RW_Reg(uchar reg, uchar value);
uint SPI_Read_Buf(uchar reg, uchar *pBuf, uchar uchars);
uint SPI_Write_Buf(uchar reg, uchar *pBuf, uchar uchars);
unsigned char nRF24L01_RxPacket(unsigned char* rx_buf);
void nRF24L01_TxPacket(unsigned char * tx_buf);

uchar flag,a;

//*****************************************长延时*****************************************
void Delay(unsigned int s)
{
	unsigned int i;
	for(i=0; i<s; i++);
	for(i=0; i<s; i++);
}
//******************************************************************************************
uint  bdata sta;   //状态标志
sbit RX_DR =sta^6;		 //判断是否接收到数据	  若收到数据 则被置1
sbit TX_DS =sta^5;
sbit MAX_RT =sta^4;
/******************************************************************************************
/*延时函数
/******************************************************************************************/
void inerDelay_us(unsigned char n)
{
	for(;n>0;n--)
    _nop_();
}
//****************************************************************************************
/*NRF24L01初始化
//***************************************************************************************/
void init_NRF24L01(void)
{
    inerDelay_us(100);
    CE=0;    // chip enable
    CSN=1;   // Spi disable
    SCK=0;   // Spi clock line init high
	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS_2, TX_ADR_WIDTH);    // 写本地地址
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); // 写接收端地址

	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);      //  频道0自动 ACK应答允许禁止
	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);  //  允许接收地址只有频道0，如果需要多频道可以参考Page21  
	
	SPI_RW_Reg(WRITE_REG + SETUP_AW, 0x02); // Setup address width=5 bytes	  安装地址宽度＝5字节
	SPI_RW_Reg(WRITE_REG + RF_CH, 40);        //   设置信道工作为2.4GHZ，收发必须一致
	SPI_RW_Reg(WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); //设置接收数据长度，本次设置为32字节
	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);     //设置发射速率为1MHZ，发射功率为最大值0dB
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);     // IRQ收发完成中断响应，16位CRC ，主接收
}
//****************************************************************************************
/*NRF24L01发送初始化
//***************************************************************************************/
void init_NRF24L01_Send(void)
{
   /*
		两个nrf24l01通信，需要满足3个条件相同：
		1.频道相同（设置频道寄存器RF_CH）
		2.地址相同（设置TX_ADDR和RX_ADDR_P0相同）
		3.每次发送接收的字节数相同（如果设置了通道的有效数据宽度为n，那么每次发送的字节数也必须为n，当然，n<=32）
	*/
    inerDelay_us(100);
    CE=0;    // chip enable  芯片使能
    CSN=1;   // Spi disable  SPI禁用
    SCK=0;   // Spi clock line init high   SPI时钟线
	  IRQ=1;
	  SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS_2, TX_ADR_WIDTH);    // 写本地地址     与接收代码的接收端地址相同
	  SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); // 写接收端地址
	
	//有了以下这三个配置，发送方的流程就变成了发送-触发中断。这样就抛开了接收方，可以专心去调试发送
//	SPI_RW_Reg(WRITE_REG + EN_AA, 0x00);                 // 失能通道0自动应答
//  SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x00);            // 失能接收通道0
//	SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x00);          // 失能自动重发
	
	//以上三句只是调试方法，最终的产品如果不加上应答和重发的话那么数据的稳定性是很难保证的，所以在基本的通讯建立之后就要把发送的配置改为以下这三个配置 
	//这样发送和接收就进入了一个标准状态，发送-等应答-（自动重发）-触发中断；接收-应答-触发中断，一切按部就班，程序里加上自己的应用部分就能实现很多功能了
	  SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);               // 使能接收通道0自动应答
    SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);           // 使能接收通道0               
    SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x1a);          // 自动重发10次，间隔500us
	
  	SPI_RW_Reg(WRITE_REG + SETUP_AW, 0x02); // Setup address width=5 bytes  安装地址宽度＝5字节	  
  	SPI_RW_Reg(WRITE_REG + RF_CH, 0);        			//设置信道工作为2.4GHZ，收发必须一致
  	SPI_RW_Reg(WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH); 	//设置接收数据长度，本次设置为32字节
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);     		//设置发射速率为2MHZ，发射功率为最大值0dB
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);      			// IRQ收发完成中断响应，16位CRC，发射模式
}
/****************************************************************************************************
/*函数：uint SPI_RW(uint uchar)
/*功能：NRF24L01的SPI写时序
/****************************************************************************************************/
uint SPI_RW(uint uchar)
{
uint bit_ctr;
    for(bit_ctr=0;bit_ctr<8;bit_ctr++) // output 8-bit
    {
		  MOSI = (uchar & 0x80);         // output 'uchar', MSB to MOSI		输出“uchar”，MSB到MOSI
		  uchar = (uchar << 1);           // shift next bit into MSB..		将下一位移到MSB
		  SCK = 1;                      // Set SCK high..					设置SCK高。
		  uchar |= MISO;           // capture current MISO bit				获取当前 MISO bit
		  SCK = 0;                // ..then set SCK low again				然后再次设置SCK低电平
    }
    return(uchar);               // return read uchar						返回读取uchar
}
/****************************************************************************************************
/*函数：uchar SPI_Read(uchar reg)
/*功能：NRF24L01的SPI时序
/****************************************************************************************************/
uchar SPI_Read(uchar reg)
{
	uchar reg_val;
	
	CSN = 0;                // CSN low, initialize SPI communication...	CSN低，初始化SPI通信
	SPI_RW(reg);            // Select register to read from..			写寄存器地址  选择寄存器读取
	reg_val = SPI_RW(0);    // ..then read registervalue				然后读取注册值 写入读寄存器指令
	CSN = 1;                // CSN high, terminate SPI communication	CSN高，终止SPI通信
	
	return(reg_val);        // return register value					返回寄存器值
}
/****************************************************************************************************/
/*功能：NRF24L01读写寄存器函数
/****************************************************************************************************/
uint SPI_RW_Reg(uchar reg, uchar value)
{
	uint status;
	
	CSN = 0;                   // CSN low, init SPI transaction	  CSN置低 进入SPI通信
	status = SPI_RW(reg);      // select register				  选择寄存器
	SPI_RW(value);             // ..and write value to it..		  并将其写入值
	CSN = 1;                   // CSN high again				  CSN再高																  
	return(status);            // return nRF24L01 status uchar	  返回nRF24L01状态uchar
}
/****************************************************************************************************/
/*函数：uint SPI_Read_Buf(uchar reg, uchar *pBuf, uchar uchars)
/*功能: 用于读数据，reg：为寄存器地址，pBuf：为待读出数据地址，uchars：读出数据的个数
/****************************************************************************************************/
uint SPI_Read_Buf(uchar reg, uchar *pBuf, uchar uchars)
{
	uint status,uchar_ctr;
	
	CSN = 0;                      // Set CSN low, init SPI tranaction	 CSN置低 进入SPI通信
	status = SPI_RW(reg);         // Select register to write to and read status uchar 选择寄存器写入和读取状态uCHAR  写入要读取的寄存器地址
	
	for(uchar_ctr=0;uchar_ctr<uchars;uchar_ctr++)
	  pBuf[uchar_ctr] = SPI_RW(0);    //
	
	CSN = 1;                           
	
	return(status);                    // return nRF24L01 status uchar
}
/*********************************************************************************************************
/*函数：uint SPI_Write_Buf(uchar reg, uchar *pBuf, uchar uchars)
/*功能: 用于写数据：为寄存器地址，pBuf：为待写入数据地址，uchars：写入数据的个数
/*********************************************************************************************************/
uint SPI_Write_Buf(uchar reg, uchar *pBuf, uchar uchars)
{
	uint status,uchar_ctr;
	
	CSN = 0;            //SPI使能      
	status = SPI_RW(reg);   
	for(uchar_ctr=0; uchar_ctr<uchars; uchar_ctr++) //
	  SPI_RW(*pBuf++);
	CSN = 1;           //关闭SPI
	return(status);    //
}
/****************************************************************************************************/
/*函数：void SetRX_Mode(void)
/*功能：数据接收配置
/****************************************************************************************************/
void SetRX_Mode(void)
{
	CE=0;
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);     // IRQ收发完成中断响应，16位CRC ，主接收
	CE = 1;
	inerDelay_us(130);
}
/****************************************************************************************************/
/*函数：void SetTX_Mode(void)
/*功能：数据发送配置
/****************************************************************************************************/
void SetTX_Mode(void)
{
	CE=0;
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);     // IRQ收发完成中断响应，16位CRC ，主接收
	CE = 1;
	inerDelay_us(200);
}
/******************************************************************************************************/
/*函数：unsigned char nRF24L01_RxPacket(unsigned char* rx_buf)
/*功能：数据读取后放如rx_buf接收缓冲区中
/******************************************************************************************************/
unsigned char nRF24L01_RxPacket(unsigned char* rx_buf)
{
    unsigned char revale=0;
	sta=SPI_Read(STATUS); 	// 读取状态寄存其来判断数据接收状况
	if(RX_DR)    			// 判断是否接收到数据
	{
	     CE = 0;    		//SPI使能
		 SPI_Read_Buf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer	从RXXFIFO缓冲器读取接收有效载荷
		 revale =1;   		//读取数据完成标志
	}
	SPI_RW_Reg(WRITE_REG+STATUS,sta);   				//接收到数据后RX_DR,TX_DS,MAX_PT都置高为1，通过写1来清楚中断标志
	return revale;
}
/***********************************************************************************************************
/*函数：void nRF24L01_TxPacket(unsigned char * tx_buf)
/*功能：发送 tx_buf中数据
/**********************************************************************************************************/

void nRF24L01_TxPacket(unsigned char * tx_buf)
{
	CE=0;   			//StandBy I模式
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS_2, RX_ADR_WIDTH); // 装载接收端地址
	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH);     		// 装载数据
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);      						// IRQ收发完成中断响应，16位CRC，主发送
	CE=1;   			//置高CE，激发数据发送
	inerDelay_us(10);
}


///******* 中断初始化  ******/
//void init_com(void)
//{
//    TMOD=0x20;						//定时器1,8位自动重
//	TH1=0xfd;
//	TL1=0xfd;
//	TR1=1;
//	REN=1;							//允许串口中断
//	SM0=0;							//设置串口工作方式为1
//	SM1=1;
//	EA=1;							//开总中断
//	ES=1;	
//}							
//void SendByte(unsigned char dat)	   //发送一个字节的数据
//{
//   SBUF = dat;
//   while(!TI);
//      TI = 0;
//}
//void SendStr(unsigned char *s)			//发送一个字符串的数据
//{
//   while(*s!='\0')						// \0 表示字符串结束标志，
//                						//通过检测是否字符串末尾
//   {
//      SendByte(*s);
//      s++;
//   }
//}
void uart1(void)
{
	P0M0 = 0x00;
    P0M1 = 0x00;
    P1M0 = 0x00;
    P1M1 = 0x00;
    P2M0 = 0x00;
    P2M1 = 0x00;
    P3M0 = 0x00;
    P3M1 = 0x00;
    P4M0 = 0x00;
    P4M1 = 0x00;
    P5M0 = 0x00;
    P5M1 = 0x00;
    P6M0 = 0x00;
    P6M1 = 0x00;
    P7M0 = 0x00;
    P7M1 = 0x00;

    ACC = P_SW1;
    ACC &= ~(S1_S0 | S1_S1);    //S1_S0=0 S1_S1=0
    P_SW1 = ACC;                //(P3.0/RxD, P3.1/TxD)

	#if (PARITYBIT == NONE_PARITY)
    SCON = 0x50;                //8位可变波特率
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    SCON = 0xda;                //9位可变波特率,校验位初始为1
#elif (PARITYBIT == SPACE_PARITY)
    SCON = 0xd2;                //9位可变波特率,校验位初始为0
#endif

    AUXR = 0x40;                //定时器1为1T模式
    TMOD = 0x00;                //定时器1为模式0(16位自动重载)
    TL1 = (65536 - (FOSC/4/BAUD));   //设置波特率重装值
    TH1 = (65536 - (FOSC/4/BAUD))>>8;
    TR1 = 1;                    //定时器1开始启动
    ES = 1;                     //使能串口中断
    EA = 1;
}
 /********************************************/
/* 函数功能：检测24L01是否存在 */
/* 返回值； 1 存在 */
/* 2 不存在 */
/********************************************/
uchar NRF24L01_Check(void)
{
int NRF_CE=0;
uchar check_in_buf[5]={0x11,0x22,0x33,0x44,0x55};
uchar check_out_buf[5]={0x00};
//SendStr("check Init\r\n");


SPI_Write_Buf(WRITE_REG+TX_ADDR, check_in_buf, 5);

SPI_Read_Buf(READ_REG+TX_ADDR, check_out_buf, 5);

if((check_out_buf[0] == 0x11)&&\
(check_out_buf[1] == 0x22)&&\
(check_out_buf[2] == 0x33)&&\
(check_out_buf[3] == 0x44)&&\
(check_out_buf[4] == 0x55))return 1;
else return 2;
}
//************************************主函数************************************************************
void main(void)
{
	uchar a;  //NRF模块是否存在标志位
	uchar i,c;
	uchar RxBuf[2];				  //定义接收数据的数组
	init_NRF24L01();				  //NRF初始化
	uart1();
	Init_T4();       //定时器初始化
	EA = 1;           //打开全局中断
	OLED_Init();			//初始化OLED  
	OLED_Clear(); 
	
	
	a=NRF24L01_Check();//检测NRF标志位，NRF存在led1和led3亮，否则led1,2亮。
	if(a == 1){
				led0 = 0;
				led3 =0;
			}
	else if(a == 2){
				led1=0;
				led2=0;			
			}
	else{
				led0=0;led1=0;led2=0;led3=0;
	    }
	Delay(6000);
	
	
	
	while(1)
	{
		uchar temp[]={"002"};
		unsigned char TxBuf[20]={0x37,0x38};    //将要发送的数据放在数组中
		init_NRF24L01();
	  SetRX_Mode();   //无按键按下为接受模式
		//SetTX_Mode();   //有按键按下为发送模式
		nRF24L01_RxPacket(RxBuf);
		
		OLED_ShowCHinese(0,0,0);//临
		OLED_ShowCHinese(18,0,1);//大
		OLED_ShowCHinese(36,0,2);//物
		OLED_ShowCHinese(54,0,3);//联
		OLED_ShowCHinese(72,0,4);//实
		OLED_ShowCHinese(90,0,5);//验
		OLED_ShowCHinese(108,0,6);//室
		
		/*OLED_ShowCHinese(0,3,11);//温
		OLED_ShowCHinese(18,3,17);//湿
		OLED_ShowCHinese(36,3,12);//度
		OLED_ShowCHinese(54,3,13);//采
		OLED_ShowCHinese(72,3,14);//集
		OLED_ShowCHinese(90,3,15);//节
		OLED_ShowCHinese(108,3,16);//点
		OLED_ShowString(20,6,temp);
		*/
		OLED_ShowCHinese(0,3,13);//住
		OLED_ShowCHinese(18,3,14);//户
		OLED_ShowString(36,3,temp);//002
		if(flag1 == 1)
		{
			flag1 =0;   //复位
			speak = !speak;  //蜂鸣器按下响，在按下不响
			init_NRF24L01_Send();
		  SetTX_Mode();   //有按键按下为发送模式
			nRF24L01_TxPacket(TxBuf);			 	//发送数据
			//P0=SPI_Read(STATUS);				 	//读状态寄存器的值  如果数据成功发送，那么STATUS的值应该为0x2e
			SPI_RW_Reg(WRITE_REG+STATUS,0XFF);   	//清状态寄存器
			Delay(6000);
			speak =1;               //关闭蜂鸣器
		}
		
		
		
//三种串口发送方式
		if(IRQ == 0)	//单片机通过观察它的IRQ引脚，就可以知道是否接收到了数据，IRQ为低电平，说明接收到了数据
		{
			OLED_ShowCHinese(36,6,15);//开
			OLED_ShowCHinese(54,6,16);//锁
			//OLED_ShowString(20,6,RxBuf);
			nRF24L01_RxPacket(RxBuf);
			Delay(600);
			 for(i = 0;i < 2;i++)				//通过串口读出数据
			 	SendData(RxBuf[i]);
			Delay(6000);
		}
		else
		{
			Delay(60);
		}
		/*
		/////////////////////////////////////////////////////////////////////////////////
	    if(nRF24L01_RxPacket(RxBuf))			//若返回 1  说明数据接收成功
		{
		 //    led2= 0;
			 Delay(600);
			 for(i = 0;i < 2;i++)				//通过串口读出数据
			 	SendData(RxBuf[i]);	
//			 SendString(RxBuf[20]);
		}
		else
		{
				//led2 = 1;
				Delay(60);
		}
		
/////////////////////////////////////////////////////////////////////////////////////////
		nRF24L01_RxPacket(RxBuf);				//接收数据
		c = SPI_Read(STATUS);				 	//通过读取STATUS的值来判断是哪个事件触发了中断
		if(c == 0x40)                      		//查看是否接收成功	   如果数据成功接收，那么STATUS的值应该为0x40
		{
		   
			 nRF24L01_RxPacket(RxBuf);
			 Delay(600);
			 for(i = 0;i < 2;i++)				//通过串口读出数据
			 	SendData(RxBuf[i]);
			 Delay(6000);     
		}
		else
		{
			 
			 Delay(60);
		}*/
	}
	
	
}


/*----------------------------
UART 中断服务程序
-----------------------------*/
void Uart() interrupt 4 using 1
{
    if (RI)
    {
        RI = 0;                 //清除RI位
        P0 = SBUF;              //P0显示串口数据
        P22 = RB8;              //P2.2显示校验位
    }
    if (TI)
    {
        TI = 0;                 //清除TI位
        busy = 0;               //清忙标志
    }
}

/*----------------------------
发送串口数据
----------------------------*/
void SendData(BYTE dat)
{
    while (busy);               //等待前面的数据发送完成
    ACC = dat;                  //获取校验位P (PSW.0)
    if (P)                      //根据P来设置校验位
    {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 0;                //设置校验位为0
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 1;                //设置校验位为1
#endif
    }
    else
    {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 1;                //设置校验位为1
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 0;                //设置校验位为0
#endif
    }
    busy = 1;
    SBUF = ACC;                 //写数据到UART数据寄存器
}

/*----------------------------
发送字符串
----------------------------*/
void SendString(char *s)
{
    while (*s)                  //检测字符串结束标志
    {
        SendData(*s++);         //发送当前字符
    }
}