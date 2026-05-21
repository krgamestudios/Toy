#pragma once

typedef enum Toy_OpcodeType {
	//This offsets the opcode values, so I can see TOY_OPCODE_READ in GDB clearly
	TOY_OPCODE_UNUSED = 0,

	//variable instructions
	TOY_OPCODE_READ,
	TOY_OPCODE_DECLARE,
	TOY_OPCODE_ASSIGN,
	TOY_OPCODE_ASSIGN_COMPOUND, //assign to a compound's internals
	TOY_OPCODE_ACCESS,
	TOY_OPCODE_INVOKE, //for calling functions
	TOY_OPCODE_ATTRIBUTE, //for accessing parts of compounds
	TOY_OPCODE_ITERABLE, //for operating on all members of a compound
	TOY_OPCODE_DUPLICATE, //duplicate the top of the stack
	TOY_OPCODE_ELIMINATE, //remove the top of the stack

	//arithmetic instructions
	TOY_OPCODE_ADD,
	TOY_OPCODE_SUBTRACT,
	TOY_OPCODE_MULTIPLY,
	TOY_OPCODE_DIVIDE,
	TOY_OPCODE_MODULO,

	//comparison instructions
	TOY_OPCODE_COMPARE_EQUAL,
	// TOY_OPCODE_COMPARE_NOT, //NOTE: optimized into a composite of TOY_OPCODE_COMPARE_EQUAL + TOY_OPCODE_NEGATE
	TOY_OPCODE_COMPARE_LESS,
	TOY_OPCODE_COMPARE_LESS_EQUAL,
	TOY_OPCODE_COMPARE_GREATER,
	TOY_OPCODE_COMPARE_GREATER_EQUAL,

	//logical instructions
	TOY_OPCODE_AND,
	TOY_OPCODE_OR,
	TOY_OPCODE_TRUTHY,
	TOY_OPCODE_NEGATE,

	//control instructions
	TOY_OPCODE_RETURN,
	TOY_OPCODE_JUMP,   //JUMP, ADDR
	TOY_OPCODE_ESCAPE, //JUMP, ADDR, UNWIND

	TOY_OPCODE_SCOPE_PUSH,
	TOY_OPCODE_SCOPE_POP,

	//various action instructions
	TOY_OPCODE_ASSERT,
	TOY_OPCODE_PRINT,
	TOY_OPCODE_CONCAT,
	TOY_OPCODE_INDEX,

	//meta instructions
	TOY_OPCODE_PASS,
	TOY_OPCODE_ERROR,
	TOY_OPCODE_EOF = 255,
} Toy_OpcodeType;

//specific opcode flags
typedef enum Toy_OpParamJumpType {
	TOY_OP_PARAM_JUMP_ABSOLUTE = 0, //from the start of the routine's code section
	TOY_OP_PARAM_JUMP_RELATIVE = 1,
} Toy_OpParamJumpType;

typedef enum Toy_OpParamJumpConditional {
	TOY_OP_PARAM_JUMP_ALWAYS = 0,
	TOY_OP_PARAM_JUMP_IF_TRUE = 1,
	TOY_OP_PARAM_JUMP_IF_FALSE = 2,
} Toy_OpParamJumpConditional;

