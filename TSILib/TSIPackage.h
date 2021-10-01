//---------------------------------------------------------------------------

#ifndef TSIPackageH
#define TSIPackageH

#include "Uart.h"
#include "MTKPackage.h"

//---------------------------------------------------------------------------
enum DevType
{
	DEVICE_TYPE_G05,
	DEVICE_TYPE_G05H,
	DEVICE_TYPE_G07N,
	DEVICE_TYPE_G07P,
	DEVICE_TYPE_G08,
	DEVICE_TYPE_G08P,
	DEVICE_TYPE_G06MA,
	DEVICE_TYPE_G06,
	DEVICE_TYPE_P51,
	DEVICE_TYPE_770,
	DEVICE_TYPE_870,
	DEVICE_TYPE_BT1000,
    DEVICE_TYPE_G11M,
	NUM_OF_DEVICE_TYPE  // 13
} ;

enum UpdatePathOption
{
	UPDATE_PATH_FW,
    UPDATE_PATH_BOOT,
	UPDATE_PATH_VOICE,
	UPDATE_PATH_CLOCK_SKIN,
	UPDATE_PATH_COURSE,
    UPDATE_PATH_COURSE_NAME,
	UPDATE_PATH_LAYOUT,             // 11­ÓÀÉ®×
	UPDATE_PATH_GPS = 				17
} ;

enum TSIOptionId
{
	TSI_ACK_SUCCESS =             1,
	TSI_OPTION_ID_510 =         510,
	TSI_OPTION_ID_770 =         770,
	TSI_OPTION_ID_990 =         990,
	TSI_OPTION_ID_997 =         997,
	TSI_OPTION_ID_998 =         998,
	TSI_OPTION_ID_999 =         999,     // reset device
	TSI_OPTION_ID_1600 =        1600,
} ;

enum TSICmd
{
	TSI_QUERY_STATUS,
	TSI_CMD_WRITE,
	TSI_CMD_READ,
	TSI_SHUTDOWN =           	6,
	TSI_MSG_RATE,
	TSI_POWERSAVING,
	TSI_ABOUT_CONNECT,
	TSI_BOOTLOADER
} ;

enum TSI770Cmd
{
	TSI_CMD_WRITE_770 =         3,
    TSI_ERASE_LOG_STATUS_770 =  90
} ;

enum TSI998Cmd
{
    TSI_STATUS_BOOT
} ;

enum TSI999Cmd
{
	TSI_RESET_NMEA_OUTPUT,
	TSI_ERASE_LOG,
	TSI_WRITE_SERIES_999 =      8,
	TSI_QUERY_RELEASE
} ;

enum TSI1600CMd
{
	TSI_ERASE_FLASH
} ;

enum CmdStatus
{
	CMD_OFF,
	CMD_ON
} ;

enum LogBufNum
{
	LOG_BUF_NUM_G06MA =    		9,
	LOG_BUF_NUM_G06 =    		9,
	LOG_BUF_NUM_P51 =           15
} ;

//---------------------------------------------------------------------------
typedef struct WriteSnSettingCheck870
{
	unsigned char apVer[BASE_DATA_SIZE] ;
	unsigned char bootVer[BASE_DATA_SIZE] ;
	unsigned char bleVer[BASE_DATA_SIZE] ;
	unsigned char flashId[BASE_DATA_SIZE] ;
} WriteSnSettingCheck870 ;

typedef struct WriteSnSettingSN870
{
	//unsigned char minSn[BASE_DATA_SIZE] ;
	//unsigned char maxSn[BASE_DATA_SIZE] ;
	unsigned char model[BASE_DATA_SIZE] ;
	unsigned char mode2[BASE_DATA_SIZE] ;
	unsigned char year[BASE_DATA_SIZE] ;
	unsigned char month[BASE_DATA_SIZE] ;
} WriteSnSettingSN870 ;

typedef struct WriteSnSetting870
{
	WriteSnSettingCheck870 check ;
	WriteSnSettingSN870 sn ;
	bool delLog ;
} WriteSnSetting870 ;

typedef struct WriteSnSettingCheck770
{
	unsigned char apVer[BASE_DATA_SIZE] ;
	unsigned char bootVer[BASE_DATA_SIZE] ;
	unsigned char bleVer[BASE_DATA_SIZE] ;
	unsigned char flashId[BASE_DATA_SIZE] ;
} WriteSnSettingCheck770 ;

typedef struct WriteSnSettingSN770
{
	//unsigned char minSn[BASE_DATA_SIZE] ;
	//unsigned char maxSn[BASE_DATA_SIZE] ;
	unsigned char model[BASE_DATA_SIZE] ;
	unsigned char year[BASE_DATA_SIZE] ;
	unsigned char month[BASE_DATA_SIZE] ;
} WriteSnSettingSN770 ;

typedef struct WriteSnSetting770
{
	WriteSnSettingCheck770 check ;
	WriteSnSettingSN770 sn ;
	bool delLog ;
} WriteSnSetting770 ;

typedef struct WriteSnSettingCheckP51
{
	unsigned char bootVer[BASE_DATA_SIZE] ;
	unsigned char apVer[BASE_DATA_SIZE] ;
} WriteSnSettingCheckP51 ;

typedef struct WriteSnSettingSNP51
{
	unsigned char startLine[BASE_DATA_SIZE] ;
} WriteSnSettingSNP51 ;

typedef struct WriteSnSettingP51
{
	WriteSnSettingCheckP51 check ;
	WriteSnSettingSNP51 sn ;
} WriteSnSettingP51 ;

typedef struct WriteSnSettingCheckG06
{
	unsigned char bootVer[BASE_DATA_SIZE] ;
	unsigned char apVer[BASE_DATA_SIZE] ;
	unsigned char cVer[BASE_DATA_SIZE] ;
	unsigned char cNameVer[BASE_DATA_SIZE] ;
	unsigned char waveVer[BASE_DATA_SIZE] ;
} WriteSnSettingCheckG06 ;

typedef struct WriteSnSettingSNG06
{
	//unsigned char minSn[BASE_DATA_SIZE] ;
	//unsigned char maxSn[BASE_DATA_SIZE] ;
	unsigned char mode1[BASE_DATA_SIZE] ;
	unsigned char mode2[BASE_DATA_SIZE] ;
	unsigned char mode3[BASE_DATA_SIZE] ;
	unsigned char mode4[BASE_DATA_SIZE] ;
} WriteSnSettingSNG06 ;

typedef struct WriteSnSettingG06
{
	WriteSnSettingCheckG06 check ;
	WriteSnSettingSNG06 sn ;
} WriteSnSettingG06 ;

typedef struct WriteSnSettingCheckG06MA
{
	unsigned char bootVer[BASE_DATA_SIZE] ;
	unsigned char apVer[BASE_DATA_SIZE] ;
	unsigned char waveVer[BASE_DATA_SIZE] ;
} WriteSnSettingCheckG06MA ;

typedef struct WriteSnSettingSNG06MA
{
	//unsigned char minSn[BASE_DATA_SIZE] ;
	//unsigned char maxSn[BASE_DATA_SIZE] ;
	unsigned char mode1[BASE_DATA_SIZE] ;
	unsigned char mode2[BASE_DATA_SIZE] ;
	unsigned char mode3[BASE_DATA_SIZE] ;
	unsigned char mode4[BASE_DATA_SIZE] ;
} WriteSnSettingSNG06MA ;

typedef struct WriteSnSettingG06MA
{
	WriteSnSettingCheckG06MA check ;
	WriteSnSettingSNG06MA sn ;
} WriteSnSettingG06MA ;

typedef struct WriteSnSettingCheckG05Series
{
	unsigned char bootVer[BASE_DATA_SIZE] ;
	unsigned char apVer[BASE_DATA_SIZE] ;
	unsigned char cVer[BASE_DATA_SIZE] ;
	unsigned char flashInfoFi[BASE_DATA_SIZE] ;
	unsigned char flashInfoDig[BASE_DATA_SIZE] ;
	unsigned char flashInfoAnl[BASE_DATA_SIZE] ;
} WriteSnSettingCheckG05Series ;

typedef struct WriteSnSettingSNG05Series
{
	//unsigned char minSn[BASE_DATA_SIZE] ;
	//unsigned char maxSn[BASE_DATA_SIZE] ;
	unsigned char model[BASE_DATA_SIZE] ;
	unsigned char dateNum[BASE_DATA_SIZE] ;
	unsigned char lotNum[BASE_DATA_SIZE] ;
} WriteSnSettingSNG05Series ;

typedef struct WriteSnSettingG05Series
{
	WriteSnSettingCheckG05Series check ;
	WriteSnSettingSNG05Series sn ;
} WriteSnSettingG05Series ;

typedef union WriteSnSettingDevInfo
{
	WriteSnSettingG05Series g05Series ;
	WriteSnSettingG06MA g06ma ;
	WriteSnSettingG06 g06 ;
	WriteSnSettingP51 p51 ;
	WriteSnSetting770 d770 ;
	WriteSnSetting870 d870 ;
} WriteSnSettingDevInfo ;

typedef struct WriteSnSettingInfo
{
	WriteSnSettingDevInfo dev ;
	int minSn ;
	int maxSn ;
	int writeSn ;
} WriteSnSettingInfo ;

///////////////////////////////////////////////////////////////////
typedef struct DevInfoG05Series
{
	int cVer ;
	unsigned char dateNum[BASE_DATA_SIZE] ;
	unsigned char lotNum[BASE_DATA_SIZE] ;
	unsigned char flashInfoFi[BASE_DATA_SIZE] ;
	unsigned char flashInfoDig[BASE_DATA_SIZE] ;
	unsigned char flashInfoAnl[BASE_DATA_SIZE] ;
} DevInfoG05Series ;

typedef struct DevInfoG06Series
{
	int uctZone ;
	unsigned char cNameVer[BASE_DATA_SIZE] ;
	unsigned char cVer[BASE_DATA_SIZE] ;
	unsigned char waveVer[BASE_DATA_SIZE] ;
} DevInfoG06Series ;

typedef struct DevInfo770
{
	int eraseLogStatus ;
	unsigned char bleVer[BASE_DATA_SIZE] ;
} DevInfo770 ;

typedef struct DevInfo870
{
	unsigned char bleVer[BASE_DATA_SIZE] ;
} DevInfo870 ;

typedef struct DevInfoBT1000
{
	unsigned char bleVer[BASE_DATA_SIZE] ;
} DevInfoBT1000 ;

typedef union ExtraDevInfo
{
	DevInfoG05Series g05Series ;
	DevInfoG06Series g06Series ;
	DevInfo770 d770 ;
	DevInfo870 d870 ;
	DevInfoBT1000 bt1000 ;
} ExtraDevInfo ;

typedef struct BaseDevInfo
{
	int recordCount ;
	unsigned char devSeries[BASE_DATA_SIZE] ;
	unsigned char btVer[BASE_DATA_SIZE] ;
	unsigned char fwVer[BASE_DATA_SIZE] ;
} BaseDevInfo ;

typedef struct DeviceInfo
{
	BaseDevInfo base ;
	ExtraDevInfo extra ;
} DeviceInfo ;

typedef struct TSICmdAck
{
	int cmd ;
	int status ;
} TSICmdAck ;

typedef struct TSIAckStatus
{
	unsigned int optionId ;
	unsigned int curReadAddr ;
	DeviceInfo devInfo ;
	TSICmdAck cmdAck ;
	unsigned char readData[MTK_READ_DATA_SIZE] ;
} TSIAckStatus ;

//---------------------------------------------------------------------------
int CreateTSIPkt(unsigned char *writeData, const char *format, ...) ;
int JudgeTSIPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int AnalysisTSIData(int devType, unsigned char *buf, int bufSize, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int inline AnalysisTSI510Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline AnalysisTSI770Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline AnalysisTSI997Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline AnalysisTSI999Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline AnalysisTSI1600Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline Analysis1600EraseFlash(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline AnalysisG06Status(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline Analysis510ReadData(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline AnalysisTSIReadData(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline AnalysisTSI770Write(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline AnalysisTSIStatus(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
int inline AnalysisTSIRelease(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo) ;
bool AskLogFullHandle(HANDLE comPort, int logStartAddr, MTKAckStatus *ackInfo) ;

#endif
