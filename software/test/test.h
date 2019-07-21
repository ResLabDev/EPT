#ifndef TEST_H_
#define TEST_H_

#include <stdio.h>
#include "../driver/driver.h"
#include "../common/common.h"

#define EPT_CTR_OVF			0		// EPT Counter overflow parameter

void systemTest(void);

// Timer Tests
int testSystemTimer(void);

// EPT Tests
int testEptRam(unsigned int pattern, int displayData);
int testEptCounter(unsigned int overflow);

#endif	// TEST_H_
