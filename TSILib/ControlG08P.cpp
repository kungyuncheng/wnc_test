//---------------------------------------------------------------------------

#pragma hdrstop

#include "ControlG08P.h"
#include "MASAPackage.h"
#include "Message.h"
#include "GetToken.h"
#include "ConnectComport.h"
#include "DFUPackage.h"
#include "ControlG05.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int ResetDeviceSettingG08P(HANDLE comPort)
{
	return ClearFlash4KG08P(comPort, G08P_SETTING_ADDR, 0x1000) ;
} // ResetDeviceSettingG08P

/**
*/
int AskDevInfoG08P(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	pktInfo.cmd.name = CMD_DL_DEV_INFO ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.devType = DEVICE_TYPE_G08P ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08P, pktInfo) ;
	return (status = WriteToCom(&comInfo, tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // AskDevInfoG08P

/**
*/
int UpdateG08P(int updateOption, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int fileOffSet[UPDATE_MAX_CONTAIN_NUM_G08P] = {0} ;
	bool isLastChildFile = false, isUpdateBoot = false ;
	int status = API_SUCCESS, fileSize = 0 ;

	fseek(fp, G08P_FILE_HEADER_OFFSET, SEEK_SET) ;
	fread(&fileOffSet, 1, sizeof(fileOffSet), fp) ;
	for (int i = 0 ; (i < UPDATE_MAX_CONTAIN_NUM_G08P) && !isLastChildFile ; ++i)
	{
		if (0 != fileOffSet[i + 1])
			fileSize = fileOffSet[i + 1] - fileOffSet[i] - sizeof(FileInfoG08P) ;
		else
		{
			fileSize = FileSize(fp) - fileOffSet[i] - sizeof(FileInfoG08P) - G08P_FILE_HEADER_OFFSET ;
			isLastChildFile = true ;
		} // else

		if (API_SUCCESS != (status = UpdateFileG08P(updateOption, comPort, fp, G08P_FILE_HEADER_OFFSET + fileOffSet[i], fileSize, processBar, callBackUpdateProcessFun)))
			goto END_HANDLE ;
	} // for

END_HANDLE :
	return status ;
} // UpdateG08P

/**
*/
int UpdateFileG08P(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
    unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	FileInfoG08P fileInfo = {0} ;
	ConnectComport *connectCom = new ConnectComport() ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;
	TSIAckStatus tsiAckInfo ;
	int status = API_SUCCESS, oriBaudRate = 0, dfuUpfateMode = DFU_UPDATE_FW ;

	fseek(fp, offSet, SEEK_SET) ;
	fread (&fileInfo, 1, sizeof(fileInfo), fp) ;
	if (G08P_AP_ADDR == fileInfo.writeAddr)   // G08P AP 使用 DFU更新
	{   // 原始.mas檔案包含標頭, offset資訊 DFU更新時不需要這些資訊
		fseek(fp, offSet + G08P_FILE_DATA_OFFSET + 16, SEEK_SET) ;

		comInfo.comPort = comPort ;
        comInfo.writeBuf = &writeBuf[0] ;
		comInfo.resBuf = &resBuf[0] ;
		comInfo.timeOut = TIME_OUT ;
		comInfo.devType = DEVICE_TYPE_G08P ;

		connectCom->GetBaudRate(comPort, &oriBaudRate) ;
		if (DEVICE_BAUDRATE_DFU_BOOT != oriBaudRate)
		{
			pktInfo.cmd.name = CMD_DL_JUMP_ADDR ;
			comInfo.pktType = NO_ACK ;
			comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08P, pktInfo) ;
			if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
				return status ;
		} // if

		connectCom->SetBaudRate(&comInfo.comPort, DEVICE_BAUDRATE_DFU_BOOT, false) ;
		status = WriteFileDFU(comInfo.comPort, DFU_FIRMWARE_ID_V800, dfuUpfateMode, fp, FileRemainSize(fp), processBar, callBackUpdateProcessFun) ;
		if (dfuUpfateMode == DFU_UPDATE_FW)   // boot更新完後接著更新AP, baudrate不用切換
			connectCom->SetBaudRate(&comInfo.comPort, DEVICE_BAUDRATE_G08P, false) ;

		if (API_SUCCESS != status)
			return status ;

		Sleep(2500) ;  // 緩衝時間
		return (dfuUpfateMode == DFU_UPDATE_BOOT ? status : StartCommucateMASA(comInfo.comPort, DEVICE_TYPE_G08P)) ; // G08P 更新完後自動關掉溝通通道
	} // if

	if (G05_COURSE_ADDR == fileInfo.writeAddr)
		fileInfo.writeAddr = G08P_COURSE_ADDR ;

	if (API_SUCCESS != (status = ClearFlashG08P(comPort, fileInfo.writeAddr, fileSize)))
		return status ;

	return SendMASAPkt(comPort, fp, DEVICE_TYPE_G08P, G08P_PACKAGE_SIZE, CMD_DL_WRITE, fileInfo.writeAddr, fileSize, processBar, callBackUpdateProcessFun) ;
} // UpdateFileG08P

/**
*/
int ClearFlashG08P(HANDLE comPort, int handleAddr, int handelLen)
{
	unsigned char writeBuf[MASA_BUFFER_MAX_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
    unsigned char cmd = '\0' ;
	int blockSize = 0, fullToBlockSize = 0, status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	blockSize = (handleAddr >= G08P_FW_ADDR ? MCU_BLOCK_SIZE : AP_BLOCK_SIZE) ;
	fullToBlockSize = handelLen + (0 == handelLen % blockSize ? 0 : (blockSize - handelLen % blockSize)) ;

	pktInfo.cmd.name = (handleAddr >= G08P_FW_ADDR ? CMD_DL_ERASE_MCU : CMD_DL_ERASE) ;
	pktInfo.cmd.item.erase.numOfBlock = (handleAddr >= G08P_FW_ADDR ? G08P_NUM_OF_MCU_BLOCK : G08P_NUM_OF_FLASH_BLOCK) ;
	pktInfo.cmd.item.erase.fwStartAddress = (handleAddr >= G08P_FW_ADDR ? G08P_FW_ADDR : 0) ;
	pktInfo.cmd.item.erase.blockSize = blockSize ;
	pktInfo.handleAddress = handleAddr ;
	pktInfo.handleLength = fullToBlockSize ;

	comInfo.timeOut = (handelLen > 1024 * 1024 * 10 ? ERASE_FLASH_TIME_OUT * 10 : ERASE_FLASH_TIME_OUT * 5) ;
	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.devType = DEVICE_TYPE_G08P ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08P, pktInfo) ;
	return (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // ClearFlashG08P

/**
*/
int WriteSnG08P(HANDLE comPort, WriteSnSettingInfo snInfo)
{
	unsigned char data[BASE_DATA_SIZE] = {"\0"}, sn[BASE_DATA_SIZE] = {"\0"} ;
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int dataSize = 0, status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	MTKAckStatus mtkAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	dataSize = sprintf(data, "%s%05d\0", snInfo.dev.g05Series.sn.model, snInfo.writeSn) ;

    pktInfo.cmd.name = CMD_DL_WRITE_SN ;
	pktInfo.cmd.item.write.data = &data[0] ;
	pktInfo.handleLength = dataSize ;
	pktInfo.handleAddress = G08P_SERIAL_NUM_ADDR ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
    comInfo.devType = DEVICE_TYPE_G08P ;
	comInfo.pktType = NORMAL_PACKAGE ;
    comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08P, pktInfo) ;
    if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
		return status ;

	dataSize = sprintf(sn, "G08P%s%05d", snInfo.dev.g05Series.sn.model, snInfo.writeSn) ;
	if (API_SUCCESS != (status = AskDevInfoG08P(comPort, &tsiAckInfo, &mtkAckInfo)))
		return status ;

	if (0 != strncmp(tsiAckInfo.devInfo.base.devSeries, sn, dataSize))
		return EXTRA_TEST_WRITE_SN_ERROR ;
	// date code & lot number
	dataSize = sprintf(data, "%s%s\0", snInfo.dev.g05Series.sn.dateNum, snInfo.dev.g05Series.sn.lotNum) ;
	pktInfo.handleLength = dataSize ;
	pktInfo.handleAddress = G08P_DATE_NUM_ADDR ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08P, pktInfo) ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
        return status ;

	return (status = ResetDeviceSettingG08P(comPort)) ;
} // WriteSnG08P

/**
*/
int ClearFlash4KG08P(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int blockSize = 0x1000, numOfBlock = handelLen / blockSize, status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;
	bool needCallBack = (NULL != callBackUpdateProcessFun && NULL != processBar) ;
					// 4K
	if (0 != handleAddr % blockSize)
		return ClearFlashMASA_START_ADDR_ERROR ;

	if (0 != handelLen % blockSize)
		return ClearFlashMASA_CLEAR_RANGE_ERROR ;

	pktInfo.cmd.name = CMD_DL_ERASE_4K ;
	pktInfo.handleAddress = handleAddr ;
	pktInfo.handleLength = 0 ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = ERASE_FLASH_TIME_OUT ;
	comInfo.devType = DEVICE_TYPE_G08P ;
	comInfo.pktType = NORMAL_PACKAGE ;
	for (int i = 0 ; i < numOfBlock ; ++i, pktInfo.handleAddress += blockSize)
	{
		comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08P, pktInfo) ;
		if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
			return status ;

		if (needCallBack)
			callBackUpdateProcessFun(processBar, i, numOfBlock) ;
	} // for

	return API_SUCCESS ;
} // ClearFlash4KG08P

/**
*/
int ReadDataG08P(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS ;
	FILE *dwFp = NULL ;

	if (NULL == (dwFp = fopen(outputFileName, "wb+")))
		return FILE_NOT_EXIST ;

	status = SendMASAPkt(*comPort, dwFp, DEVICE_TYPE_G08P, G08P_PACKAGE_SIZE, CMD_DL_READ, handleAddr, handelLen, processBar, callBackUpdateProcessFun) ;
	fclose(dwFp) ;
	return status ;
} // ReadDataG08P
