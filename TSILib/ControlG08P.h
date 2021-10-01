//---------------------------------------------------------------------------

#ifndef ControlG08PH
#define ControlG08PH
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define UPDATE_MAX_CONTAIN_NUM_G08P           16
#define DEVICE_BAUDRATE_G08P 			      115200
#define MAX_UPDATE_FILE_NUM_G08P 			  2
#define G08P_PACKAGE_SIZE				      128
#define G08P_FILE_HEADER_OFFSET               0x40
#define G08P_FILE_DATA_OFFSET                 0x80
#define G08P_NUM_OF_MCU_BLOCK                 16
#define G08P_NUM_OF_FLASH_BLOCK               16

#define G08P_SETTING_ADDR                     0x00701000
#define G08P_COURSE_ADDR			          0x200000
#define G08P_DATE_NUM_ADDR			          0x7FB000
#define G08P_SERIAL_NUM_ADDR	              0x700000
#define G08P_SCORE_ADDR				          0x702000
#define G08P_SHOT_ADDR				          0x706000
#define G08P_AP_ADDR			              0x8180000
#define G08P_FW_ADDR			              0x8000000

#define G08P_SCORE_LEN 			              0x1000
#define G08P_SHOT_LEN				          0x9000

//---------------------------------------------------------------------------
typedef struct FileInfoG08P
{
	unsigned int makeDate ;
	unsigned int writeAddr ;
	unsigned int reserve[30] ;
} FileInfoG08P ;

int ResetDeviceSettingG08P(HANDLE comPort) ;
int AskDevInfoG08P(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int UpdateG08P(int updateOption, HANDLE comPort, FILE *fp, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline UpdateFileG08P(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlashG08P(HANDLE comPort, int handleAddr, int handelLen) ;
int WriteSnG08P(HANDLE comPort, WriteSnSettingInfo snInfo) ;
int ReadDataG08P(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlash4KG08P(HANDLE comPort, int handleAddr, int handelLen, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;

#endif
