/*******************************************************************************
  MPLAB Harmony Application Header File

  File Name:
    adcp.h

  Summary:
    This header file provides prototypes and definitions for using the 12-bit
    ADC on PIC32MZ Embedded Connectivity devices.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "SYS_" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
//DOM-IGNORE-END

#ifndef ADCP_H
#define	ADCP_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "peripheral/adcp/plib_adcp.h"

//------------------------BEGIN USER DEFINES-----------------------
// TODO: User to configure these items according to their application
//-----------------------------------------------------------------
#define APP_NUM_ADC_SAMPLES 1	// The number of ADC data samples to capture
#define APP_FCY 80000000		// System frequency in Hz, which will be used to derive the ADC TAD clock
				//  User must configure PLL and oscillator accordingly. APP_FCY =
#define APP_PBCLK3 8000000	// APP_PBCLK3 frequency in Hz, which will drive Timer3 for triggering
#define APP_TAD 8000000		// Desired TAD clock Frequency (Use only interger value)
#define APP_TAD_Sample_Time 3	// For shared S/H_5 chan, how many TAD clks to sample (SAMC = 10)
#define APP_NUM_ANX_PINS    8	// How many shared channels will be used
#define APP_NUM_ANX_TOTAL_PINS    11	// How many shared channels will be used

#define ADC_MODE	0	// 0=ADC Single ended, 2=ADC Differential mode
#define ADC_SIGN	0	// 0=Unsigned, 1=Signed

#define ADC_MODE_SINGLE		// ADC_MODE_SINGLE or ADC_MODE_DIFFERENTIAL mode
#define ADC_SIGN_UNSIGNED	// SIGNED or UNSIGNED mode

//-------- ADC Ext Ref voltages and Anx inputs to sample -----------
#define EXT_VREF_PLUS   3.3	// External VREF+ pin voltage level
#define EXT_VREF_MINUS  0.0 	// External VREF- pin voltage level

#define PINS_TO_SAMPLE  {12,19,30,6,10,7,31,4} // Which ANx inputs will be sampled during the run
#define PINS_TO_SAMPLE2  {12,19,30,6,10,7,31,4,24,8,9} // Which ANx inputs will be sampled during the run

//------------------------END USER DEFINES-----------------------

//----------------- USER DEFINES ERROR CHECKING ------------------
#if (APP_FCY > 200000000) || (APP_PBCLK3 > (APP_FCY/2))
 #error "Invalid APP_FCY or APP_PBCLK3"
#endif

#if (APP_TAD > 16000000) || (APP_TAD < 1000000) || (APP_TAD_Sample_Time < 3)
 #error "Invalid APP_TAD or APP_TAD_Sample_Time"
#endif

#if defined(ADC_SIGN_UNSIGNED)
  typedef unsigned short ADC_DATA_TYPE;
#elif defined(ADC_SIGN_SIGNED)
  typedef short ADC_DATA_TYPE;
#else
  #error "Select ADC_SIGN_SIGNED or ADC_SIGN_UNSIGNED"
#endif

//------------------- BEGIN PROGRAM DEFINES----------------------------
// User should not edit the defines in this section
//-----------------------------------------------------------------
#if defined(ADC_SIGN_SIGNED)
  #define TRN_MODE	ADCP_SH_MODE_DIFFERENTIAL_TWOS_COMP
  #if defined(ADC_MODE_SINGLE)
    #define RUN_MODE	ADCP_SH_MODE_SINGLE_ENDED_TWOS_COMP
  #else
    #define RUN_MODE	ADCP_SH_MODE_DIFFERENTIAL_TWOS_COMP
  #endif
#else
  #define TRN_MODE	ADCP_SH_MODE_DIFFERENTIAL_UNIPOLAR
  #if defined(ADC_MODE_SINGLE)
    #define RUN_MODE	ADCP_SH_MODE_SINGLE_ENDED_UNIPOLAR
  #else
    #define RUN_MODE	ADCP_SH_MODE_DIFFERENTIAL_UNIPOLAR
  #endif
#endif

#if defined(ADC_SIGN_UNSIGNED)
#if defined(ADC_MODE_SINGLE)
  #define IDEAL_IVREF (((1.2 * 16384) / EXT_VREF_PLUS) + 0.5)
#else
  #define IDEAL_IVREF ((1.2-EXT_VREF_MINUS) * 32768 / (EXT_VREF_PLUS - EXT_VREF_MINUS) + 0.5)
#endif
#else
#if defined(ADC_MODE_SINGLE)
  #define IDEAL_IVREF (-8192 + ((1.2 * 16384) / EXT_VREF_PLUS) + 0.5)
#else
  #define IDEAL_IVREF (-8192 + (1.2-EXT_VREF_MINUS) * 32768 / (EXT_VREF_PLUS - EXT_VREF_MINUS) + 0.5)
#endif
#endif

typedef unsigned char ANx_Array[APP_NUM_ANX_PINS];

  extern ANx_Array ANx_Pins;

typedef unsigned char ANx_Array2[APP_NUM_ANX_TOTAL_PINS];

  extern ANx_Array2 ANx_Pins2;

#define ADC_MAX_UNSIGNED_14BITS (ADC_DATA_TYPE)0x3FFF
#define ADC_MAX_SIGNED_14BITS   (ADC_DATA_TYPE)0x1FFF
#define ADC_MIN_SIGNED_14BITS   (ADC_DATA_TYPE)0xE000

#ifdef	__cplusplus
}
#endif

/*******************************************************************************
  Function:
    void APP_ADC_Setup ( void )

  Summary:
    Set up the ADC for the input channels.

  Description:
    This routine sets up the ADC to collect data on the inputs the user
    defined in system_config.h. This takes advantage of the ADC already having
    been initialized by the system, and only sets up the scan channels and
    filters.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_ADC_Setup();
    </code>

  Remarks:
    This routine must be called from APP_Tasks() routine.
 */

void APP_ADC_Setup ( void );

/*******************************************************************************
  Function:
    void APP_DMA_Setup ( void )

  Summary:
    Set up the DMA for the input channels.

  Description:
    This routine sets up the DMA to collect data from the ADC Filters and put it
    in data buffers.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_DMA_Setup();
    </code>

  Remarks:
    This routine must be called from APP_Tasks() routine.
 */

void APP_DMA_Setup ( void );

/*******************************************************************************
  Function:
    ADC_DATA_TYPE ADC_Normalize_Data ( ADC_DATA_TYPE )

  Summary:
    Normalize the data collected from the ADC.

  Description:
    This routine takes the data collected from the ADC and removes the DC offset
    determined during system initialization.

  Precondition:
    None.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    correctData = APP_Normalize_Data(originalData);
    </code>

  Remarks:
    This routine must be called from APP_Tasks() routine.
 */

ADC_DATA_TYPE ADC_Normalize_Data ( ADC_DATA_TYPE );

/*******************************************************************************
  Function:
    void ADC_Scan_Set ( int anx_num )

  Summary:
    Set the scan channel for the desired pin.

  Description:
    This routine changes the ADCP scan input to the pin in the list indexed
    by the parameter.

  Precondition:
    None.

  Parameters:
    int anx_num: The index in the channel list to scan.

  Returns:
    None.

  Example:
    <code>
    ADC_Scan_Set(0);
    </code>

  Remarks:
    None.
 */

void ADC_Scan_Set(int);

#endif	/* ADCP_H */

