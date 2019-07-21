//===============================================
// EPT Initialization Layer Function Collection
//===============================================

#include <math.h>
#include "init.h"

//-----------------------------------------
// Function Prototypes with Internal Access
//-----------------------------------------
static stat_t stdevCalc(int *sample, int sampleNum);	// Standard deviation calculation

//-----------------------------------------
// Initialization Function Collection
//-----------------------------------------

// Fills the address interval of the on-chip RAM with the input data
status_t ramInit(int addressStart, int addressStop, unsigned int data)
{
	status_t status = {NO_ERROR, "SUCCESS"};
	int i;
	alt_u32 *ram = (alt_u32 *)DRV_EPT_RAM_PTR;			// Set RAM to starting address

	// Validate the input address interval
	if ((addressStart > addressStop) || (addressStop > EPT_RAM_ADDRESS_MAX))
	{
		status.type = INVALID_ADDRESS;
		strcpy(status.description, "FAIL - Invalid input address");
		return status;
	}
	// Fill RAM with the data
	for (i=0; i<=addressStop; i++)
	{
		if (i >= addressStart)
		{
			*ram = (alt_u32)data;						// If i reaches or beyond the starting interval, fill the RAM with the data
			if (data != (unsigned int)*ram)				// Validate the written data
			{
				status.type = RAM_ACCESS;
				strcpy(status.description, "FAIL - RAM data mismatch");
				return status;
			}
		}
		ram++;		// Increment address
	}

	return status;
}

// I/O START-STOP offset validation for each task IDs
ioOffset_t ioOffsetCalibration(int numberOfTasks)
{
	ioOffset_t ioOffset = {{0, 0, 0}, {NO_ERROR, "SUCCESS"}};
	status_t status = {NO_ERROR, "SUCCESS."};
	alt_u32 *taskPtr = (alt_u32 *)DRV_EPT_TASK_PTR;
	alt_u32 *ramPtr = (alt_u32 *)DRV_EPT_RAM_PTR;
	alt_u32 *ramStartPtr = (alt_u32 *)DRV_EPT_RAM_PTR;
	alt_u8 taskIdOn = 0x80;
	alt_u8 taskIdOff = 0;
	int taskResult[EPT_RAM_ADDRESS_MAX] = {0};
	int i;

// --- 1. RAM initialization ---
#if DEBUG_INIT
	printf("  -> RamInit...\n");
#endif		// DEBUG_INIT

	DRV_EPT_RESET;								// Module Reset
	if (!DRV_EPT_STATUS_GET)					// Check module status
	{
		ioOffset.status.type = EPT_STATUS;
		strcpy(ioOffset.status.description, "FAIL - ETP module is not ready");
		return ioOffset;
	}
	status = ramInit(0, EPT_RAM_ADDRESS_MAX, 0);
	if (status.type)		// Get status from RAM initialization
	{
		ioOffset.status.type = RAM_ACCESS;
		strcpy(ioOffset.status.description, "FAIL - RAM initialization is failed");
		return ioOffset;						// Initialization failed
	}

// --- 2. Run TaskID IO offset measurement ---
	// Validate Number of Tasks Input
#if DEBUG_INIT
	printf("  -> Run Task...\n");
#endif		// DEBUG_INIT
	// Validate maximum number of tasks
	if (numberOfTasks > TASK_ID_MAX)
	{
		ioOffset.status.type = INVALID_DATA;
		strcpy(ioOffset.status.description, "FAIL - Number of tasks is out of the limit");
		return ioOffset;
	}
	// Measure IO overhead for each task
	DRV_EPT_START;
	if (DRV_EPT_STATUS_GET)						// Check module status
	{
		ioOffset.status.type = EPT_STATUS;
		strcpy(ioOffset.status.description, "FAIL - ETP module is not ready");
		return ioOffset;
	}
	i = numberOfTasks;
SetTask:
	*taskPtr = taskIdOn;						// Start current task
	*taskPtr = taskIdOff;						// Stop current task
	taskIdOn++;
	taskIdOff++;
	i--;
	if (i)
	{
		goto SetTask;
	}

// --- 3. Read all task data from RAM ---
#if DEBUG_INIT
	printf("  -> Read All Task from RAM...\n");
#endif		// DEBUG_INIT

	DRV_EPT_STOP;							// RAM is accessible only at module ready status
	if (!DRV_EPT_STATUS_GET)				// Check module status
	{
		ioOffset.status.type = EPT_STATUS;
		strcpy(ioOffset.status.description, "FAIL - ETP module is not ready");
		return ioOffset;
	}
	ramPtr = ramStartPtr;	// Reinitialize RAM pointer
	// Accessing the Task results
	for (i=0; i<numberOfTasks; i++)
	{
		taskResult[i] = (int)*ramPtr;
		ramPtr++;
	}

// --- 4. Evaluating the obtained data ---
#if DEBUG_INIT
	printf("  -> Calculating Statistic: %d - %d\n", taskResult[0], taskResult[1]);
#endif		// DEBUG_INIT
	// Calculating the result
	ioOffset.result = stdevCalc(taskResult, numberOfTasks);

// --- 5. Sending calibration result to EPT ---
	DRV_EPT_IOOF_SET((alt_u8) ioOffset.result.mean);
	// Validating I/O offset data
	if (((alt_u8) ioOffset.result.mean) != DRV_EPT_IOOF_GET)
	{
		ioOffset.status.type = INVALID_DATA;
		strcpy(ioOffset.status.description, "Unable to set EPT I/O offset");
	}

	return ioOffset;
}

// === Functions with Internal Access ===
// Standard deviation calculation
static stat_t stdevCalc(int *sample, int sampleNum)
{
    stat_t result;
    float partRes = 0;
    int i, sum = 0;

    result.N = (unsigned int)sampleNum;
    // Calculate the Mean
    for (i=0; i<sampleNum; i++)
    {
        sum += sample[i];
    }
    result.mean = (float)sum / (float)sampleNum;

    // Calculate the standard deviation
    for (i=0; i<sampleNum; i++)
    {
        partRes += ((float)sample[i]-result.mean) * ((float)sample[i]-result.mean);
    }
    result.stdev = sqrt( 1/(float)(result.N-1) * partRes );

    return result;
}
