//---------------------------------------------------------------------------

#pragma hdrstop

#include "ControlG06.h"
#include "Message.h"
#include "GetToken.h"
#include "ConnectComport.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int AskDevInfoG06(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT * 3 ;
	info.devType = DEVICE_TYPE_G06 ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%04d,%d,%08X*", TSI_OPTION_ID_1600, TSI_CMD_READ, G06_SERIES_NUM_ADDR) ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d*", TSI_OPTION_ID_997) ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	if (0xa4 == tsiAckInfo->readData[12])
		tsiAckInfo->readData[12] = 0x24 ;

	if (0xa4 == tsiAckInfo->readData[13])
		tsiAckInfo->readData[13] = 0x24 ;

	for (int i = 0, j = 0 ; j < SERIES_SIZE_G06 ; )
	{
		if (j < 4)
			sprintf((tsiAckInfo->devInfo.base.devSeries + i++), "%c", tsiAckInfo->readData[j++]) ;
		else if (4 == j || 6 == j || 9 == j || 14 == j)
			j += (4 == j ? 1 : 2) ; // reserve index
		else
		{
			sprintf((tsiAckInfo->devInfo.base.devSeries + i), ((5 == j || 12 == j || 13 == j) ? "%02x" : "%x"), tsiAckInfo->readData[j]) ;
			((5 == j || 12 == j || 13 == j) ? i += 2 : i += 1) ;
				++j ;
		} // else
	} // for

	return status ;
} // AskDevInfoG06

/**
* @brief write a series command so that device change boot state to bool loader.
* @param[in] *comPort comport which connect to device.
* @param[in] devType option of different device.
* @note after device change to boot loader must change baud rate.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int JumpToBootG06(HANDLE *comPort)
{
	unsigned char bootRomStartCmd[BOOT_ROM_START_CMD_NUM] = {BOOT_ROM_START_CMD1, BOOT_ROM_START_CMD2, BOOT_ROM_START_CMD3, BOOT_ROM_START_CMD4} ;
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	ConnectComport *connectCom = new ConnectComport() ;
	WriteComInfo info ;
	TSIAckStatus tsiAckInfo ;

	info.comPort = *comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT ;
																	  // $PTSI998,0,00000000*X/r/n
	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d,%08X*", TSI_OPTION_ID_998, TSI_STATUS_BOOT, time(NULL)) ;
	info.devType = DEVICE_TYPE_G06 ;
	info.pktType = NO_ACK ;
	if (API_SUCCESS != (status = WriteToCom(&info, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;                                               // 2 等待  wait 1.6 second
																	  // 3 提高baud change 至 460800
	connectCom->SetBaudRate(comPort, DEVICE_BAUDRATE_G06_BOOT, false) ;
	delete connectCom ;

	for (int i = 0 ; i < BOOT_ROM_START_CMD_NUM ; ++i)
	{   // 4 送出A0 0A 50 05 device會回傳 5F F5 AF FA
		info.writeBufSize = safe_memcpy(info.writeBuf, &bootRomStartCmd[i], sizeof(unsigned char)) ;
		info.pktType = NORMAL_PACKAGE ;
		if (API_SUCCESS != (status = WriteToCom(&info, NULL, NULL, JudgeBootRomPkt)))
			return status ;
	} // for

	return status ;
} // JumpToBootG06

/**
* @brief jump to target flash address.
* @param[in] comPort comport which connect to device.
* @param[in] addr target address.
* @param[in] devType option of different device.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int JumpToAddrG06(HANDLE *comPort, int addr)
{   // 6 A8 00 00 00 20 01 00 00 00
	// 這是一個讓他jump to 0x20000000的packet
	// 更新檔案前跳到 0x20000000 (大概需要1秒的緩衝)  更新後跳到 0x08001400
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int cmdLen = 0x01, status = API_SUCCESS ;
	ConnectComport *connectCom = new ConnectComport() ;
	WriteComInfo info ;

	info.comPort = *comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT ;

	info.writeBufSize = CreateBootRomPkt(info.writeBuf, NULL, BOOT_ROM_JUMP_CMD, addr, cmdLen) ;
    info.pktType = NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, NULL, NULL, JudgeBootRomPkt)))
		return status ;

	if (G06_AP_ADDR == addr)
		connectCom->SetBaudRate(comPort, DEVICE_BAUDRATE_G06, false) ;

	delete connectCom ;
	return status ;
} // JumpToAddrG06

/**
*/
int UpdateG06(int updateIdx, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	FileInfoG06 fileInfo ;
	int status = API_SUCCESS ;

	fread(&fileInfo, 1, sizeof(FileInfoG06), fp) ;
	for (int i = 0 ; i < fileInfo.totalFileNum ; ++i)
	{
		fseek(fp, fileInfo.fileOffset[i], SEEK_SET) ;
		fread(&fileInfo.childInfo[i], 1 , sizeof(ChileFileInfoG06), fp) ;
		if (API_SUCCESS != (status = WriteChildFileG06(comPort, fp, fileInfo, i, processBar, callBackUpdateProcessFun)))
			return status ;
	} // for

	return status ;
	/* 7. 分析要燒錄的files
		header 1024 bytes

		struct
		{
			int numberOfFilesToBeProcessed;
			fileSize[10];
			int offset[10];
		}

		這個structure一定沒有1024 bytes，剩下的空間補0x00

		seek(offset[i])可以找到file#i的資料。這個資料裡面最前面16 bytes是header，格式如下
		int writeAddr ;
		int fileSize ;
		int checkSum;
		char reserve[4] (“TSI”);

		後面接著的是要燒錄進去的資料，這部分的長度是 fileSize
		舉例來說，如果它的數值是

		writeAddr = 0x00000000, fileSize = 0x00000200

		那麼要送出去的packet就是

		A1 00 00 00 00 00 80 00 00 [128 bytes payload]
		A1 00 10 00 00 00 80 00 00 [128 bytes payload]

		****************有一個例外*******************

		如果 writeAddr = 0x080013f0
		則所要燒的資料就要包含這16 bytes的header，而且長度是fileSize + 16
		所以他要送出去的packet看起來將會是

		A1 F0 13 00 08 00 80 00 00 [F0 13 00 08 ....]
		A1 00 14 00 08 00 80 00 00 [....]
		A1 10 14 00 08 00 80 00 00 [....]
		*/
} // UpdateG06

/**
* @brief a uplode file maybe include another child file to update.
* @param[in] comPort comport which connect to device.
* @param[in] *readFile parent file pointer.
* @param[in] devType option of different device.
* @param[in] headerInfo a struct save child file's information.
* @param[in] childFileIdx index of now uplode child file.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int inline WriteChildFileG06(HANDLE comPort, FILE *fp, FileInfoG06 fileInfo, int fIdx, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned int checkSum = 0, writeAddr = 0, fileSize = 0, status = API_SUCCESS ;

	checkSum = CalculateFileCheckSumG06(fp, fileInfo.childInfo[fIdx].fileSize, fileInfo.fileOffset[fIdx] + sizeof(fileInfo.childInfo[fIdx])) ;
	if (checkSum != fileInfo.childInfo[fIdx].checkSum)
		return FILE_CHECKSUM_ERROR ;

	writeAddr = fileInfo.childInfo[fIdx].writeAddr ;
	fileSize = fileInfo.childInfo[fIdx].fileSize ;
	switch (writeAddr)
	{
		case 0x00000000 :
			fileSize = fileInfo.fileSize[fIdx] - 0x90 ;
			fseek(fp, fileInfo.fileOffset[fIdx] + sizeof(fileInfo.childInfo[fIdx]), SEEK_SET) ;
			break ;
		case 0x00040000 :
		case 0x08000000 :
		case 0x00421000 :
		case 0x445000 :
			fileSize = fileInfo.fileSize[fIdx] - sizeof(fileInfo.childInfo[fIdx]) ;
			fseek(fp, fileInfo.fileOffset[fIdx] + sizeof(fileInfo.childInfo[fIdx]), SEEK_SET) ;
			break ;
		default :
			fileSize = fileInfo.fileSize[fIdx] ;
			fseek(fp, fileInfo.fileOffset[fIdx], SEEK_SET) ;
			break ;
	} // switch

	return (status = SendBootRomPkt(comPort, fp, BOOT_ROM_WRITE_CMD, writeAddr, fileSize, processBar, callBackUpdateProcessFun)) ;
} // WriteChildFileG06VoiceCoach

/**
*/
int ClearFlash4KG06(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS, startTime = 0 ;
	int checkSum = 0 ;
	WriteComInfo info ;
	TSIAckStatus tsiAckInfo ;

	if (API_SUCCESS != (status = JumpToAddrG06(&comPort, G06_AP_ADDR))) // 重開機
		return status ;

	Sleep(1000) ;

	info.comPort = comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.devType = DEVICE_TYPE_G06 ;
	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%04d,%d,%08X*", TSI_OPTION_ID_1600, TSI_ERASE_FLASH, handleAddr) ;

	info.pktType = MTK_NORMAL_PACKAGE ;
	info.timeOut = TIME_OUT * 3 ;
	return (status = WriteToCom(&info, &tsiAckInfo, NULL, JudgeTSIPkt)) ;
} // ClearFlash4KG06

/**
*/
int ReadDataG06(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackFun)
{
	FILE *rwFp = fopen(outputFileName, "wb+") ;
	int status = API_SUCCESS ;

	if (NULL == rwFp)
		return FILE_NOT_EXIST ;

	if (API_SUCCESS != (status = JumpToAddrG06(comPort, G06_RAM_ADDR)))
		return status ;

	Sleep(1000) ;
	return (status = SendBootRomPkt(*comPort, rwFp, BOOT_ROM_READ_CMD, handleAddr, handelLen, processBar, callBackFun)) ;
} // ReadDataG06

/**
*/
int WriteSnG06(HANDLE comPort, WriteSnSettingInfo snInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	unsigned char hexChr[4] = {"\0"}, model[5] = {"\0"}, month = '\0', batch = '\0', num[3] = {"\0"} ;
	unsigned char sn[BASE_DATA_SIZE] = {"\0"} ;
	int status = API_SUCCESS, dataSize = 0 ;
	TSIAckStatus tsiAckInfo ;
	MTKAckStatus mtkAckInfo ;
	WriteComInfo comInfo ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
                                                           // 清除序號位址資料
	comInfo.writeBufSize = CreateTSIPkt(comInfo.writeBuf, "$PTSI%04d,%d,%08X*", TSI_OPTION_ID_1600, TSI_ERASE_FLASH, G06_SERIES_NUM_ADDR) ;
	comInfo.devType = DEVICE_TYPE_G06 ;
	comInfo.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	sprintf(model, "%s%s", snInfo.dev.g06.sn.mode1, snInfo.dev.g06.sn.mode2) ;
	for (int i = 0, temp = 0, mask = 10000, mask2 = 100 ; i < 3 ; ++i, mask /= 100)
	{
		if (0x24 == (temp = (snInfo.writeSn / mask) % mask2))
			temp = 0xA4 ;

		sprintf(hexChr, "0x%02d\0", temp) ;
		sprintf(&num[i], "%c", strtol(hexChr, NULL, 16)) ;
	} // for

	month = strtol(strcat(strcpy(hexChr, "0x"), snInfo.dev.g06.sn.mode3), NULL, 16) ;
	batch = strtol(strcat(strcpy(hexChr, "0x"), snInfo.dev.g06.sn.mode4), NULL, 16) ;

	comInfo.writeBufSize = CreateTSIPkt(comInfo.writeBuf, "$PTSI%04d,%d,%08X,%s%c%c%c%c%c%c%c%c%c%c%c%c*",
														   TSI_OPTION_ID_1600, TSI_CMD_WRITE, G06_SERIES_NUM_ADDR,
														   model, 0x00, month, 0x00, 0x00, batch, 0x00, 0x00,
														   num[0], num[1], num[2], 0x00, 0x00) ;
	comInfo.pktType = MTK_NORMAL_PACKAGE ;
    if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	dataSize = sprintf(sn, "%s%s%s%s%05d", snInfo.dev.g06.sn.mode1, snInfo.dev.g06.sn.mode2, snInfo.dev.g06.sn.mode3,
										   snInfo.dev.g06.sn.mode4, snInfo.writeSn) ;
	if (API_SUCCESS != (status = AskDevInfoG06(comPort, &tsiAckInfo, &mtkAckInfo)))
		return status ;

	if (0 != strncmp(tsiAckInfo.devInfo.base.devSeries, sn, dataSize))
		return EXTRA_TEST_WRITE_SN_ERROR ;  // 檢查是否寫入

	return status ;
} // ExtraTestG06Series

/**
*/
void ReadLogInfoG06(FILE *fp, LogInfoG06 *info)
{
	void *buf[LOG_BUF_NUM_G06] = {&info->utc, &info->latitude, &info->longtitude,
								  &info->gpsValid, &info->reserve, &info->courseId,
								  &info->holeNum, &info->holeNumSeq, &info->strokesAtCurHole} ;
	int bufSize[LOG_BUF_NUM_G06] = {sizeof(info->utc), sizeof(info->latitude), sizeof(info->longtitude),
									sizeof(info->gpsValid), sizeof(info->reserve), sizeof(info->courseId),
									sizeof(info->holeNum), sizeof(info->holeNumSeq), sizeof(info->strokesAtCurHole)} ;

	for (int i = 0 ; i < LOG_BUF_NUM_G06 ; ++i)
		fread(buf[i], 1, bufSize[i], fp) ;
} // ReadLogInfoG06

/**
*/
unsigned int CalculateFileCheckSumG06(FILE *readFile, unsigned int fileLen, unsigned int nowAddr)
{
	unsigned int checkSum = 0 ;

	for (int i = 0; i < fileLen ; i++)
		checkSum += fgetc(readFile) ;

	fseek(readFile, nowAddr, SEEK_SET) ;
	return checkSum ;
} // CalculateFileCheckSumG06
