//===============================
// Counter for cycle counting
//===============================

/*** @Brief: ***
* Simple N-bit counter cycle solution with Enable and Reset features
****************/

/*** Instantiation ***
	counter #(.COUNTER_SIZE(COUNTER_SIZE)) ccounter
	(
		// Clock-reset
		.clock_i(clock),
		.reset_i(reset),
		// Control signals
		.enable_i(enable),
		// Output(s)	
		.counterOut_o(COUNTER_SIZE)		// Get actual counter's transparent value output
	);
*/

module counter
#(
	parameter 
		COUNTER_SIZE	= 40
)
(
	// Clock-reset
	input wire 								clock_i,
	input wire 								reset_i,
	// Control signals	
	input wire 								enable_i,
	// Output(s)	
	output wire	[COUNTER_SIZE-1:0]	counterOut_o		// Get actual counter's transparent value output
);

	// Signal declaration
	reg [COUNTER_SIZE-1:0] counterReg;
	
	// Counter module
	always @ (posedge clock_i, posedge reset_i) begin
		if (reset_i) begin
			counterReg <= 0;
		end
		else if (enable_i) begin
			counterReg <= counterReg + 1;
		end
	end
	
	// Output assignment
	assign counterOut_o = counterReg;				// Output counter's actual value

endmodule



