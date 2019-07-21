//==========================================
// Function Collection of the driver layer
//==========================================

#include "driver.h"

// Concatenate Execution Performance Cycle Counter
alt_u64 eptCounterConcat(eptCounter_t *eptCounter)
{

	return ((BYTE_TO_QWORD_CONVERT(eptCounter->High)) << 32) | (WORD_TO_QWORD_CONVERT(eptCounter->Low));
}

// Calculate elapsed time in milliseconds
double elapsedTimeMillisec(alt_u64 elapsedCycle)
{
	return (((double)elapsedCycle / (double)SYSTEM_CLOCK) * 1000);
}
