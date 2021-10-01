#ifndef __PKT_STRUCT_H__
#define __PKT_STRUCT_H__

#include <windows.h>
#include <stdio.h>

#include "NMEAPackage.h"
#include "TSIPackage.h"
#include "NMEAPackage.h"
#include "FinalTest.h"

/* test1 */
/* test5 */

typedef struct AckBaseStatus
{
	unsigned int connect ;
} AckBaseStatus ;

typedef struct AckExtraStatus
{
	MTKAckStatus mtk ;
	TSIAckStatus tsi ;
	NmeaInfo nmea ;
} AckExtraStatus ;

typedef struct AckStatus
{
	AckBaseStatus base ;
    AckExtraStatus extra ;
} AckStatus ;

typedef struct AckInfo
{
	AckStatus ackStatus ;
	FinalTestResult fTest ;
	FILE *dwLogFp ;
} AckInfo ;

#endif
