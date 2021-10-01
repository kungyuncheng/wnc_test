//---------------------------------------------------------------------------

#ifndef Control870H
#define Control870H

#include "MTKPackage.h"
#include "TSIPackage.h"
#include "ControlGPS.h"

#define UPDATE_MAX_CONTAIN_NUM_870       4
#define DEVICE_BAUDRATE_870				 921600
#define DEVICE_BAUDRATE_870_BOOT		 115200

#define MAX_UPDATE_FILE_NUM_870			 2
#define D870_NUM_OF_MCU_BLOCK            64
#define D870_NUM_OF_FLASH_BLOCK          16

#define SERIES_SIZE_870					 11

#define D870_PACKAGE_SIZE				 512

#define D870_BLE_ADDR				     0x20000000
#define D870_SERIES_NUM_ADDR		     0x7F0000
#define D870_FW_ADDR			         0x8000000

//---------------------------------------------------------------------------
typedef struct FileInfo870
{
	unsigned int fileSize[UPDATE_MAX_CONTAIN_NUM_870] ;
	unsigned int fileOffset[UPDATE_MAX_CONTAIN_NUM_870] ;
	unsigned int writeAddr[UPDATE_MAX_CONTAIN_NUM_870] ;
	unsigned int fileVer[UPDATE_MAX_CONTAIN_NUM_870] ;
	unsigned int reserve[8] ;
} FileInfo870 ;

//---------------------------------------------------------------------------
int AskDevInfo870(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int JumpToBoot870(HANDLE *comPort) ;
int Update870(int fileIdx, HANDLE *comPort, unsigned char *fileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun, UpdateGPSInfo updateGPSInfo) ;
int inline Update870Normal(HANDLE *comPort, unsigned char *fileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int inline Update870Gps(HANDLE *comPort, unsigned char *fileName, UpdateGPSInfo updateGPSInfo) ;
int inline UpdateFile870(HANDLE comPort, FILE *fp, FileInfo870 fileInfo, int fIdx, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlash870(HANDLE comPort, int handleAddr, int handelLen) ;
int DeleteLog870(HANDLE comPort) ;
int ReadData870(int tIdx, HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName) ;
int WriteSn870(HANDLE comPort, WriteSnSettingInfo snInfo) ;

#endif
