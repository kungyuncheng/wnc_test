//---------------------------------------------------------------------------

#ifndef Control770H
#define Control770H

#include "MTKPackage.h"
#include "TSIPackage.h"

#define DEVICE_BAUDRATE_770				115200
#define DEVICE_BAUDRATE_770_DOWNLOAD	921600

#define MAX_UPDATE_FILE_NUM_770				 1
#define D770_RESET_TIMEOUT              100000  // 100 s

#define D770_SERIES_NUM_ADDR			0x6000

#define D770_LOG_LEN				    0x7E0000
#define MAX_SECTION_770                (D770_LOG_LEN / AP_BLOCK_SIZE)     // total 128, 1個存setting, 1個保留

//---------------------------------------------------------------------------
int AskDevInfo770(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int ReadData770(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackFun) ;
int ChangeNMEAFre770(HANDLE comPort, bool on5HZ) ;
int DeleteLog770(HANDLE comPort, bool waitAck) ;
int ResetDevSetting770(HANDLE comPort) ;
int Update770(int updateIdx, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackFun) ;
int WriteSn770(HANDLE comPort, WriteSnSettingInfo snInfo) ;

#endif
