/* Includes ------------------------------------------------------------------*/

#include "stdheader.h"

#include "dcc_stdio.h"

#include "hw_configuration.h"
#include "init.h"

#include "version.h"

#define useLIBDEBUG 1

#if (useLIBDEBUG == 1)
#include "stm32f2xx_ip_dbg.h"
#endif

#include "watchdog.h"

static RCC_ClocksTypeDef RCC_Clocks;

#if (configGENERATE_RUN_TIME_STATS == 1)

static volatile unsigned long ulRunTimeCount;

unsigned long vGetRunTimeCounterValue(void)
{
    return ulRunTimeCount;
}

void TIM6_DAC_IRQHandler(void)
{
    ulRunTimeCount++;
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}

void vConfigureTimerForRunTimeStats(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

    // --------------------------------------------------------------------------------
    // TIM6 configuration: generate interrupt for counter for runtimestats
    // The frequency of TIM6 depends on the frequency of APB1, APB1 prescaler,
    // the prescaler of TIM6, and the value of TIM_Period.
    // The frequency of APB1 depends on the SystemCoreClock and the APB1 prescaler.
    // Both are defined in "system_stm32f2xx.c".
    // Calculating the value of the prescaler for TIM6:
    // The frequency of the counter should be about 10 kHz (10 times the frequency of
    // tick count).
    // System frequency is 120 MHz, prescaler of APB1 is 4 so clock to timer is 60 MHz.
    // Number of counts is 2.
    // Prescaler = (60 MHz / 3 * 10 kHz) = 2000.
    // --------------------------------------------------------------------------------

    // Time Base configuration
    TIM_TimeBaseStructure.TIM_Prescaler = 2000;  // note comment above
    TIM_TimeBaseStructure.TIM_Period = 2;
    TIM_TimeBaseStructure.TIM_CounterMode = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    TIM_UpdateDisableConfig(TIM6, DISABLE);

    /* TIM6 counter enable */
    TIM_Cmd(TIM6, ENABLE);
}
#endif

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName)
{
    dbg_write_str("FAIL: Stack Overflow!");  // FIXME: dbg_write must not be called from an unprivileged task!
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
        asm("nop");
    }
}

#if (configUSE_TRACE_FACILITY == 1)
void vTraceCreateMutexFailed(void)
{
    dbg_write_str("FAIL: CreateMutex");  // FIXME: dbg_write must not be called from an unprivileged task!
}

void vTraceTaskCreateFailed(void)
{
    dbg_write_str("FAIL: CreateTask");  // FIXME: dbg_write must not be called from an unprivileged task!
}

void vTraceQueueCreateFailed(unsigned char ucQueueType)
{
    dbg_write_str("FAIL: QueueCreate");  // FIXME: dbg_write must not be called from an unprivileged task!
}

void vInitAnalogueOutput(void)
{
    DAC_InitTypeDef DAC_InitStructure;
    DAC_StructInit(&DAC_InitStructure);

    /* DAC channel1 Configuration */
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = 0;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);

    /* Enable DAC Channel1: Once the DAC channel1 is enabled, PA.04 is
       automatically connected to the DAC converter. */
    DAC_Cmd(DAC_Channel_1, ENABLE);
}

void vSetAnalogueOutput(int value)
{
    /* Set DAC Channel1 DHR12L register */
    DAC_SetChannel1Data(DAC_Align_8b_R, value);
}
#endif

extern uint8_t _isr_vector_offs[];

int main(void)
{
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)_isr_vector_offs);

#if (useLIBDEBUG == 1)
    // setup debug structures in LibDEBUG
    IP_Debug();
#endif

    /*!< At this stage the microcontroller clock setting is already configured,
         this is done through SystemInit() function which is called from startup
         file (startup_stm32f2xx.s) before to branch to application main.
         To reconfigure the default setting of SystemInit() function, refer to
         system_stm32f2xx.c file
       */

    RCC_GetClocksFreq(&RCC_Clocks);
    HWCONFIG_Init();

#if (configGENERATE_RUN_TIME_STATS == 1)
    vConfigureTimerForRunTimeStats();
#endif

#if (configUSE_TRACE_FACILITY == 1)
    vInitAnalogueOutput();
#endif


    /************************************************************************************/
    /* Enable interrupt controller                                                      */
    /************************************************************************************/

    xTaskCreate(WATCHDOG_Task,       (signed portCHAR*)"Watchdog       ", configMINIMAL_STACK_SIZE + 32,     NULL, (configMAX_PRIORITIES - 1) | portPRIVILEGE_BIT, NULL );
    xTaskCreate(INIT_Task,           (signed portCHAR*)"Init           ", configMINIMAL_STACK_SIZE + 128,    NULL, (configMAX_PRIORITIES - 2) | portPRIVILEGE_BIT, NULL );

    //asm ("bl 0x1233");

    /************************************************************************************/
    /* Startup the scheduler                                                            */
    /************************************************************************************/
    vTaskStartScheduler();
}


#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    dbg_write_str("Assert Failed: Wrong parameters value.");  // FIXME: dbg_write must not be called from an unprivileged task!
    dbg_write_str((char *)file);  // FIXME: dbg_write must not be called from an unprivileged task!
    dbg_write_u32(&line, 1);  // FIXME: dbg_write must not be called from an unprivileged task!

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

