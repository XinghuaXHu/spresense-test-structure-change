#include <stdio.h>
#include <stdlib.h>
#include "stub/assertion.h"

int AssertionFailed = 0;
const char* AssertionFailed_exp = NULL;
const char* AssertionFailed_filename = NULL;
int AssertionFailed_line = 0;
void _AssertionFail(const char* exp, const char* filename, int line)
{
	fprintf(stderr,"%s, Assert at  file:%s  line:%d\n",exp,filename,line);
	//exit(1);
	AssertionFailed_exp = exp;
	AssertionFailed_filename = filename;
	AssertionFailed_line = line;
	AssertionFailed++;
}

