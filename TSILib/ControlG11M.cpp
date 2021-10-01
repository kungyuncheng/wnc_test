//---------------------------------------------------------------------------

#pragma hdrstop

#include "ControlG11M.h"
#include "MASAPackage.h"
#include "Message.h"
#include "GetToken.h"
#include "ConnectComport.h"
#include "DFUPackage.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int ResetDeviceSettingG11m(HANDLE comPort)
{
	return ClearFlash4KG11m(comPort, G11M_SETTING_ADDR, 0x1000) ;
} // ResetDeviceSettingG11M

/**
*/
int AskDevInfoG11m(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	unsigned char g11mFlashVerTitle[G11M_FLASH_VER_NUM] = {'F', 'D', 'A'} ;
	unsigned char *g11mFlashVer[G11M_FLASH_VER_NUM] = {&tsiAckInfo->devInfo.extra.g05Series.flashInfoFi[0],
													   &tsiAckInfo->devInfo.extra.g05Series.flashInfoDig[0],
													   &tsiAckInfo->devInfo.extra.g05Series.flashInfoAnl[0]} ;
    int flashAddr[G11M_FLASH_VER_NUM] = {G11M_FLASH_VER1_ADDR, G11M_FLASH_VER2_ADDR, G11M_FLASH_VER3_ADDR} ;
	int ver = 0, status = API_SUCCESS ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	pktInfo.cmd.name = CMD_DL_DEV_INFO ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.devType = DEVICE_TYPE_G11M ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G11M, pktInfo) ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, tsiAckInfo, NULL, JudgeMASAPkt)))
		return status ;      // 問基本資訊

	pktInfo.cmd.name = CMD_DL_READ ;
	for (int i = 0 ; i < G11M_FLASH_VER_NUM ; ++i)
	{                       // G11m 額外資訊
		pktInfo.handleAddress = flashAddr[i] ;
		pktInfo.handleLength = sizeof(int) ;
		comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G11M, pktInfo) ;
		if (API_SUCCESS != (status = WriteToCom(&comInfo, tsiAckInfo, NULL, JudgeMASAPkt)))
			return status ;

		memcpy(&ver, resBuf + 13, sizeof(ver)) ;
		if (0xff == (ver &= 0xff))
			ver = 0 ;

		ver = (ver + 1) % 0xff ;
		sprintf(g11mFlashVer[i], "%c%02d\0", g11mFlashVerTitle[i], ver) ;
	} // for

	return status ;
} // AskDevInfoG11M

/**
*/
int UpdateG11m(int updateOption, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	FWFileInfoG11m fileTable ;
	int status = API_SUCCESS ;

	if (UPDATE_PATH_FW == updateOption)
	{   // 新格式的fw檔包含 ap, boot, flash
		fread(&fileTable, 1, sizeof(fileTable), fp) ;
		for (int i = 0 ; i < fileTable.childFileNum ; ++i)
		{
			if (API_SUCCESS != (status = UpdateFwFileG11m(fileTable.dataMark[i], comPort, fp, fileTable.fileOffset[i], fileTable.fileLen[i], processBar, callBackUpdateProcessFun)))
				return status ;
		} // for

		return status ;
	} // if

	return UpdateAnotherG11m(0, comPort, fp, processBar, callBackUpdateProcessFun) ;   // 其餘檔案更新流程與之前相同
} // UpdateG011m

/**
*/
int UpdateFwFileG11m(int updateEvent, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	WriteComInfo comInfo ;
	TSIAckStatus tsiAckInfo ;
	CreateMasaPktInfo pktInfo ;
	int status = API_SUCCESS, oriBaudRate = 0, dfuUpfateMode = ((updateEvent & G11M_MARK_4_AP) ? DFU_UPDATE_FW : DFU_UPDATE_BOOT) ;
	ConnectComport *connectCom = new ConnectComport() ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.devType = DEVICE_TYPE_G11M ;

	fseek(fp, offSet, SEEK_SET) ;
	connectCom->GetBaudRate(comPort, &oriBaudRate) ;
	if (updateEvent & (G11M_MARK_4_AP | G11M_MARK_4_BOOT))
	{
		if (DEVICE_BAUDRATE_DFU_BOOT != oriBaudRate)
		{
			pktInfo.cmd.name = CMD_DL_JUMP_ADDR ;
			comInfo.pktType = NO_ACK ;
			comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G11M, pktInfo) ;
			if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
				return status ;
		} // if

		connectCom->SetBaudRate(&comInfo.comPort, DEVICE_BAUDRATE_DFU_BOOT, false) ;
		status = WriteFileDFU(comInfo.comPort, DFU_FIRMWARE_ID_V800, dfuUpfateMode, fp, fileSize, processBar, callBackUpdateProcessFun) ;
		if (dfuUpfateMode == DFU_UPDATE_FW)
			connectCom->SetBaudRate(&comInfo.comPort, DEVICE_BAUDRATE_G11M, false) ;

		if (API_SUCCESS != status)
			return status ;

		Sleep(2500) ;  // 緩衝時間
		return (dfuUpfateMode == DFU_UPDATE_BOOT ? status : StartCommucateMASA(comInfo.comPort, DEVICE_TYPE_G11M)) ; // G011M AP 更新完後自動關掉溝通通道
	} // if
	/* 更新flash, 固定從0開始  */
	if (API_SUCCESS != (status = ClearFlashSeriesG11m(comPort, 0, fileSize)))
		return status ;

	return (status = SendMASAPkt(comPort, fp, DEVICE_TYPE_G11M, G11M_PACKAGE_SIZE, CMD_DL_WRITE, 0, fileSize, processBar, callBackUpdateProcessFun)) ;
} // UpdateFileG011m

/**
*/
int ClearFlashSeriesG11m(HANDLE comPort, int handleAddr, int handelLen)
{
	unsigned char writeBuf[MASA_BUFFER_MAX_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	CreateMasaPktInfo pktInfo ;
	WriteComInfo comInfo ;

	pktInfo.cmd.name = CMD_DL_SEL_ERASE ;
	pktInfo.handleAddress = handleAddr ;
	pktInfo.handleLength = handelLen ;
    pktInfo.cmd.item.erase.blockSize = G11M_BLOCK_SIZE ;
					// 每 0x2000 需 2s
	comInfo.timeOut = ((handelLen / 0x2000) + (0 == handelLen % 0x2000 ? 0 : 1)) * TIME_OUT ;
	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.devType = DEVICE_TYPE_G11M ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G11M, pktInfo) ;
	return (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // ClearFlashG11m

/**
*/
int UpdateAnotherG11m(int updateOption, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int fileOffSet[UPDATE_MAX_CONTAIN_NUM_G11M] = {0} ;
	bool isLastChildFile = false, isUpdateBoot = false ;
	int status = API_SUCCESS, fileSize = 0 ;

	fseek(fp, G11M_FILE_HEADER_OFFSET, SEEK_SET) ;
	fread(&fileOffSet, 1, sizeof(fileOffSet), fp) ;
	for (int i = 0 ; (i < UPDATE_MAX_CONTAIN_NUM_G11M) && !isLastChildFile ; ++i)
	{
		if (0 != fileOffSet[i + 1])
			fileSize = fileOffSet[i + 1] - fileOffSet[i] - sizeof(AnotherFileInfoG11m) ;
		else
		{
			fileSize = FileSize(fp) - fileOffSet[i] - sizeof(AnotherFileInfoG11m) - G11M_FILE_HEADER_OFFSET ;
			isLastChildFile = true ;
		} // else

		if (API_SUCCESS != (status = UpdateAnotherFileG11m(updateOption, comPort, fp, G11M_FILE_HEADER_OFFSET + fileOffSet[i], fileSize, processBar, callBackUpdateProcessFun)))
			goto END_HANDLE ;
	} // for

END_HANDLE :
	return status ;
} // UpdateAnotherG11m

/**
*/
int UpdateAnotherFileG11m(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	AnotherFileInfoG11m fileInfo = {0} ;
	WriteComInfo comInfo ;
	TSIAckStatus tsiAckInfo ;
	int status = API_SUCCESS ;

	fseek(fp, offSet, SEEK_SET) ;
	fread (&fileInfo, 1, sizeof(fileInfo), fp) ;
	if (API_SUCCESS != (status = ClearFlashG11m(comPort, fileInfo.writeAddr, fileSize)))
		return status ;

	return SendMASAPkt(comPort, fp, DEVICE_TYPE_G11M, G11M_PACKAGE_SIZE, CMD_DL_WRITE, fileInfo.writeAddr, fileSize, processBar, callBackUpdateProcessFun) ;
} // UpdateAnotherFileG11m

/**
*/
int ClearFlashG11m(HANDLE comPort, int handleAddr, int handelLen)
{
	unsigned char writeBuf[MASA_BUFFER_MAX_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
    unsigned char cmd = '\0' ;
	int blockSize = 0, fullToBlockSize = 0, status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	blockSize = (handleAddr >= G11M_FW_ADDR ? MCU_BLOCK_SIZE : AP_BLOCK_SIZE) ;
	fullToBlockSize = handelLen + (0 == handelLen % blockSize ? 0 : (blockSize - handelLen % blockSize)) ;

	pktInfo.cmd.name = (handleAddr >= G11M_FW_ADDR ? CMD_DL_ERASE_MCU : CMD_DL_ERASE) ;
	pktInfo.cmd.item.erase.numOfBlock = (handleAddr >= G11M_FW_ADDR ? G11M_NUM_OF_MCU_BLOCK : G11M_NUM_OF_FLASH_BLOCK) ;
	pktInfo.cmd.item.erase.fwStartAddress = (handleAddr >= G11M_FW_ADDR ? G11M_FW_ADDR : 0) ;
	pktInfo.cmd.item.erase.blockSize = blockSize ;
	pktInfo.handleAddress = handleAddr ;
	pktInfo.handleLength = fullToBlockSize ;

	comInfo.timeOut = (handelLen > 1024 * 1024 * 10 ? ERASE_FLASH_TIME_OUT * 10 : ERASE_FLASH_TIME_OUT * 5) ;
	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.devType = DEVICE_TYPE_G11M ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G11M, pktInfo) ;
	return (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // ClearFlashG11m

/**
*/
int WriteSnG11m(HANDLE comPort, WriteSnSettingInfo snInfo)
{
	const int G11M_SERIES_OFFSET = 2, G11M_SERIES_SIZE = 32 ;
	unsigned char data[BASE_DATA_SIZE] = {"\0"}, sn[BASE_DATA_SIZE] = {"\0"} ;
    unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;

	int dataSize = 0, status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	MTKAckStatus mtkAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	dataSize = sprintf(data, "G011M%s%05d\0", snInfo.dev.g05Series.sn.model, snInfo.writeSn) ;
    sprintf(data + dataSize + G11M_SERIES_OFFSET, "%s%s\0", snInfo.dev.g05Series.sn.dateNum, snInfo.dev.g05Series.sn.lotNum) ;
	dataSize = G11M_SERIES_SIZE ;      // G11M 連 date code 一起寫
                                       // 序號格式與G11c一樣, 14個序號 + 2個 '\0', 再接 date code, 最後補滿 '\0'至 32 byte
    pktInfo.cmd.name = CMD_DL_WRITE_SN ;
	pktInfo.cmd.item.write.data = &data[0] ;
	pktInfo.handleLength = dataSize ;
	pktInfo.handleAddress = G11M_SERIAL_NUM_ADDR ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.devType = DEVICE_TYPE_G11M ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G11M, pktInfo) ;
    if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
		return status ;

	dataSize = sprintf(sn, "G011M%s%05d", snInfo.dev.g05Series.sn.model, snInfo.writeSn) ;
	if (API_SUCCESS != (status = AskDevInfoG11m(comPort, &tsiAckInfo, &mtkAckInfo)))
		return status ;

	if (0 != strncmp(tsiAckInfo.devInfo.base.devSeries, sn, dataSize))
		return EXTRA_TEST_WRITE_SN_ERROR ;

    return (status = ResetDeviceSettingG11m(comPort)) ;
} // WriteSnG11m

/**
*/
int ReadDataG11m(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS ;
	FILE *dwFp = NULL ;

	if (NULL == (dwFp = fopen(outputFileName, "wb+")))
		return FILE_NOT_EXIST ;

	status = SendMASAPkt(*comPort, dwFp, DEVICE_TYPE_G11M, G11M_PACKAGE_SIZE, CMD_DL_READ, handleAddr, handelLen, processBar, callBackUpdateProcessFun) ;
	fclose(dwFp) ;
	return status ;
} // ReadDataG11m

/**
*/
int ClearFlash4KG11m(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int blockSize = 0x1000, startAddr = handleAddr, numOfBlock = handelLen / blockSize, status = API_SUCCESS ;
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
	comInfo.devType = DEVICE_TYPE_G11M ;
	comInfo.pktType = NORMAL_PACKAGE ;
	for (int i = 0 ; i < numOfBlock ; ++i, pktInfo.handleAddress += blockSize)
	{
		comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G11M, pktInfo) ;
		if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
			return status ;

		if (needCallBack)
			callBackUpdateProcessFun(processBar, i, numOfBlock) ;
	} // for

	return API_SUCCESS ;
} // ClearFlash4KG11m
