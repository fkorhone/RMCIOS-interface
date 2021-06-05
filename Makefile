GCC?=gcc
TEST_DIR?=RMCIOS-test
TEST_NAME=test_functions

test: build_test
	${TEST_NAME}.exe

build_test:
	$(GCC) test_functions.c RMCIOS-test/test.c -I${TEST_DIR} -o ${TEST_NAME}.exe

