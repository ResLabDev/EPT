//===========================
// System Tests Functions
//===========================

#include "test.h"

void systemTest(void)
{
	int result;

	// --- System Timer Test ---
	if ((result = testSystemTimer()) > 0) printf("...PASS\n");
		else printf("...%d item(s) FAIL.\n", (-1*result));

	DRV_EPT_STOP;														// Stop EPT for Accessing RAM
	// --- Onchip RAM Test ---
	printf("---\n");
	if (!(result = testEptRam(0xffffff00, 0))) printf("...PASS\n");
		else printf("...%d item(s) FAIL.\n", (-1*result));

	// --- EPT Cycle Counter Test ---
	printf("---\n");
	if (!testEptCounter(EPT_CTR_OVF)) printf("...PASS\n");
			else printf("...FAIL.\n");
}
