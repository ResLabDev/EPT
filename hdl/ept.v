//=================================================================================================
// Execution Performance Tester Version 0.0
// 	@Brief:
//		  - NIOSii/e based (non-vectored IR, no chache and branch prediction)
//		  - No multitask measurement support
//		  - Detects task execution
//      - Measures exception handling timings: IR latency, context saving, ISR handling, context restoring
//		@Operation Modes:
//		  - Basic 40 bit cycle counter with reset feature
//=================================================================================================

/*** Instantiation ***
ept
#(
	.COUNTER_SIZE(40),
	.DATA_WIDTH(32),
	.RAM_SIZE(7),									
	.TASK_ID_SIZE(RAM_SIZE + 1),
	.OFFSET_SIZE(8)
)
ept1
(
	// Clock-reset
	.clock_i(),
	.reset_i(),
	// RAM Interfacing
	.ramAddress_o(RAM_SIZE),
	.ramWriteData_o(DATA_WIDTH),
	.ramReadData_i(DATA_WIDTH),
	.ramWrite_o(),
	.ramRead_o(),
	// Control input
	.start_i(),
	.stop_i(),
	.taskID_i(TASK_ID_SIZE),				// Storing the actual task ID -> MSB is the current task activity
	.offset_i(OFFSET_SIZE),				// Offset duration of a control write operation
	.irqAssert_i(),							// Posedge triggering at start
	.isrHandling_i(),							// Posedge triggering at start()(), negedge at stop
	.contextSave_i(),  						// Posedge triggering at start()(), negedge at stop
	.contextRestore_i(),						// Posedge triggering at start()(), negedge at stop
	// Data I/O
	.counterData_o(COUNTER_SIZE),
	// Status output
	.ready_o(),
	.doneTick_o()
);
*/

module ept
#(
	parameter
		COUNTER_SIZE 		= 40,
		DATA_WIDTH 			= 32,
		RAM_SIZE				= 7,									
		TASK_ID_SIZE		= RAM_SIZE + 1,
		OFFSET_SIZE			= 8
)
(
	// Clock-reset
	input wire 								clock_i,
	input wire 								reset_i,
	// RAM Interfacing
	output wire [RAM_SIZE-1:0] 		ramAddress_o,
	output wire [DATA_WIDTH-1:0] 		ramWriteData_o,
	input wire [DATA_WIDTH-1:0] 		ramReadData_i,
	output wire								ramWrite_o,
	output wire								ramRead_o,
	// Control input
	input wire 								start_i,
	input wire 								stop_i,
	input wire [TASK_ID_SIZE-1:0]		taskID_i,				// Storing the actual task ID -> MSB is the current task activity
	input wire [OFFSET_SIZE-1:0]		offset_i,				// Offset duration of a control write operation
	input wire 								irqAssert_i,			// Posedge triggering at start
	input wire 								isrHandling_i,			// Posedge triggering at start(), negedge at stop
	input wire 								contextSave_i,  		// Posedge triggering at start(), negedge at stop
	input wire 								contextRestore_i,		// Posedge triggering at start(), negedge at stop
	// Data I/O
	output wire [COUNTER_SIZE-1:0]	counterData_o,
	// Status output
	output reg 								ready_o,
	output reg								doneTick_o
);

	//---------------------------------
	// Internal parameter declaration
	//---------------------------------
	// Basic definitions
	localparam COUNTER_ZEROS = {(COUNTER_SIZE-DATA_WIDTH){1'b0}};
	
	// RAM allocation for STATE_EXCEPTION handling timing parameters: IR Latency, Context Save, ISR Handling, Context Restore
	localparam RESERVED_PARAMETER_SIZE = 4;												
	localparam [RAM_SIZE-1:0] 
		RAM_ADDRESS_MAX 			= ~('b0),															// The maximum addressable RAM
		RAM_ADDRESS_RESERVED		= RAM_ADDRESS_MAX - RESERVED_PARAMETER_SIZE + 1;		// The last addresses is reserved for IRQ latency
	
	// FSM State Definitions
	localparam FSM_SIZE = 3;
	localparam [FSM_SIZE-1:0]
		STATE_IDLE 							= 3'b000,
		STATE_WATCH							= 3'b001,
		STATE_EXCEPTION 					= 3'b010,
		STATE_SUMMARIZE					= 3'b011,
		STATE_DONE							= 3'b100,
		STATE_STORE							= 3'b101;
		
	//-----------------------
	// Signal declaration
	//-----------------------
	
	// Basic registers
	reg [FSM_SIZE-1:0] stateReg, stateNextReg;
	reg [TASK_ID_SIZE-1:0] taskIDReg, taskIDNextReg;
	reg [RAM_SIZE-1:0] ramAddressReg, ramAddressNextReg, taskAddressReg;
	// Counter interfacing
	wire counterEnable, counterReset;
	reg counterResetReg;
	// Measurement Timings
	reg [COUNTER_SIZE-1:0] startTimestampReg, startTimestampNextReg, taskPartTimeReg, taskPartTimeNextReg, elapsedReg, elapsedNextReg, elapsedSumReg, elapsedSumNextReg;
	// Measurement Timestamp Triggers
	reg irqReg, irqNextReg, isrReg, isrNextReg, contextSaveReg, contextSaveNextReg, contextRestoreReg, contextRestoreNextReg, exceptionFlagReg, exceptionFlagNextReg;
	reg taskStartCCR, taskStartNextCCR, taskStopCCR, taskStopNextCCR;
	reg irqStartCCR, irqStartNextCCR, isrStartCCR, isrStartNextCCR, isrStopCCR, isrStopNextCCR, contextSaveStartCCR, contextSaveStartNextCCR,
		 contextSaveStopCCR, contextSaveStopNextCCR, contextRestoreStartCCR, contextRestoreStartNextCCR, contextRestoreStopCCR, contextRestoreStopNextCCR;
	wire taskStartTick, taskStopTick;
	wire irqStartTick, isrStartTick, isrStopTick, contextSaveStartTick, contextSaveStopTick, contextRestoreStartTick, contextRestoreStopTick;
	wire taskEnableRamAddress;
	
	//-------------------------------
	// Clock-edge synchronized DFFs
	//-------------------------------
	always @ (posedge clock_i, posedge reset_i) begin
		if (reset_i) begin
			stateReg 									<= STATE_IDLE;
			taskIDReg 									<= 0;
			ramAddressReg								<= 0;
			taskAddressReg								<= 0;
			taskStartCCR								<= 0;
			taskStopCCR									<= 0;
			irqStartCCR									<= 0;
			isrStartCCR									<= 0;
			isrStopCCR									<= 0;
			contextSaveStartCCR						<= 0;
			contextSaveStopCCR						<= 0;
			contextRestoreStartCCR					<= 0;
			contextRestoreStopCCR					<= 0;
			irqReg										<= 0;
			isrReg										<= 0;
			contextSaveReg								<= 0;
			contextRestoreReg							<= 0;
			startTimestampReg							<= 0;
			taskPartTimeReg							<= 0;
			elapsedReg									<= 0;
			elapsedSumReg								<= 0;
			exceptionFlagReg							<= 0;
		end
		else begin
			stateReg 									<= stateNextReg;
			taskIDReg									<= taskIDNextReg;
			ramAddressReg								<= ramAddressNextReg;
			// Capture Control Registers
			if (taskStartTick) begin
				taskStartCCR							<= 1'b1;
				taskAddressReg							<= taskID_i[TASK_ID_SIZE-2:0];	// Map TaskID to RAM address at each new task
			end
			else begin
				taskStartCCR							<= taskStartNextCCR;
			end
			if (taskStopTick) begin
				taskStopCCR								<= 1'b1;
			end
			else begin
				taskStopCCR								<= taskStopNextCCR;
			end
			if (irqStartTick) begin
				irqStartCCR								<= 1'b1;
			end
			else begin
				irqStartCCR								<= irqStartNextCCR;
			end
			if (isrStartTick) begin
				isrStartCCR								<= 1'b1;
			end
			else begin
				isrStartCCR								<= isrStartNextCCR;
			end
			if (isrStopTick) begin
				isrStopCCR								<= 1'b1;
			end
			else begin
				isrStopCCR								<= isrStopNextCCR;
			end
			if (contextSaveStartTick) begin
				contextSaveStartCCR					<= 1'b1;
			end
			else begin
				contextSaveStartCCR					<= contextSaveStartNextCCR;
			end
			if (contextSaveStopTick) begin
				contextSaveStopCCR					<= 1'b1;
			end
			else begin
				contextSaveStopCCR					<= contextSaveStopNextCCR;
			end
			if (contextRestoreStartTick) begin
				contextRestoreStartCCR				<= 1'b1;
			end
			else begin
				contextRestoreStartCCR				<= contextRestoreStartNextCCR;
			end
			if (contextRestoreStopTick) begin
				contextRestoreStopCCR				<= 1'b1;
			end
			else begin
				contextRestoreStopCCR				<= contextRestoreStopNextCCR;
			end
			irqReg										<= irqNextReg;
			isrReg										<= isrNextReg;
			contextSaveReg								<= contextSaveNextReg;
			contextRestoreReg							<= contextRestoreNextReg;
			startTimestampReg							<= startTimestampNextReg;
			taskPartTimeReg							<= taskPartTimeNextReg;
			elapsedReg									<= elapsedNextReg;
			elapsedSumReg								<= elapsedSumNextReg;
			exceptionFlagReg							<= exceptionFlagNextReg;
		end
	end
	
	//-----------------------------
	// Finite-state machine logic
	//-----------------------------
	always @* begin
		// Basic registers
		stateNextReg 								= stateReg;
		taskIDNextReg								= taskID_i;
		ramAddressNextReg							= ramAddressReg;
		// Capture Control Registers
		taskStartNextCCR							= taskStopCCR;
		taskStopNextCCR							= taskStopCCR;
		irqStartNextCCR							= irqStartCCR;
		isrStartNextCCR							= isrStartCCR;
		isrStopNextCCR								= isrStopCCR;
		contextSaveStartNextCCR					= contextSaveStartCCR;
		contextSaveStopNextCCR					= contextSaveStopCCR;
		contextRestoreStartNextCCR				= contextRestoreStartCCR;
		contextRestoreStopNextCCR				= contextRestoreStopCCR;
		// Control signals
		irqNextReg									= irqAssert_i;
		isrNextReg									= isrHandling_i;
		contextSaveNextReg						= contextSave_i;
		contextRestoreNextReg					= contextRestore_i;
		exceptionFlagNextReg						= exceptionFlagReg;
		// Timings
		startTimestampNextReg					= startTimestampReg;
		taskPartTimeNextReg						= taskPartTimeReg;
		elapsedNextReg								= elapsedReg;
		elapsedSumNextReg							= elapsedSumReg;
		// Status and control
		counterResetReg								= 1'b0;
		ready_o 										= 1'b0;
		doneTick_o 									= 1'b0;
		// State logic
		case (stateReg)
//--- Stand-by mode: waiting for start signal
			STATE_IDLE: begin
				ready_o 								= 1'b1;
				// Detect start input signal
				if (start_i) begin
					counterResetReg					= 1'b1;
					stateNextReg					= STATE_WATCH;
				end
			end
//--- Detects TaskSwitch and STATE_EXCEPTION
			STATE_WATCH: begin
				if (stop_i) begin
					stateNextReg		 		= STATE_DONE;					// Stop the measurement, obtaining the data
				end
				else begin
					// IRQ is asserted, waiting for STATE_EXCEPTION handling
					if (irqStartCCR) begin
						irqStartNextCCR				= 0;																			// Reset captured task register
						startTimestampNextReg		= counterData_o;															// IRQAssert timestamp
						exceptionFlagNextReg			= 1'b1;																		// STATE_EXCEPTION handling is started
						taskPartTimeNextReg			= taskPartTimeReg + (counterData_o - startTimestampReg);		// Task is interrupted, calculate the elapsed part-time without IR latency
						stateNextReg					= STATE_EXCEPTION;														// Initiate STATE_EXCEPTION handling measurements
					end
					// Task Execution
					else begin					
						// A task is started
						if (taskStartCCR) begin
							taskStartNextCCR			= 0;												// Reset captured task register
							startTimestampNextReg 	= counterData_o;								// TaskStart timestamp
							taskPartTimeNextReg		= 0;											// Reset part time register
						end
						// A task is finished
						if (taskStopCCR) begin
							taskStopNextCCR				= 0;																												// Reset captured task register
							elapsedNextReg 				= (counterData_o - startTimestampReg) + taskPartTimeReg - {COUNTER_ZEROS, offset_i};	// Calculate the Task duration
							// Set RAM address
							if (taskEnableRamAddress) begin
								ramAddressNextReg 	= taskAddressReg;											
							end
							else begin
								ramAddressNextReg		= RAM_ADDRESS_RESERVED - 1;			// Disable reserved memory address, set to the last available value
							end
							stateNextReg = STATE_SUMMARIZE;
						end
					end
				end
			end
//--- STATE_EXCEPTION handler: IRQ -> CPU ContextSaving -> ISR -> CPU ContextRestoring
			STATE_EXCEPTION: begin
				// IR Latency = Context Save Start - IRQ Assert
				if (contextSaveStartCCR) begin
					contextSaveStartNextCCR				= 0;																						// Reset captured task register
					elapsedNextReg							= (counterData_o - startTimestampReg) - {COUNTER_ZEROS, offset_i};	// Duration of IR latency
					startTimestampNextReg				= counterData_o;																		// Set contextSaveStartTick timestamp
					ramAddressNextReg						= RAM_ADDRESS_RESERVED;																// Set RAM to IR latency
					stateNextReg							= STATE_SUMMARIZE;
				end
				// Context Save = Context Save Stop - Context Save Start
				if (contextSaveStopCCR) begin
					contextSaveStopNextCCR				= 0;																						// Reset captured task register
					taskPartTimeNextReg					= taskPartTimeReg + elapsedReg;													// Add IR latency to interrupted Task's part time
					elapsedNextReg							= (counterData_o - startTimestampReg) - {COUNTER_ZEROS, offset_i};	// Duration of Context Save
					ramAddressNextReg						= RAM_ADDRESS_RESERVED + 1;														// Set RAM to Context Save
					stateNextReg							= STATE_SUMMARIZE;
				end
				// ISR = ISR Stop - ISR Start
				if (isrStartCCR) begin
					isrStartNextCCR						= 0;																						// Reset captured task register
					startTimestampNextReg				= counterData_o;
				end
				if (isrStopCCR) begin
					isrStopNextCCR							= 0;																						// Reset captured task register
					elapsedNextReg							= (counterData_o - startTimestampReg) - {COUNTER_ZEROS, offset_i};	// Duration of ISR
					ramAddressNextReg						= RAM_ADDRESS_RESERVED + 2;														// Set RAM to ISR
					stateNextReg							= STATE_SUMMARIZE;
				end
				// Context Restore = Context Restore Stop - Context Restore Start
				if (contextRestoreStartCCR) begin
					contextRestoreStartNextCCR			= 0;																						// Reset captured task register
					startTimestampNextReg				= counterData_o;
				end
				if (contextRestoreStopCCR) begin
					contextRestoreStopNextCCR			= 0;																						// Reset captured task register
					elapsedNextReg							= (counterData_o - startTimestampReg) - {COUNTER_ZEROS, offset_i};	// Duration of Context Restore
					ramAddressNextReg						= RAM_ADDRESS_RESERVED + 3;														// Set RAM to Context Restore
					exceptionFlagNextReg					= 1'b0;																					// The STATE_EXCEPTION handling is finished
					startTimestampNextReg				= counterData_o;																		// Set start timestamp for interrupted task snippet part-time measurement
					stateNextReg							= STATE_SUMMARIZE;
				end
			end
//--- STATE_SUMMARIZE and prepare data for storing data in external RAM
			STATE_SUMMARIZE: begin
				elapsedSumNextReg 		= elapsedReg + ramReadData_i;				// STATE_SUMMARIZE the elapsed cycles
				stateNextReg				= STATE_STORE;
			end
			STATE_STORE: begin
				// STATE_EXCEPTION handling is ongoing
				if (exceptionFlagReg) begin
					stateNextReg		= STATE_EXCEPTION;
				end
				// STATE_EXCEPTION handling is finished
				else begin
					stateNextReg		= STATE_WATCH;
				end
			end
			STATE_DONE: begin
				doneTick_o				= 1'b1;
				stateNextReg			= STATE_IDLE;
			end
			default: begin
				stateNextReg 			= STATE_IDLE;
			end
		endcase
	end
	
	// Instantiate Counter
	counter #(.COUNTER_SIZE(COUNTER_SIZE)) counter1
	(
		// Clock-reset
		.clock_i(clock_i),
		.reset_i(counterReset),
		// Control signals
		.enable_i(counterEnable),
		// Output(s)	
		.counterOut_o(counterData_o)		// Get actual counter's transparent value output
	);

	//------------------------
	// Control logic signals
	//------------------------
	assign counterReset	= (reset_i) ? reset_i : counterResetReg;
	assign counterEnable = (stateReg != STATE_IDLE);
	// Posedge detection of task ID input MSB -> shows the task starting activity
	assign taskStartTick 				= (taskIDNextReg[TASK_ID_SIZE-1:TASK_ID_SIZE-1] > taskIDReg[TASK_ID_SIZE-1:TASK_ID_SIZE-1]);
	// Negedge detection of task ID input MSB -> shows the task stopping activity
	assign taskStopTick 					= (taskIDNextReg[TASK_ID_SIZE-1:TASK_ID_SIZE-1] < taskIDReg[TASK_ID_SIZE-1:TASK_ID_SIZE-1]);
	// IRQ, ISR and Context Saving triggers
	assign irqStartTick 					= (irqNextReg > irqReg) ? 1'b1 : 0;																		// Posedge detection
	assign isrStartTick 					= (isrNextReg > isrReg) ? 1'b1 : 0;																		// Posedge detection
	assign isrStopTick 					= (isrNextReg < isrReg) ? 1'b1 : 0;																		// Negedge detection
	assign contextSaveStartTick		= (contextSaveNextReg > contextSaveReg) ? 1'b1 : 0;												// Posedge detection
	assign contextSaveStopTick			= (contextSaveNextReg < contextSaveReg) ? 1'b1 : 0;												// Negedge detection
	assign contextRestoreStartTick	= (contextRestoreNextReg > contextRestoreReg) ? 1'b1 : 0;										// Posedge detection
	assign contextRestoreStopTick		= (contextRestoreNextReg < contextRestoreReg) ? 1'b1 : 0;										// Negedge detection
	assign taskEnableRamAddress		= (stateReg == STATE_WATCH) & (taskIDReg[TASK_ID_SIZE-2:0] < RAM_ADDRESS_RESERVED);		// Last addresses are reserved for STATE_EXCEPTION latency storing
	
	//------------------------
	// Output assignments
	//------------------------	
	// RAM control signals
	assign ramRead_o					= (stateReg == STATE_SUMMARIZE);
	assign ramWrite_o 				= (stateReg == STATE_STORE);																				// Enable RAM writing only at STATE_STORE state
	assign ramAddress_o				= (stateReg == STATE_IDLE) ? 0 : ramAddressNextReg;														
	assign ramWriteData_o			= (stateReg == STATE_STORE) ? elapsedSumReg[DATA_WIDTH-1:0] : 0;								// For storing the summarized data in the RAM

endmodule

