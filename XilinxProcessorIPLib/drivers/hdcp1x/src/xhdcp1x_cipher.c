/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xhdcp1x_cipher.c
*
* This file contains the main implementation of the driver associated with
* the Xilinx HDCP Cipher core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* 1.01         07/23/15 Additional documentation and formating
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>
#include <string.h>
#include "xhdcp1x.h"
#include "xhdcp1x_cipher_hw.h"
#include "xhdcp1x_cipher.h"
#include "xil_assert.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* This function performs register read from a cipher
*
* @param InstancePtr  the instance to read from
* @param Reg  the register to read
*
* @return
*   The current contents of the indicated register
*
* @note
*   None.
*
******************************************************************************/
#define RegRead(InstancePtr, Reg)  \
	XHdcp1x_CipherReadReg(InstancePtr->CfgPtr->BaseAddress, Reg)

/*****************************************************************************/
/**
*
* This function performs register write to a cipher
*
* @param InstancePtr  the instance to write to
* @param Reg  the register to write
* @param Value  the value to write
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
#define RegWrite(InstancePtr, Reg, Value)  \
	XHdcp1x_CipherWriteReg(InstancePtr->CfgPtr->BaseAddress, Reg, Value)

/*****************************************************************************/
/**
*
* This queries a cipher to determine if it is Display Port (DP)
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating DP (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define IsDP(InstancePtr)  \
	XHdcp1x_CipherIsDP(InstancePtr)

/*****************************************************************************/
/**
*
* This queries a cipher to determine if it is HDMI
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating HDMI (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define IsHDMI(InstancePtr)  \
	XHdcp1x_CipherIsHDMI(InstancePtr)

/*****************************************************************************/
/**
*
* This queries a cipher to determine if it is a receiver
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating receiver (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define IsRX(InstancePtr)  \
	XHdcp1x_CipherIsRX(InstancePtr)

/*****************************************************************************/
/**
*
* This queries a cipher to determine if it is a transmitter
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating transmitter (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define IsTX(InstancePtr)  \
	XHdcp1x_CipherIsTX(InstancePtr)

/*****************************************************************************/
/**
*
* This queries a cipher to determine if it is enabled
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating transmitter (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define IsEnabled(InstancePtr)  \
	((RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL) & \
	XHDCP1X_CIPHER_BITMASK_CONTROL_ENABLE) != 0)

/*****************************************************************************/
/**
*
* This queries a cipher to determine if the XOR (encryption) function is
* currently in progress
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating in progress (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define XorInProgress(InstancePtr)  \
	((RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_STATUS) & \
	XHDCP1X_CIPHER_BITMASK_CIPHER_STATUS_XOR_IN_PROG) != 0)

/*****************************************************************************/
/**
*
* This queries a cipher to determine if the local KSV is ready to read
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating ready (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define LocalKsvReady(InstancePtr)  \
	((RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_STATUS) & \
	XHDCP1X_CIPHER_BITMASK_KEYMGMT_STATUS_KSV_READY) != 0)

/*****************************************************************************/
/**
*
* This queries a cipher to determine if the Km value is ready
*
* @param InstancePtr  the instance to query
*
* @return
*   Truth value indicating ready (TRUE) or not (FALSE)
*
* @note
*   None.
*
******************************************************************************/
#define KmReady(InstancePtr)  \
	((RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_STATUS) & \
	XHDCP1X_CIPHER_BITMASK_KEYMGMT_STATUS_Km_READY) != 0)

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function enables a hdcp cipher
*
* @param InstancePtr  the device to enable
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void Enable(XHdcp1x_Cipher* InstancePtr)
{
	u32 Value = 0;

	/* Clear the register update bit */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Ensure that all encryption is disabled for now */
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_H, 0x00ul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_L, 0x00ul);

	/* Ensure that XOR is disabled on tx and enabled for rx to start */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CIPHER_CONTROL_XOR_ENABLE;
	if (IsRX(InstancePtr)) {
		Value |= XHDCP1X_CIPHER_BITMASK_CIPHER_CONTROL_XOR_ENABLE;
	}
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL, Value);

	/* Enable it */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_ENABLE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Ensure that the register update bit is set */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	return;
}

/*****************************************************************************/
/**
*
* This function disables a hdcp cipher
*
* @param InstancePtr  the device to disable
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void Disable(XHdcp1x_Cipher* InstancePtr)
{
	u32 Value = 0;

	/* Ensure all interrupts are disabled */
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_INTERRUPT_MASK, 0xFFFFFFFFul);

	/* Enable bypass operation */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_ENABLE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Ensure that all encryption is disabled for now */
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_H, 0x00ul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_L, 0x00ul);

	/* Ensure that XOR is disabled */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CIPHER_CONTROL_XOR_ENABLE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL, Value);

	/* Ensure that the register update bit is set */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Wait until the XOR has actually stopped */
	while (XorInProgress(InstancePtr));

	return;
}

/*****************************************************************************/
/**
*
* This function initializes a hdcp cipher
*
* @param InstancePtr  the device to initialize
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
static void Init(XHdcp1x_Cipher* InstancePtr)
{
	u32 Value = 0;

	/* Reset it */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_RESET;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_RESET;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Ensure all interrupts are disabled and cleared */
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_INTERRUPT_MASK, (u32) (-1));
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_INTERRUPT_STATUS, (u32) (-1));

	/* Check for DP */
	if (IsDP(InstancePtr)) {

		/* Configure for four lanes SST */
		Value  = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
		Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_NUM_LANES;
		Value |= (4u << 4);
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);
	}

	/* Ensure that the register update bit is set */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	return;
}

/*****************************************************************************/
/**
*
* This function initializes a cipher device
*
* @param InstancePtr  the device to initialize
* @param CfgPtr  the device configuration
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherCfgInitialize(XHdcp1x_Cipher* InstancePtr,
		const XHdcp1x_Config* CfgPtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady != XIL_COMPONENT_IS_READY);

	/* Initialize InstancePtr */
	memset(InstancePtr, 0, sizeof(XHdcp1x_Cipher));
	InstancePtr->CfgPtr = CfgPtr;

	/* Check for mismatch on direction */
	if (IsRX(InstancePtr)) {
		 if (!(CfgPtr->IsRx)) {
			 Status = XST_FAILURE;
		 }
	}
	else if (CfgPtr->IsRx) {
		Status = XST_FAILURE;
	}

	/* Check for mismatch on protocol */
	if (IsHDMI(InstancePtr)) {
		if (!(CfgPtr->IsHDMI)) {
			Status = XST_FAILURE;
		}
	}
	else if (CfgPtr->IsHDMI) {
		Status = XST_FAILURE;
	}

	/* Initialize it */
	if (Status == XST_SUCCESS) {
		Init(InstancePtr);
		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function queries the link state of a cipher device
*
* @param InstancePtr  the device to query
*
* @return
*   Truth value
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherIsLinkUp(const XHdcp1x_Cipher* InstancePtr)
{
	int isUp = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for currently enabled */
	if (IsEnabled(InstancePtr)) {

		u32 Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_STATUS);
		if ((Value & XHDCP1X_CIPHER_BITMASK_INTERRUPT_LINK_FAIL) != 0)
			isUp = TRUE;
	}

	return (isUp);
}

/*****************************************************************************/
/**
*
* This function enables a hdcp cipher
*
* @param InstancePtr  the device to enable
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherEnable(XHdcp1x_Cipher* InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for currently disabled */
	if (!IsEnabled(InstancePtr)) {
		Enable(InstancePtr);
	}
	else {
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function disables a hdcp cipher
*
* @param InstancePtr  the device to disable
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherDisable(XHdcp1x_Cipher* InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for currently enabled */
	if (IsEnabled(InstancePtr)) {
		Disable(InstancePtr);
	}

	/* Return */
	return (Status);
}

/*****************************************************************************/
/**
*
* This function configures the key selection value
*
* @param InstancePtr  the device to configure
* @param KeySelect  the desired key select value
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherSetKeySelect(XHdcp1x_Cipher* InstancePtr, u8 KeySelect)
{
	int Status = XST_SUCCESS;
	u32 Value = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(KeySelect < 8);

	/* Update the device */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_KEYMGMT_CONTROL_SET_SELECT;
	Value |= (KeySelect << 16);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL, Value);

	return (Status);
}

/*****************************************************************************/
/**
*
* This function initiates a request within the hdcp cipher
*
* @param InstancePtr  the device to submit the request to
* @param Request  the request to submit
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherDoRequest(XHdcp1x_Cipher* InstancePtr,
		XHdcp1x_CipherRequestType Request)
{
	u32 Value = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Request >= (XHDCP1X_CIPHER_REQUEST_BLOCK));
//	Xil_AssertNonvoid(Request <  (XHDCP1X_CIPHER_REQUEST_MAX));

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Determine if there is a request in progress */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_STATUS);
	Value &= XHDCP1X_CIPHER_BITMASK_CIPHER_STATUS_REQUEST_IN_PROG;

	/* Check that it is not busy */
	if (Value != 0) {
		return (XST_DEVICE_BUSY);
	}

	/* Ensure that the register update bit is set */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Set the appropriate request bit and ensure that Km is always used */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CIPHER_CONTROL_REQUEST;
	Value |= (XHDCP1X_CIPHER_VALUE_CIPHER_CONTROL_REQUEST_BLOCK<<Request);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL, Value);

	/* Ensure that the request bit(s) get cleared for next time */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CIPHER_CONTROL_REQUEST;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL, Value);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function queries the progress of the current request
*
* @param InstancePtr  the device to query
*
* @return
*   Truth value
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherIsRequestComplete(const XHdcp1x_Cipher* InstancePtr)
{
	u32 Value = 0;
	int IsComplete = TRUE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Value */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_STATUS);
	Value &= XHDCP1X_CIPHER_BITMASK_CIPHER_STATUS_REQUEST_IN_PROG;

	/* Update IsComplete */
	if (Value != 0) {
		IsComplete = FALSE;
	}

	return (IsComplete);
}

/*****************************************************************************/
/**
*
* This function retrieves the current number of lanes of the hdcp cipher
*
* @param InstancePtr  the device to query
*
* @return
*   The current number of lanes
*
* @note
*   None.
*
******************************************************************************/
u32 XHdcp1x_CipherGetNumLanes(const XHdcp1x_Cipher* InstancePtr)
{
	/* Locals */
	u32 NumLanes = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for currently enabled */
	if (IsEnabled(InstancePtr)) {

		/* Determine NumLanes */
		NumLanes = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
		NumLanes &= XHDCP1X_CIPHER_BITMASK_CONTROL_NUM_LANES;
		NumLanes >>= 4;
	}

	/* Return */
	return (NumLanes);
}

/*****************************************************************************/
/**
*
* This function configures the number of lanes of the hdcp cipher
*
* @param InstancePtr  the device to configure
* @param NumLanes  the number of lanes to configure
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherSetNumLanes(XHdcp1x_Cipher* InstancePtr, u32 NumLanes)
{
	int Status = XST_SUCCESS;
	u32 Value = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(NumLanes > 0);
	Xil_AssertNonvoid(NumLanes <= 4);

	/* Check for HDMI */
	if (IsHDMI(InstancePtr)) {

		/* Verify NumLanes (again) */
		Xil_AssertNonvoid(NumLanes == 1);

		/* Update the control register */
		Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
		Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_NUM_LANES;
		Value |= (NumLanes << 4);
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	}
	/* Otherwise - must be DP */
	else {

		/* Verify NumLanes (again) */
		Xil_AssertNonvoid(NumLanes != 3);

		/* Update the control register */
		Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
		Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_NUM_LANES;
		Value |= (NumLanes << 4);
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function retrieves the current encryption stream map
*
* @param InstancePtr  the device to query
*
* @return
*   The current encryption stream map.
*
* @note
*   In the case of the receiver version of this core, the XOR in progress bit
*   needs to be checked as well as the encryption map to fully determine if
*   encryption is enabled for the SST case.  This is reason for the additional
*   check in this code
*
******************************************************************************/
u64 XHdcp1x_CipherGetEncryption(const XHdcp1x_Cipher* InstancePtr)
{
	u64 StreamMap = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (StreamMap);
	}

	/* Determine StreamMap */
	StreamMap  = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_H);
	StreamMap <<= 32;
	StreamMap |= RegRead(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_L);

	/* Check for special case of just XOR in progress */
	if ((StreamMap == 0) && (XorInProgress(InstancePtr))) {
		StreamMap = 0x01ul;
	}

	return (StreamMap);
}

/*****************************************************************************/
/**
*
* This function enables encryption on a set of streams
*
* @param InstancePtr  the device to configure
* @param StreamMap  the bit map of streams to enable encryption on
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherEnableEncryption(XHdcp1x_Cipher* InstancePtr, u64 StreamMap)
{
	u32 Value = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Check that it is not a receiver */
	if (IsRX(InstancePtr)) {
		return (XST_FAILURE);
	}

	/* Check for nothing to do */
	if (StreamMap == 0) {
		return (XST_SUCCESS);
	}

	/* Clear the register update bit */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Update the LS 32-bits */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_L);
	Value |= ((u32) (StreamMap & 0xFFFFFFFFul));
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_L, Value);

	/* Write the MS 32-bits */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_H);
	Value |= ((u32) ((StreamMap >> 32) & 0xFFFFFFFFul));
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_H, Value);

	/* Ensure that the XOR is enabled */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CIPHER_CONTROL_XOR_ENABLE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL, Value);

	/* Set the register update bit */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Wait until the XOR has actually started */
	while (!XorInProgress(InstancePtr));

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function disables encryption on a set of streams
*
* @param InstancePtr  the device to configure
* @param StreamMap  the bit map of streams to disable encryption on
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_CipherDisableEncryption(XHdcp1x_Cipher* InstancePtr, u64 StreamMap)
{
	u32 Val = 0;
	int DisableXor = TRUE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Check that it is not a receiver */
	if (IsRX(InstancePtr)) {
		return (XST_FAILURE);
	}

	/* Check for nothing to do */
	if (StreamMap == 0) {
		return (XST_SUCCESS);
	}

	/* Clear the register update bit */
	Val = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Val &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Val);

	/* Update the LS 32-bits */
	Val = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_L);
	Val &= ~((u32) (StreamMap & 0xFFFFFFFFul));
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_L, Val);
	if (Val != 0) {
		DisableXor = FALSE;
	}

	/* Write the MS 32-bits */
	Val = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_H);
	Val &= ~((u32) ((StreamMap >> 32) & 0xFFFFFFFFul));
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_ENCRYPT_ENABLE_H, Val);
	if (Val != 0) {
		DisableXor = FALSE;
	}

	/* Check HDMI special case */
	if (IsHDMI(InstancePtr)) {
		DisableXor = TRUE;
	}

	/* Check for XOR disable */
	if (DisableXor) {
		Val = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL);
		Val &= ~XHDCP1X_CIPHER_BITMASK_CIPHER_CONTROL_XOR_ENABLE;
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_CONTROL, Val);
	}

	/* Set the register update bit */
	Val = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Val |= XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Val);

	/* If disabling the XOR, wait until no longer in progress */
	if (DisableXor) {
		while (XorInProgress(InstancePtr));
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function reads the local KSV value from the cipher
*
* @param InstancePtr  the device to query
*
 * @return
*   The local KSV value
*
* @note
*   None.
*
******************************************************************************/
u64 XHdcp1x_CipherGetLocalKsv(const XHdcp1x_Cipher* InstancePtr)
{
	u32 Val = 0;
	u32 Guard = 0x400ul;
	u64 Ksv = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (Ksv);
	}

	/* Check if the local ksv is not available */
	Val  = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_STATUS);
	Val &= XHDCP1X_CIPHER_BITMASK_KEYMGMT_STATUS_KSV_READY;
	if (Val == 0) {

		/* Abort any running Km calculation just in case */
		Val = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL);
		Val |= XHDCP1X_CIPHER_BITMASK_KEYMGMT_CONTROL_ABORT_Km;
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL, Val);
		Val &= ~XHDCP1X_CIPHER_BITMASK_KEYMGMT_CONTROL_ABORT_Km;
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL, Val);

		/* Load the local ksv */
		Val = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL);
		Val |= XHDCP1X_CIPHER_BITMASK_KEYMGMT_CONTROL_LOCAL_KSV;
		RegWrite(InstancePtr,  XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL, Val);
		Val &= ~XHDCP1X_CIPHER_BITMASK_KEYMGMT_CONTROL_LOCAL_KSV;
		RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL, Val);

		/* Wait until local KSV available */
		while ((!LocalKsvReady(InstancePtr)) && (--Guard > 0));
	}

	/* Confirm no timeout */
	if (Guard != 0) {

		/* Update Ksv */
		Ksv = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KSV_LOCAL_H);
		Ksv &= 0xFFul;
		Ksv <<= 32;
		Ksv |= RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KSV_LOCAL_L);
	}

	return (Ksv);
}

/*****************************************************************************/
/**
*
* This function reads the remote KSV value from the cipher
*
* @param InstancePtr  the device to query
*
* @return
*   The remote KSV value
*
* @note
*   None.
*
******************************************************************************/
u64 XHdcp1x_CipherGetRemoteKsv(const XHdcp1x_Cipher* InstancePtr)
{
	/* Locals */
	u64 Ksv = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Ksv */
	Ksv = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KSV_REMOTE_H);
	Ksv <<= 32;
	Ksv |= RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KSV_REMOTE_L);

	/* Return */
	return (Ksv);
}

/*****************************************************************************/
/**
*
* This function writes the remote KSV value to the cipher
*
* @param InstancePtr  the device to write to
* @param Ksv  the remote KSV value to write
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   Whenever this function is called, the underlying driver will initiate
*   the calculation of the Km value and wait for it to complete.
*
******************************************************************************/
int XHdcp1x_CipherSetRemoteKsv(XHdcp1x_Cipher* InstancePtr, u64 Ksv)
{
	u32 Value = 0;
	u32 Guard = 0x400ul;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Read local ksv to put things into a known state */
	XHdcp1x_CipherGetLocalKsv(InstancePtr);

	/* Clear the register update bit */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Write the LS 32-bits */
	Value = (u32) (Ksv & 0xFFFFFFFFul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_KSV_REMOTE_L, Value);

	/* Write the MS 8-bits */
	Value = (u32) ((Ksv >> 32) & 0xFFul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_KSV_REMOTE_H, Value);

	/* Set the register update bit */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Trigger the calculation of theKm */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL);
	Value &= 0xFFFFFFF0ul;
	Value |= XHDCP1X_CIPHER_BITMASK_KEYMGMT_CONTROL_BEGIN_Km;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL, Value);
	Value &= ~XHDCP1X_CIPHER_BITMASK_KEYMGMT_CONTROL_BEGIN_Km;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_KEYMGMT_CONTROL, Value);

	/* Wait until Km is available */
	while ((!KmReady(InstancePtr)) && (--Guard > 0));

	/* Check for timeout */
	if (Guard == 0) {
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/

/**
*
* This function reads the contents of the B register in BM0
*
* @param InstancePtr  the device to query
* @param X  to be loaded with the contents of Bx
* @param Y  to be loaded with the contents of By
* @param Z  to be loaded with the contents of Bz
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   A NULL pointer can be passed in any of X, Y and Z.  If so, then
*   this portion of the B register is not returned to the caller.
*
******************************************************************************/
int XHdcp1x_CipherGetB(const XHdcp1x_Cipher* InstancePtr, u32* X, u32* Y,
		u32* Z)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Get X if requested */
	if (X != NULL) {
		*X = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Bx);
		*X &= 0x0FFFFFFFul;
	}

	/* Get Y if requested */
	if (Y != NULL) {
		*Y = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_By);
		*Y &= 0x0FFFFFFFul;
	}

	/* Get Z if requested */
	if (Z != NULL) {
		*Z = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Bz);
		*Z &= 0x0FFFFFFFul;
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function writes the contents of the B register in BM0
*
* @param InstancePtr  the device to write to
* @param X  the value to be written to Bx
* @param Y  the value to be written to By
* @param Z  the value to be written to Bz
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None
*
******************************************************************************/
int XHdcp1x_CipherSetB(XHdcp1x_Cipher* InstancePtr, u32 X, u32 Y, u32 Z)
{
	u32 Value = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Clear the register update bit */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Update the Bx */
	Value = (X & 0x0FFFFFFFul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Bx, Value);

	/* Update the By */
	Value = (Y & 0x0FFFFFFFul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_By, Value);

	/* Update the Bz */
	Value = (Z & 0x0FFFFFFFul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Bz, Value);

	/* Set the register update bit */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function reads the contents of the K register in BM0
*
* @param InstancePtr  the device to query
* @param X  to be loaded with the contents of Kx
* @param Y  to be loaded with the contents of Ky
* @param Z  to be loaded with the contents of Kz
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   A NULL pointer can be passed in any of X, Y and Z.  If so, then
*   this portion of the K register is not returned to the caller.
*
******************************************************************************/
int XHdcp1x_CipherGetK(const XHdcp1x_Cipher* InstancePtr, u32* X, u32* Y,
		u32* Z)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Get X if requested */
	if (X != NULL) {
		*X = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Kx);
		*X &= 0x0FFFFFFFul;
	}

	/* Get Y if requested */
	if (Y != NULL) {
		*Y = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Ky);
		*Y &= 0x0FFFFFFFul;
	}

	/* Get Z if requested */
	if (Z != NULL) {
		*Z = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Kz);
		*Z &= 0x0FFFFFFFul;
	}

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function writes the contents of the K register in BM0
*
* @param InstancePtr  the device to write to
* @param X  the value to be written to Kx
* @param Y  the value to be written to Ky
* @param Z  the value to be written to Kz
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None
*
******************************************************************************/
int XHdcp1x_CipherSetK(XHdcp1x_Cipher* InstancePtr, u32 X, u32 Y, u32 Z)
{
	u32 Value = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Clear the register update bit */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value &= ~XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	/* Update the Kx */
	Value = (X & 0x0FFFFFFFul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Kx, Value);

	/* Update the Ky */
	Value = (Y & 0x0FFFFFFFul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Ky, Value);

	/* Update the Kz */
	Value = (Z & 0x0FFFFFFFul);
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Kz, Value);

	/* Set the register update bit */
	Value = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL);
	Value |= XHDCP1X_CIPHER_BITMASK_CONTROL_UPDATE;
	RegWrite(InstancePtr, XHDCP1X_CIPHER_REG_CONTROL, Value);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function reads the contents of the Mi/An register of BM0
*
* @param InstancePtr  the device to query
*
* @return
*   The contents of the register
*
* @note
*   None
*
******************************************************************************/
u64 XHdcp1x_CipherGetMi(const XHdcp1x_Cipher* InstancePtr)
{
	u64 Mi = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Update Mi */
	Mi = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Mi_H);
	Mi <<= 32;
	Mi |= RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Mi_L);

	return (Mi);
}

/*****************************************************************************/
/**
*
* This function reads the contents of the Ri register of BM0
*
* @param InstancePtr  the device to query
*
* @return
*   The contents of the register
*
* @note
*   None
*
******************************************************************************/
u16 XHdcp1x_CipherGetRi(const XHdcp1x_Cipher* InstancePtr)
{
	u16 Ri = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Determine theRi */
	Ri = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Ri);

	return (Ri);
}

/*****************************************************************************/
/**
*
* This function reads the contents of the Mo register of the device
*
* @param InstancePtr  the device to query
*
* @return
*   The contents of the Mo register
*
* @note
*   None
*
******************************************************************************/
u64 XHdcp1x_CipherGetMo(const XHdcp1x_Cipher* InstancePtr)
{
	u64 Mo = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Determine Mo */
	Mo = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Mo_H);
	Mo <<= 32;
	Mo |= RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Mo_L);

	return (Mo);
}

/*****************************************************************************/
/**
*
* This function reads the contents of the Ro register of the device
*
* @param InstancePtr  the device to query
*
* @return
*   The contents of the Ro register
*
* @note
*   None
*
******************************************************************************/
u16 XHdcp1x_CipherGetRo(const XHdcp1x_Cipher* InstancePtr)
{
	u16 Ro = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check that it is not disabled */
	if (!IsEnabled(InstancePtr)) {
		return (XST_NOT_ENABLED);
	}

	/* Determine Ro */
	Ro = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_CIPHER_Ro);

	return (Ro);
}

/*****************************************************************************/
/**
*
* This function reads the version of the hdcp cipher core
*
* @param InstancePtr  the device to query
*
* @return
*   The version of the hdcp cipher device
*
* @note
*   None
*
******************************************************************************/
u32 XHdcp1x_CipherGetVersion(const XHdcp1x_Cipher* InstancePtr)
{
	u32 Version = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Determine Version */
	Version = RegRead(InstancePtr, XHDCP1X_CIPHER_REG_VERSION);

	return (Version);
}