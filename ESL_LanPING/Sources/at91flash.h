/*! \file at91flash.h \brief Internal FLASH Write Routines for Atmel AT91. */
//*****************************************************************************
//
// File Name	: 'at91flash.h'
// Title		: Internal FLASH Write Routines for Atmel AT91
// Author		: Pascal Stang - Copyright (C) 2006
// Created		: 9/20/2006
// Revised		: 9/25/2006
// Version		: 0.1
// Target MCU	: Atmel AT91 series
// Editor Tabs	: 4
//
///	\ingroup driver_arm_at91
/// \defgroup flash_at91 Internal FLASH Write Routines for Atmel AT91 (armlib/arch/at91/at91flash.c)
/// \code #include "at91flash.h" \endcode
/// \par Overview
///		To be written...
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************
//@{

#ifndef ARMLIB_AT91FLASH_H
#define ARMLIB_AT91FLASH_H

#include "main.h"

__ramfunc void at91flashInit(void);
__ramfunc void at91flashWrite(unsigned int flashaddr, unsigned char* buffer, unsigned int len);
__ramfunc void at91flashWritePage(unsigned int  flashaddr, unsigned char* buffer, unsigned int len);
__ramfunc void at91flashErase(void);
__ramfunc int at91flashGetLock(unsigned int  flashaddr);
__ramfunc void at91flashSetLock(unsigned int  flashaddr, int lockstate);


#endif
//@}
