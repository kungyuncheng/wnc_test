//---------------------------------------------------------------------------

#ifndef ControlG07PH
#define ControlG07PH
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define UPDATE_MAX_CONTAIN_NUM_G07P          16
#define DEVICE_BAUDRATE_G07P 			     115200
#define MAX_UPDATE_FILE_NUM_G07P 			 14
#define G07P_PACKAGE_SIZE	 			     512
#define G07P_FILE_HEADER_OFFSET              0x40
#define G07P_NUM_OF_MCU_BLOCK                16
#define G07P_NUM_OF_FLASH_BLOCK              512
#define G07P_AP_BLOCK_SIZE				     0x1000

#define G07P_SETTING_ADDR                    0x00FDA000
#define G07P_SETTING_BACKUP_ADDR	         0x00FDB000
#define G07P_COURSE_ADDR	                 0x180000
#define G07P_DATE_NUM_ADDR			         0x5cF000
#define G07P_SERIAL_NUM_ADDR	             0xFCF000
#define G07P_SCORE_ADDR				         0xFCE000
#define G07P_SHOT_ADDR				         0xFD1000
#define G07P_AP_ADDR	                     0x7E0000
#define G07P_LOG_ADDR	                     0x611000
#define G07P_FW_ADDR			             0x8000000

#define G07P_SCORE_LEN 			             0x1000
#define G07P_SHOT_LEN				         0x9000
#define G07P_LOG_LEN				         0x4000
#define G07P_FW_LEN			                 0x20000
#define G07P_FLASH_LEN				         0x1000000

//---------------------------------------------------------------------------
typedef struct FileInfoG07P
{
	unsigned int makeDate ;
	unsigned int writeAddr ;
	unsigned int reserve[30] ;
} FileInfoG07P ;

//---------------------------------------------------------------------------
int ResetDeviceSettingG07P(HANDLE comPort) ;
int AskDevInfoG07P(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int UpdateG07P(int updateOption, HANDLE comPort, FILE *fp, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline UpdateFileG07P(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlashG07P(HANDLE comPort, int handleAddr, int handelLen) ;
int WriteSnG07P(HANDLE comPort, WriteSnSettingInfo snInfo) ;
int ReadDataG07P(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlash4KG07P(HANDLE comPort, int handleAddr, int handelLen, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;

#endif
