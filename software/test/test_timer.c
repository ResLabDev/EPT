//---------------------------------
// Timer Test function collection
//---------------------------------

#include "test.h"

// 4 steps: Test system timer modes
int testSystemTimer(void)
{
	alt_u32 match = 0xffff;
	alt_u32 elapsedTime;
	alt_u32 i;
	int step = 1;
	int result = 0;
	timerIR_t *timer = (timerIR_t *)DRV_TMRSYS_PTR;

	printf("System timer Test:\n");

	// 1. Reset-Disable function
	/*
	DRV_TMRSYS_FREERUN;
	DRV_TMRSYS_RESET;
	DRV_TMRSYS_DISABLE;
	*/
	timer->ccr = TMRIR_CCTR_CMD_FREERUN;	// Freerun to be reseted
	timer->data = 0;						// Reset
	timer->ccr = TMRIR_CCTR_CMD_DISABLE;	// Disable

	elapsedTime = timer->data;
	elapsedTime = timer->data - elapsedTime;
	if (!elapsedTime)
	{
		result++;
		printf("%d. SystemTimer Reset-Disable (CCR %u): PASS - %u\n", step, (unsigned int)timer->ccr, (unsigned int)elapsedTime);
	}
	else
	{
		result--;
		printf("%d. SystemTimer Reset-Disable (CCR %u): FAIL - %u\n", step, (unsigned int)timer->ccr, (unsigned int)elapsedTime);
	}

	// 2. Start-Stop
	step++;
	timer->ccr = TMRIR_CCTR_CMD_FREERUN;	// Freerun to be reseted
	timer->data = 0;						// Reset counter
	elapsedTime = timer->data;
	elapsedTime = timer->data - elapsedTime;
	if (elapsedTime)
	{
		if (result > 0)
		{
			result++;
		}
		printf("%d. SystemTimer Start-Stop (CCR %u): PASS - %u\n", step, (unsigned int)timer->ccr, (unsigned int)elapsedTime);
	}
	else
	{
		result--;
		printf("%d. SystemTimer Start-Stop (CCR %u): FAIL - %u\n", step, (unsigned int)timer->ccr, (unsigned int)elapsedTime);
	}

	// 3. IRQ generation without reset
	step++;
	elapsedTime = 0;
	timer->irq = 0;							// Clear IRQ register
	timer->match = match;					// Set matching value
	timer->data = 0;						// Reset counter
	timer->ccr = TMRIR_CCTR_CMD_MTCNRES;	// IRQ but no reset
	for (i=0; i<match; i++)
	{
		if (timer->irq)
		{
			elapsedTime = timer->data;
			break;
		}
	}
	if ((timer->irq) && (elapsedTime > match))
	{
		if (result > 0) result++;
		printf("%d. SystemTimer IRQ without RESET (CCR %u): PASS - %u > %u\n",
				step, (unsigned int)timer->ccr, (unsigned int)elapsedTime, (unsigned int)timer->match);
	}
	else
	{
		result--;
		printf("%d. SystemTimer IRQ without RESET (CCR %u): FAIL - %u > %u\n",
				step, (unsigned int)timer->ccr, (unsigned int)elapsedTime, (unsigned int)timer->match);
	}

	// 4. IRQ generation with reset
	step++;
	elapsedTime = 0xff;
	timer->irq = 0;							// Clear IRQ register
	timer->match = match;					// Set matching value
	timer->data = 0;						// Reset counter
	timer->ccr = TMRIR_CCTR_CMD_MATCRES;	// IRQ with reset
	for (i=0; i<match; i++)
	{
		if (timer->irq)
		{
			elapsedTime = timer->data;
			break;
		}
	}
	if ((timer->irq) && (elapsedTime < match))
	{
		if (result > 0) result++;
		printf("%d. SystemTimer IRQ with RESET (CCR %u): PASS - %u > %u\n",
						step, (unsigned int)timer->ccr, (unsigned int)elapsedTime, (unsigned int)timer->match);
	}
	else
	{
		result--;
		printf("%d. SystemTimer IRQ with RESET (CCR %u): FAIL - %u > %u\n",
						step, (unsigned int)timer->ccr, (unsigned int)elapsedTime, (unsigned int)timer->match);
	}

	return result;
}
