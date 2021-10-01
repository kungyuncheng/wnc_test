//---------------------------------------------------------------------------

#ifndef ControlG11MH
#define ControlG11MH
//---------------------------------------------------------------------------
#include "TSIPackage.h"

#define UPDATE_MAX_CONTAIN_NUM_G11M         4
#define DEVICE_BAUDRATE_G11M                115200
#define MAX_UPDATE_FILE_NUM_G11M 			6
#define G11M_PACKAGE_SIZE                   512
#define G11M_FILE_HEADER_OFFSET             0x40
#define G11M_NUM_OF_MCU_BLOCK               16
#define G11M_NUM_OF_FLASH_BLOCK             64
#define G11M_BLOCK_SIZE                     0x1000

#define G11M_COURSE_ADDR                    0x20f000
#define G11M_AP_ADDR                        0x8180000
#define G11M_FW_ADDR			            0x8000000
#define G11M_SERIAL_NUM_ADDR                0x1FFE000
#define G11M_DATE_NUM_ADDR 			        0x1FFF000
#define G11M_SETTING_ADDR                   0x01FFC000
#define G11M_CLOCK_SKIN_ANL_ADDR            0x0200000
#define G11M_CLOCK_SKIN_DIG_ADDR            0x020C000

#define G11M_FLASH_VER_NUM                  3
#define G11M_CLK_ANALOG_ADDR                0x0200000
#define G11M_CLK_DIGITAL_ADDR               0x020C000
#define G11M_FLASH_VER1_ADDR                0x000042c
#define G11M_FLASH_VER2_ADDR               (G11M_CLK_DIGITAL_ADDR + 0x5C)
#define G11M_FLASH_VER3_ADDR               (G11M_CLK_ANALOG_ADDR + 0x1FC)

//---------------------------------------------------------------------------
enum G11mUpdateTag
{
	G11M_MARK_4_AP	=		0x01,
	G11M_MARK_4_BOOT =		0x02,
	G11M_MARK_4_FLASH =	    0x04
} ;

typedef struct FWFileInfoG11m
{
	unsigned int childFileNum;
	unsigned char dataMark[UPDATE_MAX_CONTAIN_NUM_G11M];
	unsigned int ver[UPDATE_MAX_CONTAIN_NUM_G11M];
	unsigned int date;
	unsigned int reserve;
	unsigned int fileOffset[UPDATE_MAX_CONTAIN_NUM_G11M];
	unsigned int fileLen[UPDATE_MAX_CONTAIN_NUM_G11M];
} FWFileInfoG11m ;

typedef struct AnotherFileInfoG11m
{
	unsigned int makeDate ;
	unsigned int writeAddr ;
	unsigned int reserve[30] ;
} AnotherFileInfoG11m ;

//---------------------------------------------------------------------------
int ResetDeviceSettingG11m(HANDLE comPort) ;
int AskDevInfoG11m(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo) ;
int UpdateG11m(int updateOption, HANDLE comPort, FILE *fp, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline UpdateFwFileG11m(int updateEvent, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlashG11m(HANDLE comPort, int handleAddr, int handelLen) ;
int ClearFlashSeriesG11m(HANDLE comPort, int handleAddr, int handelLen) ;
int inline UpdateAnotherG11m(int updateOption, HANDLE comPort, FILE *fp, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int inline UpdateAnotherFileG11m(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int WriteSnG11m(HANDLE comPort, WriteSnSettingInfo snInfo) ;
int ReadDataG11m(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;
int ClearFlash4KG11m(HANDLE comPort, int handleAddr, int handelLen, void *processBar = NULL, UpdateProcessFun callBackUpdateProcessFun = NULL) ;

#endif
