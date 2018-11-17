//ʵ�龧��λ11.0592MHz��������λ9600
//NRF�ӻ�2
#include "stc15w4k.h"
#include "T4_Time.h"
#include "oled.h"
#include "T4_Time.h"
#include <intrins.h>
typedef unsigned char uchar;
typedef unsigned char uint;

/******************************STC15����1��ʱ��1ģʽ0����************************************/
typedef unsigned char BYTE;
typedef unsigned int WORD;

#define FOSC 11059200L          //ϵͳƵ��
#define BAUD 9600             //���ڲ�����

#define NONE_PARITY     0       //��У��
#define ODD_PARITY      1       //��У��
#define EVEN_PARITY     2       //żУ��
#define MARK_PARITY     3       //���У��
#define SPACE_PARITY    4       //�հ�У��

#define PARITYBIT EVEN_PARITY   //����У��λ

#define S1_S0 0x40              //P_SW1.6
#define S1_S1 0x80              //P_SW1.7
bit busy;

void SendData(BYTE dat);
void SendString(char *s);

//****************************************IO�˿ڶ���***************************************

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
uint const TX_ADDRESS_2[TX_ADR_WIDTH]= {0x10,0x10,0x10,0x10,0x20}; //���ص�ַ�ӻ�2	  0x10,0x10,0x10,0x10,0x20
uint const RX_ADDRESS[RX_ADR_WIDTH]= {0x01,0x01,0x01,0x01,0x01}; //����������ַ	  0x01,0x01,0x01,0x01,0x01
//***************************************NRF24L01�Ĵ���ָ��*******************************************************
#define READ_REG        0x00   //�����üĴ���,��5λΪ�Ĵ�����ַ
#define WRITE_REG       0x20  //д���üĴ���,��5λΪ�Ĵ�����ַ
#define RD_RX_PLOAD     0x61   // ��ȡ��������ָ��	   ��RX��Ч����,1~32�ֽ�
#define WR_TX_PLOAD     0xA0   // д��������ָ��	   дTX��Ч����,1~32�ֽ�
#define FLUSH_TX        0xE1  // ��ϴ���� FIFOָ��	   ���TX FIFO�Ĵ���.����ģʽ����
#define FLUSH_RX        0xE2   // ��ϴ���� FIFOָ��	   ���RX FIFO�Ĵ���.����ģʽ����
#define REUSE_TX_PL     0xE3   // �����ظ�װ������ָ��	����ʹ����һ������,CEΪ��,���ݰ������Ϸ���.
#define NOP             0xFF   // ����				   �ղ���,����������״̬�Ĵ���
//*************************************SPI(nRF24L01)�Ĵ�����ַ****************************************************
#define CONFIG          0x00  // �����շ�״̬��CRCУ��ģʽ�Լ��շ�״̬��Ӧ��ʽ
#define EN_AA           0x01  // �Զ�Ӧ��������
#define EN_RXADDR       0x02  // �����ŵ�����
#define SETUP_AW        0x03  // �շ���ַ�������
#define SETUP_RETR      0x04  // �Զ��ط���������
#define RF_CH           0x05  // ����Ƶ������
#define RF_SETUP        0x06  // �������ʡ����Ĺ�������	  RF�Ĵ���
#define STATUS          0x07  // ״̬�Ĵ���
#define OBSERVE_TX      0x08  // ���ͼ�⹦��
#define CD              0x09  // ��ַ���           	 �ز����Ĵ���
#define RX_ADDR_P0      0x0A  // Ƶ��0�������ݵ�ַ
#define RX_ADDR_P1      0x0B  // Ƶ��1�������ݵ�ַ
#define RX_ADDR_P2      0x0C  // Ƶ��2�������ݵ�ַ
#define RX_ADDR_P3      0x0D  // Ƶ��3�������ݵ�ַ
#define RX_ADDR_P4      0x0E  // Ƶ��4�������ݵ�ַ
#define RX_ADDR_P5      0x0F  // Ƶ��5�������ݵ�ַ
#define TX_ADDR         0x10  // ���͵�ַ�Ĵ���
#define RX_PW_P0        0x11  // ����Ƶ��0�������ݳ���	   (1~32�ֽ�)
#define RX_PW_P1        0x12  // ����Ƶ��0�������ݳ���
#define RX_PW_P2        0x13  // ����Ƶ��0�������ݳ���
#define RX_PW_P3        0x14  // ����Ƶ��0�������ݳ���
#define RX_PW_P4        0x15  // ����Ƶ��0�������ݳ���
#define RX_PW_P5        0x16  // ����Ƶ��0�������ݳ���
#define FIFO_STATUS     0x17  // FIFOջ��ջ��״̬�Ĵ�������
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

//*****************************************����ʱ*****************************************
void Delay(unsigned int s)
{
	unsigned int i;
	for(i=0; i<s; i++);
	for(i=0; i<s; i++);
}
//******************************************************************************************
uint  bdata sta;   //״̬��־
sbit RX_DR =sta^6;		 //�ж��Ƿ���յ�����	  ���յ����� ����1
sbit TX_DS =sta^5;
sbit MAX_RT =sta^4;
/******************************************************************************************
/*��ʱ����
/******************************************************************************************/
void inerDelay_us(unsigned char n)
{
	for(;n>0;n--)
    _nop_();
}
//****************************************************************************************
/*NRF24L01��ʼ��
//***************************************************************************************/
void init_NRF24L01(void)
{
    inerDelay_us(100);
    CE=0;    // chip enable
    CSN=1;   // Spi disable
    SCK=0;   // Spi clock line init high
	SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS_2, TX_ADR_WIDTH);    // д���ص�ַ
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); // д���ն˵�ַ

	SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);      //  Ƶ��0�Զ� ACKӦ�������ֹ
	SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);  //  ������յ�ַֻ��Ƶ��0�������Ҫ��Ƶ�����Բο�Page21  
	
	SPI_RW_Reg(WRITE_REG + SETUP_AW, 0x02); // Setup address width=5 bytes	  ��װ��ַ��ȣ�5�ֽ�
	SPI_RW_Reg(WRITE_REG + RF_CH, 40);        //   �����ŵ�����Ϊ2.4GHZ���շ�����һ��
	SPI_RW_Reg(WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH); //���ý������ݳ��ȣ���������Ϊ32�ֽ�
	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);     //���÷�������Ϊ1MHZ�����书��Ϊ���ֵ0dB
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);     // IRQ�շ�����ж���Ӧ��16λCRC ��������
}
//****************************************************************************************
/*NRF24L01���ͳ�ʼ��
//***************************************************************************************/
void init_NRF24L01_Send(void)
{
   /*
		����nrf24l01ͨ�ţ���Ҫ����3��������ͬ��
		1.Ƶ����ͬ������Ƶ���Ĵ���RF_CH��
		2.��ַ��ͬ������TX_ADDR��RX_ADDR_P0��ͬ��
		3.ÿ�η��ͽ��յ��ֽ�����ͬ�����������ͨ������Ч���ݿ��Ϊn����ôÿ�η��͵��ֽ���Ҳ����Ϊn����Ȼ��n<=32��
	*/
    inerDelay_us(100);
    CE=0;    // chip enable  оƬʹ��
    CSN=1;   // Spi disable  SPI����
    SCK=0;   // Spi clock line init high   SPIʱ����
	  IRQ=1;
	  SPI_Write_Buf(WRITE_REG + TX_ADDR, TX_ADDRESS_2, TX_ADR_WIDTH);    // д���ص�ַ     ����մ���Ľ��ն˵�ַ��ͬ
	  SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH); // д���ն˵�ַ
	
	//�����������������ã����ͷ������̾ͱ���˷���-�����жϡ��������׿��˽��շ�������ר��ȥ���Է���
//	SPI_RW_Reg(WRITE_REG + EN_AA, 0x00);                 // ʧ��ͨ��0�Զ�Ӧ��
//  SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x00);            // ʧ�ܽ���ͨ��0
//	SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x00);          // ʧ���Զ��ط�
	
	//��������ֻ�ǵ��Է��������յĲ�Ʒ���������Ӧ����ط��Ļ���ô���ݵ��ȶ����Ǻ��ѱ�֤�ģ������ڻ�����ͨѶ����֮���Ҫ�ѷ��͵����ø�Ϊ�������������� 
	//�������ͺͽ��վͽ�����һ����׼״̬������-��Ӧ��-���Զ��ط���-�����жϣ�����-Ӧ��-�����жϣ�һ�а����Ͱ࣬����������Լ���Ӧ�ò��־���ʵ�ֺܶ๦����
	  SPI_RW_Reg(WRITE_REG + EN_AA, 0x01);               // ʹ�ܽ���ͨ��0�Զ�Ӧ��
    SPI_RW_Reg(WRITE_REG + EN_RXADDR, 0x01);           // ʹ�ܽ���ͨ��0               
    SPI_RW_Reg(WRITE_REG + SETUP_RETR, 0x1a);          // �Զ��ط�10�Σ����500us
	
  	SPI_RW_Reg(WRITE_REG + SETUP_AW, 0x02); // Setup address width=5 bytes  ��װ��ַ��ȣ�5�ֽ�	  
  	SPI_RW_Reg(WRITE_REG + RF_CH, 0);        			//�����ŵ�����Ϊ2.4GHZ���շ�����һ��
  	SPI_RW_Reg(WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH); 	//���ý������ݳ��ȣ���������Ϊ32�ֽ�
  	SPI_RW_Reg(WRITE_REG + RF_SETUP, 0x07);     		//���÷�������Ϊ2MHZ�����书��Ϊ���ֵ0dB
  	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);      			// IRQ�շ�����ж���Ӧ��16λCRC������ģʽ
}
/****************************************************************************************************
/*������uint SPI_RW(uint uchar)
/*���ܣ�NRF24L01��SPIдʱ��
/****************************************************************************************************/
uint SPI_RW(uint uchar)
{
uint bit_ctr;
    for(bit_ctr=0;bit_ctr<8;bit_ctr++) // output 8-bit
    {
		  MOSI = (uchar & 0x80);         // output 'uchar', MSB to MOSI		�����uchar����MSB��MOSI
		  uchar = (uchar << 1);           // shift next bit into MSB..		����һλ�Ƶ�MSB
		  SCK = 1;                      // Set SCK high..					����SCK�ߡ�
		  uchar |= MISO;           // capture current MISO bit				��ȡ��ǰ MISO bit
		  SCK = 0;                // ..then set SCK low again				Ȼ���ٴ�����SCK�͵�ƽ
    }
    return(uchar);               // return read uchar						���ض�ȡuchar
}
/****************************************************************************************************
/*������uchar SPI_Read(uchar reg)
/*���ܣ�NRF24L01��SPIʱ��
/****************************************************************************************************/
uchar SPI_Read(uchar reg)
{
	uchar reg_val;
	
	CSN = 0;                // CSN low, initialize SPI communication...	CSN�ͣ���ʼ��SPIͨ��
	SPI_RW(reg);            // Select register to read from..			д�Ĵ�����ַ  ѡ��Ĵ�����ȡ
	reg_val = SPI_RW(0);    // ..then read registervalue				Ȼ���ȡע��ֵ д����Ĵ���ָ��
	CSN = 1;                // CSN high, terminate SPI communication	CSN�ߣ���ֹSPIͨ��
	
	return(reg_val);        // return register value					���ؼĴ���ֵ
}
/****************************************************************************************************/
/*���ܣ�NRF24L01��д�Ĵ�������
/****************************************************************************************************/
uint SPI_RW_Reg(uchar reg, uchar value)
{
	uint status;
	
	CSN = 0;                   // CSN low, init SPI transaction	  CSN�õ� ����SPIͨ��
	status = SPI_RW(reg);      // select register				  ѡ��Ĵ���
	SPI_RW(value);             // ..and write value to it..		  ������д��ֵ
	CSN = 1;                   // CSN high again				  CSN�ٸ�																  
	return(status);            // return nRF24L01 status uchar	  ����nRF24L01״̬uchar
}
/****************************************************************************************************/
/*������uint SPI_Read_Buf(uchar reg, uchar *pBuf, uchar uchars)
/*����: ���ڶ����ݣ�reg��Ϊ�Ĵ�����ַ��pBuf��Ϊ���������ݵ�ַ��uchars���������ݵĸ���
/****************************************************************************************************/
uint SPI_Read_Buf(uchar reg, uchar *pBuf, uchar uchars)
{
	uint status,uchar_ctr;
	
	CSN = 0;                      // Set CSN low, init SPI tranaction	 CSN�õ� ����SPIͨ��
	status = SPI_RW(reg);         // Select register to write to and read status uchar ѡ��Ĵ���д��Ͷ�ȡ״̬uCHAR  д��Ҫ��ȡ�ļĴ�����ַ
	
	for(uchar_ctr=0;uchar_ctr<uchars;uchar_ctr++)
	  pBuf[uchar_ctr] = SPI_RW(0);    //
	
	CSN = 1;                           
	
	return(status);                    // return nRF24L01 status uchar
}
/*********************************************************************************************************
/*������uint SPI_Write_Buf(uchar reg, uchar *pBuf, uchar uchars)
/*����: ����д���ݣ�Ϊ�Ĵ�����ַ��pBuf��Ϊ��д�����ݵ�ַ��uchars��д�����ݵĸ���
/*********************************************************************************************************/
uint SPI_Write_Buf(uchar reg, uchar *pBuf, uchar uchars)
{
	uint status,uchar_ctr;
	
	CSN = 0;            //SPIʹ��      
	status = SPI_RW(reg);   
	for(uchar_ctr=0; uchar_ctr<uchars; uchar_ctr++) //
	  SPI_RW(*pBuf++);
	CSN = 1;           //�ر�SPI
	return(status);    //
}
/****************************************************************************************************/
/*������void SetRX_Mode(void)
/*���ܣ����ݽ�������
/****************************************************************************************************/
void SetRX_Mode(void)
{
	CE=0;
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);     // IRQ�շ�����ж���Ӧ��16λCRC ��������
	CE = 1;
	inerDelay_us(130);
}
/****************************************************************************************************/
/*������void SetTX_Mode(void)
/*���ܣ����ݷ�������
/****************************************************************************************************/
void SetTX_Mode(void)
{
	CE=0;
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0f);     // IRQ�շ�����ж���Ӧ��16λCRC ��������
	CE = 1;
	inerDelay_us(200);
}
/******************************************************************************************************/
/*������unsigned char nRF24L01_RxPacket(unsigned char* rx_buf)
/*���ܣ����ݶ�ȡ�����rx_buf���ջ�������
/******************************************************************************************************/
unsigned char nRF24L01_RxPacket(unsigned char* rx_buf)
{
    unsigned char revale=0;
	sta=SPI_Read(STATUS); 	// ��ȡ״̬�Ĵ������ж����ݽ���״��
	if(RX_DR)    			// �ж��Ƿ���յ�����
	{
	     CE = 0;    		//SPIʹ��
		 SPI_Read_Buf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH);// read receive payload from RX_FIFO buffer	��RXXFIFO��������ȡ������Ч�غ�
		 revale =1;   		//��ȡ������ɱ�־
	}
	SPI_RW_Reg(WRITE_REG+STATUS,sta);   				//���յ����ݺ�RX_DR,TX_DS,MAX_PT���ø�Ϊ1��ͨ��д1������жϱ�־
	return revale;
}
/***********************************************************************************************************
/*������void nRF24L01_TxPacket(unsigned char * tx_buf)
/*���ܣ����� tx_buf������
/**********************************************************************************************************/

void nRF24L01_TxPacket(unsigned char * tx_buf)
{
	CE=0;   			//StandBy Iģʽ
	SPI_Write_Buf(WRITE_REG + RX_ADDR_P0, TX_ADDRESS_2, RX_ADR_WIDTH); // װ�ؽ��ն˵�ַ
	SPI_Write_Buf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH);     		// װ������
	SPI_RW_Reg(WRITE_REG + CONFIG, 0x0e);      						// IRQ�շ�����ж���Ӧ��16λCRC��������
	CE=1;   			//�ø�CE���������ݷ���
	inerDelay_us(10);
}


///******* �жϳ�ʼ��  ******/
//void init_com(void)
//{
//    TMOD=0x20;						//��ʱ��1,8λ�Զ���
//	TH1=0xfd;
//	TL1=0xfd;
//	TR1=1;
//	REN=1;							//�������ж�
//	SM0=0;							//���ô��ڹ�����ʽΪ1
//	SM1=1;
//	EA=1;							//�����ж�
//	ES=1;	
//}							
//void SendByte(unsigned char dat)	   //����һ���ֽڵ�����
//{
//   SBUF = dat;
//   while(!TI);
//      TI = 0;
//}
//void SendStr(unsigned char *s)			//����һ���ַ���������
//{
//   while(*s!='\0')						// \0 ��ʾ�ַ���������־��
//                						//ͨ������Ƿ��ַ���ĩβ
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
    SCON = 0x50;                //8λ�ɱ䲨����
#elif (PARITYBIT == ODD_PARITY) || (PARITYBIT == EVEN_PARITY) || (PARITYBIT == MARK_PARITY)
    SCON = 0xda;                //9λ�ɱ䲨����,У��λ��ʼΪ1
#elif (PARITYBIT == SPACE_PARITY)
    SCON = 0xd2;                //9λ�ɱ䲨����,У��λ��ʼΪ0
#endif

    AUXR = 0x40;                //��ʱ��1Ϊ1Tģʽ
    TMOD = 0x00;                //��ʱ��1Ϊģʽ0(16λ�Զ�����)
    TL1 = (65536 - (FOSC/4/BAUD));   //���ò�������װֵ
    TH1 = (65536 - (FOSC/4/BAUD))>>8;
    TR1 = 1;                    //��ʱ��1��ʼ����
    ES = 1;                     //ʹ�ܴ����ж�
    EA = 1;
}
 /********************************************/
/* �������ܣ����24L01�Ƿ���� */
/* ����ֵ�� 1 ���� */
/* 2 ������ */
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
//************************************������************************************************************
void main(void)
{
	uchar a;  //NRFģ���Ƿ���ڱ�־λ
	uchar i,c;
	uchar RxBuf[2];				  //����������ݵ�����
	init_NRF24L01();				  //NRF��ʼ��
	uart1();
	Init_T4();       //��ʱ����ʼ��
	EA = 1;           //��ȫ���ж�
	OLED_Init();			//��ʼ��OLED  
	OLED_Clear(); 
	
	
	a=NRF24L01_Check();//���NRF��־λ��NRF����led1��led3��������led1,2����
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
		unsigned char TxBuf[20]={0x37,0x38};    //��Ҫ���͵����ݷ���������
		init_NRF24L01();
	  SetRX_Mode();   //�ް�������Ϊ����ģʽ
		//SetTX_Mode();   //�а�������Ϊ����ģʽ
		nRF24L01_RxPacket(RxBuf);
		
		OLED_ShowCHinese(0,0,0);//��
		OLED_ShowCHinese(18,0,1);//��
		OLED_ShowCHinese(36,0,2);//��
		OLED_ShowCHinese(54,0,3);//��
		OLED_ShowCHinese(72,0,4);//ʵ
		OLED_ShowCHinese(90,0,5);//��
		OLED_ShowCHinese(108,0,6);//��
		
		/*OLED_ShowCHinese(0,3,11);//��
		OLED_ShowCHinese(18,3,17);//ʪ
		OLED_ShowCHinese(36,3,12);//��
		OLED_ShowCHinese(54,3,13);//��
		OLED_ShowCHinese(72,3,14);//��
		OLED_ShowCHinese(90,3,15);//��
		OLED_ShowCHinese(108,3,16);//��
		OLED_ShowString(20,6,temp);
		*/
		OLED_ShowCHinese(0,3,13);//ס
		OLED_ShowCHinese(18,3,14);//��
		OLED_ShowString(36,3,temp);//002
		if(flag1 == 1)
		{
			flag1 =0;   //��λ
			speak = !speak;  //�����������죬�ڰ��²���
			init_NRF24L01_Send();
		  SetTX_Mode();   //�а�������Ϊ����ģʽ
			nRF24L01_TxPacket(TxBuf);			 	//��������
			//P0=SPI_Read(STATUS);				 	//��״̬�Ĵ�����ֵ  ������ݳɹ����ͣ���ôSTATUS��ֵӦ��Ϊ0x2e
			SPI_RW_Reg(WRITE_REG+STATUS,0XFF);   	//��״̬�Ĵ���
			Delay(6000);
			speak =1;               //�رշ�����
		}
		
		
		
//���ִ��ڷ��ͷ�ʽ
		if(IRQ == 0)	//��Ƭ��ͨ���۲�����IRQ���ţ��Ϳ���֪���Ƿ���յ������ݣ�IRQΪ�͵�ƽ��˵�����յ�������
		{
			OLED_ShowCHinese(36,6,15);//��
			OLED_ShowCHinese(54,6,16);//��
			//OLED_ShowString(20,6,RxBuf);
			nRF24L01_RxPacket(RxBuf);
			Delay(600);
			 for(i = 0;i < 2;i++)				//ͨ�����ڶ�������
			 	SendData(RxBuf[i]);
			Delay(6000);
		}
		else
		{
			Delay(60);
		}
		/*
		/////////////////////////////////////////////////////////////////////////////////
	    if(nRF24L01_RxPacket(RxBuf))			//������ 1  ˵�����ݽ��ճɹ�
		{
		 //    led2= 0;
			 Delay(600);
			 for(i = 0;i < 2;i++)				//ͨ�����ڶ�������
			 	SendData(RxBuf[i]);	
//			 SendString(RxBuf[20]);
		}
		else
		{
				//led2 = 1;
				Delay(60);
		}
		
/////////////////////////////////////////////////////////////////////////////////////////
		nRF24L01_RxPacket(RxBuf);				//��������
		c = SPI_Read(STATUS);				 	//ͨ����ȡSTATUS��ֵ���ж����ĸ��¼��������ж�
		if(c == 0x40)                      		//�鿴�Ƿ���ճɹ�	   ������ݳɹ����գ���ôSTATUS��ֵӦ��Ϊ0x40
		{
		   
			 nRF24L01_RxPacket(RxBuf);
			 Delay(600);
			 for(i = 0;i < 2;i++)				//ͨ�����ڶ�������
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
UART �жϷ������
-----------------------------*/
void Uart() interrupt 4 using 1
{
    if (RI)
    {
        RI = 0;                 //���RIλ
        P0 = SBUF;              //P0��ʾ��������
        P22 = RB8;              //P2.2��ʾУ��λ
    }
    if (TI)
    {
        TI = 0;                 //���TIλ
        busy = 0;               //��æ��־
    }
}

/*----------------------------
���ʹ�������
----------------------------*/
void SendData(BYTE dat)
{
    while (busy);               //�ȴ�ǰ������ݷ������
    ACC = dat;                  //��ȡУ��λP (PSW.0)
    if (P)                      //����P������У��λ
    {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 0;                //����У��λΪ0
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 1;                //����У��λΪ1
#endif
    }
    else
    {
#if (PARITYBIT == ODD_PARITY)
        TB8 = 1;                //����У��λΪ1
#elif (PARITYBIT == EVEN_PARITY)
        TB8 = 0;                //����У��λΪ0
#endif
    }
    busy = 1;
    SBUF = ACC;                 //д���ݵ�UART���ݼĴ���
}

/*----------------------------
�����ַ���
----------------------------*/
void SendString(char *s)
{
    while (*s)                  //����ַ���������־
    {
        SendData(*s++);         //���͵�ǰ�ַ�
    }
}