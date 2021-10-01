//---------------------------------------------------------------------------

#ifndef ControlG08H
#define ControlG08H
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define UPDATE_MAX_CONTAIN_NUM_G08           16
#define DEVICE_BAUDRATE_G08 			     115200
#define MAX_UPDATE_FILE_NUM_G08 			 2
#define G08_PACKAGE_SIZE				     128
#define G08_FILE_HEADER_OFFSET               0x40
#define G08_NUM_OF_MCU_BLOCK                 16
#define G08_NUM_OF_FLASH_BLOCK               16

#define G08_SETTING_ADDR                     0x00601000
#define G08_COURSE_ADDR	                     0x180000
#define G08_DATE_NUM_ADDR			         0x7CB000
#define G08_SERIAL_NUM_ADDR	                 0x600000
#define G08_AP_ADDR	                         0x7E0000
#define G08_LOG_ADDR				         0x606000
#define G08_FW_ADDR			                 0x8000000

#define G08_SCORE_LEN 			             0x1000
#define G08_SHOT_LEN				         0x9000
#define G08_LOG_LEN 					     0x9000
#define G08_FW_LEN			                 0x20000
#define G08_FLASH_LEN 		                 0x800000

//---------------------------------------------------------------------------
typedef struct FileInfoG08
{
	unsigned int makeDate ;
	unsigned int writeAddr ;
	unsigned int reserve[30] ;
} FileInfoG08 ;

//---------------------------------------------------------------------------
int ResetDeviceSettingG08(HANDLE comPort) ;
int AskDevInfoG08(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int UpdateG08(int updateOption, HANDLE comPort, FILE *fp, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline UpdateFileG08(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlashG08(HANDLE comPort, int handleAddr, int handelLen) ;
int WriteSnG08(HANDLE comPort, WriteSnSettingInfo snInfo) ;
int ReadDataG08(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlash4KG08(HANDLE comPort, int handleAddr, int handelLen, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;

#endif
