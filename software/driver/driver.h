//========================================
// Driver Abstraction Layer
//========================================
#ifndef DRIVER_H_
#define DRIVER_H_

#include "system.h"
#include "timerir.h"
#include "ept.h"
#include "gpio.h"

// Constant Definitions
#define BYTE_TO_QWORD_CONVERT(data)			(((alt_u64)data & BYTE_MASK))
#define WORD_TO_QWORD_CONVERT(data)			(((alt_u64)data & WORD_MASK))
#define SYSTEM_CLOCK						50000000LL									// 50 MHz clock cycle
#define IR_TIMING_PARAM						4											// Interrupt timing parameters
#define TASK_ID_MAX							(EPT_RAM_ADDRESS_MAX+1 - IR_TIMING_PARAM)	// Maximum number of TASK ID

//------------------------
// System Timer
//------------------------
#define DRV_TMRSYS_PTR						TMRIR_REG_PTR(TIMER_IR_BASE)								// Pointer to system timer registers
#define DRV_TMRSYS_CCTR_GET					TMRIR_READ_CCTR(TIMER_IR_BASE)								// Get capture control register
#define DRV_TMRSYS_RESET					TMRIR_WRITE_DATA(TIMER_IR_BASE, 0)							// Reset timer
#define DRV_TMRSYS_DISABLE					TMRIR_WRITE_CCTR(TIMER_IR_BASE, TMRIR_CCTR_CMD_DISABLE)		// Disable timer
#define DRV_TMRSYS_FREERUN					TMRIR_WRITE_CCTR(TIMER_IR_BASE, TMRIR_CCTR_CMD_FREERUN)		// FreeRun mode
#define DRV_TMRSYS_MTCNRES					TMRIR_WRITE_CCTR(TIMER_IR_BASE, TMRIR_CCTR_CMD_MTCNRES)		// Generate IRQ at match without reset mode
#define DRV_TMRSYS_MATCRES					TMRIR_WRITE_CCTR(TIMER_IR_BASE, TMRIR_CCTR_CMD_MATCRES)		// Generate IRQ at match with reset mode
#define DRV_TMRSYS_MATC_GET					TMRIR_READ_MATC(TIMER_IR_BASE)								// Get match register
#define DRV_TMRSYS_MATC_SET(data)			TMRIR_WRITE_MATC(TIMER_IR_BASE, data)						// Set match register
#define DRV_TMRSYS_DATA_GET					TMRIR_READ_DATA(TIMER_IR_BASE)								// Get system timer
#define DRV_TMRSYS_DATA_SET(data)			TMRIR_WRITE_DATA(TIMER_IR_BASE, data)						// Timer register data write
#define DRV_TMRSYS_IRQ_GET					TMRIR_READ_IRQ(TIMER_IR_BASE)								// Get IRQ register
#define DRV_TMRSYS_IRQ_CLR					TMRIR_WRITE_IRQ(TIMER_IR_BASE, 0)							// Clear IRQ register
#define DRV_TMRSYS_IRQ_SET(data)			TMRIR_WRITE_IRQ(TIMER_IR_BASE, data)						// Set IRQ register

//-------------------------------
// Execution Performance Tester
//-------------------------------
#define DRV_EPT_RAM_SET(address, data)		EPT_WRITE_RAM(EPT_BASE, address, data)		// Set onchip RAM data
#define DRV_EPT_RAM_GET(address)			EPT_READ_RAM(EPT_BASE, address)				// Get onchip RAM data
#define DRV_EPT_CTR_LO_GET 					EPT_READ_CTR_LO(EPT_BASE)					// Get Counter Low
#define DRV_EPT_CTR_HI_GET 					EPT_READ_CTR_HI(EPT_BASE)					// Get Counter High
#define DRV_EPT_STATUS_GET 					EPT_READ_STATUS(EPT_BASE)					// Get IsReady Status
#define DRV_EPT_START_SET(data)				EPT_WRITE_START(EPT_BASE, data)				// Set Start register
#define DRV_EPT_STOP_SET(data)				EPT_WRITE_STOP(EPT_BASE, data)				// Set Stop register
#define DRV_EPT_TASK_SET(data)				EPT_WRITE_TASK(EPT_BASE, data)				// Set Task ID
#define DRV_EPT_TASK_GET					EPT_READ_TASK(EPT_BASE)						// Get Task ID
#define DRV_EPT_IOOF_SET(data)				EPT_WRITE_IOOF(EPT_BASE, data)				// Set IO offset
#define DRV_EPT_IOOF_GET					EPT_READ_IOOF(EPT_BASE)						// Get IO offset
#define DRV_EPT_ISR_SET(data)				EPT_WRITE_ISR(EPT_BASE, data)				// Set Interrupt Service Routine trigger
#define DRV_EPT_CTXSAV_SET(data)			EPT_WRITE_CTX_SAVE(EPT_BASE, data)			// Set Context Saving trigger
#define DRV_EPT_CTXRES_SET(data)			EPT_WRITE_CTX_REST(EPT_BASE, data)			// Set Context Restoring trigger
#define DRV_EPT_EXEC_GET					EPT_READ_EXEC(EPT_BASE)						// Get Executed
#define DRV_EPT_RESET_SET(data)				EPT_WRITE_RESET(EPT_BASE, data)				// Set Reset

// Direct Memory Mapped Access
#define DRV_EPT_RAM_PTR						EPT_RAM_PTR(EPT_BASE, (SYSTEM_BUS_WIDTH / 8))						// RAM address pointer
#define DRV_EPT_RAM_IR_PTR					EPT_RAM_IR_PTR(EPT_BASE, (SYSTEM_BUS_WIDTH / 8))					// Pointer to Interrupt Timing data in the RAM
#define DRV_EPT_CTR_PTR						EPT_CTR_PTR(EPT_BASE, (SYSTEM_BUS_WIDTH / 8))						// Counter address pointer
#define DRV_EPT_TASK_PTR					EPT_TASK_PTR(EPT_BASE, (SYSTEM_BUS_WIDTH / 8))						// Task ID address pointer
#define DRV_EPT_ISR_PTR						EPT_ISR_PTR(EPT_BASE, (SYSTEM_BUS_WIDTH / 8))						// ISR address pointer
#define DRV_EPT_CTXSAV_PTR					EPT_CTX_SAVE_PTR(EPT_BASE, (SYSTEM_BUS_WIDTH / 8))					// Context Save address pointer
#define DRV_EPT_CTXRES_PTR					EPT_CTX_RESTORE_PTR(EPT_BASE, (SYSTEM_BUS_WIDTH / 8))				// Context Restore address pointer

// Command macros
#define DRV_EPT_START						{\
												DRV_EPT_START_SET(1);\
												DRV_EPT_START_SET(0);\
											}													// Start trigger command
#define DRV_EPT_STOP						{\
												DRV_EPT_STOP_SET(1);\
												DRV_EPT_STOP_SET(0);\
											}													// Stop trigger command
#define DRV_EPT_RESET						{\
												DRV_EPT_RESET_SET(1);\
												DRV_EPT_RESET_SET(0);\
											}

// Function Prototypes
alt_u64 eptCounterConcat(eptCounter_t *eptCounter);			// Concatenate Execution Performance Cycle Counter
double elapsedTimeMillisec(alt_u64 elapsedCycle);		// Calculate elapsed time in milliseconds


#endif	// DRIVER_H_
