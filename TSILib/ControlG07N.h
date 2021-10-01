//---------------------------------------------------------------------------

#ifndef ControlG07NH
#define ControlG07NH
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define UPDATE_MAX_CONTAIN_NUM_G07N          16
#define DEVICE_BAUDRATE_G07N 			     115200
#define MAX_UPDATE_FILE_NUM_G07N 			 3
#define G07N_PACKAGE_SIZE				     256
#define G07N_FILE_HEADER_OFFSET              0x40
#define G07N_NUM_OF_MCU_BLOCK                16
#define G07N_NUM_OF_FLASH_BLOCK              16

#define G07N_SETTING_ADDR                    0x00601000
#define G07N_SETTING_BACKUP_ADDR	         0x00610000
#define G07N_COURSE_ADDR	                 0x180000
#define G07N_DATE_NUM_ADDR			         0x616000          // date code & lot number
#define G07N_SERIAL_NUM_ADDR			     0x603000
#define G07N_SCORE_ADDR		                 0x602000
#define G07N_SHOT_ADDR		                 0x606000
#define G07N_AP_ADDR	                     0x7E0000
#define G07N_LOG_ADDR	                     0x611000
#define G07N_FW_ADDR			             0x8000000

#define G07N_SCORE_LEN 			             0x1000
#define G07N_SHOT_LEN				         0x9000
#define G07N_LOG_LEN				         0x4000
#define G07N_FW_LEN			                 0x20000
#define G07N_FLASH_LEN 		                 0x800000

//---------------------------------------------------------------------------
typedef struct FileInfoG07N
{
	unsigned int makeDate ;
	unsigned int writeAddr ;
	unsigned int reserve[30] ;
} FileInfoG07N ;

//---------------------------------------------------------------------------
int ResetDeviceSettingG07N(HANDLE comPort) ;
int AskDevInfoG07N(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int UpdateG07N(int updateOptionx, HANDLE comPort, FILE *fp, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline UpdateFileG07N(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlashG07N(HANDLE comPort, int handleAddr, int handelLen) ;
int WriteSnG07N(HANDLE comPort, WriteSnSettingInfo snInfo) ;
int ReadDataG07N(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlash4KG07N(HANDLE comPort, int handleAddr, int handelLen, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;

#endif
