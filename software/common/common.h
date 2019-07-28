//==========================
// Common Layer Header File
//==========================

#ifndef _COMMON_H_
#define _COMMON_H_

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
// Function Prototypes
//-------------------
void stringCopy (char *target, char *source);


#endif			// _COMMON_H_
