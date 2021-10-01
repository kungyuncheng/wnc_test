//---------------------------------------------------------------------------

#pragma hdrstop

#include <windows.h>
#include <setupapi.h>   // 使用此.h需先 include windows.h

#include "GetToken.h"
#include "Message.h"
#include "ConnectComport.h"
#include "com_enum.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
int inline SortCompare(const void *element1, const void *element2) ;

ConnectComport::ConnectComport(void)
{
} // ConnectComport

ConnectComport::~ConnectComport(void)
{
} // ~ConnectComport

/**
* @brief Connect target comport.
* @param[in] comPortId target comport ID.
* @param[in] baudRate
* @param[out] *comPort comport pointer.
* @return if work normaly return API_SUCCESS, or return error message (reference Message.h).
*/
int ConnectComport::Connect(int comPortId, int baudRate, bool flowControlOn, HANDLE *comPort)
{
	COMMTIMEOUTS timeout ;
	DCB dcb ;
	char comPortName[MAX_PATH] = {"\0"} ;

	if (comPortId > 9)
		sprintf(comPortName, "\\\\.\\COM%d", comPortId) ;
	else
        sprintf(comPortName, "COM%d", comPortId) ;

	*comPort = CreateFileA(comPortName,
						  GENERIC_READ | GENERIC_WRITE,
						  0,
						  NULL,
						  OPEN_EXISTING,
						  FILE_FLAG_OVERLAPPED,
						  NULL) ;

	if (*comPort == INVALID_HANDLE_VALUE)
	{
		CloseHandle(*comPort) ;
		return ConnectComPort_CREATE_COMPORT_FAIL ;
	} // if

	timeout.ReadIntervalTimeout = 0 ;
	timeout.ReadTotalTimeoutMultiplier = 0 ;
	timeout.ReadTotalTimeoutConstant = 300 ;
	timeout.WriteTotalTimeoutMultiplier = 0 ;
	timeout.WriteTotalTimeoutConstant = 1000 ;
	SetCommTimeouts(*comPort, &timeout) ;

	if (!SetupComm(*comPort, 0x10000, 0x1000))  //8192
	{
		CloseHandle(*comPort) ;
		*comPort = INVALID_HANDLE_VALUE ;
		return ConnectComPort_SETUP_COMPORT_FAIL ;
	} // if

	if (!GetCommState(*comPort, &dcb))
	{
		CloseHandle(*comPort) ;
		*comPort = INVALID_HANDLE_VALUE ;
		return ConnectComPort_GET_COMPORT_STATE_FAIL ;
	} // if

	dcb.BaudRate = baudRate ;
	dcb.ByteSize = 8 ;
	dcb.Parity = NOPARITY ;
	dcb.StopBits = ONESTOPBIT ;

	dcb.fBinary = 1 ;
	dcb.fOutxCtsFlow = 0 ;
	dcb.fOutxDsrFlow = 0 ;

	dcb.fDtrControl = 1 ;
	dcb.fDsrSensitivity = 0 ;
	dcb.fTXContinueOnXoff = 0 ;
	dcb.fOutX = 0 ;
	dcb.fInX = 0 ;
	dcb.fErrorChar = 0 ;

	dcb.fNull = 0 ;
	dcb.fRtsControl = 0 ;
	dcb.fAbortOnError = 0 ;
	dcb.fDummy2 = 0 ;
	dcb.XonLim = 4096 ;
	dcb.XoffLim = 1024 ;

	if (flowControlOn)
	{
		dcb.fOutX = true ;
		dcb.fInX = true ;
		dcb.fRtsControl = RTS_CONTROL_ENABLE ;
		dcb.fDtrControl = DTR_CONTROL_ENABLE ;
		dcb.fOutxCtsFlow = false ;
		dcb.fOutxDsrFlow = false ;
	} // if

	dcb.XonChar = 0x11 ;
	dcb.XoffChar = 0x13 ;
	dcb.ErrorChar = 0 ;
	dcb.EofChar = 0 ;
	dcb.EvtChar = 0 ;
	dcb.wReserved1 = 0 ;

	if (!SetCommState(*comPort, &dcb))
	{
		CloseHandle(*comPort) ;
		*comPort = INVALID_HANDLE_VALUE ;
		return ConnectComPort_SET_COMPORT_STATE_FAIL ;
	} // if

	if (!PurgeComm(*comPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR))
	{
		CloseHandle(*comPort) ;
		*comPort = INVALID_HANDLE_VALUE ;
		return ConnectComPort_PURGE_COMPORT_FAIL ;
	} // if

	return API_SUCCESS ;
} // ConnectComPort

/**
*/
int ConnectComport::SetBaudRate(HANDLE *comPort, int baudRate, bool flowControlOn)
{
	DCB dcb ;

	if (!GetCommState(*comPort, &dcb))
	{
		CloseHandle(*comPort) ;
		*comPort = INVALID_HANDLE_VALUE ;
		return ConnectComPort_GET_COMPORT_STATE_FAIL ;
	} // if

	dcb.BaudRate = baudRate ;
	if (flowControlOn)
	{
		dcb.fOutX = true;
		dcb.fInX = true;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fOutxCtsFlow = false;
		dcb.fOutxDsrFlow = false;
	} // if

	if (!SetCommState(*comPort, &dcb))
	{
		CloseHandle(*comPort) ;
		*comPort = INVALID_HANDLE_VALUE ;
		return ConnectComPort_SET_COMPORT_STATE_FAIL ;
	} // if

	if (!PurgeComm(*comPort, PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR))
	{
		CloseHandle(*comPort);
		*comPort = INVALID_HANDLE_VALUE ;
		return ConnectComPort_PURGE_COMPORT_FAIL ;
	} // if

	return API_SUCCESS ;
} // SetBaudRate

/**
*/
int ConnectComport::GetBaudRate(HANDLE comPort, int *baudRate)
{
	DCB dcb ;

	if (!GetCommState(comPort, &dcb))
	{
		CloseHandle(comPort) ;
		comPort = INVALID_HANDLE_VALUE ;
		return ConnectComPort_GET_COMPORT_STATE_FAIL ;
	} // if

	*baudRate = dcb.BaudRate ;
	return API_SUCCESS ;
} // GetBaudRate

/**
*/
int ConnectComport::EnumComList(unsigned short *comList, unsigned short *comNum)
{         /*
	HKEY hKey ;
	LPCTSTR PortPath = L"HARDWARE\\DEVICEMAP\\SERIALCOMM\\" ;
	int status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PortPath, 0, KEY_QUERY_VALUE , &hKey), comIdx = 0 ;
	DWORD cSubKey = 0, cMaxSubkeyLength = 0, cMaxClassLen = 0, cValues = 0, cMaxValueNameLen = 0, cMaxValueLen = 0 ;
	DWORD cValueName = 128, cType = 0, cData = 0 ;
	char *pData = NULL ;
	wchar_t szValueName[128] = {L"\0"} ;

	*comNum = 0 ;
	if(ERROR_SUCCESS != status)
		return ConnectComPort_OPEN_KEY_FAIL ;

	RegQueryInfoKey(hKey, NULL, NULL, NULL, &cSubKey, &cMaxSubkeyLength, &cMaxClassLen, &cValues, &cMaxValueNameLen, &cMaxValueLen, NULL, NULL) ;
	for (DWORD i = 0 ; i < cValues ; ++i)
	{
		//RegEnumValueA(hKey, i, szValueName, &cValueName, NULL, &cType, pData, &cData) ;
		RegEnumValue(hKey, i, szValueName, &cValueName, NULL, &cType, pData, &cData) ;
		pData = (char *)calloc(cData, sizeof(char)) ;

		//RegEnumValueA(hKey, i, szValueName, &cValueName, NULL, &cType, pData, &cData) ;
		RegEnumValue(hKey, i, szValueName, &cValueName, NULL, &cType, pData, &cData) ;
		switch (cType)
		{
			case REG_SZ:
				if (NULL != wcsstr(szValueName, L"Silabser") || NULL != wcsstr(szValueName, L"slabser"))
				{
					sscanf(pData, "COM%d", &comList[(*comNum)++]) ;
				} // if
				break;
		} // switch

		memset(szValueName, 0x00, sizeof(szValueName)) ;
     	free(pData) ;
	} // for

	RegCloseKey(hKey) ;
	qsort(comList, *comNum, sizeof(short), SortCompare) ;  // comlist 用 quick sort 由小到大排序
		  */

	/*
	HANDLE com = INVALID_HANDLE_VALUE ;
	char comName[50] = {"\0"} ;

	*comNum = 0 ;
	for (int i = 1 ; i < MAX_COM_NUMBER ; ++i)
	{
		if (i > 9)
			sprintf(comName, "\\\\.\\COM%d\0", i) ;
		else
            sprintf(comName, "COM%d\0", i) ;

		com = CreateFileA(comName,
						 GENERIC_READ | GENERIC_WRITE,
						 0,
						 NULL,
						 OPEN_EXISTING,
						 FILE_ATTRIBUTE_NORMAL,
						 NULL) ;

		if (INVALID_HANDLE_VALUE != com)
		{
			comList[(*comNum)++] = i ;
			CloseHandle(com) ;
			com = INVALID_HANDLE_VALUE ;
        } // if
	} // for */

	int status = API_FAIL ;

	*comNum = MAX_COM_NUMBER ;
	if (COM_ENUM_OK != (status = ComPortEnumerate(comList, comNum)))
		return API_FAIL ;

	return API_SUCCESS ;
} // EnumComList

/**
*/
int inline SortCompare(const void *element1, const void *element2)
{
	return (*((short *)element1) <= (*(short *)element2) ? -1 : 1) ;
} // SortCompare
