//---------------------------------------------------------------------------

#ifndef ControlP51H
#define ControlP51H
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define HEADER_TABLE_SIZE_P51             1024
#define UPDATE_MAX_CONTAIN_NUM_P51 	      10
#define MAX_UPDATE_FILE_NUM_P51 		  1
#define MAX_SECTOR_NUM_P51				  64

#define DEVICE_BAUDRATE_P51				  9600
#define DEVICE_BAUDRATE_P51_BOOT          460800

#define P51_RAM_ADDR			          0x20000000
#define P51_LOG_ADDR				  	  0x400000
#define P51_SERIES_NUM_ADDR				  0x6000

#define P51_PACKAGE_SIZE				  128
#define RECORD_SIZE_P51					  64

#define P51_LOG_LEN 				      0x400000
#define P51_FLASH_LEN		  		      0x800000
//---------------------------------------------------------------------------

typedef struct LogInfoP51
{
	unsigned long utc ;
	char valid ;
	unsigned char reserve[3] ;
	double latitude ;
	double longtitude ;
	float speed ;
	float height ;
	float heading ;
	float ecomposs ;
	float gsensorX ;
	float gsensorY ;
	float gsensorZ ;
	int heartRate ;
	unsigned short flag;
	char padding[4] ;
} LogInfoP51 ;

typedef struct ChileFileInfoP51
{
	unsigned int writeAddr ;
	unsigned int fileSize ;
	unsigned int checkSum ;
	unsigned char reserve[4] ;
} ChileFileInfoP51 ;

typedef struct FileInfoP51
{
	unsigned int totalFileNum ;
	unsigned int fileSize[UPDATE_MAX_CONTAIN_NUM_P51] ;
	unsigned int fileOffset[UPDATE_MAX_CONTAIN_NUM_P51] ;
	unsigned int checkSum ;
	unsigned char reserve[HEADER_TABLE_SIZE_P51 - (sizeof(unsigned int) * (UPDATE_MAX_CONTAIN_NUM_P51 * 2 + 2))] ;
	ChileFileInfoP51 childInfo[UPDATE_MAX_CONTAIN_NUM_P51] ;
} FileInfoP51 ;
//---------------------------------------------------------------------------

int AskDevInfoP51(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int JumpToBootP51(HANDLE *comPort) ;
int JumpToAddrP51(HANDLE *comPort, int addr) ;
int UpdateP51(int updateIdx, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int inline WriteChildFileP51(HANDLE comPort, FILE *fp, FileInfoP51 fileInfo, int fIdx, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int ReadDataP51(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackFun) ;
int ClearFlash4KP51(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackFun) ;
int WriteSnP51(HANDLE comPort, WriteSnSettingInfo snInfo) ;
void ReadLogInfoP51(FILE *fp, LogInfoP51 *info) ;
unsigned int CalculateFileCheckSumP51(FILE *readFile, unsigned int fileLen, unsigned int nowAddr) ;

#endif
