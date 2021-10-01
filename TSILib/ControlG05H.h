//---------------------------------------------------------------------------

#ifndef ControlG05HH
#define ControlG05HH
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define UPDATE_MAX_CONTAIN_NUM_G05H          16
#define DEVICE_BAUDRATE_G05H 			     115200
#define MAX_UPDATE_FILE_NUM_G05H			 3
#define G05H_PACKAGE_SIZE	                 256
#define G05H_FILE_HEADER_OFFSET              0
#define G05H_NUM_OF_MCU_BLOCK                16
#define G05H_NUM_OF_FLASH_BLOCK              16

#define G05H_SETTING_ADDR                    0x00601000
#define G05H_SETTING_BACKUP_ADDR	         0x00610000
#define G05H_COURSE_ADDR	                 0x180000
#define G05H_SERIAL_NUM_ADDR			     0x603000
#define G05H_SCORE_ADDR		                 0x602000
#define G05H_SHOT_ADDR		                 0x606000
#define G05H_AP_ADDR	                     0x7E0000
#define G05H_LOG_ADDR	                     0x611000
#define G05H_FW_ADDR			             0x8000000

#define G05H_SCORE_LEN 			             0x1000
#define G05H_SHOT_LEN				         0x9000
#define G05H_LOG_LEN				         0x4000
#define G05H_FW_LEN			                 0x20000
#define G05H_FLASH_LEN 		                 0x800000

//---------------------------------------------------------------------------
typedef struct FileInfoG05H
{
	unsigned int makeDate ;
	unsigned int writeAddr ;
	unsigned int reserve[30] ;
} FileInfoG05H ;

//---------------------------------------------------------------------------
int ResetDeviceSettingG05H(HANDLE comPort) ;
int AskDevInfoG05H(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int UpdateG05H(int updateOption, HANDLE comPort, FILE *fp, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline UpdateFileG05H(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlashG05H(HANDLE comPort, int handleAddr, int handelLen) ;
int WriteSnG05H(HANDLE comPort, WriteSnSettingInfo snInfo) ;
int ReadDataG05H(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlash4KG05H(HANDLE comPort, int handleAddr, int handelLen, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;

#endif
