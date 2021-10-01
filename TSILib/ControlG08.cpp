//---------------------------------------------------------------------------

#pragma hdrstop

#include "ControlG08.h"
#include "MASAPackage.h"
#include "Message.h"
#include "GetToken.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int ResetDeviceSettingG08(HANDLE comPort)
{
	return ClearFlash4KG08(comPort, G08_SETTING_ADDR, 0x1000) ;
} // ResetDeviceSettingG08

/**
*/
int AskDevInfoG08(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
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
	comInfo.devType = DEVICE_TYPE_G08 ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08, pktInfo) ;
	return (status = WriteToCom(&comInfo, tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // AskDevInfoG08

/**
*/
int UpdateG08(int updateOption, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int fileOffSet[UPDATE_MAX_CONTAIN_NUM_G08] = {0} ;
	bool isLastChildFile = false, isUpdateBoot = false ;
	int status = API_SUCCESS, fileSize = 0 ;

	fseek(fp, G08_FILE_HEADER_OFFSET, SEEK_SET) ;
	fread(&fileOffSet, 1, sizeof(fileOffSet), fp) ;
	for (int i = 0 ; (i < UPDATE_MAX_CONTAIN_NUM_G08) && !isLastChildFile ; ++i)
	{
		if (0 != fileOffSet[i + 1])
			fileSize = fileOffSet[i + 1] - fileOffSet[i] - sizeof(FileInfoG08) ;
		else
		{
			fileSize = FileSize(fp) - fileOffSet[i] - sizeof(FileInfoG08) - G08_FILE_HEADER_OFFSET ;
			isLastChildFile = true ;
		} // else

		if (API_SUCCESS != (status = UpdateFileG08(updateOption, comPort, fp, G08_FILE_HEADER_OFFSET + fileOffSet[i], fileSize, processBar, callBackUpdateProcessFun)))
			goto END_HANDLE ;
	} // for

END_HANDLE :
	return status ;
} // UpdateG08

/**
*/
int UpdateFileG08(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	FileInfoG08 fileInfo = {0} ;
	WriteComInfo comInfo ;
	TSIAckStatus tsiAckInfo ;
	int status = API_SUCCESS ;

	fseek(fp, offSet, SEEK_SET) ;
	fread (&fileInfo, 1, sizeof(fileInfo), fp) ;
    if (API_SUCCESS != (status = ClearFlashG08(comPort, fileInfo.writeAddr, fileSize)))
		return status ;

	return SendMASAPkt(comPort, fp, DEVICE_TYPE_G08, G08_PACKAGE_SIZE, CMD_DL_WRITE, fileInfo.writeAddr, fileSize, processBar, callBackUpdateProcessFun) ;
} // UpdateFileG08

/**
*/
int ClearFlashG08(HANDLE comPort, int handleAddr, int handelLen)
{
	unsigned char writeBuf[MASA_BUFFER_MAX_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
    unsigned char cmd = '\0' ;
	int blockSize = 0, fullToBlockSize = 0, status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	blockSize = (handleAddr >= G08_FW_ADDR ? MCU_BLOCK_SIZE : AP_BLOCK_SIZE) ;
	fullToBlockSize = handelLen + (0 == handelLen % blockSize ? 0 : (blockSize - handelLen % blockSize)) ;

	pktInfo.cmd.name = (handleAddr >= G08_FW_ADDR ? CMD_DL_ERASE_MCU : CMD_DL_ERASE) ;
	pktInfo.cmd.item.erase.numOfBlock = (handleAddr >= G08_FW_ADDR ? G08_NUM_OF_MCU_BLOCK : G08_NUM_OF_FLASH_BLOCK) ;
	pktInfo.cmd.item.erase.fwStartAddress = (handleAddr >= G08_FW_ADDR ? G08_FW_ADDR : 0) ;
	pktInfo.cmd.item.erase.blockSize = blockSize ;
	pktInfo.handleAddress = handleAddr ;
	pktInfo.handleLength = fullToBlockSize ;

	comInfo.timeOut = (handelLen > 1024 * 1024 * 10 ? ERASE_FLASH_TIME_OUT * 10 : ERASE_FLASH_TIME_OUT * 5) ;
	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.devType = DEVICE_TYPE_G08 ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08, pktInfo) ;
	return (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // ClearFlashG08

/**
*/
int WriteSnG08(HANDLE comPort, WriteSnSettingInfo snInfo)
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
	pktInfo.handleAddress = G08_SERIAL_NUM_ADDR ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.devType = DEVICE_TYPE_G08 ;
	comInfo.pktType = NORMAL_PACKAGE ;
    comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08, pktInfo) ;
    if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
		return status ;

	dataSize = sprintf(sn, "G08%s%05d", snInfo.dev.g05Series.sn.model, snInfo.writeSn) ;
	if (API_SUCCESS != (status = AskDevInfoG08(comPort, &tsiAckInfo, &mtkAckInfo)))
		return status ;

	if (0 != strncmp(tsiAckInfo.devInfo.base.devSeries, sn, dataSize))
		return EXTRA_TEST_WRITE_SN_ERROR ;
    // date code & lot number
	dataSize = sprintf(data, "%s%s\0", snInfo.dev.g05Series.sn.dateNum, snInfo.dev.g05Series.sn.lotNum) ;
	pktInfo.handleLength = dataSize ;
	pktInfo.handleAddress = G08_DATE_NUM_ADDR ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08, pktInfo) ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
        return status ;

	return (status = ResetDeviceSettingG08(comPort)) ;
} // WriteSnG08

/**
*/
int ClearFlash4KG08(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
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
	comInfo.devType = DEVICE_TYPE_G08 ;
	comInfo.pktType = NORMAL_PACKAGE ;
	for (int i = 0 ; i < numOfBlock ; ++i, pktInfo.handleAddress += blockSize)
	{
		comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G08, pktInfo) ;
		if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
			return status ;

		if (needCallBack)
			callBackUpdateProcessFun(processBar, i, numOfBlock) ;
	} // for

	return API_SUCCESS ;
} // ClearFlash4KG08

/**
*/
int ReadDataG08(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS ;
	FILE *dwFp = NULL ;

	if (NULL == (dwFp = fopen(outputFileName, "wb+")))
		return FILE_NOT_EXIST ;

	status = SendMASAPkt(*comPort, dwFp, DEVICE_TYPE_G08, G08_PACKAGE_SIZE, CMD_DL_READ, handleAddr, handelLen, processBar, callBackUpdateProcessFun) ;
	fclose(dwFp) ;
	return status ;
} // ReadDataG08
