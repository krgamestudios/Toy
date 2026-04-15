#include "bytecode_inspector.h"
#include "toy_console_colors.h"
#include "toy_opcodes.h"
#include "toy_value.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>

int inspect_instruction(unsigned char* bytecode, unsigned int pc, unsigned int jumps_addr, unsigned int data_addr);
int inspect_read(unsigned char* bytecode, unsigned int pc, unsigned int jumps_addr, unsigned int data_addr);

// void inspect_jumps(unsigned char* bytecode, unsigned int pc, unsigned int size);
// void inspect_param(unsigned char* bytecode, unsigned int pc, unsigned int size);
// void inspect_data(unsigned char* bytecode, unsigned int pc, unsigned int size);
// void inspect_subs(unsigned char* bytecode, unsigned int pc, unsigned int size);

#define MARKER_VALUE(pc, type) \
	(pc * sizeof(type))

#define MARKER "\033[" TOY_CC_FONT_BLACK "m" " %lu\t" TOY_CC_RESET

//exposed functions
void inspect_bytecode(unsigned char* bytecode) {
	//TODO: handle version info

	unsigned int const header_size = 0;
	unsigned int const header_jumps = 1;
	unsigned int const header_param = 2;
	unsigned int const header_data = 3;
	unsigned int const header_subs = 4;

	//header size
	printf(MARKER TOY_CC_NOTICE "Bytecode Size: \t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(header_size, unsigned int), ((unsigned int*)(bytecode))[header_size]);

	//header counts
	printf(MARKER TOY_CC_NOTICE "Jumps Size:\t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(header_jumps, unsigned int), ((unsigned int*)(bytecode))[header_jumps]);
	printf(MARKER TOY_CC_NOTICE "Param Size:\t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(header_param, unsigned int), ((unsigned int*)(bytecode))[header_param]);
	printf(MARKER TOY_CC_NOTICE "Data Size:\t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(header_data, unsigned int), ((unsigned int*)(bytecode))[header_data]);
	printf(MARKER TOY_CC_NOTICE "Subs Size:\t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(header_subs, unsigned int), ((unsigned int*)(bytecode))[header_subs]);

	printf("\n---\n");

	//some addresses may be absent
	unsigned int addr_pc = 4;
	unsigned int code_addr = 0;
	unsigned int jumps_addr = 0;
	unsigned int param_addr = 0;
	unsigned int data_addr = 0;
	unsigned int subs_addr = 0;


	//header addresses
	if (true) {
		addr_pc++;
		printf(MARKER TOY_CC_NOTICE "Code Address:\t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(addr_pc, unsigned int), ((unsigned int*)(bytecode))[addr_pc]);
		code_addr = ((unsigned int*)(bytecode))[addr_pc];
	}

	if (((unsigned int*)(bytecode))[header_jumps] > 0) {
		addr_pc++;
		printf(MARKER TOY_CC_NOTICE "Jumps Address:\t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(addr_pc, unsigned int), ((unsigned int*)(bytecode))[addr_pc]);
		jumps_addr = ((unsigned int*)(bytecode))[addr_pc];
	}

	if (((unsigned int*)(bytecode))[header_param] > 0) {
		addr_pc++;
		printf(MARKER TOY_CC_NOTICE "Param Address:\t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(addr_pc, unsigned int), ((unsigned int*)(bytecode))[addr_pc]);
		param_addr = ((unsigned int*)(bytecode))[addr_pc];
	}

	if (((unsigned int*)(bytecode))[header_data] > 0) {
		addr_pc++;
		printf(MARKER TOY_CC_NOTICE "Data Address:\t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(addr_pc, unsigned int), ((unsigned int*)(bytecode))[addr_pc]);
		data_addr = ((unsigned int*)(bytecode))[addr_pc];
	}

	if (((unsigned int*)(bytecode))[header_subs] > 0) {
		addr_pc++;
		printf(MARKER TOY_CC_NOTICE "Subs Address:\t\t%u" TOY_CC_RESET "\n", MARKER_VALUE(addr_pc, unsigned int), ((unsigned int*)(bytecode))[addr_pc]);
		subs_addr = ((unsigned int*)(bytecode))[addr_pc];
	}

	printf("\n---\n");

	unsigned int pc = code_addr;
	while(bytecode[pc] != TOY_OPCODE_RETURN) {
		pc += inspect_instruction(bytecode, pc, jumps_addr, data_addr);
	}
	pc += inspect_instruction(bytecode, pc, jumps_addr, data_addr); //one more for the final return

	(void)jumps_addr;
	(void)param_addr;
	(void)data_addr;
	(void)subs_addr;
}

int inspect_instruction(unsigned char* bytecode, unsigned int pc, unsigned int jumps_addr, unsigned int data_addr) {
	//read and print the opcode instruction at 'pc'

	Toy_OpcodeType opcode = bytecode[pc];

	switch(opcode) {
		case TOY_OPCODE_READ:
			return inspect_read(bytecode, pc, jumps_addr, data_addr);

		case TOY_OPCODE_DECLARE: {
			unsigned int indexValue = *((unsigned int*)(bytecode + pc + 4));
			unsigned int jumpValue = *((unsigned int*)(bytecode + jumps_addr + indexValue));
			char* cstr = ((char*)(bytecode + data_addr + jumpValue));
			printf(MARKER "DECLARE %s: %s%s\n", MARKER_VALUE(pc, unsigned char),
				cstr,
				Toy_private_getValueTypeAsCString(bytecode[pc + 1]),
				bytecode[pc + 3] ? " const" : ""
			);
			return 8;
		}

		case TOY_OPCODE_ASSIGN:
			printf(MARKER "ASSIGN %s\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] ? "(chained)" : "");
			return 4;

		case TOY_OPCODE_ASSIGN_COMPOUND:
			printf(MARKER "ASSIGN COMPOUND %s\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] ? "(chained)" : "");
			return 4;

		case TOY_OPCODE_ACCESS:
			printf(MARKER "ACCESS\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_INVOKE:
			printf(MARKER "INVOKE %s (%d args)\n", MARKER_VALUE(pc, unsigned char),
			Toy_private_getValueTypeAsCString(bytecode[pc + 1]),
			bytecode[pc + 2]);
			return 4;

		case TOY_OPCODE_DUPLICATE:
			printf(MARKER "DUPLICATE %s\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] ? "and ACCESS" : "");
			return 4;

		case TOY_OPCODE_ELIMINATE:
			printf(MARKER "ELIMINATE\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_ADD:
			printf(MARKER "ADD %s\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] == TOY_OPCODE_ASSIGN ? "ASSIGN" : "");
			return 4;

		case TOY_OPCODE_SUBTRACT:
			printf(MARKER "SUBTRACT %s\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] == TOY_OPCODE_ASSIGN ? "ASSIGN" : "");
			return 4;

		case TOY_OPCODE_MULTIPLY:
			printf(MARKER "MULTIPLY %s\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] == TOY_OPCODE_ASSIGN ? "ASSIGN" : "");
			return 4;

		case TOY_OPCODE_DIVIDE:
			printf(MARKER "DIVIDE %s\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] == TOY_OPCODE_ASSIGN ? "ASSIGN" : "");
			return 4;

		case TOY_OPCODE_MODULO:
			printf(MARKER "MODULO %s\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] == TOY_OPCODE_ASSIGN ? "ASSIGN" : "");
			return 4;

		case TOY_OPCODE_COMPARE_EQUAL:
			printf(MARKER "COMPARE %s\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] != TOY_OPCODE_NEGATE ? "==" : "!=");
			return 4;

		case TOY_OPCODE_COMPARE_LESS:
			printf(MARKER "COMPARE '<'\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_COMPARE_LESS_EQUAL:
			printf(MARKER "COMPARE '<='\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_COMPARE_GREATER:
			printf(MARKER "COMPARE '>'\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_COMPARE_GREATER_EQUAL:
			printf(MARKER "COMPARE '>='\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_AND:
			printf(MARKER "LOGICAL '&&'\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_OR:
			printf(MARKER "LOGICAL '||'\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_TRUTHY:
			printf(MARKER "LOGICAL TRUTHY\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_NEGATE:
			printf(MARKER "LOGICAL NEGATE\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_RETURN:
			printf(MARKER "Keyword RETURN (%u)\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1]);
			return 4;

		case TOY_OPCODE_JUMP:
			printf(MARKER  TOY_CC_DEBUG "JUMP %s%s (%s%d) (GOTO %u)\n" TOY_CC_RESET, MARKER_VALUE(pc, unsigned char),
				bytecode[pc + 1] == TOY_OP_PARAM_JUMP_ABSOLUTE ? "absolute" : "relative",
				bytecode[pc + 2] == TOY_OP_PARAM_JUMP_ALWAYS ? "" :
					bytecode[pc + 2] == TOY_OP_PARAM_JUMP_IF_TRUE ? " if true" : " if false",
				bytecode[pc + 4] > 0 ? "+" : "", //show a + sign when positive
				bytecode[pc + 4],
				bytecode[pc + 4] + pc + 8
				);
			return 8;

		case TOY_OPCODE_ESCAPE:
			printf(MARKER  TOY_CC_DEBUG "ESCAPE relative %s%d (GOTO %u) and pop %d\n" TOY_CC_RESET, MARKER_VALUE(pc, unsigned char),
				bytecode[pc + 4] > 0 ? "+" : "", //show a + sign when positive
				bytecode[pc + 4],
				bytecode[pc + 4] + pc + 12,
				bytecode[pc + 8]
			);
			return 12;

		case TOY_OPCODE_SCOPE_PUSH:
			printf(MARKER "SCOPE PUSH\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_SCOPE_POP:
			printf(MARKER "SCOPE POP\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_ASSERT:
			printf(MARKER TOY_CC_WARN "Keyword ASSERT (cond%s)\n" TOY_CC_RESET, MARKER_VALUE(pc, unsigned char), bytecode[pc + 1] > 1 ? ",msg" : "");
			return 4;

		case TOY_OPCODE_PRINT:
			printf(MARKER TOY_CC_NOTICE "Keyword PRINT\n" TOY_CC_RESET, MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_CONCAT:
			printf(MARKER "CONCATENATE strings\n", MARKER_VALUE(pc, unsigned char));
			return 4;

		case TOY_OPCODE_INDEX:
			printf(MARKER "INDEX (%d elements)\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 1]);
			return 4;

		// case TOY_OPCODE_UNUSED:
		// case TOY_OPCODE_PASS:
		// case TOY_OPCODE_ERROR:
		// case TOY_OPCODE_EOF:

		default:
			printf(MARKER TOY_CC_WARN "Unknown Word: [%u, %u, %u, %u]" TOY_CC_RESET "\n", MARKER_VALUE(pc, unsigned char), bytecode[pc], bytecode[pc+1], bytecode[pc+2], bytecode[pc+3]);
			return 4;
	}
}

int inspect_read(unsigned char* bytecode, unsigned int pc, unsigned int jumps_addr, unsigned int data_addr) {
	Toy_ValueType type = bytecode[pc + 1];

	switch(type) {
		case TOY_VALUE_NULL: {
			printf(MARKER "READ NULL\n", MARKER_VALUE(pc, unsigned char));
			return 4;
		}

		case TOY_VALUE_BOOLEAN: {
			if (bytecode[pc + 2]) {
				printf(MARKER "READ BOOL true\n", MARKER_VALUE(pc, unsigned char));
			}
			else {
				
			}
			return 4;
		}

		case TOY_VALUE_INTEGER: {
			int i = *(int*)(bytecode + pc + 4);
			printf(MARKER "READ INTEGER %d\n", MARKER_VALUE(pc, unsigned char), i);
			return 8;
		}

		case TOY_VALUE_FLOAT: {
			float i = *(float*)(bytecode + pc + 4);
			printf(MARKER "READ FLOAT %f\n", MARKER_VALUE(pc, unsigned char), i);
			return 8;
		}

		case TOY_VALUE_STRING: {
			Toy_StringType stringType = (Toy_StringType)(*(bytecode + pc + 2)); //Probably not needed
			int len = bytecode[pc + 3]; //only used for names?

			(void)stringType;

			unsigned int indexValue = *((unsigned int*)(bytecode + pc + 4));
			unsigned int jumpValue = *((unsigned int*)(bytecode + jumps_addr + indexValue));
			char* cstr = ((char*)(bytecode + data_addr + jumpValue));

			printf(MARKER "READ STRING (%d) %s\n", MARKER_VALUE(pc, unsigned char), len, cstr);

			return 8;
		}

		case TOY_VALUE_FUNCTION:
			printf(MARKER "READ FUNCTION (%d params)\n", MARKER_VALUE(pc, unsigned char), bytecode[pc + 2]);
			return 8;

		case TOY_VALUE_ARRAY:
		case TOY_VALUE_TABLE: 
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_UNKNOWN:
		default: {
			printf(MARKER "READ %s (unhandled by inspector)\n", MARKER_VALUE(pc, unsigned char), Toy_private_getValueTypeAsCString(type));
			return 4;
		}
	}
}

//URGENT: Check if strings are reused in the bytecode