//==========================
// GPIO Driver Header file
//==========================


#ifndef GPIO_H_
#define GPIO_H_

#include "io.h"
#include "alt_types.h"

// GPIO OFFWRITES
#define GPIO_DATA_REG_OFF 0				// Data register address offWRITE
#define GPIO_DIRE_REG_OFF 1				// Direction register address offWRITE
#define GPIO_INTM_REG_OFF 2				// Interrupt mask register address offWRITE
#define GPIO_EDGE_REG_OFF 3				// Edge capture register address offWRITE
#define GPIO_OWRITE_REG_OFF 4			// EutWRITE -> Specifies which bit of the output port to WRITE. Value is not stored into a physical register in the IP core.
#define GPIO_OCLR_REG_OFF 5				// Outclear -> Specifies which output bit to clear. Value is not stored into a physical register in the IP core.

// GPIO register access macros
#define GPIO_READ_DATA(base) 			(IORD(base, GPIO_DATA_REG_OFF))
#define GPIO_WRITE_DATA(base, data) 	(IOWR(base, GPIO_DATA_REG_OFF, data))		// write access denied on input ports
#define GPIO_READ_DIRE(base) 			(IORD(base, GPIO_DIRE_REG_OFF))				// 0 -> input port; 1 -> output port
#define GPIO_WRITE_DIRE(base, data)	 	(IOWR(base, GPIO_DIRE_REG_OFF, data))		// 0 -> input port; 1 -> output port
#define GPIO_READ_INTM(base) 			(IORD(base, GPIO_INTM_REG_OFF))				// 0 -> disable IRQ; 1 -> enable IRQ
#define GPIO_WRITE_INTM(base)			(IOWR(base, GPIO_INTM_REG_OFF, data))		// 0 -> disable IRQ; 1 -> enable IRQ
#define GPIO_READ_EDGE(base) 			(IORD(base, GPIO_EDGE_REG_OFF))
#define GPIO_WRITE_EDGE(base, data) 	(IOWR(base, GPIO_EDGE_REG_OFF, data))
#define GPIO_WRITE_OWRITE(base, data) 	(IOWR(base, GPIO_OWRITE_REG_OFF, data))
#define GPIO_WRITE_OCLR(base, data) 	(IOWR(base, GPIO_OCLR_REG_OFF, data))

// Direct memory mapped data access
#define GPIO_DATA_REG_PTR(base)			((volatile void *)(base + GPIO_DATA_REG_OFF))

#endif /* GPIO_H_ */
