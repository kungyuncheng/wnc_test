//---------------------------------------------------------------------------

#ifndef ControlG06H
#define ControlG06H
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define HEADER_TABLE_SIZE_G06             1024
#define UPDATE_MAX_CONTAIN_NUM_G06 		  10
#define MAX_UPDATE_FILE_NUM_G06 		  4
#define MAX_SECTOR_NUM_G06 			      9

#define DEVICE_BAUDRATE_G06 			  9600
#define DEVICE_BAUDRATE_G06_BOOT		  460800

#define G06_RAM_ADDR			          0x20000000
#define G06_AP_ADDR			              0x08001400
#define G06_LOG_ADDR				      0x7F4000
#define G06_SERIES_NUM_ADDR			      0x007FB000

#define SERIES_SIZE_G06 				  16
#define G06_PACKAGE_SIZE				  128
#define RECORD_SIZE_G06			          16

#define G06_LOG_LEN 				      0x006000
#define G06_FLASH_LEN		  		      0x800000
//---------------------------------------------------------------------------

typedef struct LogInfoG06
{
	unsigned int utc ;
	float latitude ;
	float longtitude ;
	unsigned char gpsValid ;
	unsigned char reserve[3] ;
	unsigned int courseId ;
	unsigned char holeNum ;
	unsigned char holeNumSeq ;
	unsigned short strokesAtCurHole ;
} LogInfoG06 ;

typedef struct ChileFileInfoG06
{
	unsigned int writeAddr ;
	unsigned int fileSize ;
	unsigned int checkSum ;
	unsigned char reserve[4] ;
} ChileFileInfoG06 ;

typedef struct FileInfoG06
{
	unsigned int totalFileNum ;
	unsigned int fileSize[UPDATE_MAX_CONTAIN_NUM_G06] ;
	unsigned int fileOffset[UPDATE_MAX_CONTAIN_NUM_G06] ;
	unsigned int checkSum ;
	unsigned char reserve[HEADER_TABLE_SIZE_G06 - (sizeof(unsigned int) * (UPDATE_MAX_CONTAIN_NUM_G06 * 2 + 2))] ;
	ChileFileInfoG06 childInfo[UPDATE_MAX_CONTAIN_NUM_G06] ;
} FileInfoG06 ;
//---------------------------------------------------------------------------

int AskDevInfoG06(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int JumpToBootG06(HANDLE *comPort) ;
int JumpToAddrG06(HANDLE *comPort, int addr) ;
int UpdateG06(int updateIdx, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int inline WriteChildFileG06(HANDLE comPort, FILE *fp, FileInfoG06 fileInfo, int fIdx, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int ClearFlash4KG06(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackFun) ;
int ReadDataG06(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackFun) ;
int WriteSnG06(HANDLE comPort, WriteSnSettingInfo snInfo) ;
void ReadLogInfoG06(FILE *fp, LogInfoG06 *info) ;
unsigned int CalculateFileCheckSumG06(FILE *readFile, unsigned int fileLen, unsigned int nowAddr) ;

#endif
