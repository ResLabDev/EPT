//==========================
// Common Layer Header File
//==========================

#ifndef _COMMON_H_
#define _COMMON_H_

#include <string.h>

//-------------------
// Status Response
//-------------------
#define STATUS_DESCR_SIZE 	100		// Maximum character of the error description

typedef enum errorType
{
	NO_ERROR,
	RAM_ACCESS,
	EPT_STATUS,
	INVALID_ADDRESS,
	INVALID_DATA
} errorType_t;

typedef struct status
{
	errorType_t type;
	char description[STATUS_DESCR_SIZE];
} status_t;

//-------------------
// DEBUG
//-------------------
#define DEBUG_INIT		0

#endif			// _COMMON_H_
