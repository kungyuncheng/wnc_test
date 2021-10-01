#ifndef ControlBT1000H
#define ControlBT1000H

#include "Uart.h"
#include "TSIPackage.h"
#include "ControlGPS.h"

#define UPDATE_MAX_CONTAIN_NUM_BT1000        4
#define DEVICE_BAUDRATE_BT1000			     921600
#define DEVICE_BAUDRATE_BT1000_BOOT		     460800

#define MAX_UPDATE_FILE_NUM_BT1000 			 2
#define BT1000_NUM_OF_MCU_BLOCK              64
#define BT1000_NUM_OF_FLASH_BLOCK            16

#define SERIES_SIZE_BT1000					 11

#define BT1000_PACKAGE_SIZE					 512

#define BT1000_BLE_ADDR				         0x20000000
#define BT1000_SERIES_NUM_ADDR			     0x7F0000
#define BT1000_FW_ADDR			             0x8000000

#define BT1000_LOG_LEN					     0x800000

#define MAX_SECTION_BT1000 			         (BT1000_LOG_LEN / AP_BLOCK_SIZE)

//---------------------------------------------------------------------------
typedef struct FileInfoBT1000       // »P 870¤@¼Ë
{
	unsigned int fileSize[UPDATE_MAX_CONTAIN_NUM_BT1000] ;
	unsigned int fileOffset[UPDATE_MAX_CONTAIN_NUM_BT1000] ;
	unsigned int writeAddr[UPDATE_MAX_CONTAIN_NUM_BT1000] ;
	unsigned int fileVer[UPDATE_MAX_CONTAIN_NUM_BT1000] ;
	unsigned int reserve[8] ;
} FileInfoBT1000 ;

//---------------------------------------------------------------------------
int AskDevInfoBT1000(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int JumpToBootBT1000(HANDLE *comPort) ;
int UpdateBT1000(int fileIdx, HANDLE *comPort, unsigned char *fileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun, UpdateGPSInfo updateGPSInfo) ;
int inline UpdateBT1000Normal(HANDLE *comPort, unsigned char *fileName, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline UpdateBT1000Gps(HANDLE *comPort, unsigned char *fileName, UpdateGPSInfo updateGPSInfo) ;
int inline UpdateFileBT1000(HANDLE comPort, FILE *fp, FileInfoBT1000 fileInfo, int fIdx, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlashBT1000(HANDLE comPort, int handleAddr, int handelLen) ;
int DeleteLogBT1000(HANDLE comPort) ;
int ReadDataBT1000(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int WriteSnBT1000(HANDLE comPort, WriteSnSettingInfo snInfo) ;

#endif
