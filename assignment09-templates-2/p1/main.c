#include "testlib.h"
#include "malloc.h"

int main()
{
	test_start("malloc.c");

	int res = allocator_test();
	if(res==0) test_failed_message("alloctor_test returned false, seems like something is wrong");

	return test_end();
}
