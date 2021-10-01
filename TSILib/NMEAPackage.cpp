//---------------------------------------------------------------------------

#pragma hdrstop

#include "NMEAPackage.h"
#include "Message.h"
#include "GetToken.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

unsigned char *mCompareList = ",*" ;

/**
*/
int MtkPktToNmea(FILE *fp, MTKLog log, int event)
{
	const unsigned char *title = "[Time]" ;
	MtkPktToNmeaFun mtkPktToNmeaFun[NUM_OF_NMEA] = {MtkPktToNmeaGGA, MtkPktToNmeaGLL, MtkPktToNmeaGSA,
												    MtkPktToNmeaGSV, MtkPktToNmeaRMC, MtkPktToNmeaVTG} ;

	fprintf(fp, "%s\r\n", title) ;
	for (int i = NMEA_GGA_OFFSET ; i <= NMEA_VTG_OFFSET ; ++i)
	{
		if (event & (1 << i))
		{
			mtkPktToNmeaFun[i](fp, log) ;
			fflush(fp) ;
        } // if
    } // for
} // MtkPktToNmea

/**
*/
int inline MtkPktToNmeaGLL(FILE *fp, MTKLog log)
{
	const int ITEM = 2 ;
	int bufSize = 0, fixStatus[ITEM] = {LOG_FIX_INVALID, LOG_FIX_SPS} ; ;
	unsigned char buf[BASE_DATA_SIZE * 4] = {"\0"}, *title = "$GPGLL", timeBuf[BASE_DATA_SIZE] = {"\0"} ;
	unsigned char fixStr[ITEM] = {'V', 'A'} ;
    MTKNormalLog selfLog ;

	memcpy(&selfLog, &log.logInfo.normal, sizeof(selfLog)) ;
	bufSize = strftime(timeBuf, sizeof(timeBuf), "%X\0", gmtime(&selfLog.utc)) ;   // localtime : 當地
	for (int i = 0 ; i < bufSize ; ++i)
	{
		if (':' == timeBuf[i])
		{
			memcpy(timeBuf + i, timeBuf + (i + 1), (bufSize - (i + 1))) ;
			timeBuf[--bufSize] = '\0' ;
        } // if
	} // for

	sprintf(timeBuf + bufSize, ".%03d", selfLog.ms) ;

	bufSize = sprintf(buf, "%s,", title) ;
	bufSize += sprintf(buf + bufSize, "%08.06f,%c,", DegreeToMinute(selfLog.latitude), (DegreeToMinute(selfLog.latitude) > 0 ? 'N' : 'S')) ;
	bufSize += sprintf(buf + bufSize, "%09.06f,%c,", DegreeToMinute(selfLog.longitude), (DegreeToMinute(selfLog.longitude) > 0 ? 'E' : 'W')) ;
    bufSize += sprintf(buf + bufSize, "%s,", timeBuf) ;
    for (int i = 0 ; i < ITEM ; ++i)
	{
		if (selfLog.fix & fixStatus[i])
			bufSize += sprintf(buf + bufSize, "%c,", fixStr[i]) ;
	} // for

	bufSize += sprintf(buf + bufSize, "A*") ;
	bufSize += sprintf(buf + bufSize, "%02X\r\n", CalCheckSumMTK(buf, bufSize)) ;

	fprintf(fp, "%s", buf) ;
	return API_SUCCESS ;
} // MtkPktToNmeaGLL

/**
*/
int inline MtkPktToNmeaRMC(FILE *fp, MTKLog log)
{
	const int ITEM = 2 ;
	int bufSize = 0, fixStatus[ITEM] = {LOG_FIX_INVALID, LOG_FIX_SPS} ;
	unsigned char buf[BASE_DATA_SIZE * 4] = {"\0"}, *title = "$GPRMC", timeBuf[BASE_DATA_SIZE] = {"\0"} ;
	unsigned char fixStr[ITEM] = {'V', 'A'} ;
	MTKNormalLog selfLog ;

	memcpy(&selfLog, &log.logInfo.normal, sizeof(selfLog)) ;
	bufSize = strftime(timeBuf, sizeof(timeBuf), "%X\0", gmtime(&selfLog.utc)) ;
	for (int i = 0 ; i < bufSize ; ++i)
	{
		if (':' == timeBuf[i])
		{
			memcpy(timeBuf + i, timeBuf + (i + 1), (bufSize - (i + 1))) ;
			timeBuf[--bufSize] = '\0' ;
        } // if
	} // for

	sprintf(timeBuf + bufSize, ".%03d", selfLog.ms) ;

	bufSize = sprintf(buf, "%s,%s,", title, timeBuf) ;
	for (int i = 0 ; i < ITEM ; ++i)
	{
		if (selfLog.fix & fixStatus[i])
			bufSize += sprintf(buf + bufSize, "%c,", fixStr[i]) ;
	} // for

	bufSize += sprintf(buf + bufSize, "%08.06f,%c,", DegreeToMinute(selfLog.latitude), (DegreeToMinute(selfLog.latitude) > 0 ? 'N' : 'S')) ;
	bufSize += sprintf(buf + bufSize, "%09.06f,%c,", DegreeToMinute(selfLog.longitude), (DegreeToMinute(selfLog.longitude) > 0 ? 'E' : 'W')) ;
	bufSize += sprintf(buf + bufSize, "%05.01f,", selfLog.speed * KM_TO_MILE * MPH_TO_KNOT) ;
	bufSize += sprintf(buf + bufSize, "%05.01f,", selfLog.track) ;

	strftime(timeBuf, sizeof(timeBuf), "%d%m%y\0", gmtime(&selfLog.utc)) ;
	bufSize += sprintf(buf + bufSize, "%s,,,*", timeBuf) ;
	bufSize += sprintf(buf + bufSize, "%02X\r\n", CalCheckSumMTK(buf, bufSize)) ;

	fprintf(fp, "%s", buf) ;
	return API_SUCCESS ;
} // MtkPktToNmeaRMC

/**
*/
int inline MtkPktToNmeaGSV(FILE *fp, MTKLog log)
{
	const int ITEM = 2, targetSatStatus[ITEM] = {NMEA_GPS, NMEA_GLONASS} ;
	unsigned char buf[BASE_DATA_SIZE * 4] = {"\0"}, *title[ITEM] = {"$GPGSV", "$GLGSV"} ;
	int vIdx = MAX_SAT_NUM, lIdx = 1, satStatus[MAX_SAT_NUM] = {0}, satNum[ITEM] = {0, 0} ;
	int	bufSize = 0, titleOffset = 0, lOffset = 0, totalLine = 0 ;
	bool remaindData = false, isTargetSat = false ;
	MTKNormalLog selfLog ;

	memcpy(&selfLog, &log.logInfo.normal, sizeof(selfLog)) ;
	qsort(selfLog.satInfo, MAX_SAT_NUM, sizeof(MTKSatInfo), SatIdSort) ;  // quick sort 衛星ID
	for (int i = 0 ; i < MAX_SAT_NUM ; ++i)
	{
		if (0 != selfLog.satInfo[i].sid.id)
		{
			if (i < vIdx)
				vIdx = i ;

			if ((selfLog.satInfo[i].sid.id <= GPS_ID_MAX) ||
				(GPS_SUP_ID_MIN <= selfLog.satInfo[i].sid.id && selfLog.satInfo[i].sid.id <= GPS_SUP_ID_MAX) ||
				(SBAS_ID_MIN <= selfLog.satInfo[i].sid.id && selfLog.satInfo[i].sid.id <= SBAS_ID_MAX))
			{
				satStatus[i] = NMEA_GPS ;
				++satNum[0] ;   // GSV sat num
			} // if

			if (GLO_ID_MIN <= selfLog.satInfo[i].sid.id && selfLog.satInfo[i].sid.id <= GLO_ID_MAX)
			{
				satStatus[i] = NMEA_GLONASS ;
				++satNum[1] ;   // GLO sat num
            }
		} // if
	} // for

	for (int i = 0, satInUse = 0 ; i < ITEM ; ++i) //  GPGSV & GLGSV
	{
		satInUse = 0 ;
		bufSize = titleOffset = 0 ;
		lIdx = 1 ;
		totalLine = (satNum[i] / SATELLITE_NUM) + (0 == (satNum[i] % SATELLITE_NUM) ? 0 : 1) ;
		bufSize += (lOffset = sprintf(buf + bufSize, "%s,%d,", title[i], totalLine)) ; // line idx 之後才填入 先將offset位置預留
		titleOffset = (bufSize += (sprintf(buf + bufSize, "%d,%02d", lIdx, satNum[i]))) ;
		for (int j = vIdx, satNum = 0 ; j <= MAX_SAT_NUM ; ++j)
		{
			if (targetSatStatus[i] == satStatus[j])
			{
            	remaindData = true ;
				bufSize += sprintf(buf + bufSize, ",%02d,%02d,%03d,%02d", selfLog.satInfo[j].sid.id, selfLog.satInfo[j].ele,
																		  selfLog.satInfo[j].azi, selfLog.satInfo[j].snr) ;
				if (SATELLITE_NUM == ++satNum)
				{
					buf[lOffset] = 0x30 + lIdx++ ;   // 字元轉數字
					bufSize += sprintf(buf + bufSize, "%c", '*') ;
					bufSize += sprintf(buf + bufSize, "%02X\r\n", CalCheckSumMTK(buf, bufSize)) ;
					buf[bufSize] = '\0' ;
					fprintf(fp, "%s", buf) ;

					remaindData = false ;
					bufSize = titleOffset ;
					satNum = 0 ;
				} // if
			} // if
		} // for

		if (remaindData)    // 剩餘不足四個衛星資訊
		{
			buf[lOffset] = 0x30 + lIdx++ ;
			bufSize += sprintf(buf + bufSize, "%c", '*') ;
			bufSize += sprintf(buf + bufSize, "%02X\r\n", CalCheckSumMTK(buf, bufSize)) ;
			buf[bufSize] = '\0' ;
			fprintf(fp, "%s", buf) ;

			remaindData = false ;
		} // if
	} // for

	return API_SUCCESS ;
} // MtkPktToNmeaGSV

/**
*/
int inline MtkPktToNmeaGSA(FILE *fp, MTKLog log)
{
 	unsigned char buf[BASE_DATA_SIZE * 4] = {"\0"}, *title = "$GPGSA" ;
	int bufSize = 0, titleOffset = 0, satUseNum = 0, fixOption = 0 ;
	MTKNormalLog selfLog ;
    memcpy(&selfLog, &log.logInfo.normal, sizeof(selfLog)) ;

	bufSize = (titleOffset = sprintf(buf, "%s,A,", title)) ; // 3D fix 計算完使用的衛星數後才知道 先預留空位
	bufSize += sprintf(buf + bufSize, "%d,", 0) ;
	for (int i = 0 ; i < MAX_SAT_NUM && satUseNum < MAX_SATELLITE_NUM ; ++i)
	{
		if (/*selfLog.satInfo[i].sid.id < GPS_ID_THRESHOLD && */SAT_INUSE == selfLog.satInfo[i].sid.sat)
		{
			bufSize += sprintf(buf + bufSize, "%02d,", selfLog.satInfo[i].sid.id) ;
			++satUseNum ;
        } // if
	} // for

	for (int i = 0 ; i < (MAX_SATELLITE_NUM - satUseNum) ; ++i)
    	bufSize += sprintf(buf + bufSize, ",") ;

	if (0 == satUseNum)
		fixOption = 1 ;
	else if (1 <= satUseNum && satUseNum < 4)
		fixOption = 2 ;
	else
		fixOption = 3 ;

	buf[titleOffset] = 0x30 + fixOption ;
	bufSize += sprintf(buf + bufSize, "%03.1f,", ((float)selfLog.pdop / 100.0)) ;
	bufSize += sprintf(buf + bufSize, "%03.1f,", ((float)selfLog.hdop / 100.0)) ;
	bufSize += sprintf(buf + bufSize, "%03.1f*", ((float)selfLog.vdop / 100.0)) ;
  	bufSize += sprintf(buf + bufSize, "%02X\r\n", CalCheckSumMTK(buf, bufSize)) ;

	fprintf(fp, "%s", buf) ;
	return API_SUCCESS ;
} // MtkPktToNmeaGSA

/**
*/
int inline MtkPktToNmeaVTG(FILE *fp, MTKLog log)
{
	unsigned char buf[BASE_DATA_SIZE * 4] = {"\0"}, *title = "$GPVTG" ;
	int bufSize = 0 ;
	MTKNormalLog selfLog ;
    memcpy(&selfLog, &log.logInfo.normal, sizeof(selfLog)) ;

	bufSize = sprintf(buf, "%s,%06.2f,T,,M,%05.1f,N,%05.1f,K*", title, selfLog.track, (selfLog.speed * KM_TO_MILE * MPH_TO_KNOT), selfLog.speed) ;
	bufSize += sprintf(buf + bufSize, "%02X\r\n", CalCheckSumMTK(buf, bufSize)) ;

    fprintf(fp, "%s", buf) ;
	return API_SUCCESS ;
} // MtkPktToNmeaVTG

/**
*/
int inline MtkPktToNmeaGGA(FILE *fp, MTKLog log)
{
	const int ITEM = 9 ;
	unsigned char buf[BASE_DATA_SIZE * 4] = {"\0"}, *title = "$GPGGA", timeBuf[BASE_DATA_SIZE] = {"\0"} ;
	int bufSize = 0 ;
	int fixStatus[ITEM] = {LOG_FIX_INVALID, LOG_FIX_SPS, LOG_FIX_DGPS, LOG_FIX_PPS, LOG_FIX_RTK,
						   LOG_FIX_FRTK, LOG_FIX_ESTIMATED, LOG_FIX_MANUAL_INPUT, LOG_FIX_SIMULATE} ;
	MTKNormalLog selfLog ;

	memcpy(&selfLog, &log.logInfo.normal, sizeof(selfLog)) ;
	bufSize = strftime(timeBuf, sizeof(timeBuf), "%X\0", gmtime(&selfLog.utc)) ;
	for (int i = 0 ; i < bufSize ; ++i)
	{
		if (':' == timeBuf[i])
		{
			memcpy(timeBuf + i, timeBuf + (i + 1), (bufSize - (i + 1))) ;
			timeBuf[--bufSize] = '\0' ;
        } // if
	} // for

	sprintf(timeBuf + bufSize, ".%03d", selfLog.ms) ;

    bufSize = sprintf(buf, "%s,%s,", title, timeBuf) ;
	bufSize += sprintf(buf + bufSize, "%011.06f,%c,", DegreeToMinute(selfLog.latitude), (DegreeToMinute(selfLog.latitude) > 0 ? 'N' : 'S')) ;
	bufSize += sprintf(buf + bufSize, "%012.06f,%c,", DegreeToMinute(selfLog.longitude), (DegreeToMinute(selfLog.longitude) > 0 ? 'E' : 'W')) ;
	if (0 == selfLog.fix)
    	selfLog.fix = LOG_FIX_SPS ; // 會log基本上都是有定位到, 做假的fix值

	for (int i = 0 ; i < ITEM ; ++i)
	{
		if (selfLog.fix & fixStatus[i])
			bufSize += sprintf(buf + bufSize, "%d,", i) ;
	} // for

	bufSize += sprintf(buf + bufSize, "%02d,", selfLog.nsat.satInUse) ;
	bufSize += sprintf(buf + bufSize, "%03.1f,", ((float)selfLog.hdop / 100.0)) ;
	bufSize += sprintf(buf + bufSize, "%07.3f,M,0.0,M,", selfLog.height) ;
	bufSize += sprintf(buf + bufSize, /*"%02d,", selfLog.dsta*/"0.0,") ;
	bufSize += sprintf(buf + bufSize, /*"%02.1f*", selfLog.dage*/"0*") ;
    bufSize += sprintf(buf + bufSize, "%02X\r\n", CalCheckSumMTK(buf, bufSize)) ;

	fprintf(fp, "%s", buf) ;
	return API_SUCCESS ;
} // MtkPktToNmeaGGA

/**
*/
int inline SatIdSort(const void *element1, const void *element2)
{
	return (((MTKSatInfo *)element1)->sid.id - ((MTKSatInfo *)element2)->sid.id) ;
} // SortCompare

/**
*/
double inline DegreeToMinute(double degree)
{
	double fPart = degree - (int)degree ;

    return ((100 * (int)degree) + (fPart * DEGREE_TO_MINUTE)) ;
} // DegreeToMinute

/**
*/
int AnalysisNMEAData(unsigned char *buf, int bufSize, NmeaInfo *nmeaInfo)
{
	int status = API_FAIL, offSet = 0 ;

	if (0 == strncmp(buf, "$GPRMC", sizeof("$GPRMC") - 1) || 0 == strncmp(buf, "$GNRMC", sizeof("$GNRMC") - 1))
	{
	    offSet = sizeof("$GPRMC") - 1 ;
		status = AnalysisRMCData(buf + offSet, bufSize - offSet, nmeaInfo) ;
    } // if
	else if (0 == strncmp(buf, "$GPGSA", sizeof("$GPGSA") - 1) || 0 == strncmp(buf, "$GNGSA", sizeof("$GNGSA") - 1))
	{
		offSet = sizeof("$GPGSA") - 1 ;
		status = AnalysisGSAData(buf + offSet, bufSize - offSet, nmeaInfo) ;
    } // else if
	else if (0 == strncmp(buf, "$GPGSV", sizeof("$GPGSV") - 1) || 0 == strncmp(buf, "$GLGSV", sizeof("$GLGSV") - 1))
	{
		offSet = sizeof("$GPGSV") - 1 ;
		status = AnalysisGSVData(buf + offSet, bufSize - offSet, nmeaInfo) ;
    } // else if

	return status ;
} // AnalysisNMEAData

/**
*/
int AnalysisRMCData(unsigned char *buf, int bufSize, NmeaInfo *nmeaInfo)
{
	int tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"}, title[TOKEN_SIZE] = {"\0"} ;

	//memset(&ackInfo->ackStatus.extra.nmea.type.rmc, 0x00, sizeof(RMCInfo)) ;
	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	strcpy(title, token) ;
	nmeaInfo->id = NMEA_RMC_OFFSET ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.utc = safe_atoi(token) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.dataStatus = token[0] ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.latitude = safe_atof(token) ;

    GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.latitude *= (0 == strcmp(token, "S") ? -1 : 1) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.longitude = safe_atof(token) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.longitude *= (0 == strcmp(token, "W") ? -1 : 1) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.speed = safe_atof(token) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.trackAngle = safe_atof(token) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.date = safe_atoi(token) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.magneticVariation = safe_atof(token) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.rmc.magneticVariationUnit = token[0] ;

	return API_SUCCESS ;
} // AnalysisRMCData

/**
*/
int AnalysisGSAData(unsigned char *buf, int bufSize, NmeaInfo *nmeaInfo)
{
	int tokenSize = 0, svId = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"}, title[TOKEN_SIZE] = {"\0"} ;

	//memset(&ackInfo->ackStatus.extra.nmea.type.gsa, 0x00, sizeof(GSAInfo)) ;
	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	strcpy(title, token) ;
	nmeaInfo->id = NMEA_GSA_OFFSET ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.gsa.fixAction = token[0] ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.gsa.fixStatus = safe_atoi(token) ;

	for (int i = 0, j = 0 ; i < MAX_SATELLITE_NUM ; ++i)
	{
		GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
        if (0 != (svId = safe_atoi(token)))
			nmeaInfo->type.gsa.prnNumber[j++] = svId ;
	} // for

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.gsa.pdop = safe_atof(token) ;

    GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.gsa.hdop = safe_atof(token) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	nmeaInfo->type.gsa.vdop = safe_atof(token) ;

	return API_SUCCESS ;
} // AnalysisGSAData

/**
*/
int AnalysisGSVData(unsigned char *buf, int bufSize, NmeaInfo *nmeaInfo)
{
	int tokenSize = 0, totalSen = 0, nowSen = 0, totalSat = 0, satNum = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"}, title[TOKEN_SIZE] = {"\0"} ;

	//memset(&ackInfo->ackStatus.extra.nmea.type.gsv, 0x00, sizeof(GSVInfo)) ;
	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	strcpy(title, token) ;
	nmeaInfo->id = NMEA_GSV_OFFSET ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	totalSen = nmeaInfo->type.gsv.totalSentence = safe_atoi(token) ;

   	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
    nowSen = nmeaInfo->type.gsv.nowSentence = safe_atoi(token) ;

    GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	totalSat = nmeaInfo->type.gsv.totalSatellite = safe_atoi(token) ;

	satNum = (nowSen < totalSen ? SATELLITE_NUM : (totalSat - (totalSen - 1) * SATELLITE_NUM)) ;
	for (int i = 0 ; i < satNum ; ++i)
	{
		GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
		nmeaInfo->type.gsv.satelliteInfo[i].prnNumber = safe_atoi(token) ;

		GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
		nmeaInfo->type.gsv.satelliteInfo[i].elevation = safe_atoi(token) ;

		GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
		nmeaInfo->type.gsv.satelliteInfo[i].azimuth = safe_atoi(token) ;

		GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
		nmeaInfo->type.gsv.satelliteInfo[i].snr = safe_atoi(token) ;
    } // for

 	return API_SUCCESS ;
} // AnalysisGSVData

/**
*@brief call back function
*/
int JudgeNMEAPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char *pktStartPtr = NULL, *pktEndPtr = NULL ;
	int status = API_FAIL, offset = 0, pktSize = 0 ;

	while (API_SUCCESS != status &&             // 可能收到包含其他類別的 package
		  (NULL != (pktStartPtr = strchr_bysize((char *)comInfo->resBuf + offset, '$', comInfo->resBufSize - offset))) &&         // 先確定有開頭字元
		  (NULL != (pktEndPtr = strchr_bysize(pktStartPtr, '\n', comInfo->resBufSize - (offset += pktStartPtr - &comInfo->resBuf[offset])))))   // 從開頭字元的位置往後找結尾字元
	{
		pktSize = pktEndPtr - &comInfo->resBuf[offset] + 1 ;
		if (CheckSumMTK(comInfo->resBuf + offset, pktSize))
			status = AnalysisNMEAData(comInfo->resBuf + offset, pktSize, (NmeaInfo *)ackInfo) ;

		offset += pktSize ;
	} // while

	if (0 < (comInfo->resBufSize -= offset))      // 剩餘不足一個pkt的資料搬到buffer最前端
		memcpy(comInfo->resBuf, comInfo->resBuf + offset, comInfo->resBufSize) ;

	return status ;
} // JudgeTSIPkt

