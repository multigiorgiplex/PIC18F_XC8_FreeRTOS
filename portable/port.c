#include "FreeRTOS.h"
#include "task.h"

#define portCOMPILER_TEMP_AREA_SIZE    32
#define portINTERRUPT_INITIAL_MASK     0x80
#define portINTERRUPT_ENABLED_MASK     0x80

#define portRETURN()    asm ( "return" )
#define portM8( x )     do { x; x; x; x; x; x; x; x; } while( 0 )
#define portM32( x )    do { portM8( x ); portM8( x ); portM8( x ); portM8( x ); } while( 0 )

/* Suppress: (2028) external declaration for identifier "__Ltemp" doesn't indicate storage location */
#pragma warning push
#pragma warning disable 2028
/* Beginning of the temp area (Linker-created variable). */
extern uint8_t _Ltemp;
#pragma warning pop

/* TCB */
struct tskTaskControlBlock;
typedef struct tskTaskControlBlock TCB_t;
extern TCB_t * volatile pxCurrentTCB;

uint8_t intcon, wreg, status;

/*
 * Context switch.
 */

#define portSAVE_CONTEXT(forceInterrupts)                             \
    do                                                                \
    {                                                                 \
        status = STATUS;                                              \
        wreg = WREG;                                                  \
        intcon = forceInterrupts | INTCON0;                           \
        portDISABLE_INTERRUPTS();                                     \
        /* Save the registers. */                                     \
        POSTINC1 = status;                                            \
        POSTINC1 = wreg;                                              \
        POSTINC1 = intcon;                                            \
        POSTINC1 = BSR;                                               \
        POSTINC1 = PCLATH;                                            \
        POSTINC1 = PCLATU;                                            \
        POSTINC1 = FSR0L;                                             \
        POSTINC1 = FSR0H;                                             \
        POSTINC1 = FSR2L;                                             \
        POSTINC1 = FSR2H;                                             \
        POSTINC1 = PRODL;                                             \
        POSTINC1 = PRODH;                                             \
        POSTINC1 = TABLAT;                                            \
        POSTINC1 = TBLPTRL;                                           \
        POSTINC1 = TBLPTRH;                                           \
        POSTINC1 = TBLPTRU;                                           \
        /* Save the temp area. 32 bytes long. */                      \
        FSR0 = ( uint16_t ) &_Ltemp;                                  \
        portM32( POSTINC1 = POSTINC0 );                               \
        /* Save the return stack. */                                  \
        FSR0L = STKPTR;                                               \
        while( STKPTR > 0 )                                           \
        {                                                             \
            POSTINC1 = TOSL;                                          \
            POSTINC1 = TOSH;                                          \
            POSTINC1 = TOSU;                                          \
            asm ( "POP" );                                            \
        }                                                             \
        POSTINC1 = FSR0L;                                             \
        /* Save the stack pointer into pxCurrentTCB->pxTopOfStack. */ \
        *( uint16_t * ) pxCurrentTCB = FSR1;                          \
    } while( 0 )

#define portRESTORE_CONTEXT()                                                \
    do                                                                       \
    {                                                                        \
        /* Restore the original stack pointer. */                            \
        FSR1 = *( uint16_t * ) pxCurrentTCB;                                 \
        /* Skip the next available byte. */                                  \
        WREG = POSTDEC1;                                                     \
        /* Restore the return stack. */                                      \
        FSR0L = POSTDEC1;                                                    \
        STKPTR = 0;                                                          \
        while( FSR0L > 0 )                                                   \
        {                                                                    \
            asm ( "PUSH" );                                                  \
            WREG = POSTDEC1;                                                 \
            TOSU = WREG;                                                     \
            WREG = POSTDEC1;                                                 \
            TOSH = WREG;                                                     \
            WREG = POSTDEC1;                                                 \
            TOSL = WREG;                                                     \
            FSR0L--;                                                         \
        }                                                                    \
        /* Restore the temp area. 32 bytes long. */                          \
        FSR0 = ( uint16_t ) ( &_Ltemp + portCOMPILER_TEMP_AREA_SIZE - 1 );   \
        portM32( POSTDEC0 = POSTDEC1 );                                      \
        /* Restore the registers. */                                         \
        TBLPTRU = POSTDEC1;                                                  \
        TBLPTRH = POSTDEC1;                                                  \
        TBLPTRL = POSTDEC1;                                                  \
        TABLAT = POSTDEC1;                                                   \
        PRODH = POSTDEC1;                                                    \
        PRODL = POSTDEC1;                                                    \
        FSR2H = POSTDEC1;                                                    \
        FSR2L = POSTDEC1;                                                    \
        FSR0H = POSTDEC1;                                                    \
        FSR0L = POSTDEC1;                                                    \
        PCLATU = POSTDEC1;                                                   \
        PCLATH = POSTDEC1;                                                   \
        BSR = POSTDEC1;                                                      \
        intcon = POSTDEC1;                                                   \
        wreg = POSTDEC1;                                                     \
        status = INDF1;                                                      \
        if(intcon & portINTERRUPT_ENABLED_MASK )                             \
        {                                                                    \
            WREG = wreg;                                                     \
            STATUS = status;                                                 \
            asm ("RETFIE 0");                                                \
        } else {                                                             \
            WREG = wreg;                                                     \
            STATUS = status;                                                 \
            asm ("RETURN 0");                                                \
        }                                                                    \
    } while( 0 )

#define portCREATE_CONTEXT( pxTopOfStack, pxCode, pvParameters )          \
    do                                                                    \
    {                                                                     \
        FSR0 = ( uint16_t ) pxTopOfStack;                                 \
        /* Function parameters. */                                        \
        FSR2 = ( uint16_t ) &pvParameters;                                \
        POSTINC0 = POSTINC2;                                              \
        POSTINC0 = POSTINC2;                                              \
        /* Create a task context in the stack. */                         \
        /* The registers */                                               \
        POSTINC0 = ( uint8_t ) 0x00;                       /* STATUS. */  \
        POSTINC0 = ( uint8_t ) 0xBB;                       /* WREG. */    \
        POSTINC0 = ( uint8_t ) portINTERRUPT_INITIAL_MASK; /* INTCON0 */  \
        POSTINC0 = ( uint8_t ) 0x00;                       /* BSR. */     \
        POSTINC0 = ( uint8_t ) 0x00;                       /* PCLATH. */  \
        POSTINC0 = ( uint8_t ) 0x00;                       /* PCLATU. */  \
        POSTINC0 = ( uint8_t ) 0x11;                       /* FSR0L. */   \
        POSTINC0 = ( uint8_t ) 0x22;                       /* FSR0H. */   \
        POSTINC0 = ( uint8_t ) 0x33;                       /* FSR2L. */   \
        POSTINC0 = ( uint8_t ) 0x44;                       /* FSR2H. */   \
        POSTINC0 = ( uint8_t ) 0x55;                       /* PRODL. */   \
        POSTINC0 = ( uint8_t ) 0x66;                       /* PRODH. */   \
        POSTINC0 = ( uint8_t ) 0x77;                       /* TABLAT. */  \
        POSTINC0 = ( uint8_t ) 0x88;                       /* TBLPTRL. */ \
        POSTINC0 = ( uint8_t ) 0x99;                       /* TBLPTRH. */ \
        POSTINC0 = ( uint8_t ) 0x00;                       /* TBLPTRU. */ \
        /* Temp area. */                                                  \
        WREG = ( uint8_t ) 0xEE;                                          \
        portM32( POSTINC0 = WREG );                                       \
        /* Address of the task function. */                               \
        FSR2 = ( uint16_t ) &pxCode;                                      \
        POSTINC0 = POSTINC2;                                              \
        POSTINC0 = POSTINC2;                                              \
        POSTINC0 = POSTINC2;                                              \
        POSTINC0 = ( uint8_t ) 1; /* STKPTR. */                           \
        pxTopOfStack = ( StackType_t * ) FSR0;                            \
    } while( 0 )
/*-----------------------------------------------------------*/


/* Custom interrupt manager copied from mcc_generated_files/interrupt_manager.c 
 * The ISR is copied so the __interrupt() attribute is not used as the custom
 * portSAVE_CONTEXT and portRESTORE_CONTEXT is needed instead.
 */
__at(0x000008) void PORT_InterruptManagerHigh (void)
{
    portSAVE_CONTEXT(portINTERRUPT_ENABLED_MASK);
    
   // interrupt handler
    if(PIE3bits.TMR2IE == 1 && PIR3bits.TMR2IF == 1)
    {
        TMR2_ISR();
    }
    else
    {
        //Unhandled Interrupt
    }
    

    portRESTORE_CONTEXT();
}

/* vPortYield performs a context switch. */
void vPortYield( void )
{
    portSAVE_CONTEXT(0x00);
    vTaskSwitchContext();
    portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters )
{
    /* Create a task context. */
    portCREATE_CONTEXT( pxTopOfStack, pxCode, pvParameters );

    return pxTopOfStack;

    /* Dummy call for preventing the XC8 from deleting an 'uncalled' function. */
    pxCode( pvParameters );
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
    /* Start the system tick count. */
    portSTART_TICK_COUNT();

    /* Start the first task. */
    portRESTORE_CONTEXT();
    portRETURN();

    /* Does not get here. Adjust the XC8. */
    vPortAdjustCompiler();
    PORT_InterruptManagerHigh();

    return pdTRUE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not implemented. */
}
/*-----------------------------------------------------------*/

/* Tick count interrupt handler.
 * This increments the tick count and is a callback function,
 * which will be registered to the MCC-generated ISRs.
 */
void vPortTickCountHandler( void )
{
    /* Increment the tick count. */
    if (xTaskIncrementTick() == pdTRUE)
    {
        /* Switch if needed */
        vTaskSwitchContext();
    }
}
/*-----------------------------------------------------------*/

/* task.c */
extern List_t pxReadyTasksLists[ configMAX_PRIORITIES ];
extern List_t * volatile pxDelayedTaskList;
extern List_t xPendingReadyList;

/* This function is not called any time. Just tell pointer sizes to the XC8. */
void vPortAdjustCompiler( void )
{
    /* The XC8 might assume "void *" pointers as 24-bit or NULL only.
     * Assign 16-bit values for demonstrating to the compiler. */
    pxReadyTasksLists[ 0 ].xListEnd.pxNext->pvOwner = ( void * ) FSR0;
    pxDelayedTaskList->xListEnd.pxNext->pvOwner = ( void * ) FSR0;
    xPendingReadyList.xListEnd.pxNext->pvOwner = ( void * ) FSR0;
}