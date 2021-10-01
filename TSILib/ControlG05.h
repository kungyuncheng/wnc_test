//---------------------------------------------------------------------------

#ifndef ControlG05H
#define ControlG05H
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define UPDATE_MAX_CONTAIN_NUM_G05          16
#define DEVICE_BAUDRATE_G05 			    115200
#define MAX_UPDATE_FILE_NUM_G05 			3
#define G05_PACKAGE_SIZE				    256
#define G05_FILE_HEADER_OFFSET              0
#define G05_NUM_OF_MCU_BLOCK                16
#define G05_NUM_OF_FLASH_BLOCK              16

#define LOG_BUF_NUM_G05_SERIES              13

#define G05_CLOCK_ADDR		                0x170000
#define G05_SETTING_ADDR                    0x00601000
#define G05_SETTING_BACKUP_ADDR	            0x00610000
#define G05_FW_ADDR			                0x8000000
#define G05_COURSE_ADDR	                    0x180000
#define G05_SERIAL_NUM_ADDR			        0x603000
#define G05_SCORE_ADDR		                0x602000
#define G05_SHOT_ADDR		                0x606000
#define G05_AP_ADDR	                        0x7E0000
#define G05_LOG_ADDR	                    0x611000
#define G05_FW_ADDR			                0x8000000

#define G05_SCORE_LEN 			            0x1000
#define G05_SHOT_LEN				        0x9000
#define G05_LOG_LEN				            0x4000
#define G05_FW_LEN			                0x20000
#define G05_FLASH_LEN 		                0x800000

#define RECORD_SIZE_G05_SERIES		  		36
#define MAX_SECTOR_NUM_G05_SERIES			6

//---------------------------------------------------------------------------
typedef struct FileInfoG05
{
	unsigned int makeDate ;
	unsigned int writeAddr ;
	unsigned int reserve[30] ;
} FileInfoG05 ;

typedef struct LogInfoG05Series
{
	unsigned short year ;
	unsigned char month ;
	unsigned char day ;
	unsigned char hour ;
	unsigned char minute ;
	unsigned char second ;
	unsigned int courseId ;
	unsigned short holeNum ;
	unsigned short holeNumSeq ;
	double latitude ;
	double longtitude ;
	unsigned int hight ;
	unsigned char reserve ;
} LogInfoG05Series ;

//---------------------------------------------------------------------------
int ResetDeviceSettingG05(HANDLE comPort) ;
int AskDevInfoG05(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int UpdateG05(int updateOption, HANDLE comPort, FILE *fp, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline UpdateFileG05(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlashG05(HANDLE comPort, int handleAddr, int handelLen) ;
int WriteSnG05(HANDLE comPort, WriteSnSettingInfo snInfo) ;
void ReadLogInfoG05Series(FILE *fp, LogInfoG05Series *info, int devType) ;
int ReadDataG05(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlash4KG05(HANDLE comPort, int handleAddr, int handelLen, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;

#endif
