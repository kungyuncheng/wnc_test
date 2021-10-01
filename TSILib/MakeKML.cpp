//---------------------------------------------------------------------------

#pragma hdrstop

#include "MakeKML.h"
#include "Message.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

/**
*/
int ConvertBinFile(int option, unsigned char *inFileName, unsigned char *outFileName, int devType)
{
	FILE *inFp = fopen(inFileName, "rb"), *outFp = fopen(outFileName, "w") ;
	KmlInfo info ;
	
	if (NULL == inFp || NULL == outFp)
		return FILE_NOT_EXIST ;

	switch (option)
	{
		case CONVERT_CSV :
			ConvertBinFile2CSV(devType, inFp, outFp) ;
			break ;
		case CONVERT_KML :
			ConvertBinFile2KML(devType, inFp, outFp) ;
			break ;
	} // switch

	fclose(inFp) ;
	fclose(outFp) ;

	return API_SUCCESS ;
} // ConvertBinFile

/**
*/
void inline ConvertBinFile2CSV(int devType, FILE *inFp, FILE *outFp)
{
	unsigned char temp[MAX_PATH] = {"\0"} ;
	int idx = 0 ;
	KmlInfo info ;

	fseek(inFp, 0, SEEK_SET) ;
	switch (devType)
	{
		case DEVICE_TYPE_G05 :
		case DEVICE_TYPE_G05H :
		case DEVICE_TYPE_G07N :
		case DEVICE_TYPE_G07P :
		case DEVICE_TYPE_G08 :
		case DEVICE_TYPE_G08P :
			fprintf(outFp, "%s", CSV_TITLE_G05SERIES) ;
			do
			{
				memset(&info.g05Series, 0x00, sizeof(LogInfoG05Series)) ;
				ReadLogInfoG05Series(inFp, &info.g05Series, devType) ;
				if (0xffff != info.g05Series.year)
				{
					fprintf(outFp, "%04d/%02d/%02d %02d:%02d:%02d, ", info.g05Series.year, info.g05Series.month, info.g05Series.day,
																	  info.g05Series.hour, info.g05Series.minute, info.g05Series.second) ;
					fprintf(outFp, "%.8f,%.8f,%d,%d,%d\n", info.g05Series.latitude, info.g05Series.longtitude,
														   info.g05Series.courseId, info.g05Series.holeNum, info.g05Series.holeNumSeq) ;
				} // if
			}   while (0xffff != info.g05Series.year) ;
			break ;
		case DEVICE_TYPE_G06MA :
		case DEVICE_TYPE_G06 :
			fprintf(outFp, "%s", CSV_TITLE_G06SERIES) ;
			do
			{
				memset(&info.g06Series, 0x00, sizeof(LogInfoG06)) ;
				ReadLogInfoG06(inFp, &info.g06Series) ;
				if (0xffffffff != info.g06Series.utc)
				{
					strftime(temp, MAX_PATH, "%Y\/%m\/%d %X\0", localtime((time_t *)&info.g06Series.utc)) ;
					fprintf(outFp, "%s,%.8f,%.8f,%d,%d,%d,%d,%d\n", temp, info.g06Series.latitude, info.g06Series.longtitude,
																	info.g06Series.gpsValid, info.g06Series.courseId, info.g06Series.holeNum,
																	info.g06Series.holeNumSeq, info.g06Series.strokesAtCurHole) ;
				} // if
			}   while (0xffffffff != info.g06Series.utc) ;
			break ;
		case DEVICE_TYPE_P51 :
			fprintf(outFp, "%s", CSV_TITLE_P51) ;
            do
			{
				memset(&info.p51, 0x00, sizeof(LogInfoP51)) ;
				ReadLogInfoP51(inFp, &info.p51) ;
				if (0xffffffffffffffff != info.p51.utc)
				{
					strftime(temp, MAX_PATH, "%Y\/%m\/%d %X\0", localtime((time_t *)&info.p51.utc)) ;
					fprintf(outFp, "%s,%d,%.8f,%.8f,%.2f,%f,%f,%f,%f,%f,%f,%d\n",
																	temp, info.p51.valid, info.p51.latitude, info.p51.longtitude,
																	info.p51.speed, info.p51.height, info.p51.heading,
																	info.p51.ecomposs, info.p51.gsensorX, info.p51.gsensorY,
																	info.p51.gsensorZ, info.p51.flag) ;
				} // if
			}   while (0xffffffffffffffff != info.p51.utc) ;
			break ;
	} // switch
} // ConvertBinFile2CSV

/**
*/
void inline ConvertBinFile2KML(int devType, FILE *inFp, FILE *outFp) 
{
	unsigned char temp[MAX_PATH] = {"\0"} ;
	int idx = 0 ;
	KmlInfo info ;

	MakeKmlTitle(outFp) ;
    for (int makeOption = MAKE_KML_POINT ; makeOption <= MAKE_KML_LINE ; ++makeOption)
	{
		fseek(inFp, 0, SEEK_SET) ;
    	switch (makeOption)
		{
			case MAKE_KML_POINT :
				break ;
			case MAKE_KML_LINE :
				fprintf(outFp, KML_PLACEMARK) ;
				fprintf(outFp, KML_STYLE) ;
				fprintf(outFp, KML_LINE_STYLE) ;
				sprintf(temp, "%08X\0", DEFAULT_LINE_COLOR) ;
				WriteKmlLabel(outFp, KML_LINE_COLOR, temp) ;

				sprintf(temp, "%d\0", DEFAULT_LINE_WIDTH) ;
				WriteKmlLabel(outFp, KML_LINE_WIDTH, temp) ;

				fprintf(outFp, KML_LINE_STYLE_END) ;
				fprintf(outFp, KML_STYLE_END) ;

				fprintf(outFp, KML_LINE) ;
				fprintf(outFp, "<%s>\r\n", KML_COORDINATES) ;
				break ;
		} // switch

		switch (devType)
		{
			case DEVICE_TYPE_G05 :
			case DEVICE_TYPE_G05H :
			case DEVICE_TYPE_G07N :
			case DEVICE_TYPE_G07P :
			case DEVICE_TYPE_G08 :
			case DEVICE_TYPE_G08P :
				do
				{
					memset(&info.g05Series, 0x00, sizeof(LogInfoG05Series)) ;
					ReadLogInfoG05Series(inFp, &info.g05Series, devType) ;
					if (0xffff != info.g05Series.year)
						MakeKmlContent(idx++, makeOption, KML_CONVERT_G05SERIES, info, outFp) ;
				}   while (0xffff != info.g05Series.year) ;
				break ;
			case DEVICE_TYPE_G06MA :
			case DEVICE_TYPE_G06 :
				do
				{
					memset(&info.g06Series, 0x00, sizeof(LogInfoG06)) ;
					ReadLogInfoG06(inFp, &info.g06Series) ;
					if (0xffffffff != info.g06Series.utc)
						MakeKmlContent(idx++, makeOption, KML_CONVERT_G06SERIES, info, outFp) ;
				}   while (0xffffffff != info.g06Series.utc) ;
				break ;
			case DEVICE_TYPE_P51 :
				do
				{
					memset(&info.p51, 0x00, sizeof(LogInfoP51)) ;
					ReadLogInfoP51(inFp, &info.p51) ;
					if (0xffffffffffffffff != info.p51.utc)
						MakeKmlContent(idx++, makeOption, KML_CONVERT_P51, info, outFp) ;
				}   while (0xffffffffffffffff != info.p51.utc) ;
				break ;
		} // switch

		switch (makeOption)
		{
			case MAKE_KML_POINT :
				break ;
			case MAKE_KML_LINE :
				fprintf(outFp, "<\/%s>\r\n", KML_COORDINATES) ;
				fprintf(outFp, KML_LINE_END) ;
				fprintf(outFp, KML_PLACEMARK_END) ;
				break ;
        } // switch
	} // for

	MakeKmlEnd(outFp) ;
} // ConvertBinFile2KML

/**
*/
void MakeKmlTitle(FILE *fp)
{
	fprintf(fp, KML_TITLE_1) ;
	fprintf(fp, KML_TITLE_2) ;
	fprintf(fp, KML_DOCUMENT) ;
	WriteKmlLabel(fp, KML_NAME, "My Trips") ;
} // MakeKmlTitle

/**
*/
void MakeKmlEnd(FILE *fp)
{
	fprintf(fp, KML_DOCUMENT_END) ;
	fprintf(fp, KML_TITLE_END) ;
} // MakeKmlEnd

/**
*/
void WriteKmlLabel(FILE *fp, unsigned char *label, unsigned char *str)
{
	fprintf(fp, "<%s>%s<\/%s>\r\n", label, str, label) ;
} // WriteKmlName

/**
*/
void MakeKmlContent(int idx, int option, int type, KmlInfo info, FILE *fp)
{
	MakeContentFpt funAddrPoint[] = {MakeKmlContentDefault, MakeKmlContentG05Series, MakeKmlContentG06Series, MakeKmlContentP51} ;
	MakeLineContentFpt funAddrLine[] = {MakeKmlLineContentDefault, MakeKmlLineContentG05Series, MakeKmlLineContentG06Series, MakeKmlLineContentP51} ;

	switch (option)
	{
		case MAKE_KML_POINT :
			(*funAddrPoint[type])(idx, info, fp) ;
			break ;
		case MAKE_KML_LINE :
			(*funAddrLine[type])(idx, info, fp)	;
			break ;
    } // switch
} // MakeKmlContentDefault

/**
*/
void inline MakeKmlContentDefault(int idx, KmlInfo info, FILE *fp)
{
	unsigned char temp[MAX_PATH * 2] = {"\0"}, time[MAX_PATH] = {"\0"} ;

	fprintf(fp, KML_PLACEMARK) ;
	sprintf(temp, "%d, %d, %.6f, %.6f M\0", idx, info.def.fix, info.def.speed, info.def.dis) ;
	WriteKmlLabel(fp, KML_NAME, temp) ;

	fprintf(fp, KML_LOOK_AT) ;
	sprintf(temp, "%.6f\0", info.def.longitude) ;
	WriteKmlLabel(fp, KML_LONGTITUDE, temp) ;

	sprintf(temp, "%.6f\0", info.def.latitude) ;
	WriteKmlLabel(fp, KML_LATITUDE, temp) ;

	sprintf(temp, "%.6f\0", info.def.height) ;
	WriteKmlLabel(fp, KML_ATITUDE, temp) ;

	sprintf(temp, "%.6f\0", info.def.track) ;
	WriteKmlLabel(fp, KML_HEADING, temp) ;

	sprintf(temp, "%d\0", RANGE_VALUE) ;      // 視角高度
	WriteKmlLabel(fp, KML_RANGE, temp) ;

	sprintf(temp, "RelativeToGround\0") ;
	WriteKmlLabel(fp, KML_ALTITUDE_MODE, temp) ;
	fprintf(fp, KML_LOOK_AT_END) ;

	fprintf(fp, KML_POINT) ;
	sprintf(temp, "%.6f,%.6f,%.6f\0", info.def.longitude, info.def.latitude, info.def.height) ;
	WriteKmlLabel(fp, KML_COORDINATES, temp) ;
	fprintf(fp, KML_POINT_END) ;

	strftime(time, MAX_PATH, "%Y/%m/%d/%X", localtime(&info.def.utc)) ;
	sprintf(temp, "<![CDATA[INDEX: %d<br />LOCAL: %s<br />VALID: %d<br />LATITUDE: %.6f<br />LONGTITUDE: %.6f<br />HEIGHT: %.6f<br />SPEED : %.6f<br />HEADING: %.6f<br />]]>\0",
			idx, time, info.def.fix, info.def.latitude, info.def.longitude,info.def.height, info.def.speed, info.def.track) ;
	WriteKmlLabel(fp, KML_DESCRIPTION, temp) ;
	fprintf(fp, KML_PLACEMARK_END) ;
} // MakeKmlContentDefault

/**
*/
void inline MakeKmlContentG05Series(int idx, KmlInfo info, FILE *fp)
{
	unsigned char temp[MAX_PATH * 2] = {"\0"}, time[MAX_PATH] = {"\0"} ;

	fprintf(fp, KML_PLACEMARK) ;
	sprintf(time, "%d\/%d\/%d\/-%d\/%d\/%d\0", info.g05Series.year, info.g05Series.month, info.g05Series.day,
											 info.g05Series.hour, info.g05Series.minute, info.g05Series.second) ;
	sprintf(temp, "%d-%d %s\0\n", info.g05Series.holeNum, info.g05Series.holeNumSeq, time) ;
	WriteKmlLabel(fp, KML_NAME, temp) ;

	fprintf(fp, KML_LOOK_AT) ;
	sprintf(temp, "%.6f\0", info.g05Series.longtitude) ;
	WriteKmlLabel(fp, KML_LONGTITUDE, temp) ;

	sprintf(temp, "%.6f\0", info.g05Series.latitude) ;
	WriteKmlLabel(fp, KML_LATITUDE, temp) ;

	sprintf(temp, "%d\0", info.g05Series.hight) ;
	WriteKmlLabel(fp, KML_ATITUDE, temp) ;

	sprintf(temp, "%d\0", RANGE_VALUE) ;      // 視角高度
	WriteKmlLabel(fp, KML_RANGE, temp) ;

	sprintf(temp, "RelativeToGround\0") ;
	WriteKmlLabel(fp, KML_ALTITUDE_MODE, temp) ;
	fprintf(fp, KML_LOOK_AT_END) ;

	fprintf(fp, KML_POINT) ;
	sprintf(temp, "%.6f,%.6f,%d\0", info.g05Series.longtitude, info.g05Series.latitude, info.g05Series.hight) ;
	WriteKmlLabel(fp, KML_COORDINATES, temp) ;
	fprintf(fp, KML_POINT_END) ;

	sprintf(temp, "<![CDATA[INDEX: %d<br />LOCAL: %s<br />LATITUDE: %.6f<br />LONGTITUDE: %.6f<br />HEIGHT: %d<br />]]>\0",
			idx, time, info.g05Series.latitude, info.g05Series.longtitude, info.g05Series.hight) ;
	WriteKmlLabel(fp, KML_DESCRIPTION, temp) ;
	fprintf(fp, KML_PLACEMARK_END) ;
} // MakeKmlContentG05Series

/**
*/
void inline MakeKmlContentG06Series(int idx, KmlInfo info, FILE *fp)
{
	unsigned char temp[MAX_PATH * 2] = {"\0"}, time[MAX_PATH] = {"\0"} ;

	fprintf(fp, KML_PLACEMARK) ;
	strftime(time, MAX_PATH, "%Y/%m/%d/%X", localtime((time_t *)&info.g06Series.utc)) ;

	sprintf(temp, "%d-%d %s\0\n", info.g06Series.holeNum, info.g06Series.holeNumSeq, time) ;
	WriteKmlLabel(fp, KML_NAME, temp) ;

	fprintf(fp, KML_LOOK_AT) ;
	sprintf(temp, "%.6f\0", info.g06Series.longtitude) ;
	WriteKmlLabel(fp, KML_LONGTITUDE, temp) ;

	sprintf(temp, "%.6f\0", info.g06Series.latitude) ;
	WriteKmlLabel(fp, KML_LATITUDE, temp) ;

	sprintf(temp, "%d\0", RANGE_VALUE) ;      // 視角高度
	WriteKmlLabel(fp, KML_RANGE, temp) ;

	sprintf(temp, "RelativeToGround\0") ;
	WriteKmlLabel(fp, KML_ALTITUDE_MODE, temp) ;
	fprintf(fp, KML_LOOK_AT_END) ;

	fprintf(fp, KML_POINT) ;
	sprintf(temp, "%.6f,%.6f,0\0", info.g06Series.longtitude, info.g06Series.latitude) ;
	WriteKmlLabel(fp, KML_COORDINATES, temp) ;
	fprintf(fp, KML_POINT_END) ;

	sprintf(temp, "<![CDATA[INDEX: %d<br />LOCAL: %s<br />LATITUDE: %.6f<br />LONGTITUDE: %.6f<br />]]>\0",
			idx, time, info.g06Series.latitude, info.g06Series.longtitude) ;
	WriteKmlLabel(fp, KML_DESCRIPTION, temp) ;
	fprintf(fp, KML_PLACEMARK_END) ;
} // MakeKmlContentG06Series

/**
*/
void inline MakeKmlContentP51(int idx, KmlInfo info, FILE *fp)
{
	unsigned char temp[MAX_PATH * 2] = {"\0"}, time[MAX_PATH] = {"\0"} ;

	fprintf(fp, KML_PLACEMARK) ;
	strftime(time, MAX_PATH, "%Y/%m/%d/%X", localtime((time_t *)&info.p51.utc)) ;

	sprintf(temp, "%d. %s\0\n", idx, time) ;
	WriteKmlLabel(fp, KML_NAME, temp) ;

	fprintf(fp, KML_LOOK_AT) ;
	sprintf(temp, "%.6f\0", info.p51.longtitude) ;
	WriteKmlLabel(fp, KML_LONGTITUDE, temp) ;

	sprintf(temp, "%.6f\0", info.p51.latitude) ;
	WriteKmlLabel(fp, KML_LATITUDE, temp) ;

	sprintf(temp, "%d\0", RANGE_VALUE) ;      // 視角高度
	WriteKmlLabel(fp, KML_RANGE, temp) ;

	sprintf(temp, "RelativeToGround\0") ;
	WriteKmlLabel(fp, KML_ALTITUDE_MODE, temp) ;
	fprintf(fp, KML_LOOK_AT_END) ;

	fprintf(fp, KML_POINT) ;
	sprintf(temp, "%.6f,%.6f,%.6f\0", info.p51.longtitude, info.p51.latitude, info.p51.height) ;
	WriteKmlLabel(fp, KML_COORDINATES, temp) ;
	fprintf(fp, KML_POINT_END) ;

	sprintf(temp, "<![CDATA[INDEX: %d<br />LOCAL: %s<br />LATITUDE: %.6f<br />LONGTITUDE: %.6f<br />HEIGHT: %.6f<br />SPEED : %.6f<br />]]>\0",
			idx, time, info.p51.latitude, info.p51.longtitude, info.p51.height, info.p51.speed) ;
	WriteKmlLabel(fp, KML_DESCRIPTION, temp) ;
	fprintf(fp, KML_PLACEMARK_END) ;
} // MakeKmlContentp51

/**
*/
void inline MakeKmlLineContentDefault(int idx, KmlInfo info, FILE *fp)
{
	fprintf(fp, "%.6f,%.6f,%.6f\r\n", info.def.longitude, info.def.latitude, info.def.height) ;
} // MakeKmlLineContentDefault

/**
*/
void inline MakeKmlLineContentG05Series(int idx, KmlInfo info, FILE *fp)
{
	fprintf(fp, "%.6f,%.6f,%.6f\r\n", info.g05Series.longtitude, info.g05Series.latitude, info.g05Series.hight) ;
} // MakeKmlLineContentDefault

/**
*/
void inline MakeKmlLineContentG06Series(int idx, KmlInfo info, FILE *fp)
{
	fprintf(fp, "%.6f,%.6f,0\r\n", info.g06Series.longtitude, info.g06Series.latitude) ;
} // MakeKmlLineContentDefault

/**
*/
void inline MakeKmlLineContentP51(int idx, KmlInfo info, FILE *fp)
{
	fprintf(fp, "%.6f,%.6f,%.6f\r\n", info.p51.longtitude, info.p51.latitude, info.p51.height) ;
} // MakeKmlLineContentP51


