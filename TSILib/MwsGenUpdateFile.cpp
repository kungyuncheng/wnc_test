//---------------------------------------------------------------------------

#pragma hdrstop
#include <stdio.h>

#include "MwsGenUpdateFile.h"
#include "Message.h"
#include "GetToken.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int AddSnToB0B1FontFile(AddSnInfo info1, AddSnInfo info2, AddSnInfo info3)
{
	const int ITEM = 3, VER_NUM_SIZE = 8;
	const unsigned char reserve = 0xff;
	const unsigned int fileSize[ITEM] = {BOOT_SIZE, MASS_SIZE, FONT_SIZE}, validFileNum = 0;

	char *fileName[ITEM] = {info1.filePath, info2.filePath, info3.filePath};
	char *ver[ITEM] = {info1.ver, info2.ver, info3.ver};
	unsigned char buf[VER_NUM_SIZE] = {"\0"};
    int status = API_SUCCESS;
	FILE *fp = NULL;

	for (int i = 0, oriSize = 0 ; i < ITEM ; ++i)
	{
		if (NULL == (fp = fopen(fileName[i], "rb+")))
			return (status = FILE_NOT_EXIST);

		if ((oriSize = FileSize(fp)) < fileSize[i])
		{
			fseek(fp, 0, SEEK_END);
			for (int j = oriSize ; j < (fileSize[i] - VER_NUM_SIZE) ; ++j)
				fputc(reserve, fp);

			memset(buf, 0xff, sizeof(buf));
			memcpy(buf, ver[i], strlen(ver[i]));
			fwrite(buf, sizeof(buf), 1, fp);
		} // if

		fclose(fp);
	} // for

	return status;
} // AddSnToB0B1FontFile

/**
*/
int AddSnToApFile(AddSnInfo info)
{
	const unsigned char reserve = 0xff;
	const unsigned int VER_NUM_SIZE = 0x10, headerSize = 0x100;
	unsigned char buf[headerSize] = {"\0"};
	bool alreadyAddSn = true;
	int doTimes = 0, wIdx = 0, rIdx = 0, verSize = 0;;
	int bufSize = 0, fileSize = 0, apVer = 0, status = API_SUCCESS;
	FILE *fp = NULL;

	if (NULL == (fp = fopen(info.filePath, "rb+")))
		return (status = FILE_NOT_EXIST);

	fseek(fp, VER_NUM_SIZE, SEEK_SET);
	fread(buf, sizeof(buf), 1, fp);
	for (int i = 0 ; i < headerSize && alreadyAddSn ; ++i)
	{
		if (reserve != buf[i])
			alreadyAddSn = false;
	} // for

	if (alreadyAddSn)
		goto END_HANDLE;

	fseek(fp, 0, SEEK_SET);
	fileSize = FileSize(fp);
	wIdx = (fileSize >= headerSize ? fileSize : headerSize);
	rIdx = (fileSize >= headerSize ? wIdx - headerSize : 0);
	doTimes = (rIdx / headerSize) + (0 == (rIdx % headerSize) ? 0 : 1);
	while (doTimes-- >= 0)
	{
		bufSize = ((wIdx - rIdx) > fileSize ? fileSize : (wIdx - rIdx));

		fseek(fp, rIdx, SEEK_SET);
		fread(buf, bufSize, 1, fp);

		fseek(fp, wIdx, SEEK_SET);
		fwrite(buf, bufSize, 1, fp);

		rIdx = (rIdx >= headerSize ? rIdx - headerSize : 0);
		wIdx = rIdx + headerSize;
	} // while

	fseek(fp, 0, SEEK_SET);
	memset(buf, reserve, sizeof(buf));
	sscanf(info.ver, "%d", &apVer);
	memcpy(buf, &apVer, sizeof(apVer));
	fwrite(buf, sizeof(buf), 1, fp);

END_HANDLE:
	fclose(fp);
	return status;
} // AddSnToApFile

/**
*/
int ReadVerAp(ReadVerInfo *info)
{
	const unsigned char reserve = 0xff;
	const unsigned int VER_NUM_SIZE = 0x10, headerSize = 0x100 - VER_NUM_SIZE;
	unsigned char buf[headerSize] = {"\0"};
	int status = API_SUCCESS;
	FILE *fp = NULL;

	info->alreadyAddSn = true;
	if (NULL != (fp = fopen(info->filePath, "rb")))
	{
		fseek(fp, VER_NUM_SIZE, SEEK_SET);
		fread(buf, sizeof(buf), 1, fp);
		for (int i = 0 ; i < headerSize && info->alreadyAddSn ; ++i)
		{
			if (reserve != buf[i])
				info->alreadyAddSn = false;
		} // for

		if (info->alreadyAddSn)
		{   // 已增加過版本
			fseek(fp, 0, SEEK_SET);
			fread(info->ver, sizeof(int), 1, fp);
        } // if

		fclose(fp);
	} // if

	return status;
} // ReadVerAp

/**
*/
int ReadVerB0B1Font(ReadVerInfo *info1, ReadVerInfo *info2, ReadVerInfo *info3)
{
	const int ITEM = 3, SERIES_NUM_SIZE = 8;
	const char reserve = 0xff;
	unsigned int fileSize[ITEM] = {BOOT_SIZE, MASS_SIZE, FONT_SIZE};
	FILE *fp = NULL;
	ReadVerInfo *info[ITEM] = {info1, info2, info3};
	int status = API_SUCCESS;

	for (int i = 0, oriSize = 0, endIdx = 0 ; i < ITEM ; ++i)
	{
		info[i]->alreadyAddSn = false;
		if (NULL != (fp = fopen(info[i]->filePath, "rb")))
		{
			if ((oriSize = FileSize(fp)) == fileSize[i])
			{
				fseek(fp, -1 * SERIES_NUM_SIZE, SEEK_END);
				fread(info[i]->ver, SERIES_NUM_SIZE, 1, fp);
				endIdx = strchr(info[i]->ver, reserve) - info[i]->ver;
				info[i]->ver[endIdx] = 0x00;
				info[i]->alreadyAddSn = true;
			} // if

			fclose(fp);
		} // if
	} // for

	return status;
} // ReadVerB0B1Font

/**
*/
int GenFlashFwFile(char *bootPath, char *massPath, char *apPath, char *outPath)
{
	const int ITEM = 3;
	const unsigned char *inFileName[ITEM] = {bootPath, massPath, apPath};
	FILE *inFp = NULL, *outFp = fopen(outPath, "wb");
	int status = API_SUCCESS;

	if (NULL == outFp)
	{
		status = FILE_NOT_EXIST;
		goto END_HANDLE;
    } // if

	for (int i = 0 ; i < ITEM ; ++i)
	{
		if (NULL == (inFp = fopen(inFileName[i], "rb")))
		{
			status = FILE_NOT_EXIST;
			goto END_HANDLE;
        } // if

		for (int j = 0, fileSize = FileSize(inFp) ; j < fileSize ; ++j)
			fputc(fgetc(inFp), outFp);

		fclose(inFp);
	} // for

END_HANDLE:
	fclose(inFp);
	fclose(outFp);
	return status;
} // GenFlashFwFile

/**
*/
int GenFlashFontFile(char *fontPath, char *outPath)
{
	const int offset = 0x3ffff;
	const unsigned char reserve = 0xff;
	FILE *inFp = fopen(fontPath, "rb"), *outFp = fopen(outPath, "wb");
	int status = API_SUCCESS;
	AnsiString str;

	if (NULL == inFp || NULL == outFp)
	{
		status = FILE_NOT_EXIST;
		goto END_HANDLE;
    } // if

	for (int i = 0 ; i <= offset ; ++i)
		fwrite(&reserve, sizeof(reserve), 1, outFp);

	for (int i = 0, fileSize = FileSize(inFp) ; i < fileSize ; ++i)
		fputc(fgetc(inFp), outFp);

END_HANDLE:
	fclose(inFp);
	fclose(outFp);
    return status;
} // GenFlashFontFile

/**
*/
int MakeUpdateFile(char *updateInfoPath)
{
	FILE *wfp = NULL, *rfp = NULL, *cpyFp = NULL;
	char fName[16] = {"\0"};
	//int i, j, k;
	int tmp_s32;
	int len, fileIdx, comma;
	unsigned char tmp_buf[256];
	unsigned char tmp_u8;
	osInf osinf;

	for(int i = 0 ; i < 6 ; i++)
	{
		if (4 != i)
		{
			sprintf(fName, "Output/TSI%d.bin", i);
			if(rfp = fopen(fName, "rb"))
			{
				fclose(rfp);
				if(remove(fName) !=0 )
					return API_FAIL;
			}
		}
	}

	fileIdx = 0;
	if (NULL == (rfp = fopen(updateInfoPath, "r")))
		return FILE_NOT_EXIST;

	while(!feof(rfp))
	{
		len = 0;
		comma = 0;
		memset(&osinf, 0, sizeof(osInf));
		fgets(tmp_buf, sizeof(tmp_buf), rfp);
		for(int i = 0 ; ; i++)
		{
			if((tmp_buf[i] == ',') || (tmp_buf[i] == '\n') || (tmp_buf[i] == '\0'))
			{
				comma++;
				if(comma == 1)
				{
					memcpy(fName, &tmp_buf[0], len);
					fName[len] = '\0';
					strcat(&fName[len], ".bin");
				}
				else if(comma == 2)
				{
					osinf.cmd = tmp_buf[i-1]-'0';
				}
				else if(comma == 3)
				{
					for(int j = i - 1, k = 0 ; k < len ; j--, k++)
					{
						if(tmp_buf[j]>='A'&&tmp_buf[j]<='F')
							tmp_s32 = tmp_buf[j]-'A'+10;
						else if(tmp_buf[j]>='a'&&tmp_buf[j]<='f')
							tmp_s32 = tmp_buf[j]-'a'+10;
						else
							tmp_s32 = tmp_buf[j]-'0';
						osinf.startAddr += tmp_s32*pow(16,k);
					}
					break;
				}
				len = 0;
			}
			else
				len ++;
		}
		cpyFp = fopen(fName, "rb");
		if(cpyFp == NULL)
			continue;
		else
		{
			if (4 != fileIdx)
			{
				fseek(cpyFp, 0, SEEK_END);
				osinf.totoalLen = ftell (cpyFp);
				sprintf(fName, "Output/TSI%d.bin", fileIdx);
				wfp = fopen(fName, "wb+");

				// copy stuff
				fseek(wfp, 16, SEEK_SET);
				fseek(cpyFp, 0, SEEK_SET);
				for(int i = 0 ; i < osinf.totoalLen ; i++)
				{
					fread(&tmp_u8, 1, 1, cpyFp);
					fwrite(&tmp_u8, 1, 1, wfp);
					osinf.chkSum += tmp_u8;
				}

				//Adding Header
				fseek(wfp, 0, SEEK_SET);
				fwrite(&osinf, 16, 1, wfp);
				fclose(wfp);
			}
			fclose(cpyFp);
			fileIdx++;
		}
	}

	return API_SUCCESS;
} // MakeUpdateFile


