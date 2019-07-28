//========================================
// EPT Initialization Layer Header
//========================================

#ifndef _INIT_H_
#define _INIT_H_

#include "../driver/driver.h"
#include "../common/common.h"

#if DEBUG_INIT
	#include <stdio.h>
#endif 			// DEBUG_INIT

//---------------------
// Constant Definitions
//---------------------

//---------------------
// Type Definitions
//---------------------

// Statistic for standard deviation
typedef struct stat
{
    unsigned int N;
    float mean;
    float stdev;
} stat_t;

// IO Offset Type
typedef struct ioOffset
{
	stat_t result;
	status_t status;
} ioOffset_t;

//---------------------
// Function Prototypes
//---------------------
status_t ramInit(int addressStart, int addressStop, unsigned int data);		// Fills the address interval of the on-chip RAM with the input data
ioOffset_t ioOffsetCalibration(int numberOfTasks);							// I/O START-STOP offset validation for each task IDs


#endif			// _INIT_H_
