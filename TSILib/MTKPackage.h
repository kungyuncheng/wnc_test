//---------------------------------------------------------------------------

#ifndef MTKPackageH
#define MTKPackageH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>

#include <time.h>
#include <stdio.h>
#include "Uart.h"

#define FORMAT_NUM            		20
#define SECTOR_PATTERN_SIZE 		0x200
#define MAX_SAT_NUM             	32
#define MTK_READ_DATA_SIZE          0x800
#define BOOT_ROM_PACKAGE_SIZE		128
#define DLDA_OFFSET                 (1024 + 16)

//---------------------------------------------------------------------------
enum BootRomCommand
{
	BOOT_ROM_START_CMD1 = 			0xa0,
	BOOT_ROM_START_CMD2 = 			0x0a,
	BOOT_ROM_START_CMD3 = 			0x50,
	BOOT_ROM_START_CMD4 = 			0x05,
	BOOT_ROM_WRITE_CMD = 			0xa1,
	BOOT_ROM_CHECKSUM_CMD = 		0xa4,
	BOOT_ROM_READ_CMD = 			0xa5,
	BOOT_ROM_JUMP_CMD = 			0xa8,

	BOOT_ROM_START_CMD1_ACK =       0x5F,
	BOOT_ROM_START_CMD2_ACK =       0xF5,
	BOOT_ROM_START_CMD3_ACK =       0xAF,
	BOOT_ROM_START_CMD4_ACK =       0xFA,

	BOOT_ROM_START_CMD_NUM =        4
} ;

enum MTKPktType
{
	PMTK_ACK = 						1,
	PMTK_SYS_MSG =                  10,
    PMTK_TXT_MSG,
	PMTK_CMD_COLD_START =           103,
	PMTK_CMD_FULL_COLD_START,
	PMTK_CMD_NMEA_START_CMD =       180,
	PMTK_LOG =    					182,
	PMTK_SET_NMEA_UPDATERATE =      220,
	PMTK_SET_NMEA_OUTPUT =          314,
	PMTK_Q_NMEA_OUTPUT =			414,
	PMTK_DT_NMEA_OUTPUT =           514,
	PMTK_Q_RELEASE =                605,
	PMTK_DT_VERSION =               704,
	PMTK_DT_RELEASE =               705,
	PMTK_TEST_ALL_ACQ =             813,
	PMTK_TEST_ALL_BITSYNC
} ;

enum STMCmd
{
	STM_COLD_CLEAR_ALMANAC =        0x01,
	STM_COLD_CLEAR_EPHEMERIS =      0x02,
	STM_COLD_CLEAR_POSTION =        0x04,
	STM_COLD_CLEAR_TIME =           0x08
} ;

enum MTKLogApi
{
	LOG_CONTROL =                   1,
	LOG_QUERY,
	LOG_RETURN,
	LOG_START,
	LOG_STOP,
	LOG_FORMAT,
	LOG_READ,
	LOG_OUTPUT,
	LOG_INITIAL,
	LOG_ENABLE,
	LOG_DISABLE,
	LOG_WRITE,
	LOG_BAUDRATE =  				14
} ;

enum MTKLogRet
{
	LOG_RET_SPI_STATUS =            1,
	LOG_RET_FMT_REG,
	LOG_RET_SECOND,
	LOG_RET_DISTANCE,
	LOG_RET_SPEED,
	LOG_RET_REC_METHOD,
	LOG_RET_STATUS,
	LOG_RET_REC_ADDR,
	LOG_RET_FLASH_ID,
	LOG_RET_REC_COUNT,
	LOG_RET_REC_FSECTOR,
    LOG_RET_VERSION
} ;

enum MTKNmeaUpdateRate
{
	MTK_NMEA_1HZ =                1000,
	MTK_NMEA_5HZ =                200,
    MTK_NMEA_10HZ =               100
} ;

enum MTKSpiStatus
{
	SPI_READY =                   1,
	SPI_BUSY,
	SPI_FULL
} ;

enum MTKLogStatus
{
	AUTO_LOG_START_OFFSET,
	LOG_METHOD_OFFSET,
	LOG_FUNCTION_ENABLED_OFFSET = 7,
	LOG_FUNCTION_DISABLED_OFFSET,
	LOG_NEED_FORMAT_OFFSET,
	LOG_FULL_OFFSET,
	AUTO_LOG_START =              (1 << AUTO_LOG_START_OFFSET),
	LOG_METHOD =                  (1 << LOG_METHOD_OFFSET),
	LOG_FUNCTION_ENABLED =        (1 << LOG_FUNCTION_ENABLED_OFFSET),
	LOG_FUNCTION_DISABLED =       (1 << LOG_FUNCTION_DISABLED_OFFSET),
	LOG_NEED_FORMAT =      		  (1 << LOG_NEED_FORMAT_OFFSET),
    LOG_FULL =                    (1 << LOG_FULL_OFFSET)
} ;

enum MTKRecStatus
{
	REC_OVERLAPPED =              1,
	REC_STOP
} ;

enum MTKAckOption
{
	ACK_INVALID,
	ACK_UNSUPPORTED,
	ACK_FAIL,
	ACK_SUCCEEDED,
} ;

enum MTKFormatOption
{
	FORMAT_ALL =                  1,
	FORMAT_PARTIAL
} ;

enum RegisterFormat
{
	REG_UTC_OFFSET,
	REG_VALID_OFFSET,
	REG_LATITUDE_OFFSET,
	REG_LONGITUDE_OFFSET,
	REG_HEIGHT_OFFSET,
	REG_SPEED_OFFSET,
	REG_TRACK_OFFSET,
	REG_DSTA_OFFSET,
	REG_DAGE_OFFSET,
	REG_PDOP_OFFSET,
	REG_HDOP_OFFSET,
	REG_VDOP_OFFSET,
	REG_NSAT_OFFSET,
	REG_SID_OFFSET,
	REG_ELE_OFFSET,
	REG_AZI_OFFSET,
	REG_SNR_OFFSET,
	REG_RCR_OFFSET,
	REG_MS_OFFSET,
	REG_DIS_OFFSET,
	REG_SEPARATE_OFFSET,
	REG_TSI_DEFINE_G_VALUE_OFFSET,
	REG_UTC =                     (1 << REG_UTC_OFFSET),
	REG_VALID =                   (1 << REG_VALID_OFFSET),
	REG_LATITUDE =                (1 << REG_LATITUDE_OFFSET),
	REG_LONGITUDE =               (1 << REG_LONGITUDE_OFFSET),
	REG_HEIGHT =                  (1 << REG_HEIGHT_OFFSET),
	REG_SPEED =                   (1 << REG_SPEED_OFFSET),
	REG_TRACK =                   (1 << REG_TRACK_OFFSET),
	REG_DSTA =                    (1 << REG_DSTA_OFFSET),
	REG_DAGE =                    (1 << REG_DAGE_OFFSET),
	REG_PDOP =                    (1 << REG_PDOP_OFFSET),
	REG_HDOP =                    (1 << REG_HDOP_OFFSET),
	REG_VDOP =                    (1 << REG_VDOP_OFFSET),
	REG_NSAT =                    (1 << REG_NSAT_OFFSET),
	REG_SID =                     (1 << REG_SID_OFFSET),
	REG_ELE =                     (1 << REG_ELE_OFFSET),
	REG_AZI =                     (1 << REG_AZI_OFFSET),
	REG_SNR =                     (1 << REG_SNR_OFFSET),
	REG_RCR =                     (1 << REG_RCR_OFFSET),
	REG_MS =                      (1 << REG_MS_OFFSET),
	REG_DIS =                     (1 << REG_DIS_OFFSET),
	REG_SEPARATE =                (1 << REG_SEPARATE_OFFSET),
    REG_TSI_DEFINE_G_VALUE =      (1 << REG_TSI_DEFINE_G_VALUE_OFFSET)
} ;

enum RcrOption
{
	TIME_CRITERIA_OFFSET,
	SPEED_CRITERIA_OFFSET,
	DIS_CRITERIA_OFFSET,
	BUTTON_CRITERIA_OFFSET,
	TIME_CRITERIA =				  (1 << TIME_CRITERIA_OFFSET),
	SPEED_CRITERIA =              (1 << SPEED_CRITERIA_OFFSET),
	DIS_CRITERIA =                (1 << DIS_CRITERIA_OFFSET),
	BUTTON_CRITERIA =             (1 << BUTTON_CRITERIA_OFFSET)
} ;

enum LogItemNum
{
	LOG_ITEM1_NUM =               12,
	LOG_ITEM2_NUM =               3,
	TSI_DEFINE_G_NUM =            3,
	LOG_NSAT_NUM =                2,
	LOG_SID_NUM =                 4,
	LOG_SAT_NUM =                 3,
	FIX_OPTION_NUM =              9,
	RCR_OPTION_NUM =              4
} ;

enum LogFixOption
{
	LOG_FIX_INVALID_OFFSET,
	LOG_FIX_SPS_OFFSET,
	LOG_FIX_DGPS_OFFSET,
	LOG_FIX_PPS_OFFSET,
	LOG_FIX_RTK_OFFSET,
	LOG_FIX_FRTK_OFFSET,
	LOG_FIX_ESTIMATED_OFFSET,
	LOG_FIX_MANUAL_INPUT_OFFSET,
	LOG_FIX_SIMULATE_OFFSET,
	LOG_FIX_INVALID = 		      (1 << LOG_FIX_INVALID_OFFSET),
	LOG_FIX_SPS =                 (1 << LOG_FIX_SPS_OFFSET),
	LOG_FIX_DGPS =                (1 << LOG_FIX_DGPS_OFFSET),
	LOG_FIX_PPS =                 (1 << LOG_FIX_PPS_OFFSET),
	LOG_FIX_RTK =                 (1 << LOG_FIX_RTK_OFFSET),
	LOG_FIX_FRTK =                (1 << LOG_FIX_FRTK_OFFSET),
	LOG_FIX_ESTIMATED =           (1 << LOG_FIX_ESTIMATED_OFFSET),
	LOG_FIX_MANUAL_INPUT =        (1 << LOG_FIX_MANUAL_INPUT_OFFSET),
	LOG_FIX_SIMULATE =            (1 << LOG_FIX_SIMULATE_OFFSET)
} ;

enum SatUsedInfo
{
	SAT_NOUSE,
	SAT_INUSE
} ;

enum DynLogEvent
{
	LOG_NOW =               1,
	RCD_FIELD,
	BY_SEC,
	BY_DIS,
	BY_SPD,
	RCD_METHOD,
	LOG_STA
} ;

enum MTLLogKind
{
	NORMAL_DATA,
	EVENT_DATA,
	NULL_DATA,
	UNKNOW_DATA
} ;

//---------------------------------------------------------------------------
typedef struct MTKCmdAck
{
	int logApi ;
	int cmd ;
	int status ;
} MTKCmdAck ;

typedef struct MTKRecInfo
{
	unsigned int curAddr ;
	unsigned int curSection ;
	unsigned int curReadAddr ;
	unsigned int recordCount ;
	unsigned int sec ;
	unsigned int dis ;
	unsigned int spe ;
	unsigned int fmtReg ;
	bool overWrite ;
} MTKRecInfo ;

typedef struct MTKNmeaInfo
{
	int gllNum ;
	int rmcNum ;
	int vtgNum ;
	int ggaNum ;
	int gsaNum ;
	int gsvNum ;
	int zdaNum ;
	int mchnNum ;
	int glonassNum ;
} MTKNmeaInfo ;

typedef struct MTKAckStatus
{
	unsigned int regMethod ;
	unsigned int spi ;
	unsigned int flashId ;
	unsigned int logStatus ;
	MTKCmdAck cmdAck ;
	MTKNmeaInfo nmeaInfo ;
	MTKRecInfo recInfo ;
	unsigned char releaseStr[BASE_DATA_SIZE] ;
	unsigned char buildId[BASE_DATA_SIZE] ;
	unsigned char internal1[BASE_DATA_SIZE] ;
	unsigned char internal2[BASE_DATA_SIZE] ;
	unsigned char readData[MTK_READ_DATA_SIZE] ;
} MTKAckStatus ;

typedef struct MTKNsatInfo
{
	unsigned char satInView ;
	unsigned char satInUse ;
} MTKNsatInfo ;

typedef struct MTKSidInfo
{
	unsigned char id ;     // ID of satellites in view
	unsigned char sat ;    // SAT in use
	unsigned char num ;    // Number of satellites in view
	unsigned char reserve ;
} MTKSidInfo ;

typedef struct MTKSatInfo
{
	MTKSidInfo sid ;
	short ele ;
	short azi ;
	short snr ;
} MTKSatInfo ;

typedef struct MTKNormalLog
{
	short fix ;
	short rcr ;
	short ms ;
	short dsta ;
	short pdop ;           // pdop * 100
	short hdop ;           // hdop * 100
	short vdop ;           // vdop * 100
	short gx;              // TSI Define
	short gy;              // TSI Define
	short gz;              // TSI Define
	float height ;           // meter
	float speed ;            // km / hr
	float track ;          	 // degree
	float dage ;
	time_t utc ;
    MTKNsatInfo nsat ;
	double latitude ;
	double longitude ;
	double dis ;
	MTKSatInfo satInfo[MAX_SAT_NUM] ;
} MTKNormalLog ;

typedef struct MTKEventLog
{
	int id ;
	int data ;
} MTKEventLog ;

typedef union MTKLogData
{
	MTKNormalLog normal ;
	MTKEventLog event ;
} MTKLogData ;

typedef struct MTKLog
{
	unsigned int size ;
	unsigned int format ;
    MTKLogData logInfo ;
} MTKLog ;

const static unsigned int mFormatReg[FORMAT_NUM] = {REG_UTC, REG_VALID, REG_LATITUDE, REG_LONGITUDE, REG_HEIGHT, REG_SPEED, REG_TRACK,
													REG_DSTA, REG_DAGE, REG_PDOP, REG_HDOP, REG_VDOP, REG_NSAT, REG_SID, REG_ELE, REG_AZI,
										   			REG_SNR, REG_RCR, REG_MS, REG_DIS} ;
const static unsigned int mFormatSize[FORMAT_NUM] = {4, 2, 8, 8, 4, 4, 4, 2, 4, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 8} ;

//---------------------------------------------------------------------------
int CreateMTKPkt(unsigned char *writeData, const char *format, ...) ;
int JudgeMTKPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int AnalysisMtkData(unsigned char *buf, int bufSize, MTKAckStatus *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int inline AnalysisPMTKAck(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKSys(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKTxt(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int AnalysisPMTKLog(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int AnalysisNmeaFreq(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQuery(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQuerySpiStatus(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQueryFmtReg(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQuerySec(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQueryDis(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQuerySpe(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQueryRecordMethod(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQueryRecordStatus(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQueryRecordAddr(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQueryFlashId(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogQueryRecordCount(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;
int inline AnalysisPMTKLogReceiveData(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int inline AnalysisReleaseInfo(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo) ;

void RefreshMTKDownloadBin(FILE *readFile, unsigned char *filePath) ;
time_t GetMTKSectorTime(FILE *fp, const int startAddr) ;
bool IsMTKSectValid(FILE *fp, const int startAddr, unsigned int *format) ;
int ParseMTKLogData(FILE *fp, MTKLog *log) ;
bool inline CheckEventData(FILE *fp, MTKLog *log) ;
bool inline CheckNormalData(FILE *fp, MTKLog *log) ;
void inline ReadSatInfo(FILE *fp, unsigned int *checkSum, MTKLog *log) ;
bool inline CalLogCheckSum(unsigned char *buf, unsigned int bufSize, unsigned int *checkSum) ;
bool inline CheckSectorPattern(unsigned char *dataBuf) ;
bool CheckSumMTK(unsigned char *src, const int size) ;
int CalCheckSumMTK(unsigned char *src, const int size) ;

int CreateBootRomPkt(unsigned char *buf, unsigned char *data, unsigned char cmd, int handleAddr, int handelLen) ;
int SendBootRomPkt(HANDLE comPort, FILE *rwFp, unsigned char cmd, int startAddr, int dataSize, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int SetDlda(HANDLE comPort, unsigned char *fileName, int writeAddr, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int JudgeBootRomPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun) ;
int AnalysisBootRomData(unsigned char *buf, int bufSize, MTKAckStatus *ackInfo, FILE *dwFp, UpdateProcessFun callBackUpdateProcessFun) ;
int inline AnalysisBootRomReadCmd(unsigned char *buf, int bufSize, MTKAckStatus *ackInfo, FILE *dwFp, UpdateProcessFun callBackUpdateProcessFun) ;

#endif
