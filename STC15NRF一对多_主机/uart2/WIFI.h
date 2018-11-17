#ifndef __8266_H_
#define __8266_H_
typedef unsigned char BYTE;
typedef unsigned int WORD;

//unsigned char config[]={"AT+MCONFIG=\"869300031657095\",\"5qx9dzUzHMvls4e9kMAO0010066c00\",\"355d19eef2104fb5bb28f8caea0b8b3a\"\x00D\x00A"};
//-------------以下为功能定义------------------------------------------------------------
//BYTE code  WiFi_AT[];	                			//格式化命令。
//BYTE code  WiFi_CWMODE[];	          				//获取版本信息
//BYTE code  WiFi_CWJAP[];	           				//1，附着网络；0未附着网络
//BYTE code  WiFi_CIFSR[];	                  //AT+SAPBR，IP设置，CONTYPE因特网连接，GPRS：gprs连接
//BYTE code  WiFi_CIPSTART[];	                //AT+SAPBR，IP设置，CONTYPE因特网连接，GPRS：gprs连接
//BYTE code  WiFi_CIPMODE[];	                //AT+SAPBR，IP设置，CONTYPE因特网连接，GPRS：gprs连接
//BYTE code  WiFi_CIPSEND[];               		//air202启用HEX MQTT模式
//BYTE code  WiFi_CONNECT[];               		//air202启用HEX MQTT模式
//BYTE    WiFi_UPDATA[]; 



#endif
