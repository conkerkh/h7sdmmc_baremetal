#include "us_handler.h"

uint8_t dwt_initialised = 0;

/**
 * @brief  Initializes DWT_Clock_Cycle_Count for DWT_Delay_us function
 * @return Error DWT counter
 *         1: clock cycle counter not started
 *         0: clock cycle counter works
 */
uint32_t DWT_Init(void) {
    if (dwt_initialised) return 0;
#ifdef RTOS
    taskENTER_CRITICAL();
#endif
    /* Disable TRC */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
    /* Enable TRC */
    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

    /* Disable clock cycle counter */
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
    /* Enable  clock cycle counter */
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk; //0x00000001;

    /* Reset the clock cycle counter value */
    DWT->CYCCNT = 0;

    /* 3 NO OPERATION instructions */
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");

    /* Check if clock cycle counter has started */
    if(DWT->CYCCNT)
    {
        /*clock cycle counter started*/
        dwt_initialised = 1;
#ifdef RTOS
        taskEXIT_CRITICAL();
#endif
        return 0;
    }
    else
    {
        /*clock cycle counter not started*/
        dwt_initialised = 0;
#ifdef RTOS
        taskEXIT_CRITICAL();
#endif
        return 1;
    }
}

uint8_t DWT_IsInitialised(void)
{
    return dwt_initialised;
}
