//---------------------------------------------------------------------------

#pragma hdrstop

#include <stdio.h>
#include <string.h>

#include "TSIPackage.h"
#include "Message.h"
#include "GetToken.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

unsigned char *mCompareList = ",*" ;

/**
*/
int CreateTSIPkt(unsigned char *writeData, const char *format, ...)
{
	int offset = 0 ;
	va_list args ;

	va_start(args, format) ;
	offset = vsprintf(writeData + offset, format, args) ;
	va_end(args) ;

	offset += sprintf(writeData + offset, "%02X\r\n", CalCheckSumMTK(writeData, offset)) ;

	return offset ;
} // CreateTSIPkt

/**
*@brief call back function
*/
int JudgeTSIPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char *pktStartPtr = NULL, *pktEndPtr = NULL ;
	int status = API_FAIL, offset = 0, pktSize = 0 ;

	while (API_SUCCESS != status &&             // 可能收到包含其他類別的 package
		  (NULL != (pktStartPtr = strchr_bysize((char *)comInfo->resBuf + offset, '$', comInfo->resBufSize - offset))) &&         // 先確定有開頭字元
		  (NULL != (pktEndPtr = strchr_bysize(pktStartPtr, '\n', comInfo->resBufSize - (offset += pktStartPtr - &comInfo->resBuf[offset])))))   // 從開頭字元的位置往後找結尾字元
	{
		pktSize = pktEndPtr - &comInfo->resBuf[offset] + 1 ;  // 確定是 $PTSI 開頭的才分析
		if (0 == strncmp(comInfo->resBuf + offset, "$PTSI", strlen("$PTSI") - 1) && CheckSumMTK(comInfo->resBuf + offset, pktSize))
		{
			status = AnalysisTSIData(comInfo->devType, comInfo->resBuf + offset, pktSize, ackInfo, dwFp, processBar, callBackUpdateProcessFun) ;
			if (G06MA_ERASE_4K & comInfo->pktType)
			{
				if (!((API_SUCCESS == status) &&
					  (TSI_ERASE_FLASH == ((TSIAckStatus *)ackInfo)->cmdAck.cmd) &&
					  (64 == ++comInfo->g06maErase4KackCount)))
				{   // G06MA erase 的指令回ack時 一次回 erase 1k 成功, 共64次
					status = API_FAIL ;
				} // if
			} // if

			if (MTK_NEED_ACK_PACKAGE & comInfo->pktType)
				status = (ACK_SUCCEEDED == status ? API_SUCCESS : API_FAIL) ;
		} // if

		offset += pktSize ;                     // 不管是否 TSI 的 package 都要跳過
	} // while

	if (0 < (comInfo->resBufSize -= offset))      // 剩餘不足一個pkt的資料搬到buffer最前端
		memcpy(comInfo->resBuf, comInfo->resBuf + offset, comInfo->resBufSize) ;

	return status ;
} // JudgeTSIPkt

/**
@param ackInfo : 因為TSI182分析路徑走MTK182路徑, 所以型別使用 void* 再依情況轉 (TSIAckStatus *) 或 (MTKAckStatus *)
*/
int AnalysisTSIData(int devType, unsigned char *buf, int bufSize, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS, offSet = sizeof("$PTSI") - 1, tokenSize = 0, optionId = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	buf += offSet ;
	bufSize -= offSet ;
	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	optionId = safe_atoi((char *)token) ;
	switch (optionId)
	{
		case TSI_OPTION_ID_510 :
			((TSIAckStatus *)ackInfo)->optionId = optionId ;
			status = AnalysisTSI510Ack(devType, buf, bufSize, &offSet, (TSIAckStatus *)ackInfo) ;
			break ;
		case TSI_OPTION_ID_770 :
			((TSIAckStatus *)ackInfo)->optionId = optionId ;
			status = AnalysisTSI770Ack(devType, buf, bufSize, &offSet, (TSIAckStatus *)ackInfo) ;
			break ;
		case TSI_OPTION_ID_997 :
			((TSIAckStatus *)ackInfo)->optionId = optionId ;
			status = AnalysisTSI997Ack(devType, buf, bufSize, &offSet, (TSIAckStatus *)ackInfo) ;
			break ;
		case TSI_OPTION_ID_999 :
			((TSIAckStatus *)ackInfo)->optionId = optionId ;
			status = AnalysisTSI999Ack(devType, buf, bufSize, &offSet, (TSIAckStatus *)ackInfo) ;
			break ;
		case TSI_OPTION_ID_1600 :
			((TSIAckStatus *)ackInfo)->optionId = optionId ;
			status = AnalysisTSI1600Ack(devType, buf, bufSize, &offSet, (TSIAckStatus *)ackInfo) ;
			break ;
		case PMTK_LOG :  // 182
			status = AnalysisPMTKLog(buf, bufSize, &offSet, (MTKAckStatus *)ackInfo, dwFp, processBar, callBackUpdateProcessFun) ;
			break ;
	} // switch

	return status ;
} // AnalysisTSIData

/**
*/
int inline AnalysisTSI510Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int cmdOption = 0, status = API_FAIL, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	cmdOption = safe_atoi((char *)token) ;
	ackInfo->cmdAck.cmd = cmdOption ;
	switch (cmdOption)
	{
		case TSI_CMD_READ :
			status = Analysis510ReadData(devType, buf, bufSize, offSet, ackInfo) ;
			break ;
    } // switch

	ackInfo->cmdAck.status = status ;
	return status ;
} // AnalysisTSI510Ack

/**
*/
int inline AnalysisTSI770Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int cmdOption = 0, status = API_FAIL, tokenSize = 0 ;
    unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	cmdOption = safe_atoi((char *)token) ;
	ackInfo->cmdAck.cmd = cmdOption ;
	switch (cmdOption)
	{
		case TSI_ACK_SUCCESS :
			status = AnalysisTSI770Ack(devType, buf, bufSize, offSet, ackInfo) ;
			break ;
		case TSI_CMD_READ :
			status = AnalysisTSIReadData(devType, buf, bufSize, offSet, ackInfo) ;
			break ;
		case TSI_CMD_WRITE_770 :
			status = AnalysisTSI770Write(devType, buf, bufSize, offSet, ackInfo) ;
			break ;
		case TSI_SHUTDOWN :
		case TSI_MSG_RATE :
		case TSI_POWERSAVING :
		case TSI_ABOUT_CONNECT :
		case TSI_ERASE_LOG_STATUS_770 :
			status = AnalysisTSIStatus(devType, buf, bufSize, offSet, ackInfo) ;
			break ;
    } // switch

	ackInfo->cmdAck.status = status ;
	return status ;
} // AnalysisTSI770Ack

/**
*/
int inline AnalysisTSI997Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int cmdOption = 0, status = API_FAIL, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	cmdOption = safe_atoi((char *)token) ;
	ackInfo->cmdAck.cmd = cmdOption ;
	switch (cmdOption)
	{
		case TSI_QUERY_STATUS :
			status = AnalysisG06Status(devType, buf, bufSize, offSet, ackInfo) ;
			break ;
    } // switch

	ackInfo->cmdAck.status = status ;
	return status ;
} // AnalysisTSI997Ack

/**
*/
int inline AnalysisTSI999Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int cmdOption = 0, status = API_FAIL, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	cmdOption = safe_atoi((char *)token) ;
	ackInfo->cmdAck.cmd = cmdOption ;
	switch (cmdOption)
	{
		case TSI_RESET_NMEA_OUTPUT :
		case TSI_ERASE_LOG :
			status = safe_atoi((char *)token) ;
			ackInfo->cmdAck.status = status ;
			break ;
		case TSI_WRITE_SERIES_999 :
            GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
			strcpy(ackInfo->devInfo.base.devSeries, token) ;
			status = API_SUCCESS ;
			break ;
		case TSI_QUERY_RELEASE :
			status = AnalysisTSIRelease(devType, buf, bufSize, offSet, ackInfo) ;
			break ;
	} // switch

	//ackInfo->ackStatus.extra.tsi.cmdAck.status = status ;
	return status ;
} // AnalysisTSI999Ack

/**
*/
int inline AnalysisTSI1600Ack(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int cmdOption = 0, status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	cmdOption = safe_atoi((char *)token) ;
	ackInfo->cmdAck.cmd = cmdOption ;
	switch (cmdOption)
	{
		case TSI_ERASE_FLASH :
			status = Analysis1600EraseFlash(devType, buf, bufSize, offSet, ackInfo) ;
			break ;
		case TSI_CMD_READ :
			status = AnalysisTSIReadData(devType, buf, bufSize, offSet, ackInfo) ;
			break ;
	} // switch

	ackInfo->cmdAck.status = status ;
	return status ;
} // AnalysisTSI1600Ack

/**
*/
int inline Analysis1600EraseFlash(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int address = 0, status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf((char *)token, "%x", &address) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;

    return (status = safe_atoi((char *)token)) ;
} // Analysis1600EraseFlash

/**
*/
int inline AnalysisG06Status(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{   // G06 :   $PTSI997,0,BOOT_VER,AP_VER,COURSE_VER,CNAME_VER,WAV_VER
	// G06MA : $PTSI997,0,BOOT_VER,AP_VER,WAV_VER
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	if (DEVICE_TYPE_G06MA != devType && DEVICE_TYPE_G06 != devType)
		return API_FAIL ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->devInfo.base.btVer, token, tokenSize) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->devInfo.base.fwVer, token, tokenSize) ;
	switch (devType)
	{
		case DEVICE_TYPE_G06 :
			GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
			memcpy(ackInfo->devInfo.extra.g06Series.cVer, token, tokenSize) ;

			GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
			memcpy(ackInfo->devInfo.extra.g06Series.cNameVer, token, tokenSize) ;

			GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
			memcpy(ackInfo->devInfo.extra.g06Series.waveVer, token, tokenSize) ;
			break ;
		case DEVICE_TYPE_G06MA :
			GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
			memcpy(ackInfo->devInfo.extra.g06Series.waveVer, token, tokenSize) ;
			break ;
	} // switch

	return status ;
} // AnalysisG06Status

/**
*/
int inline Analysis510ReadData(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{   //$PTSI510,2,-12,0.00.52,00.00.03,000000
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf((char *)token, "%x", &ackInfo->devInfo.extra.g06Series.uctZone) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->devInfo.base.fwVer, token, tokenSize) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->devInfo.base.btVer, token, tokenSize) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;  // reserve

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->devInfo.base.devSeries, token, tokenSize) ;

	return status ;
} // Analysis510ReadData

/**
*/
int inline AnalysisTSIReadData(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf((char *)token, "%x", &ackInfo->curReadAddr) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->readData, token, tokenSize) ;
	ackInfo->readData[tokenSize] = '\0' ;

	return status ;
} // Analysis770ReadData

/**
*/
int inline AnalysisTSI770Write(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0, writeAddr = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

    GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	safe_sscanf((char *)token, "%x", &writeAddr) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;

	return (status = safe_atoi((char *)token)) ;
} // AnalysisTSI770Write

/**
*/
int inline AnalysisTSIStatus(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;

	return (status = safe_atoi((char *)token)) ;
} // Analysis770Connect

/**
*/
int inline AnalysisTSIRelease(int devType, unsigned char *buf, int bufSize, int *offSet, TSIAckStatus *ackInfo)
{
	int status = API_SUCCESS, tokenSize = 0 ;
	unsigned char token[TOKEN_SIZE] = {"\0"} ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->devInfo.base.btVer, token, tokenSize) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	memcpy(ackInfo->devInfo.base.fwVer, token, tokenSize) ;

	GetToken(buf, bufSize, token, &tokenSize, mCompareList) ; buf += (tokenSize + 1) ; bufSize -= (tokenSize + 1) ;
	switch (devType)
	{
		case DEVICE_TYPE_770 :
			memcpy(ackInfo->devInfo.extra.d770.bleVer, token, tokenSize) ;
			break ;
		case DEVICE_TYPE_870 :
			memcpy(ackInfo->devInfo.extra.d870.bleVer, token, tokenSize) ;
			break ;
		case DEVICE_TYPE_BT1000 :
            memcpy(ackInfo->devInfo.extra.bt1000.bleVer, token, tokenSize) ;
			break ;
    } // switch

	return status ;
} // AnalysisTSIRelease

/**
*/
bool AskLogFullHandle(HANDLE comPort, int logStartAddr, MTKAckStatus *ackInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	unsigned char compareStr[16] = {"FFFFFFFFFFFFFFFF"} ;
	int startAddr = 0, writeBufSize = 0, status = API_SUCCESS ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT ;

	memset(compareStr, 0xFF, sizeof(compareStr)) ;

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%08X,%08X*", PMTK_LOG, LOG_READ, logStartAddr, MTK_READ_DATA_SIZE) ;
	info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, ackInfo, NULL, JudgeMTKPkt)))
		return false ;

    ackInfo->recInfo.overWrite = (0 != strncmp(ackInfo->readData, compareStr, sizeof(compareStr))) ;
	return true ;
} // AskLogFullHandle
