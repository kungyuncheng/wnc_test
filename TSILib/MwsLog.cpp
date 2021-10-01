//---------------------------------------------------------------------------

#pragma hdrstop
#include <time.h>
#include <vcl.h>

#include "MwsLog.h"
#include "GetToken.h"
#include "Message.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

int mPreTime = 0, mDelayStartTime = 0, mDelayTotalTime = 0;
double mPreLatitude = 0, mPreLontitude = 0;

/**
*/
int ParseLog(FILE *rfp, FILE *wfp, FILE *wGBinfp, FILE *wGCsvfp, int devType, int sportLogType)
{
	unsigned char buf[HEADER_OFFSET] = {"\0"}, logType = 0;
	int status = API_SUCCESS, fileSize = 0, bufSize = 0, offset = 0, totalGNum = 0;
	time_t nowTime = time(0);

	mPreTime = 0;
	mDelayTotalTime = 0;
	mPreLatitude = 0;
	mPreLontitude = 0;

	fread(buf, 1, HEADER_OFFSET, rfp);
	//fwrite(buf, 1, HEADER_OFFSET, wGBinfp);
	fseek(rfp, SUMMARY_OFFSET, SEEK_SET);
	if (API_SUCCESS != (status = ParseLogSummary(rfp, wfp, devType)))
		return status;

	fileSize = FileSize(rfp) ;
	fprintf(wfp, "%s", TITLE_STR);
	fseek(rfp, HEADER_OFFSET, SEEK_SET);
	offset = ftell(rfp);
	while ((API_SUCCESS == status) && (offset < fileSize))
	{
		logType = fgetc(rfp);
		fseek(rfp, -1, SEEK_CUR);
		switch (logType)
		{
			case LOG_RUN_OUT:
				status = ParseLogRunOut(rfp, wfp, /*(logType == sportLogType ? wGBinfp : NULL)*/NULL, devType); break;
			case LOG_CYCLE_OUT:
				status = ParseLogCycleOut(rfp, wfp, /*(logType == sportLogType ? wGBinfp : NULL)*/NULL, devType); break;
			case LOG_SWIM_IN:
				status = ParseLogSwinIn(rfp, wfp, /*(logType == sportLogType ? wGBinfp : NULL)*/NULL, devType); break;
			case LOG_SWIN_OUT:
				status = ParseLogSwinOut(rfp, wfp, /*(logType == sportLogType ? wGBinfp : NULL)*/NULL, devType); break;
			case LOG_RUN_IN:
				status = ParseLogRunIn(rfp, wfp, /*(logType == sportLogType ? wGBinfp : NULL)*/NULL, devType); break;
			case LOG_CYCLE_IN:
				status = ParseLogCycleIn(rfp, wfp, /*(logType == sportLogType ? wGBinfp : NULL)*/NULL, devType); break;
			case LOG_PAUSE:
				status = ParseLogPause(rfp, wfp, /*wGBinfp*/NULL, devType); break;
			case LOG_LAP:
				status = ParseLogLap(rfp, wfp, /*wGBinfp*/NULL, devType); break;
			case LOG_RESTART:
				status = ParseLogRestart(rfp, wfp, /*(logType == sportLogType ? wGBinfp : NULL)*/NULL, devType); break;
			case LOG_G_VALUE:
				status = ParseLogGValue(rfp, wGBinfp, wGCsvfp, devType, sportLogType, &totalGNum); break;
			default:
				//if (DEVICE_MW == devType)
				//{
				//	status = ParseLogGValue(rfp, wGfp, devType); break;
				//} // if
				//else
				fseek(rfp, 1, SEEK_CUR);
		} // switch

		offset = ftell(rfp);
		fflush(wfp);
		fflush(wGBinfp);
		fflush(wGCsvfp);
	} // while

	fprintf(wfp, "Delay Time,%s", MakeTimeStr(mDelayTotalTime, buf));
	fprintf(wGCsvfp, "Total G Num,%d", totalGNum);
	return status;
} // ParseLog

/**
*/
int ParseLogSummary(FILE* rfp, FILE* wfp, int devType)
{
	const static int SPORT_ITEM = 4, SWIM_ITEM = 4;
	const static unsigned char *sportType[SPORT_ITEM] = {"Run", "Cycling", "Swim", "Multi - Sport"};
	const static unsigned char *swimType[SPORT_ITEM] = {"Free Style", "Breast Stroke", "Back Stroke", "Butterfly"};
	S_Summary info;
	unsigned char buf[sizeof(S_Summary)] = {"\0"}, timeBuf[32] = {"\0"}, startTimeBuf[32] = {"\0"}, avgPathBuf[32] = {"\0"}, addrStr[32] = {"\0"};
	int status = API_SUCCESS, readSize = GetSummaryTypeSize();

	fread(buf, 1, readSize, rfp);
	GetLogInfoSummary(buf, &info, devType);

	strftime(startTimeBuf, sizeof(timeBuf), "%X", localtime((time_t *)&info.Start_Time));
	fprintf(wfp, TITLE_SUMMARY);
	fprintf(wfp, "%s,%d,%.4f,%s,%s,%.4f,%d,%d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%s,%s,%d,%d,%d,%d,%s,%d,%.4f,%d,%d,%d,%d,%d,%d\n",
			startTimeBuf, info.Total_Laps, (float)(info.Total_Dist / 10.0), MakeTimeStr(info.Total_Time, timeBuf), MakeTimeStr(info.Avg_Pace, avgPathBuf),
			(float)(info.Avg_Speed / 10.0), info.Total_Cal, info.Avg_Hr, info.Total_Asc, info.Avg_Stroke,
			info.Avg_Swolf, info.Time_Zone, info.Dayight_Saving, sportType[info.MainSport], info.SalveSport,
			info.SportMode, info.spVal_0, info.spVal_1, info.spVal_2, info.spVal_3,
			(info.setting & 0x01) ? "On" : "Off", (info.setting & 0x02) ? "On" : "Off", info.tsi_ver_0, info.tsi_ver_1, info.tsi_ver_2,
			info.ap_ver, swimType[info.stroke_tyle], info.steps, (float)(info.dis_per_step / 100.0), info.Avg_steps_per_min,
			info.hr_zone_cnt_1, info.hr_zone_cnt_2, info.hr_zone_cnt_3, info.hr_zone_cnt_4, info.hr_zone_cnt_5);

	return status;
} // ParseLogSummary

/**
*/
int ParseLogRunOut(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType)
{
	SL_Type1 info;
	static double totalDis = 0;
	unsigned char buf[sizeof(SL_Type1)] = {"\0"}, timeBuf[BASE_DATA_SIZE] = {"\0"}, addrStr[BASE_DATA_SIZE] = {"\0"};
	int status = API_SUCCESS, readSize = GetRunOutTypeSize();
	double dis = 0, caloByDis = 0, caloByHr = 0, calo = 0;

	sprintf(addrStr, "0x%08X", ftell(rfp));

	fread(buf, 1, readSize, rfp);
	if (NULL != wGBinfp)
		fwrite(buf, 1, readSize, wGBinfp);

	GetLogInfoRunOut(buf, &info, devType);

	if (0 == mPreLatitude && 0 == mPreLontitude)
	{
		mPreTime = info.utc;
		totalDis = 0;
	} // if
	else
		totalDis += (dis = DistanceCount((info.lat / 1000000.0), (info.lon / 1000000.0), mPreLatitude, mPreLontitude));

	caloByDis = (dis / 1000.0) * 60 * 0.90;
	caloByHr = ((-55.0969 + (0.6309 * 91) + (0.1988 * 60) + (0.2017 * 40)) / 4.184) / 60 * (info.utc - mPreTime);
	calo = (caloByDis > caloByHr ? caloByDis : caloByHr);

	mPreLatitude = (info.lat / 1000000.0);
	mPreLontitude = (info.lon / 1000000.0);
	mPreTime = info.utc;

	strftime(timeBuf, sizeof(timeBuf), "%X", localtime((time_t *)&info.utc));
	fprintf(wfp, "%s,%s,%.8f,%.8f,%.4f,%d,%d,,%f,%s,%f\n", timeBuf, "Run Out", (double)(info.lat / 1000000.0), (double)(info.lon / 1000000.0),
													   (float)(info.elev / 10.0), info.hr, info.pace, totalDis, addrStr, calo);
	return status;
} // ParseLogRunOut

/**
*/
int ParseLogRunIn(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType)
{
	SL_Type6 info;
	unsigned char buf[sizeof(SL_Type6)] = {"\0"}, timeBuf[BASE_DATA_SIZE] = {"\0"}, addrStr[BASE_DATA_SIZE] = {"\0"};
    int status = API_SUCCESS, readSize = GetRunInTypeSize();

	sprintf(addrStr, "0x%08X", ftell(rfp));
   
	fread(buf, 1, readSize, rfp);
	if (NULL != wGBinfp)
		fwrite(buf, 1, readSize, wGBinfp);

	GetLogInfoRunIn(buf, &info, devType);

	strftime(timeBuf, sizeof(timeBuf), "%X", localtime((time_t *)&info.utc));
	fprintf(wfp, "%s,%s,,,,%d,,,%.4f,%s,\n", timeBuf, "Run In", info.hr, (float)(info.dist / 10.0), addrStr);

	return status;
} // ParseLogRunIn

/**
*/
int ParseLogSwinOut(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType)
{
	SL_Type4 info;
	unsigned char buf[sizeof(SL_Type4)] = {"\0"}, timeBuf[BASE_DATA_SIZE] = {"\0"}, addrStr[BASE_DATA_SIZE] = {"\0"};
	int status = API_SUCCESS, readSize = GetSwinOutTypeSize();

	sprintf(addrStr, "0x%08X", ftell(rfp));

	fread(buf, 1, readSize, rfp);
	if (NULL != wGBinfp)
		fwrite(buf, 1, readSize, wGBinfp);

	GetLogInfoSwinOut(buf, &info, devType);

	strftime(timeBuf, sizeof(timeBuf), "%X", localtime((time_t *)&info.utc));
	fprintf(wfp, "%s,%s,%.8f,%.8f,,,,,%.4f,%s,\n", timeBuf, "Swin Out", (double)(info.lat / 1000000.0), (double)(info.lon / 1000000.0), (float)(info.dist / 10.0), addrStr);

	return status;
} // ParseLogSwinOut

/**
*/
int ParseLogSwinIn(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType)
{
	SL_Type3 info;
	unsigned char buf[sizeof(SL_Type3)] = {"\0"}, timeBuf[BASE_DATA_SIZE] = {"\0"}, addrStr[BASE_DATA_SIZE] = {"\0"};
	int status = API_SUCCESS, readSize = GetSwinInTypeSize();

	sprintf(addrStr, "0x%08X", ftell(rfp));

	fread(buf, 1, readSize, rfp);
	if (NULL != wGBinfp)
		fwrite(buf, 1, readSize, wGBinfp);

	GetLogInfoSwinIn(buf, &info, devType);

	strftime(timeBuf, sizeof(timeBuf), "%X", localtime((time_t *)&info.utc));
	fprintf(wfp, "%s,%s,,,,,,,%.4f,%s,\n", timeBuf, "Swin In", (float)(info.dist / 10.0), addrStr);

	return status;
} // ParseLogSwinIn

/**
*/
int ParseLogCycleOut(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType)
{
	SL_Type2 info;
	unsigned char buf[sizeof(SL_Type2)] = {"\0"}, timeBuf[BASE_DATA_SIZE] = {"\0"}, addrStr[BASE_DATA_SIZE] = {"\0"};
	int status = API_SUCCESS, readSize = GetCycleOutTypeSize();

	sprintf(addrStr, "0x%08X", ftell(rfp));

	fread(buf, 1, readSize, rfp);
	if (NULL != wGBinfp)
		fwrite(buf, 1, readSize, wGBinfp);

	GetLogInfoCycleOut(buf, &info, devType);

	strftime(timeBuf, sizeof(timeBuf), "%X", localtime((time_t *)&info.utc));
	fprintf(wfp, "%s,%s,%.8f,%.8f,%.4f,%d,,%d,%.4f,%s,\n", timeBuf, "Cycle Out", (double)(info.lat / 1000000.0), (double)(info.lon / 1000000.0),
														   (float)(info.elev / 10.0), info.hr, info.cad, (float)(info.dist / 10.0), addrStr);

	return status;
} // ParseLogCycleOut

/**
*/
int ParseLogCycleIn(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType)
{
	SL_Type7 info;
	unsigned char buf[sizeof(SL_Type7)] = {"\0"}, timeBuf[BASE_DATA_SIZE] = {"\0"}, addrStr[BASE_DATA_SIZE] = {"\0"};
	int status = API_SUCCESS, readSize = GetCycleInTypeSize();

	sprintf(addrStr, "0x%08X", ftell(rfp));

	fread(buf, 1, readSize, rfp);
	if (NULL != wGBinfp)
		fwrite(buf, 1, readSize, wGBinfp);

	GetLogInfoCycleIn(buf, &info, devType);

	strftime(timeBuf, sizeof(timeBuf), "%X", localtime((time_t *)&info.utc));
	fprintf(wfp, "%s,%s,,,,%d,,%d,%.4f,%s,\n", timeBuf, "Cycle In", info.hr, info.cad, (float)(info.dist / 10.0), addrStr);

	return status;
} // ParseLogCycleIn

/**
*/
int ParseLogPause(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType)
{
	SL_Type10 info;
	unsigned char buf[sizeof(SL_Type10)] = {"\0"}, timeBuf[BASE_DATA_SIZE] = {"\0"}, addrStr[BASE_DATA_SIZE] = {"\0"};
	int status = API_SUCCESS, readSize = GetPauseTypeSize();

	sprintf(addrStr, "0x%08X", ftell(rfp));
	
	fread(buf, 1, readSize, rfp);
	if (NULL != wGBinfp)
		fwrite(buf, 1, readSize, wGBinfp);

	GetLogInfoPause(buf, &info, devType);

	mDelayStartTime = info.utc;
	strftime(timeBuf, sizeof(timeBuf), "%X", localtime((time_t *)&info.utc));
	fprintf(wfp, "%s,%s,,,,,,,,%s,\n", timeBuf, "Pause", addrStr);

	return status;
} // ParseLogPause

/**
*/
int ParseLogRestart(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType)
{
	SL_Type13 info;
	unsigned char buf[sizeof(SL_Type13)] = {"\0"}, timeBuf[BASE_DATA_SIZE] = {"\0"}, addrStr[BASE_DATA_SIZE] = {"\0"};
	int status = API_SUCCESS, readSize = GetRestartTypeSize();

	sprintf(addrStr, "0x%08X", ftell(rfp));

	fread(buf, 1, readSize, rfp);
	if (NULL != wGBinfp)
		fwrite(buf, 1, readSize, wGBinfp);

	GetLogInfoRestart(buf, &info, devType);

	mDelayTotalTime += (info.utc - mDelayStartTime);
	strftime(timeBuf, sizeof(timeBuf), "%X", localtime((time_t *)&info.utc));
	fprintf(wfp, "%s,%s,,,,,,,,%s,\n", timeBuf, "Restart", addrStr);

	return status;
} // ParseLogRestart

/**
*/
int ParseLogLap(FILE* rfp, FILE* wfp, FILE *wGBinfp, int devType)
{
	S_Lap info;
	unsigned char buf[sizeof(S_Lap)] = {"\0"}, timeBuf[BASE_DATA_SIZE] = {"\0"}, paceBuf[BASE_DATA_SIZE] = {"\0"}, addrStr[BASE_DATA_SIZE] = {"\0"};
	int status = API_SUCCESS, readSize = GetLapTypeSize();

	sprintf(addrStr, "0x%08X", ftell(rfp));

	fread(buf, 1, readSize, rfp);
	if (NULL != wGBinfp)
		fwrite(buf, 1, readSize, wGBinfp);

	GetLogInfoLap(buf, &info, devType);

	fprintf(wfp, "%s", TITLE_LAP);
	fprintf(wfp, "%d,%.4f,%s,%s,%d,%d,%d,,,%s,\n", info.Lap_Serial, (float)(info.Lap_Dist / 10), MakeTimeStr(info.Lap_Time, timeBuf), MakeTimeStr(info.Lap_Pace_Speed, paceBuf),
												   info.Lap_Steps, info.Lap_Stroke, info.Lap_turnTime, addrStr);
	return status;
} // ParseLogLap

/**
*/
int ParseLogGValue(FILE* rfp, FILE* wGBinfp, FILE* wGCsvfp, int devType, int sportLogType, int *totalGNum)
{
	const static int G_AXIS = 3;
	short gBuf[G_AXIS] = {0};
	int status = API_SUCCESS;
	unsigned char gNum = 0;

	if (DEVICE_MWS == devType)
		fseek(rfp, 1, SEEK_CUR);

	gNum = fgetc(rfp);
	//fputc((gNum = fgetc(rfp)), wGBinfp);
	if (0 == gNum)
		goto END_HANDLE;
/*
	fprintf(wfp, "%d,", gNum);
	if (0 == (gNum = fgetc(rfp)))
		goto END_HANDLE;

	fprintf(wfp, "%d,", gNum);
	if (0 == (gNum = fgetc(rfp)))
		goto END_HANDLE;
*/
	*totalGNum += gNum;
	fprintf(wGCsvfp, "%d,\n", gNum);
	for (int i = 0 ; i < gNum ; ++i)
	{
		fread(&gBuf, sizeof(gBuf), 1, rfp);
		/*              // AP 端已經交換過
		if (DEVICE_MWS == devType)
		{
			switch (sportLogType)
			{
				case LOG_SWIM_IN:
					gBuf[0] ^= gBuf[1];
					gBuf[1] ^= gBuf[0];
					gBuf[0] ^= gBuf[1];
					gBuf[0] >>= 2;           // mws 轉成 mw 格式
					gBuf[1] >>= 2;
					gBuf[2] >>= 2;
					break;
				case LOG_RUN_OUT:
					gBuf[0] >>= 5;           // mws 轉成 mw 格式
					gBuf[1] >>= 5;
					gBuf[2] >>= 5;
					break;
            } // switch
		} // if     */

		fwrite(&gBuf, sizeof(gBuf), 1, wGBinfp);
		fprintf(wGCsvfp, "%d,%d,%d,\n", gBuf[0], gBuf[1], gBuf[2]);
    } // for
END_HANDLE:
	return status;
} // ParseLogGValue

/**
*/
int ConvertGValueMWS2MW(FILE* rfp, FILE* wfp)
{
	unsigned char buf[SUMMARY_OFFSET + HEADER_OFFSET] = {"\0"}, logType = 0, gNum = 0;
	int offset = 0, bufSize = 0, fileSize = FileSize(rfp);

    fseek(rfp, 0, SEEK_SET);
	fseek(wfp, 0, SEEK_SET);

	fread(buf, (SUMMARY_OFFSET + HEADER_OFFSET), 1, rfp);
	fwrite(buf, (SUMMARY_OFFSET + HEADER_OFFSET), 1, wfp);

	while (offset < fileSize)
	{
		logType = fgetc(rfp);
		fseek(rfp, -1, SEEK_CUR);
		switch (logType)
		{
			case LOG_RUN_OUT:
				fread(buf, bufSize = GetRunOutTypeSize(), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
			case LOG_CYCLE_OUT:
				fread(buf, bufSize = GetCycleOutTypeSize(), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
			case LOG_SWIM_IN:
				fread(buf, bufSize = GetSwinInTypeSize(), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
			case LOG_SWIN_OUT:
				fread(buf, bufSize = GetSwinOutTypeSize(), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
			case LOG_RUN_IN:
				fread(buf, bufSize = GetRunInTypeSize(), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
			case LOG_CYCLE_IN:
				fread(buf, bufSize = GetCycleInTypeSize(), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
			case LOG_PAUSE:
				fread(buf, bufSize = GetPauseTypeSize(), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
			case LOG_LAP:
				fread(buf, bufSize = GetLapTypeSize(), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
			case LOG_RESTART:
				fread(buf, bufSize = GetRestartTypeSize(), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
			case LOG_G_VALUE:
				fseek(rfp, 1, SEEK_CUR);
				gNum = fgetc(rfp);
				fwrite(&gNum, sizeof(gNum), 1, wfp);
				for (int i = 0 ; i < gNum ; ++i)
				{
					short x = 0, y = 0, z = 0;

					fread(&x, sizeof(x), 1, rfp);
					fread(&y, sizeof(y), 1, rfp);
					fread(&z, sizeof(z), 1, rfp);

					x ^= y;
					y ^= x;
					x ^= y;

					x >>= 2;
					y >>= 2;
					z >>= 2;

					fwrite(&x, sizeof(x), 1, wfp);
				    fwrite(&y, sizeof(y), 1, wfp);
					fwrite(&z, sizeof(z), 1, wfp);
                } // for
				break;
			default:
				fread(buf, bufSize = sizeof(char), 1, rfp);
				fwrite(buf, bufSize, 1, wfp);
				break;
		} // switch

		offset = ftell(rfp);
	} // while

	return API_SUCCESS;
} // ConvertGValueMWS2MW

/**
*/
char * MakeTimeStr(int time, char *buf)
{
	int hour = 0, minute = 0, remaind = 0;

	hour = time / 3600;
	remaind = time % 3600;
	minute = remaind / 60;
	remaind = remaind % 60;
	sprintf(buf, "%d:%02d:%02d", hour, minute, remaind);

	return buf;
} // CountDelayTime

/**
*/
void GetLogInfoSummary(unsigned char *buf, S_Summary *info, int devType)
{
	void *item[SUNNARY_INFO_NUM] = {(void *)&info->Start_Time, (void *)&info->Total_Laps, (void *)&info->Total_Dist, (void *)&info->Total_Time,
									(void *)&info->Avg_Pace, (void *)&info->Avg_Speed, (void *)&info->Total_Cal, (void *)&info->Avg_Hr,
									(void *)&info->Total_Asc, (void *)&info->Avg_Stroke, (void *)&info->Avg_Swolf, (void *)&info->Time_Zone,
									(void *)&info->Dayight_Saving, (void *)&info->MainSport, (void *)&info->SalveSport, (void *)&info->SportMode,
									(void *)&info->spVal_0, (void *)&info->spVal_1, (void *)&info->spVal_2, (void *)&info->spVal_3,
									(void *)&info->setting, (void *)&info->tsi_ver_0, (void *)&info->tsi_ver_1, (void *)&info->tsi_ver_2,
									(void *)&info->ap_ver, (void *)&info->stroke_tyle, (void *)&info->steps, (void *)&info->dis_per_step,
									(void *)&info->Avg_steps_per_min, (void *)&info->hr_zone_cnt_1, (void *)&info->hr_zone_cnt_2, (void *)&info->hr_zone_cnt_3,
									(void *)&info->hr_zone_cnt_4, (void *)&info->hr_zone_cnt_5};
	int itemSize[SUNNARY_INFO_NUM] = {sizeof(info->Start_Time), sizeof(info->Total_Laps), sizeof(info->Total_Dist), sizeof(info->Total_Time),
									  sizeof(info->Avg_Pace), sizeof(info->Avg_Speed), sizeof(info->Total_Cal), sizeof(info->Avg_Hr),
									  sizeof(info->Total_Asc), sizeof(info->Avg_Stroke), sizeof(info->Avg_Swolf), sizeof(info->Time_Zone),
									  sizeof(info->Dayight_Saving), sizeof(info->MainSport), sizeof(info->SalveSport), sizeof(info->SportMode),
									  sizeof(info->spVal_0), sizeof(info->spVal_1), sizeof(info->spVal_2), sizeof(info->spVal_3),
									  sizeof(info->setting), sizeof(info->tsi_ver_0), sizeof(info->tsi_ver_1), sizeof(info->tsi_ver_2),
									  sizeof(info->ap_ver), sizeof(info->stroke_tyle), sizeof(info->steps), sizeof(info->dis_per_step),
									  sizeof(info->Avg_steps_per_min), sizeof(info->hr_zone_cnt_1), sizeof(info->hr_zone_cnt_2), sizeof(info->hr_zone_cnt_3),
									  sizeof(info->hr_zone_cnt_4), sizeof(info->hr_zone_cnt_5)};

	for (int i = 0, offset = 0 ; i < SUNNARY_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoSummary

/**
*/
void GetLogInfoRunOut(unsigned char *buf, SL_Type1 *info, int devType)
{
	void *item[RUNOUT_INFO_NUM] = {(void *)&info->type, (void *)&info->lat, (void *)&info->lon, (void *)&info->utc,
								   (void *)&info->elev, (void *)&info->hr, (void *)&info->pace};
	int itemSize[RUNOUT_INFO_NUM] = {sizeof(info->type), sizeof(info->lat), sizeof(info->lon), sizeof(info->utc),
									 sizeof(info->elev), sizeof(info->hr), sizeof(info->pace)};

	for (int i = 0, offset = 0 ; i < RUNOUT_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoSummary

/**
*/
void GetLogInfoRunIn(unsigned char *buf, SL_Type6 *info, int devType)
{
	void *item[RUNIN_INFO_NUM] = {(void *)&info->type, (void *)&info->utc, (void *)&info->dist, (void *)&info->hr};
	int itemSize[RUNIN_INFO_NUM] = {sizeof(info->type), sizeof(info->utc), sizeof(info->dist), sizeof(info->hr)};

	for (int i = 0, offset = 0 ; i < RUNIN_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoRunIn

/**
*/
void GetLogInfoSwinOut(unsigned char *buf, SL_Type4 *info, int devType)
{
	void *item[SWINOUT_INFO_NUM] = {(void *)&info->type, (void *)&info->lat, (void *)&info->lon, (void *)&info->utc, (void *)&info->dist};
	int itemSize[SWINOUT_INFO_NUM] = {sizeof(info->type), sizeof(info->lat), sizeof(info->lon), sizeof(info->utc), sizeof(info->dist)};

	for (int i = 0, offset = 0 ; i < SWINOUT_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoSwinOut

/**
*/
void GetLogInfoSwinIn(unsigned char *buf, SL_Type3 *info, int devType)
{
	void *item[SWININ_INFO_NUM] = {(void *)&info->type, (void *)&info->utc, (void *)&info->dist};
	int itemSize[SWININ_INFO_NUM] = {sizeof(info->type), sizeof(info->utc), sizeof(info->dist)};

	for (int i = 0, offset = 0 ; i < SWININ_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoSwinIn

/**
*/
void GetLogInfoCycleOut(unsigned char *buf, SL_Type2 *info, int devType)
{
	void *item[CYCLEOUT_INFO_NUM] = {(void *)&info->type, (void *)&info->lat, (void *)&info->lon, (void *)&info->utc, (void *)&info->dist,
									 (void *)&info->elev, (void *)&info->hr, (void *)&info->cad};
	int itemSize[CYCLEOUT_INFO_NUM] = {sizeof(info->type), sizeof(info->lat), sizeof(info->lon), sizeof(info->utc), sizeof(info->dist),
									   sizeof(info->elev), sizeof(info->hr), sizeof(info->cad)};

	for (int i = 0, offset = 0 ; i < CYCLEOUT_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoCycleOut

/**
*/
void GetLogInfoCycleIn(unsigned char *buf, SL_Type7 *info, int devType)
{
	void *item[CYCLEIN_INFO_NUM] = {(void *)&info->type, (void *)&info->utc, (void *)&info->dist, (void *)&info->hr, (void *)&info->cad};
	int itemSize[CYCLEIN_INFO_NUM] = {sizeof(info->type), sizeof(info->utc), sizeof(info->dist), sizeof(info->hr), sizeof(info->cad)};

	for (int i = 0, offset = 0 ; i < CYCLEIN_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoCycleIn

/**
*/
void GetLogInfoPause(unsigned char *buf, SL_Type10 *info, int devType)
{
	void *item[PAUSE_INFO_NUM] = {(void *)&info->type, (void *)&info->utc};
	int itemSize[PAUSE_INFO_NUM] = {sizeof(info->type), sizeof(info->utc)};

	for (int i = 0, offset = 0 ; i < PAUSE_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoPause

/**
*/
void GetLogInfoRestart(unsigned char *buf, SL_Type13 *info, int devType)
{
	void *item[RESTART_INFO_NUM] = {(void *)&info->type, (void *)&info->utc};
	int itemSize[RESTART_INFO_NUM] = {sizeof(info->type), sizeof(info->utc)};

	for (int i = 0, offset = 0 ; i < RESTART_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoRestart

/**
*/
void GetLogInfoLap(unsigned char *buf, S_Lap *info, int devType)
{
	void *item[LAP_INFO_NUM] = {(void *)&info->type, (void *)&info->subtype, (void *)&info->Lap_Serial, (void *)&info->Lap_Dist,
								(void *)&info->Lap_Time, (void *)&info->Lap_Pace_Speed, (void *)&info->Lap_Steps, (void *)&info->Lap_Stroke,
								(void *)&info->Lap_turnTime, (void *)&info->Lap_preAddr, (void *)&info->Lap_nextAddr};
	int itemSize[LAP_INFO_NUM] = {sizeof(info->type), sizeof(info->subtype), sizeof(info->Lap_Serial), sizeof(info->Lap_Dist),
								  sizeof(info->Lap_Time), sizeof(info->Lap_Pace_Speed), sizeof(info->Lap_Steps), sizeof(info->Lap_Stroke),
								  sizeof(info->Lap_turnTime), sizeof(info->Lap_preAddr), sizeof(info->Lap_nextAddr)};

	for (int i = 0, offset = 0 ; i < LAP_INFO_NUM ; offset += itemSize[i], ++i)
		memcpy(item[i], buf + offset, itemSize[i]);
} // GetLogInfoLap

/**
*/
double DistanceCount(double newLat, double newLon, double oldLat, double oldLon)
{
	double radLat1 = newLat * Deg2Rad;
	double radLat2 = oldLat * Deg2Rad;
	double radLon1 = newLon * Deg2Rad;
	double radLon2 = oldLon * Deg2Rad;
	double disLat  = radLat1 - radLat2;
	double disLon  = radLon1 - radLon2;
	double dis2Point = 0;

	dis2Point = 2 * asin(sqrt(pow(sin(disLat / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(disLon / 2), 2)));
	dis2Point *= EARTH_RADIUS;

	return dis2Point;
} // DistanceCount

/**
*/
int GetSummaryTypeSize(void)
{
	S_Summary info;
	int itemSize[SUNNARY_INFO_NUM] = {sizeof(info.Start_Time), sizeof(info.Total_Laps), sizeof(info.Total_Dist), sizeof(info.Total_Time),
									  sizeof(info.Avg_Pace), sizeof(info.Avg_Speed), sizeof(info.Total_Cal), sizeof(info.Avg_Hr),
									  sizeof(info.Total_Asc), sizeof(info.Avg_Stroke), sizeof(info.Avg_Swolf), sizeof(info.Time_Zone),
									  sizeof(info.Dayight_Saving), sizeof(info.MainSport), sizeof(info.SalveSport), sizeof(info.SportMode),
									  sizeof(info.spVal_0), sizeof(info.spVal_1), sizeof(info.spVal_2), sizeof(info.spVal_3),
									  sizeof(info.setting), sizeof(info.tsi_ver_0), sizeof(info.tsi_ver_1), sizeof(info.tsi_ver_2),
									  sizeof(info.ap_ver), sizeof(info.stroke_tyle), sizeof(info.steps), sizeof(info.dis_per_step),
									  sizeof(info.Avg_steps_per_min), sizeof(info.hr_zone_cnt_1), sizeof(info.hr_zone_cnt_2), sizeof(info.hr_zone_cnt_3),
									  sizeof(info.hr_zone_cnt_4), sizeof(info.hr_zone_cnt_5)}, size = 0;

	for (int i = 0 ; i < SUNNARY_INFO_NUM ; ++i)
		size += itemSize[i];      // sizeof(S_Summary) 因為記憶體配置 aligment 後大小不一定等於 member總和

	return size;
} // GetSummaryTypeSize

/**
*/
int GetRunOutTypeSize(void)
{
	SL_Type1 info;
	int itemSize[RUNOUT_INFO_NUM] = {sizeof(info.type), sizeof(info.lat), sizeof(info.lon), sizeof(info.utc),
									 sizeof(info.elev), sizeof(info.hr), sizeof(info.pace)}, size = 0;


    for (int i = 0 ; i < RUNOUT_INFO_NUM ; ++i)
		size += itemSize[i];

	return size;
} // GetRunOutTypeSize

/**
*/
int GetRunInTypeSize(void)
{
	SL_Type6 info;
	int itemSize[RUNIN_INFO_NUM] = {sizeof(info.type), sizeof(info.utc), sizeof(info.dist), sizeof(info.hr)}, size = 0;

    for (int i = 0 ; i < RUNIN_INFO_NUM ; ++i)
		size += itemSize[i];

	return size;
} // GetRunInTypeSize

/**
*/
int GetSwinOutTypeSize(void)
{
	SL_Type4 info;
	int itemSize[SWINOUT_INFO_NUM] = {sizeof(info.type), sizeof(info.lat), sizeof(info.lon), sizeof(info.utc), sizeof(info.dist)}, size = 0;

	for (int i = 0 ; i < SWINOUT_INFO_NUM ; ++i)
		size += itemSize[i];

	return size;
} // GetSwinOutTypeSize

/**
*/
int GetSwinInTypeSize(void)
{
	SL_Type3 info;
	int itemSize[SWININ_INFO_NUM] = {sizeof(info.type), sizeof(info.utc), sizeof(info.dist)}, size = 0;

	for (int i = 0 ; i < SWININ_INFO_NUM ; ++i)
		size += itemSize[i];

	return size;
} // GetSwinInTypeSize

/**
*/
int GetCycleOutTypeSize(void)
{
	SL_Type2 info;
	int itemSize[CYCLEOUT_INFO_NUM] = {sizeof(info.type), sizeof(info.lat), sizeof(info.lon), sizeof(info.utc), sizeof(info.dist),
									   sizeof(info.elev), sizeof(info.hr), sizeof(info.cad)}, size = 0;

	for (int i = 0 ; i < CYCLEOUT_INFO_NUM ; ++i)
		size += itemSize[i];

	return size;
} // GetCycleOutTypeSize

/**
*/
int GetCycleInTypeSize(void)
{
	SL_Type7 info;
	int itemSize[CYCLEIN_INFO_NUM] = {sizeof(info.type), sizeof(info.utc), sizeof(info.dist), sizeof(info.hr), sizeof(info.cad)}, size = 0;

	for (int i = 0 ; i < CYCLEIN_INFO_NUM ; ++i)
		size += itemSize[i];

	return size;
} // GetCycleInTypeSize

/**
*/
int GetPauseTypeSize(void)
{
	SL_Type10 info;
	int itemSize[PAUSE_INFO_NUM] = {sizeof(info.type), sizeof(info.utc)}, size = 0;

	for (int i = 0 ; i < PAUSE_INFO_NUM ; ++i)
		size += itemSize[i];

	return size;
} // GetPauseTypeSize

/**
*/
int GetRestartTypeSize(void)
{
	SL_Type13 info;
	int itemSize[RESTART_INFO_NUM] = {sizeof(info.type), sizeof(info.utc)}, size = 0;

	for (int i = 0 ; i < RESTART_INFO_NUM ; ++i)
		size += itemSize[i];

	return size;
} // GetRestartTypeSize

/**
*/
int GetLapTypeSize(void)
{
	S_Lap info;
	int itemSize[LAP_INFO_NUM] = {sizeof(info.type), sizeof(info.subtype), sizeof(info.Lap_Serial), sizeof(info.Lap_Dist),
								  sizeof(info.Lap_Time), sizeof(info.Lap_Pace_Speed), sizeof(info.Lap_Steps), sizeof(info.Lap_Stroke),
								  sizeof(info.Lap_turnTime), sizeof(info.Lap_preAddr), sizeof(info.Lap_nextAddr)}, size = 0;

	for (int i = 0 ; i < LAP_INFO_NUM ; ++i)
		size += itemSize[i];

	return size;
} // GetLapTypeSize






