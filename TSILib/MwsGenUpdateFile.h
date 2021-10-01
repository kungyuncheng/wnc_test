//---------------------------------------------------------------------------

#ifndef MwsGenUpdateFileH
#define MwsGenUpdateFileH
//---------------------------------------------------------------------------
#include <vcl.h>

#define GO_LIFE

typedef struct ReadVerInfo
{
    bool alreadyAddSn;
	char ver[0x10];
	char filePath[MAX_PATH];
} ReadVerInfo;

typedef struct AddSnInfo
{
	char ver[0x10];
	char filePath[MAX_PATH];
} AddSnInfo;

typedef struct osInf
{
	unsigned int cmd;
	unsigned int startAddr;
	unsigned int totoalLen;
	unsigned int chkSum;
} osInf;

enum FlashSize
{
	BOOT_SIZE =             0x7000,
#ifdef GO_LIFE
	MASS_SIZE =             0x9F00,
#else
	MASS_SIZE =             0xAF00,
#endif
	FONT_SIZE =             0x200000
};

int AddSnToB0B1FontFile(AddSnInfo info1, AddSnInfo info2, AddSnInfo info3);
int AddSnToApFile(AddSnInfo info);
int ReadVerAp(ReadVerInfo *info);
int ReadVerB0B1Font(ReadVerInfo *info1, ReadVerInfo *info2, ReadVerInfo *info3);
int GenFlashFwFile(char *bootPath, char *massPath, char *apPath, char *outPath);
int GenFlashFontFile(char *fontPath, char *outPath);
int MakeUpdateFile(char *updateInfoPath);

#endif
