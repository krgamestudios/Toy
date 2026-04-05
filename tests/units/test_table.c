#include "toy_table.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_table_allocation(void) {
	//allocate and free a table
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//check
		if (table == NULL)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a table\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	return 0;
}

int test_table_simple_insert_lookup_and_remove(void) {
	//simple insert
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		Toy_Value key = TOY_VALUE_FROM_INTEGER(1);
		Toy_Value value = TOY_VALUE_FROM_INTEGER(42);

		//insert
		Toy_insertTable(&table, key, value);
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 1)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to insert into a table\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//lookup
		Toy_Value result = Toy_lookupTable(&table, TOY_VALUE_FROM_INTEGER(1));

		//check lookup
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 1 ||
			TOY_VALUE_AS_INTEGER(result) != 42)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to lookup from a table\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//remove
		Toy_removeTable(&table, TOY_VALUE_FROM_INTEGER(1));

		//check remove
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to remove from a table\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	return 0;
}

//macros are a godsend
#define TEST_ENTRY_STATE(i, k, v, p) \
	TOY_VALUE_IS_INTEGER(table->data[i].key) != true || \
	TOY_VALUE_AS_INTEGER(table->data[i].key) != k || \
	TOY_VALUE_IS_INTEGER(table->data[i].value) != true || \
	TOY_VALUE_AS_INTEGER(table->data[i].value) != v || \
	table->data[i].psl != p

int test_table_contents_no_expansion(void) {
	//single insert
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//insert a key and value
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(1), TOY_VALUE_FROM_INTEGER(42));

		//check the state
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 1 ||

			TEST_ENTRY_STATE(7, 1, 42, 0)
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unrecognized state from table data, single insert {1:42}\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//multiple inserts, no collisions
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(1), TOY_VALUE_FROM_INTEGER(42)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(2), TOY_VALUE_FROM_INTEGER(69)); //hash: 0
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(3), TOY_VALUE_FROM_INTEGER(420)); //hash: 5

		//check the state
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 3 ||

			TEST_ENTRY_STATE(7, 1, 42, 0) ||
			TEST_ENTRY_STATE(0, 2, 69, 0) ||
			TEST_ENTRY_STATE(5, 3, 420, 0)
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unrecognized state from table data, multiple inserts, no collisions\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//multiple inserts, with collisions
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(5), TOY_VALUE_FROM_INTEGER(42)); //hash: 2
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(19), TOY_VALUE_FROM_INTEGER(69)); //hash: 2
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(37), TOY_VALUE_FROM_INTEGER(420)); //hash: 2
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(65), TOY_VALUE_FROM_INTEGER(8891)); //hash: 2

		//check the state
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 4 ||

			TEST_ENTRY_STATE(2, 5, 42, 0) ||
			TEST_ENTRY_STATE(3, 19, 69, 1) ||
			TEST_ENTRY_STATE(4, 37, 420, 2) ||
			TEST_ENTRY_STATE(5, 65, 8891, 3)
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unrecognized state from table data, muiltiple inserts, with collisions\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//multiple inserts, with collisions, modulo wrap
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(1), TOY_VALUE_FROM_INTEGER(42)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(14), TOY_VALUE_FROM_INTEGER(69)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(76), TOY_VALUE_FROM_INTEGER(420)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(80), TOY_VALUE_FROM_INTEGER(8891)); //hash: 7

		//check the state
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 4 ||

			TEST_ENTRY_STATE(7, 1, 42, 0) ||
			TEST_ENTRY_STATE(0, 14, 69, 1) ||
			TEST_ENTRY_STATE(1, 76, 420, 2) ||
			TEST_ENTRY_STATE(2, 80, 8891, 3)

			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unrecognized state from table data, muiltiple inserts, with collisions, modulo wrap\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//lookup, with collisions, modulo wrap
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(17), TOY_VALUE_FROM_INTEGER(42)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(33), TOY_VALUE_FROM_INTEGER(69)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(70), TOY_VALUE_FROM_INTEGER(420)); //hash: 7

		//lookup
		Toy_Value result = Toy_lookupTable(&table, TOY_VALUE_FROM_INTEGER(33));

		//check the state
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 3 ||

			TOY_VALUE_IS_INTEGER(result) != true ||
			TOY_VALUE_AS_INTEGER(result) != 69
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Bad result from table lookup with collisions and modulo wrap\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//multiple inserts, with collisions, modulo wrap, psl overlap
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(17), TOY_VALUE_FROM_INTEGER(42)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(33), TOY_VALUE_FROM_INTEGER(69)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(70), TOY_VALUE_FROM_INTEGER(420)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(35), TOY_VALUE_FROM_INTEGER(8891)); //hash: 1

		//check the state
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 4 ||

			TEST_ENTRY_STATE(7, 17, 42, 0) ||
			TEST_ENTRY_STATE(0, 33, 69, 1) ||
			TEST_ENTRY_STATE(1, 70, 420, 2) ||
			TEST_ENTRY_STATE(2, 35, 8891, 1)
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unrecognized state from table data, muiltiple inserts, with collisions, modulo wrap, psl overlap\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//multiple inserts, with collisions, modulo wrap, psl overlap, psl shift
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(17), TOY_VALUE_FROM_INTEGER(42)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(33), TOY_VALUE_FROM_INTEGER(69)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(70), TOY_VALUE_FROM_INTEGER(420)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(35), TOY_VALUE_FROM_INTEGER(8891)); //hash: 1

		//remove
		Toy_removeTable(&table, TOY_VALUE_FROM_INTEGER(33));

		//check the state
		if (table == NULL ||
			table->capacity != 8 ||
			table->count != 3 ||

			TEST_ENTRY_STATE(7, 17, 42, 0) ||
			TEST_ENTRY_STATE(0, 70, 420, 1) ||
			TEST_ENTRY_STATE(1, 35, 8891, 0)
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unrecognized state from table data, muiltiple inserts, with collisions, modulo wrap, psl overlap, psl shift\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	return 0;
}

int test_table_contents_with_expansions(void) {
	//simple expansion
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//insert a key and value
		for (int i = 0; i < 20; i++) {
			Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(i), TOY_VALUE_FROM_INTEGER(42));
		}

		//check the state
		if (table == NULL ||
			table->capacity != 32 ||
			table->count != 20
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to expand table capacity\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//expansion, multiple inserts, no collisions
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(0), TOY_VALUE_FROM_INTEGER(42)); //hash: 0
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(35), TOY_VALUE_FROM_INTEGER(42)); //hash: 1
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(19), TOY_VALUE_FROM_INTEGER(42)); //hash: 2
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(8), TOY_VALUE_FROM_INTEGER(42)); //hash: 3
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(10), TOY_VALUE_FROM_INTEGER(42)); //hash: 4
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(3), TOY_VALUE_FROM_INTEGER(42)); //hash: 5
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(28), TOY_VALUE_FROM_INTEGER(42)); //hash: 6
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(1), TOY_VALUE_FROM_INTEGER(42)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(6), TOY_VALUE_FROM_INTEGER(42)); //hash: 8
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(93), TOY_VALUE_FROM_INTEGER(42)); //hash: 9
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(85), TOY_VALUE_FROM_INTEGER(42)); //hash: 10
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(9), TOY_VALUE_FROM_INTEGER(42)); //hash: 11
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(11), TOY_VALUE_FROM_INTEGER(42)); //hash: 12
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(22), TOY_VALUE_FROM_INTEGER(42)); //hash: 13
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(13), TOY_VALUE_FROM_INTEGER(42)); //hash: 14
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(17), TOY_VALUE_FROM_INTEGER(42)); //hash: 15
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(43), TOY_VALUE_FROM_INTEGER(42)); //hash: 16
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(4), TOY_VALUE_FROM_INTEGER(42)); //hash: 17
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(5), TOY_VALUE_FROM_INTEGER(42)); //hash: 18
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(7), TOY_VALUE_FROM_INTEGER(42)); //hash: 19

		//check the state
		if (table == NULL ||
			table->capacity != 32 ||
			table->count != 20 ||

			//effectively, check that each key was placed in the correct position after the expansion
			TEST_ENTRY_STATE(0, 0, 42, 0) ||
			TEST_ENTRY_STATE(1, 35, 42, 0) ||
			TEST_ENTRY_STATE(2, 19, 42, 0) ||
			TEST_ENTRY_STATE(3, 8, 42, 0) ||
			TEST_ENTRY_STATE(4, 10, 42, 0) ||
			TEST_ENTRY_STATE(5, 3, 42, 0) ||
			TEST_ENTRY_STATE(6, 28, 42, 0) ||
			TEST_ENTRY_STATE(7, 1, 42, 0) ||
			TEST_ENTRY_STATE(8, 6, 42, 0) ||
			TEST_ENTRY_STATE(9, 93, 42, 0) ||
			TEST_ENTRY_STATE(10, 85, 42, 0) ||
			TEST_ENTRY_STATE(11, 9, 42, 0) ||
			TEST_ENTRY_STATE(12, 11, 42, 0) ||
			TEST_ENTRY_STATE(13, 22, 42, 0) ||
			TEST_ENTRY_STATE(14, 13, 42, 0) ||
			TEST_ENTRY_STATE(15, 17, 42, 0) ||
			TEST_ENTRY_STATE(16, 43, 42, 0) ||
			TEST_ENTRY_STATE(17, 4, 42, 0) ||
			TEST_ENTRY_STATE(18, 5, 42, 0) ||
			TEST_ENTRY_STATE(19, 7, 42, 0)
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unrecognized state from table data after expansion, multiple inserts, no collisions\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//multiple inserts, with collisions
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(0), TOY_VALUE_FROM_INTEGER(42)); //hash: 0
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(35), TOY_VALUE_FROM_INTEGER(42)); //hash: 1
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(19), TOY_VALUE_FROM_INTEGER(42)); //hash: 2
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(8), TOY_VALUE_FROM_INTEGER(42)); //hash: 3
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(10), TOY_VALUE_FROM_INTEGER(42)); //hash: 4
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(3), TOY_VALUE_FROM_INTEGER(42)); //hash: 5
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(28), TOY_VALUE_FROM_INTEGER(42)); //hash: 6
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(1), TOY_VALUE_FROM_INTEGER(42)); //hash: 7
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(6), TOY_VALUE_FROM_INTEGER(42)); //hash: 8
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(93), TOY_VALUE_FROM_INTEGER(42)); //hash: 9
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(85), TOY_VALUE_FROM_INTEGER(42)); //hash: 10
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(9), TOY_VALUE_FROM_INTEGER(42)); //hash: 11
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(11), TOY_VALUE_FROM_INTEGER(42)); //hash: 12
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(22), TOY_VALUE_FROM_INTEGER(42)); //hash: 13
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(13), TOY_VALUE_FROM_INTEGER(42)); //hash: 14
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(17), TOY_VALUE_FROM_INTEGER(42)); //hash: 15
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(43), TOY_VALUE_FROM_INTEGER(42)); //hash: 16
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(4), TOY_VALUE_FROM_INTEGER(42)); //hash: 17
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(5), TOY_VALUE_FROM_INTEGER(42)); //hash: 18
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(7), TOY_VALUE_FROM_INTEGER(42)); //hash: 19

		//insert one more
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(70), TOY_VALUE_FROM_INTEGER(42)); //hash: 15

		//check the state
		if (table == NULL ||
			table->capacity != 32 ||
			table->count != 21 ||

			TEST_ENTRY_STATE(0, 0, 42, 0) ||
			TEST_ENTRY_STATE(1, 35, 42, 0) ||
			TEST_ENTRY_STATE(2, 19, 42, 0) ||
			TEST_ENTRY_STATE(3, 8, 42, 0) ||
			TEST_ENTRY_STATE(4, 10, 42, 0) ||
			TEST_ENTRY_STATE(5, 3, 42, 0) ||
			TEST_ENTRY_STATE(6, 28, 42, 0) ||
			TEST_ENTRY_STATE(7, 1, 42, 0) ||
			TEST_ENTRY_STATE(8, 6, 42, 0) ||
			TEST_ENTRY_STATE(9, 93, 42, 0) ||
			TEST_ENTRY_STATE(10, 85, 42, 0) ||
			TEST_ENTRY_STATE(11, 9, 42, 0) ||
			TEST_ENTRY_STATE(12, 11, 42, 0) ||
			TEST_ENTRY_STATE(13, 22, 42, 0) ||
			TEST_ENTRY_STATE(14, 13, 42, 0) ||
			TEST_ENTRY_STATE(15, 17, 42, 0) ||

			TEST_ENTRY_STATE(16, 70, 42, 1) || //the collision

			TEST_ENTRY_STATE(17, 43, 42, 1) ||
			TEST_ENTRY_STATE(18, 4, 42, 1) ||
			TEST_ENTRY_STATE(19, 5, 42, 1) ||
			TEST_ENTRY_STATE(20, 7, 42, 1)
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unrecognized state from table data after expansion, muiltiple inserts, with collisions\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//multiple inserts, with collisions, modulo wrap
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(123), TOY_VALUE_FROM_INTEGER(42)); //hash: 20
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(42), TOY_VALUE_FROM_INTEGER(42)); //hash: 21
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(132), TOY_VALUE_FROM_INTEGER(42)); //hash: 22
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(34), TOY_VALUE_FROM_INTEGER(42)); //hash: 23
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(2), TOY_VALUE_FROM_INTEGER(42)); //hash: 24
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(32), TOY_VALUE_FROM_INTEGER(42)); //hash: 25
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(21), TOY_VALUE_FROM_INTEGER(42)); //hash: 26
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(44), TOY_VALUE_FROM_INTEGER(42)); //hash: 27
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(104), TOY_VALUE_FROM_INTEGER(42)); //hash: 28
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(15), TOY_VALUE_FROM_INTEGER(42)); //hash: 29
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(57), TOY_VALUE_FROM_INTEGER(42)); //hash: 30
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(33), TOY_VALUE_FROM_INTEGER(42)); //hash: 31
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(0), TOY_VALUE_FROM_INTEGER(42)); //hash: 32
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(35), TOY_VALUE_FROM_INTEGER(42)); //hash: 33
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(19), TOY_VALUE_FROM_INTEGER(42)); //hash: 34
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(8), TOY_VALUE_FROM_INTEGER(42)); //hash: 35
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(10), TOY_VALUE_FROM_INTEGER(42)); //hash: 36
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(3), TOY_VALUE_FROM_INTEGER(42)); //hash: 37
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(28), TOY_VALUE_FROM_INTEGER(42)); //hash: 38
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(1), TOY_VALUE_FROM_INTEGER(42)); //hash: 39

		//insert one more
		Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(79), TOY_VALUE_FROM_INTEGER(42)); //hash: 23

		//check the state
		if (table == NULL ||
			table->capacity != 32 ||
			table->count != 21 ||

			TEST_ENTRY_STATE(20, 123, 42, 0) ||
			TEST_ENTRY_STATE(21, 42, 42, 0) ||
			TEST_ENTRY_STATE(22, 132, 42, 0) ||
			TEST_ENTRY_STATE(23, 34, 42, 0) ||

			TEST_ENTRY_STATE(24, 79, 42, 1) || //the collision

			TEST_ENTRY_STATE(25, 2, 42, 1) ||
			TEST_ENTRY_STATE(26, 32, 42, 1) ||
			TEST_ENTRY_STATE(27, 21, 42, 1) ||
			TEST_ENTRY_STATE(28, 44, 42, 1) ||
			TEST_ENTRY_STATE(29, 104, 42, 1) ||
			TEST_ENTRY_STATE(30, 15, 42, 1) ||
			TEST_ENTRY_STATE(31, 57, 42, 1) ||
			TEST_ENTRY_STATE(0, 33, 42, 1) ||
			TEST_ENTRY_STATE(1, 0, 42, 1) ||
			TEST_ENTRY_STATE(2, 35, 42, 1) ||
			TEST_ENTRY_STATE(3, 19, 42, 1) ||
			TEST_ENTRY_STATE(4, 8, 42, 1) ||
			TEST_ENTRY_STATE(5, 10, 42, 1) ||
			TEST_ENTRY_STATE(6, 3, 42, 1) ||
			TEST_ENTRY_STATE(7, 28, 42, 1) ||
			TEST_ENTRY_STATE(8, 1, 42, 1)
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unrecognized state from table data after expansion, muiltiple inserts, with collisions, modulo wrap\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//lookup, with collisions, modulo wrap
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		//inserts
		for (int i = 0; i < 20; i++) { //enough to expand
			Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(i), TOY_VALUE_FROM_INTEGER(100 - i));
		}

		//lookup
		Toy_Value result = Toy_lookupTable(&table, TOY_VALUE_FROM_INTEGER(15));

		//check the state
		if (table == NULL ||
			table->capacity != 32 ||
			table->count != 20 ||

			TOY_VALUE_IS_INTEGER(result) != true ||
			TOY_VALUE_AS_INTEGER(result) != 85
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Bad result from table lookup after expansion, with collisions and modulo wrap\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	//Skipped: multiple inserts, with collisions, modulo wrap, psl overlap
	//Skipped: multiple inserts, with collisions, modulo wrap, psl overlap, psl shift

	//Note: since psl overlap and psl shift both work without expansion, I'm leaving these tests unimplemented.

	return 0;
}

int test_table_expansions_under_stress(void) {
	//multiple expansions, find one value
	{
		//setup
		Toy_Table* table = Toy_allocateTable();

		int top = 300;

		//insert keys and values
		for (int i = 0; i < 400; i++) {
			Toy_insertTable(&table, TOY_VALUE_FROM_INTEGER(i), TOY_VALUE_FROM_INTEGER(top - i));
		}

		Toy_Value result = Toy_lookupTable(&table, TOY_VALUE_FROM_INTEGER(265));

		//check the state
		if (table == NULL ||
			table->capacity != 512 ||
			table->count != 400 ||

			TOY_VALUE_IS_INTEGER(result) != true ||
			TOY_VALUE_AS_INTEGER(result) != 35
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Table expansions under stress failed\n" TOY_CC_RESET);
			Toy_freeTable(table);
			return -1;
		}

		//free
		Toy_freeTable(table);
	}

	return 0;
}

int main(void) {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_table_allocation();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_table_simple_insert_lookup_and_remove();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_table_contents_no_expansion();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_table_contents_with_expansions();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_table_expansions_under_stress();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
