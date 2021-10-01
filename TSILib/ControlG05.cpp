//---------------------------------------------------------------------------

#pragma hdrstop

#include "ControlG05.h"
#include "MASAPackage.h"
#include "Message.h"
#include "GetToken.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int ResetDeviceSettingG05(HANDLE comPort)
{
	const int eraseCount = 2, eraseAddr[eraseCount] = {G05_SETTING_ADDR, G05_SETTING_BACKUP_ADDR} ;
	int status = API_SUCCESS ;

	for (int i = 0 ; i < eraseCount ; ++i)
	{                                                                                      // erase 4k
		if (API_SUCCESS != (status = ClearFlash4KG05(comPort, eraseAddr[i], 0x1000)))
			return status ;
	} // for

	return status ;
} // ResetDeviceSettingG05

/**
*/
int AskDevInfoG05(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo = {0} ;

	pktInfo.cmd.name = CMD_DL_DEV_INFO ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.devType = DEVICE_TYPE_G05 ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G05, pktInfo) ;
	return (status = WriteToCom(&comInfo, tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // AskDevInfoG05

/**
*/
int UpdateG05(int updateOption, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int fileOffSet[UPDATE_MAX_CONTAIN_NUM_G05] = {0} ;
	bool isLastChildFile = false, isUpdateBoot = false ;
	int status = API_SUCCESS, fileSize = 0 ;

	fseek(fp, G05_FILE_HEADER_OFFSET, SEEK_SET) ;
	fread(&fileOffSet, 1, sizeof(fileOffSet), fp) ;
	for (int i = 0 ; (i < UPDATE_MAX_CONTAIN_NUM_G05) && !isLastChildFile ; ++i)
	{
		if (0 != fileOffSet[i + 1])
			fileSize = fileOffSet[i + 1] - fileOffSet[i] - sizeof(FileInfoG05) ;
		else
		{
			fileSize = FileSize(fp) - fileOffSet[i] - sizeof(FileInfoG05) - G05_FILE_HEADER_OFFSET ;
			isLastChildFile = true ;
		} // else

		if (API_SUCCESS != (status = UpdateFileG05(updateOption, comPort, fp, G05_FILE_HEADER_OFFSET + fileOffSet[i], fileSize, processBar, callBackUpdateProcessFun)))
			goto END_HANDLE ;
	} // for

END_HANDLE :
	return status ;
} // UpdateG05

/**
* @param[in] comPort comport which connect to device.
* @param[in] *readFile parent file pointer.
* @param[in] devType option of different device.
*/
int UpdateFileG05(int updateOption, HANDLE comPort, FILE *fp, int offSet, int fileSize, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	FileInfoG05 fileInfo = {0} ;
	WriteComInfo comInfo ;
	TSIAckStatus tsiAckInfo ;
	int status = API_SUCCESS ;

	fseek(fp, offSet, SEEK_SET) ;
	fread (&fileInfo, 1, sizeof(fileInfo), fp) ;
	if (API_SUCCESS != (status = ClearFlashG05(comPort, fileInfo.writeAddr, fileSize)))
		return status ;

	return SendMASAPkt(comPort, fp, DEVICE_TYPE_G05, G05_PACKAGE_SIZE, CMD_DL_WRITE, fileInfo.writeAddr, fileSize, processBar, callBackUpdateProcessFun) ;
} // UpdateFileG05

/**
*/
int ClearFlashG05(HANDLE comPort, int handleAddr, int handelLen)
{
	unsigned char writeBuf[MASA_BUFFER_MAX_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int blockSize = 0, fullToBlockSize = 0, status = API_SUCCESS ;
	TSIAckStatus tsiAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo = {0} ;

	blockSize = (handleAddr >= G05_FW_ADDR ? MCU_BLOCK_SIZE : AP_BLOCK_SIZE) ;
	fullToBlockSize = handelLen + (0 == handelLen % blockSize ? 0 : (blockSize - handelLen % blockSize)) ;

	pktInfo.cmd.name = (handleAddr >= G05_FW_ADDR ? CMD_DL_ERASE_MCU : CMD_DL_ERASE) ;
	pktInfo.cmd.item.erase.numOfBlock = (handleAddr >= G05_FW_ADDR ? G05_NUM_OF_MCU_BLOCK : G05_NUM_OF_FLASH_BLOCK) ;
    pktInfo.cmd.item.erase.fwStartAddress = (handleAddr >= G05_FW_ADDR ? G05_FW_ADDR : 0) ;
	pktInfo.cmd.item.erase.blockSize = blockSize ;

	pktInfo.handleAddress = handleAddr ;
	pktInfo.handleLength = fullToBlockSize ;

	comInfo.timeOut = (handelLen > 1024 * 1024 * 10 ? ERASE_FLASH_TIME_OUT * 10 : ERASE_FLASH_TIME_OUT * 5) ;
	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.devType = DEVICE_TYPE_G05 ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G05, pktInfo) ;
	return (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // ClearFlashG05

/**
*/
int WriteSnG05(HANDLE comPort, WriteSnSettingInfo snInfo)
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
	pktInfo.handleAddress = G05_SERIAL_NUM_ADDR ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
    comInfo.devType = DEVICE_TYPE_G05 ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G05, pktInfo) ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
		return status ;

	dataSize = sprintf(sn, "G05%s%05d", snInfo.dev.g05Series.sn.model, snInfo.writeSn) ;
	if (API_SUCCESS != (status = AskDevInfoG05(comPort, &tsiAckInfo, &mtkAckInfo)))
		return status ;

	if (0 != strncmp(tsiAckInfo.devInfo.base.devSeries, sn, dataSize))
		return EXTRA_TEST_WRITE_SN_ERROR ;

    return (status = ResetDeviceSettingG05(comPort)) ;
} // WriteSnG05

/**
*/
void ReadLogInfoG05Series(FILE *fp, LogInfoG05Series *info, int devType)
{
	void *buf[LOG_BUF_NUM_G05_SERIES] = {&info->year, &info->month, &info->day,
										 &info->hour, &info->minute, &info->second,
										 &info->courseId, &info->holeNum, &info->holeNumSeq,
										 &info->latitude, &info->longtitude, &info->hight, &info->reserve} ;
	int bufSize[LOG_BUF_NUM_G05_SERIES] = {sizeof(info->year), sizeof(info->month), sizeof(info->day),
										   sizeof(info->hour), sizeof(info->minute), sizeof(info->second),
										   sizeof(info->courseId), sizeof(info->holeNum), sizeof(info->holeNumSeq),
										   sizeof(info->latitude), sizeof(info->longtitude), sizeof(info->hight), sizeof(info->reserve)} ;

    for (int i = 0 ; i < LOG_BUF_NUM_G05_SERIES ; ++i)
		fread(buf[i], 1, bufSize[i], fp) ;
} // ReadLogInfoG05Series

/**
*/
int ClearFlash4KG05(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
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
	comInfo.devType = DEVICE_TYPE_G05 ;
	comInfo.pktType = NORMAL_PACKAGE ;
	for (int i = 0 ; i < numOfBlock ; ++i, pktInfo.handleAddress += blockSize)
	{
		comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, DEVICE_TYPE_G05, pktInfo) ;
		if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)))
			return status ;

		if (needCallBack)
			callBackUpdateProcessFun(processBar, i, numOfBlock) ;
	} // for

	return API_SUCCESS ;
} // ClearFlash4KG05

/**
*/
int ReadDataG05(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS ;
	FILE *dwFp = NULL ;

	if (NULL == (dwFp = fopen(outputFileName, "wb+")))
		return FILE_NOT_EXIST ;

	status = SendMASAPkt(*comPort, dwFp, DEVICE_TYPE_G05, G05_PACKAGE_SIZE, CMD_DL_READ, handleAddr, handelLen, processBar, callBackUpdateProcessFun) ;
	fclose(dwFp) ;
	return status ;
} // ReadDataG05
