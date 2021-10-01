//---------------------------------------------------------------------------

#ifndef MakeKMLH
#define MakeKMLH

#include "MTKPackage.h"
#include "TSIPackage.h"
#include "ControlG05.h"
#include "ControlG06MA.h"
#include "ControlG06.h"
#include "ControlP51.h"
#include "Common.h"

#define RANGE_VALUE             1000
#define AC_BASE_YEAR            1900
#define DEFAULT_LINE_COLOR      0xff00ffff
#define DEFAULT_LINE_WIDTH      2

//---------------------------------------------------------------------------
#define KML_TITLE_1  			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
#define KML_TITLE_2  			"<kml xmlns=\"http:\/\/earth.google.com\/kml\/2.0\">\r\n"
#define KML_TITLE_3  			"<Style id=\"PointColorIcon\"><IconStyle><Icon><href>http://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href></Icon></IconStyle></Style>\r\n"
#define KML_TITLE_4  			"<Style id=\"SeqColorIcon\"><IconStyle><Icon><href>http://maps.google.com/mapfiles/kml/shapes/placemark_square.png</href></Icon></IconStyle></Style>\r\n"
#define KML_TITLE_END  			"</kml>\r\n"
#define KML_DOCUMENT  			"<Document id=\"MadeByTSI\">\r\n"
#define KML_DOCUMENT_END  		"</Document>\r\n"
#define KML_NAME  				"name"
#define KML_LONGTITUDE          "longitude"
#define KML_LATITUDE            "latitude"
#define KML_ATITUDE             "atitude"
#define KML_RANGE               "range"
#define KML_HEADING             "heading"
#define KML_ALTITUDE_MODE       "altitudeMode"
#define KML_FOLDER  			"<Folder>\r\n"
#define KML_FOLDER_END  		"</Folder>\r\n\n"
#define KML_PLACEMARK  			"<Placemark>\r\n"
#define KML_PLACEMARK_END  		"</Placemark>\r\n\n"
#define KML_LOOK_AT             "<LookAt>\r\n"
#define KML_LOOK_AT_END         "</LookAt>\r\n\n"
#define KML_POINT_STYLE  		"<styleUrl>#PointColorIcon</styleUrl>\r\n"
#define KML_POINT  				"<Point>\r\n"
#define KML_POINT_END  			"</Point>\r\n"
#define KML_COORDINATES 		"coordinates"
#define KML_DESCRIPTION  		"description"
#define KML_LINE  				"<LineString>\r\n"
#define KML_LINE_END  			"</LineString>\r\n"
#define KML_STYLE               "<Style>\r\n"
#define KML_STYLE_END           "</Style>\r\n"
#define KML_LINE_STYLE          "<LineStyle>\r\n"
#define KML_LINE_STYLE_END      "</LineStyle>\r\n"
#define	KML_LINE_COLOR			"color"
#define	KML_LINE_WIDTH          "width"

#define CSV_TITLE_G05SERIES     "UTC,Latitude,Longitude,Course ID,Hole No.,Seq.\n"
#define CSV_TITLE_G06SERIES  	"UTC,Latitude,Longitude,Valid,Course ID,Hole No.,Seq.,Strokes\n"
#define CSV_TITLE_P51           "UTC,Valid,Latitude,Longitude,Speed,Height,Heading,Barometer,Gx,Gy,Gz,Flag\n"

//---------------------------------------------------------------------------
enum ConvertOption
{
	CONVERT_CSV,
	CONVERT_KML
} ;

enum KmlContentOption
{
	MAKE_KML_POINT,
	MAKE_KML_LINE
} ;

enum KmlConvertType
{
	KML_CONVERT_DEFAULT,
	KML_CONVERT_G05SERIES,
	KML_CONVERT_G06SERIES,
	KML_CONVERT_P51
} ;

typedef union KmlInfo
{
	MTKNormalLog def ;
	LogInfoG05Series g05Series ;
	LogInfoG06 g06Series ;
	LogInfoP51 p51 ;
} KmlInfo ;

typedef void (*MakeContentFpt)(int, KmlInfo, FILE *) ;
typedef void (*MakeLineContentFpt)(int, KmlInfo, FILE *) ;

int ConvertBinFile(int option, unsigned char *inFileName, unsigned char *outFileName, int devType) ;
void inline ConvertBinFile2CSV(int devType, FILE *inFp, FILE *outFp) ;
void inline ConvertBinFile2KML(int devType, FILE *inFp, FILE *outFp) ;
void MakeKmlTitle(FILE *fp) ;
void MakeKmlEnd(FILE *fp) ;
void WriteKmlLabel(FILE *fp, unsigned char *label, unsigned char *str) ;
void MakeKmlContent(int idx, int option, int type, KmlInfo info, FILE *fp) ;
void inline MakeKmlContentDefault(int idx, KmlInfo info, FILE *fp) ;
void inline MakeKmlContentG05Series(int idx, KmlInfo info, FILE *fp) ;
void inline MakeKmlContentG06Series(int idx, KmlInfo info, FILE *fp) ;
void inline MakeKmlContentP51(int idx, KmlInfo info, FILE *fp) ;
void inline MakeKmlLineContentDefault(int idx, KmlInfo info, FILE *fp) ;
void inline MakeKmlLineContentG05Series(int idx, KmlInfo info, FILE *fp) ;
void inline MakeKmlLineContentG06Series(int idx, KmlInfo info, FILE *fp) ;
void inline MakeKmlLineContentP51(int idx, KmlInfo info, FILE *fp) ;

#endif
