//====================================================
// Execution Performance Tester Driver Header file
//====================================================

/*  @Brief:
*		- NIOSii/e based (non-vectored IR, no cache and branch prediction)
*		- No multitask measurement support
*	 	- Detects task execution
*      	- Measures exception timings: IR latency, context saving, ISR handling, context restoring
*	@Interfacing
*		Operation			|	Address(RAMaddr)	|	WriteData	|	ReadData
*		 -----------------------------------------------------------------------
*	 	1. Acess RAM			0x0(RAMaddr)			data			data
*      	2. CounterLO			0x80					X				Counter data
*      	3. CounterHI			0x81					X				Counter data
*      	4. Ready Status			0x82					X				Status
*      	5. Start				0x83					0x1				X
*      	6. Stop					0x84					0x1				X
*		7. Task ID				0x85					Task ID			X
*		8. IO Offset			0x86					Offset			X
*		9. ISR handling			0x87					0x1				X
*	   10. Context saving		0x88					0x1				X
*	   11. Context restoring	0x89					0x1				X
*	   12. Executed				0x8a					X				Executed
*	   13. Module reset			0x8b					0x1				X
*/

#ifndef EPT_H_
#define EPT_H_

#include "io.h"
#include "alt_types.h"

//------------
// Data Masks
//------------
#define EPT_RAM_SIZE							7
#define EPT_RAM_ADDRESS_MASK					0x7f
#define EPT_RAM_ADDRESS_MAX						0x7f
#define WORD_MASK								0xffffffffLL
#define BYTE_MASK								0x000000ffLL

//--------------------------------------------------------
// Execution Performance Tester register address offsets
//--------------------------------------------------------
#define	EPT_RAM_OF								0x00
#define	EPT_RAM_IR_OF							0x7C
#define EPT_CTR_LO_OF							0x80					// Counter LOW address offset
#define EPT_CTR_HI_OF							0x81					// Counter HIGH address offset
#define EPT_STATUS_OF							0x82					// IsReady Status address offset
#define EPT_START_OF							0x83					// Start address offset
#define EPT_STOP_OF								0x84					// Stop address offset
#define EPT_TASK_ID_OF							0x85					// Stop address offset
#define EPT_IO_OFFSET_OF						0x86					// Stop address offset
#define EPT_ISR_OF								0x87					// Stop address offset
#define EPT_CTX_SAVE_OF							0x88					// Stop address offset
#define EPT_CTX_REST_OF							0x89					// Stop address offset
#define EPT_EXEC_OF								0x8a					// Stop address offset
#define EPT_RESET_OF							0x8b					// Stop address offset

//---------------------------------------------------------------
// Execution Performance Tester Register Write / Read Operations
//---------------------------------------------------------------
#define EPT_WRITE_RAM(base, address, data)		(IOWR(base, (address & EPT_RAM_ADDRESS_MASK), (data & WORD_MASK)))		// Write data to onchip RAM
#define EPT_READ_RAM(base, address)				(IORD(base, (address & EPT_RAM_ADDRESS_MASK)))							// Read data to onchip RAM
#define EPT_READ_CTR_LO(base)					(IORD(base, EPT_CTR_LO_OF))												// Read counter LOW
#define EPT_READ_CTR_HI(base)					(IORD(base, EPT_CTR_HI_OF))												// Read counter HIGH
#define EPT_READ_STATUS(base)					(IORD(base, EPT_STATUS_OF))												// Read IsReady Status
#define EPT_WRITE_START(base, data)				(IOWR(base, EPT_START_OF, (data & 1)))									// Write Start trigger
#define EPT_WRITE_STOP(base, data)				(IOWR(base, EPT_STOP_OF, (data & 1)))									// Read IsReady Status
#define EPT_WRITE_TASK(base, data)				(IOWR(base, EPT_TASK_ID_OF, (data & BYTE_MASK)))						// Write Task ID
#define EPT_READ_TASK(base)						(IORD(base, EPT_TASK_ID_OF) & BYTE_MASK)								// Read Task ID
#define EPT_WRITE_IOOF(base, data)				(IOWR(base, EPT_IO_OFFSET_OF, (data & BYTE_MASK)))						// Write IO offset
#define EPT_READ_IOOF(base)						(IORD(base, EPT_IO_OFFSET_OF) & BYTE_MASK)								// Read IO offset
#define EPT_WRITE_ISR(base, data)				(IOWR(base, EPT_ISR_OF, (data & 1)))									// Write Interrupt Service Routine trigger
#define EPT_WRITE_CTX_SAVE(base, data)			(IOWR(base, EPT_CTX_SAVE_OF, (data & 1)))								// Write Context Saving trigger
#define EPT_WRITE_CTX_REST(base, data)			(IOWR(base, EPT_CTX_REST_OF, (data & 1)))								// Write Context Restoring trigger
#define EPT_READ_EXEC(base)						(IORD(base, EPT_EXEC_OF))												// Read Executed
#define EPT_WRITE_RESET(base, data)				(IOWR(base, EPT_RESET_OF, (data & 1)))									// Write Reset

//---------------------------
// Memory Mapped interfacing
//---------------------------
#define EPT_RAM_PTR(base, regnum)				((volatile void *)(base + EPT_RAM_OF * regnum))			// EPT RAM Pointer to start address
#define EPT_RAM_IR_PTR(base, regnum)			((volatile void *)(base + EPT_RAM_IR_OF * regnum))		// EPT RAM Pointer to IR timing
#define EPT_CTR_PTR(base, regnum)				((volatile void *)(base + EPT_CTR_LO_OF * regnum))		// EPT Counter Pointer
#define EPT_TASK_PTR(base, regnum)				((volatile void *)(base + EPT_TASK_ID_OF * regnum))		// EPT Task ID pointer
#define EPT_ISR_PTR(base, regnum)				((volatile void *)(base + EPT_ISR_OF * regnum))			// EPT ISR pointer
#define EPT_CTX_SAVE_PTR(base, regnum)			((volatile void *)(base + EPT_CTX_SAVE_OF * regnum))	// EPT Context Save pointer
#define EPT_CTX_REST_PTR(base, regnum)			((volatile void *)(base + EPT_CTX_REST_OF * regnum))	// EPT Context Restore pointer

//----------------------
// Type definitions
//----------------------

// 40 bit Counter
typedef struct eptCounter
{
	alt_u32 Low;
	alt_u8 High;
} eptCounter_t;

// Interrupt Timing Data
typedef struct eptIR
{
	alt_u32 irLatency;
	alt_u32 ctxSave;
	alt_u32 isrHandle;
	alt_u32 ctxRestore;
} eptIR_t;



#endif	//  EPT_H_
