//---------------------------------------------------------------------------

#pragma hdrstop

#include "ControlBT1000.h"
#include "MASAPackage.h"
#include "ConnectComport.h"
#include "Message.h"
#include "GetToken.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int AskDevInfoBT1000(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.timeOut = TIME_OUT * 4 ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%08X,%08X*", PMTK_LOG, LOG_READ, BT1000_SERIES_NUM_ADDR, SERIES_SIZE_BT1000) ;
    info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;

	strcpy(tsiAckInfo->devInfo.base.devSeries, mtkAckInfo->readData) ;

	info.timeOut = TIME_OUT ;
	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_SPI_STATUS) ;
    info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask spi status

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_REC_METHOD) ;
    info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask record method

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_FLASH_ID) ;
    info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask flash id

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_REC_ADDR) ;
    info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask current addr

	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_REC_COUNT) ;
    info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;        // ask record count

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d*", TSI_OPTION_ID_999, TSI_QUERY_RELEASE) ;
	info.devType = DEVICE_TYPE_BT1000 ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;        // ask sw ver

							  // 檢查最後一個section 是否有資料
	if (!AskLogFullHandle(comPort, (MAX_SECTION_BT1000 - 1) * AP_BLOCK_SIZE, mtkAckInfo))
		return API_FAIL ;        // ask overwrite or not

	if (mtkAckInfo->recInfo.overWrite)
		mtkAckInfo->recInfo.curSection = BT1000_LOG_LEN / AP_BLOCK_SIZE ;

	return status ;
} // AskDevInfoBT1000

/**
*/
int JumpToBootBT1000(HANDLE *comPort)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	ConnectComport *connectCom = new ConnectComport() ;
	MTKAckStatus mtkAckInfo ;
	WriteComInfo info ;

	info.comPort = *comPort ;
	info.timeOut = TIME_OUT ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d*", TSI_OPTION_ID_998) ;
	info.devType = DEVICE_TYPE_BT1000 ;
	info.pktType = NO_ACK ;
	if (API_SUCCESS != (status = WriteToCom(&info, &mtkAckInfo, NULL, JudgeMTKPkt)))
		return status ;

	connectCom->SetBaudRate(comPort, DEVICE_BAUDRATE_BT1000_BOOT, false) ;
	delete connectCom ;
	return status ;
} // JumpToBootBT1000

/**
*/
int UpdateBT1000(int fileIdx, HANDLE *comPort, unsigned char *fileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun, UpdateGPSInfo updateGPSInfo)
{
	int status = API_FAIL, baudRate = 0 ;
	ConnectComport *connect = new ConnectComport() ;

	connect->GetBaudRate(*comPort, &baudRate) ;
	if (DEVICE_BAUDRATE_BT1000_BOOT != baudRate)
	{   // 是AP則先跳到BOOT
		if (API_SUCCESS != (status = JumpToBootBT1000(comPort)))
			return status ;
    } // if

	switch (fileIdx)
	{
		case UPDATE_PATH_FW :
			status = UpdateBT1000Normal(comPort, fileName, processBar, callBackUpdateProcessFun) ;
			break ;
		case UPDATE_PATH_GPS :
			status = UpdateBT1000Gps(comPort, fileName, updateGPSInfo) ;
			break ;
	} // switch

	delete connect ;
	return status ;
} // UpdateBT1000

/**
*/
int UpdateBT1000Normal(HANDLE *comPort, unsigned char *fileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS ;
	FileInfoBT1000 fileInfo ;
	FILE *fp = fopen(fileName, "rb") ;

    if (NULL == fp)
		return FILE_NOT_EXIST ;

	fread(&fileInfo, 1, sizeof(fileInfo), fp) ;
	for (int i = 0 ; i < UPDATE_MAX_CONTAIN_NUM_BT1000 ; ++i)
	{
		if (fileInfo.fileSize[i] > 0)
		{
			if (fileInfo.writeAddr[i] >= BT1000_BLE_ADDR)  		    // update ble
			{

			} // if
			else if (fileInfo.writeAddr[i] >= BT1000_FW_ADDR)   // update ap or bt
			{
				if (API_SUCCESS != (status = UpdateFileBT1000(*comPort, fp, fileInfo, i, processBar, callBackUpdateProcessFun)))
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
} // UpdateBT1000Normal

/**
*/
int UpdateBT1000Gps(HANDLE *comPort, unsigned char *fileName, UpdateGPSInfo updateGPSInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_FAIL, oriBaudRate = 0 ;
	WriteComInfo comInfo ;
	TSIAckStatus tsiAckInfo ;
	CreateMasaPktInfo pktInfo ;
	ConnectComport *connect = new ConnectComport() ;

	connect->GetBaudRate(*comPort, &oriBaudRate) ;

	pktInfo.cmd.name = CMD_DL_SW ;

	comInfo.comPort = *comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT * 2 ;
	comInfo.devType = DEVICE_TYPE_BT1000 ;
	comInfo.pktType = NO_ACK ; // 目前下此命令收到的ack資料有異狀  所以不用 NORMAL_PACKAGE, 改用 NO_ACK 等一段時間後跳過 ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_BT1000, pktInfo) ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
	    goto END_HANDLE ;

	CloseHandle(comInfo.comPort) ;   // MTK 提供的 download flash function 會自己開comport, 所以先關閉
	comInfo.comPort = INVALID_HANDLE_VALUE ;

    status = UpdateGPS(comPort, fileName, updateGPSInfo) ;
	connect->Connect(updateGPSInfo.comId, oriBaudRate, false, comPort) ;    // 重新開comport

END_HANDLE :

	delete connect ;
	return status ;
} // UpdateBT1000Gps

/**
*/
int UpdateFileBT1000(HANDLE comPort, FILE *fp, FileInfoBT1000 fileInfo, int fIdx, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{   // BT1000 更新流程與 G05 系列相同
	int status = API_SUCCESS ;

	fseek(fp, fileInfo.fileOffset[fIdx], SEEK_SET) ;
	if (API_SUCCESS != (status = StartCommucateMASA(comPort, DEVICE_TYPE_BT1000)))
		return status ;

	if (API_SUCCESS != (status = ClearFlashBT1000(comPort, fileInfo.writeAddr[fIdx], fileInfo.fileSize[fIdx])))
		return status ;

	return (status = SendMASAPkt(comPort, fp, DEVICE_TYPE_870, BT1000_PACKAGE_SIZE, CMD_DL_WRITE, fileInfo.writeAddr[fIdx], fileInfo.fileSize[fIdx], processBar, callBackUpdateProcessFun)) ;
} // UpdateFileBT1000

/**
*/
int ClearFlashBT1000(HANDLE comPort, int handleAddr, int handelLen)
{
	unsigned char writeBuf[MASA_BUFFER_MAX_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int blockSize = 0, fullToBlockSize = 0, status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	blockSize = (handleAddr >= BT1000_FW_ADDR ? MCU_BLOCK_SIZE : AP_BLOCK_SIZE) ;
	fullToBlockSize = handelLen + (0 == handelLen % blockSize ? 0 : (blockSize - handelLen % blockSize)) ;

	pktInfo.cmd.name = (handleAddr >= BT1000_FW_ADDR ? CMD_DL_ERASE_MCU : CMD_DL_ERASE) ;
	pktInfo.cmd.item.erase.numOfBlock = (handleAddr >= BT1000_FW_ADDR ? BT1000_NUM_OF_MCU_BLOCK : BT1000_NUM_OF_FLASH_BLOCK) ;
    pktInfo.cmd.item.erase.fwStartAddress = (handleAddr >= BT1000_FW_ADDR ? BT1000_FW_ADDR : 0) ;
	pktInfo.cmd.item.erase.blockSize = blockSize ;

	pktInfo.handleAddress = handleAddr ;
	pktInfo.handleLength = fullToBlockSize ;

	comInfo.timeOut = (handelLen > 1024 * 1024 * 10 ? ERASE_FLASH_TIME_OUT * 10 : ERASE_FLASH_TIME_OUT * 5) ;
	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.devType = DEVICE_TYPE_BT1000 ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_BT1000, pktInfo) ;
	return (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // ClearFlashBT1000

/**
*/
int DeleteLogBT1000(HANDLE comPort)
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
	info.devType = DEVICE_TYPE_BT1000 ;
	info.pktType = MTK_NORMAL_PACKAGE ;

	return (status = WriteToCom(&info, &mtkAckInfo, NULL, JudgeTSIPkt)) ;
} // DeleteLogBT1000

/**
*/
int ReadDataBT1000(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS, baudRate = 0, timeout = (handelLen / AP_BLOCK_SIZE) * READ_SECTION_DATA_TIME_OUT ; //   READ_SECTION_DATA_TIME_OUT  為 baud rate 921600 時的速度
	WriteComInfo info ;
	MTKAckStatus mtkAckInfo ;
	FILE *dwFp = fopen(outputFileName, "wb+") ;
	ConnectComport *connect = new ConnectComport() ;

	if (NULL == dwFp)
		return FILE_NOT_EXIST ;

	info.comPort = *comPort ;
	info.timeOut = TIME_OUT ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;

	mtkAckInfo.recInfo.curSection = handelLen / AP_BLOCK_SIZE ;     // callBackUpdateProcessFun 讀資料的最大值

	connect->GetBaudRate(*comPort, &baudRate) ;
	timeout *= (BAUD_RATE_921600 / baudRate) ;          // 因應不同baudrate修改timeout時間

	info.timeOut = timeout ;
	info.writeBufSize = CreateMTKPkt(info.writeBuf, "$PMTK%03d,%d,%08X,%08X*", PMTK_LOG, LOG_READ, handleAddr, handelLen) ;
	info.devType = DEVICE_TYPE_BT1000 ;
	info.pktType = MTK_NEED_ACK_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, &mtkAckInfo, dwFp, JudgeMTKPkt, processBar, callBackUpdateProcessFun)))
		return status ;

	RefreshMTKDownloadBin(dwFp, outputFileName) ;
	delete connect ;
	return status ;
} // ReadData870

/**
*/
int WriteSnBT1000(HANDLE comPort, WriteSnSettingInfo snInfo)
{
	return API_SUCCESS ;
} // WriteSnBT1000

