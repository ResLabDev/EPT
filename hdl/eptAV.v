//=================================================================================================
// Execution Performance Tester Interface
// 	@Brief:
//		  - NIOSii/e based (non-vectored IR, no cache and branch prediction)
//		  - No multitask measurement support
//		  - Detects task execution
//      - Measures exception timings: IR latency, context saving, ISR handling, context restoring
//		@Operation Modes by Address:
//			 Operation			|	Address(RAMaddr)	|	WriteData	|	ReadData
//			 -----------------------------------------------------------------------
//		  1. Acess RAM				0x0(RAMaddr)			data				data
//      2. GetCounterLO			0x80						X					Counter data
//      3. GetCounterHI			0x81						X					Counter data
//      4. Ready Status			0x82						X					Status
//      5. Start					0x83						0x1				X					
//      6. Stop					0x84						0x1				X					
//		  7. Set Task ID			0x85						Task ID			X
//		  8. Set offset			0x86						Offset			X
//		 9. ISR handling			0x87						0x1				X
//		 10. Context saving		0x88						0x1				X
//		 11. Context restoring	0x89						0x1				X
//		 12. Get executed			0x8a						X					Executed
//		 13. Module reset			0x8b						0x1				X
//=================================================================================================

module eptAV
#( 
	parameter
		ADDRESS_WIDTH		= 8,									
		DATA_WIDTH			= 32,
		COUNTER_SIZE		= 40
)
(
	// Clock - Reset
	input wire 													ept_clock,
	input wire 													ept_reset,
	// Avalon MM Slave
	input wire	[ADDRESS_WIDTH-1:0]						ept_address,
	input wire 	[DATA_WIDTH-1:0]							ept_writedata,
	output wire	[DATA_WIDTH-1:0]							ept_readdata,
	input wire 													ept_chipselect,
	input wire 													ept_write,
	// Conduit to interrupt
	input wire 													ept_irc,
	// Conduit to status
	output wire 												ept_status,
	// Conduit to RAM
	output wire	[ADDRESS_WIDTH-2:0]		 				ept_ramaddress_exp,
	output wire	[DATA_WIDTH-1:0]  						ept_ramwritedata_exp,
	input wire  [DATA_WIDTH-1:0] 							ept_ramreaddata_exp,
	output wire													ept_ramwrite_exp
);

	//----------------------------------
	// Local defintions
	//----------------------------------
	localparam
		RAM_ADDRESS_WIDTH 	= ADDRESS_WIDTH - 1,
		TASK_ID_SIZE			= RAM_ADDRESS_WIDTH + 1,
		OFFSET_SIZE				= 8;
		
	// Memory Mapped Reference Addresses
	localparam [ADDRESS_WIDTH-1:0]
		MM_COUNTER_LO	= 8'h80,
		MM_COUNTER_HI	= 8'h81,
		MM_READY			= 8'h82,
		MM_START			= 8'h83,
		MM_STOP			= 8'h84,
		MM_TASK_ID		= 8'h85,
		MM_OFFSET		= 8'h86,
		MM_ISR			= 8'h87,
		MM_CTX_SAVE		= 8'h88,
		MM_CTX_RESTORE	= 8'h89,
		MM_EXECUTED		= 8'h8a,
		MM_RESET			= 8'h8b;
	
	//----------------------------------
	// Signal declaration
	//----------------------------------
	wire [COUNTER_SIZE-1:0] counterData;
	wire [DATA_WIDTH-1:0] 
		counterLow = counterData[DATA_WIDTH-1:0],
		counterHigh = counterData[COUNTER_SIZE-1:DATA_WIDTH];
	wire write, ramWrite, start, stop, ready, ramDirectAccess;
	wire [RAM_ADDRESS_WIDTH-1:0] ramAddress;
	wire [DATA_WIDTH-1:0] ramWriteData;
	reg  [TASK_ID_SIZE-1:0] taskIDReg;
	reg  [OFFSET_SIZE-1:0] offsetReg;
	reg startReg, stopReg, isrHandlingReg, contextSavingReg, contextRestoringReg;
	reg executedReg, resetReg;
	wire doneTick, reset, setReset;
	wire setTaskID, setOffset, isrHandling, contextSaving, contextRestoring;
	
	//----------------------------------
	// Synchronization DFFs
	//----------------------------------
	always @ (posedge ept_clock, posedge reset) begin
		if (reset) begin
			startReg						<= 0;
			stopReg						<= 0;
			taskIDReg					<= 0;
			offsetReg					<= 0;
			isrHandlingReg				<= 0;
			contextSavingReg			<= 0;
			contextRestoringReg		<= 0;
			executedReg					<= 0;
			resetReg						<= 0;
		end
		else begin
			if (write) begin
				if (start) begin
					startReg					<= ept_writedata[0];							// Set start register
				end
				if (stop) begin
					stopReg					<= ept_writedata[0];							// Set start register
				end
				if (setTaskID) begin
					taskIDReg				<= ept_writedata[TASK_ID_SIZE-1:0];		// Set task ID register
				end
				if (setOffset) begin
					offsetReg				<= ept_writedata[OFFSET_SIZE-1:0];		// Set IO Offset register
				end
				if (isrHandling) begin
					isrHandlingReg			<= ept_writedata[0];							// Set Interrupt Service Routin activity
				end
				if (contextSaving) begin
					contextSavingReg		<= ept_writedata[0];							// Set Context Saving activity
				end	
				if (contextRestoring) begin
					contextRestoringReg	<= ept_writedata[0];							// Set Context Restoring activity
				end
				if (setReset) begin
					resetReg					<= ept_writedata[0];							// Set Reset register
				end
			end
			if (doneTick) begin
				executedReg <= executedReg + 1;
			end
		end
	end
	
	//----------------------------------
	// Controller logic
	//----------------------------------
	assign write 				= ept_write & ept_chipselect;
	assign start 				= (ept_address == MM_START);										// Start the EPT
	assign stop 				= (ept_address == MM_STOP);										// Stop the EPT
	assign isrHandling		= (ept_address == MM_ISR) & write;
	assign contextSaving		= (ept_address == MM_CTX_SAVE) & write;
	assign contextRestoring	= (ept_address == MM_CTX_RESTORE) & write;					
	assign setTaskID			= (ept_address == MM_TASK_ID) & write;
	assign setOffset			= (ept_address == MM_OFFSET) & write;
	assign setReset			= (ept_address == MM_RESET) & write;
	assign reset				= (ept_reset | resetReg);											// Generate module reset from global OR command reset
	
	//----------------------------------
	// I/O Assignments
	//----------------------------------
	// RAM Interfacing
	assign ramDirectAccess				= ready & ~start & ~ept_address[ADDRESS_WIDTH-1];												// Direct RAM Access decoder
	assign ept_ramaddress_exp			= (ramDirectAccess) ? ept_address[RAM_ADDRESS_WIDTH-1:0] : ramAddress;
	assign ept_ramwritedata_exp		= (ramDirectAccess) ? ept_writedata : ramWriteData;
	assign ept_ramwrite_exp				= (ramDirectAccess) ? write : ramWrite;
	assign ept_status						= ready;
	// Avalon MM Readdata decoding
	assign ept_readdata 					= (ramDirectAccess) ? ept_ramreaddata_exp :
												  (ept_address == MM_COUNTER_LO) ? counterLow :
												  (ept_address == MM_COUNTER_HI) ? counterHigh :
												  (ept_address == MM_READY) ? {{(DATA_WIDTH-1){1'b0}}, ready} :
												  (ept_address == MM_START) ? {{(DATA_WIDTH-1){1'b0}}, startReg} :
												  (ept_address == MM_STOP) ? {{(DATA_WIDTH-1){1'b0}}, stopReg} :
												  (ept_address == MM_TASK_ID) ? {{(DATA_WIDTH-TASK_ID_SIZE){1'b0}}, taskIDReg} :
												  (ept_address == MM_OFFSET) ? {{(DATA_WIDTH-OFFSET_SIZE){1'b0}}, offsetReg} :
												  (ept_address == MM_ISR) ? {{(DATA_WIDTH-1){1'b0}}, isrHandlingReg} :
												  (ept_address == MM_CTX_SAVE) ? {{(DATA_WIDTH-1){1'b0}}, contextSavingReg} :
												  (ept_address == MM_CTX_RESTORE) ? {{(DATA_WIDTH-TASK_ID_SIZE){1'b0}}, contextRestoringReg} :
												  (ept_address == MM_EXECUTED) ? {{(DATA_WIDTH-1){1'b0}}, executedReg} :
												  (ept_address == MM_RESET) ? {{(DATA_WIDTH-1){1'b0}}, resetReg} : 0;
	
	//----------------------------------
	// Instantiate Task Watcher Module
	//----------------------------------
	ept
	#(
		.COUNTER_SIZE(COUNTER_SIZE),
		.DATA_WIDTH(DATA_WIDTH),
		.RAM_SIZE(RAM_ADDRESS_WIDTH),									
		.TASK_ID_SIZE(TASK_ID_SIZE),
		.OFFSET_SIZE(OFFSET_SIZE)
	)
	ept1
	(
		// Clock - Reset
		.clock_i(ept_clock),
		.reset_i(reset),
		// RAM Interfacing
		.ramAddress_o(ramAddress),
		.ramWriteData_o(ramWriteData),
		.ramReadData_i(ept_ramreaddata_exp),
		.ramRead_o(),
		.ramWrite_o(ramWrite),
		// Control input
		.start_i(startReg),
		.stop_i(stopReg),
		.taskID_i(taskIDReg),									// Storing the actual task ID -> MSB is the current task activity
		.offset_i(offsetReg),									// Offset duration of a control write operation
		.irqAssert_i(ept_irc),							// Posedge triggering at start
		.isrHandling_i(isrHandlingReg),						// Posedge triggering at start()(), negedge at stop
		.contextSave_i(contextSavingReg),  					// Posedge triggering at start()(), negedge at stop
		.contextRestore_i(contextRestoringReg),				// Posedge triggering at start()(), negedge at stop
		// Data I/O
		.counterData_o(counterData),
		// Status output
		.ready_o(ready),
		.doneTick_o(doneTick)
	);
	
endmodule
