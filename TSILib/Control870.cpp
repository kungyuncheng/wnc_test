#pragma hdrstop

#include "Control870.h"
#include "MASAPackage.h"
#include "Message.h"
#include "GetToken.h"
#include "ConnectComport.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int AskDevInfo870(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.timeOut = TIME_OUT * 2 ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%08X,%08X*", PMTK_LOG, LOG_READ, D870_SERIES_NUM_ADDR, SERIES_SIZE_870) ;
    info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;

	strcpy(tsiAckInfo->devInfo.base.devSeries, mtkAckInfo->readData) ;

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_FLASH_ID) ;
    info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask flash id

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d*", TSI_OPTION_ID_999, TSI_QUERY_RELEASE) ;
	info.devType = DEVICE_TYPE_870 ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;        // ask sw version

	return status ;
} // AskDevInfo870

/**
*/
int JumpToBoot870(HANDLE *comPort)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	ConnectComport *connectCom = new ConnectComport() ;
	WriteComInfo info ;
	TSIAckStatus tsiAckInfo ;

	info.comPort = *comPort ;
	info.timeOut = TIME_OUT ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d*", TSI_OPTION_ID_998) ;
	info.devType = DEVICE_TYPE_870 ;
	info.pktType = NO_ACK ;
	if (API_SUCCESS != (status = WriteToCom(&info, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

    connectCom->SetBaudRate(comPort, DEVICE_BAUDRATE_870_BOOT, false) ;
	delete connectCom ;
	return status ;
} // JumpToBoot870

/**
*/
int Update870(int fileIdx, HANDLE *comPort, unsigned char *fileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun, UpdateGPSInfo updateGPSInfo)
{
	int status = API_FAIL ;

	switch (fileIdx)
	{
		case UPDATE_PATH_FW :
			status = Update870Normal(comPort, fileName, processBar, callBackUpdateProcessFun) ;
			break ;
		case UPDATE_PATH_GPS :
			status = Update870Gps(comPort, fileName, updateGPSInfo) ;
			break ;
	} // switch

	return status ;
} // Update870

/**
*/
int Update870Normal(HANDLE *comPort, unsigned char *fileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS ;
	FileInfo870 fileInfo ;
	FILE *fp = fopen(fileName, "rb") ;

	fread(&fileInfo, 1, sizeof(fileInfo), fp) ;
	for (int i = 0 ; i < UPDATE_MAX_CONTAIN_NUM_870 ; ++i)
	{
		if (fileInfo.fileSize[i] > 0)
		{
			if (fileInfo.writeAddr[i] >= D870_BLE_ADDR)  		    // update ble
			{

			} // if
			else if (fileInfo.writeAddr[i] >= D870_FW_ADDR)   // update ap or bt
			{
				if (API_SUCCESS != (status = UpdateFile870(*comPort, fp, fileInfo, i, processBar, callBackUpdateProcessFun)))
					goto END_HANDLE ;
			} // else
			else     	   											// update flash
			{

			} // else
        } // if
    } // for

END_HANDLE :
	fclose(fp) ;
	return status ;
} // Update870Normal

/**
*/
int Update870Gps(HANDLE *comPort, unsigned char *fileName, UpdateGPSInfo updateGPSInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_FAIL, oriBaudRate = 0 ;
	WriteComInfo comInfo ;
	TSIAckStatus tsiAckInfo ;
	ConnectComport *connect = new ConnectComport() ;

	connect->GetBaudRate(*comPort, &oriBaudRate) ;

	comInfo.comPort = *comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT * 2 ;
	comInfo.devType = DEVICE_TYPE_870 ;
	comInfo.pktType = NO_ACK ;
	comInfo.writeBufSize = CreateTSIPkt(comInfo.writeBuf, "$PTSI%03d*", TSI_OPTION_ID_990) ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeTSIPkt)))
		goto END_HANDLE ;

	CloseHandle(comInfo.comPort) ;   // MTK 提供的 download flash function 會自己開comport, 所以先關閉
	comInfo.comPort = INVALID_HANDLE_VALUE ;

	status = UpdateGPS(comPort, fileName, updateGPSInfo) ;
	connect->Connect(updateGPSInfo.comId, oriBaudRate, false, comPort) ;    // 重新開comport

END_HANDLE :

	delete connect ;
	return status ;
} // Update870Gps

/**
*/
int UpdateFile870(HANDLE comPort, FILE *fp, FileInfo870 fileInfo, int fIdx, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{   // 870 更新流程與 G05 系列相同
	int status = API_SUCCESS ;

	fseek(fp, fileInfo.fileOffset[fIdx], SEEK_SET) ;
	if (API_SUCCESS != (status = JumpToBoot870(&comPort)))
		return status ;

	if (API_SUCCESS != (status = StartCommucateMASA(comPort, DEVICE_TYPE_870)))
		return status ;

	if (API_SUCCESS != (status = ClearFlash870(comPort, fileInfo.writeAddr[fIdx], fileInfo.fileSize[fIdx])))
		return status ;

	return (status = SendMASAPkt(comPort, fp, DEVICE_TYPE_870, D870_PACKAGE_SIZE, CMD_DL_WRITE, fileInfo.writeAddr[fIdx], fileInfo.fileSize[fIdx], processBar, callBackUpdateProcessFun)) ;
} // UpdateFile870

/**
*/
int ClearFlash870(HANDLE comPort, int handleAddr, int handelLen)
{
	unsigned char writeBuf[MASA_BUFFER_MAX_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int blockSize = 0, fullToBlockSize = 0, status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	blockSize = (handleAddr >= D870_FW_ADDR ? MCU_BLOCK_SIZE : AP_BLOCK_SIZE) ;
	fullToBlockSize = handelLen + (0 == handelLen % blockSize ? 0 : (blockSize - handelLen % blockSize)) ;

	pktInfo.cmd.name = (handleAddr >= D870_FW_ADDR ? CMD_DL_ERASE_MCU : CMD_DL_ERASE) ;
	pktInfo.cmd.item.erase.numOfBlock = (handleAddr >= D870_FW_ADDR ? D870_NUM_OF_MCU_BLOCK : D870_NUM_OF_FLASH_BLOCK) ;
    pktInfo.cmd.item.erase.fwStartAddress = (handleAddr >= D870_FW_ADDR ? D870_FW_ADDR : 0) ;
	pktInfo.cmd.item.erase.blockSize = blockSize ;

	pktInfo.handleAddress = handleAddr ;
	pktInfo.handleLength = fullToBlockSize ;

	comInfo.timeOut = (handelLen > 1024 * 1024 * 10 ? ERASE_FLASH_TIME_OUT * 10 : ERASE_FLASH_TIME_OUT * 5) ;
	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.devType = DEVICE_TYPE_870 ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_870, pktInfo) ;
	return (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // ClearFlash870

/**
*/
int DeleteLog870(HANDLE comPort)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	MTKAckStatus mtkAckInfo ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.timeOut = TIME_OUT * 2 ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d*", TSI_OPTION_ID_999, TSI_ERASE_LOG) ;
	info.devType = DEVICE_TYPE_870 ;
	info.pktType = MTK_NORMAL_PACKAGE ;

	return (status = WriteToCom(&info, &mtkAckInfo, NULL, JudgeTSIPkt)) ;
} // DeleteLog870

/**
*/
int ReadData870(int tIdx, HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName)
{
	int status = API_SUCCESS ;

	return status ;
} // ReadData870

/**
*/
int WriteSn870(HANDLE comPort, WriteSnSettingInfo snInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	unsigned char sn[BASE_DATA_SIZE] = {"\0"} ;
	int status = API_SUCCESS, dataSize = 0 ;
	TSIAckStatus tsiAckInfo ;
	MTKAckStatus mtkAckInfo ;
	WriteComInfo comInfo ;

	comInfo.comPort = comPort ;
	comInfo.timeOut = TIME_OUT * 2 ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;

	comInfo.writeBufSize = CreateTSIPkt(comInfo.writeBuf, "$PTSI%03d,%d,%s%s%s%s%05d*",
														  TSI_OPTION_ID_999, TSI_WRITE_SERIES_999, snInfo.dev.d870.sn.model, snInfo.dev.d870.sn.mode2,
														  snInfo.dev.d870.sn.year, snInfo.dev.d870.sn.month, snInfo.writeSn) ;
	comInfo.devType = DEVICE_TYPE_870 ;
	comInfo.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	if (snInfo.dev.d870.delLog && (API_SUCCESS != (status = DeleteLog870(comPort))))
		return status ;

	return status ;
} // WriteSn870
