//---------------------------------------------------------------------------

#pragma hdrstop

#include "ControlGPS.h"
#include "DownLoadFlashThread.h"
#include "Message.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int UpdateGPS(HANDLE comPort, unsigned char *fileName, UpdateGPSInfo updateGPSInfo)
{
	TFlashThread *flashThread = new TFlashThread(true, updateGPSInfo.progressBar, updateGPSInfo.comId, UPDATE_GPS_BAUDRATE) ;
    int status = API_SUCCESS ;

	DA_Create(&flashThread->mDaHandle) ;
	DL_Create(&flashThread->mDlHandle) ;

	DA_SetStartAddr(flashThread->mDaHandle, 0x00C00);
	status = DA_LoadByFilepath(flashThread->mDaHandle, updateGPSInfo.daPath) ;
	if (FTHND_OK != FTHND_RET(status))
		return (status = API_FAIL) ;

	DL_LoadScatter(flashThread->mDlHandle, updateGPSInfo.scatPath) ;
	DL_SetPacketLength(flashThread->mDlHandle, 256) ;
	status = DL_Rom_LoadByFilepath(flashThread->mDlHandle, 0, fileName) ;
	if (FTHND_OK != FTHND_RET(status))
		return (status = API_FAIL) ;

	DL_Rom_SetEnableAttr(flashThread->mDlHandle, 0, _TRUE) ;
	flashThread->Resume() ;
	flashThread->WaitFor() ;
	if (FTHND_OK != flashThread->Result)
		return (status = API_FAIL) ;

	return API_SUCCESS ;
} // UpdateGPS
