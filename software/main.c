//============================
// Task Watcher System V0
//============================

/* @Brief:
 * 	- Root Path: d:\Development\ExecutionPerformance\TaskWatcher\SystemV0\
 */

#include "main.h"


int main()
{
	ioOffset_t offset;
	status_t status;

	printf("\n === EPT SYSTEM ===\n\n");

	//------------------
	// Tests
	//------------------
	printf("--- Driver Tests ---\n");
	systemTest();
	printf("\n");

	//------------------
	// Initialization
	//------------------
	// I/O offset calibration
	printf("--- Initialization ---\n");
	offset = ioOffsetCalibration(TASK_ID_MAX);
	printf(" >> IO Offset Calibration: %s -> N: %d, Mean: %.2lf, StDev: %.2lf\n", offset.status.description, offset.result.N, offset.result.mean, offset.result.stdev);
	// RAM initialization
	status = ramInit(0, EPT_RAM_ADDRESS_MAX, 0);
	printf(" >> EPT RAM initialization to 0: %s\n", status.description);

	return 0;
}
