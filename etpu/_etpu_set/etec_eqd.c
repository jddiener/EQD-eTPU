/*********************************************************************
 * ASH WARE has modifed and enhanced the QD eTPU function; see the
 * original copyright notice at beginning and end of file.
 ********************************************************************/

/*******************************************************************************
*
*  FREESCALE SEMICONDUCTOR INC. 2004
*  ALL RIGHTS RESERVED
*  COPYRIGHT (c)
*  
********************************************************************************
*
*  DESCRIPTION:
*   Enhanced QD (Quadrature Decoder) eTPU function .
*
*******************************************************************************
*
*  REVISION HISTORY:
*                                                         
*  REVISION    AUTHOR       DATE        DESCRIPTION OF CHANGE                  
*  --------    -----------  ---------   ------------------------------------                  
*  0.01        A. Butok     10/Feb/04   Initial version of file.
*  0.2         M. Princ     02/Nov/04   Automatic switch between 3 modes:
*                                       slow-normal-fast. Commutations removed.
*                                       Generation of interrupts when
*                                       pc=pc_interrupts.
*  0.3         M. Princ     15/Dec/04   Code optimized
*              M. Brejl                 
*  0.4         M. Princ     05/Jan/05   The Second possible Position Counter
*                                       value at which the interrupt is generated
*                                       was added.
*                                       Last_edge renamed to last_leading_edge.
*                                       Last_edge parameter used for saving
*                                       the last edge time.
*  1.0         M. Princ     21/Feb/05   Formal changes for set3 release 1.0.
*  1.1         M. Brejl     08/Jan/10   Option QD_WINDOWING_DISABLED added.
*  2.0         J. Diener    08/Dec/20   Update QD function, support 32-bit period
*                                       collection, provide consistent period value,
*                                       flexible phase input channels, output 
*                                       windowing error info.
*  2.1         J. Diener    19/May/22   Fix a bug in the windowing calculation.
*              
*******************************************************************************/

#include <etpu_std.h>

/* provide hint that channel frame base addr same on all chans touched by func */
#pragma same_channel_frame_base QD

#ifndef QD_FUNCTION_NUMBER
#define QD_FUNCTION_NUMBER autocfsr
#define QDHOME_FUNCTION_NUMBER autocfsr
#define QDINDEX_FUNCTION_NUMBER autocfsr
#endif

/************************************************************************
*  Definitions. 
*************************************************************************/
/* HSR values */
#define   QD_HSR_INIT                    1
#define   QD_HOME_HSR_INIT               1
#define   QD_INDEX_HSR_INIT              1
#define   QD_LATCH_AND_CLEAR_ERRORS      7

/* Direction values */
#define   QD_DIRECTION_INCREMENT         1
#define   QD_DIRECTION_DECREMENT        -1
#define   QD_DIRECTION_INCREMENT_FAST    4
#define   QD_DIRECTION_DECREMENT_FAST   -4
#define   QD_DIRECTION_BIT7              0x80

/* QD mode_current parameter bits */
#define   QD_MODE_SLOW                   0x01
#define   QD_MODE_NORMAL                 0x02
#define   QD_MODE_FAST                   0x04
#define   QD_LEADING_EDGE_INDICATION     0x08
#define   QD_FAST_TO_NORMAL_SWITCH       0x10

/* QD options parameter bits */
#define   QD_PC_MAX_ENABLED              0x01
#define   QD_PC_INTERRUPT_ENABLED        0x02
#define   QD_WINDOWING_DISABLED          0x04

/* QD pins parameter bits */
#define   QD_PIN_A                       0x01
#define   QD_PIN_B                       0x02
#define   QD_CONFIGURATION               0x04

/* QD error buts */
#define   QD_ERROR_WINDOWING             0x01

/* QD Function Modes */
#define   QD_CHANNEL_PRIMARY            (fm0==0)
#define   QD_CHANNEL_SECONDARY          (fm0==1)
#define   QD_TIMER_TCR1                 (fm1==0)
#define   QD_TIMER_TCR2                 (fm1==1)

/* QD_HOME Function Modes */
#define   QD_HOME_DETECT_LOW_HIGH       (fm0==0)
#define   QD_HOME_DETECT_HIGH_LOW       (fm0==1)
#define   QD_HOME_DETECT_ANY            (fm1==1)

/* QD_INDEX Function Modes */
#define   QD_INDEX_PULSE_POSITIVE       (fm0==0)
#define   QD_INDEX_PULSE_NEGATIVE       (fm0==1)
#define   QD_INDEX_PC_NO_RESET          (fm1==0)
#define   QD_INDEX_PC_RESET             (fm1==1)

/************************************************************************
* NAME: QD
*
* DESCRIPTION: Quadrature Decoder  
* 
* CHANNEL PARAMETERS:
*
*  pc                    - Position Counter value.                              
*  rc                    - Revolution Counter value.                            
*  period                - Time between two leading edges.
*  pc_max                - Maximum value of Position Counter. 
*  pc_interrupt1         - The first possible Position Counter value
*                          at which the interrupt is generated.
*  pc_interrupt2         - The second possible Position Counter value
*                          at which the interrupt is generated.
*  slow_normal_threshold - Threshold for switching from slow mode to normal
*                          mode.
*  normal_slow_threshold - Threshold for switching from normal mode to slow
*                          mode.
*  normal_fast_threshold - Threshold for switching from normal mode to fast
*                          mode.
*  fast_normal_threshold - Threshold for switching from fast mode to normal
*                          mode.
*  last_leading_edge     - The last leading edge time.
*  last_edge             - The last edge time.
*  pc_sc                 - Special position counter used by Speed Controller
*  direction             - Direction. 
*                          1 or 4 for incremental, -1 or -4 for decremental.
*  last_direction        - Last direction value on first INDEX transition.
*  pins                  - Actual QD pin states and configuration
*                          - bit0=0 ? primary QD channel pin state is low
*                          - bit0=1 ? primary QD channel pin state is high
*                          - bit1=0 ? secondary QD channel pin state is low
*                          - bit1=1 ? secondary QD channel pin state is high
*                          - bit2=0 ? both QD pins are 0 when INDEX is reached
*                          - bit2=1 ? both QD pins are 1 when INDEX is reached
*  mode_current          - Current QD mode and leading edge indication.
*                          - bit0=1 ? SLOW mode
*                          - bit1=1 ? NORMAL mode
*                          - bit2=1 ? FAST mode
*                          - bit3=0 ? the last edge is not a leading edge
*                          - bit3=1 ? the last edge is a leading edge
*  options               - QD options
*                          - bit0=0 ? reset of pc when pc=pc_max disabled      
*                            bit0=1 ? reset of pc when pc=pc_max enabled
*                          - bit1=0 ? generation of interrupt when pc=pc_interrupt disabled   
*                            bit1=1 ? generation of interrupt when pc=pc_interrupt enabled
*  
*  ratio1                - This parameter applies in the window mode for setting
*                          of the window beginning.
*  ratio2                - This parameter applies in the window mode for setting
*                          of the window end.
*
*  phase_A_chan          - The phase A input channel number.
*  phase_B_chan          - The phase B input channel number.
*  error_flags(_latched) - The current state of error flags (working copy and latched).
*                          
* CHANNEL FLAG USAGE: 
*
*   Flag0 is used to select the mode.
*     0 = slow or normal mode
*     1 = fast mode
*   Flag1 is used to select the expected edge.
*     0 = the next edge must be rising edge
*     1 = the next edge must be falling edge
*
* NOTES: !!!!! Phase A and B channels must have the same base address (and home/index!). !!!!!!
*************************************************************************/

union Data_32_or_8_24
{
    uint32_t _data_32;
    struct
    {
        uint8_t   _data_8_msb;
        uint24_t  _data_24_lsb;
    } _data_8_24;
};

_eTPU_class QD
{
   /* channel frame data */
   int24_t        pc;
   int24_t        rc;
   union Data_32_or_8_24 period; 
   uint24_t       pc_max;
   int24_t        pc_interrupt1;
   int24_t        pc_interrupt2;
   uint24_t       slow_normal_threshold;
   uint24_t       normal_slow_threshold;
   uint24_t       normal_fast_threshold;
   uint24_t       fast_normal_threshold;
   int24_t        last_leading_edge;
   int24_t        last_edge;
   int24_t        pc_sc;
   int8_t         direction;     
   int8_t         last_direction;
   int8_t         pins;
   int8_t         mode_current;
   int8_t         options;
   fract24_t      ratio1;
   fract24_t      ratio2;
   uint8_t        phase_A_chan;
   uint8_t        phase_B_chan;
   uint8_t        error_flags;
   uint8_t        error_flags_latched;
   union Data_32_or_8_24 period_accum; 
   _Bool          found_leading_edge;

   /* main QD */
   
   /* threads */
   _eTPU_thread Init(_eTPU_matches_disabled);
   _eTPU_thread LatchAndClearErrors(_eTPU_matches_enabled);
   _eTPU_thread SlowNormalFallingEdge(_eTPU_matches_enabled);
   _eTPU_thread SlowNormalModeRisingEdge(_eTPU_matches_enabled);
   _eTPU_thread PeriodOverflow(_eTPU_matches_enabled);
   _eTPU_thread FastModeEdge(_eTPU_matches_enabled);

   /* fragments */
   _eTPU_fragment SlowNormalCommon();
   _eTPU_fragment Common();

   /* entry table */
   _eTPU_entry_table QD;
   
   
#ifdef QDHOME_FUNCTION_NUMBER
   /* QD HOME */

   /* threads */
   _eTPU_thread Home_Init(_eTPU_matches_disabled);
   _eTPU_thread Home_Transition(_eTPU_matches_enabled);

   /* entry table */
   _eTPU_entry_table QD_HOME;
#endif


#ifdef QDINDEX_FUNCTION_NUMBER
   /* QD INDEX */
   
   /* threads */
   _eTPU_thread Index_Init(_eTPU_matches_disabled);
   _eTPU_thread Index_FirstTransition(_eTPU_matches_enabled);
   _eTPU_thread Index_FirstTransitionLink(_eTPU_matches_enabled);
   _eTPU_thread Index_SecondTransition(_eTPU_matches_enabled);
   _eTPU_thread Index_SecondTransitionLink(_eTPU_matches_enabled);

   /* fragments */
   _eTPU_fragment Index_FirstTransitionCommon();
   _eTPU_fragment Index_SecondTransitionCommon();

   /* entry table */
   _eTPU_entry_table QD_INDEX;
#endif
 

};

DEFINE_ENTRY_TABLE(QD, QD, alternate, inputpin, QD_FUNCTION_NUMBER)
{
   /*           HSR    LSR M1 M2 PIN F0 F1 vector */
   ETPU_VECTOR2(2,3,   x,  x, x, 0,  0, x, _Error_handler_unexpected_thread),
   ETPU_VECTOR2(2,3,   x,  x, x, 0,  1, x, _Error_handler_unexpected_thread),
   ETPU_VECTOR2(2,3,   x,  x, x, 1,  0, x, _Error_handler_unexpected_thread),
   ETPU_VECTOR2(2,3,   x,  x, x, 1,  1, x, _Error_handler_unexpected_thread),
   ETPU_VECTOR3(1,4,5, x,  x, x, x,  x, x, Init),
   ETPU_VECTOR2(6,7,   x,  x, x, x,  x, x, LatchAndClearErrors),
   ETPU_VECTOR1(0,     1,  0, 0, 0,  x, x, _Error_handler_unexpected_thread),
   ETPU_VECTOR1(0,     1,  0, 0, 1,  x, x, _Error_handler_unexpected_thread),

   ETPU_VECTOR1(0,     x,  1, 0, 0,  1, 0, _Error_handler_unexpected_thread),
   ETPU_VECTOR1(0,     x,  1, 0, 0,  1, 1, _Error_handler_unexpected_thread),
   ETPU_VECTOR1(0,     x,  1, 0, 1,  1, 0, _Error_handler_unexpected_thread),
   ETPU_VECTOR1(0,     x,  1, 0, 1,  1, 1, _Error_handler_unexpected_thread),

   /* Slow or normal mode, rising edge */
   ETPU_VECTOR1(0,     x,  0, 1, 0,  0, 0, SlowNormalModeRisingEdge),
   ETPU_VECTOR1(0,     x,  0, 1, 1,  0, 0, SlowNormalModeRisingEdge),
   ETPU_VECTOR1(0,     x,  1, 1, 0,  0, 0, SlowNormalModeRisingEdge),
   ETPU_VECTOR1(0,     x,  1, 1, 1,  0, 0, SlowNormalModeRisingEdge),

   /* Slow or normal mode, falling edge */
   ETPU_VECTOR1(0,     x,  0, 1, 0,  0, 1, SlowNormalFallingEdge),
   ETPU_VECTOR1(0,     x,  0, 1, 1,  0, 1, SlowNormalFallingEdge),
   ETPU_VECTOR1(0,     x,  1, 1, 0,  0, 1, SlowNormalFallingEdge),
   ETPU_VECTOR1(0,     x,  1, 1, 1,  0, 1, SlowNormalFallingEdge),

    /* */
   ETPU_VECTOR1(0,     x,  1, 0, 0,  0, 0, PeriodOverflow),
   ETPU_VECTOR1(0,     x,  1, 0, 0,  0, 1, PeriodOverflow),
   ETPU_VECTOR1(0,     x,  1, 0, 1,  0, 0, PeriodOverflow),
   ETPU_VECTOR1(0,     x,  1, 0, 1,  0, 1, PeriodOverflow),

   /* Fast mode activates on edge and flag0 == 1 */
   ETPU_VECTOR1(0,     x,  0, 1, 0,  1, 0, FastModeEdge),
   ETPU_VECTOR1(0,     x,  0, 1, 0,  1, 1, FastModeEdge),
   ETPU_VECTOR1(0,     x,  0, 1, 1,  1, 0, FastModeEdge),
   ETPU_VECTOR1(0,     x,  0, 1, 1,  1, 1, FastModeEdge),
   ETPU_VECTOR1(0,     x,  1, 1, 0,  1, 0, FastModeEdge),
   ETPU_VECTOR1(0,     x,  1, 1, 0,  1, 1, FastModeEdge),
   ETPU_VECTOR1(0,     x,  1, 1, 1,  1, 0, FastModeEdge),
   ETPU_VECTOR1(0,     x,  1, 1, 1,  1, 1, FastModeEdge),
};

#ifdef QDHOME_FUNCTION_NUMBER
DEFINE_ENTRY_TABLE(QD, QD_HOME, standard, inputpin, QDHOME_FUNCTION_NUMBER)
{
	//           HSR LSR M1 M2 PIN F0 F1 vector
	ETPU_VECTOR1(1,  x,  x, x, 0,  0, x, Home_Init),
	ETPU_VECTOR1(1,  x,  x, x, 0,  1, x, Home_Init),
	ETPU_VECTOR1(1,  x,  x, x, 1,  0, x, Home_Init),
	ETPU_VECTOR1(1,  x,  x, x, 1,  1, x, Home_Init),
	ETPU_VECTOR1(2,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(3,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(4,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(5,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(6,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(7,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  1, 1, x,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  1, 1, x,  1, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  0, 1, 0,  0, x, Home_Transition),
	ETPU_VECTOR1(0,  0,  0, 1, 0,  1, x, Home_Transition),
	ETPU_VECTOR1(0,  0,  0, 1, 1,  0, x, Home_Transition),
	ETPU_VECTOR1(0,  0,  0, 1, 1,  1, x, Home_Transition),
	ETPU_VECTOR1(0,  0,  1, 0, 0,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  1, 0, 0,  1, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  1, 0, 1,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  1, 0, 1,  1, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  1, 1, 0,  0, x, Home_Transition),
	ETPU_VECTOR1(0,  0,  1, 1, 0,  1, x, Home_Transition),
	ETPU_VECTOR1(0,  0,  1, 1, 1,  0, x, Home_Transition),
	ETPU_VECTOR1(0,  0,  1, 1, 1,  1, x, Home_Transition),
	ETPU_VECTOR1(0,  1,  0, 0, 0,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  0, 0, 0,  1, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  0, 0, 1,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  0, 0, 1,  1, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  0, 1, x,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  0, 1, x,  1, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  1, 0, x,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  1, 0, x,  1, x, _Error_handler_unexpected_thread),
};
#endif

#ifdef QDINDEX_FUNCTION_NUMBER
DEFINE_ENTRY_TABLE(QD, QD_INDEX, standard, inputpin, QDINDEX_FUNCTION_NUMBER)
{
	//           HSR LSR M1 M2 PIN F0 F1 vector
	ETPU_VECTOR1(1,  x,  x, x, 0,  0, x, Index_Init),
	ETPU_VECTOR1(1,  x,  x, x, 0,  1, x, Index_Init),
	ETPU_VECTOR1(1,  x,  x, x, 1,  0, x, Index_Init),
	ETPU_VECTOR1(1,  x,  x, x, 1,  1, x, Index_Init),
	ETPU_VECTOR1(2,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(3,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(4,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(5,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(6,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(7,  x,  x, x, x,  x, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  1, 1, x,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  1, 1, x,  1, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  0, 1, 0,  0, x, Index_SecondTransition),
	ETPU_VECTOR1(0,  0,  0, 1, 0,  1, x, Index_FirstTransition),
	ETPU_VECTOR1(0,  0,  0, 1, 1,  0, x, Index_FirstTransition),
	ETPU_VECTOR1(0,  0,  0, 1, 1,  1, x, Index_SecondTransition),
	ETPU_VECTOR1(0,  0,  1, 0, 0,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  1, 0, 0,  1, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  1, 0, 1,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  1, 0, 1,  1, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  0,  1, 1, 0,  0, x, Index_SecondTransition),
	ETPU_VECTOR1(0,  0,  1, 1, 0,  1, x, Index_FirstTransition),
	ETPU_VECTOR1(0,  0,  1, 1, 1,  0, x, Index_FirstTransition),
	ETPU_VECTOR1(0,  0,  1, 1, 1,  1, x, Index_SecondTransition),
	ETPU_VECTOR1(0,  1,  0, 0, 0,  0, x, Index_SecondTransitionLink),
	ETPU_VECTOR1(0,  1,  0, 0, 0,  1, x, Index_FirstTransitionLink),
	ETPU_VECTOR1(0,  1,  0, 0, 1,  0, x, Index_FirstTransitionLink),
	ETPU_VECTOR1(0,  1,  0, 0, 1,  1, x, Index_SecondTransitionLink),
	ETPU_VECTOR1(0,  1,  0, 1, x,  0, x, Index_FirstTransition),
	ETPU_VECTOR1(0,  1,  0, 1, x,  1, x, Index_FirstTransition),
	ETPU_VECTOR1(0,  1,  1, 0, x,  0, x, _Error_handler_unexpected_thread),
	ETPU_VECTOR1(0,  1,  1, 0, x,  1, x, _Error_handler_unexpected_thread),
};
#endif


/**********************************************
* Phase A/B Channel Initialization.
**********************************************/
_eTPU_thread QD::Init(_eTPU_matches_disabled)
{   
   if(QD_TIMER_TCR1)                                       // With FM1 select TCR1 or TCR2.
   {  
      ActionUnitA( MatchTCR1, CaptureTCR1, GreaterEqual);  // TCR1 clock selected.
      ActionUnitB( MatchTCR1, CaptureTCR1, GreaterEqual);  // TCR1 clock selected.
      SingleMatchSingleTransition();                         // Channel mode: Single Match Single Transition when not windowing.
      ClearAllLatches();                                   // Negate all pending events.
      last_leading_edge=tcr1;                              // Store current TCR1 value. 
   }
   else
   {  
      ActionUnitA( MatchTCR2, CaptureTCR2, GreaterEqual);  // TCR2 clock selected.
      ActionUnitB( MatchTCR2, CaptureTCR2, GreaterEqual);  // TCR2 clock selected.
      SingleMatchSingleTransition();                         // Channel mode: Single Match Single Transition when not windowing.
      ClearAllLatches();                                   // Negate all pending events.
      last_leading_edge=tcr2;                              // Store current TCR2 value.
   }
   if(CurrentInputPin==1)                                  // Get latest pin state.
   {
      OnTransA(HighLow);                                    // Pin is configured to detect high low transitions.
      if(QD_CHANNEL_PRIMARY)                                // for primary channel
      {   
         Set(flag1);                                         // The next edge must be falling edge
         pins |= QD_PIN_A;                                   // Set primary channel pin_A bit of pins parameter
      }
      else                                                  // for secondary channel
      {
         Set(flag1);                                         // The next edge must be falling edge
         pins |= QD_PIN_B;                                   // Set secondary channel pin_B bit of pins parameter
      }
   }
   else
   {
      OnTransA(LowHigh);                                    // Pin is configured to detect low high transitions.
      if(QD_CHANNEL_PRIMARY)                                // for primary channel
      {   
         Clear(flag1);                                       // The next edge must be rising edge
         pins &= ~QD_PIN_A;                                  // Clear primary channel pin_A bit of pins parameter
      }
      else                                                  // for secondary channel
      {
         Clear(flag1);                                       // The next edge must be rising edge
         pins &= ~QD_PIN_B;                                  // Clear secondary channel pin_B bit of pins parameter
      }
   }
   pc=0;                                                   // Set Position Counter to 0.
   mode_current=QD_MODE_SLOW;                              // Set mode_current to slow
   period_accum._data_32 = 0;
   flag0=0;                                                // Slow/Normal mode.
   
   found_leading_edge = FALSE;
   erta = last_leading_edge + 0x800000;
   WriteErtAToMatchAAndEnable();
   
   EnableEventHandling();                                  // Enable channel.
}

/**********************************************
* Latch the error flags and clear the working copy
**********************************************/
_eTPU_thread QD::LatchAndClearErrors(_eTPU_matches_enabled)
{
   error_flags_latched = error_flags;
   error_flags = 0;
}
   
/************************************************************
* Falling Edge in Slow/Normal Mode
************************************************************/
_eTPU_thread QD::SlowNormalFallingEdge(_eTPU_matches_enabled)
{
   OnTransA(LowHigh);                                       // Pin is configured to detect low high transitions.
   if(QD_CHANNEL_PRIMARY)                                   // for primary channel
   {   
      Clear(flag1);                                          // The next edge must be rising edge
      pins &= ~QD_PIN_A;                                     // Clear primary channel pin_A bit of pins parameter
   }
   else                                                     // for secondary channel
   {
      Clear(flag1);                                          // The next edge must be rising edge
      pins &= ~QD_PIN_B;                                     // Clear secondary channel pin_B bit of pins parameter
   }
   SlowNormalCommon();
}

/************************************************************
* Rising Edge in Slow/Normal Mode
************************************************************/
_eTPU_thread QD::SlowNormalModeRisingEdge(_eTPU_matches_enabled)
{
     OnTransA(HighLow);                                       // Pin is configured to detect high low transitions.   
     if(QD_CHANNEL_PRIMARY)                                   // for primary channel                                 
     {                                                                                                               
       Set(flag1);                                            // The next edge must be falling edge
       pins |= QD_PIN_A;                                      // Set primary channel pin_A bit of pins parameter
     }                                                                                                               
     else                                                     // for secondary channel                               
     {                                                                                                               
       Set(flag1);                                            // The next edge must be falling edge
       pins |= QD_PIN_B;                                      // Set secondary channel pin_B bit of pins parameter
     }
     SlowNormalCommon();
}

/************************************************************
* Common Slow/Normal Mode Processing
************************************************************/
_eTPU_fragment QD::SlowNormalCommon()
{
   if (mode_current & QD_MODE_SLOW)                         // For slow mode
   {
      if((QD_CHANNEL_SECONDARY)^(pins & QD_PIN_A)^((pins & QD_PIN_B)>>1)) // Perform lead/lag test.
         direction=QD_DIRECTION_INCREMENT;                   // Set direction.
      else
         direction=QD_DIRECTION_DECREMENT;                   // Set direction.
   }
   if (!IsTransALatched())                                  // Transition detected or detection window end?
   {
      erta = last_edge + (period._data_8_24._data_24_lsb >> 2);                        // Estimate edge time from previous egde and period
      error_flags |= QD_ERROR_WINDOWING;
   }
   Common();
}

/************************************************************
* Period Overflow detected
************************************************************/
_eTPU_thread QD::PeriodOverflow(_eTPU_matches_enabled)
{
   ClearMatchALatch();
   period_accum._data_8_24._data_24_lsb += (erta - last_leading_edge);
   if (CC.C)
   {
      period_accum._data_8_24._data_8_msb += 1;
   }
   last_leading_edge = erta;
   erta += 0x800000;
   WriteErtAToMatchAAndEnable();
}

/************************************************************
* FAST Mode, Edge Detected
************************************************************/
_eTPU_thread QD::FastModeEdge(_eTPU_matches_enabled)
{
   if (!IsTransALatched())                                 // Transition detected or detection window end?
   {
      erta = last_edge + period._data_8_24._data_24_lsb;                            // Estimate edge time from previous egde and period
      error_flags |= QD_ERROR_WINDOWING;
   }
   Common();
}

/************************************************************
* Common Processing and any edge, all modes
************************************************************/
_eTPU_fragment QD::Common()
{
   uint8_t tmp_chan;
   uint24_t tmp_period;

   DisableMatchDetection();                                         // end any matches in progress

   last_edge = erta;
   pc+=direction;                                          // Decrement or Increment the PC.
   pc_sc+=direction;                                       // Decrement or Increment the PC_SC.

   if((options & QD_PC_INTERRUPT_ENABLED) &&
      ((pc==pc_interrupt1)||(pc==pc_interrupt2)))
      SetChannelInterrupt();                                // Generate interrupt each time when pc=pc_interrupt1 or pc=pc_interrupt2

   if((pins==(QD_PIN_A+QD_PIN_B+QD_CONFIGURATION))||
      (pins==0x0))                                         // If the current edge is the leading edge
   {
      if(options & QD_PC_MAX_ENABLED)                       // If pc_max is enabled
      {
         if(__abs(pc)>=pc_max)
         {
            pc=0;                                             // Reset PC
         }
      }
      tmp_period = period_accum._data_8_24._data_24_lsb += (erta - last_leading_edge);               // Period between two leading edges
      if (CC.C)  // set if 24 bits of period overflows
      {
          period_accum._data_8_24._data_8_msb += 1;
      }
      period._data_32 = period_accum._data_32;
      period_accum._data_32 = 0;
      last_leading_edge = erta; 
      if (!found_leading_edge)
      {
         found_leading_edge = TRUE;
         mode_current |= QD_LEADING_EDGE_INDICATION;          // Set leading edge indication bit
         goto QD_FINAL;
      }
      
      if (mode_current & QD_MODE_SLOW)                      // For slow mode
      {
         if(tmp_period < slow_normal_threshold && period._data_8_24._data_8_msb == 0)                  // Exit Slow mode and enter Normal mode.
         {
            mode_current=QD_MODE_NORMAL;                       // Set mode_current to normal.
            tmp_period >>= 1; // windowing based upon half period
         }
      }
      else if (mode_current & QD_MODE_NORMAL)               // For normal mode
      {
         if(tmp_period < normal_fast_threshold)                  // Exit Normal mode and enter Fast mode
         {
            mode_current=QD_MODE_FAST;                        // Set mode_current to fast.
            if (direction & QD_DIRECTION_BIT7)                // If direction is negative
            {
               direction=QD_DIRECTION_DECREMENT_FAST;          // Direction = -4
            }
            else
            {
               direction=QD_DIRECTION_INCREMENT_FAST;          // Direction = 4
            }
            Set(flag0);                                       // Set Fast mode
            /* According to the configuration detect high_low/low_high edges */
            if(pins & QD_CONFIGURATION)
            {
               OnTransA(LowHigh);                              // Pin is configured to detect low high transitions.
               Clear(flag1);                                   // The next edge must be rising edge
            }
            else
            {
               OnTransA(HighLow);                              // Pin is configured to detect high low transitions.
               Set(flag1);                                     // The next edge must be falling edge
            }
            tmp_chan = chan;
            if(QD_CHANNEL_PRIMARY)                            // for primary channel
            {   
               chan = phase_B_chan;
            }
            else                                              // for secondary channel
            {
               chan = phase_A_chan; 
            }
            OnTransA(NoDetect);                               // Pin is configured to detect no transitions.
            DisableMatchDetection();                          // Prevent from end-window match
            ClearAllLatches();
            chan = tmp_chan;
         }
         else if(tmp_period > normal_slow_threshold)             // Exit Normal mode and enter Slow mode
         {
            mode_current = (QD_LEADING_EDGE_INDICATION + QD_MODE_SLOW);   // Set mode_current to slow.
            goto QD_FINAL;
         }
         else
         {
            tmp_period >>= 1; // windowing based upon half period when in NORMAL mode
         }
      }
      else if (mode_current & QD_MODE_FAST)                 // For Fast mode
      {
         if(tmp_period > fast_normal_threshold)                  // Exit Fast mode and enter Normal mode
         {
            /* Set mode_current to normal and 
               set the QD_FAST_TO_NORMAL_SWITCH bit to indicate 
               the edge in which the mode swithes from Fast to Normal */
            mode_current= (QD_FAST_TO_NORMAL_SWITCH + QD_MODE_NORMAL);

            if (direction & QD_DIRECTION_BIT7)                // If direction is negative
            {
               direction=QD_DIRECTION_DECREMENT;               // Direction = -1
            }
            else
            {
               direction=QD_DIRECTION_INCREMENT;               // Direction = 1
            }
            Clear(flag0);                                          // Set SLOW/NORMAL mode
            /* According to the configuration detect high_low/low_high edges */
            if(pins & QD_CONFIGURATION)
            {
               OnTransA(HighLow);                              // Pin is configured to detect high low transitions.
               Set(flag1);                                     // The next edge must be falling edge
            }
            else
            {
               OnTransA(LowHigh);                              // Pin is configured to detect low high transitions.
               Clear(flag1);                                   // The next edge must be rising edge
            }
            pc += direction;                                  // Decrement or Increment the PC.    
            pc_sc += direction;                               // Decrement or Increment the PC_SC. 
            tmp_chan = chan;
            if(QD_CHANNEL_PRIMARY)                            // for primary channel
            {   
               chan = phase_B_chan;                                         // Switch to secondary channel
            }
            else                                              // for secondary channel
            {
               chan = phase_A_chan;                                         // Switch to primary channel
            }
            /* According to the configuration detect high_low/low_high edges */
            if(pins & QD_CONFIGURATION)
            {
               OnTransA(LowHigh);                              // Pin is configured to detect low high transitions.
               Clear(flag1);                                   // The next edge must be rising edge
            }
            else
            {
               OnTransA(HighLow);                              // Pin is configured to detect high low transitions.
               Set(flag1);                                     // The next edge must be falling edge
            }
            SingleMatchSingleTransition();                      // Channel mode: Both Match Single Transition.
            chan = tmp_chan;
            tmp_period >>= 1; // windowing based upon half period when in NORMAL mode
         }
      }
      mode_current |= QD_LEADING_EDGE_INDICATION;          // Set leading edge indication bit
   }
   else
   {
      /* Clear leading edge indication bit and Fast to Normal Switch indication bit */
      mode_current &= (~(QD_LEADING_EDGE_INDICATION | QD_FAST_TO_NORMAL_SWITCH));
      tmp_period = period._data_8_24._data_24_lsb;
      if (mode_current & QD_MODE_NORMAL)
          tmp_period >>= 1; // windowing based upon half period when in NORMAL mode
   }

   erta = last_edge;  // just in case erta is incorrect after channel changes etc.
   
   if(((options & QD_WINDOWING_DISABLED)==0) && ((mode_current & QD_MODE_SLOW)==0))  // For Fast or Normal mode
   {
      Match2SingleTransition();                           // Channel mode: Match B Single Transition
      ertb = erta + tmp_period;                               // Setup next window end
      ertb += mulir(tmp_period, ratio2);
      /* When the transition did not come (Match B is latched) 
         let the window opened, else setup next window beginning */
      if(!(IsLatchedMatchB()))
      {
         erta += mulir(tmp_period, ratio1);                    // Setup next window beginning
      }
      ClearAllLatches();                                    // Negate all pending events.
      WriteErtAToMatchAAndEnable();
      WriteErtBToMatchBAndEnable();
   }
   else
   {
QD_FINAL:
      SingleMatchSingleTransition();                        // Channel mode: Single Match Single Transition when not windowing.
      ClearAllLatches();                                    // Negate all pending events.
   }

   if (mode_current & QD_MODE_SLOW)
   {
      erta += 0x800000;                               // start up period overflow match
      WriteErtAToMatchAAndEnable();
   }
}


#ifdef QDHOME_FUNCTION_NUMBER

/************************************************************************
* NAME: QDHOME
*
* DESCRIPTION: Monitors a "HOME position" of the motion system. 
*              Resets PC and RC. 
* FUNCTION PARAMETERS: same as for all QD functions
*
*************************************************************************/

/**********************************************
* HOME Channel Initialization.
**********************************************/
_eTPU_thread QD::Home_Init(_eTPU_matches_disabled)
{
   if(QD_HOME_DETECT_ANY)                                 // FM==1x
   {
      OnTransA(AnyTrans);                                 // Pin is configured to detect any transition.
      EitherMatchNonBlockingSingleTransition();           // Channel mode: Non Blocking Single Transition.
      ActionUnitA( MatchTCR1, CaptureTCR1, GreaterEqual); // TCR1 clock selected.
      ClearAllLatches();                                  // Negate all pending events.
   }
   else
   {
                                                          // FM==01
      OnTransA(HighLow);                                  // Pin is configured to detect high-low transition.
      EitherMatchNonBlockingSingleTransition();           // Channel mode: Non Blocking Single Transition.
      ActionUnitA( MatchTCR1, CaptureTCR1, GreaterEqual); // TCR1 clock selected.
      ClearAllLatches();                                  // Negate all pending events.
      
      if(QD_HOME_DETECT_LOW_HIGH)                         // FM==00
      {
         OnTransA(LowHigh);                               // Pin is configured to detect low-high transition.
      }
   }
   EnableEventHandling();                                 // Enable channel.
}

/**********************************************
* New transition detection. 
**********************************************/
_eTPU_thread QD::Home_Transition(_eTPU_matches_enabled)
{
   ClearTransLatch();                                     // Clear the transition latch to enable further transition detection.
   rc=0;                                                  // Reset Revolution Counter to 0.
   pc=0;                                                  // Reset Position Counter to 0.
}

#endif


#ifdef QDINDEX_FUNCTION_NUMBER

/************************************************************************
* NAME: QDINDEX
*
* DESCRIPTION: Monitors an input signal that indicates each revolution 
*              of the motion system. 
* FUNCTION PARAMETERS: same as for all QD functions
*
* CHANNEL FLAG USAGE: 
*   Flag0 is used to select the expected edge.
*     0 = the next edge must be rising edge
*     1 = the next edge must be falling edge
*************************************************************************/
   
/**********************************************
* INDEX Channel Initialization.
**********************************************/
_eTPU_thread QD::Index_Init(_eTPU_matches_disabled)
{
   OnTransA(AnyTrans);                                    // Pin is configured to detect any transition.
   EitherMatchNonBlockingSingleTransition();              // Channel mode: Non Blocking Single Transition.
   ActionUnitA( MatchTCR1, CaptureTCR1, GreaterEqual);    // TCR1 clock selected.
   ClearAllLatches();                                     // Negate all pending events.
		
   if(QD_INDEX_PULSE_POSITIVE)                            // Detection of low-high transition.
   {
      Clear(flag0);                                       // Detect Low High transition as 1st transition.
      if(CurrentInputPin==1) 
         last_direction=0;                                // To indicate the position inside the pulse.
   }
   else                                                   // Detection of high-low transition.
   {
      Set(flag0);                                         // Detect High Low transition as 1st transition.
      if(CurrentInputPin==0) 
         last_direction=0;                                // To indicate the position inside the pulse.
   }

   rc=0;                                                  // Reset Revolution Counter to 0.

   EnableEventHandling();                                 // Enable channel.
}
   
/**********************************************
///////////// First Transition ////////////////
***********************************************
* Low High transition and flag0=0.
* High Low transition and flag0=1.
**********************************************/
_eTPU_thread QD::Index_FirstTransition(_eTPU_matches_enabled)
{
   ClearTransLatch();                                     // Clear the transition latch to enable further transition detection.

   Index_FirstTransitionCommon();
}

/**********************************************
* Common First Transition Processing
**********************************************/
_eTPU_fragment QD::Index_FirstTransitionCommon()
{
   /* The first transition could not come before leading edge, 
      wait till the leading edge is reached */
   if (!(mode_current & QD_LEADING_EDGE_INDICATION))
   {
      link=chan;                                          // Send link to self - wait for state after leading edge
   }
   else
   {
      if(QD_INDEX_PC_RESET)                               // FM==1x
      {
         if(mode_current & QD_FAST_TO_NORMAL_SWITCH)
         {
            pc=direction;                    // In case the first index transition come at the same time as the leading edge in which the mode swithes from Fast to Normal
                                             // (the next QD edge after this is not scheduled so the pc could not be reset to 0)
         }
         else
         {
            pc=0;                            // Reset Position Counter to 0. 
         }                
      }
         
      /* Decrement revolution counter when direction < 0, 
         increment revolution counter when direction > 0 */
      if(direction & QD_DIRECTION_BIT7)
      {
         rc--;
      }
      else
      {
         rc++;
      }
         
      last_direction=direction;                           // Save direction.

      /* Clear leading edge indication bit in fast mode */
      if (mode_current & QD_MODE_FAST)
      {
         mode_current &= ~QD_LEADING_EDGE_INDICATION;
      }

      /* Do not detect both edges but only the first one in Normal and Fast mode */
      if (mode_current & QD_MODE_SLOW)
      {
         OnTransA(AnyTrans);
      }
      else
      {
         if(QD_INDEX_PULSE_POSITIVE)
         {
            OnTransA(LowHigh);
         }
         else
         {
            OnTransA(HighLow);
         }
         ClearAllLatches();
      }
   }
}
   
/**********************************************
* Link detection, Pin is High and flag0=0.
* Link detection, Pin is Low and flag0=1.
**********************************************/
_eTPU_thread QD::Index_FirstTransitionLink(_eTPU_matches_enabled)
{
   ClearLinkServiceRequestEvent();

   Index_FirstTransitionCommon();
}
   
/**********************************************
///////////// Second Transition ///////////////
***********************************************
* High Low transition and flag0=0.
* Low High transition and flag0=1.
**********************************************/
_eTPU_thread QD::Index_SecondTransition(_eTPU_matches_enabled)
{
   ClearTransLatch();                                     // Clear the transition latch to enable further transition detection.
      
   /* If transition is first after init(last_direction==0)=> skip the event.
      Othewise => normal processing. */
   if(last_direction!=0)
   {
         Index_SecondTransitionCommon();
   }
}

/**********************************************
* Common Second Transition Processing
**********************************************/
_eTPU_fragment QD::Index_SecondTransitionCommon()
{
   /* The second INDEX transition could not come before the first 
      QD transition after leading edge, wait till the leading edge 
      indication is cleared. */
   if (!(mode_current & QD_LEADING_EDGE_INDICATION))
   {
      if(last_direction-direction!=0) // In case when the direction was changed during the INDEX active state.
      {
         /* Decrement revolution counter when direction < 0, 
            increment revolution counter when direction > 0 */
         if(direction & QD_DIRECTION_BIT7)
         {
            rc--;
         }
         else
         {
            rc++;
         }
      }
   }
   else
   {
      link=chan;                                       // Send link to self - wait for state when the leading edge bit is cleared
   }
}

/**********************************************
* Link detection, Pin is High and flag0=0.
* Link detection, Pin is Low and flag0=1.
**********************************************/
_eTPU_thread QD::Index_SecondTransitionLink(_eTPU_matches_enabled)
{
   ClearLinkServiceRequestEvent();

   Index_SecondTransitionCommon();
}

#endif

/************************************************************************
*  Information exported to Host CPU program.
*************************************************************************/
#pragma write h, (::ETPUfilename (cpu\etpu_eqd_auto.h));
#pragma write h, (/**************************************************************** );
#pragma write h, (* WARNING this file is automatically generated DO NOT EDIT IT!    );
#pragma write h, (*                                                                 );
#pragma write h, (* This file provides an interface between eTPU code and CPU       );
#pragma write h, (* code. All references to the QD, QDHOME and QDINDEX functions    );
#pragma write h, (* should be made with information in this file. This allows only  );
#pragma write h, (* symbolic information to be referenced which allows the eTPU     );
#pragma write h, (* code to be optimized without effecting the CPU code.            );
#pragma write h, (*****************************************************************/);
#pragma write h, (#ifndef _ETPU_EQD_AUTO_H_);
#pragma write h, (#define _ETPU_EQD_AUTO_H_);
#pragma write h, ( );
#pragma write h, (/****************************************************************);
#pragma write h, (* Function Configuration Information. );
#pragma write h, (****************************************************************/);
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_FUNCTION_NUMBER      ) ::ETPUfunctionnumber(QD::QD) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_TABLE_SELECT         ) ::ETPUentrytype(QD::QD) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_NUM_PARMS            ) ::ETPUram(QD) );
#pragma write h, ( );
#ifdef QDHOME_FUNCTION_NUMBER
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_HOME_FUNCTION_NUMBER ) ::ETPUfunctionnumber(QD::QD_HOME) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_HOME_TABLE_SELECT    ) ::ETPUentrytype(QD::QD_HOME));
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_HOME_NUM_PARMS       ) ::ETPUram(QD) );
#pragma write h, ( );
#endif
#ifdef QDINDEX_FUNCTION_NUMBER
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_INDEX_FUNCTION_NUMBER) ::ETPUfunctionnumber(QD::QD_INDEX) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_INDEX_TABLE_SELECT   ) ::ETPUentrytype(QD::QD_INDEX) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_INDEX_NUM_PARMS      ) ::ETPUram(QD) );
#pragma write h, ( );
#endif
#pragma write h, (/****************************************************************);
#pragma write h, (* Host Service Request Definitions. );
#pragma write h, (****************************************************************/);
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_INIT      ) QD_HSR_INIT );
#ifdef QDHOME_FUNCTION_NUMBER
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_HOME_INIT ) QD_HOME_HSR_INIT );
#endif
#ifdef QDINDEX_FUNCTION_NUMBER
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_INDEX_INIT) QD_INDEX_HSR_INIT );
#endif
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_LATCH_AND_CLEAR_ERRORS) QD_LATCH_AND_CLEAR_ERRORS );
#pragma write h, ( );
#pragma write h, (/****************************************************************);
#pragma write h, (* Parameter Definitions. );
#pragma write h, (****************************************************************/);
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PC_OFFSET                 ) ::ETPUlocation (QD, pc) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_RC_OFFSET                 ) ::ETPUlocation (QD, rc) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PERIOD_OFFSET             ) ::ETPUlocation (QD, period) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PCMAX_OFFSET              ) ::ETPUlocation (QD, pc_max) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PCINTERRUPT1_OFFSET       ) ::ETPUlocation (QD, pc_interrupt1) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PCINTERRUPT2_OFFSET       ) ::ETPUlocation (QD, pc_interrupt2) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_SLOW_NORMAL_THR_OFFSET    ) ::ETPUlocation (QD, slow_normal_threshold) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_NORMAL_SLOW_THR_OFFSET    ) ::ETPUlocation (QD, normal_slow_threshold) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_NORMAL_FAST_THR_OFFSET    ) ::ETPUlocation (QD, normal_fast_threshold) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_FAST_NORMAL_THR_OFFSET    ) ::ETPUlocation (QD, fast_normal_threshold) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_LAST_LEADING_EDGE_OFFSET  ) ::ETPUlocation (QD, last_leading_edge) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_LAST_EDGE_OFFSET          ) ::ETPUlocation (QD, last_edge) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PC_SC_OFFSET              ) ::ETPUlocation (QD, pc_sc) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_DIRECTION_OFFSET          ) ::ETPUlocation (QD, direction) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_LAST_DIRECTION_OFFSET     ) ::ETPUlocation (QD, last_direction) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PINS_OFFSET               ) ::ETPUlocation (QD, pins) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_MODE_CURRENT_OFFSET       ) ::ETPUlocation (QD, mode_current) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_OPTIONS_OFFSET            ) ::ETPUlocation (QD, options) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_RATIO1_OFFSET             ) ::ETPUlocation (QD, ratio1) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_RATIO2_OFFSET             ) ::ETPUlocation (QD, ratio2) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PHASE_A_CHAN_OFFSET       ) ::ETPUlocation (QD, phase_A_chan) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PHASE_B_CHAN_OFFSET       ) ::ETPUlocation (QD, phase_B_chan) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_ERROR_FLAGS_OFFSET        ) ::ETPUlocation (QD, error_flags) );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_LATCHED_ERROR_FLAGS_OFFSET) ::ETPUlocation (QD, error_flags_latched) );
#pragma write h, ( );
#pragma write h, (/****************************************************************);
#pragma write h, (* Value Definitions. );
#pragma write h, (****************************************************************/);
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_FM_CHANNEL_PRIMARY        ) 0 );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_FM_CHANNEL_SECONDARY      ) 1 );
#pragma write h, ( );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_HOME_FM_DETECT_LOW_HIGH   ) 0 );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_HOME_FM_DETECT_HIGH_LOW   ) 1 );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_HOME_FM_DETECT_ANY        ) 2 );
#pragma write h, ( );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_INDEX_FM_PULSE_POSITIVE   ) 0 );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_INDEX_FM_PULSE_NEGATIVE   ) 1 );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_INDEX_FM_PC_NO_RESET      ) 0 );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_INDEX_FM_PC_RESET         ) 2 );
#pragma write h, ( );
#pragma write h, (/* option bits */ );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PC_MAX_ENABLED            ) QD_PC_MAX_ENABLED );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PC_INTERRUPT_ENABLED      ) QD_PC_INTERRUPT_ENABLED );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_WINDOWING_DISABLED        ) QD_WINDOWING_DISABLED );
#pragma write h, ( );
#pragma write h, (/* pins bits */ );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PINS_PIN_A                ) QD_PIN_A );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PINS_PIN_B                ) QD_PIN_B );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_PINS_CONFIGURATION        ) QD_CONFIGURATION );
#pragma write h, ( );
#pragma write h, (/* error bits */ );
#pragma write h, (::ETPUliteral(#define FS_ETPU_QD_ERROR_WINDOWING           ) QD_ERROR_WINDOWING );
#pragma write h, ( );
#pragma write h, (#endif);

/*********************************************************************
 *
 * Copyright:
 *	Freescale Semiconductor, INC. All Rights Reserved.
 *  You are hereby granted a copyright license to use, modify, and
 *  distribute the SOFTWARE so long as this entire notice is
 *  retained without alteration in any modified and/or redistributed
 *  versions, and that such modified versions are clearly identified
 *  as such. No licenses are granted by implication, estoppel or
 *  otherwise under any patents or trademarks of Freescale
 *  Semiconductor, Inc. This software is provided on an "AS IS"
 *  basis and without warranty.
 *
 *  To the maximum extent permitted by applicable law, Freescale
 *  Semiconductor DISCLAIMS ALL WARRANTIES WHETHER EXPRESS OR IMPLIED,
 *  INCLUDING IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *  PARTICULAR PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
 *  REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS THEREOF)
 *  AND ANY ACCOMPANYING WRITTEN MATERIALS.
 *
 *  To the maximum extent permitted by applicable law, IN NO EVENT
 *  SHALL Freescale Semiconductor BE LIABLE FOR ANY DAMAGES WHATSOEVER
 *  (INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
 *  BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR OTHER
 *  PECUNIARY LOSS) ARISING OF THE USE OR INABILITY TO USE THE SOFTWARE.
 *
 *  Freescale Semiconductor assumes no responsibility for the
 *  maintenance and support of this software
 ********************************************************************/
