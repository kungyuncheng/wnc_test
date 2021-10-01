//---------------------------------------------------------------------------

#pragma hdrstop

#include "ControlP51.h"
#include "Message.h"
#include "GetToken.h"
#include "ConnectComport.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int AskDevInfoP51(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT * 3 ;
	info.devType = DEVICE_TYPE_P51 ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d*", TSI_OPTION_ID_510, TSI_QUERY_STATUS) ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	return (status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt)) ;
} // AskDevInfoP51

/**
* @brief write a series command so that device change boot state to bool loader.
* @param[in] *comPort comport which connect to device.
* @param[in] devType option of different device.
* @note after device change to boot loader must change baud rate.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int JumpToBootP51(HANDLE *comPort)
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
	info.devType = DEVICE_TYPE_P51 ;
	info.pktType = NO_ACK ;
	if (API_SUCCESS != (status = WriteToCom(&info, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;                                               // 2 等待  wait 1.6 second
                                                                      // 3 提高baud change 至 460800
	connectCom->SetBaudRate(comPort, DEVICE_BAUDRATE_P51_BOOT, false) ;
	delete connectCom ;

	for (int i = 0 ; i < BOOT_ROM_START_CMD_NUM ; ++i)
	{   // 4 送出A0 0A 50 05 device會回傳 5F F5 AF FA
		info.writeBufSize = safe_memcpy(info.writeBuf, &bootRomStartCmd[i], sizeof(unsigned char)) ;
		info.pktType = NORMAL_PACKAGE ;
		if (API_SUCCESS != (status = WriteToCom(&info, NULL, NULL, JudgeBootRomPkt)))
			return status ;
	} // for

	return status ;
} // JumpToBootP51

/**
* @brief jump to target flash address.
* @param[in] comPort comport which connect to device.
* @param[in] addr target address.
* @param[in] devType option of different device.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int JumpToAddrP51(HANDLE *comPort, int addr)
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

	connectCom->SetBaudRate(comPort, DEVICE_BAUDRATE_P51, false) ;
	delete connectCom ;
	return status ;
} // JumpToAddrP51

/**
*/
int UpdateP51(int updateIdx, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	FileInfoP51 fileInfo ;
	int status = API_SUCCESS ;

	fread(&fileInfo, 1, sizeof(FileInfoP51), fp) ;
	for (int i = 0 ; i < fileInfo.totalFileNum ; ++i)
	{
		fseek(fp, fileInfo.fileOffset[i], SEEK_SET) ;
		fread(&fileInfo.childInfo[i], 1 , sizeof(ChileFileInfoP51), fp) ;
		if (API_SUCCESS != (status = WriteChildFileP51(comPort, fp, fileInfo, i, processBar, callBackUpdateProcessFun)))
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
} // UpdateP51

/**
* @brief a uplode file maybe include another child file to update.
* @param[in] comPort comport which connect to device.
* @param[in] *readFile parent file pointer.
* @param[in] devType option of different device.
* @param[in] headerInfo a struct save child file's information.
* @param[in] childFileIdx index of now uplode child file.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int inline WriteChildFileP51(HANDLE comPort, FILE *fp, FileInfoP51 fileInfo, int fIdx, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned int checkSum = 0, writeAddr = 0, fileSize = 0, status = API_SUCCESS ;

	checkSum = CalculateFileCheckSumP51(fp, fileInfo.childInfo[fIdx].fileSize, fileInfo.fileOffset[fIdx] + sizeof(fileInfo.childInfo[fIdx])) ;
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
			fileSize = fileInfo.fileSize[fIdx] - sizeof(fileInfo.childInfo[fIdx]) ;
			fseek(fp, fileInfo.fileOffset[fIdx] + sizeof(fileInfo.childInfo[fIdx]), SEEK_SET) ;
			break ;
		default :
			fileSize = fileInfo.fileSize[fIdx] ;
			fseek(fp, fileInfo.fileOffset[fIdx], SEEK_SET) ;
			break ;
	} // switch

	return (status = SendBootRomPkt(comPort, fp, BOOT_ROM_WRITE_CMD, writeAddr, fileSize, processBar, callBackUpdateProcessFun)) ;
} // WriteChildFileP51

/**
*/
int ReadDataP51(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackFun)
{
	FILE *rwFp = fopen(outputFileName, "wb+") ;
	int status = API_SUCCESS ;

	if (NULL == rwFp)
		return FILE_NOT_EXIST ;

	if (API_SUCCESS != (status = JumpToAddrP51(comPort, P51_RAM_ADDR)))
		return status ;

	Sleep(1000) ;
	return (status = SendBootRomPkt(*comPort, rwFp, BOOT_ROM_READ_CMD, handleAddr, handelLen, processBar, callBackFun)) ;
} // ReadDataP51

/**
*/
int ClearFlash4KP51(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackFun)
{
	return API_SUCCESS ;
} // ClearFlash4KG06Series

/**
*/
int WriteSnP51(HANDLE comPort, WriteSnSettingInfo snInfo)
{
	return API_SUCCESS ;
} // ExtraTestP51

/**
*/
void ReadLogInfoP51(FILE *fp, LogInfoP51 *info)
{
	void *buf[LOG_BUF_NUM_P51] = {&info->utc, &info->valid, &info->reserve,
								  &info->latitude, &info->longtitude, &info->speed,
								  &info->height, &info->heading, &info->ecomposs,
								  &info->gsensorX, &info->gsensorY, &info->gsensorZ,
								  &info->heartRate, &info->flag, &info->padding} ;
	int bufSize[LOG_BUF_NUM_P51] = {sizeof(info->utc), sizeof(info->valid), sizeof(info->reserve),
									sizeof(info->latitude), sizeof(info->longtitude), sizeof(info->speed),
									sizeof(info->height), sizeof(info->heading), sizeof(info->ecomposs),
									sizeof(info->gsensorX), sizeof(info->gsensorY), sizeof(info->gsensorZ),
									sizeof(info->heartRate), sizeof(info->flag), sizeof(info->padding)} ;

	for (int i = 0 ; i < LOG_BUF_NUM_P51 ; ++i)
		fread(buf[i], 1, bufSize[i], fp) ;
} // ReadLogInfoP51

/**
*/
unsigned int CalculateFileCheckSumP51(FILE *readFile, unsigned int fileLen, unsigned int nowAddr)
{
	unsigned int checkSum = 0 ;

	for (int i = 0; i < fileLen ; i++)
		checkSum += fgetc(readFile) ;

	fseek(readFile, nowAddr, SEEK_SET) ;
	return checkSum ;
} // CalculateFileCheckSumP51

