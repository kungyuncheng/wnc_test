//---------------------------------------------------------------------------

#pragma hdrstop

#include "MASAPackage.h"
#include "Message.h"
#include "GetToken.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

const bool mNeedEncode[NUM_OF_DEVICE_TYPE] = {true, true, true, true, true, true, false, false, false, false, false, false, true} ;
const unsigned char mPackageTitle[NUM_OF_DEVICE_TYPE][2] = {{'G', '5'}, {'G', 'H'}, {'7', 'N'}, {'7', 'P'}, {'G', '8'},
														    {'8', 'P'}, {NULL}, {NULL}, {NULL}, {NULL}, {0x03, 0x66}, {0x03, 0xE8}, {'1', 'M'}} ;

/**
*/
int CreateMASAPkt(unsigned char *buf, int devType, CreateMasaPktInfo createInfo)
{
	int bufSize = 0 ;

	switch (createInfo.cmd.name)
	{
		case CMD_DL_END :
		case CMD_DL_DEV_INFO :
		case CMD_DL_JUMP_ADDR :
			bufSize = CreateMASAPktBase(buf, devType, createInfo) ;
			break ;
		case CMD_DL_ERASE_MCU :
		case CMD_DL_ERASE :
			bufSize = CreateMASAPktErase(buf, devType, createInfo) ;
			break ;
		case CMD_DL_WRITE :
		case CMD_DL_WRITE_SN :
			bufSize = CreateMASAPkt_CMD_DL_WRITE(buf, devType, createInfo) ;
			break ;	
		case CMD_DL_READ :
			bufSize = CreateMASAPkt_CMD_DL_READ(buf, devType, createInfo) ;
			break ;
		case CMD_DL_ERASE_4K :
			bufSize = CreateMASAPkt_CMD_DL_ERASE_4K(buf, devType, createInfo) ;
			break ;
		case CMD_DL_START :
			bufSize = CreateMASAPkt_CMD_DL_START(buf, devType, createInfo) ;
			break ;
		case CMD_DL_SW :
			bufSize = CreateMASAPkt_CMD_DL_SW(buf, devType, createInfo) ;
			break ;
		case CMD_DL_SEL_ERASE :
			bufSize = CreateMASAPkt_CMD_DL_SEL_ERASE(buf, devType, createInfo) ;
			break ;
    } // switch
	
	return bufSize ;
} // CreateMASAPkt

/**
	不帶data資料, 只有命令部分的package
*/
int CreateMASAPktBase(unsigned char *buf, int devType, CreateMasaPktInfo createInfo)
{
	int pktSize = 0, dataSize = 0 ;

	pktSize = CreateMASAPktHeadPart(buf, devType, createInfo, dataSize) ;
	return pktSize += CreateMASAPktCheckSumPart(buf + pktSize, dataSize) ;
} // CreateMASAPktBase

/**
*/
int CreateMASAPktErase(unsigned char *buf, int devType, CreateMasaPktInfo createInfo)
{
 	int pktSize = 0, offset = MASA_PACKAGE_TITLE_SIZE + MASA_PACKAGE_CMD_SIZE + MASA_PACKAGE_DATA_SIZE ;
	int dataSize = createInfo.cmd.item.erase.numOfBlock ;

	pktSize = CreateMASAPktHeadPart(buf, devType, createInfo, dataSize) ;
	pktSize += CreateMASAPktEraseData(buf + pktSize, createInfo) ;
	if (mNeedEncode[devType])
		EncodeMASAPkt(buf + offset, (pktSize - offset)) ;

	return pktSize += CreateMASAPktCheckSumPart(buf + offset, (pktSize - offset)) ;
} // CreateMASAPktErase

/**
*/
int CreateMASAPkt_CMD_DL_WRITE(unsigned char *buf, int devType, CreateMasaPktInfo createInfo)
{
	int pktSize = 0, offset = MASA_PACKAGE_TITLE_SIZE + MASA_PACKAGE_CMD_SIZE + MASA_PACKAGE_DATA_SIZE ;
	int dataSize = sizeof(createInfo.handleAddress) + sizeof(createInfo.handleLength) + createInfo.handleLength ;

	pktSize = CreateMASAPktHeadPart(buf, devType, createInfo, dataSize) ;
	pktSize += safe_memcpy(buf + pktSize, &createInfo.handleAddress, sizeof(createInfo.handleAddress)) ;
	pktSize += safe_memcpy(buf + pktSize, &createInfo.handleLength, sizeof(createInfo.handleLength)) ;
	pktSize += safe_memcpy(buf + pktSize, createInfo.cmd.item.write.data, dataSize) ;
	if (mNeedEncode[devType])
		EncodeMASAPkt(buf + offset, (pktSize - offset)) ;
	
	return pktSize += CreateMASAPktCheckSumPart(buf + offset, (pktSize - offset)) ;
} // CreateMASAPkt_CMD_DL_WRITE

/**
*/
int CreateMASAPkt_CMD_DL_READ(unsigned char *buf, int devType, CreateMasaPktInfo createInfo)
{
	int pktSize = 0, offset = MASA_PACKAGE_TITLE_SIZE + MASA_PACKAGE_CMD_SIZE + MASA_PACKAGE_DATA_SIZE ;
    int dataSize = sizeof(createInfo.handleAddress) + sizeof(createInfo.handleLength) ;

	pktSize = CreateMASAPktHeadPart(buf, devType, createInfo, dataSize) ;
	pktSize += safe_memcpy(buf + pktSize, &createInfo.handleAddress, sizeof(createInfo.handleAddress)) ;
	pktSize += safe_memcpy(buf + pktSize, &createInfo.handleLength, sizeof(createInfo.handleLength)) ;
	if (mNeedEncode[devType])
		EncodeMASAPkt(buf + offset, (pktSize - offset)) ;
	
	return pktSize += CreateMASAPktCheckSumPart(buf + offset, (pktSize - offset)) ;    
} // CreateMASAPkt_CMD_DL_READ

/**
*/
int CreateMASAPkt_CMD_DL_ERASE_4K(unsigned char *buf, int devType, CreateMasaPktInfo createInfo)
{
	int pktSize = 0, offset = MASA_PACKAGE_TITLE_SIZE + MASA_PACKAGE_CMD_SIZE + MASA_PACKAGE_DATA_SIZE ;
    int dataSize = sizeof(createInfo.handleAddress) ;

	pktSize = CreateMASAPktHeadPart(buf, devType, createInfo, dataSize) ;
	pktSize += safe_memcpy(buf + pktSize, &createInfo.handleAddress, sizeof(createInfo.handleAddress)) ;
	if (mNeedEncode[devType])
		EncodeMASAPkt(buf + offset, (pktSize - offset)) ;
	
	return pktSize += CreateMASAPktCheckSumPart(buf + offset, (pktSize - offset)) ;  
} // CreateMASAPkt_CMD_DL_ERASE_4K

/**
*/
int CreateMASAPkt_CMD_DL_START(unsigned char *buf, int devType, CreateMasaPktInfo createInfo)
{
	time_t now = (DEVICE_TYPE_BT1000 == devType ? 0 : time(NULL)) ;
	int pktSize = 0, offset = MASA_PACKAGE_TITLE_SIZE + MASA_PACKAGE_CMD_SIZE + MASA_PACKAGE_DATA_SIZE ;
	int dataSize = sizeof(now) ;

	pktSize = CreateMASAPktHeadPart(buf, devType, createInfo, dataSize) ;
	pktSize += safe_memcpy(buf + pktSize, &now, sizeof(now)) ;
	if (mNeedEncode[devType])
		EncodeMASAPkt(buf + offset, (pktSize - offset)) ;
	
	return pktSize += CreateMASAPktCheckSumPart(buf + offset, (pktSize - offset)) ;  
} // CreateMASAPkt_CMD_DL_START 

/**
*/
int CreateMASAPkt_CMD_DL_SW(unsigned char *buf, int devType, CreateMasaPktInfo createInfo)
{
    unsigned char dlSwData = 0x02 ;
	int pktSize = 0, offset = MASA_PACKAGE_TITLE_SIZE + MASA_PACKAGE_CMD_SIZE + MASA_PACKAGE_DATA_SIZE ;
    int dataSize = sizeof(dlSwData) ;

	pktSize = CreateMASAPktHeadPart(buf, devType, createInfo, dataSize) ;
	pktSize += safe_memcpy(buf + pktSize, &dlSwData, sizeof(dlSwData)) ;
	if (mNeedEncode[devType])
		EncodeMASAPkt(buf + offset, (pktSize - offset)) ;
	
	return pktSize += CreateMASAPktCheckSumPart(buf + offset, (pktSize - offset)) ;
} // CreateMASAPkt_CMD_DL_SW

/**
*/
int CreateMASAPkt_CMD_DL_SEL_ERASE(unsigned char *buf, int devType, CreateMasaPktInfo createInfo)
{
    int blockSIze = createInfo.cmd.item.erase.blockSize ;
	int blockNum = (createInfo.handleLength / blockSIze ) + (0 == createInfo.handleLength % blockSIze  ? 0 : 1) ;
	int dataSize = sizeof(createInfo.handleAddress) + sizeof(blockNum) ;
	int pktSize = 0, offset = MASA_PACKAGE_TITLE_SIZE + MASA_PACKAGE_CMD_SIZE + MASA_PACKAGE_DATA_SIZE ;

	pktSize = CreateMASAPktHeadPart(buf, devType, createInfo, dataSize) ;
	pktSize += safe_memcpy(buf + pktSize, &createInfo.handleAddress, sizeof(createInfo.handleAddress)) ;
	pktSize += safe_memcpy(buf + pktSize, &blockNum, sizeof(blockNum)) ;
	if (mNeedEncode[devType])
		EncodeMASAPkt(buf + offset, (pktSize - offset)) ;
	
	return pktSize += CreateMASAPktCheckSumPart(buf + offset, (pktSize - offset)) ;
} // CreateMASAPkt_CMD_DL_SEL_ERASE

/**
*/
int CreateMASAPktHeadPart(unsigned char *buf, int devType, CreateMasaPktInfo createInfo, int dataSize)
{
	int pktSize = 0 ;

	memcpy(buf, mPackageTitle[devType], (pktSize = sizeof(mPackageTitle[devType]))) ;
	pktSize += safe_memcpy(buf + pktSize, &createInfo.cmd.name, sizeof(createInfo.cmd.name)) ;
	return pktSize += safe_memcpy(buf + pktSize, &dataSize, sizeof(dataSize)) ;
} // CreateMASAPktHead

/**
*/
int CreateMASAPktCheckSumPart(unsigned char *buf, int bufSize)
{
	int checkSum = 0 ;

	checkSum = CalculateMASAPktChecksum(buf, bufSize) ;
	return safe_memcpy(buf + bufSize, &checkSum, sizeof(checkSum)) ;
} // CreateMASAPktCheckSumPart

/**
*/
int CreateMASAPktEraseData(unsigned char *buf, CreateMasaPktInfo createInfo)
{
	unsigned char eraseData[MASA_BUFFER_MAX_SIZE] = {"\0"} ;
	CreateMasaPktEraseInfo *eraseInfo = &createInfo.cmd.item.erase ;

	for (int i = 0, compareAddr = 0 ; i < (eraseInfo->numOfBlock * BITS_OF_BYTE) ; i++)
	{
		compareAddr = eraseInfo->fwStartAddress + i*eraseInfo->blockSize ;
		if ((compareAddr >= createInfo.handleAddress) && (compareAddr < (createInfo.handleAddress + createInfo.handleLength)))
			eraseData[i / BITS_OF_BYTE] |= (1 << i % BITS_OF_BYTE) ;
	} // for

	return safe_memcpy(buf, eraseData, eraseInfo->numOfBlock) ;
} // CreateMASAPktEraseData

/**
*/
int CalculateMASAPktChecksum(unsigned char *buf, int bufSize)
{
	int checkSum = 0;

	for (int i = 0 ; i < bufSize ; ++i) 
		checkSum += buf[i] ;

	return checkSum ;
} // CalculateMASAPktChecksum

/**
*/
void EncodeMASAPkt(unsigned char *buf, int bufSize)
{
	for (int i = 0 ; i < bufSize ; ++i)
		buf[i] = mMASAEncodeTable[buf[i]] ;
} // EncodeMASAPkt 

/**
*/
void inline DecodeMASAPkt(unsigned char *buf, int bufSize)
{
	for (int i = 0 ; i < bufSize ; ++i)  	// decode
		buf[i] = mMASADecodeTable[buf[i]] ;
} // DecodeMASAPkt

/**
*@brief call back function
*/
int JudgeMASAPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char expectBuf[MASA_BUFFER_MAX_SIZE] = {"\0"} ;
	int status = API_FAIL, expectBufSize = 0 ;

	expectBufSize = CreateExpectMASAPkt(comInfo->devType, comInfo->writeBuf, comInfo->writeBufSize, expectBuf) ;
	if (API_SUCCESS != (status = CheckMASAPkt(comInfo->devType, expectBuf, expectBufSize, comInfo->resBuf, comInfo->resBufSize)))
		return status ;

	return AnalysisMASAPkt(comInfo->resBuf, comInfo->resBufSize, comInfo->devType, (TSIAckStatus *)ackInfo, dwFp) ;
} // JudgeMASAPkt

/**
*/
int CreateExpectMASAPkt(int devType, unsigned char *oriBuf, int oriBufSize, unsigned char *expectBuf)
{													 
	int pktSize = 0 ;
	unsigned char cmd = 0 ;

	memcpy(expectBuf, oriBuf, (pktSize = 2)) ;
    switch (cmd = oriBuf[2])
	{
		case CMD_DL_DEV_INFO :
			pktSize += CreateExpectMASAPkt_CMD_DL_ACK_DEV_INFO(devType, expectBuf + pktSize) ;
			break ;
		case CMD_DL_READ :
			pktSize += CreateExpectMASAPkt_CMD_DL_ACK_READ(oriBuf, expectBuf + pktSize) ;
			break ;
		case CMD_DL_END :
		case CMD_DL_WRITE_SN :
		case CMD_DL_ERASE_4K :
		case CMD_DL_ERASE_MCU :
		case CMD_DL_ERASE :
		case CMD_DL_WRITE :
		case CMD_DL_START :
		case CMD_DL_JUMP_ADDR :
		case CMD_DL_SW :
		case CMD_DL_SEL_ERASE :
			pktSize += CreateExpectMASAPkt_CMD_DL_ACK(expectBuf + pktSize) ;
			break ;
	} // switch

	return pktSize ;
} // CreateExpectMASAPkt

/**
*/
int CreateExpectMASAPkt_CMD_DL_ACK(unsigned char *buf)
{
	unsigned char cmd = CMD_DL_ACK ;
	short pktDataSize = 1, pktInfoSize = 1 ;
	int pktSize = 0 ;

	pktSize = safe_memcpy(buf, &cmd, sizeof(cmd)) ;
	pktSize += safe_memcpy(buf + pktSize, &pktInfoSize, sizeof(pktInfoSize)) ;
	return pktSize += (pktDataSize + sizeof(int)) ;     // checksum size
} // CreateExpectMASAPkt_CMD_DL_ACK

/**
*/
int CreateExpectMASAPkt_CMD_DL_ACK_READ(unsigned char *oriBuf, unsigned char *buf)
{
	unsigned char cmd = CMD_DL_ACK_READ ;
	short pktDataSize = 0, pktInfoSize = 0 ;
	int pktSize = 0, handleAddr = 0, handelLen = 0 ;

	memcpy(&handleAddr, oriBuf + 3 + sizeof(pktInfoSize), sizeof(handleAddr)) ;
	memcpy(&handelLen, oriBuf + 3 + sizeof(pktInfoSize) + sizeof(handleAddr), sizeof(handelLen)) ;

	DecodeMASAPkt((unsigned char *)&handleAddr, sizeof(handleAddr)) ;
	DecodeMASAPkt((unsigned char *)&handelLen, sizeof(handelLen)) ;

	pktInfoSize = pktDataSize = sizeof(handleAddr) + sizeof(handelLen) + handelLen ;
	pktSize = safe_memcpy(buf, &cmd, sizeof(cmd)) ;
	pktSize += safe_memcpy(buf + pktSize, &pktInfoSize, sizeof(pktInfoSize)) ;
	return pktSize += (pktDataSize + sizeof(int)) ;     // sizeof(int) : checksum size
} // CreateExpectMASAPkt_CMD_DL_ACK_READ 

/**
*/
int CreateExpectMASAPkt_CMD_DL_ACK_DEV_INFO(int devType, unsigned char *buf)
{
	unsigned char cmd = CMD_DL_ACK_DEV_INFO ;
	short pktDataSize = 31, pktInfoSize = 31 ;
	int pktSize = 0 ;

	switch (devType)
	{
		case DEVICE_TYPE_G08 :
		case DEVICE_TYPE_G08P :
			pktDataSize = 31 + 8 ;   // 目前 G08,G08P,G11M多 8 byte 無意義 0x00 但在 package 放 package size的地方不需加這8個
			break ;
		case DEVICE_TYPE_G11M :
			pktDataSize = 51 + 8 ;
			pktInfoSize = 51 ;
			break ;
	}

	pktSize = safe_memcpy(buf, &cmd, sizeof(cmd)) ;
	pktSize += safe_memcpy(buf + pktSize, &pktInfoSize, sizeof(pktInfoSize)) ;
	return pktSize += (pktDataSize + sizeof(int)) ;      // checksum size
} // CreateExpectMASAPkt_CMD_DL_ACK_DEV_INFO

/**
*/
int CheckMASAPkt(int devType, unsigned char *expectBuf, int expectBufSize, unsigned char *resBuf, int resBufSize)
{   // 'G' '5' : title 2 bytes  '0xf0' : cmd 1 byte  '0x01 0x00' : package data size 2 bytes
	int headerSize = MASA_PACKAGE_TITLE_SIZE + MASA_PACKAGE_CMD_SIZE + MASA_PACKAGE_DATA_SIZE ;
	int pktCheckSum = 0, calCheckSum = 0, checkSumOffset = 4 ;

	if (expectBufSize != resBufSize)
		return API_FAIL ;

	if (0 != strncmp(resBuf, expectBuf, headerSize))     // 檢查title
		return API_FAIL ;

	if (((DEVICE_TYPE_G08 == devType) || (DEVICE_TYPE_G08P == devType) || (DEVICE_TYPE_G11M == devType)) &&
		CMD_DL_ACK_DEV_INFO == resBuf[2])
	{
		checkSumOffset += 8 ;  // G08 & G08P & G011m 目前版本多8個無意義byte
    } // if

	memcpy(&pktCheckSum, resBuf + (resBufSize - checkSumOffset), sizeof(pktCheckSum)) ;
	calCheckSum = CalculateMASAPktChecksum(resBuf + headerSize, resBufSize - checkSumOffset) ;
	if (calCheckSum != pktCheckSum)          // 檢查checksum
		return PACKAGE_CHECKSUM_ERROR ;

	return API_SUCCESS ;
} // CheckMASAPkt

/**
*/
int AnalysisMASAPkt(unsigned char *buf, int bufSize, int devType, TSIAckStatus *ackInfo, FILE *dwFp)
{
	unsigned char cmd = buf[2] ;
	int status = API_FAIL ;

	switch (cmd)
	{
		case CMD_DL_ACK_DEV_INFO :
			status = AnalysisMASAPkt_CMD_DL_ACK_DEV_INFO(buf, bufSize, devType, ackInfo) ;
			break ;
		case CMD_DL_ACK_READ :
			status = AnalysisMASAPkt_CMD_DL_ACK_READ(buf, bufSize, devType, dwFp) ;
			break ;
		case CMD_DL_ACK :
			status = AnalysisMASAPkt_CMD_DL_ACK(buf, bufSize, devType) ;
			break ;
	} // switch

	return status ;
} // AnalysisMASAPkt

/**
*/
int AnalysisMASAPkt_CMD_DL_ACK_DEV_INFO(unsigned char *buf, int bufSize, int devType, TSIAckStatus *ackInfo)
{
	const int headerSize = 5, seriesNumSize = 9, apVerSize = 7, bootVerSize = 7, scoreNumSize = 1, courseVerSize = 4 ;
	const unsigned char *outputDevType[NUM_OF_DEVICE_TYPE] = {"G05", "G05H", "G07N", "G07P", "G08", "G08P", "", "", "", "", "", "", "G011M"} ;
	int infoDataSize = 0, outputDevTypeSize = 0 ;
	int offset = headerSize + (DEVICE_TYPE_G11M == devType ? strlen("G011M") : 0) ;
	int apVerOffset = (DEVICE_TYPE_G11M == devType ? 18 : 3) ;
											  // check sum size : sizeof(int)
	DecodeMASAPkt(buf + headerSize, bufSize - sizeof(int)) ;
	outputDevTypeSize = strchr((char *)outputDevType[devType], '\0') - outputDevType[devType] ;

	memcpy(ackInfo->devInfo.base.devSeries, outputDevType[devType], (infoDataSize = outputDevTypeSize)) ;    	  // 'G','5',0xF2,0x1F,0x00
	for (int i = 0 ; i < seriesNumSize ; ++i)
		sprintf((ackInfo->devInfo.base.devSeries + infoDataSize++), "%c", buf[offset++]) ;        		// Byte 5~16 is serial number

	offset += apVerOffset ;
	memcpy(ackInfo->devInfo.base.fwVer, buf + offset, apVerSize) ;      		                		// Byte 17~23 is AP version
	memcpy(ackInfo->devInfo.base.btVer, buf + (offset += apVerSize), bootVerSize) ;             		// Byte 24~30 is Boot version
	ackInfo->devInfo.base.recordCount = (int)buf[offset += bootVerSize] ;								// Byte 31 is total score number
	memcpy(&ackInfo->devInfo.extra.g05Series.cVer, buf + (offset += scoreNumSize), courseVerSize) ;  	// Byte 32~35 is Course version
																										// Byte 36~39 is checksum
	return API_SUCCESS ;
} // AnalysisMASAPkt_CMD_DL_ACK_DEV_INFO

/**
*/
int AnalysisMASAPkt_CMD_DL_ACK(unsigned char *buf, int bufSize, int devType)
{   // 0x01 : success, 0x00 : fail
	return (0x01 == buf[5] ? API_SUCCESS : API_FAIL) ;
} // AnalysisMASAPkt_CMD_DL_ACK

/**
*/
int AnalysisMASAPkt_CMD_DL_ACK_READ(unsigned char *buf, int bufSize, int devType, FILE *dwFp)
{                                  //  1   1   1      2
	const int headerSize = 5 ;     // 'G' '5' cmd  pktSize
	short pktDataSize = 0 ;

	memcpy(&pktDataSize, buf + 3, sizeof(pktDataSize)) ;
	DecodeMASAPkt(buf + headerSize, bufSize - sizeof(int)) ;

	if (NULL != dwFp)       // start addr, data length;
		fwrite(buf + headerSize + 2 * sizeof(int), 1, pktDataSize - 2 * sizeof(int), dwFp) ;

	return API_SUCCESS ;
} // AnalysisMASAPkt_CMD_DL_ACK_READ

/**
*/
int SendMASAPkt(HANDLE comPort, FILE *rwFp, int devType, int pktSize, char cmd, int startAddr, int dataSize, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[MASA_BUFFER_MAX_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	unsigned int status = API_SUCCESS ;
	unsigned char data[MASA_BUFFER_MAX_SIZE] = {"\0"} ;

	TSIAckStatus tsiAckInfo ;
	WriteComInfo comInfo ;
	CreateMasaPktInfo pktInfo ;

	if (CMD_DL_WRITE == (pktInfo.cmd.name = cmd))
		pktInfo.cmd.item.write.data = &data[0] ;

	pktInfo.handleAddress = startAddr ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.devType = devType ;
	comInfo.pktType = NORMAL_PACKAGE ;
	for (int sendDataSize = 0, remindDataSize = dataSize ; remindDataSize > 0 ; pktInfo.handleAddress += pktInfo.handleLength, remindDataSize -= pktInfo.handleLength)
	{
		sendDataSize += (pktInfo.handleLength = (remindDataSize >= pktSize ? pktSize : remindDataSize)) ;
		if (CMD_DL_WRITE == cmd)
			fread(data, 1, pktInfo.handleLength, rwFp) ;

		comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, devType, pktInfo) ;
		if (API_SUCCESS != (status = WriteToCom(&comInfo, &tsiAckInfo, (CMD_DL_WRITE == cmd ? NULL : rwFp), JudgeMASAPkt)))
			return status ;

		if (NULL != callBackUpdateProcessFun)
			callBackUpdateProcessFun(processBar, sendDataSize, dataSize) ;
	} // while

	return API_SUCCESS ;
} // SendMASAPkt

/**
* @brief write a series command so that device can be commucation.
* @param[in] comPort comport which connect to device.
* @param[in] devType option of different device.
* @note after device change to boot loader must change baud rate.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int StartCommucateMASA(HANDLE comPort, int devType)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo comInfo ;
	TSIAckStatus tsiAckInfo ;
	CreateMasaPktInfo pktInfo ;

	pktInfo.cmd.name = CMD_DL_START ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.devType = devType ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, devType, pktInfo) ;
	return (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // StartCommucateMASA

/**
* @brief End communcation to device.
* @param[in] comPort comport which connect to device.
* @param[in] devType option of different device.
* @return if get error return error message(Message.h), or return API_SUCCESS.
*/
int EndCommucateMASA(HANDLE comPort, int devType)
{
	unsigned char writeBuf[WRITE_DATA_BUFFER_SIZE] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	int status = API_SUCCESS ;
	WriteComInfo comInfo ;
	TSIAckStatus tsiAckInfo ;
	CreateMasaPktInfo pktInfo ;

	pktInfo.cmd.name = CMD_DL_END ;

	comInfo.comPort = comPort ;
	comInfo.writeBuf = &writeBuf[0] ;
	comInfo.resBuf = &resBuf[0] ;
	comInfo.timeOut = TIME_OUT ;
	comInfo.devType = devType ;
	comInfo.pktType = NORMAL_PACKAGE ;
	comInfo.writeBufSize = CreateMASAPkt(comInfo.writeBuf, devType, pktInfo) ;
	return (status = WriteToCom(&comInfo, &tsiAckInfo, NULL, JudgeMASAPkt)) ;
} // EndCommucateMASA









