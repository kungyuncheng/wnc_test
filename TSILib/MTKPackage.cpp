//---------------------------------------------------------------------------

#pragma hdrstop

#include <string.h>

#include "MTKPackage.h"
#include "Message.h"
#include "GetToken.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

unsigned char *mCompareList = ",*" ;

/**
*/
int CreateMTKPkt(unsigned char *writeData, const char *format, ...)
{
	int offset = 0 ;
	va_list args ;

	va_start(args, format) ;
	offset = vsprintf(writeData + offset, format, args) ;
	va_end(args) ;

	offset += sprintf(writeData + offset, "%02X\r\n", CalCheckSumMTK(writeData, offset)) ;

	return offset ;
} // CreateMTKPkt

/**
*@brief call back function
*/
int JudgeMTKPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char *pktStartPtr = NULL, *pktEndPtr = NULL ;
	int status = API_FAIL, offset = 0, pktSize = 0 ;

	while (API_SUCCESS != status &&             // 可能收到包含其他類別的 package
		  (NULL != (pktStartPtr = strchr_bysize((char *)comInfo->resBuf + offset, '$', comInfo->resBufSize - offset))) &&   // 先確定有開頭字元
		  (NULL != (pktEndPtr = strchr_bysize(pktStartPtr, '\n', comInfo->resBufSize - (offset += pktStartPtr - &comInfo->resBuf[offset])))))  // 從開頭字元的位置往後找結尾字元
	{
		pktSize = pktEndPtr - &comInfo->resBuf[offset] + 1 ;
		if (0 == strncmp(comInfo->resBuf + offset, "$PMTK", strlen("$PMTK") - 1) && CheckSumMTK(comInfo->resBuf + offset, pktSize))
		{    // 確定是 $PMTK 開頭的才分析
			status = AnalysisMtkData(comInfo->resBuf + offset, pktSize, (MTKAckStatus *)ackInfo, dwFp, processBar, callBackUpdateProcessFun) ;
			if (MTK_NEED_ACK_PACKAGE & comInfo->pktType)
				status = (ACK_SUCCEEDED == status ? API_SUCCESS : API_FAIL) ;
		} // if

		offset += pktSize ;                     // 不管是否 MTK 的 package 都要跳過
	} // while

	if (0 < (comInfo->resBufSize -= offset))      // 剩餘不足一個pkt的資料搬到buffer最前端
		memcpy(comInfo->resBuf, comInfo->resBuf + offset, comInfo->resBufSize) ;

	return status ;
} // JudgeMTKPkt

/**
*/
int AnalysisMtkData(unsigned char *buf, int bufSize, MTKAckStatus *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int mtkOptionId = 0, status = API_FAIL, offSet = sizeof("$PMTK") - 1, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	buf += offSet ;
	bufSize -= offSet ;
	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	mtkOptionId = safe_atoi((char *)token) ;
	switch (mtkOptionId)
	{
		case PMTK_ACK :
			status = AnalysisPMTKAck(buf, bufSize, &offSet, ackInfo) ;
			break ;
		case PMTK_SYS_MSG :
			status = AnalysisPMTKSys(buf, bufSize, &offSet, ackInfo) ;
			break ;
		case PMTK_TXT_MSG :
			status = AnalysisPMTKTxt(buf, bufSize, &offSet, ackInfo) ;
			break ;
		case PMTK_LOG :
			status = AnalysisPMTKLog(buf, bufSize, &offSet, ackInfo, dwFp, processBar, callBackUpdateProcessFun) ;
			break ;
		case PMTK_DT_NMEA_OUTPUT :
			status = AnalysisNmeaFreq(buf, bufSize, &offSet, ackInfo) ;
			break ;
		case PMTK_DT_RELEASE :
			status = AnalysisReleaseInfo(buf, bufSize, &offSet, ackInfo) ;
			break ;
	} // switch

	return status ;
} // AnalysisMtkData

/**
*/
int inline AnalysisPMTKAck(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	ackInfo->cmdAck.logApi = safe_atoi((char *)token) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	ackInfo->cmdAck.cmd = safe_atoi((char *)token) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	ackInfo->cmdAck.status = safe_atoi((char *)token) ;

	return ackInfo->cmdAck.status ;
} // AnalysisPMTKAck

/**
*/
int inline AnalysisPMTKSys(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	ackInfo->cmdAck.status = atoi(token) ;

	return ackInfo->cmdAck.status ;
} // AnalysisPMTKSys

/**
*/
int inline AnalysisPMTKTxt(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->readData, token, tokenSize) ;

	status = (0 == strcmp(ackInfo->readData, "MTKGPS") ? API_SUCCESS : API_FAIL) ;
    return status ;
} // AnalysisPMTKTxt

/**
*   TSI 也有 182 指令
*/
int AnalysisPMTKLog(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS, logOption = 0, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	logOption = safe_atoi((char *)token) ;
	switch (logOption)
	{
		case LOG_RETURN :
			status = AnalysisPMTKLogQuery(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_OUTPUT :
			status = AnalysisPMTKLogReceiveData(buf, bufSize, offSet, ackInfo, dwFp, processBar, callBackUpdateProcessFun) ;
			break ;
	} // switch

	return status ;
} // AnalysisPMTKLog

/**
*/
int AnalysisNmeaFreq(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	const int ITEM = 19 ;
	int status = API_SUCCESS, tokenSize = 0, reserve = 0 ;
	int *nmeaVal[ITEM] = {&ackInfo->nmeaInfo.gllNum, &ackInfo->nmeaInfo.rmcNum, &ackInfo->nmeaInfo.vtgNum,
						  &ackInfo->nmeaInfo.ggaNum, &ackInfo->nmeaInfo.gsaNum, &ackInfo->nmeaInfo.gsvNum,
						  &reserve, &reserve, &reserve, &reserve, &reserve, &reserve, &reserve, &reserve, &reserve, &reserve, &reserve,
						  &ackInfo->nmeaInfo.zdaNum, &ackInfo->nmeaInfo.mchnNum} ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	for (int i = 0 ; i < ITEM ; ++i)
	{
		GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
		*nmeaVal[i] = atoi(token) ;
    } // for

    return status ;
} // AnalysisNmeaFreq

/**
*/
int inline AnalysisPMTKLogQuery(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, logQueryOption = 0, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	logQueryOption = safe_atoi((char *)token) ;
	switch(logQueryOption)
	{
		case LOG_RET_SPI_STATUS :
			status = AnalysisPMTKLogQuerySpiStatus(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_FMT_REG :
			status = AnalysisPMTKLogQueryFmtReg(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_SECOND :
			status = AnalysisPMTKLogQuerySec(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_DISTANCE :
			status = AnalysisPMTKLogQueryDis(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_SPEED :
			status = AnalysisPMTKLogQuerySpe(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_REC_METHOD :
			status = AnalysisPMTKLogQueryRecordMethod(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_STATUS :
			status = AnalysisPMTKLogQueryRecordStatus(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_REC_ADDR :
			status = AnalysisPMTKLogQueryRecordAddr(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_FLASH_ID :
			status = AnalysisPMTKLogQueryFlashId(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_REC_COUNT :
			status = AnalysisPMTKLogQueryRecordCount(buf, bufSize, offSet, ackInfo) ;
			break ;
		case LOG_RET_REC_FSECTOR :
			break ;
		case LOG_RET_VERSION :
			break ;
	} // switch

	return status ;
} // AnalysisPMTKLogQuery

/**
*/
int inline AnalysisPMTKLogQuerySpiStatus(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%x", &ackInfo->spi) ;

	return status ;
} // AnalysisPMTKLogQuerySpiStatus

/**
*/
int inline AnalysisPMTKLogQueryFmtReg(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%x", &ackInfo->recInfo.fmtReg) ;

	return status ;
} // AnalysisPMTKLogQueryFmtReg

/**
*/
int inline AnalysisPMTKLogQuerySec(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%d", &ackInfo->recInfo.sec) ;

	return status ;
} // AnalysisPMTKLogQuerySec

/**
*/
int inline AnalysisPMTKLogQueryDis(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%d", &ackInfo->recInfo.dis) ;

	return status ;
} // AnalysisPMTKLogQueryDis

/**
*/
int inline AnalysisPMTKLogQuerySpe(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
 	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%d", &ackInfo->recInfo.spe) ;

	return status ;
} // AnalysisPMTKLogQuerySpe

/**
*/
int inline AnalysisPMTKLogQueryRecordMethod(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%x", &ackInfo->regMethod) ;

	return status ;
} // AnalysisPMTKLogQueryRecentAddr

/**
*/
int inline AnalysisPMTKLogQueryRecordStatus(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
    safe_sscanf(token, "%d", &ackInfo->logStatus) ;

	return status ;
} // AnalysisPMTKLogQueryRecordStatus

/**
*/
int inline AnalysisPMTKLogQueryRecordAddr(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%x", &ackInfo->recInfo.curAddr) ;
	ackInfo->recInfo.curSection = (ackInfo->recInfo.curAddr / AP_BLOCK_SIZE) + (0 == (ackInfo->recInfo.curAddr % AP_BLOCK_SIZE) ? 0 : 1) ;

	return status ;
} // AnalysisPMTKLogQueryRecentAddr

/**
*/
int inline AnalysisPMTKLogQueryFlashId(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%x", &ackInfo->flashId) ;

	return status ;
} // AnalysisPMTKLogQueryFlashId

/**
*/
int inline AnalysisPMTKLogQueryRecordCount(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%x", &ackInfo->recInfo.recordCount) ;

	return status ;
} // AnalysisPMTKLogQueryRecentCount

/**
*/
int inline AnalysisPMTKLogReceiveData(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[MTK_READ_DATA_SIZE * 2] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf(token, "%x", &ackInfo->recInfo.curReadAddr) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	HexToBin((char*)token, ackInfo->readData, MTK_READ_DATA_SIZE) ;

	if (NULL != dwFp)  // if ackInfo->dwLogFp equal NULL means just read data, no need to save as a file.
		fwrite(ackInfo->readData, MTK_READ_DATA_SIZE, 1, dwFp) ;

	if (NULL != callBackUpdateProcessFun)
		callBackUpdateProcessFun(processBar, ackInfo->recInfo.curReadAddr, ((ackInfo->recInfo.curSection + 1) * AP_BLOCK_SIZE)) ;

	return status ;
} // AnalysisPMTKLogReceiveData

/**
*/
int inline AnalysisReleaseInfo(unsigned char *buf, int bufSize, int *offSet, MTKAckStatus *ackInfo)
{
	unsigned char token[TOKEN_SIZE] = {"\0"} ;
	int status = API_SUCCESS, tokenSize = 0 ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->releaseStr, token, tokenSize) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->buildId, token, tokenSize) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memset(ackInfo->internal1, 0x00, BASE_DATA_SIZE) ;
	memcpy(ackInfo->internal1, token, tokenSize) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memset(ackInfo->internal2, 0x00, BASE_DATA_SIZE) ;
	memcpy(ackInfo->internal2, token, tokenSize) ;

	return status ;
} // AnalysisReleaseInfo

/**
*/
void RefreshMTKDownloadBin(FILE *readFile, unsigned char *filePath)
{
	const int fileSize = FileSize(readFile), numOfSector = (fileSize / AP_BLOCK_SIZE) + (0 == fileSize % AP_BLOCK_SIZE ? 0 : 1) ;
	int overWriteAddr = 0, curAddr = 0, rwSize = 0, reserveSize = 0 ;

	time_t timeBuf = 0, preTimeBuf = 0 ;

	for (int i = 0 ; i < numOfSector ; ++i)
	{
		curAddr = i * AP_BLOCK_SIZE ;
		timeBuf = GetMTKSectorTime(readFile, curAddr) ;
		if (-1 != timeBuf)
		{
			if (timeBuf < preTimeBuf)
				overWriteAddr = curAddr ;
			preTimeBuf = timeBuf ;
		} // if
	} // for

	if (0 == overWriteAddr)
	{
		fclose(readFile) ;
		return ;
	} // if
#ifdef REFRESH_LOG_BIN
	SwapFileData(readFile, 0, overWriteAddr, fileSize) ;
#endif
	fclose(readFile) ;
	readFile = NULL ;
} // RefreshMTKDownloadBin

/**
*/
time_t GetMTKSectorTime(FILE *fp, const int startAddr)
{
	unsigned int logOption = UNKNOW_DATA ;
	time_t sectorTime = -1 ;
	MTKLog log ;

	if (!IsMTKSectValid(fp, startAddr, &log.format))
    	return -1 ;

	fseek(fp, startAddr + SECTOR_PATTERN_SIZE, SEEK_SET) ;
	do
	{
		switch (logOption = ParseMTKLogData(fp, &log))
		{
			case UNKNOW_DATA :
			case NULL_DATA :
				return -1 ;
			case EVENT_DATA :
				if (RCD_FIELD == log.logInfo.event.id)
					log.format = log.logInfo.event.data ;
				break ;
			case NORMAL_DATA :
				return log.logInfo.normal.utc ;
        } // switch
	}	while (EVENT_DATA == logOption) ;
} // GetMTKSectorTime

/**
*/
bool IsMTKSectValid(FILE *fp, const int startAddr, unsigned int *format)
{
	char sectorPatternBuf[SECTOR_PATTERN_SIZE] = {"\0"} ;
	int offset = 2 ;

    fseek(fp, startAddr, SEEK_SET) ;
	fread(sectorPatternBuf, sizeof(sectorPatternBuf), 1, fp) ;
	if (!CheckSectorPattern(sectorPatternBuf))
		return false ;

	memcpy(format, sectorPatternBuf + offset, sizeof(format)) ;
	if (0xffffffff == *format)
		return false ;

	return true ;
} // IsMTKSectValid

/**
*/
int ParseMTKLogData(FILE *fp, MTKLog *log)
{
	const int EVENT_OPTION = 0xaaaaaaaa, NULL_OPTION = 0xffffffff ;
	int option = 0, format = log->format ;

	memset(log, 0x00, sizeof(MTKLog)) ;
    log->format = format ;

	fread(&option, 1, sizeof(option), fp) ;
	fseek(fp, -1 * sizeof(option), SEEK_CUR) ;
	switch (option)
	{
		case EVENT_OPTION :
			return CheckEventData(fp, log) ? EVENT_DATA : UNKNOW_DATA ;
		case NULL_OPTION :
			return NULL_DATA ;
		default :
			return CheckNormalData(fp, log) ? NORMAL_DATA : UNKNOW_DATA ;
	} // switch
} // ParseMTKData

/**
*/
bool inline CheckEventData(FILE *fp, MTKLog *log)
{
	const int EVENT_DATA_SIZE = 16 ;
	unsigned char buf[EVENT_DATA_SIZE] = {"\0"} ;

	fread(buf, 1, EVENT_DATA_SIZE, fp) ;
	for (int i = 0 ; i < EVENT_DATA_SIZE ; ++i)
	{
		if ((i <= 6 && 0xaa != buf[i]) || (i >= 12 && 0xbb != buf[i]))
		{
			fseek(fp, -1 * EVENT_DATA_SIZE, SEEK_CUR) ;
			return false ;
        } // if
    } // for

	log->size = EVENT_DATA_SIZE ;
	log->logInfo.event.id = buf[7] ;
	memcpy(&log->logInfo.event.data, &buf[8], sizeof(int)) ;
	return true ;
} // CheckEventData

/**
*/
bool inline CheckNormalData(FILE *fp, MTKLog *log)
{
	unsigned int calCheckSum = 0 ;
	unsigned short checkSum = 0 ;
	void *buf1[LOG_ITEM1_NUM] = {(void *)&log->logInfo.normal.utc, (void *)&log->logInfo.normal.fix, (void *)&log->logInfo.normal.latitude,
								 (void *)&log->logInfo.normal.longitude, (void *)&log->logInfo.normal.height, (void *)&log->logInfo.normal.speed,
								 (void *)&log->logInfo.normal.track, (void *)&log->logInfo.normal.dsta, (void *)&log->logInfo.normal.dage,
					 			 (void *)&log->logInfo.normal.pdop, (void *)&log->logInfo.normal.hdop, (void *)&log->logInfo.normal.vdop} ;
	void *buf2[LOG_ITEM2_NUM] = {(void *)&log->logInfo.normal.rcr, (void *)&log->logInfo.normal.ms, (void *)&log->logInfo.normal.dis} ;
	void *buf3[TSI_DEFINE_G_NUM] = {(void *)&log->logInfo.normal.gx, (void *)&log->logInfo.normal.gy, (void *)&log->logInfo.normal.gz} ;

	for (int i = REG_UTC_OFFSET ; i <= REG_VDOP_OFFSET ; ++i)
	{
		if (mFormatReg[i] & log->format)
		{
			fread(buf1[i], 1, mFormatSize[i], fp) ;
			CalLogCheckSum((unsigned char *)buf1[i], mFormatSize[i], &calCheckSum) ;
			log->size += mFormatSize[i] ;
		} // if
	} // for

	ReadSatInfo(fp, &calCheckSum, log) ;
    for (int i = REG_RCR_OFFSET ; i <= REG_DIS_OFFSET ; ++i)
	{
		if (mFormatReg[i] & log->format)
		{
			fread(buf2[i - REG_RCR_OFFSET], 1, mFormatSize[i], fp) ;
			CalLogCheckSum((unsigned char *)buf2[i - REG_RCR_OFFSET], mFormatSize[i], &calCheckSum) ;
			log->size += mFormatSize[i] ;
		} // if
	} // for

	if (REG_TSI_DEFINE_G_VALUE & log->format)
	{
		for (int i = 0 ; i < TSI_DEFINE_G_NUM ; ++i)
		{
			fread(buf3[i], 1, sizeof(short), fp) ;
			CalLogCheckSum((unsigned char *)buf3[i], sizeof(short), &calCheckSum) ;
			log->size += sizeof(short) ;
        } // for
    } // if

	fread(&checkSum, 1, sizeof(checkSum), fp) ;
	log->size += sizeof(checkSum) ;
	if ('*' != (checkSum & 0xff) || calCheckSum != ((checkSum & 0xff00) >> BITS_OF_BYTE))
	{   // checksum 不正確 回到開始位置
		fseek(fp, -1 * log->size, SEEK_CUR) ;
		log->size = 0 ;
		return false ;
	} // if

	return true ;
} // CheckNormalData

/**
*/
void inline ReadSatInfo(FILE *fp, unsigned int *checkSum, MTKLog *log)
{
	void *nsatBuf[LOG_NSAT_NUM] = {(void *)&log->logInfo.normal.nsat.satInView, (void *)&log->logInfo.normal.nsat.satInUse} ;
	void *sidBuf[LOG_SID_NUM] = {NULL} ;
	void *satBuf[LOG_SAT_NUM] = {NULL} ;
	int satIdx = 0 ;

	if (REG_NSAT & log->format)
	{
		for (int i = 0 ; i < LOG_NSAT_NUM ; ++i)
		{
			fread(nsatBuf[i], 1, sizeof(unsigned char), fp) ;
			CalLogCheckSum((unsigned char *)nsatBuf[i], sizeof(unsigned char), checkSum) ;
        } // for

		log->size += mFormatSize[REG_NSAT_OFFSET] ;
    } // if

	do
	{
		if (REG_SID & log->format)
		{
			sidBuf[0] = (void *)&log->logInfo.normal.satInfo[satIdx].sid.id ;
			sidBuf[1] = (void *)&log->logInfo.normal.satInfo[satIdx].sid.sat ;
			sidBuf[2] = (void *)&log->logInfo.normal.satInfo[satIdx].sid.num ;
			sidBuf[3] = (void *)&log->logInfo.normal.satInfo[satIdx].sid.reserve ;
			for (int i = 0 ; i < LOG_SID_NUM ; ++i)
			{
				fread(sidBuf[i], 1, sizeof(unsigned char), fp) ;
				CalLogCheckSum((unsigned char *)sidBuf[i], sizeof(unsigned char), checkSum) ;
			} // for

			log->size += mFormatSize[REG_SID_OFFSET] ;
        } // if

		satBuf[0] = (void *)&log->logInfo.normal.satInfo[satIdx].ele ;
		satBuf[1] = (void *)&log->logInfo.normal.satInfo[satIdx].azi ;
		satBuf[2] = (void *)&log->logInfo.normal.satInfo[satIdx].snr ;
		for (int i = REG_ELE_OFFSET ; i <= REG_SNR_OFFSET ; ++i) // REG_ELE, REG_AZI, REG_SNR
		{
			if (mFormatReg[i] & log->format)
			{
				fread(satBuf[i - REG_ELE_OFFSET], 1, mFormatSize[i], fp) ;
				log->size += mFormatSize[i] ;
				CalLogCheckSum((unsigned char *)satBuf[i - REG_ELE_OFFSET], mFormatSize[i], checkSum) ;
			} // if
		} // for

		++satIdx ;
	}	while (satIdx < log->logInfo.normal.satInfo[0].sid.num && satIdx < MAX_SAT_NUM) ;
} // ReadSatInfo

/**
*/
bool inline CalLogCheckSum(unsigned char *buf, unsigned int bufSize, unsigned int *checkSum)
{
	for (int i = 0 ; i < bufSize ; ++i)
		*checkSum ^= buf[i]	;

	return true ;
} // CalLogCheckSum

/**
*/
bool inline CheckSectorPattern(unsigned char *dataBuf)
{
	int endStr = 0, checkSum = 0, calCheckSum = 0 ;
	unsigned char chkFmt = 0, chkMode = 0, chkSec = 0, chkDis = 0, chkSpd = 0 ;

	memcpy(&endStr, dataBuf + 508, sizeof(endStr)) ;
	if ('*' != dataBuf[506] || 0xBBBBBBBB != endStr)
		return false ;

	checkSum = dataBuf[507] ;
	for (int i = 2 ; i <= 5 ; ++i)
		chkFmt ^= dataBuf[i] ;

	for (int i = 6 ; i <= 7 ; ++i)
		chkMode ^= dataBuf[i] ;

	for (int i = 8 ; i <= 11 ; ++i)
		chkSec ^= dataBuf[i] ;

	for (int i = 12 ; i <= 15 ; ++i)
		chkDis ^= dataBuf[i] ;

   	for (int i = 16 ; i <= 19 ; ++i)
		chkSpd ^= dataBuf[i] ;

	calCheckSum = chkFmt | chkMode | chkSec | chkDis | chkSpd ;
	return (calCheckSum == checkSum ? true : false) ;
} // CheckSectorPattern

/**
* @brief Check input string's checksum part and calcuate its checksum confirm or not.
* @param[in] *src input string.
* @return if checksum confirm return true, or return false.
*/
bool CheckSumMTK(unsigned char *src, const int size)
{
	char *checkSumStr = strchr_bysize(src, '*', size) ;
	int calSum = CalCheckSumMTK(src, size), strSum = 0 ;

	if (NULL == checkSumStr)
		return false ;

	sscanf(++checkSumStr, "%2x", &strSum) ;
	return (calSum == strSum) ? true : false ;
} // CheckSumMTK

/**
* @brief Calcuate input string's checksum.
* @param[in] *src input string.
* @return input string's checksum.
*/
int CalCheckSumMTK(unsigned char *src, const int size)
{
	int sum = 0, endIndex = IndexOfChar(src, '*', size) ;

	if ('$' != *(src++))
		return -1 ;

	if (-1 == endIndex)
		return -1 ;

	for	(int i = 1 ; i < endIndex ; ++i)
		sum ^= *(src++) ;

	return sum ;
} // CalCheckSumMTK

/**
*/
int CreateBootRomPkt(unsigned char *buf, unsigned char *data, unsigned char cmd, int handleAddr, int handelLen)
{    /*
	Packet format :

	[Command: 1 byte][Address: 4 bytes][Length of payload: 4 byte （最多128）][Payload 如果Length=0，那麼這個field就不存在]
	Note： 裡面沒有checksum，所有的field裡面都是LSB先送

	WIRTE command = 0x000000A1
	所以這個送出DLDA.bin所送出的packets大約長成這個樣子

	A1 00 00 00 20 00 80 00 00 [128 bytes payload]
	A1 00 10 00 20 00 80 00 00 [128 bytes payload]
	A0 00 20 00 20 00 80 00 00 [128 bytes payload]         */

	int bufSize = 0 ;

	bufSize += safe_memcpy(buf + bufSize, &cmd, sizeof(cmd)) ;
	bufSize += safe_memcpy(buf + bufSize, &handleAddr, sizeof(handleAddr)) ;
	bufSize += safe_memcpy(buf + bufSize, &handelLen, sizeof(handelLen)) ;
	switch (cmd)
	{
		case BOOT_ROM_WRITE_CMD :
			bufSize += safe_memcpy(buf + bufSize, data, handelLen) ;
			break ;
		case BOOT_ROM_READ_CMD :
			break ;
	} // switch

	return bufSize ;
} // CreateBootRomPkt

/**
*/
int SendBootRomPkt(HANDLE comPort, FILE *rwFp, unsigned char cmd, int startAddr, int dataSize, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	unsigned int writeAddr = startAddr, writeLen = 0, status = API_SUCCESS ;
	unsigned char data[BOOT_ROM_PACKAGE_SIZE * 2] = {"\0"}/*, apTitle[6] = "TSI_AP"*/ ;
	MTKAckStatus mtkAckInfo ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT * 2 ;
	info.pktType = NORMAL_PACKAGE ;

	for (int sendDataSize = 0, remindDataSize = dataSize ; remindDataSize > 0 ; writeAddr += writeLen, remindDataSize -= writeLen)
	{
		sendDataSize += (writeLen = (remindDataSize >= BOOT_ROM_PACKAGE_SIZE ? BOOT_ROM_PACKAGE_SIZE : remindDataSize)) ;
		if (BOOT_ROM_WRITE_CMD == cmd)
			fread(data, 1, writeLen, rwFp) ;

		info.writeBufSize = CreateBootRomPkt(info.writeBuf, data, cmd, writeAddr, writeLen) ;
		if (API_SUCCESS != (status = WriteToCom(&info, &mtkAckInfo, (BOOT_ROM_WRITE_CMD == cmd ? NULL : rwFp), JudgeBootRomPkt)))
			return status ;

		if (NULL != callBackUpdateProcessFun)
			callBackUpdateProcessFun(processBar, sendDataSize, dataSize) ;
	} // while

	return API_SUCCESS ;
} // SendBootRomPkt

/**
* @brief uplode DlDa.bin file to devie.
* @param[in] comPort comport which connect to device.
* @param[in] devType option of different device.
* @param[in] *fileName file name of DlDa.bin.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int SetDlda(HANDLE comPort, unsigned char *fileName, int writeAddr, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	FILE *dldaBin = fopen(fileName, "rb+") ;
	unsigned int fileSize = 0, status = API_SUCCESS ;

	if (NULL == dldaBin)
		return FILE_NOT_EXIST ;

	fileSize = FileSize(dldaBin) ;
	fseek(dldaBin, DLDA_OFFSET, SEEK_SET) ;        // 5 把DLDA.bin的檔案，去掉最前面（1024 + 16）bytes的header
												   // 後面的內容利用WRITE packet寫到從0x20000000的地方。
	if (API_SUCCESS != (status = SendBootRomPkt(comPort, dldaBin, BOOT_ROM_WRITE_CMD, writeAddr, fileSize, processBar, callBackUpdateProcessFun)))
		status = status ;

	fclose(dldaBin) ;
	return status ;
} // SetDlda

/**
*@brief call back function
*/
int JudgeBootRomPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	const unsigned char startCmd[BOOT_ROM_START_CMD_NUM] = {BOOT_ROM_START_CMD1, BOOT_ROM_START_CMD2, BOOT_ROM_START_CMD3, BOOT_ROM_START_CMD4} ;
	const unsigned char startCmdAck[BOOT_ROM_START_CMD_NUM] = {BOOT_ROM_START_CMD1_ACK, BOOT_ROM_START_CMD2_ACK, BOOT_ROM_START_CMD3_ACK, BOOT_ROM_START_CMD4_ACK} ;
	int status = API_SUCCESS, readDataSize = 0 ;

	if (comInfo->resBufSize < comInfo->writeBufSize)
		return API_FAIL ;  // 還未收集到足夠package, bufSize 不歸零

	switch (comInfo->writeBuf[0])
	{
		case BOOT_ROM_START_CMD1 :
		case BOOT_ROM_START_CMD2 :
		case BOOT_ROM_START_CMD3 :
		case BOOT_ROM_START_CMD4 :
			for (int i = 0 ; i < BOOT_ROM_START_CMD_NUM ; ++i)
			{                                           // 如果 ack 不正確或是 ack size 不正確則跳出
				if (startCmd[i] == comInfo->writeBuf[0] && (startCmdAck[i] != comInfo->resBuf[0] || 1 != comInfo->resBufSize))
				{
                    status = API_FAIL ;
					goto END_HANDLE ;
                } // if
            } // for
            break ;
		case BOOT_ROM_READ_CMD :    // 跳過 cmd, addr
			memcpy(&readDataSize, comInfo->resBuf + sizeof(char) + sizeof(int), sizeof(readDataSize)) ;
			if (comInfo->resBufSize < (comInfo->writeBufSize + readDataSize))
				return API_FAIL ;   // data 部分 還未收集到, bufSize 不歸零
			break ;
		default :                   // 其餘命令 寫入的 package & 收到的 package 一樣
			if (0 != strncmp(comInfo->resBuf, comInfo->writeBuf, comInfo->writeBufSize))
			{
				status = API_FAIL ;
				goto END_HANDLE ;
            } // if
			break ;
	} // switch

	status = AnalysisBootRomData(comInfo->resBuf, comInfo->resBufSize, (MTKAckStatus *)ackInfo, dwFp, callBackUpdateProcessFun) ;

END_HANDLE :
	return status ;
} // JudgeBootRomPkt

/**
*/
int AnalysisBootRomData(unsigned char *buf, int bufSize, MTKAckStatus *ackInfo, FILE *dwFp, UpdateProcessFun callBackUpdateProcessFun)
{
    unsigned char cmd = buf[0] ;
	int status = API_SUCCESS ;

	switch (cmd)
	{
		case BOOT_ROM_READ_CMD :
			status = AnalysisBootRomReadCmd(buf + sizeof(cmd), bufSize - sizeof(cmd), ackInfo, dwFp, callBackUpdateProcessFun) ;
			break ;
    } // switch

    return status ;
} // AnalysisBootRomData

/**
*/
int inline AnalysisBootRomReadCmd(unsigned char *buf, int bufSize, MTKAckStatus *ackInfo, FILE *dwFp, UpdateProcessFun callBackUpdateProcessFun)
{
	int addr = 0, length = 0, offset = 0 ;

    offset += safe_memcpy(&addr, buf + offset, sizeof(addr)) ;
    offset += safe_memcpy(&length, buf + offset, sizeof(length)) ;
	offset += safe_memcpy(ackInfo->readData ,buf + offset, length) ;

	if (offset != bufSize)
		return API_FAIL ;

	if (NULL != dwFp)
    	fwrite(ackInfo->readData, length, 1, dwFp) ;

	return API_SUCCESS ;
} // AnalysisBootRomReadCmd
