//---------------------------------------------------------------------------

#pragma hdrstop
#include <windows.h>
//#include <fstream>
//#include <string.h>
//#include <stdlib.h>
//#include <stdarg.h>
#include <math.h>

#include "GetToken.h"
#include "Message.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
bool GetToken(unsigned char *buf, int bufSize, unsigned char *token, int *tokenSize, unsigned char *compareList)
{
	unsigned char *tokenEndPtr = NULL ;
	int numOfCompare = strchr((char *)compareList, '\0') - compareList ;
	unsigned int size = 0xFFFFFFFF ;

	*tokenSize = 0 ;
	for (int i = 0 ; i < numOfCompare ; ++i)
	{
		if (NULL != (tokenEndPtr = strchr_bysize((char *)buf, compareList[i], bufSize)))
		{
			if ((tokenEndPtr - buf) < size)
				size = tokenEndPtr - buf ;
		} // if
	} // for

	if (0xFFFFFFFF == size)
		return false ;

	memcpy(token, buf, size) ;
	token[size] = '\0' ;

    *tokenSize = size ;
	return true ;
} // GetToken

/**
* @brief Check input buffer equal NULL or not before call safe_atoi() function.
* @param[in] buffer input buffer.
* @return if input buffer equal NULL return 0, or return the float number after transform.
*/
int safe_atoi(const char *buf)
{
	const int ITEM = 1 ;
	const char *compare = "," ;
	char *temp = NULL ;
	int bufSize = 0, val = 0 ;

	if (NULL == buf)
		return 0 ;

	bufSize = strchr(buf, '\0') - buf + 1 ;
	if (NULL == (temp = (char *)calloc(bufSize, sizeof(char))))
		return 0 ;

	memcpy(temp, (void *)buf, bufSize) ;
	for (int i = 0 ; i < bufSize ; ++i)
	{
		for (int j = 0 ; j < ITEM ; ++j)
		{
			if (compare[j] == temp[i])
			{
				memcpy((void *)(temp + i), (void *)(temp + (i + 1)), (bufSize - (i + 1))) ;
				bufSize -= 1 ;
				temp[bufSize] = '\0' ;
            } // if
        } // for
    } // for

	val = atoi(temp) ;
	free(temp) ;
	return val ;
} // safe_atoi

/**
*/
double safe_atof(const char *buf)
{
	const ITEM = 1 ;
	const char *compare = "," ;
	char *temp = NULL ;
	int bufSize = 0 ;
	double val = 0 ;

	if (NULL == buf)
		return 0 ;

	bufSize = strchr(buf, '\0') - buf + 1 ;
	if (NULL == (temp = (char *)calloc(bufSize, sizeof(char))))
		return 0 ;

	memcpy(temp, (void *)buf, bufSize) ;
	for (int i = 0 ; i < bufSize ; ++i)
	{
		for (int j = 0 ; j < ITEM ; ++j)
		{
			if (compare[j] == temp[i])
			{
				memcpy((void *)(temp + i), (void *)(temp + (i + 1)), (bufSize - (i + 1))) ;
				bufSize -= 1 ;
				temp[bufSize] = '\0' ;
            } // if
        } // for
	} // for

	val = atof(temp) ;
	free(temp) ;
	return val ;
} // safe_atof

/**
* @brief Check input buffer equal NULL or not before call sscanf() function.
* @param[in] *buffer input buffer.
* @param[in] *format format type.
* @param[in] ... target buffer after transform from input buffer.
*/
int safe_sscanf(const char *buffer, const char *format, ...)
{
	va_list args ;

	if (NULL == buffer)
		return API_FAIL ;

	va_start(args, format) ;
	vsscanf(buffer, format, args) ;
	va_end(args) ;

	return API_SUCCESS ;
} // safe_sscanf

/**
* @brief Return index of target character.
* @param[in] *data_buffer input data array.
* @param[in] target target character.
* @return index of target character.
*/
int IndexOfChar(unsigned char *data_buffer, const int target, const int size)
{
	char *target_buffer = (NULL == data_buffer ? NULL : strchr_bysize((char *)data_buffer, target, size)) ;

	return (NULL == target_buffer ? -1 : target_buffer - data_buffer) ;
} // IndexOfChar


/**
* @brief Count input file size.
* @param[in] *file_name input file name.
* @return file size(bytes).
*/
int FileSize(const char *file_name)
{
	FILE *fp = fopen(file_name, "rb") ;
	int len = 0 ;

	if (NULL == fp)
		return len ;  /// @note if file not exist, return 0.

	fseek(fp, 0, SEEK_END) ;
	len = ftell(fp) ;
	fclose(fp) ;

	return len ;
} // FileSize

/**
* @brief Count input file size.
* @param[in] *file_name input file name.
* @return file size(bytes).
*/
int FileSize(FILE *inputFile)
{
	int len = 0 ;

	fseek(inputFile, 0, SEEK_END) ;
	len = ftell(inputFile) ;
	fseek(inputFile, 0, SEEK_SET) ;

	return len ;
} // FileSize

/**
*/
int FileRemainSize(FILE *inputFile)
{
	int len = 0, nowPos = 0 ;

	if (NULL == inputFile)
		return len ;

	nowPos = ftell(inputFile) ;
	fseek(inputFile, 0, SEEK_END) ;
	len = ftell(inputFile) ;
	fseek(inputFile, nowPos, SEEK_SET) ;

	return (len - nowPos) ;
} // FileSize

/**
* @brief transfor char to wchar_t, unlike mbstowcs(), this function also can transform chinese.
* @param[in] *str a char array which wnats transfor to a wchar_t array.
* @return a wchar_t array after transform.
*/
wchar_t* utf8_to_wchar(const char *str)
{
	int size = 0;
	wchar_t* wstr = NULL;

	// Find out the size we need to allocate for our converted string
	size = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	if (size <= 1)	// An empty string would be size 1
		return NULL;

	if ((wstr = (wchar_t*)calloc(size, sizeof(wchar_t))) == NULL)
		return NULL;

	MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, size);
	return wstr;
} // utf8_to_wchar


/**
* @brief transfor wchar_t to char, unlike wcstombs(), this function also can transform chinese.
* @param[in] *wstr a wchar_t array which wnats transfor to a char array.
* @return a char array after transform.
*/
char* wchar_to_utf8(const wchar_t *wstr)
{
	int size = 0;
	char* str = NULL ;
	// Find out the size we need to allocate for our converted string
	size = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL) ;
	if (size <= 1)	// An empty string would be size 1
		return NULL ;

	if ((str = (char*)calloc(size, 1)) == NULL)
		return NULL ;

	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, size, NULL, NULL) ;
	return str ;
} // wchar_to_utf8

/**
* @brief Check the input string including at least one confirm string or not.
* @param[in] **inputData input string.
* @param[in] *dataSize input_data size
* @return if input string checksum correct return true, or return false.
*/
bool CheckFullSentence(unsigned char **inputData, int *dataSize)
{
	int endIndex = IndexOfChar(*inputData, '\n', *dataSize) ;

	if (0 == *dataSize)
		return false ;
										 
	if (-1 == endIndex)   // 若字串不包含 '\n' 回傳 false
		return false ;

	if (endIndex >= *dataSize)	
		return false ;

	return true ;   
} // CheckTargetChar

/**
* @brief Check the last index of target char in input data.
* @param[in] *inputData input data buffer.
* @param[in] targetChar target character.
* @return if the last character exist return its index, or return -1.
*/
int GetLastIndexOf(unsigned char *inputData, const int targetChar)
{
	char *targetStr = NULL ;

	if (NULL == inputData)
		return -1 ;

	targetStr = strrchr((char *)inputData, targetChar) ;
	return (NULL == targetStr ? -1 : targetStr - inputData) ;
} // GetLastIndexOf

/**
* @brief Detect computer's windows version.
*/
int DetectOsVersion(void)
{
	OSVERSIONINFO osVersionInfo ;

	memset(&osVersionInfo, 0, sizeof(OSVERSIONINFO)) ;
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
	if ((0 == GetVersionEx(&osVersionInfo)) || (osVersionInfo.dwPlatformId != VER_PLATFORM_WIN32_NT))
		return WINDOWS_UNSUPPORTED ;

	if (5 == osVersionInfo.dwMajorVersion)
	{
		if (0 == osVersionInfo.dwMinorVersion)
			return WINDOWS_2K ;
		else if (1 == osVersionInfo.dwMinorVersion)
			return WINDOWS_XP ;
		else if (2 == osVersionInfo.dwMinorVersion)
			return WINDOWS_2003_XP64 ;
	} // if
	else if (6 <= osVersionInfo.dwMajorVersion)
	{
		if (7000 > osVersionInfo.dwBuildNumber)
			return WINDOWS_VISTA ;
		else
			return WINDOWS_7 ;
	} // else if

	return WINDOWS_UNSUPPORTED ;
} // DetectOsVersion

/**
*/
char *strchr_bysize(unsigned char *buf, int ch, int size)
{
	int targetSize = 0 ;
	char *targetPt = NULL ;

	if (NULL != (targetPt = strchr((char *)buf, ch)))
		return targetPt ;

	if ((targetSize = (strchr((char *)buf, '\0') - buf + 1)) >= size)
		return NULL ;

	for (int i = targetSize ; i < size ; ++i)
	{
		if (ch == *(buf + i))
			return (targetPt = buf + i) ;
	} // for

	return NULL ;
} // strchr_bysize

/**
*/
char *strstr_bysize(unsigned char *buf, unsigned char *find, int unsigned size)
{
	unsigned char c = '\0', sc = '\0' ;
	size_t len = 0, targetSize = 1 ;

	if (0 != (c = *find++))
	{
		len = strlen(find) ;
		do
		{
			do
			{
				sc = *buf++ ;
				if (targetSize++ >= size && 0 == sc)
					return (NULL) ;
			} 	while (sc != c) ;
		} 	while (0 != strncmp(buf, find, len)) ;
		buf-- ;
	} // if

	return ((char *)buf) ;
} // strchr_bysize

/**
* @brief Call memcpy function safely. (memcpy copy null pointer will occur error)
* @param[in] *target_buffer target buffer.
* @param[in] *ori_buffer origin buffer.
* @param[in] size of origin buffer.
* @return if *ori_buffer is NULL retun 0, or return size of origin buffer.
*/
unsigned int safe_memcpy(void *targetBuffer, void *oriBuffer, int oriBufferSize)
{
	if (NULL == oriBuffer)
		return 0 ;

	memcpy(targetBuffer, oriBuffer, oriBufferSize) ;
	return oriBufferSize ;
} // safe_memcpy

/**
*/
int AllocateMemory(int numOfBuf, int bufSize, ...)
{
	int num = 0 ;
	unsigned char **bufListPt[MAX_PATH] = {NULL}, **bufPt = NULL ;
	va_list args ;

	va_start(args, bufSize) ;
	while (num < numOfBuf)
	{
		bufPt = va_arg(args, unsigned char **) ;
		if (NULL != (*bufPt = (unsigned char *)calloc(bufSize, sizeof(unsigned char))))
			bufListPt[num++] = bufPt ;
		else
		{
			for (int i = 0 ; i < num ; ++i)
			{
				free(*bufListPt[i]) ;
				*bufListPt[i] = NULL ;
			} // for

			return AllocateThreadParameter_MEMORY_NULL ;
		} // else
	} // while

	va_end(args) ;
	return API_SUCCESS ;
} // AllocateMemory

/**
*/
bool ReverseArray(void *array, int size)
{
	int count = size / 2 ;

	for (int i = 0, j = size - 1 ; i < count ; ++i, --j)
	{
		((char *)array)[i] ^= ((char *)array)[j] ;
		((char *)array)[j] ^= ((char *)array)[i] ;
        ((char *)array)[i] ^= ((char *)array)[j] ;
	} // for

	return true ;
} // ReverseArray

/**
*@note if startAddr too small (decided from fileSize), the handle time maybe become larger.
*/
bool SwapFileData(FILE *fp, int startAddr, int swapAddr, int fileSize)
{
	int p1Size = 0, p2Size = 0, readSize = 0, offset = swapAddr ;
	int	moveSize = 0, remaindSize = 0 ;
	unsigned char buf1[SWAP_FILE_MAX_SIZE] = {"\0"}, buf2[SWAP_FILE_MAX_SIZE] = {"\0"} ;

	do
	{
		if (0 == swapAddr || startAddr >= swapAddr)
			return false ;

		p1Size = swapAddr - startAddr ;
		p2Size = fileSize - swapAddr ;
		remaindSize = moveSize = (p1Size > p2Size ? p2Size : p1Size) ;
		do
		{   // 每次最大搬移 SWAP_FILE_MAX_SIZE , 若需搬移的size > SWAP_FILE_MAX_SIZE則分段搬
			readSize = (remaindSize >= SWAP_FILE_MAX_SIZE ? SWAP_FILE_MAX_SIZE : remaindSize) ;

			fseek(fp, startAddr, SEEK_SET) ;
			fread(buf1, 1, readSize, fp) ;

			fseek(fp, offset, SEEK_SET) ;
			fread(buf2, 1, readSize, fp) ;

			fseek(fp, offset, SEEK_SET) ;
			fwrite(buf1, 1, readSize, fp) ;

			fseek(fp, startAddr, SEEK_SET) ;
			fwrite(buf2, 1, readSize, fp) ;

			remaindSize -= readSize ;
			startAddr += readSize ;
			offset += readSize ;
		}   while (remaindSize > 0) ;

        offset = swapAddr = (p1Size > p2Size ? swapAddr : startAddr + moveSize) ;
	}	while (p1Size != p2Size) ;

	return true ;
} // SwapFileData

/**
*/
float Log2(int x)
{
	float temp1 = log((double)x), temp2 = log(2.0) ;

	return temp1 / temp2 ;
} // Log2

