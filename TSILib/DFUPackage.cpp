//---------------------------------------------------------------------------

#pragma hdrstop

#include "DFUPackage.h"
#include "Message.h"
#include "GetToken.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int WriteFileDFU(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_SUCCESS ;
	DFUAckInfo ackInfo ;

	ackInfo.ackNum = 0x02 ;
	ackInfo.seqNum = 0x01 ;
	if (API_SUCCESS != (status = SendStartDFUPkt(comPort, fwId, updateMode, fp, fileSize, progressBar, callBackUpdateProcessFun, &ackInfo)))
		return status ;
#if (DFU_SDK_VERSION == 710 || DFU_SDK_VERSION == 800)
	Sleep(DFU_BUFFER_TIME) ;
	if (API_SUCCESS != (status = SendInitDFUPkt(comPort, fwId, updateMode, fp, fileSize, progressBar, callBackUpdateProcessFun, &ackInfo)))
		return status ;
#endif
	if (API_SUCCESS != (status = SendDataDFUPkt(comPort, fwId, updateMode, fp, fileSize, progressBar, callBackUpdateProcessFun, &ackInfo)))
		return status ;

    if (API_SUCCESS != (status = SendStopDFUPkt(comPort, fwId, updateMode, fp, fileSize, progressBar, callBackUpdateProcessFun, &ackInfo)))
		return status ;

	return status ;
} // WriteFileDFU

/**
*/
int inline SendStartDFUPkt(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar, UpdateProcessFun callBackProcessFun, DFUAckInfo *ackInfo)
{
	DFUPktInfo pktInfo ;

	pktInfo.updateMode = updateMode ;
	pktInfo.fieldType = START_PACKET ;
	pktInfo.paramInfo.startPkt.apSize = (DFU_UPDATE_FW == updateMode ? fileSize : 0) ;
	pktInfo.size = sizeof(unsigned int) ;
#if (DFU_SDK_VERSION == 710 || DFU_SDK_VERSION == 800)
	pktInfo.paramInfo.startPkt.sdSize = 0 ;
	pktInfo.size += sizeof(unsigned int) ;
	pktInfo.paramInfo.startPkt.blSize = (DFU_UPDATE_FW == updateMode ? 0 : fileSize) ;
	pktInfo.size += sizeof(unsigned int) ;
	pktInfo.size += sizeof(unsigned int) ;   //  updateMode
#endif
	return SendDFUPkt(comPort, &pktInfo, ackInfo, progressBar, callBackProcessFun) ;
} // SendStartDFUPkt

/**
*/
int inline SendInitDFUPkt(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar, UpdateProcessFun callBackProcessFun, DFUAckInfo *ackInfo)
{
	const unsigned short seedCrc = 0xffff ;
	DFUPktInfo pktInfo ;

	pktInfo.fieldType = INIT_PACKET ;
	pktInfo.paramInfo.initPkt.devType = 0xffff ;
	pktInfo.size = sizeof(unsigned short) ;
	pktInfo.paramInfo.initPkt.devRevision = 0xffff ;
	pktInfo.size += sizeof(unsigned short) ;
	pktInfo.paramInfo.initPkt.appVersion = 0xffffffff ;
	pktInfo.size += sizeof(unsigned int) ;
	pktInfo.paramInfo.initPkt.vlidSfDevListLen = 0x0001 ;
	pktInfo.size += sizeof(unsigned short) ;
	pktInfo.paramInfo.initPkt.vlidSfDevList = fwId ;
	pktInfo.size += sizeof(unsigned short) ;
	pktInfo.paramInfo.initPkt.binCrc = ComputeCRC((void *)fp, fileSize, &seedCrc, true) ;
	pktInfo.size += sizeof(unsigned short) ;
	pktInfo.paramInfo.initPkt.filler = 0x0000 ;
	pktInfo.size += sizeof(unsigned short) ;

	return SendDFUPkt(comPort, &pktInfo, ackInfo, progressBar, callBackProcessFun) ;
} // SendInitDFUPkt

/**
*/
int inline SendDataDFUPkt(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar, UpdateProcessFun callBackProcessFun, DFUAckInfo *ackInfo)
{
	DFUPktInfo pktInfo ;

	pktInfo.fieldType = DATA_PACKET ;
	pktInfo.paramInfo.dataPkt.filePt = fp ;
	pktInfo.paramInfo.dataPkt.apSize = fileSize ;
	pktInfo.size = fileSize ;
	
	return SendDFUPkt(comPort, &pktInfo, ackInfo, progressBar, callBackProcessFun) ;
} // SendDataDFUPkt

/**
*/
int inline SendStopDFUPkt(HANDLE comPort, int fwId, int updateMode, FILE *fp, int fileSize, void *progressBar, UpdateProcessFun callBackProcessFun, DFUAckInfo *ackInfo)
{
	DFUPktInfo pktInfo ;

	pktInfo.fieldType = STOP_PACKET ;
	pktInfo.size = 0 ;

	return SendDFUPkt(comPort, &pktInfo, ackInfo, progressBar, callBackProcessFun) ;
} // SendStopDFUPkt

/**
*/
int inline SendDFUPkt(HANDLE comPort, DFUPktInfo *pktInfo, DFUAckInfo *ackInfo, void *progressBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char writeBuf[DFU_PACKAGE_SIZE * 2] = {"\0"}, resBuf[RESPONSE_DATA_BUFFER_SIZE] = {"\0"} ;
	unsigned char readBuf[DFU_PACKAGE_SIZE * 2] = {"\0"} ;
	unsigned int status = API_SUCCESS, processSize = 0, remainSize = pktInfo->size, readSize = 0 ;
	WriteComInfo info ;

	info.comPort = comPort ;
	info.writeBuf = &writeBuf[0] ;
	info.resBuf = &resBuf[0] ;
	info.timeOut = TIME_OUT * 2 ;
	info.pktType = MTK_NORMAL_PACKAGE ;

	do
	{
		readSize = (remainSize > DFU_PACKAGE_SIZE ? DFU_PACKAGE_SIZE : remainSize) ;
		if (DATA_PACKET == pktInfo->fieldType)
			fread(readBuf, 1, readSize, pktInfo->paramInfo.dataPkt.filePt) ;

		info.writeBufSize = CreateDFUPkt(info.writeBuf, readBuf, readSize, pktInfo, ackInfo) ;
		if (API_SUCCESS != (status = WriteToCom(&info, ackInfo, NULL, JudgeDFUPkt)))
			return SEND_PACKAGE_TIME_OUT ;

		remainSize -= readSize ;
		processSize += readSize ;
        // stop pkt 沒帶資料  pktInfo->size = 0 除以0會出錯
		if ((STOP_PACKET != pktInfo->fieldType) && (NULL != callBackUpdateProcessFun))
			callBackUpdateProcessFun(progressBar, processSize, pktInfo->size) ;
	} 	while(remainSize > 0) ;

	return status ;
} // SendDFUPackage

/**
*/
int inline CreateDFUPkt(unsigned char *buf, unsigned char *date, int dataSize, DFUPktInfo *pktInfo, DFUAckInfo *ackInfo)
{
	unsigned char tempBuf[DFU_PACKAGE_SIZE * 2] = {"\0"} ;
	unsigned int header = 0, tempBufSize = 0 ;
	unsigned short crc = 0 ;
#if (DFU_SDK_VERSION == 710 || DFU_SDK_VERSION == 800)
	//unsigned int updateMode = 4 ;            // ap : 4, bootloader : 2
#endif
	header = CreateDFUHeader(/**((unsigned int *)package_type)*/0x0e, dataSize, ackInfo) ;
	tempBufSize += safe_memcpy(tempBuf + tempBufSize, &header, sizeof(header)) ;
	tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->fieldType, sizeof(pktInfo->fieldType)) ;
	switch (pktInfo->fieldType)
	{
		case START_PACKET :
#if (DFU_SDK_VERSION == 710 || DFU_SDK_VERSION == 800)
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->updateMode, sizeof(int)) ;
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.startPkt.sdSize, sizeof(int)) ;
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.startPkt.blSize, sizeof(int)) ;
#endif
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.startPkt.apSize, sizeof(int)) ;
			break ;
#if (DFU_SDK_VERSION == 710 || DFU_SDK_VERSION == 800)
		case INIT_PACKET :
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.initPkt.devType, sizeof(short)) ;
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.initPkt.devRevision, sizeof(short)) ;
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.initPkt.appVersion, sizeof(int)) ;
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.initPkt.vlidSfDevListLen, sizeof(short)) ;
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.initPkt.vlidSfDevList, sizeof(short)) ;
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.initPkt.binCrc, sizeof(short)) ;
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, &pktInfo->paramInfo.initPkt.filler, sizeof(short)) ;
			break ;
#endif
		case DATA_PACKET :
			tempBufSize += safe_memcpy(tempBuf + tempBufSize, date, dataSize) ;
			break ;
		case STOP_PACKET :
			break ;
	} // switch

	crc = ComputeCRC(tempBuf, tempBufSize, NULL, false) ;
	tempBufSize += safe_memcpy(tempBuf + tempBufSize, &crc, sizeof(crc)) ;

	return ExpandPackageContext(tempBuf, tempBufSize, buf) ;
} // CreateDFUPkt

/**
* @brief Create package's header.
* @param[in] package_type type of package.
* @param[in] buffer_size data size of package.
* @param[in] package_info store package's ack_number and seq_number.
*/
unsigned int inline CreateDFUHeader(int pktType, int bufSize, DFUAckInfo *ackInfo)
{
	unsigned int header = (0x00000000 | INTEGRETY_CHECK | RELIABLE_PACKAGE) ;

	header |= (ackInfo->seqNum & 0x07) << SEQ_NUMBER_OFFSET ;   // seq number : 3 bits
	header |= (ackInfo->ackNum & 0x07) << ACK_NUMBER_OFFSET ;   // ack number : 3 bits
	header |= ((bufSize + sizeof(unsigned int)) & 0x00000fff) << PAYLOAD_LEN_OFFSET ;
	header |= (pktType & 0x000000ff) << PACKAGE_TYPE_OFFSET ;   // default : 0x0e
	header |= ComputeDFUCheckSum(header) << CHECKSUM_OFFSET ;

	return header ;
} // CreateDFUHeader

/**
* @brief Compute checksum.
* @param[in] header input header.
* @return checksum of input header.
*/
unsigned char inline ComputeDFUCheckSum(unsigned int header)
{
	unsigned char checkSum = 0 ;
	// add input unsigned int's byte2 ~ 4, byte1 store checksum.
	for (unsigned int i = 0, mask = 0x000000ff ; i < (sizeof(header) - 1) ; i++, mask <<= BITS_OF_BYTE)
		checkSum += (header & mask) >> (i * BITS_OF_BYTE) ;

	checkSum &= 0xFFu ;
	checkSum = (~checkSum + 1u) ;

	return checkSum ;
} // ComputeDFUCheckSum

/**
* @brief Compute input data's CRC.
* @param[in] *data_buffer input data buffer.
* @param[in] buffer_size size of input data buffer.
* @param[in] *seed_crc seed of compute CRC.
* @return CRC of input data buffer.
*/
unsigned short ComputeCRC(void *buf, unsigned int bufSize, const unsigned short *seed, bool fromFile)
{
	unsigned short crc = (seed == NULL) ? 0xffff : *seed ;
	int oriPos = 0 ;

	if (fromFile)
		oriPos = ftell((FILE *)buf) ;

	for (int i = 0 ; i < bufSize ; i++)
    {
		crc = (unsigned char)(crc >> 8) | (crc << 8) ;
		crc ^= (fromFile ? fgetc((FILE *)buf) : *((unsigned char *)buf + i)) ;
		crc ^= (unsigned char)(crc & 0xff) >> 4 ;
		crc ^= (crc << 8) << 4 ;
        crc ^= ((crc & 0xff) << 4) << 1 ;
    } // for

	if (fromFile)
		fseek((FILE *)buf, oriPos, SEEK_SET) ;

	return crc ;
} // ComputeCRC

/**
* @brief Expand data buffer if it's including special byte(0xc0, 0xdb).
* @param[in] *ori_data_buffer origin input data buffer.
* @param[in] ori_buffer_size size of origin data buffer.
* @param[out] *expand_context_size size of data buffer that after expand.
*/
int inline ExpandPackageContext(unsigned char *oriBuf, unsigned int oriBufSize, unsigned char *outBuf)
{
	const unsigned char startStop = 0xc0 ;
	unsigned char expByte[2] ;
	unsigned short *expList = CheckExpandByteList(oriBuf, oriBufSize) ;
	unsigned short oriOffset = 0, expandOffset = 1 ;
	int outBufSize = 0 ;

	outBufSize = oriBufSize + expList[0] + 2 * sizeof(startStop) ;
	outBuf[0] = outBuf[outBufSize - 1] = startStop ;
	if (0 == expList[0])
	{
		free(expList) ;   // 如果此 ori_data_buffer 都沒有需要擴張的字元則離開
		memcpy(outBuf + expandOffset, oriBuf, oriBufSize) ;
		return outBufSize ;
	} // if

	for (int i = 1, byteIdx = 0, beforeByte = 0, afterByte = 0 ; i <= expList[0] ; ++i)
	{                                               //  ex : c0 x x x x x x c0 x x x x x x c0 x x x x x x c0 x x x x x
		byteIdx = expList[i] ;                      //          before byte    before byte    before byte   after byte
		beforeByte = (1 == i ? byteIdx : byteIdx - expList[i - 1] - 1) ;
		afterByte = (expList[0] == i ? oriBufSize - byteIdx - 1 : 0) ;
													// 複製特殊字元之前的資料
		memcpy(outBuf + expandOffset, oriBuf + oriOffset, beforeByte) ;
		expandOffset += beforeByte ;
		oriOffset += beforeByte ;

		expByte[0] = APP_SLIP_ESC ;  			// 處理特殊字元所在位置
		expByte[1] = (oriBuf[byteIdx] == APP_SLIP_END ? APP_SLIP_ESC_END : APP_SLIP_ESC_ESC) ;
		memcpy(outBuf + expandOffset, expByte, sizeof(expByte)) ;
		expandOffset += sizeof(expByte) ;
		oriOffset += sizeof(char) ;
													// 複製特殊字元之後的資料 (只有最後一個特殊字元會做此項處理)
		memcpy(outBuf + expandOffset, oriBuf + oriOffset, afterByte) ;
	} // for

	return outBufSize ;
} // ExpandPackageContext

/**
* @brief Create a list store all special byte's(0xc0, 0xdb) index.
* @param[in] *check_buffer input data buffer.
* @param[in] buffer_size size of input data buffer.
* @return array[0] store number of special byte, then store their index individually.
*/
__inline unsigned short* CheckExpandByteList(unsigned char *buf, unsigned int bufSize)
{
	unsigned short *tempList = (unsigned short *)calloc(bufSize + 1, sizeof(unsigned short)), *expandList = NULL ;

	for (int i = 0, j = 1 ; i < bufSize ; ++i)
	{
		if (APP_SLIP_END == buf[i] || APP_SLIP_ESC == buf[i])
		{
			tempList[0]++ ;
			tempList[j++] = i ;
		} // if
	} // for

	expandList = (unsigned short *)calloc(tempList[0] + 1, sizeof(unsigned short)) ;
	memcpy(expandList, tempList, sizeof(unsigned short) * (tempList[0] + 1)) ;

	free(tempList) ;
	return expandList ;
} // ExpandPackageContextSize

/**
@brief call back function
*/
int JudgeDFUPkt(WriteComInfo *comInfo, void *ackInfo, FILE *dwFp, void *progressBar, UpdateProcessFun callBackUpdateProcessFun)
{
	unsigned char *startPtr = NULL, *endPtr = NULL ;
	int status = API_SUCCESS, offset = 0, pktSize = 0 ;

	if (NULL == (startPtr = strchr_bysize(comInfo->resBuf, APP_SLIP_END, comInfo->resBufSize)))
		return API_FAIL ;  // 沒有 pkt 開頭字元

	offset = startPtr - comInfo->resBuf ;
	if (NULL == (endPtr = strchr_bysize(startPtr + 1, APP_SLIP_END, comInfo->resBufSize - offset)))
		return API_FAIL ;  // 沒有 pkt 結尾字元

	pktSize = endPtr - (comInfo->resBuf + offset) + 1 ;
	if (DFU_ACK_PACKAGE_SIZE != pktSize)
		return API_FAIL ;

	return CheckDFUResponse(comInfo->resBuf + offset, pktSize, (DFUAckInfo *)ackInfo) ;
} // JudgeDFUPkt

/**
* @brief Anslysis response context.
* @param[out] *package_info store package's ack number and seq number information.
*/
int CheckDFUResponse(unsigned char *buf, int bufSize, DFUAckInfo *pktInfo)
{
	unsigned int resContext = 0 ;
	unsigned char resAckNum = 0, resSeqNum = 0 ;

	memcpy(&resContext, buf + 1, sizeof(resContext)) ;   // 6個byte 前後為 start stop byte : 0xc0 , 中間為int (4 byte)
	if (!((APP_SLIP_END == buf[0]) && APP_SLIP_END == buf[bufSize - 1]))
		return DFU_ACK_FORMAT_ERROR ;

	if (ComputeDFUCheckSum(resContext) != (resContext & 0xff000000) >> CHECKSUM_OFFSET)
		return DFU_ACK_CHECKSUM_ERROR ;

	resAckNum = ((resContext & 0x000000ff) & 0x38) >> 3 ;
	resSeqNum = ((resContext & 0x000000ff) & 0x07) >> 0 ;

	if (resAckNum != (pktInfo->seqNum + 0x01))
	{
		if (!(0x00 == resAckNum && 0x07 == pktInfo->seqNum))
			return DFU_ACK_ACK_NUM_ERROR ;
	} // if
/*
	if (response_seq_num != package_info->ack_num - 0x01)
	{
		if (!(0x00 == package_info->ack_num && 0x07 == response_seq_num))
        {
         	Event |= PACKAGE_SEQ_ERROR ;
			return ;
        } // if
	} // if
*/
	pktInfo->seqNum = resAckNum ;
	pktInfo->ackNum = (pktInfo->ackNum + 1 > 0x07 ? 0x00 : pktInfo->ackNum + 1) ;
	//package_info->ack_num = (response_seq_num + 1 > 0x07 ? 0x02 : response_seq_num + 1) ;
	return API_SUCCESS ;
} // CheckRespond


