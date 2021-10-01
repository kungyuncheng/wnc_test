//---------------------------------------------------------------------------

#ifndef UartH
#define UartH

#include <windows.h>
#include <stdio.h>

#define BITS_OF_BYTE                	 8

//---------------------------------------------------------------------------
typedef struct WriteComInfo
{
	unsigned int timeOut ;        // 全部動作之timeout, 一個動作可能有好幾個package
	unsigned int writeBufSize ;
	unsigned int resBufSize ;
	unsigned int devType ;
	unsigned int pktType ;
	unsigned int g06maErase4KackCount ;   // just for g06ma
	HANDLE comPort ;
	unsigned char *writeBuf ;
    unsigned char *resBuf ;
} WriteComInfo ;

//---------------------------------------------------------------------------
enum BaudRate
{
	BAUD_RATE_110 =                  110,
	BAUD_RATE_300  =                 300,
	BAUD_RATE_1200 =                 1200,
	BAUD_RATE_2400 =                 2400,
	BAUD_RATE_4800 =                 4800,
	BAUD_RATE_9600 =                 9600,
    BAUD_RATE_14400 =                14400,
	BAUD_RATE_19200 =                19200,
	BAUD_RATE_38400 =                38400,
	BAUD_RATE_57600 =                57600,
	BAUD_RATE_115200 =               115200,
	BAUD_RATE_230400 =               230400,
	BAUD_RATE_460800 =               460800,
	BAUD_RATE_921600 =               921600
} ;

enum PktOption
{
	NORMAL_PACKAGE_OFFSET,                   // 0
	NO_ACK_OFFSET,
	MTK_NORMAL_PACKAGE_OFFSET,
	MTK_NEED_ACK_PACKAGE_OFFSET,
	RECEIVE_DATA_OFFSET,
	CHECK_RMC_OFFSET,                        // 5
	CHECK_SNR_OFFSET,
	CHECK_SINGLE_CHANNEL_OFFSET,
	CHECK_ACQ_OFFSET,
	UPDATE_SV_OFFSET,
	OSP_NORMAL_PACKAGE_OFFSET,               // 10
	NMEA_CHANGE_OSP_OFFSET,
	OSP_COLDSTART_PACKAGE_OFFSET,
	G06MA_ERASE_4K_OFFSET,
	NORMAL_PACKAGE	= 				(1 << NORMAL_PACKAGE_OFFSET),
	NO_ACK = 						(1 << NO_ACK_OFFSET),
	MTK_NORMAL_PACKAGE =            (1 << MTK_NORMAL_PACKAGE_OFFSET),
	MTK_NEED_ACK_PACKAGE =          (1 << MTK_NEED_ACK_PACKAGE_OFFSET),
	RECEIVE_DATA =                  (1 << RECEIVE_DATA_OFFSET),
	CHECK_RMC =                     (1 << CHECK_RMC_OFFSET),
	CHECK_SNR =                     (1 << CHECK_SNR_OFFSET),
	CHECK_SINGLE_CHANNEL =          (1 << CHECK_SINGLE_CHANNEL_OFFSET),
    CHECK_ACQ =                     (1 << CHECK_ACQ_OFFSET),
	UPDATE_SV =                     (1 << UPDATE_SV_OFFSET),
	OSP_NORMAL_PACKAGE =            (1 << OSP_NORMAL_PACKAGE_OFFSET),
	NMEA_CHANGE_OSP =               (1 << NMEA_CHANGE_OSP_OFFSET),
	OSP_COLDSTART_PACKAGE =         (1 << OSP_COLDSTART_PACKAGE_OFFSET),
	G06MA_ERASE_4K =                (1 << G06MA_ERASE_4K_OFFSET),
} ;

enum ResTimeOut
{
	DFU_BUFFER_TIME =             10000,  // DFU start package 到 init package之間的delay時間
	MTK_CHANGE_BAUDRATE_TIME =    2000,
	RECEIVE_DATA_TIME_OUT =       200,
	TIME_OUT =              	  2000,   // 2 s
	READ_SECTION_DATA_TIME_OUT =  3500,   // 770 baudrate 921600 時
	ERASE_SECTION_TIME_OUT =      1000,    // 770
	CONVERT_COURSE_TIME_OUT =     90000,  // 90 s
	ERASE_FLASH_TIME_OUT =        60000,  // 60 s
	OSP_BUFFER_TIME =             500
} ;

enum BufferSize
{
	BASE_DATA_SIZE =        		    0x40,
	READ_DATA_SIZE =                    0x200,
	WRITE_DATA_BUFFER_SIZE =            4 * BASE_DATA_SIZE,
	RESPONSE_DATA_BUFFER_SIZE =         128 * BASE_DATA_SIZE,
	READ_DATA_BUFFER_SIZE =             32 * BASE_DATA_SIZE,
	EXPECT_DATA_BUFFER_SIZE =           4 * BASE_DATA_SIZE,
	AP_BLOCK_SIZE =				        0x10000,
    MCU_BLOCK_SIZE =				    0x100
} ;

typedef void (*UpdateProcessFun)(void *, int, int) ;    //  void *processBar, int nowProcess, int maxProcess
typedef int (*JudgePktFun)(WriteComInfo *, void *, FILE *, void *, UpdateProcessFun) ;

//---------------------------------------------------------------------------
int WriteToCom(WriteComInfo *info, void *ackInfo, FILE *dwFp, JudgePktFun callBackJudgePktFun, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ReadFromCom(WriteComInfo *info, void *ackInfo, FILE *dwFp, OVERLAPPED *overLapped, JudgePktFun callBackJudgePktFun, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;

#endif
