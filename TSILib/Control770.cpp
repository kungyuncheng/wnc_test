//---------------------------------------------------------------------------

#pragma hdrstop

#include "Control770.h"
#include "GetToken.h"
#include "DFUPackage.h"
#include "Message.h"
#include "ConnectComport.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int AskDevInfo770(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS, retryCount = 0 ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.timeOut = TIME_OUT * 1.5 ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d,%d*", TSI_OPTION_ID_770, TSI_QUERY_STATUS, TSI_ABOUT_CONNECT) ;
	info.devType = DEVICE_TYPE_770 ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	do
	{
		status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt) ;
		if (TSI_ERASE_LOG_STATUS_770 == tsiAckInfo->cmdAck.cmd && CMD_OFF == tsiAckInfo->cmdAck.status)
			return API_SUCCESS ;  // 收到此回復表示機器需要 erase log 直接跳走 

	}   while (++retryCount < 2 && !(TSI_ABOUT_CONNECT == tsiAckInfo->cmdAck.cmd && CMD_ON == tsiAckInfo->cmdAck.status)) ;

	if (API_SUCCESS != status)
		return status ;

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_SPI_STATUS) ;
	info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;         // ask spi status

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_REC_METHOD) ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask record method

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_FLASH_ID) ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask flash id

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d*", PMTK_Q_RELEASE) ;
    info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask device info

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d*", TSI_OPTION_ID_999, TSI_QUERY_RELEASE) ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;        // ask sw version

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_REC_ADDR) ;
	info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask current addr

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_REC_COUNT) ;
	info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask record count

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d,%08x*", TSI_OPTION_ID_770, TSI_CMD_READ, D770_SERIES_NUM_ADDR) ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;        // ask device series

	strcpy(tsiAckInfo->devInfo.base.devSeries, tsiAckInfo->readData) ;

							// 檢查最後一個section 是否有資料
	if (!AskLogFullHandle(comPort, (MAX_SECTION_770 - 1) * AP_BLOCK_SIZE, mtkAckInfo))
		return API_FAIL ;        // ask overwrite or not

	if (mtkAckInfo->recInfo.overWrite)
		mtkAckInfo->recInfo.curSection = D770_LOG_LEN / AP_BLOCK_SIZE ;

	strcpy(tsiAckInfo->devInfo.base.fwVer, mtkAckInfo->internal2) ;
	return status ;
} // AskDevInfo770

/**
*/
int ReadData770(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS, timeout = (handelLen / AP_BLOCK_SIZE) * READ_SECTION_DATA_TIME_OUT, baudRate = 0 ;
	ConnectComport *connectCom = new ConnectComport() ;
	WriteComInfo info ;
	MTKAckStatus mtkAckInfo ;
	FILE *dwFp = fopen(outputFileName, "wb+") ;

	if (NULL == dwFp)
	{
		status = FILE_NOT_EXIST ;
		goto END_HANDLE ;
	} // if

	if (API_SUCCESS != (status = connectCom->GetBaudRate(*comPort, &baudRate)))
		goto END_HANDLE ;

	info.comPort = *comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT ;

	mtkAckInfo.recInfo.curSection = handelLen / AP_BLOCK_SIZE ;       // callBackUpdateProcessFun 讀資料的最大值

	if (DEVICE_BAUDRATE_770_DOWNLOAD != baudRate)
	{
		info.timeOut = TIME_OUT ;
		info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_BAUDRATE, DEVICE_BAUDRATE_770_DOWNLOAD) ;
        info.pktType = NO_ACK ;
		if (API_SUCCESS != (status = WriteToCom(&info, &mtkAckInfo, NULL, JudgeMTKPkt)))  //  MTK_CHANGE_BAUDRATE 一定回傳API_SUCCESS
        	goto END_HANDLE ;

		if (API_SUCCESS != (status = connectCom->SetBaudRate(comPort, DEVICE_BAUDRATE_770_DOWNLOAD, true)))
			goto END_HANDLE ;
	} // if

	if (API_SUCCESS != (status = connectCom->GetBaudRate(*comPort, &baudRate)))
		goto END_HANDLE ;

	info.timeOut = timeout ;
	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%08X,%08X*", PMTK_LOG, LOG_READ, handleAddr, handelLen) ;
    info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, &mtkAckInfo, dwFp, JudgeMTKPkt, processBar, callBackFun)))
		goto END_HANDLE ;

	RefreshMTKDownloadBin(dwFp, outputFileName) ;
END_HANDLE :
	if (NULL != dwFp)
		fclose(dwFp) ;

	info.timeOut = TIME_OUT ;
	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_BAUDRATE, DEVICE_BAUDRATE_770) ;
	info.pktType = NO_ACK ;
	if (API_SUCCESS != (status = WriteToCom(&info, &mtkAckInfo, dwFp, JudgeMTKPkt)))
		return status ;

	connectCom->SetBaudRate(comPort, DEVICE_BAUDRATE_770, true) ;
	delete connectCom ;
	return status ;
} // ReadData770

/**
*/
int ChangeNMEAFre770(HANDLE comPort, bool on5HZ)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo info ;
	TSIAckStatus tsiAckInfo ;

	info.comPort = comPort ;
	info.timeOut = MTK_CHANGE_BAUDRATE_TIME ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d,%d*", TSI_OPTION_ID_770, TSI_MSG_RATE, (on5HZ ? CMD_ON : CMD_OFF)) ;
	info.devType = DEVICE_TYPE_770 ;
	info.pktType = NO_ACK ;

	return (status = WriteToCom(&info, &tsiAckInfo, NULL, JudgeTSIPkt)) ;
} // ChangeNMEAFre

/**
*/
int DeleteLog770(HANDLE comPort, bool waitAck)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	int timeout = (waitAck ? MAX_SECTION_770 * ERASE_SECTION_TIME_OUT : TIME_OUT) ;
	WriteComInfo info ;
	MTKAckStatus mtkAckInfo ;

	info.comPort = comPort ;
	info.timeOut = timeout ;
    info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_FORMAT, FORMAT_ALL) ;
    info.pktType = (waitAck ? MTK_NEED_ACK_PACKAGE : NO_ACK) ;
	return (status = WriteToCom(&info, &mtkAckInfo, NULL, JudgeMTKPkt)) ;
} // DeleteLog770

/**
*/
int ResetDevSetting770(HANDLE comPort)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo info ;
	TSIAckStatus tsiAckInfo ;
	MTKAckStatus mtkAckInfo ;

	info.comPort = comPort ;
	info.timeOut = D770_RESET_TIMEOUT ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d*", TSI_OPTION_ID_999) ;
	info.devType = DEVICE_TYPE_770 ;
	info.pktType = MTK_NORMAL_PACKAGE ;

	return (status = WriteToCom(&info, &tsiAckInfo, NULL, JudgeTSIPkt)) ;
} // ResetDevSetting770

/**
*/
int Update770(int updateIdx, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS, baudRate = 0 ;
	WriteComInfo info ;
	TSIAckStatus tsiAckInfo ;
	ConnectComport *connect = new ConnectComport() ;

	info.comPort = comPort ;
	info.timeOut = MTK_CHANGE_BAUDRATE_TIME ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.devType = DEVICE_TYPE_770 ;

	connect->GetBaudRate(comPort, &baudRate) ;

	if (DEVICE_BAUDRATE_DFU_BOOT != baudRate)
	{
		info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d*", TSI_OPTION_ID_770, TSI_BOOTLOADER) ;
		info.pktType = NO_ACK ;
		if (API_SUCCESS != (status = WriteToCom(&info, &tsiAckInfo, NULL, JudgeTSIPkt)))
			return status ;
    } // if

	connect->SetBaudRate(&info.comPort, DEVICE_BAUDRATE_DFU_BOOT, true) ; // 開 flow control
	status = WriteFileDFU(comPort, DFU_FIRMWARE_ID_V710, DFU_UPDATE_FW, fp, FileRemainSize(fp), processBar, callBackFun) ;
	connect->SetBaudRate(&info.comPort, DEVICE_BAUDRATE_770, false) ;

	return status ;
} // Update770

/**
*/
int WriteSn770(HANDLE comPort, WriteSnSettingInfo snInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	unsigned char sn[BASE_DATA_SIZE] = {"\0"} ;
	int status = API_SUCCESS, dataSize = 0 ;
	TSIAckStatus tsiAckInfo ;
	WriteComInfo comInfo ;

	comInfo.comPort = comPort ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;

	comInfo.writeBufSize = CreateTSIPkt(comInfo.writeBuf, "$PTSI%03d,%d,%08X,%s%s%s%05d*",
														  TSI_OPTION_ID_770, TSI_CMD_WRITE_770, D770_SERIES_NUM_ADDR, snInfo.dev.d770.sn.model,
														  snInfo.dev.d770.sn.year, snInfo.dev.d770.sn.month, snInfo.writeSn) ;
	comInfo.devType = DEVICE_TYPE_770 ;
	comInfo.pktType = MTK_NORMAL_PACKAGE ;
    if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	if (snInfo.dev.d770.delLog && (API_SUCCESS != (status = DeleteLog770(comPort, false))))
		return status ;

	return status ;
} // ExtraTest770
