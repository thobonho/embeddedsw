/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xenv_linux.h
* @addtogroup common_v1_00_a
* @{
*
* Defines common services specified by xenv.h.
*
* @note
* 	This file is not intended to be included directly by driver code.
* 	Instead, the generic xenv.h file is intended to be included by driver
* 	code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a wgr  02/28/07 Added cache handling macros.
* 1.00a wgr  02/27/07 Simplified code. Deprecated old-style macro names.
* 1.00a xd   11/03/04 Improved support for doxygen.
* 1.00a ch   10/24/02 First release
* 1.10a wgr  03/22/07 Converted to new coding style.
* </pre>
*
*
******************************************************************************/

#ifndef XENV_LINUX_H
#define XENV_LINUX_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

#include <asm/cache.h>
#include <asm/cacheflush.h>
#include <linux/string.h>
#include <linux/delay.h>


/******************************************************************************
 *
 * MEMCPY / MEMSET related macros.
 *
 * Those macros are defined to catch legacy code in Xilinx drivers. The
 * XENV_MEM_COPY and XENV_MEM_FILL macros were used in early Xilinx driver
 * code. They are being replaced by memcpy() and memset() function calls. These
 * macros are defined to catch any remaining occurences of those macros.
 *
 ******************************************************************************/

/*****************************************************************************/
/**
 *
 * Copies a non-overlapping block of memory.
 *
 * @param	DestPtr
 *		Destination address to copy data to.
 *
 * @param	SrcPtr
 * 		Source address to copy data from.
 *
 * @param	Bytes
 * 		Number of bytes to copy.
 *
 * @return	None.
 *
 *****************************************************************************/

#define XENV_MEM_COPY(DestPtr, SrcPtr, Bytes) \
		memcpy(DestPtr, SrcPtr, Bytes)
/*		do_not_use_XENV_MEM_COPY_use_memcpy_instead */


/*****************************************************************************/
/**
 *
 * Fills an area of memory with constant data.
 *
 * @param	DestPtr
 *		Destination address to copy data to.
 *
 * @param	Data
 * 		Value to set.
 *
 * @param	Bytes
 * 		Number of bytes to copy.
 *
 * @return	None.
 *
 *****************************************************************************/

#define XENV_MEM_FILL(DestPtr, Data, Bytes) \
		memset(DestPtr, Data, Bytes)
/*		do_not_use_XENV_MEM_FILL_use_memset_instead */


/******************************************************************************
 *
 * TIME related macros
 *
 ******************************************************************************/
/**
 * A structure that contains a time stamp used by other time stamp macros
 * defined below. This structure is processor dependent.
 */
typedef int XENV_TIME_STAMP;

/*****************************************************************************/
/**
 *
 * Time is derived from the 64 bit PPC timebase register
 *
 * @param   StampPtr is the storage for the retrieved time stamp.
 *
 * @return  None.
 *
 * @note
 *
 * Signature: void XENV_TIME_STAMP_GET(XTIME_STAMP *StampPtr)
 * <br><br>
 * This macro must be implemented by the user.
 *
 *****************************************************************************/
#define XENV_TIME_STAMP_GET(StampPtr)

/*****************************************************************************/
/**
 *
 * This macro is not yet implemented and always returns 0.
 *
 * @param   Stamp1Ptr is the first sampled time stamp.
 * @param   Stamp2Ptr is the second sampled time stamp.
 *
 * @return  0
 *
 * @note
 *
 * This macro must be implemented by the user.
 *
 *****************************************************************************/
#define XENV_TIME_STAMP_DELTA_US(Stamp1Ptr, Stamp2Ptr)     (0)

/*****************************************************************************/
/**
 *
 * This macro is not yet implemented and always returns 0.
 *
 * @param   Stamp1Ptr is the first sampled time stamp.
 * @param   Stamp2Ptr is the second sampled time stamp.
 *
 * @return  0
 *
 * @note
 *
 * This macro must be implemented by the user
 *
 *****************************************************************************/
#define XENV_TIME_STAMP_DELTA_MS(Stamp1Ptr, Stamp2Ptr)     (0)

/*****************************************************************************/
/**
 *
 * Delay the specified number of microseconds.
 *
 * @param	delay
 * 		Number of microseconds to delay.
 *
 * @return	None.
 *
 * @note	XENV_USLEEP is deprecated. Use udelay() instead.
 *
 *****************************************************************************/

#define XENV_USLEEP(delay)	udelay(delay)
/*		do_not_use_XENV_MEM_COPY_use_memcpy_instead */


/******************************************************************************
 *
 * CACHE handling macros / mappings
 *
 * The implementation of the cache handling functions can be found in
 * arch/microblaze.
 *
 * These #defines are simple mappings to the Linux API.
 *
 * The underlying Linux implementation will take care of taking the right
 * actions depending on the configuration of the MicroBlaze processor in the
 * system.
 *
 ******************************************************************************/

#define XCACHE_ENABLE_DCACHE()		__enable_dcache()
#define XCACHE_DISABLE_DCACHE()		__disable_dcache()
#define XCACHE_ENABLE_ICACHE()		__enable_icache()
#define XCACHE_DISABLE_ICACHE()		__disable_icache()

#define XCACHE_INVALIDATE_DCACHE_RANGE(Addr, Len) invalidate_dcache_range((u32)(Addr), ((u32)(Addr)+(Len)))
#define XCACHE_FLUSH_DCACHE_RANGE(Addr, Len)      flush_dcache_range((u32)(Addr), ((u32)(Addr)+(Len)))

#define XCACHE_INVALIDATE_ICACHE_RANGE(Addr, Len) "XCACHE_INVALIDATE_ICACHE_RANGE unsupported"
#define XCACHE_FLUSH_ICACHE_RANGE(Addr, Len)      flush_icache_range(Addr, Len)

#define XCACHE_ENABLE_CACHE()	\
		{ XCACHE_ENABLE_DCACHE(); XCACHE_ENABLE_ICACHE(); }

#define XCACHE_DISABLE_CACHE()	\
		{ XCACHE_DISABLE_DCACHE(); XCACHE_DISABLE_ICACHE(); }



#ifdef __cplusplus
}
#endif

#endif            /* end of protection macro */

/** @} */
