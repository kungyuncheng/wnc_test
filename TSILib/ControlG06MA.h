//---------------------------------------------------------------------------

#ifndef ControlG06MAH
#define ControlG06MAH
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define HEADER_TABLE_SIZE_G06MA         1024
#define UPDATE_MAX_CONTAIN_NUM_G06MA 	10
#define MAX_UPDATE_FILE_NUM_G06MA		2
#define MAX_SECTOR_NUM_G06MA 			9

#define DEVICE_BAUDRATE_G06MA 			460800
#define DEVICE_BAUDRATE_G06MA_BOOT		460800

#define G06MA_RAM_ADDR			        0x20000000
#define G06MA_AP_ADDR			        0x08001400
#define G06MA_LOG_ADDR				    0x400000
#define G06MA_SERIES_NUM_ADDR		    0x003FB000

#define G06MA_PACKAGE_SIZE				128
#define RECORD_SIZE_G06MA			    16

#define G06MA_LOG_LEN				  	0x400000
#define G06MA_FLASH_LEN		  		    0x800000
//---------------------------------------------------------------------------

typedef struct LogInfoG06MA
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
} LogInfoG06MA ;

typedef struct ChileFileInfoG06MA
{
	unsigned int writeAddr ;
	unsigned int fileSize ;
	unsigned int checkSum ;
	unsigned char reserve[4] ;
} ChileFileInfoG06MA ;

typedef struct FileInfoG06MA
{
	unsigned int totalFileNum ;
	unsigned int fileSize[UPDATE_MAX_CONTAIN_NUM_G06MA] ;
	unsigned int fileOffset[UPDATE_MAX_CONTAIN_NUM_G06MA] ;
	unsigned int checkSum ;
	unsigned char reserve[HEADER_TABLE_SIZE_G06MA - (sizeof(unsigned int) * (UPDATE_MAX_CONTAIN_NUM_G06MA * 2 + 2))] ;
	ChileFileInfoG06MA childInfo[UPDATE_MAX_CONTAIN_NUM_G06MA] ;
} FileInfoG06MA ;
//---------------------------------------------------------------------------

int AskDevInfoG06MA(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int JumpToBootG06MA(HANDLE *comPort) ;
int JumpToAddrG06MA(HANDLE *comPort, int addr) ;
int UpdateG06MA(int updateIdx, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int inline WriteChildFileG06MA(HANDLE comPort, FILE *fp, FileInfoG06MA fileInfo, int fIdx, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int ClearFlash4KG06MA(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackFun) ;
int ReadDataG06MA(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackFun) ;
int WriteSnG06MA(HANDLE comPort, WriteSnSettingInfo snInfo) ;
void ReadLogInfoG06MA(FILE *fp, LogInfoG06MA *info) ;
unsigned int CalculateFileCheckSumG06MA(FILE *readFile, unsigned int fileLen, unsigned int nowAddr) ;

#endif
