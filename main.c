/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC18F27Q43
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "flash.h"
#include "partest.h"

extern void vInitializeDemo( void );
static portTASK_FUNCTION( vBlockingTask, pvParameters );
/*
                         Main application
 */
void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global Interrupts
    // Use the following macros to:

    // Enable high priority global interrupts
    //INTERRUPT_GlobalInterruptHighEnable();

    // Enable low priority global interrupts.
    //INTERRUPT_GlobalInterruptLowEnable();

    // Disable high priority global interrupts
    //INTERRUPT_GlobalInterruptHighDisable();

    // Disable low priority global interrupts.
    //INTERRUPT_GlobalInterruptLowDisable();

    vInitializeDemo();
    vTaskStartScheduler();

    while (1)
    {
        // Add your application code
    }
}

void vInitializeDemo( void )
{
    vParTestInitialise();

    /* LED Flashing demo. */
    vStartLEDFlashTasks( tskIDLE_PRIORITY + 2 );
    
    /* Blocking task */
    xTaskCreate( vBlockingTask, "BLOCK", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY +1, ( TaskHandle_t * ) NULL );
}

static portTASK_FUNCTION( vBlockingTask, pvParameters )
{
    while (1)
    {
        /* The following portENABLE_INTERRUPTS() call should not be there, but
         * it's here as per an issue with the RETFIE instruction.
         * 
         * The code in portRESTORE_CONTEXT() does invoke a RETFIE instruction
         * execution when returning from an interrupt, which should set the GIEH
         * bit in the INTCON0 register.
         * 
         * For unknown reasons, this does not occur. As I don't have access to
         * the target hardware (PIC18F27Q43) I'm using the provided MPLABX IDE
         * simulator. I suspect that the simulation is wrong here.
         * 
         * The portENABLE_INTERRUPTS() set the GIEH bit here instead of the ISR.
         * 
         * Further debugging is still required.
         */
        
        portENABLE_INTERRUPTS();
    }
}

void vApplicationIdleHook( void )
{
    /* Enter sleep mode. */
    SLEEP();
    NOP();
}
/**
 End of File
*/