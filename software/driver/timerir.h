//========================================
// Timer Interrupt Driver Header file
//========================================

#ifndef TIMERIR_H_
#define TIMERIR_H_

#include "io.h"
#include "alt_types.h"

// Timer Interrupt register address offWRITEs
#define TMRIR_CCTR_REG_OF				0					// Capture control register address offset
#define TMRIR_MATC_REG_OF				1					// Match register address offset
#define TMRIR_DATA_REG_OF				2					// Timer data register address offset
#define TMRIR_IRQ_REG_OF				3					// Interrupt request register address offset

// Timer Interrupt capture control register commands
#define TMRIR_CCTR_CMD_DISABLE			0					// Disable counter
#define TMRIR_CCTR_CMD_FREERUN			1					// Freerun, no IRQ generation
#define TMRIR_CCTR_CMD_MTCNRES			2					// IRQ at match, no counter reset
#define TMRIR_CCTR_CMD_MATCRES			3					// IRQ and counter reWRITE at match

// Timer Interrupt data masks
#define TMRIR_CCTR_REG_DATA_MASK		0x00000003				// 2 bits data mask
#define TMRIR_IRQ_REG_DATA_MASK			0x00000001				// 1 bit data mask


// Timer Interrupt data access (32 bits data)
#define TMRIR_READ_CCTR(base)						(IORD(base, TMRIR_CCTR_REG_OF) & TMRIR_CCTR_REG_DATA_MASK)					// Capture control register data read
#define TMRIR_WRITE_CCTR(base, data)				(IOWR(base, TMRIR_CCTR_REG_OF, data))										// Capture control register data write
#define TMRIR_READ_MATC(base)						(IORD(base, TMRIR_MATC_REG_OF))												// Match register data read
#define TMRIR_WRITE_MATC(base, data)				(IOWR(base, TMRIR_MATC_REG_OF, data))										// Match register data write
#define TMRIR_READ_DATA(base)						(IORD(base, TMRIR_DATA_REG_OF))												// Timer register data read
#define TMRIR_WRITE_DATA(base, data)				(IOWR(base, TMRIR_DATA_REG_OF, data))										// Timer register data write
#define TMRIR_READ_IRQ(base)						(IORD(base, TMRIR_IRQ_REG_OF) & TMRIR_IRQ_REG_DATA_MASK)					// IRQ register data read
#define TMRIR_WRITE_IRQ(base, data)					(IOWR(base, TMRIR_IRQ_REG_OF, data))										// IRQ register data write

// Direct memory mapped register access
#define TMRIR_REG_PTR(base)							((volatile void *)(base))

typedef struct timerIR
{
	alt_u32 ccr;
	alt_u32 match;
	alt_u32 data;
	alt_u32 irq;
} timerIR_t ;

#endif /* TIMERIR_H_ */
