//---------------------------------------------------------------------------

#ifndef DFUPackageH
#define DFUPackageH
//---------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include "Uart.h"

#define DFU_SDK_VERSION		    	   800
#define DFU_FIRMWARE_ID_V520       	0x002c
#define DFU_FIRMWARE_ID_V521       	0x0043
#define DFU_FIRMWARE_ID_V700       	0x004f
#define DFU_FIRMWARE_ID_V710        0x005a
#define DFU_FIRMWARE_ID_V800      	0x0064

#define DEVICE_BAUDRATE_DFU_BOOT	 38400

#define DFU_PACKAGE_SIZE			   512
#define DFU_ACK_PACKAGE_SIZE			 6

//---------------------------------------------------------------------------
enum DFUPktHeader
{
	SEQ_NUMBER_OFFSET =                0,
	ACK_NUMBER_OFFSET =                3,
	INTEGRETY_CHECK_OFFSET = 		   6,
	RELIABLE_PACKAGE_OFFSET,
	PACKAGE_TYPE_OFFSET =              8,
	PAYLOAD_LEN_OFFSET =               12,
	CHECKSUM_OFFSET =                  24,

	INTEGRETY_CHECK =                 (1 << INTEGRETY_CHECK_OFFSET),
	RELIABLE_PACKAGE =	              (1 << RELIABLE_PACKAGE_OFFSET)
} ;

enum DFUPktField
{
#if DFU_SDK_VERSION == 520
	START_PACKET =              0x00000002,
	DATA_PACKET,
	STOP_PACKET
#elif (DFU_SDK_VERSION == 710 || DFU_SDK_VERSION == 800)
	INVALID_PACKET =            0x00000000,
	INIT_PACKET,
	STOP_INIT_PACKET,
	START_PACKET,
	DATA_PACKET,
	STOP_PACKET
#endif
} ;

enum DFUUpdateMode
{
	DFU_UPDATE_BOOT =                 2,
	DFU_UPDATE_FW =                   4
} ;

typedef struct DFUAckInfo
{
	unsigned char ackNum ;
	unsigned char seqNum ;
} DFUAckInfo ;

typedef struct DFUStartPktParm
{
#if DFU_SDK_VERSION == 520
	unsigned int apSize ;
#elif (DFU_SDK_VERSION == 710 || DFU_SDK_VERSION == 800)
	unsigned int sdSize ;
	unsigned int blSize ;
	unsigned int apSize ;
#endif
} DFUStartPktParm ;

#if (DFU_SDK_VERSION == 710 || DFU_SDK_VERSION == 800)
typedef struct DFUInitPktParm
{
	unsigned short devType ;
	unsigned short devRevision ;
	unsigned int appVersion ;
	unsigned short vlidSfDevListLen ;
	unsigned short vlidSfDevList ;
	unsigned short binCrc ;
	unsigned short filler ;
} DFUInitPktParm ;
#endif

typedef struct DFUDataPktParm
{
	FILE *filePt ;
	unsigned int apSize ;
} DFUDataPktParm ;

typedef struct DFUEndPktParm
{
} DFUEndPktParm ;

typedef union DFUPktParm
{
	DFUStartPktParm startPkt ;
	DFUInitPktParm initPkt ;
	DFUDataPktParm dataPkt ;
	DFUEndPktParm endPkt ;
} DFUPktParm ;

typedef struct DFUPktInfo
{
	unsigned int fieldType ;
	unsigned int size ;
	unsigned int updateMode ;
	DFUPktParm paramInfo ;
} DFUPktInfo ;


enum AppSlip
{
	APP_SLIP_END = 			0xC0,     /**< SLIP code for identifying the beginning and end of a packet frame.. */
	APP_SLIP_ESC = 			0xDB,     /**< SLIP escape code. This code is used to specify that the following character is specially encoded. */
	APP_SLIP_ESC_END = 		0xDC,     /**< SLIP special code. When this code follows 0xDB, this character is interpreted as payload data 0xC0.. */
    APP_SLIP_ESC_ESC =      0xDD      /**< SLIP special code. When this code follows 0xDB, this character is interpreted as payload data 0xDB. */
} ;

//---------------------------------------------------------------------------
int WriteFileDFU(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline SendStartDFUPkt(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar, UpdateProcessFun callBackProcessFun, DFUAckInfo *ackInfo) ;
int inline SendInitDFUPkt(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar, UpdateProcessFun callBackProcessFun, DFUAckInfo *ackInfo) ;
int inline SendDataDFUPkt(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar, UpdateProcessFun callBackProcessFun, DFUAckInfo *ackInfo) ;
int inline SendStopDFUPkt(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar, UpdateProcessFun callBackProcessFun, DFUAckInfo *ackInfo) ;
int inline SendDFUPkt(HANDLE comPort, DFUPktInfo *pktInfo, DFUAckInfo *ackInfo, void *progressBar, UpdateProcessFun callBackUpdateProcessFun) ;
int inline CreateDFUPkt(unsigned char *buf, unsigned char *date, int dataSize, DFUPktInfo *pktInfo, DFUAckInfo *ackInfo) ;
unsigned int inline CreateDFUHeader(int pktType, int bufSize, DFUAckInfo *ackInfo) ;
unsigned char inline ComputeDFUCheckSum(unsigned int header) ;
unsigned short ComputeCRC(void *buf, unsigned int bufSize, const unsigned short *seed, bool fromFile) ;
int inline ExpandPackageContext(unsigned char *oriBuf, unsigned int oriBufSize, unsigned char *outBuf) ;
__inline unsigned short* CheckExpandByteList(unsigned char *buf, unsigned int bufSize) ;
int JudgeDFUPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *progressBar, UpdateProcessFun callBackUpdateProcessFun) ;
int CheckDFUResponse(unsigned char *buf, int bufSize, DFUAckInfo *pktInfo) ;

#endif
