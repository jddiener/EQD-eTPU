/*******************************************************************************
*
*  ASH WARE INC. 2020
*  ALL RIGHTS RESERVED
*  COPYRIGHT (c)
*  
********************************************************************************
*
*  DESCRIPTION:
*   Master file for ASH WARE ETEC eTPU function selector.  This file allows
*   a function mask to be built from any combination of NXP standard
*   functions, or other open source eTPU functions, within size limitations.
*
********************************************************************************
*
*  REVISION HISTORY:
*                                                         
*  REVISION    AUTHOR       DATE         DESCRIPTION OF CHANGE                  
*  --------    -----------  ---------    ------------------------------------              
*  1.0         J. Diener    2009-Aug-26  Initial version.
*  2.0         J. Diener    2020-Aug-25  Update for new eTPU Code Builder web app
*
* $Revision: 1.5 $
*
*******************************************************************************/ 

#pragma verify_version GE,  "2.62D", "this build requires ETEC version 2.62D or newer"

#include <ETpu_Std.h>


/*************************/
/* Shared global funcs   */
/*************************/

int24 Global_Error = 0xFF;

union Link_t {
	int32 Chans;
	int8  Chan[4];
};

#define GLOBAL_ERROR_FUNC
_eTPU_fragment Global_Error_Func();

void Link4(register_p31_0 link_chans);


/*************************/
/* Include the functions */
/*************************/

/* SET 1 */

/* GPIO function */
#ifdef INCLUDE_GPIO
#include "etpuc_gpio.c"
#endif

/* PWM (Pulse Width Modulation) function */
#ifdef INCLUDE_PWM
#include "etpuc_pwm.c"
#endif

/* IC Input Capture function */
#ifdef INCLUDE_IC
#include "etpuc_ic.c"
#endif

/* OC Output Compare function */
#ifdef INCLUDE_OC
#include "etpuc_oc.c"
#endif

/* PFM (Frequency and Period Measurement) function */
#ifdef INCLUDE_FPM
#include "etpuc_fpm.c"
#endif

/* PPA (Period / Period Accumulation) function */
#ifdef INCLUDE_PPA
#include "etpuc_ppa.c"
#endif

/* UART (Universal Asynchronous Recieve / Transmitter) function */
#ifdef INCLUDE_UART
#include "etpuc_uart.c"
#endif

/* QOM (Queued Output Match) function */
#ifdef INCLUDE_QOM
#include "etpuc_qom.c"
#endif

/* SM (Stepper Motor) function */
#ifdef INCLUDE_SM
#include "etpuc_sm.c"
#endif

/* SPI (Synchronous Peripheral Interface) function */
#ifdef INCLUDE_SPI
#include "etpuc_spi.c"
#endif

/* SPWM (Synchronous Pulse Width Modulation) function */
#ifdef INCLUDE_SPWM
#include "etpuc_spwm.c"
#endif

/* POC (Protected Output Compare) function */
/*#ifdef INCLUDE_POC
  #include "etpuc_poc.c"
  #endif*/

/* SET 2 (original engine control library) */

#if defined(INCLUDE_CAM) || defined(INCLUDE_CRANK) || defined(INCLUDE_FUEL) || defined(INCLUDE_KNOCK_WINDOW) || defined(INCLUDE_SPARK) || defined(INCLUDE_TOOTHGEN) || defined(ENG_POS_SYNCH_STATUS)

int24  Eng_Pos_Sync_Status_g ; 
#endif

/* CAM function - MUST be included before crank... */
#ifdef INCLUDE_CAM
#include "etpuc_cam.c"
#endif

/* CRANK function */
#ifdef INCLUDE_CRANK
#include "etpuc_crank.c"
#endif

/* Fuel function */
#ifdef INCLUDE_FUEL
#include "etpuc_fuel.c"
#endif

/* Knock Window function */
#ifdef INCLUDE_KNOCK_WINDOW
#include "etpuc_knock_window.c"
#endif

/* Spark function */
#ifdef INCLUDE_SPARK
#include "etpuc_spark.c"
#endif

/* Tooth generation function */
#ifdef INCLUDE_TOOTHGEN
#include "etpuc_toothgen.c"
#endif

/* SET 3 */

/* PWMMDC (PWM Master for DC Motors) function */
#ifdef INCLUDE_PWMMDC
#include "etpuc_pwmmdc.c"
#endif

    /* PWMF (PWM - Full Range) function */
#ifdef INCLUDE_PWMF
#include "etpuc_pwmf.c"
#endif

    /* PWMC (PWM - Commutation Capable) function */
#ifdef INCLUDE_PWMC
#include "etpuc_pwmc.c"
#endif

/* HD (Hall Decoder) function */
#ifdef INCLUDE_HD
#include "etpuc_hd.c"
#endif

/* QD (Quadrature Decoder) function */
#ifdef INCLUDE_QD
#include "etpuc_qd.c"
#endif

/* ASDC (Analog Sensing for DC motors) function */
#ifdef INCLUDE_ASDC
#include "etpuc_asdc.c"
#endif

/* CC (Current Controller) function */
#ifdef INCLUDE_CC
#include "etpuc_cc.c"
#endif

/* SC (Speed Controller) function */
#ifdef INCLUDE_SC
#include "etpuc_sc.c"
#endif

/* BC (Break Controller) function */
#ifdef INCLUDE_BC
#include "etpuc_bc.c"
#endif

/* SET 4 */

/* PWMMAC (PWM Master for AC Motors) function */
#ifdef INCLUDE_PWMMAC
#include "etpuc_pwmmac.c"
#endif

/* ASAC (Analog Sensing for AC motors) function */
#ifdef INCLUDE_ASAC
#include "etpuc_asac.c"
#endif

/* ACIMVHZ (AC Induction Motor Volts per Hertz Control) function */
#ifdef INCLUDE_ACIMVHZ
#include "etpuc_acimvhz.c"
#endif

/* PMSMVC (Permanent Magnet Synchronous Motor Vector Control
            - Current Loop ) function  */
#ifdef INCLUDE_PMSMVC
#include "etpuc_pmsmvc.c"
#endif

/* ACIMVC (AC Induction Motor Vector Control Current Loop ) function */
#ifdef INCLUDE_ACIMVC
#include "etpuc_acimvc.c"
#endif

/* global function defintions */

_eTPU_fragment Global_Error_Func()
{
	ertb = chan;
	if (LinkServiceRequest == 1) ertb+=0x0100;
	if (MatchALatch == 1)  ertb+=0x0200;
	if (MatchBLatch == 1)  ertb+=0x0400;
	if (TransitionALatch == 1)  ertb+=0x0800;
	if (TransitionBLatch == 1)  ertb+=0x1000;
	Global_Error = ertb;
	ClearAllLatches();
}

void Link4(register_p31_0 link_chans)
{
	register_p31_24  p31_24;     // 8 bits 
	register_p23_16  p23_16;     // 8 bits 
	register_p15_8   p15_8;     // 8 bits 
	register_p7_0    p7_0;     // 8 bits 

	link = p31_24;
	link = p23_16;
	link = p15_8;
	link = p7_0;
}

#if 0 /* not needed with ETEC auto-define and auto-code generation capabilities */
/*******************************************************************************
*  Output eTPU code image and information for CPU
*******************************************************************************/

#pragma write h, (::ETPUfilename (cpu\etpu_set.h));
#pragma write h, (/***************************************************************** );
#pragma write h, (* WARNING: This file is automatically generated. DO NOT EDIT IT! );
#pragma write h, (*);
#pragma write h, (* COPYRIGHT (c) ASH WARE Inc. 2020, All Rights Reserved  );
#pragma write h, (*);
#pragma write h, (* FILE NAME: etpu_set.h  );
#pragma write h, (* COMPILER: ASHWARE v. __ETEC_VERSION__ );
#ifdef __TARGET_ETPU2__
#pragma write h, (* ARCHITECTURE: eTPU2 );
#else
#pragma write h, (* ARCHITECTURE: eTPU );
#endif
#pragma write h, (*);
#pragma write h, (* This file was generated by: __FILE__ on __DATE__, __TIME__ );
#pragma write h, (*);
#pragma write h, (*****************************************************************/);
#pragma write h, (#ifndef _ETPU_SET_H_ );
#pragma write h, (#define _ETPU_SET_H_ );
#pragma write h, ();
#pragma write h, (/***************************************************************** );
#pragma write h, (* Function Set Definitions );
#pragma write h, (*****************************************************************/);
#pragma write h, (#define FS_ETPU_ENTRY_TABLE 0x0000);
#pragma write h, (#define FS_ETPU_MISC ::ETPUmisc);
#pragma write h, (#define ETEC);
#pragma write h, (::ETPUliteral(#define FS_ETPU_CHANNEL_BASE_ADDR) ::ETPUchanframebase(2) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_STACK_SIZE) ::ETPUstacksize );
#pragma write h, (::ETPUliteral(#define FS_ETPU_A_STACK_BASE_ADDR) ::ETPUstackbase_a(2) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_A_STACK_BASE_ADDR) ::ETPUstackbase_b(2)  );
#ifdef __TARGET_ETPU2__
#pragma write h, (#define FS_ETPU_ENGINE_MEM_SIZE  ::ETPUenginememsize );
#endif
#pragma write h, ();
#pragma write h, (::ETPUliteral(#define FS_ETPU_OFFSET_GLOBAL_ERROR) ::ETPUlocation(Global_Error) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_OFFSET_ETEC_GLOBAL_ERROR) ::ETPUlocation(_Global_error_data) );
#pragma write h, ( );
#pragma write h, (/***************************************************************** );
#pragma write h, (* Initialization values of global variables );
#pragma write h, (*   On eTPU(2) initialization, this array must be copied );
#pragma write h, (*   into the eTPU DATA RAM from its start address. );
#pragma write h, (*****************************************************************/);
#pragma write h, (const uint32_t etpu_globals[] = { );
#pragma write h, (::ETPUglobalimage32 );
#pragma write h, (};);
#pragma write h, ();
#pragma write h, (/***************************************************************** );
#pragma write h, (* Image of the compiled eTPU code binary );
#pragma write h, (*   On eTPU(2) initialization, this array must be copied );
#pragma write h, (*   into the eTPU CODE RAM. );
#pragma write h, (*****************************************************************/);
#pragma write h, (const uint32_t etpu_code[] = { );
#pragma write h, (::ETPUcode32 );
#pragma write h, (};);
#pragma write h, ();
#pragma write h, (/***************************************************************** );
#pragma write h, (* List of eTPU functions included in this set );
#pragma write h, (****************************************************************** );
#pragma write h, (::ETPUnames);
#pragma write h, (*/);
#pragma write h, ();
#pragma write h, (#endif /* _ETPU_SET_H_ */ );
#pragma write h, ();
#endif


/*********************************************************************
 *
 * Copyright:
 *  ASH WARE Inc. All Rights Reserved.
 *  You are hereby granted a copyright license to use, modify, and
 *  distribute the SOFTWARE so long as this entire notice is
 *  retained without alteration in any modified and/or redistributed
 *  versions, and that such modified versions are clearly identified
 *  as such. No licenses are granted by implication, estoppel or
 *  otherwise under any patents or trademarks of ASH WARE Inc.
 *  This software is provided on an "AS IS" basis and without warranty.
 *
 *  To the maximum extent permitted by applicable law, ASH WARE Inc.
 *  DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED,
 *  INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *  PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
 *  REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF)
 *  AND ANY ACCOMPANYING WRITTEN MATERIALS.
 *
 *  To the maximum extent permitted by applicable law, IN NO EVENT
 *  SHALL ASH WARE Inc. BE LIABLE FOR ANY DAMAGES WHATSOEVER
 *  (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
 *  BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER
 *  PECUNIARY LOSS) ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.
 *
 *  ASH WARE Inc. assumes no responsibility for the maintenance and 
 *  support of this software
 *
 ********************************************************************/
