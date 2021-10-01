//---------------------------------------------------------------------------

#ifndef GetTokenH
#define GetTokenH

#include <stdio.h>

#define TOKEN_SIZE         		0x80
#define SWAP_FILE_MAX_SIZE      0x10000

//---------------------------------------------------------------------------
enum windows_version
{
	WINDOWS_UNDEFINED,
	WINDOWS_UNSUPPORTED,
	WINDOWS_2K,
	WINDOWS_XP,
	WINDOWS_2003_XP64,
	WINDOWS_VISTA,
	WINDOWS_7
};

enum CheckSumOption
{
	CHECKSUM_MTK,
    CHECKSUM_OSP
} ;

//---------------------------------------------------------------------------
bool GetToken(unsigned char *buf, int bufSize, unsigned char *token, int *tokenSize, unsigned char *compareList) ;
int safe_sscanf(const char *buffer, const char *format, ...) ;
int safe_atoi(const char *buf) ;
double safe_atof(const char *buf) ;
int IndexOfChar(unsigned char *data_buffer, const int target, const int size) ;
int FileSize(const char *file_name) ;
int FileSize(FILE *inputFile) ;
int FileRemainSize(FILE *inputFile) ;
wchar_t* utf8_to_wchar(const char *str) ;
char* wchar_to_utf8(const wchar_t *wstr) ;
bool CheckFullSentence(unsigned char **inputData, int *dataSize) ;
int GetLastIndexOf(unsigned char *inputData, const int targetChar) ;
int DetectOsVersion(void) ;
char *strchr_bysize(unsigned char *buf, int ch, int size) ;
char *strstr_bysize(unsigned char *buf, unsigned char *find, int unsigned size) ;
unsigned int safe_memcpy(void *targetBuffer, void *oriBuffer, int oriBufferSize) ;
int AllocateMemory(int numOfBuf, int bufSize, ...) ;
bool ReverseArray(void *array, int size) ;
bool SwapFileData(FILE *fp, int startAddr, int swapAddr, int fileSize) ;
float Log2(int x) ;

#endif
