/********************************************************************
 *
 *	File Name: assertion_main.c
 *	
 *	Description: Local assertion definition (tentative!)
 *
 *	Notes: (C) Copyright 2009 Sony Corporation
 *
 *	Author: Tomonobu Hayakawa
 *
 ********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>

#ifdef ASSERT_USE_RETURN_ADDR
void _AssertionFail(const char* exp, const char* filename, int line, void* ret_addr)
{
	printf("%s, Assert at %s line:%d, ret_addr:%08x\n", exp, filename, line, ret_addr);
	__breakpoint(1);
}
#else  /* ASSERT_USE_RETURN_ADDR */
void _AssertionFail(const char* exp, const char* filename, int line)
{
	printf("%s, Assert at %s line:%d\n", exp, filename, line);
	//__breakpoint(1);
	while (1);
}
#endif /* ASSERT_USE_RETURN_ADDR */
