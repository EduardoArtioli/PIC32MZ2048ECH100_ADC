/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

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

#include "system/common/sys_common.h"
#include "app.h"
#include "system_definitions.h"

// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************
 
void __ISR(_UART1_TX_VECTOR, ipl1AUTO) _IntHandlerDrvUsartTransmitInstance0(void)
{
    DRV_USART_TasksTransmit(sysObj.drvUsart0);
}
void __ISR(_UART1_RX_VECTOR, ipl1AUTO) _IntHandlerDrvUsartReceiveInstance0(void)
{
    DRV_USART_TasksReceive(sysObj.drvUsart0);
}
void __ISR(_UART1_FAULT_VECTOR, ipl1AUTO) _IntHandlerDrvUsartErrorInstance0(void)
{
    DRV_USART_TasksError(sysObj.drvUsart0);
}
 
 

 

 

 

 

 
 

void __ISR(_DMA0_VECTOR, ipl5AUTO) _IntHandlerSysDmaCh0(void)
{          
    SYS_DMA_Tasks(sysObj.sysDma, DMA_CHANNEL_0);
}

void __ISR(_DMA1_VECTOR, ipl5AUTO) _IntHandlerSysDmaCh1(void)
{          
    SYS_DMA_Tasks(sysObj.sysDma, DMA_CHANNEL_1);
}


void __ISR(_DMA2_VECTOR, ipl5AUTO) _IntHandlerSysDmaCh2(void)
{          
    SYS_DMA_Tasks(sysObj.sysDma, DMA_CHANNEL_2);
}


void __ISR(_DMA3_VECTOR, ipl5AUTO) _IntHandlerSysDmaCh3(void)
{          
    SYS_DMA_Tasks(sysObj.sysDma, DMA_CHANNEL_3);
}


void __ISR(_DMA4_VECTOR, ipl5AUTO) _IntHandlerSysDmaCh4(void)
{          
    SYS_DMA_Tasks(sysObj.sysDma, DMA_CHANNEL_4);
}


void __ISR(_DMA5_VECTOR, ipl5AUTO) _IntHandlerSysDmaCh5(void)
{          
    SYS_DMA_Tasks(sysObj.sysDma, DMA_CHANNEL_5);
}


void __ISR(_DMA6_VECTOR, ipl5AUTO) _IntHandlerSysDmaCh6(void)
{          
    SYS_DMA_Tasks(sysObj.sysDma, DMA_CHANNEL_6);
}


void __ISR(_DMA7_VECTOR, ipl5AUTO) _IntHandlerSysDmaCh7(void)
{          
    SYS_DMA_Tasks(sysObj.sysDma, DMA_CHANNEL_7);
}

 

 /*******************************************************************************
 End of File
*/
