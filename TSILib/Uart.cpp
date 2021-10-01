//---------------------------------------------------------------------------

#pragma hdrstop

#include "Uart.h"
#include "Message.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int WriteToCom(WriteComInfo *info, void *ackInfo, FILE *dwFp, JudgePktFun callBackJudgePktFun, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	DWORD nBytesRW ;
	OVERLAPPED overLapped ;

	ZeroMemory(&overLapped, sizeof(OVERLAPPED)) ;
	overLapped.hEvent = CreateEvent(NULL, true, false, NULL) ;

	PurgeComm(info->comPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR) ;
	if (!(info->pktType & RECEIVE_DATA))   // RECEIVE_DATA : 不下命令 純收資料
	{
		WriteFile(info->comPort, info->writeBuf, info->writeBufSize, &nBytesRW, &overLapped) ;
		FlushFileBuffers(info->comPort) ;
	} // if

	return ReadFromCom(info, ackInfo, dwFp, &overLapped, callBackJudgePktFun, processBar, callBackUpdateProcessFun) ;
} // WriteToCom

/**
*/
int ReadFromCom(WriteComInfo *info, void *ackInfo, FILE *dwFp, OVERLAPPED *overLapped, JudgePktFun callBackJudgePktFun, void *processBar, UpdateProcessFun callBackUpdateProcessFun)
{
	int status = API_FAIL, startTime = 0 ;
	DWORD dwEvtMask = 0, dwError = 0, nBytesRW = 0 ;
	COMSTAT stat ;

	startTime = GetTickCount() ;
    SetCommMask(info->comPort, EV_RXCHAR) ;   // 監聽comport
	WaitCommEvent(info->comPort, &dwEvtMask, overLapped) ;
	WaitForSingleObject(overLapped->hEvent, RECEIVE_DATA_TIME_OUT) ;
    info->resBufSize = 0 ;
	while (((GetTickCount() - startTime) < info->timeOut) && API_SUCCESS != status)
	{
		if (dwEvtMask & EV_RXCHAR)
		{
			ClearCommError(info->comPort, &dwError, &stat) ;
			if (stat.cbInQue > 0)
			{
				if (stat.cbInQue > (RESPONSE_DATA_BUFFER_SIZE - info->resBufSize))
                	info->resBufSize = 0 ;

				ReadFile(info->comPort, info->resBuf + info->resBufSize, stat.cbInQue, &nBytesRW, overLapped) ;
				info->resBuf[info->resBufSize += stat.cbInQue] = '\0' ;

				status = callBackJudgePktFun(info, ackInfo, dwFp, processBar, callBackUpdateProcessFun) ;
			} // if
		} // if
	} // while     TIME_OUT內收資料

	if (info->pktType & NO_ACK)
	{   // 此命令不會有response 下完命令後等待timeout即可, 避免有雜訊留在comport 在 timeout前一樣收資料
		PurgeComm(info->comPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) ;
		status = API_SUCCESS ;
	} // if

	if (API_SUCCESS != status)
		status = SEND_PACKAGE_TIME_OUT ;

	return status ;
} // ReadFromCom
