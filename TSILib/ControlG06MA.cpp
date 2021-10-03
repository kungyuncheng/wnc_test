//---------------------------------------------------------------------------

#pragma hdrstop

#include "ControlG06MA.h"
#include "Message.h"
#include "GetToken.h"
#include "ConnectComport.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int AskDevInfoG06MA(HANDLE comPort, TSIAckStatus *tsiAckInfo, MTKAckStatus *mtkAckInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT * 3 ;
	info.devType = DEVICE_TYPE_G06MA ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%04d,%d,%08X*", TSI_OPTION_ID_1600, TSI_CMD_READ, G06MA_SERIES_NUM_ADDR) ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d*", TSI_OPTION_ID_997) ;
	info.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&info, tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d,%d*", PMTK_LOG, LOG_QUERY, LOG_RET_REC_ADDR) ;
	if (API_SUCCESS != (status = WriteToCom(&info, mtkAckInfo, NULL, JudgeTSIPkt)))  // ���R�O�^�_��MTK182���|, �ҥH�� mtkAckInfo ��
		return status ;

	strcpy(tsiAckInfo->devInfo.base.devSeries, tsiAckInfo->readData) ;
	return status ;
} // AskDevInfoG06MA

/**
* @brief write a series command so that device change boot state to bool loader.
* @param[in] *comPort comport which connect to device.
* @param[in] devType option of different device.
* @note after device change to boot loader must change baud rate.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int JumpToBootG06MA(HANDLE *comPort)
{
	unsigned char bootRomStartCmd[BOOT_ROM_START_CMD_NUM] = {BOOT_ROM_START_CMD1, BOOT_ROM_START_CMD2, BOOT_ROM_START_CMD3, BOOT_ROM_START_CMD4} ;
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int checkSum = 0, status = API_SUCCESS ;
	ConnectComport *connectCom = new ConnectComport() ;
	WriteComInfo info ;
	TSIAckStatus tsiAckInfo ;

	info.comPort = *comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT ;
																	  // $PTSI998,0,00000000*X/r/n
	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%03d,%d,%08X*", TSI_OPTION_ID_998, TSI_STATUS_BOOT, time(NULL)) ;
	info.devType = DEVICE_TYPE_G06MA ;
	info.pktType = NO_ACK ;
	if (API_SUCCESS != (status = WriteToCom(&info, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;                                               // 2 ����  wait 1.6 second

	connectCom->SetBaudRate(comPort, DEVICE_BAUDRATE_G06MA_BOOT, false) ;  // 3 ����baud change �� 460800
	delete connectCom ;

	for (int i = 0 ; i < BOOT_ROM_START_CMD_NUM ; ++i)
	{   // 4 �e�XA0 0A 50 05 device�|�^�� 5F F5 AF FA
		info.writeBufSize = safe_memcpy(info.writeBuf, &bootRomStartCmd[i], sizeof(unsigned char)) ;
		info.pktType = NORMAL_PACKAGE ;
		if (API_SUCCESS != (status = WriteToCom(&info, NULL, NULL, JudgeBootRomPkt)))
			return status ;
	} // for

	return status ;
} // JumpToBootG06MA

/**
* @brief jump to target flash address.
* @param[in] comPort comport which connect to device.
* @param[in] addr target address.
* @param[in] devType option of different device.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int JumpToAddrG06MA(HANDLE *comPort, int addr)
{   // 6 A8 00 00 00 20 01 00 00 00
	// �o�O�@�����Ljump to 0x20000000��packet
	// ��s�ɮ׫e���� 0x20000000 (�j���ݭn1���w��)  ��s����� 0x08001400
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int cmdLen = 0x01, status = API_SUCCESS ;
	WriteComInfo info ;

	info.comPort = *comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT ;

	info.writeBufSize = CreateBootRomPkt(info.writeBuf, NULL, BOOT_ROM_JUMP_CMD, addr, cmdLen) ;
	info.pktType = NORMAL_PACKAGE ;
	return (status = WriteToCom(&info, NULL, NULL, JudgeBootRomPkt)) ;
} // JumpToAddrG06MA

/**
*/
int UpdateG06MA(int updateIdx, HANDLE comPort, FILE *fp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	FileInfoG06MA fileInfo ;
	int status = API_SUCCESS ;

	fread(&fileInfo, 1, sizeof(FileInfoG06MA), fp) ;
	for (int i = 0 ; i < fileInfo.totalFileNum ; ++i)
	{
		fseek(fp, fileInfo.fileOffset[i], SEEK_SET) ;
		fread(&fileInfo.childInfo[i], 1 , sizeof(ChileFileInfoG06MA), fp) ;
		if (API_SUCCESS != (status = WriteChildFileG06MA(comPort, fp, fileInfo, i, processBar, callBackUpdateProcessFun)))
			return status ;
	} // for

	return status ;
	/* 7. ���R�n�N����files
		header 1024 bytes

		struct
		{
			int numberOfFilesToBeProcessed;
			fileSize[10];
			int offset[10];
		}

		�o��structure�@�w�S��1024 bytes�A�ѤU���Ŷ���0x00

		seek(offset[i])�i�H���file#i����ơC�o�Ӹ�Ƹ̭��̫e��16 bytes�Oheader�A�榡�p�U
		int writeAddr ;
		int fileSize ;
		int checkSum;
		char reserve[4] (��TSI��);

		�᭱���۪��O�n�N���i�h����ơA�o���������׬O fileSize
		�|�Ҩӻ��A�p�G�����ƭȬO

		writeAddr = 0x00000000, fileSize = 0x00000200

		����n�e�X�h��packet�N�O

		A1 00 00 00 00 00 80 00 00 [128 bytes payload]
		A1 00 10 00 00 00 80 00 00 [128 bytes payload]

		****************���@�Өҥ~*******************

		�p�G writeAddr = 0x080013f0
		�h�ҭn�N����ƴN�n�]�t�o16 bytes��header�A�ӥB���׬OfileSize + 16
		�ҥH�L�n�e�X�h��packet�ݰ_�ӱN�|�O

		A1 F0 13 00 08 00 80 00 00 [F0 13 00 08 ....]
		A1 00 14 00 08 00 80 00 00 [....]
		A1 10 14 00 08 00 80 00 00 [....]
		*/
} // UpdateG06MA

/**
* @brief a uplode file maybe include another child file to update.
* @param[in] comPort comport which connect to device.
* @param[in] *readFile parent file pointer.
* @param[in] devType option of different device.
* @param[in] headerInfo a struct save child file's information.
* @param[in] childFileIdx index of now uplode child file.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int inline WriteChildFileG06MA(HANDLE comPort, FILE *fp, FileInfoG06MA fileInfo, int fIdx, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned int checkSum = 0, writeAddr = 0, fileSize = 0, status = API_SUCCESS ;

	checkSum = CalculateFileCheckSumG06MA(fp, fileInfo.childInfo[fIdx].fileSize, fileInfo.fileOffset[fIdx] + sizeof(fileInfo.childInfo[fIdx])) ;
	if (checkSum != fileInfo.childInfo[fIdx].checkSum)
		return FILE_CHECKSUM_ERROR ;

	writeAddr = fileInfo.childInfo[fIdx].writeAddr ;
	fileSize = fileInfo.childInfo[fIdx].fileSize ;
	switch (writeAddr)
	{
		case 0x080013f0 :  // �S���} �g�� MCU
        	fseek(fp, fileInfo.fileOffset[fIdx], SEEK_SET) ;
			fileSize += sizeof(fileInfo.childInfo[fIdx]) ;
			break ;
    } // switch

	return (status = SendBootRomPkt(comPort, fp, BOOT_ROM_WRITE_CMD, writeAddr, fileSize, processBar, callBackUpdateProcessFun)) ;
} // WriteChildFileG06MA

/**
*/
int ClearFlash4KG06MA(HANDLE comPort, int handleAddr, int handelLen, void *processBar, UpdateProcessFun callBackFun)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS, startTime = 0 ;
	WriteComInfo info ;
	TSIAckStatus tsiAckInfo ;

	if (API_SUCCESS != (status = JumpToAddrG06MA(&comPort, G06MA_AP_ADDR))) // ���}��
		return status ;

	Sleep(1000) ;

	info.comPort = comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.devType = DEVICE_TYPE_G06MA ;
	info.writeBufSize = CreateTSIPkt(info.writeBuf, "$PTSI%04d,%d,%08X*", TSI_OPTION_ID_1600, TSI_ERASE_FLASH, handleAddr) ;
	// clear 0x400000~0x800000 �C���^�� clear 0x10000 ~ 0x400000
	info.pktType = G06MA_ERASE_4K ;
	info.g06maErase4KackCount = 0;
	info.timeOut = TIME_OUT * 16 ;   // delete 1k �� 0.3��, 64k��30��
	return (status = WriteToCom(&info, &tsiAckInfo, NULL, JudgeTSIPkt)) ;
} // ClearFlash4KG06MA

/**
*/
int ReadDataG06MA(HANDLE *comPort, int handleAddr, int handelLen, unsigned char *outputFileName, void *processBar, UpdateProcessFun callBackFun)
{
	FILE *rwFp = fopen(outputFileName, "wb+") ;
	int status = API_SUCCESS ;

	if (NULL == rwFp)
		return FILE_NOT_EXIST ;

	if (API_SUCCESS != (status = JumpToAddrG06MA(comPort, G06MA_SERIES_NUM_ADDR)))
		return status ;

	Sleep(1000) ;
	return (status = SendBootRomPkt(*comPort, rwFp, BOOT_ROM_READ_CMD, handleAddr, handelLen, processBar, callBackFun)) ;
} // ReadDataG06MA

/**
*/
int WriteSnG06MA(HANDLE comPort, WriteSnSettingInfo snInfo)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	unsigned char sn[BASE_DATA_SIZE] = {"\0"} ;
	int status = API_SUCCESS, dataSize = 0 ;
	TSIAckStatus tsiAckInfo ;
	MTKAckStatus mtkAckInfo ;
	WriteComInfo comInfo ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = 500 ;//TIME_OUT ;
														   // �M���Ǹ���}���
	comInfo.writeBufSize = CreateTSIPkt(comInfo.writeBuf, "$PTSI%04d,%d,%08X*", TSI_OPTION_ID_1600, TSI_ERASE_FLASH, G06MA_SERIES_NUM_ADDR) ;
	comInfo.devType = DEVICE_TYPE_G06MA ;
	comInfo.pktType = NO_ACK ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	comInfo.writeBufSize = CreateTSIPkt(comInfo.writeBuf, "$PTSI%04d,%d,%08X,%s%s%s%s%05d%c%c%c*",
														  TSI_OPTION_ID_1600, TSI_CMD_WRITE, G06MA_SERIES_NUM_ADDR,
														  snInfo.dev.g06ma.sn.mode1, snInfo.dev.g06ma.sn.mode2, snInfo.dev.g06ma.sn.mode3,
														  snInfo.dev.g06ma.sn.mode4, snInfo.writeSn, 0x00, 0x00, 0x00) ;
	comInfo.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;            // �g�J�Ǹ�

	dataSize = sprintf(sn, "%s%s%s%s%05d", snInfo.dev.g06ma.sn.mode1, snInfo.dev.g06ma.sn.mode2, snInfo.dev.g06ma.sn.mode3,
										   snInfo.dev.g06ma.sn.mode4, snInfo.writeSn) ;

	comInfo.writeBufSize = CreateTSIPkt(comInfo.writeBuf, "$PTSI%04d,%d,%08X*", TSI_OPTION_ID_1600, TSI_CMD_READ, G06MA_SERIES_NUM_ADDR) ;
	comInfo.pktType = MTK_NORMAL_PACKAGE ;
	if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeTSIPkt)))
		return status ;

	if (0 != strncmp(tsiAckInfo.readData, sn, dataSize))
		return EXTRA_TEST_WRITE_SN_ERROR ;  // �ˬd�O�_�g�J

	return status ;
} // WriteSnG06MA

/**
*/
void ReadLogInfoG06MA(FILE *fp, LogInfoG06MA *info)
{
	void *buf[LOG_BUF_NUM_G06MA] = {&info->utc, &info->latitude, &info->longtitude,
									&info->gpsValid, &info->reserve, &info->courseId,
									&info->holeNum, &info->holeNumSeq, &info->strokesAtCurHole} ;
	int bufSize[LOG_BUF_NUM_G06MA] = {sizeof(info->utc), sizeof(info->latitude), sizeof(info->longtitude),
									  sizeof(info->gpsValid), sizeof(info->reserve), sizeof(info->courseId),
									  sizeof(info->holeNum), sizeof(info->holeNumSeq), sizeof(info->strokesAtCurHole)} ;

	for (int i = 0 ; i < LOG_BUF_NUM_G06MA ; ++i)
		fread(buf[i], 1, bufSize[i], fp) ;
} // ReadLogInfoG06MA

/**
*/
unsigned int CalculateFileCheckSumG06MA(FILE *readFile, unsigned int fileLen, unsigned int nowAddr)
{
	unsigned int checkSum = 0 ;

	for (int i = 0; i < fileLen ; i++)
		checkSum += fgetc(readFile) ;

	fseek(readFile, nowAddr, SEEK_SET) ;
	return checkSum ;
} // CalculateFileCheckSumG06MA

