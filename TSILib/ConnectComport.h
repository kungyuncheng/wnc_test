//---------------------------------------------------------------------------

#ifndef ConnectComportH
#define ConnectComportH

#include <windows.h>
#include <stdio.h>

//---------------------------------------------------------------------------
class ConnectComport
{
private :
protected :
public :
	static const int COMPORT_STATUS_SIZE = 32 ;
	static const int MAX_COM_NUMBER =      256 ;

	ConnectComport(void) ;
	~ConnectComport(void) ;
	int Connect(int comPortId, int baudRate, bool flowControlOn, HANDLE *comPort) ;
	//int Connect(int comPortId, int baudRate, bool flowControlOn, HANDLE comPort) ;
	int SetBaudRate(HANDLE *comPort, int baudRate, bool flowControlOn) ;
	int GetBaudRate(HANDLE comPort, int *baudRate) ;
	int EnumComList(unsigned short *comList, unsigned short *comNum) ;
} ;

#endif
