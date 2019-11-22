/************************************************
 * mocked.c
 * parseArguments_mocked
 ************************************************/
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

__attribute__((weak))
_Bool parseArguments(int argc, char *argv[]){
	check_expected(argc);
	check_expected_ptr(argv);
	char *strCommandMocked = argv[0];
	return 1;
}

_Bool _wrap_parseArguments(int argc, char *argv[]){
	check_expected(argc);
	check_expected_ptr(argv);
	char *strCommandMocked = argv[0];
	return 1;
}
