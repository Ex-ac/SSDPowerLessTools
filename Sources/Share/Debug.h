/**
 * @file Debug.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once


#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
//  Include files:
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// WARNING: don't include Global.h

//-----------------------------------------------------------------------------
//  Constant definitions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Macros definitions:
//-----------------------------------------------------------------------------
#define DebugPrint(fmt, argc...)	\
	fprintf(stdout, "%s:" fmt "\n", __FUNCTION__, ##argc)

#define ASSERT(x)			assert(x)
#define ASSERT_DEBUG(x)		assert(x)

//-----------------------------------------------------------------------------
//  Data type definitions: typedef, struct or class
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Public interface functions:
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//  Inline functions
//-----------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif