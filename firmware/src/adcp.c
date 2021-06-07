/*******************************************************************************
  System Initialization File

  File Name:
    adcp.c

  Summary:
    This file contains source code necessary to initialize the system.

  Description:
    This file contains source code necessary to initialize the system.  It
    implements the "SYS_Initialize" function, configuration bits, and allocates
    any necessary global system resources, such as the systemObjects structure
    that contains the object handles to all the MPLAB Harmony module objects in
    the system.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

#include <xc.h>
#include <sys/kmem.h>
#include "peripheral/adcp/plib_adcp.h"
#include "driver/tmr/drv_tmr.h"
#include "adcp.h"

ADCP_STATES     AdcpState   = ADCP_SW_TRN_SETUP;
int numTrainSamples = 0;
SYS_MODULE_OBJ tmr_handle;
ANx_Array ANx_Pins = PINS_TO_SAMPLE;

/******************************************************************************
  Function:
    void APP_ADC_Setup ( void )

  Remarks:
    See prototype in adcp.h.
 */

void SYS_ADC_Setup ( void )
{
  
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0, PORTS_PIN_MODE_ANALOG );

    PLIB_ADCP_Configure(ADCP_ID_1, ADCP_VREF_VREFP_VREFN, false, false, false,
            ADCP_CLK_SRC_SYSCLK, (APP_FCY / APP_TAD / 2), 32, 0, 32);

    AD1CAL1 = 0xF8894530;  // Use software calibration values into AD1CALx
    AD1CAL2 = 0x01E4AF69;
    AD1CAL3 = 0x0FBBBBB8;
    AD1CAL4 = 0x000004AC;
    AD1CAL5 = 0x02000002;

    // AD1CAL1 = 0xB3341210; //  DATASHEET
    // AD1CAL2 = 0x01FFA769;
    // AD1CAL3 = 0x0BBBBBB8;
    // AD1CAL4 = 0x000004AC;
    // AD1CAL5 = 0x02028002;

    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH0, TRN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH1, TRN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH2, TRN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH3, TRN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH4, TRN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH5, TRN_MODE);
    
    PLIB_ADCP_ChannelScanConfigure(ADCP_ID_1, 0, 0, ADCP_SCAN_TRG_SRC_TMR3_MATCH);
    //---------------------------------------------------------
    //Turn on the ADC. Wait for silicon ADC self cal to finish
    //---------------------------------------------------------
    PLIB_ADCP_Enable(ADCP_ID_1);
    while (!PLIB_ADCP_ModuleIsReady(ADCP_ID_1));
//    while (AD1CON2bits.ADCRDY == 0);    // wait for calibration to complete

    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH0, RUN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH1, RUN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH2, RUN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH3, RUN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH4, RUN_MODE);
    PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH5, RUN_MODE);
/*
    AD1IMODbits.SH0MOD = RUN_MODE;      // Set for final sign/unsigned single-ended/differential mode
    AD1IMODbits.SH1MOD = RUN_MODE;
    AD1IMODbits.SH2MOD = RUN_MODE;
    AD1IMODbits.SH3MOD = RUN_MODE;
    AD1IMODbits.SH4MOD = RUN_MODE;
    AD1IMODbits.SH5MOD = RUN_MODE;
*/
}

/******************************************************************************
  Function:
    void SYS_ADC_SW_Calibrate ( void )

  Remarks:
    See prototype in adcp.h.
 */

void SYS_ADC_SW_Calibrate ( void )
{
    SYS_STATUS tmrStatus;

    PLIB_ADCP_ChannelScanConfigure(ADCP_ID_1, 0, 1 << (43-32), ADCP_SCAN_TRG_SRC_SOFTWARE);
    PLIB_ADCP_OsampDigFilterConfig(ADCP_ID_1, ADCP_ODFLTR1, ADCP_IVREF, ADCP_ODFLTR_16X, false);
    PLIB_ADCP_OsampDigFilterEnable(ADCP_ID_1, ADCP_ODFLTR1);
/*
    AD1CSS2bits.CSS43 = 1; // Set internal IVref for scan

    AD1FLTR1bits.OVRSAM = 0b001;    // 16x oversampling
    AD1FLTR1bits.CHNLID = 43;       // IVref
    AD1FLTR1bits.AFEN = 1;          // Turn it on
*/
//    tmrStatus = DRV_TMR_Status(tmr_handle);
    if ( SYS_STATUS_READY != tmrStatus)
    {
    // Handle error
    }
    // Start the timer to get the data, and indicate we are calibrating.
//    DRV_TMR_Start(tmr_handle);
    AdcpState = ADCP_SW_TRAINING;
}

/******************************************************************************
  Function:
    void ADC_Trn_Status ( void )

  Remarks:
    See prototype in adcp.h.
 */

ADCP_STATES ADC_Trn_Status(void)
{
    short junk;

    if (AD1FLTR1bits.AFRDY)
    {
        numTrainSamples++;
        junk = AD1FLTR1bits.FLTRDATA;   // Clear the AFRDY bit by reading it.
        if (16 == numTrainSamples)      // Have we finished training?
        {
//            DRV_TMR_Stop(tmr_handle);   // Stop the timer.
            AdcpState = ADCP_SW_TRN_DONE;
        }
    }
    return AdcpState;
}

/******************************************************************************
  Function:
    void ADC_Scan_Set ( int )

  Remarks:
    See prototype in adcp.h.
 */

void ADC_Scan_Set(int anx_num)
{
    unsigned long long scanbits = 0;
    int i;

    for (i = 0; i < APP_NUM_ANX_PINS; i++)
    {
        scanbits |= (unsigned long long) 1 << ANx_Pins[i];
    }
    BiosPrintf("Scanbits: %d - %x \n",scanbits);
    AD1CSS1 = (unsigned int)scanbits;
    AD1CSS2 = (unsigned int)(scanbits >> 32);
}

/******************************************************************************
  Function:
    void ADC_Normalize_Data ( void )

  Remarks:
    See prototype in adcp.h.
 */

ADC_DATA_TYPE ADC_Normalize_Data(ADC_DATA_TYPE _in)
{
    return (ADC_DATA_TYPE)_in >> 4;
}

/******************************************************************************
  Function:
    void APP_ADC_Setup ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_ADC_Setup ( void )
{
    int i, num_pins = APP_NUM_ANX_PINS;
    __AD1FLTR1bits_t *pFLTR;

    AD1CON2bits.SAMC = APP_TAD_Sample_Time; //Configure Shared channel sample time

    //---------------------------------------------------------
    //There are only (6) oversample filters that can be assigned
    //to any (6) ANx inputs of 42 ANx. Truncate to 1st (6) ANx
    //inputs and ignore the rest.
    //---------------------------------------------------------
    if (num_pins > 6) num_pins = 6;

    AD1TRG1 = 0x03030303;
    AD1TRG2 = 0x03030303;
    AD1TRG3 = 0x03030303;

    ADC_Scan_Set(0);    // Set for the first channel

    // Set up filters
    for (i = 0, pFLTR = (__AD1FLTR1bits_t *)&AD1FLTR1; i < num_pins; i++, pFLTR++)
    {
        pFLTR->AFEN = 0;          // Reset the filter by turning it off
    pFLTR->OVRSAM = 0b001;    // 16x oversampling
    pFLTR->CHNLID = ANx_Pins[i];
    BiosPrintf("CHNLID: %X \n", ANx_Pins[i]);
    pFLTR->AFEN = 1;          // Enable the filter
    }
}

/*******************************************************************************/
/*******************************************************************************
 End of File
*/


