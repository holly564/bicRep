/************************************************
 * parse_ut.c
 * 
 ************************************************/
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

//bool parse_ut (void **state);
void test(void **state){
	assert_int_equal(2,2);
}

void parse_ut (void **state){
	char *msg = "test";
	assert_string_equal("test", msg);
}

int main (void){
	const struct CMUnitTest tests[] = 
	{
		cmocka_unit_test(test),
		cmocka_unit_test(parse_ut),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
