//--------------------------------------------------------
// Execution Performance Tester Test function collection
//--------------------------------------------------------

#include "test.h"

// Memory test
int testEptRam(unsigned int pattern, int displayData)
{
	int fail = 0;
	unsigned int i;
	alt_u32 setPattern = (alt_u32)pattern;
	alt_u32 data;
	alt_u32 *ramPtr = (alt_u32 *)DRV_EPT_RAM_PTR;		// Pointer to On-chip RAM absolute address


	printf("On-Chip RAM test (0 - %x) using '0x%x' pattern.\n", (unsigned int)EPT_RAM_ADDRESS_MAX, pattern);
	for (i=0; i<=EPT_RAM_ADDRESS_MAX; i++)
	{
		*ramPtr = setPattern;					// Set RAM pattern
		data = *ramPtr;							// Get RAM Pattern
		// Check data
		if ( data == setPattern)
		{
			// printf("%d. PASS: %x, ", i, (unsigned int)data);
		}
		else
		{
			fail++;
			printf("%d. FAIL: %x, ", i, (unsigned int)data);
		}
		setPattern++;
		ramPtr++;
	}

	// Display Interrupt timing parameters
	eptIR_t *irTiming = (eptIR_t * )DRV_EPT_RAM_IR_PTR;

	printf("  - Interrupt latency: 0x%x\n"
		   "  - Context Save: 0x%x\n"
		   "  - ISR handle: 0x%x\n"
		   "  - Context Restore: 0x%x\n",
		   (unsigned int)irTiming->irLatency,
		   (unsigned int)irTiming->ctxSave,
		   (unsigned int)irTiming->isrHandle,
		   (unsigned int)irTiming->ctxRestore);

	if (displayData)
	{
		printf("\n3. --- Reading memory contents ---\n");
		for (i=0; i<=EPT_RAM_ADDRESS_MAX; i++)
		{
			data = DRV_EPT_RAM_GET(i);
			if(!(i % 8))
			{
				printf("\n %d. ", i);
			}
			printf("0x%x, ", (unsigned int)data);
		}
	}

	return (-1*fail);
}

// Counter LO-HI test
int testEptCounter(unsigned int overflow)
{
	int step = 1;
	char subStep = 'a';
	alt_u32 counterTemp;
	eptCounter_t *counterPtr = (eptCounter_t *)DRV_EPT_CTR_PTR;
	eptCounter_t counter;

	printf("EPT Cycle Counter Test:\n");

	// --- 1. In Ready state the counter should be disabled
	//		  @Compare each counter registers in different times
	if(DRV_EPT_STATUS_GET)
	{
		counterTemp = counterPtr->Low;
		counter.Low = counterPtr->Low - counterTemp;
		counterTemp = counterPtr->High;
		counter.High = counterPtr->High - counterTemp;
		if (!(counter.Low - counter.High))
		{
			printf("%d. PASS: EPT Cycle counter is disabled in ready state.\n", step++);
		}
		else
		{
			printf("%d. FAIL: EPT Cycle counter is running in ready state: %u - %u\n", step++, (unsigned int)counter.High, (unsigned int)counter.Low);
			return -1;
		}
	}
	else
	{
		printf("FAIL: EPT status is not ready.");
		return -1;
	}

	// --- 2. In Active state the counter should be reseted and in freerun mode
	//		  @Compare each counter registers in different times
	DRV_EPT_START;																	// Start EPT
	counterTemp = counterPtr->Low;
	counter.Low = counterPtr->Low - counterTemp;
	counter.High = counterPtr->High;
	if (counter.Low && !counter.High)
	{
		printf("%d. PASS: Cycle counter is reseted and running at start: %u(HI) - %u(LO)\n", step++, (unsigned int)counter.High, (unsigned int)counter.Low);
	}
	else
	{
		printf("%d. FAIL: Cycle counter is not running at start: %u(HI) - %u(LO)\n", step++, (unsigned int)counter.High, (unsigned int)counter.Low);
		return -1;
	}

	// --- 3. Counter owerflow test
	//		  @Tests the counter HIGH, concatenation and millisecond conversion --> LONG TEST!
	alt_u64 elapsedCycle;

	if (overflow)
	{
		printf("%d. Counter overflow test, estimated duration: %u sec\n", step, (unsigned int)(overflow*(WORD_MASK/SYSTEM_CLOCK)));
		// Checking counter overflow
		step = 1;
		while (overflow)
		{
			// Detect overflow
			if (0x400 > counterPtr->Low)
			{
				elapsedCycle = eptCounterConcat(counterPtr);
				printf("  - %c. overflow: 0x%llx\n", subStep, (unsigned long long)elapsedCycle);
				overflow--;
				subStep++;
			}
		}
		printf("  - Elapsed time: %.3lf msec\n", elapsedTimeMillisec(elapsedCycle));

	}

	DRV_EPT_STOP;											// Stop EPT

	return 0;
}





