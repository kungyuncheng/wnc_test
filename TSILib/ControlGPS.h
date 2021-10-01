//---------------------------------------------------------------------------

#ifndef ControlGPSH
#define ControlGPSH

#include <windows.h>

#define UPDATE_GPS_BAUDRATE 			115200

//---------------------------------------------------------------------------

typedef struct UpdateGPSInfo
{
	int comId ;
	unsigned char *scatPath ;
	unsigned char *daPath ;
	void *progressBar ;
} UpdateGPSInfo ;

//---------------------------------------------------------------------------
int UpdateGPS(HANDLE comPort, unsigned char *fileName, UpdateGPSInfo updateGPSInfo) ;

#endif
