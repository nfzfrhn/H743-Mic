/*
 * H743_audio_ex.h
 *
 *  Created on: 09.03.2021
 *      Author: Nafiz
 */

#ifndef BSP_CUSTOM_H743_AUDIO_EX_H_
#define BSP_CUSTOM_H743_AUDIO_EX_H_

#include <stdlib.h>
#include "stm32h7xx_hal.h"

/*------------------------------------------------------------------------------
             CONFIGURATION: Audio Driver Configuration parameters
------------------------------------------------------------------------------*/



/* Audio status definition */
#define AUDIO_OK                            ((uint8_t)0)
#define AUDIO_ERROR                         ((uint8_t)1)
#define AUDIO_TIMEOUT                       ((uint8_t)2)

/* Audio In default settings */
#define DEFAULT_AUDIO_IN_FREQ               BSP_AUDIO_FREQUENCY_16K
#define DEFAULT_AUDIO_IN_BIT_RESOLUTION     ((uint8_t)16)
#define DEFAULT_AUDIO_IN_CHANNEL_NBR        ((uint8_t)2)
#define DEFAULT_AUDIO_IN_VOLUME             ((uint16_t)64)

/* AUDIO FREQUENCY */
#define AUDIO_FREQUENCY_192K          ((uint32_t)192000)
#define AUDIO_FREQUENCY_96K           ((uint32_t)96000)
#define AUDIO_FREQUENCY_48K           ((uint32_t)48000)
#define AUDIO_FREQUENCY_44K           ((uint32_t)44100)
#define AUDIO_FREQUENCY_32K           ((uint32_t)32000)
#define AUDIO_FREQUENCY_22K           ((uint32_t)22050)
#define AUDIO_FREQUENCY_16K           ((uint32_t)16000)
#define AUDIO_FREQUENCY_11K           ((uint32_t)11025)
#define AUDIO_FREQUENCY_8K            ((uint32_t)8000)

/*------------------------------------------------------------------------------
                        AUDIO IN CONFIGURATION
------------------------------------------------------------------------------*/

/* SAI peripheral configuration defines */
#define AUDIO_IN_SAIx                           SAI1_Block_B
#define AUDIO_IN_SAIx_CLK_ENABLE()              __HAL_RCC_SAI1_CLK_ENABLE()
#define AUDIO_IN_SAIx_CLK_DISABLE()             __HAL_RCC_SAI1_CLK_DISABLE()
#define AUDIO_IN_SAIx_AF                        GPIO_AF6_SAI1
#define AUDIO_IN_SAIx_SD_ENABLE()               __HAL_RCC_GPIOE_CLK_ENABLE()
#define AUDIO_IN_SAIx_SD_GPIO_PORT              GPIOE
#define AUDIO_IN_SAIx_SD_PIN                    GPIO_PIN_3

/* SAI DMA Stream definitions */
#define AUDIO_IN_SAIx_DMAx_CLK_ENABLE()         __HAL_RCC_DMA2_CLK_ENABLE()
#define AUDIO_IN_SAIx_DMAx_STREAM               DMA2_Stream4
#define AUDIO_IN_SAIx_DMAx_CHANNEL              DMA_CHANNEL_1
#define AUDIO_IN_SAIx_DMAx_IRQ                  DMA2_Stream4_IRQn
#define AUDIO_IN_SAIx_DMAx_PERIPH_DATA_SIZE     DMA_PDATAALIGN_HALFWORD
#define AUDIO_IN_SAIx_DMAx_MEM_DATA_SIZE        DMA_MDATAALIGN_HALFWORD

#define AUDIO_IN_INT_GPIO_ENABLE()               __HAL_RCC_GPIOJ_CLK_ENABLE()
#define AUDIO_IN_INT_GPIO_PORT                   GPIOJ
#define AUDIO_IN_INT_GPIO_PIN                    GPIO_PIN_12
#define AUDIO_IN_INT_IRQ                         EXTI15_10_IRQn

/* DFSDM Configuration defines */
#define AUDIO_DFSDMx_TOP_RIGHT_CHANNEL                  DFSDM_CHANNEL_0
#define AUDIO_DFSDMx_TOP_LEFT_CHANNEL                   DFSDM_CHANNEL_1
#define AUDIO_DFSDMx_BUTTOM_RIGHT_CHANNEL               DFSDM_CHANNEL_2
#define AUDIO_DFSDMx_BUTTOM_LEFT_CHANNEL                DFSDM_CHANNEL_3
#define AUDIO_DFSDMx_CHANNEL_4                   		DFSDM_CHANNEL_4
#define AUDIO_DFSDMx_CHANNEL_5                   		DFSDM_CHANNEL_5
#define AUDIO_DFSDMx_CHANNEL_6                   		DFSDM_CHANNEL_6
#define AUDIO_DFSDMx_CHANNEL_7                   		DFSDM_CHANNEL_7


#define AUDIO_DFSDMx_TOP_LEFT_FILTER                    DFSDM1_Filter0
#define AUDIO_DFSDMx_TOP_RIGHT_FILTER                   DFSDM1_Filter1
#define AUDIO_DFSDMx_BUTTOM_LEFT_FILTER                 DFSDM1_Filter2
#define AUDIO_DFSDMx_BUTTOM_RIGHT_FILTER                DFSDM1_Filter3

#define AUDIO_DFSDMx_CLK_ENABLE()                       __HAL_RCC_DFSDM1_CLK_ENABLE()
#define AUDIO_DFSDMx_CKOUT_PIN                          GPIO_PIN_3
#define AUDIO_DFSDMx_CKOUT_DMIC_GPIO_PORT               GPIOD
#define AUDIO_DFSDMx_CKOUT_DMIC_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOD_CLK_ENABLE()
#define AUDIO_DFSDMx_DMIC_DATIN1_PIN                    GPIO_PIN_3
#define AUDIO_DFSDMx_DMIC_DATIN5_PIN                    GPIO_PIN_11
#define AUDIO_DFSDMx_DMIC_DATIN_GPIO_PORT               GPIOC
#define AUDIO_DFSDMx_DMIC_DATIN_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOC_CLK_ENABLE()
#define AUDIO_DFSDMx_DMIC_DATIN_AF                      GPIO_AF3_DFSDM1
#define AUDIO_DFSDMx_CKOUT_AF                           GPIO_AF3_DFSDM1

/* DFSDM DMA Right and Left channels definitions */
#define AUDIO_DFSDMx_DMAx_CLK_ENABLE()                  __HAL_RCC_DMA2_CLK_ENABLE()
#define AUDIO_DFSDMx_DMAx_CHANNEL                       DMA_CHANNEL_8
#define AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE              DMA_PDATAALIGN_WORD
#define AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE                 DMA_MDATAALIGN_WORD

//Nafiz: Change from DMA2 to DMA1
#define AUDIO_DFSDMx_DMAx_TOP_LEFT_STREAM               DMA1_Stream0
#define AUDIO_DFSDMx_DMAx_TOP_LEFT_IRQ                  DMA1_Stream0_IRQn
#define AUDIO_DFSDMx_DMAx_TOP_LEFT_IRQHandler           DMA1_Stream0_IRQHandler

#define AUDIO_DFSDMx_DMAx_TOP_RIGHT_STREAM              DMA1_Stream5
#define AUDIO_DFSDMx_DMAx_TOP_RIGHT_IRQ                 DMA1_Stream5_IRQn
#define AUDIO_DFSDMx_DMAx_TOP_RIGHT_IRQHandler          DMA1_Stream5_IRQHandler

#define AUDIO_DFSDMx_DMAx_BUTTOM_LEFT_STREAM            DMA1_Stream6
#define AUDIO_DFSDMx_DMAx_BUTTOM_LEFT_IRQ               DMA1_Stream6_IRQn
#define AUDIO_DFSDMx_DMAx_BUTTOM_LEFT_IRQHandler        DMA1_Stream6_IRQHandler

#define AUDIO_DFSDMx_DMAx_BUTTOM_RIGHT_STREAM           DMA1_Stream7
#define AUDIO_DFSDMx_DMAx_BUTTOM_RIGHT_IRQ              DMA1_Stream7_IRQn
#define AUDIO_DFSDMx_DMAx_BUTTOM_RIGHT_IRQHandler       DMA1_Stream7_IRQHandler

/* Select the interrupt preemption priority and subpriority for the DMA interrupt */
#define AUDIO_IN_IRQ_PREPRIO                ((uint32_t)0x04)


/*------------------------------------------------------------------------------
                            OUTPUT DEVICES definition
------------------------------------------------------------------------------*/
/* Alias on existing output devices to adapt for 2 headphones output */
#define OUTPUT_DEVICE_HEADPHONE1 OUTPUT_DEVICE_HEADPHONE
#define OUTPUT_DEVICE_HEADPHONE2 OUTPUT_DEVICE_SPEAKER /* Headphone2 is connected to Speaker output of the wm8994 */

/*------------------------------------------------------------------------------
                           INPUT DEVICES definition
------------------------------------------------------------------------------*/
/* MP34DT01TR digital microphone on PCB top side */
#define INPUT_DEVICE_DIGITAL_MIC       ((uint16_t)0)
/* Analog microphone input from 3.5 audio jack connector */
#define INPUT_DEVICE_ANALOG_MIC        INPUT_DEVICE_INPUT_LINE_1

/**
  * @}
  */

/** @defgroup STM32F769I_DISCOVERY_AUDIO_Exported_Macros STM32F769I_DISCOVERY_AUDIO Exported Macros
  * @{
  */
#define DMA_MAX(x)           (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)

/**
  * @}
  */

/** @defgroup STM32F769I_DISCOVERY_AUDIO_IN_Exported_Functions STM32F769I_DISCOVERY_AUDIO_IN Exported Functions
  * @{
  */
uint8_t BSP_AUDIO_IN_Init(uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);
uint8_t BSP_AUDIO_IN_InitEx(uint16_t InputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);
uint8_t BSP_AUDIO_IN_AllocScratch (int32_t *pScratch, uint32_t size);
uint8_t BSP_AUDIO_IN_GetChannelNumber(void);
void    BSP_AUDIO_IN_DeInit(void);
uint8_t BSP_AUDIO_IN_Record(uint16_t *pData, uint32_t Size);
uint8_t BSP_AUDIO_IN_Stop(void);
uint8_t BSP_AUDIO_IN_Pause(void);
uint8_t BSP_AUDIO_IN_Resume(void);
uint8_t BSP_AUDIO_IN_Get_PcmBuffer(uint8_t* pbuf, uint16_t sample_count,
                                   uint16_t ScratchOffset, uint8_t res);

/* User Callbacks: user has to implement these functions in his code if they are needed. */
/* This function should be implemented by the user application.
   It is called into this driver when the current buffer is filled to prepare the next
   buffer pointer and its size. */
void    BSP_AUDIO_IN_TransferComplete_CallBack(void);
void    BSP_AUDIO_IN_HalfTransfer_CallBack(void);

/* This function is called when an Interrupt due to transfer error on or peripheral
   error occurs. */
void    BSP_AUDIO_IN_Error_CallBack(void);

/* These function can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void BSP_AUDIO_IN_ClockConfig(DFSDM_Filter_HandleTypeDef *hdfsdm_filter, uint32_t AudioFreq, void *Params);
void BSP_AUDIO_IN_MspInit(void);
void BSP_AUDIO_IN_MspDeInit(void);


#endif /* BSP_CUSTOM_H743_AUDIO_EX_H_ */
