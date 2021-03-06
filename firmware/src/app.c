/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
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


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>

#include <xc.h>
#include <sys/kmem.h>
#include "app.h"
#include "system_definitions.h"
#include "peripheral/adcp/plib_adcp.h"
#include "peripheral/tmr/plib_tmr.h"
#include "system/dma/sys_dma.h"
#include "driver/tmr/drv_tmr.h"
#include "adcp_config.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA __attribute__ ((aligned(16))) __attribute__((coherent))  appData;
ANx_Array ANx_Pins = PINS_TO_SAMPLE;
APP_DATA __attribute__ ((aligned(16))) __attribute__((coherent))  appData;
ANx_Array2 ANx_Pins2 = PINS_TO_SAMPLE2;


DMA_TRIGGER_SOURCE eventSrc;
SYS_DMA_CHANNEL_OP_MODE modeEnable = (SYS_DMA_CHANNEL_OP_MODE_AUTO);
DMA_CHANNEL channel = DMA_CHANNEL_ANY;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */
static void BiosUARTInitialize(USART_MODULE_ID index, INT_SOURCE sourceRx, INT_SOURCE sourceTx, INT_VECTOR vectorRx, INT_VECTOR vectorTx, uint32_t baudRate){
    
    //PLIB_USART_BaudRateSet(index, SYS_CLK_SystemFrequencyGet(), baudRate);
    PLIB_USART_BaudRateSet(index, SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_2), baudRate);
    PLIB_USART_OperationModeSelect(index, USART_ENABLE_TX_RX_USED);
    PLIB_USART_TransmitterInterruptModeSelect(index, USART_TRANSMIT_FIFO_EMPTY);
    PLIB_USART_ReceiverInterruptModeSelect(index, USART_RECEIVE_FIFO_ONE_CHAR);
    PLIB_USART_LineControlModeSelect(index, USART_8N1);
    PLIB_USART_ReceiverEnable(index);
    PLIB_USART_TransmitterEnable(index);
    PLIB_USART_Enable(index);
    // Interrupts
    PLIB_INT_SourceEnable(INT_ID_0, sourceRx);
    PLIB_INT_SourceDisable(INT_ID_0, sourceTx);
    PLIB_INT_VectorPrioritySet(INT_ID_0, vectorRx, INT_PRIORITY_LEVEL2);
    PLIB_INT_VectorSubPrioritySet(INT_ID_0, vectorRx, INT_SUBPRIORITY_LEVEL0);
    PLIB_INT_VectorPrioritySet(INT_ID_0, vectorTx, INT_PRIORITY_LEVEL2);
    PLIB_INT_VectorSubPrioritySet(INT_ID_0, vectorTx, INT_SUBPRIORITY_LEVEL0);
}

void BiosPrintf(const char* format, ...)
{
    char myStr[500];
    unsigned int i = 0;
    va_list args;

    va_start(args, format);
    vsprintf(myStr, format, args);
    va_end(args);
    while (myStr[i] && (i<sizeof (myStr)))
    {
        while (!(!U1STAbits.UTXBF && U1STAbits.TRMT));
        U1TXREG = myStr[i++];
    }
}

void APP_Initialize ( void )
{
    // USART BAUD IS WRONG DUE TO CLOCK, IF USING 19200 MUST READ AS 78000 BAUD.
    BiosUARTInitialize(USART_ID_1, INT_SOURCE_USART_1_RECEIVE, INT_SOURCE_USART_1_TRANSMIT, INT_VECTOR_UART1_RX, INT_VECTOR_UART1_TX, 19200);
    
//Sensor Corrente P1 - AN19   
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_C, PORTS_BIT_POS_4 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_C, PORTS_BIT_POS_4 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_C, PORTS_BIT_POS_4, PORTS_PIN_MODE_ANALOG );
//Sensor Corrente P2 - AN12   
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_8 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_8 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_G, PORTS_BIT_POS_8, PORTS_PIN_MODE_ANALOG );
//Sensor Corrente P3 - AN30   
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_13 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_13 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_13, PORTS_PIN_MODE_ANALOG );
//Sensor Corrente P4 - AN6   
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_11 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_11 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_11, PORTS_PIN_MODE_ANALOG );
//Sensor Corrente P5 - AN10   
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_15 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_15 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_15, PORTS_PIN_MODE_ANALOG );
//Sensor Corrente P6 - AN7   
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_12 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_12 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_12, PORTS_PIN_MODE_ANALOG );
//Sensor Corrente P7 - AN31   
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_12 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_12 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_F, PORTS_BIT_POS_12, PORTS_PIN_MODE_ANALOG );
//Sensor Corrente P8 - AN4   
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_4 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_4 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_4, PORTS_PIN_MODE_ANALOG );
//Sensor Temperatura - AN24                   
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_A, PORTS_BIT_POS_0, PORTS_PIN_MODE_ANALOG );
//Sensor 12 Volts - AN8
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_13 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_13 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_13, PORTS_PIN_MODE_ANALOG );
//Sensor 24 Volts - AN9
    PLIB_PORTS_PinDirectionInputSet( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_14 );
    PLIB_PORTS_ChangeNoticePullUpPerPortDisable( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_14 );
    PLIB_PORTS_PinModePerPortSelect( PORTS_ID_0, PORT_CHANNEL_B, PORTS_BIT_POS_14, PORTS_PIN_MODE_ANALOG );

    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
    appData.currentChannel = 0;         // Which channel we scan first
    appData.sampleCount = 0;            // How many samples of each channel have
                                        // been collected
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */
int addata[12];
int print_c;

int switch_dma;
void APP_Tasks ( void )
{
    unsigned long long scanbits = 0;

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
            BiosPrintf(".\n");

            // PLIB_ADCP_Configure(ADCP_ID_1, ADCP_VREF_VREFP_VREFN, false, false, false,
            //         ADCP_CLK_SRC_SYSCLK, (APP_FCY / APP_TAD / 2), 32, 0, 32);
            PLIB_ADCP_Configure(ADCP_ID_1, ADCP_VREF_AVDD_AVSS, false, false, true,
                    ADCP_CLK_SRC_SYSCLK, 150, 31, 0, 0);

            AD1CAL1 = 0xB3341210;  // Use software calibration values into AD1CALx
            AD1CAL2 = 0x01FFA769;
            AD1CAL3 = 0x0BBBBBB8;
            AD1CAL4 = 0x000004AC;
            AD1CAL5 = 0x02028002;

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

            appData.state = APP_STATE_WAIT_FOR_ADC_READY;
            break;

        case APP_STATE_WAIT_FOR_ADC_READY:
            if (PLIB_ADCP_ModuleIsReady(ADCP_ID_1))
            {
                PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH0, RUN_MODE);
                PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH1, RUN_MODE);
                PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH2, RUN_MODE);
                PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH3, RUN_MODE);
                PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH4, RUN_MODE);
                PLIB_ADCP_SHModeSelect(ADCP_ID_1, ADCP_SH5, RUN_MODE);
                appData.state = APP_STATE_START_TRAINING;
            }
            break;

        case APP_STATE_START_TRAINING:
            PLIB_ADCP_ChannelScanConfigure(ADCP_ID_1, 0, 1 << (43-32), ADCP_SCAN_TRG_SRC_TMR3_MATCH);
            DRV_TMR0_Start();
            appData.state = APP_STATE_WAIT_FOR_SW_TRN;
            break;

        case APP_STATE_WAIT_FOR_SW_TRN:
            if (PLIB_INT_SourceFlagGet(INT_ID_0, INT_SOURCE_ADC_1_DATA43 ))
            {
                PLIB_INT_SourceFlagClear(INT_ID_0, INT_SOURCE_ADC_1_DATA43);
                if (++appData.sampleCount == 8)  // Throwing out the first 8 samples.
                {
                    DRV_TMR0_Stop();
                    DRV_TMR0_CounterClear();
                    appData.sampleCount = 0;
                    appData.state = APP_STATE_SETUP_FOR_DATA_COLLECTION;
                }
            }
            break;

        case APP_STATE_SETUP_FOR_DATA_COLLECTION:
            APP_DMA_Setup();
            // Set up the oversampling filters, the channel scan, and the DMA
            scanbits |= (unsigned long long) 1 << 4;
            scanbits |= (unsigned long long) 1 << 6;
            scanbits |= (unsigned long long) 1 << 7;
            scanbits |= (unsigned long long) 1 << 8;
            scanbits |= (unsigned long long) 1 << 9;
            scanbits |= (unsigned long long) 1 << 10;
            scanbits |= (unsigned long long) 1 << 12;
            scanbits |= (unsigned long long) 1 << 19;
            scanbits |= (unsigned long long) 1 << 24;
            scanbits |= (unsigned long long) 1 << 30;
            scanbits |= (unsigned long long) 1 << 31;

            PLIB_ADCP_ChannelScanConfigure(ADCP_ID_1, (unsigned int) scanbits, 
                    (unsigned int)(scanbits >> 32), ADCP_SCAN_TRG_SRC_TMR3_MATCH);

            appData.state = APP_STATE_COLLECT_DATA;
            break;

        case APP_STATE_COLLECT_DATA:
            appData.sampleCount = 0;
            DRV_TMR0_Start();
            appData.state = APP_STATE_CHECK_DATA_COLLECTION;
            break;

        case APP_STATE_CHECK_DATA_COLLECTION:
            if (APP_NUM_ADC_SAMPLES*APP_NUM_ANX_PINS <= appData.sampleCount) // We've collected all
            {
                DRV_TMR0_Stop();
                int i;
                for(i = 0; i < 8; i++)
                    SYS_DMA_ChannelRelease(appData.dma_handle[i]);
                appData.state = APP_STATE_NORMALIZE_DATA;
            }
            break;

        case APP_STATE_NORMALIZE_DATA:
            // Set a breakpoint here or after this line to see the data collected.
            APP_Normalize_Data();
            APP_DMA_Setup();
            appData.state = APP_STATE_DISPLAY_DATA;
            break;

        case APP_STATE_DISPLAY_DATA:
            // Go back to collecting data.
            appData.state = APP_STATE_COLLECT_DATA;
            print_c++;
            break;

        case APP_STATE_SPIN:
        default:
            break;
    }
    if(print_c == 12){
        print_c = 0;
        int i;
        for (i = 0; i < 11; i++)
        {
            BiosPrintf("[%d]:%d\n",i,addata[i]);
        }
    }
}

/******************************************************************************
  Function:
    void APP_DMA_Setup ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_DMA_Setup ( void )
{
    int i;
    if(switch_dma == 0){
        for (i = 0; i < 8; i++)
        {
            appData.dma_handle[i] = SYS_DMA_ChannelAllocate(channel);

            eventSrc = DMA_TRIGGER_ADC1_DATA0 + (DMA_TRIGGER_SOURCE)ANx_Pins2[i];

            SYS_DMA_ChannelSetup(appData.dma_handle[i], modeEnable, eventSrc);

            SYS_DMA_ChannelTransferEventHandlerSet(appData.dma_handle[i],APP_DMA_EventHandler, i+1);

            SYS_DMA_ChannelTransferAdd(appData.dma_handle[i], ((const unsigned int *)&AD1DATA0) + ANx_Pins2[i],
                    sizeof(ADC_DATA_TYPE), &appData.ADC_Data[i],
                    APP_NUM_ADC_SAMPLES * sizeof(ADC_DATA_TYPE), sizeof(ADC_DATA_TYPE));

            SYS_DMA_ChannelEnable(appData.dma_handle[i]);
        }
        switch_dma = 1;
    }else{
        for (i = 3; i < 11; i++)
        {
            appData.dma_handle[i - 3] = SYS_DMA_ChannelAllocate(channel);

            eventSrc = DMA_TRIGGER_ADC1_DATA0 + (DMA_TRIGGER_SOURCE)ANx_Pins2[i];

            SYS_DMA_ChannelSetup(appData.dma_handle[i - 3], modeEnable, eventSrc);

            SYS_DMA_ChannelTransferEventHandlerSet(appData.dma_handle[i - 3],APP_DMA_EventHandler, i+1-3);

            SYS_DMA_ChannelTransferAdd(appData.dma_handle[i - 3], ((const unsigned int *)&AD1DATA0) + ANx_Pins2[i],
                    sizeof(ADC_DATA_TYPE), &appData.ADC_Data[i],
                    APP_NUM_ADC_SAMPLES * sizeof(ADC_DATA_TYPE), sizeof(ADC_DATA_TYPE));

            SYS_DMA_ChannelEnable(appData.dma_handle[i - 3]);
        }
        switch_dma = 0;
    }
}

/******************************************************************************
  Function:
    void APP_Normalize_Data ( void )

  Remarks:
    See prototype in app.h.
 */


void APP_Normalize_Data ( void )
{  
        addata[0] = appData.ADC_Data[1][0];
        addata[1] = appData.ADC_Data[0][0];
        addata[2] = appData.ADC_Data[2][0];
        addata[3] = appData.ADC_Data[3][0];
        addata[4] = appData.ADC_Data[4][0];
        addata[5] = appData.ADC_Data[5][0];
        addata[6] = appData.ADC_Data[6][0];
        addata[7] = appData.ADC_Data[7][0];
        addata[8] = appData.ADC_Data[8][0];
        addata[9] = appData.ADC_Data[9][0];
        addata[10] = appData.ADC_Data[10][0];
}

/******************************************************************************
  Function:
    void ADC_Normalize_Data ( void )

  Remarks:
    See prototype in adcp.h.
 */

ADC_DATA_TYPE ADC_Normalize_Data(ADC_DATA_TYPE _in)
{
    if (_in & 0b1000)
        return (ADC_DATA_TYPE)(_in >> 4) + 1;
    else
        return (ADC_DATA_TYPE)_in >> 4;
}

/******************************************************************************
  Function:
    void APP_DMA_Event ( void )

  Remarks:
    See prototype in app.h.
 */
void APP_DMA_EventHandler(SYS_DMA_TRANSFER_EVENT event,
        SYS_DMA_CHANNEL_HANDLE handle, uintptr_t context)
{
    switch (event)
    {
        case SYS_DMA_TRANSFER_EVENT_COMPLETE:   // We have ADC data
            appData.sampleCount++;
            break;

        case SYS_DMA_TRANSFER_EVENT_ERROR:
            BiosPrintf("SYS_DMA_TRANSFER_EVENT_ERROR\n");
            break;

        case SYS_DMA_TRANSFER_EVENT_ABORT:
            BiosPrintf("SYS_DMA_TRANSFER_EVENT_ABORT\n");
            break;

        default:
            break;
    }
}


/*******************************************************************************
 End of File
 */

