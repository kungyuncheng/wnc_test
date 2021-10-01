//---------------------------------------------------------------------------

#include <stdio.h>

#ifndef MwsLogH
#define MwsLogH
//---------------------------------------------------------------------------

#define SUMMARY_OFFSET      0x20
#define HEADER_OFFSET       0x200
#define TITLE_STR           "UTC,TYPE,LATITUDE,LONGTITUDE,ELEV,HR,PACE,CAD,DIST,OFFSET\n"
#define TITLE_SUMMARY       "Start Time,Total Laps,Total Dist,Total Time,Avg Pace,Avg Speed,Total Calorie,Avg Hr,\
							 Total Asc,Avg Stroke,Avg Swolf,Time Zone(Minute),Daylight Saving,Main Sport,Slave Sport,Sport Mode,\
							 spVal_0,spVal_1,spVal_2,spVal_3,Auto Pause,Power Saving,tsi_ver_0,tsi_ver_1,tsi_ver_2,\
							 ap_ver,stroke_tyle,Total Steps,Dist Per Step,Avg Steps Per Minute,HR Zone1 Count,HR Zone2 Count,\
							 HR Zone3 Count,HR Zone4 Count,HR Zone5 Count\n"
#define TITLE_LAP           "Lap No,DIST,TIME,Pace Speed,Steps Per Lap,Stroke,Turn Time\n"
#define BASE_DATA_SIZE      0x20

#define PI 					3.1415926535898
#define Deg2Rad 			(PI /180.0)
#define EARTH_RADIUS        6378137

//---------------------------------------------------------------------------
enum InfoNumber
{
	SUNNARY_INFO_NUM =         34,
	RUNOUT_INFO_NUM =           7,
	RUNIN_INFO_NUM =            4,
	SWINOUT_INFO_NUM =          5,
	SWININ_INFO_NUM =           3,
	CYCLEOUT_INFO_NUM =         8,
	CYCLEIN_INFO_NUM =          5,
	PAUSE_INFO_NUM =            2,
	RESTART_INFO_NUM =          2,
	LAP_INFO_NUM =             11
};


enum DeviceType
{
	DEVICE_MW,
	DEVICE_MWS
};

// Log Saving
typedef struct SavingLog_1     // Run Out
{
    char type;
    int lat;
    int lon;
    int utc;
    short elev;
    short hr;
    short pace;
}SL_Type1;    //total 19

typedef struct SavingLog_2
{
    char type;
    int lat;
    int lon;
    int utc;
    int dist;
    short elev;
    short hr;
    short cad;
}SL_Type2;    //total 23

typedef struct SavingLog_3
{
    char type;
    int utc;
    int dist;
}SL_Type3;    //total 9

typedef struct SavingLog_4
{
    char type;
    int lat;
    int lon;
    int utc;
    int dist;
}SL_Type4;    //total 17

typedef struct SavingLog_5
{
    char type;
    char transType;    //1 for T1 or 3 for T2
    int utc;
    int dist;
    int transTime;
}SL_Type5;    //total 14

typedef struct SavingLog_6
{
    char type;
    int utc;
    int dist;
    short hr;
}SL_Type6;    //total 11

typedef struct SavingLog_7
{
    char type;
    int utc;
    int dist;
    short hr;
    short cad;
}SL_Type7;    //total 13

typedef struct SavingLog_10
{
    char type;
    int utc;
}SL_Type10;    //total 5

typedef struct SavingLog_13
{
    char type;
    int utc;
}SL_Type13;    //total 5

// Lap Saving
typedef struct SavingLap_
{
    char type;
    char subtype;
    short Lap_Serial;
    int Lap_Dist;
    int Lap_Time;
    int Lap_Pace_Speed;
    int Lap_Steps;
    int Lap_Stroke;
    int Lap_turnTime;
    unsigned int Lap_preAddr;
    unsigned int Lap_nextAddr;
}S_Lap;      //total 36

// Summary
typedef struct SavingSummary_
{
    int Start_Time;
    short Total_Laps;
    int Total_Dist;
    int Total_Time;
    int Avg_Pace;
    int Avg_Speed;
    short Total_Cal;
    short Avg_Hr;
    short Total_Asc;
    short Avg_Stroke;
    short Avg_Swolf;
    short Time_Zone;    //480 for Taiwan
    char Dayight_Saving;    // 0: off, 1:on
    char MainSport;  // 0: running, 1:cycling, 2:swimming, 3:multi-sport
    char SalveSport; // for run 0,1,...,8 for basic train, dist train,..., indoor runing respectively
                     // for cycling 0, 1, ...,3 for basic train, dust train,...,clories train respectively
					 // for swimming 0,1,2 for indoor 50, indoor 25m, outdoor swim
                     // for multi-sport 0:for run, 1:for cycling, 2:swim
	char SportMode;    //0 run, 1 cycling, 2 swim
    unsigned short spVal_0;
    unsigned short spVal_1;
    unsigned short spVal_2;
    unsigned short spVal_3;   // aboce 46 byte
    unsigned char setting;    // bit0: auto pause, bit1: power saving
    char tsi_ver_0;
    char tsi_ver_1;
    char tsi_ver_2;
	int ap_ver;
	char stroke_tyle;    // 0:freestyle, 1:breaststroke, 2:backstroke, 3:butterfly
	int steps;
	int dis_per_step;
	int Avg_steps_per_min;
    int hr_zone_cnt_1;
	int hr_zone_cnt_2;
	int hr_zone_cnt_3;
	int hr_zone_cnt_4;
	int hr_zone_cnt_5;
//    unsigned char 55;    // 55 TSI used
//    unsigned int startAddr;
//    unsigned int endAddr;
}S_Summary;    //total 95
//---------------------------------------------------------------------------

enum LogType
{
	LOG_RUN_OUT =      1,
	LOG_CYCLE_OUT,
	LOG_SWIM_IN,
	LOG_SWIN_OUT,
	LOG_RUN_IN =       6,
	LOG_CYCLE_IN,
	LOG_PAUSE =        10,
	LOG_LAP,
	LOG_RESTART =      13,
	LOG_G_VALUE
};
//---------------------------------------------------------------------------

int ParseLog(FILE *rfp, FILE *wfp, FILE *wGBinfp, FILE *wGCsvfp, int devType, int sportLogType);
int inline ParseLogSummary(FILE* rfp, FILE* wfp, int devType);
int inline ParseLogRunOut(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType);
int inline ParseLogRunIn(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType);
int inline ParseLogSwinOut(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType);
int inline ParseLogSwinIn(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType);
int inline ParseLogCycleOut(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType);
int inline ParseLogCycleIn(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType);
int inline ParseLogPause(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType);
int inline ParseLogRestart(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType);
int inline ParseLogLap(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType);
int inline ParseLogGValue(FILE* rfp, FILE* wGBinfp, FILE* wGCsvfp, int devType, int sportLogType, int *totalGNum);
int ConvertGValueMWS2MW(FILE* rfp, FILE* wfp);
char * MakeTimeStr(int time, char *buf);

void GetLogInfoSummary(unsigned char *buf, S_Summary *info, int devType);
void GetLogInfoRunOut(unsigned char *buf, SL_Type1 *info, int devType);
void GetLogInfoRunIn(unsigned char *buf, SL_Type6 *info, int devType);
void GetLogInfoSwinOut(unsigned char *buf, SL_Type4 *info, int devType);
void GetLogInfoSwinIn(unsigned char *buf, SL_Type3 *info, int devType);
void  GetLogInfoCycleOut(unsigned char *buf, SL_Type2 *info, int devType);
void GetLogInfoCycleIn(unsigned char *buf, SL_Type7 *info, int devType);
void GetLogInfoPause(unsigned char *buf, SL_Type10 *info, int devType);
void GetLogInfoRestart(unsigned char *buf, SL_Type13 *info, int devType);
void GetLogInfoLap(unsigned char *buf, S_Lap *info, int devType);

double DistanceCount(double newLat, double newLon, double oldLat, double oldLon);
int inline GetSummaryTypeSize(void);
int inline GetRunOutTypeSize(void);
int inline GetRunInTypeSize(void);
int inline GetSwinOutTypeSize(void);
int inline GetSwinInTypeSize(void);
int inline GetCycleOutTypeSize(void);
int inline GetCycleInTypeSize(void);
int inline GetPauseTypeSize(void);
int inline GetRestartTypeSize(void);
int inline GetLapTypeSize(void);

#endif
