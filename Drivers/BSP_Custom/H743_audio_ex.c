/*
 * H743_audio_ex.c
 *
 *  Created on: 09.03.2021
 *      Author: Nafiz
 */

#include "H743_audio_ex.h"

/* Saturate the record PCM sample */
#define SaturaLH(N, L, H) (((N)<(L))?(L):(((N)>(H))?(H):(N)))

#define DFSDM_OVER_SAMPLING(__FREQUENCY__) \
        (__FREQUENCY__ == AUDIO_FREQUENCY_8K)  ? 256 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_11K) ? 256 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_16K) ? 128 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_22K) ? 128 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_32K) ? 64 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_44K) ? 64  \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_48K) ? 64 : 20  \

#define DFSDM_CLOCK_DIVIDER(__FREQUENCY__) \
        (__FREQUENCY__ == AUDIO_FREQUENCY_8K)  ? 24 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_11K) ? 4 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_16K) ? 24 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_22K) ? 4 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_32K) ? 24 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_44K) ? 4  \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_48K) ? 16 : 25  \

#define DFSDM_FILTER_ORDER(__FREQUENCY__) \
        (__FREQUENCY__ == AUDIO_FREQUENCY_8K)  ? DFSDM_FILTER_SINC3_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_11K) ? DFSDM_FILTER_SINC3_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_16K) ? DFSDM_FILTER_SINC3_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_22K) ? DFSDM_FILTER_SINC3_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_32K) ? DFSDM_FILTER_SINC4_ORDER \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_44K) ? DFSDM_FILTER_SINC3_ORDER  \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_48K) ? DFSDM_FILTER_SINC3_ORDER : DFSDM_FILTER_SINC5_ORDER  \

#define DFSDM_RIGHT_BIT_SHIFT(__FREQUENCY__) \
        (__FREQUENCY__ == AUDIO_FREQUENCY_8K)  ? 8 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_11K) ? 8 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_16K) ? 3 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_22K) ? 4 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_32K) ? 7 \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_44K) ? 0  \
      : (__FREQUENCY__ == AUDIO_FREQUENCY_48K) ? 0 : 4  \

/** @defgroup STM32F769I_DISCOVERY_AUDIO_Private_Variables STM32F769I_DISCOVERY_AUDIO Private Variables
  * @{
  */
/* PLAY */
//AUDIO_DrvTypeExtDef             *audio_drv;
SAI_HandleTypeDef               haudio_out_sai;
SAI_HandleTypeDef               haudio_in_sai;

DFSDM_Channel_HandleTypeDef     hAudioInTopLeftChannel;			//hAudioChannel1
DFSDM_Channel_HandleTypeDef     hAudioInTopRightChannel;		//hAudioChannel2
DFSDM_Channel_HandleTypeDef     hAudioInButtomLeftChannel;		//hAudioChannel3
DFSDM_Channel_HandleTypeDef     hAudioInButtomRightChannel;		//hAudioChannel4

DFSDM_Filter_HandleTypeDef      hAudioInTopLeftFilter;			//hAudioFilter1
DFSDM_Filter_HandleTypeDef      hAudioInTopRightFilter;			//hAudioFilter2
DFSDM_Filter_HandleTypeDef      hAudioInButtomLeftFilter;		//hAudioFilter3
DFSDM_Filter_HandleTypeDef      hAudioInButtomRightFilter;		//hAudioFilter4

DMA_HandleTypeDef               hDmaTopLeft;					//hDma1
DMA_HandleTypeDef               hDmaTopRight;					//hDma2
DMA_HandleTypeDef               hDmaButtomLeft;					//hDma3
DMA_HandleTypeDef               hDmaButtomRight;				//hDma4

/* Buffers for right and left samples */
static int32_t                  *pScratchBuff[2*DEFAULT_AUDIO_IN_CHANNEL_NBR];
static __IO int32_t             ScratchSize;
/* Channel number to be used: 2 channels by default */
static uint8_t                  AudioIn_ChannelNumber = DEFAULT_AUDIO_IN_CHANNEL_NBR;
/* Input device to be used: digital microphones by default */
static uint16_t                 AudioIn_Device = INPUT_DEVICE_DIGITAL_MIC;

/* Buffers status flags */
static uint32_t                DmaTopLeftRecHalfCplt  = 0;
static uint32_t                DmaTopLeftRecCplt      = 0;
static uint32_t                DmaTopRightRecHalfCplt = 0;
static uint32_t                DmaTopRightRecCplt     = 0;
static uint32_t                DmaButtomLeftRecHalfCplt  = 0;
static uint32_t                DmaButtomLeftRecCplt      = 0;
static uint32_t                DmaButtomRightRecHalfCplt = 0;
static uint32_t                DmaButtomRightRecCplt     = 0;

/* Application Buffer Trigger */
uint8_t __IO AudioOutResBit = 16; 		//24
uint8_t __IO AudioOutResByte = 2;		//3

/** @defgroup STM32F769I_DISCOVERY_AUDIO_Private_Function_Prototypes STM32F769I_DISCOVERY_AUDIO Private Function Prototypes
  * @{
  */
static void SAIx_Out_Init_Ext(uint32_t AudioFreq, uint8_t AudioResolution);
static void SAIx_Out_DeInit(void);
static void SAI_AUDIO_IN_MspInit(SAI_HandleTypeDef *hsai, void *Params);
static void SAI_AUDIO_IN_MspDeInit(SAI_HandleTypeDef *hsai, void *Params);
static void SAIx_In_Init(uint32_t AudioFreq);
static void SAIx_In_DeInit(void);
static void    DFSDMx_ChannelMspInit(void);
static void    DFSDMx_FilterMspInit(void);
static void    DFSDMx_ChannelMspDeInit(void);
static void    DFSDMx_FilterMspDeInit(void);
static uint8_t DFSDMx_Init(uint32_t AudioFreq);
static uint8_t DFSDMx_DeInit(void);


/**
  * @brief  Initializes SAI Audio IN MSP.
  * @param  hsai: SAI handle
  * @param  Params
  * @retval None
  */
static void SAI_AUDIO_IN_MspInit(SAI_HandleTypeDef *hsai, void *Params)
{
  static DMA_HandleTypeDef hdma_sai_rx;
  GPIO_InitTypeDef  gpio_init_structure;

  /* Enable SAI clock */
  AUDIO_IN_SAIx_CLK_ENABLE();

  /* Enable SD GPIO clock */
  AUDIO_IN_SAIx_SD_ENABLE();
  /* CODEC_SAI pin configuration: SD pin */
  gpio_init_structure.Pin = AUDIO_IN_SAIx_SD_PIN;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FAST;
  gpio_init_structure.Alternate = AUDIO_IN_SAIx_AF;
  HAL_GPIO_Init(AUDIO_IN_SAIx_SD_GPIO_PORT, &gpio_init_structure);

  /* Enable Audio INT GPIO clock */
  AUDIO_IN_INT_GPIO_ENABLE();
  /* Audio INT pin configuration: input */
  gpio_init_structure.Pin = AUDIO_IN_INT_GPIO_PIN;
  gpio_init_structure.Mode = GPIO_MODE_INPUT;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(AUDIO_IN_INT_GPIO_PORT, &gpio_init_structure);

  /* Enable the DMA clock */
  AUDIO_IN_SAIx_DMAx_CLK_ENABLE();

  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    /* Configure the hdma_sai_rx handle parameters */
//    hdma_sai_rx.Init.Channel             = AUDIO_IN_SAIx_DMAx_CHANNEL;
    hdma_sai_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_sai_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_sai_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_sai_rx.Init.PeriphDataAlignment = AUDIO_IN_SAIx_DMAx_PERIPH_DATA_SIZE;
    hdma_sai_rx.Init.MemDataAlignment    = AUDIO_IN_SAIx_DMAx_MEM_DATA_SIZE;
    hdma_sai_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_sai_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_sai_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_sai_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_sai_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE;

    hdma_sai_rx.Instance = AUDIO_IN_SAIx_DMAx_STREAM;

    /* Associate the DMA handle */
    __HAL_LINKDMA(hsai, hdmarx, hdma_sai_rx);

    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&hdma_sai_rx);

    /* Configure the DMA Stream */
    HAL_DMA_Init(&hdma_sai_rx);
  }

  /* SAI DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_IN_SAIx_DMAx_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
  HAL_NVIC_EnableIRQ(AUDIO_IN_SAIx_DMAx_IRQ);

  /* Audio INT IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_IN_INT_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
  HAL_NVIC_EnableIRQ(AUDIO_IN_INT_IRQ);
}


/**
  * @brief  De-Initializes SAI Audio IN MSP.
  * @param  hsai: SAI handle
  * @param  Params
  * @retval None
  */
static void SAI_AUDIO_IN_MspDeInit(SAI_HandleTypeDef *hsai, void *Params)
{
  GPIO_InitTypeDef  gpio_init_structure;

  /* SAI DMA IRQ Channel deactivation */
  HAL_NVIC_DisableIRQ(AUDIO_IN_SAIx_DMAx_IRQ);

  if(hsai->Instance == AUDIO_IN_SAIx)
  {
    /* Deinitialize the DMA stream */
    HAL_DMA_DeInit(hsai->hdmatx);
  }

  /* Disable SAI peripheral */
  __HAL_SAI_DISABLE(hsai);

  /* Deactivates CODEC_SAI pin SD by putting them in input mode */
  gpio_init_structure.Pin = AUDIO_IN_SAIx_SD_PIN;
  HAL_GPIO_DeInit(AUDIO_IN_SAIx_SD_GPIO_PORT, gpio_init_structure.Pin);

  gpio_init_structure.Pin = AUDIO_IN_INT_GPIO_PIN;
  HAL_GPIO_DeInit(AUDIO_IN_INT_GPIO_PORT, gpio_init_structure.Pin);

  /* Disable SAI clock */
  AUDIO_IN_SAIx_CLK_DISABLE();
}


/**
  * @brief  Initializes the Audio Codec audio interface (SAI).
  * @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
  * @note   The default SlotActive configuration is set to CODEC_AUDIOFRAME_SLOT_0123
  *         and user can update this configuration using
  * @retval None
  */
//static void SAIx_In_Init(uint32_t AudioFreq)
//{
//  /* Initialize SAI1 block A in MASTER TX */
//  /* Initialize the haudio_out_sai Instance parameter */
//  haudio_out_sai.Instance = AUDIO_OUT_SAIx;
//
//  /* Disable SAI peripheral to allow access to SAI internal registers */
//  __HAL_SAI_DISABLE(&haudio_out_sai);
//
//  /* Configure SAI_Block_x
//  LSBFirst: Disabled
//  DataSize: 16 */
//  haudio_out_sai.Init.MonoStereoMode = SAI_STEREOMODE;
//  haudio_out_sai.Init.AudioFrequency = AudioFreq;
//  haudio_out_sai.Init.AudioMode      = SAI_MODEMASTER_RX;
//  haudio_out_sai.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
//  haudio_out_sai.Init.Protocol       = SAI_FREE_PROTOCOL;
//  haudio_out_sai.Init.DataSize       = SAI_DATASIZE_16;
//  haudio_out_sai.Init.FirstBit       = SAI_FIRSTBIT_MSB;
//  haudio_out_sai.Init.ClockStrobing  = SAI_CLOCKSTROBING_FALLINGEDGE;
//  haudio_out_sai.Init.Synchro        = SAI_ASYNCHRONOUS;
//  haudio_out_sai.Init.OutputDrive    = SAI_OUTPUTDRIVE_ENABLE;
//  haudio_out_sai.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_1QF;
//  haudio_out_sai.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
//  haudio_out_sai.Init.CompandingMode = SAI_NOCOMPANDING;
//  haudio_out_sai.Init.TriState       = SAI_OUTPUT_NOTRELEASED;
//  haudio_out_sai.Init.Mckdiv         = 0;
//
//  /* Configure SAI_Block_x Frame
//  Frame Length: 64
//  Frame active Length: 32
//  FS Definition: Start frame + Channel Side identification
//  FS Polarity: FS active Low
//  FS Offset: FS asserted one bit before the first bit of slot 0 */
//  haudio_out_sai.FrameInit.FrameLength       = 64;
//  haudio_out_sai.FrameInit.ActiveFrameLength = 32;
//  haudio_out_sai.FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
//  haudio_out_sai.FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
//  haudio_out_sai.FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;
//
//  /* Configure SAI Block_x Slot
//  Slot First Bit Offset: 0
//  Slot Size  : 16
//  Slot Number: 4
//  Slot Active: All slot actives */
//  haudio_out_sai.SlotInit.FirstBitOffset = 0;
//  haudio_out_sai.SlotInit.SlotSize       = SAI_SLOTSIZE_DATASIZE;
//  haudio_out_sai.SlotInit.SlotNumber     = 4;
//  haudio_out_sai.SlotInit.SlotActive     = CODEC_AUDIOFRAME_SLOT_02;
//
//  HAL_SAI_Init(&haudio_out_sai);
//
//  /* Initialize SAI1 block B in SLAVE RX synchronous from SAI1 block A */
//  /* Initialize the haudio_in_sai Instance parameter */
//  haudio_in_sai.Instance = AUDIO_IN_SAIx;
//
//  /* Disable SAI peripheral to allow access to SAI internal registers */
//  __HAL_SAI_DISABLE(&haudio_in_sai);
//
//  /* Configure SAI_Block_x
//  LSBFirst: Disabled
//  DataSize: 16 */
//  haudio_in_sai.Init.MonoStereoMode = SAI_STEREOMODE;
//  haudio_in_sai.Init.AudioFrequency = AudioFreq;
//  haudio_in_sai.Init.AudioMode      = SAI_MODESLAVE_RX;
//  haudio_in_sai.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
//  haudio_in_sai.Init.Protocol       = SAI_FREE_PROTOCOL;
//  haudio_in_sai.Init.DataSize       = SAI_DATASIZE_16;
//  haudio_in_sai.Init.FirstBit       = SAI_FIRSTBIT_MSB;
//  haudio_in_sai.Init.ClockStrobing  = SAI_CLOCKSTROBING_RISINGEDGE;
//  haudio_in_sai.Init.Synchro        = SAI_SYNCHRONOUS;
//  haudio_in_sai.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
//  haudio_in_sai.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_1QF;
//  haudio_in_sai.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
//  haudio_in_sai.Init.CompandingMode = SAI_NOCOMPANDING;
//  haudio_in_sai.Init.TriState       = SAI_OUTPUT_RELEASED;
//  haudio_in_sai.Init.Mckdiv         = 0;
//
//  /* Configure SAI_Block_x Frame
//  Frame Length: 64
//  Frame active Length: 32
//  FS Definition: Start frame + Channel Side identification
//  FS Polarity: FS active Low
//  FS Offset: FS asserted one bit before the first bit of slot 0 */
//  haudio_in_sai.FrameInit.FrameLength       = 64;
//  haudio_in_sai.FrameInit.ActiveFrameLength = 32;
//  haudio_in_sai.FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
//  haudio_in_sai.FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
//  haudio_in_sai.FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;
//
//  /* Configure SAI Block_x Slot
//  Slot First Bit Offset: 0
//  Slot Size  : 16
//  Slot Number: 4
//  Slot Active: All slot active */
//  haudio_in_sai.SlotInit.FirstBitOffset = 0;
//  haudio_in_sai.SlotInit.SlotSize       = SAI_SLOTSIZE_DATASIZE;
//  haudio_in_sai.SlotInit.SlotNumber     = 4;
//  haudio_in_sai.SlotInit.SlotActive     = CODEC_AUDIOFRAME_SLOT_02;
//
//  HAL_SAI_Init(&haudio_in_sai);
//
//  /* Enable SAI peripheral */
//  __HAL_SAI_ENABLE(&haudio_in_sai);
//
//  /* Enable SAI peripheral to generate MCLK */
//  __HAL_SAI_ENABLE(&haudio_out_sai);
//}

/**
  * @brief  Deinitializes the output Audio Codec audio interface (SAI).
  * @retval None
  */
//static void SAIx_In_DeInit(void)
//{
//  /* Initialize the haudio_in_sai Instance parameter */
//  haudio_in_sai.Instance = AUDIO_IN_SAIx;
//  haudio_out_sai.Instance = AUDIO_OUT_SAIx;
//  /* Disable SAI peripheral */
//  __HAL_SAI_DISABLE(&haudio_in_sai);
//
//  HAL_SAI_DeInit(&haudio_in_sai);
//  HAL_SAI_DeInit(&haudio_out_sai);
//}


/**
  * @brief  Initialize wave recording.
  * @param  AudioFreq: Audio frequency to be configured for the DFSDM peripheral.
  * @param  BitRes: Audio frequency to be configured for the DFSDM peripheral.
  * @param  ChnlNbr: Audio frequency to be configured for the DFSDM peripheral.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Init(uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr)
{
  return BSP_AUDIO_IN_InitEx(INPUT_DEVICE_DIGITAL_MIC, AudioFreq, BitRes, ChnlNbr);
}


/**
  * @brief  Initialize wave recording.
  * @param  InputDevice: INPUT_DEVICE_DIGITAL_MIC or INPUT_DEVICE_ANALOG_MIC.
  * @param  AudioFreq: Audio frequency to be configured.
  * @param  BitRes: Audio bit resolution to be configured..
  * @param  ChnlNbr: Number of channel to be configured.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_InitEx(uint16_t InputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr)
{
  uint8_t ret = AUDIO_ERROR;
  AudioIn_Device = InputDevice;

  if(InputDevice == INPUT_DEVICE_DIGITAL_MIC)
  {
    AudioIn_ChannelNumber = ChnlNbr;
    /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
    BSP_AUDIO_IN_ClockConfig(&hAudioInTopLeftFilter, AudioFreq, NULL);

    /* Init the SAI MSP: this __weak function can be redefined by the application*/
    BSP_AUDIO_IN_MspInit();

    /* Initializes DFSDM peripheral */
    DFSDMx_Init(AudioFreq);
    ret = AUDIO_OK;
  }
  else
  {
//    /* Disable SAI */
//    SAIx_In_DeInit();
//
//    /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */
//    BSP_AUDIO_OUT_ClockConfig(&haudio_in_sai, AudioFreq, NULL);
//
//    haudio_in_sai.Instance = AUDIO_IN_SAIx;
//    if(HAL_SAI_GetState(&haudio_in_sai) == HAL_SAI_STATE_RESET)
//    {
//    BSP_AUDIO_OUT_MspInit(&haudio_in_sai, NULL);
//    BSP_AUDIO_IN_MspInit();
//    }
//
//    SAIx_In_Init(AudioFreq);
//
//    if((wm8994_drv.ReadID(AUDIO_I2C_ADDRESS)) == WM8994_ID)
//    {
//      /* Reset the Codec Registers */
//      wm8994_drv.Reset(AUDIO_I2C_ADDRESS);
//      /* Initialize the audio driver structure */
//      audio_drv = &wm8994_drv;
//      ret = AUDIO_OK;
//    }
//    else
//    {
//      ret = AUDIO_ERROR;
//    }
//
//    if(ret == AUDIO_OK)
//    {
//      /* Initialize the codec internal registers */
//      audio_drv->Init(AUDIO_I2C_ADDRESS, InputDevice, 100, AudioFreq,16);
//    }
  }

  /* Return AUDIO_OK when all operations are correctly done */
  return ret;
}


/**
  * @brief  Allocate channel buffer scratch
  * @param  pScratch : pointer to scratch tables.
  * @param  size of scratch buffer
  */
uint8_t BSP_AUDIO_IN_AllocScratch (int32_t *pScratch, uint32_t size)
{
  uint32_t idx;

  ScratchSize = (size / AudioIn_ChannelNumber);

  /* copy scratch pointers */
  for (idx = 0; idx < AudioIn_ChannelNumber; idx++)
  {
    pScratchBuff[idx] = (int32_t *)(pScratch + (idx * ScratchSize));
  }
  /* Return AUDIO_OK */
  return AUDIO_OK;
}


/**
  * @brief  Return audio in channel number
  * @retval Number of channel
  */
uint8_t BSP_AUDIO_IN_GetChannelNumber(void)
{
  return AudioIn_ChannelNumber;
}


/**
  * @brief  Start audio recording.
  * @param  pbuf: Main buffer pointer for the recorded data storing
  * @param  size: Current size of the recorded buffer
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Record(uint16_t* pbuf, uint32_t size)
{
  if (AudioIn_Device == INPUT_DEVICE_DIGITAL_MIC)
  {

    if(AudioIn_ChannelNumber > 2)
    {
      /* Call the Media layer start function for buttom right channel */
      if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hAudioInButtomRightFilter, pScratchBuff[2], ScratchSize))
      {
        return AUDIO_ERROR;
      }

      /* Call the Media layer start function for buttom left channel */
      if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hAudioInButtomLeftFilter, pScratchBuff[3], ScratchSize))
      {
        return AUDIO_ERROR;
      }
    }

    /* Call the Media layer start function for top right channel */
    if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hAudioInTopRightFilter, pScratchBuff[0], ScratchSize))
    {
      return AUDIO_ERROR;
    }

    /* Call the Media layer start function for top left channel */
    if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hAudioInTopLeftFilter, pScratchBuff[1], ScratchSize))
    {
      return AUDIO_ERROR;
    }
  }
  else
  {
    /* Start the process receive DMA */
    if(HAL_OK !=HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*)pbuf, size))
    {
      return AUDIO_ERROR;
    }
  }
  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}


/**
  * @brief  Stop audio recording.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Stop(void)
{
  if (AudioIn_Device == INPUT_DEVICE_DIGITAL_MIC)
  {

    if(AudioIn_ChannelNumber > 2)
    {
      /* Call the Media layer stop function for buttom right channel */
      if(HAL_OK != HAL_DFSDM_FilterRegularStop_DMA(&hAudioInButtomRightFilter))
      {
        return AUDIO_ERROR;
      }

      /* Call the Media layer stop function for buttom left channel */
      if(HAL_OK != HAL_DFSDM_FilterRegularStop_DMA(&hAudioInButtomLeftFilter))
      {
        return AUDIO_ERROR;
      }
    }

    /* Call the Media layer stop function for top right channel */
    if(HAL_OK != HAL_DFSDM_FilterRegularStop_DMA(&hAudioInTopRightFilter))
    {
      return AUDIO_ERROR;
    }

    /* Call the Media layer stop function for top left channel */
    if(HAL_OK != HAL_DFSDM_FilterRegularStop_DMA(&hAudioInTopLeftFilter))
    {
      return AUDIO_ERROR;
    }
  }
  else
  {
//    /* Call the Media layer stop function */
//    HAL_SAI_DMAStop(&haudio_in_sai);
//
//    /* Call Audio Codec Stop function */
//    if(audio_drv->Stop(AUDIO_I2C_ADDRESS, CODEC_PDWN_HW) != 0)
//    {
//      return AUDIO_ERROR;
//    }
//    else
//    {
//      /* Wait at least 100us */
//      HAL_Delay(1);
//
//      /* Return AUDIO_OK when all operations are correctly done */
//      return AUDIO_OK;
//    }
	  return AUDIO_ERROR;		//We are using digital microphone
  }
  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}


/**
  * @brief  Pause the audio file stream.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Pause(void)
{
  if(AudioIn_ChannelNumber > 2)
  {
    /* Call the Media layer stop function */
    if(HAL_OK != HAL_DFSDM_FilterRegularStop_DMA(&hAudioInButtomRightFilter))
    {
      return AUDIO_ERROR;
    }

    /* Call the Media layer stop function */
    if(HAL_OK != HAL_DFSDM_FilterRegularStop_DMA(&hAudioInButtomLeftFilter))
    {
      return AUDIO_ERROR;
    }
  }
  /* Call the Media layer stop function */
  if(HAL_OK != HAL_DFSDM_FilterRegularStop_DMA(&hAudioInTopRightFilter))
  {
    return AUDIO_ERROR;
  }

  /* Call the Media layer stop function */
  if(HAL_OK != HAL_DFSDM_FilterRegularStop_DMA(&hAudioInTopLeftFilter))
  {
    return AUDIO_ERROR;
  }

  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}


/**
  * @brief  Resume the audio file stream.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Resume(void)
{
  if(AudioIn_ChannelNumber > 2)
  {
    /* Call the Media layer start function for buttom right channel */
    if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hAudioInButtomRightFilter, pScratchBuff[2], ScratchSize))
    {
      return AUDIO_ERROR;
    }

    /* Call the Media layer start function for buttom left channel */
    if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hAudioInButtomLeftFilter, pScratchBuff[3], ScratchSize))
    {
      return AUDIO_ERROR;
    }
  }
  /* Call the Media layer start function for top right channel */
  if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hAudioInTopRightFilter, pScratchBuff[0], ScratchSize))
  {
    return AUDIO_ERROR;
  }

  /* Call the Media layer start function for top left channel */
  if(HAL_OK != HAL_DFSDM_FilterRegularStart_DMA(&hAudioInTopLeftFilter, pScratchBuff[1], ScratchSize))
  {
    return AUDIO_ERROR;
  }

  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}


/**
  * @brief  Deinit the audio IN peripherals.
  * @retval None
  */
void BSP_AUDIO_IN_DeInit(void)
{
  BSP_AUDIO_IN_MspDeInit();

  if(AudioIn_Device == INPUT_DEVICE_DIGITAL_MIC)
  {
    DFSDMx_DeInit();
  }
  else
  {
    SAIx_In_DeInit();
  }
}


/**
  * @brief  Regular conversion complete callback.
  * @note   In interrupt mode, user has to read conversion value in this function
            using HAL_DFSDM_FilterGetRegularValue.
  * @param  hdfsdm_filter : DFSDM filter handle.
  * @retval None
  */
void HAL_DFSDM_FilterRegConvCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{

  if(hdfsdm_filter == &hAudioInTopLeftFilter)
  {
    DmaTopLeftRecCplt = 1;
  }
  else if(hdfsdm_filter == &hAudioInTopRightFilter)
  {
    DmaTopRightRecCplt = 1;
  }
  else if(hdfsdm_filter == &hAudioInButtomLeftFilter)
  {
    DmaButtomLeftRecCplt = 1;
  }
  else
  {
    DmaButtomRightRecCplt = 1;
  }

  if(AudioIn_ChannelNumber > 2)
  {
    if((DmaTopLeftRecCplt == 1) && (DmaTopRightRecCplt == 1) && (DmaButtomLeftRecCplt == 1) && (DmaButtomRightRecCplt == 1))
    {
      DmaTopLeftRecCplt  = 0;
      DmaTopRightRecCplt = 0;
      DmaButtomLeftRecCplt  = 0;
      DmaButtomRightRecCplt = 0;
      BSP_AUDIO_IN_TransferComplete_CallBack();
    }
  }
  else
  {
    if((DmaTopLeftRecCplt == 1) && (DmaTopRightRecCplt == 1))
    {
      DmaTopLeftRecCplt  = 0;
      DmaTopRightRecCplt = 0;
      BSP_AUDIO_IN_TransferComplete_CallBack();
    }
  }
}


/**
  * @brief  BSP_AUDIO_IN_Get_PcmBuffer.
  * @note   Return last received PCM buffer from scratch buffer

  * @param  pbuf : Buffer where to copy samples.
  * @param  sample_count : Number of samples to copy.
  * @param  ScratchOffset : Offset from where to begin copy.
  * @param  res :audio Resolution should be 2 or 3.
  * @retval None
  */
uint8_t BSP_AUDIO_IN_Get_PcmBuffer(uint8_t* pbuf, uint16_t sample_count, uint16_t ScratchOffset, uint8_t res)
{
  if( res == 2)
  {
    uint16_t*  pbuf16 =(uint16_t*)pbuf;

    for(int i = 0, j = ScratchOffset; j < (ScratchOffset + sample_count); j++)
    {
      pbuf16[i++] = (uint16_t)(SaturaLH((pScratchBuff[1][j] >> 8), -32760, 32760));
      pbuf16[i++] = (uint16_t)(SaturaLH((pScratchBuff[0][j] >> 8), -32760, 32760));
    }
  }
  else
  {
    uint32_t sample ;

    for(int i = 0, j = ScratchOffset; j < (ScratchOffset + sample_count); j++)
    {
      sample = (uint32_t)(SaturaLH((pScratchBuff[1][j] ), -8388600, 8388600));
      pbuf[i++] =(sample)&0xFF;
      pbuf[i++] =(sample>>8)&0xFF;
      pbuf[i++] =(sample>>16)&0xFF;
      sample = (uint32_t)(SaturaLH((pScratchBuff[0][j] ), -8388600, 8388600));
      pbuf[i++] =(sample)&0xFF;
      pbuf[i++] =(sample>>8)&0xFF;
      pbuf[i++] =(sample>>16)&0xFF;
    }
  }
  return 0;
}


/**
  * @brief  Half regular conversion complete callback.
  * @param  hdfsdm_filter : DFSDM filter handle.
  * @retval None
  */
void HAL_DFSDM_FilterRegConvHalfCpltCallback(DFSDM_Filter_HandleTypeDef *hdfsdm_filter)
{

  if(hdfsdm_filter == &hAudioInTopLeftFilter)
  {
    DmaTopLeftRecHalfCplt = 1;
  }
  else if(hdfsdm_filter == &hAudioInTopRightFilter)
  {
    DmaTopRightRecHalfCplt = 1;
  }
  else if(hdfsdm_filter == &hAudioInButtomLeftFilter)
  {
    DmaButtomLeftRecHalfCplt = 1;
  }
  else
  {
    DmaButtomRightRecHalfCplt = 1;
  }

  if(AudioIn_ChannelNumber > 2)
  {
    if((DmaTopLeftRecHalfCplt == 1) && (DmaTopRightRecHalfCplt == 1) && (DmaButtomLeftRecHalfCplt == 1) && (DmaButtomRightRecHalfCplt == 1))
    {
      DmaTopLeftRecHalfCplt  = 0;
      DmaTopRightRecHalfCplt = 0;
      DmaButtomLeftRecHalfCplt  = 0;
      DmaButtomRightRecHalfCplt = 0;
      BSP_AUDIO_IN_HalfTransfer_CallBack();
    }
  }
  else
  {
    if((DmaTopLeftRecHalfCplt == 1) && (DmaTopRightRecHalfCplt == 1))
    {
      DmaTopLeftRecHalfCplt  = 0;
      DmaTopRightRecHalfCplt = 0;
      BSP_AUDIO_IN_HalfTransfer_CallBack();
    }
  }
}


/**
  * @brief  Half reception complete callback.
  * @param  hsai : SAI handle.
  * @retval None
  */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Manage the remaining file size and new address offset: This function
     should be coded by user (its prototype is already declared in stm32769i_discovery_audio.h) */
  BSP_AUDIO_IN_HalfTransfer_CallBack();
}

/**
  * @brief  Reception complete callback.
  * @param  hsai : SAI handle.
  * @retval None
  */
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Call the record update function to get the next buffer to fill and its size (size is ignored) */
  BSP_AUDIO_IN_TransferComplete_CallBack();
}

/**
  * @brief  User callback when record buffer is filled.
  * @retval None
  */
__weak void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
  /* This function should be implemented by the user alication.
     It is called into this driver when the current buffer is filled
     to prepare the next buffer pointer and its size. */
}


/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @retval None
  */
__weak void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
  /* This function should be implemented by the user application.
     It is called into this driver when the current buffer is filled
     to prepare the next buffer pointer and its size. */
}

/**
  * @brief  Audio IN Error callback function.
  * @retval None
  */
__weak void BSP_AUDIO_IN_Error_CallBack(void)
{
  /* This function is called when an Interrupt due to transfer error on or peripheral
     error occurs. */
}


/**
  * @brief  Initialize BSP_AUDIO_IN MSP.
  * @retval None
  */
__weak void BSP_AUDIO_IN_MspInit(void)
{
  if (AudioIn_Device == INPUT_DEVICE_DIGITAL_MIC)
  {
  /* MSP channels initialization */
  DFSDMx_ChannelMspInit();
  /* MSP filters initialization */
  DFSDMx_FilterMspInit();
  }
  else
  {
   SAI_AUDIO_IN_MspInit(&haudio_in_sai, NULL);
  }
}


/**
  * @brief  DeInitialize BSP_AUDIO_IN MSP.
  * @retval None
  */
__weak void BSP_AUDIO_IN_MspDeInit(void)
{
  if (AudioIn_Device == INPUT_DEVICE_DIGITAL_MIC)
  {
    /* MSP channels initialization */
    DFSDMx_ChannelMspDeInit();
    /* MSP filters initialization */
    DFSDMx_FilterMspDeInit();
  }
  else
  {
    SAI_AUDIO_IN_MspDeInit(&haudio_in_sai, NULL);
  }
}


/**
  * @brief  Clock Config.
  * @param  hdfsdm_filter: might be required to set audio peripheral predivider if any.
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Params
  * @note   This API is called by BSP_AUDIO_IN_Init()
  *         Being __weak it can be overwritten by the application
  * @retval None
  */

__weak void BSP_AUDIO_IN_ClockConfig(DFSDM_Filter_HandleTypeDef *hdfsdm_filter, uint32_t AudioFreq, void *Params)
{
  RCC_PeriphCLKInitTypeDef rcc_ex_clk_init_struct;

  HAL_RCCEx_GetPeriphCLKConfig(&rcc_ex_clk_init_struct);

  /* Set the PLL configuration according to the audio frequency */
  if((AudioFreq == AUDIO_FREQUENCY_11K) || (AudioFreq == AUDIO_FREQUENCY_22K) || (AudioFreq == AUDIO_FREQUENCY_44K))
  {
    /* Configure PLLSAI prescalers */
    /* PLLSAI_VCO: VCO_429M
    SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
    SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 214.5/19 = 11.289 Mhz */
//    rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
//    rcc_ex_clk_init_struct.Sai2ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI;
//    rcc_ex_clk_init_struct.PLLSAI.PLLSAIN = 429;
//    rcc_ex_clk_init_struct.PLLSAI.PLLSAIQ = 2;
//    rcc_ex_clk_init_struct.PLLSAIDivQ = 19;
//
//    HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct);

  }
  else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */
  {
    /* SAI clock config
    PLLSAI_VCO: VCO_344M
    SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 344/7 = 49.142 Mhz
    SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 49.142/1 = 49.142 Mhz */
    rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    rcc_ex_clk_init_struct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL;
    rcc_ex_clk_init_struct.PLL2.PLL2M = 25;
    rcc_ex_clk_init_struct.PLL2.PLL2N = 338;
    rcc_ex_clk_init_struct.PLL2.PLL2P = 7;
    rcc_ex_clk_init_struct.PLL2.PLL2Q = 2;
    rcc_ex_clk_init_struct.PLL2.PLL2R = 2;

    HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct);
  }
  rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_DFSDM1;
  rcc_ex_clk_init_struct.Dfsdm1ClockSelection = RCC_DFSDM1CLKSOURCE_D2PCLK1;
  HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct);
}


/*******************************************************************************
                            Static Functions
*******************************************************************************/
/**
  * @brief  Initialize the Digital Filter for Sigma-Delta Modulators interface (DFSDM).
  * @param  AudioFreq: Audio frequency to be used to set correctly the DFSDM peripheral.
  * @note   Channel output Clock Divider and Filter Oversampling are calculated as follow:
  *         - Clock_Divider = CLK(input DFSDM)/CLK(micro) with
  *           1MHZ < CLK(micro) < 3.2MHZ (TYP 2.4MHZ for MP34DT01TR)
  *         - Oversampling = CLK(input DFSDM)/(Clock_Divider * AudioFreq)
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
static uint8_t DFSDMx_Init(uint32_t AudioFreq)
{
  /****************************************************************************/
  /********************** Channels configuration  *****************************/
  /****************************************************************************/
  /* CHANNEL 1 configuration */
  __HAL_DFSDM_CHANNEL_RESET_HANDLE_STATE(&hAudioInTopLeftChannel);
  hAudioInTopLeftChannel.Instance                      = DFSDM1_Channel1;
  hAudioInTopLeftChannel.Init.OutputClock.Activation   = ENABLE;
  hAudioInTopLeftChannel.Init.OutputClock.Selection    = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
  /* Set the DFSDM clock OUT audio frequency configuration */
  hAudioInTopLeftChannel.Init.OutputClock.Divider      = DFSDM_CLOCK_DIVIDER(AudioFreq);
  hAudioInTopLeftChannel.Init.Input.Multiplexer        = DFSDM_CHANNEL_EXTERNAL_INPUTS;
  hAudioInTopLeftChannel.Init.Input.DataPacking        = DFSDM_CHANNEL_STANDARD_MODE;
  hAudioInTopLeftChannel.Init.Input.Pins               = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
  /* Request to sample stable data for LEFT micro on Rising edge */
  hAudioInTopLeftChannel.Init.SerialInterface.Type     = DFSDM_CHANNEL_SPI_RISING;
  hAudioInTopLeftChannel.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
  hAudioInTopLeftChannel.Init.Awd.FilterOrder          = DFSDM_FILTER_ORDER(AudioFreq);
  hAudioInTopLeftChannel.Init.Awd.Oversampling         = DFSDM_OVER_SAMPLING(AudioFreq);
//  hAudioInTopLeftChannel.Init.Awd.FilterOrder          = DFSDM_CHANNEL_FASTSINC_ORDER;
//  hAudioInTopLeftChannel.Init.Awd.Oversampling         = 10;
  hAudioInTopLeftChannel.Init.Offset                   = 0;
  hAudioInTopLeftChannel.Init.RightBitShift            = DFSDM_RIGHT_BIT_SHIFT(AudioFreq);
  if(HAL_OK != HAL_DFSDM_ChannelInit(&hAudioInTopLeftChannel))
  {
    return AUDIO_ERROR;
  }

  /* CHANNEL 0 configuration */
  __HAL_DFSDM_CHANNEL_RESET_HANDLE_STATE(&hAudioInTopRightChannel);
  hAudioInTopRightChannel.Instance                      = DFSDM1_Channel0;
  hAudioInTopRightChannel.Init.OutputClock.Activation   = ENABLE;
  hAudioInTopRightChannel.Init.OutputClock.Selection    = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
  /* Set the DFSDM clock OUT audio frequency configuration */
  hAudioInTopRightChannel.Init.OutputClock.Divider      = DFSDM_CLOCK_DIVIDER(AudioFreq);
  hAudioInTopRightChannel.Init.Input.Multiplexer        = DFSDM_CHANNEL_EXTERNAL_INPUTS;
  hAudioInTopRightChannel.Init.Input.DataPacking        = DFSDM_CHANNEL_STANDARD_MODE;
  hAudioInTopRightChannel.Init.Input.Pins               = DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS;
  /* Request to sample stable data for RIGHT micro on Falling edge */
  hAudioInTopRightChannel.Init.SerialInterface.Type     = DFSDM_CHANNEL_SPI_FALLING;
  hAudioInTopRightChannel.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
  hAudioInTopRightChannel.Init.Awd.FilterOrder          = DFSDM_CHANNEL_FASTSINC_ORDER;
  hAudioInTopRightChannel.Init.Awd.Oversampling         = 10;
  hAudioInTopRightChannel.Init.Offset                   = 0;
  hAudioInTopRightChannel.Init.RightBitShift            = DFSDM_RIGHT_BIT_SHIFT(AudioFreq);
  if(HAL_OK != HAL_DFSDM_ChannelInit(&hAudioInTopRightChannel))
  {
    return AUDIO_ERROR;
  }

  if(AudioIn_ChannelNumber > 2)
  {
    /* CHANNEL 5 configuration */
    __HAL_DFSDM_CHANNEL_RESET_HANDLE_STATE(&hAudioInButtomLeftChannel);
    hAudioInButtomLeftChannel.Instance                      = DFSDM1_Channel5;
    hAudioInButtomLeftChannel.Init.OutputClock.Activation   = ENABLE;
    hAudioInButtomLeftChannel.Init.OutputClock.Selection    = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
    /* Set the DFSDM clock OUT audio frequency configuration */
    hAudioInButtomLeftChannel.Init.OutputClock.Divider      = DFSDM_CLOCK_DIVIDER(AudioFreq);
    hAudioInButtomLeftChannel.Init.Input.Multiplexer        = DFSDM_CHANNEL_EXTERNAL_INPUTS;
    hAudioInButtomLeftChannel.Init.Input.DataPacking        = DFSDM_CHANNEL_STANDARD_MODE;
    hAudioInButtomLeftChannel.Init.Input.Pins               = DFSDM_CHANNEL_SAME_CHANNEL_PINS;
    /* Request to sample stable data for LEFT micro on Rising edge */
    hAudioInButtomLeftChannel.Init.SerialInterface.Type     = DFSDM_CHANNEL_SPI_RISING;
    hAudioInButtomLeftChannel.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
    hAudioInButtomLeftChannel.Init.Awd.FilterOrder          = DFSDM_CHANNEL_FASTSINC_ORDER;
    hAudioInButtomLeftChannel.Init.Awd.Oversampling         = 10;
    hAudioInButtomLeftChannel.Init.Offset                   = 0;
    hAudioInButtomLeftChannel.Init.RightBitShift            = DFSDM_RIGHT_BIT_SHIFT(AudioFreq);
    if(HAL_OK != HAL_DFSDM_ChannelInit(&hAudioInButtomLeftChannel))
    {
      return AUDIO_ERROR;
    }

    /* CHANNEL 4 configuration */
    __HAL_DFSDM_CHANNEL_RESET_HANDLE_STATE(&hAudioInButtomRightChannel);
    hAudioInButtomRightChannel.Instance                      = DFSDM1_Channel4;
    hAudioInButtomRightChannel.Init.OutputClock.Activation   = ENABLE;
    hAudioInButtomRightChannel.Init.OutputClock.Selection    = DFSDM_CHANNEL_OUTPUT_CLOCK_AUDIO;
    /* Set the DFSDM clock OUT audio frequency configuration */
    hAudioInButtomRightChannel.Init.OutputClock.Divider      = DFSDM_CLOCK_DIVIDER(AudioFreq);
    hAudioInButtomRightChannel.Init.Input.Multiplexer        = DFSDM_CHANNEL_EXTERNAL_INPUTS;
    hAudioInButtomRightChannel.Init.Input.DataPacking        = DFSDM_CHANNEL_STANDARD_MODE;
    hAudioInButtomRightChannel.Init.Input.Pins               = DFSDM_CHANNEL_FOLLOWING_CHANNEL_PINS;
    /* Request to sample stable data for RIGHT micro on Falling edge */
    hAudioInButtomRightChannel.Init.SerialInterface.Type     = DFSDM_CHANNEL_SPI_FALLING;
    hAudioInButtomRightChannel.Init.SerialInterface.SpiClock = DFSDM_CHANNEL_SPI_CLOCK_INTERNAL;
    hAudioInButtomRightChannel.Init.Awd.FilterOrder          = DFSDM_CHANNEL_FASTSINC_ORDER;
    hAudioInButtomRightChannel.Init.Awd.Oversampling         = 10;
    hAudioInButtomRightChannel.Init.Offset                   = 0;
    hAudioInButtomRightChannel.Init.RightBitShift            = DFSDM_RIGHT_BIT_SHIFT(AudioFreq);
    if(HAL_OK != HAL_DFSDM_ChannelInit(&hAudioInButtomRightChannel))
    {
      return AUDIO_ERROR;
    }
  }
  /****************************************************************************/
  /********************** Filters configuration  ******************************/
  /****************************************************************************/

  /* FILTER 0 configuration */
  __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(&hAudioInTopLeftFilter);
  hAudioInTopLeftFilter.Instance                          = AUDIO_DFSDMx_TOP_LEFT_FILTER;
  hAudioInTopLeftFilter.Init.RegularParam.Trigger         = DFSDM_FILTER_SW_TRIGGER;
  hAudioInTopLeftFilter.Init.RegularParam.FastMode        = ENABLE;
  hAudioInTopLeftFilter.Init.RegularParam.DmaMode         = ENABLE;
  hAudioInTopLeftFilter.Init.InjectedParam.Trigger        = DFSDM_FILTER_SW_TRIGGER;
  hAudioInTopLeftFilter.Init.InjectedParam.ScanMode       = ENABLE;
  hAudioInTopLeftFilter.Init.InjectedParam.DmaMode        = DISABLE;
  hAudioInTopLeftFilter.Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
  hAudioInTopLeftFilter.Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
  hAudioInTopLeftFilter.Init.FilterParam.SincOrder        = DFSDM_FILTER_ORDER(AudioFreq);
  /* Set the DFSDM Filters Oversampling to have correct sample rate */
  hAudioInTopLeftFilter.Init.FilterParam.Oversampling     = DFSDM_OVER_SAMPLING(AudioFreq);
  hAudioInTopLeftFilter.Init.FilterParam.IntOversampling  = 1;
  if(HAL_OK != HAL_DFSDM_FilterInit(&hAudioInTopLeftFilter))
  {
    return AUDIO_ERROR;
  }

  /* Configure injected channel */
  if(HAL_OK != HAL_DFSDM_FilterConfigRegChannel(&hAudioInTopLeftFilter, AUDIO_DFSDMx_TOP_LEFT_CHANNEL, DFSDM_CONTINUOUS_CONV_ON))
  {
    return AUDIO_ERROR;
  }

  /* FILTER 1 configuration */
  __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(&hAudioInTopRightFilter);
  hAudioInTopRightFilter.Instance                          = AUDIO_DFSDMx_TOP_RIGHT_FILTER;
  hAudioInTopRightFilter.Init.RegularParam.Trigger         = DFSDM_FILTER_SYNC_TRIGGER;
  hAudioInTopRightFilter.Init.RegularParam.FastMode        = ENABLE;
  hAudioInTopRightFilter.Init.RegularParam.DmaMode         = ENABLE;
  hAudioInTopRightFilter.Init.InjectedParam.Trigger        = DFSDM_FILTER_SW_TRIGGER;
  hAudioInTopRightFilter.Init.InjectedParam.ScanMode       = DISABLE;
  hAudioInTopRightFilter.Init.InjectedParam.DmaMode        = DISABLE;
  hAudioInTopRightFilter.Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
  hAudioInTopRightFilter.Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
  hAudioInTopRightFilter.Init.FilterParam.SincOrder        = DFSDM_FILTER_ORDER(AudioFreq);
  /* Set the DFSDM Filters Oversampling to have correct sample rate */
  hAudioInTopRightFilter.Init.FilterParam.Oversampling     = DFSDM_OVER_SAMPLING(AudioFreq);
  hAudioInTopRightFilter.Init.FilterParam.IntOversampling  = 1;
  if(HAL_OK != HAL_DFSDM_FilterInit(&hAudioInTopRightFilter))
  {
    return AUDIO_ERROR;
  }
  /* Configure injected channel */
  if(HAL_OK != HAL_DFSDM_FilterConfigRegChannel(&hAudioInTopRightFilter, AUDIO_DFSDMx_TOP_RIGHT_CHANNEL, DFSDM_CONTINUOUS_CONV_ON))
  {
    return AUDIO_ERROR;
  }

  if(AudioIn_ChannelNumber > 2)
  {
    /* FILTER 2 configuration */
    __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(&hAudioInButtomLeftFilter);
    hAudioInButtomLeftFilter.Instance                          = AUDIO_DFSDMx_BUTTOM_LEFT_FILTER;
    hAudioInButtomLeftFilter.Init.RegularParam.Trigger         = DFSDM_FILTER_SYNC_TRIGGER;
    hAudioInButtomLeftFilter.Init.RegularParam.FastMode        = ENABLE;
    hAudioInButtomLeftFilter.Init.RegularParam.DmaMode         = ENABLE;
    hAudioInButtomLeftFilter.Init.InjectedParam.Trigger        = DFSDM_FILTER_SW_TRIGGER;
    hAudioInButtomLeftFilter.Init.InjectedParam.ScanMode       = ENABLE;
    hAudioInButtomLeftFilter.Init.InjectedParam.DmaMode        = DISABLE;
    hAudioInButtomLeftFilter.Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
    hAudioInButtomLeftFilter.Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
    hAudioInButtomLeftFilter.Init.FilterParam.SincOrder        = DFSDM_FILTER_ORDER(AudioFreq);
    /* Set the DFSDM Filters Oversampling to have correct sample rate */
    hAudioInButtomLeftFilter.Init.FilterParam.Oversampling     = DFSDM_OVER_SAMPLING(AudioFreq);
    hAudioInButtomLeftFilter.Init.FilterParam.IntOversampling  = 1;
    if(HAL_OK != HAL_DFSDM_FilterInit(&hAudioInButtomLeftFilter))
    {
      return AUDIO_ERROR;
    }

    /* Configure injected channel */
    if(HAL_OK != HAL_DFSDM_FilterConfigRegChannel(&hAudioInButtomLeftFilter, AUDIO_DFSDMx_BUTTOM_LEFT_CHANNEL, DFSDM_CONTINUOUS_CONV_ON))
    {
      return AUDIO_ERROR;
    }

    /* FILTER 3 configuration */
    __HAL_DFSDM_FILTER_RESET_HANDLE_STATE(&hAudioInButtomRightFilter);
    hAudioInButtomRightFilter.Instance                          = AUDIO_DFSDMx_BUTTOM_RIGHT_FILTER;
    hAudioInButtomRightFilter.Init.RegularParam.Trigger         = DFSDM_FILTER_SYNC_TRIGGER;
    hAudioInButtomRightFilter.Init.RegularParam.FastMode        = ENABLE;
    hAudioInButtomRightFilter.Init.RegularParam.DmaMode         = ENABLE;
    hAudioInButtomRightFilter.Init.InjectedParam.Trigger        = DFSDM_FILTER_SW_TRIGGER;
    hAudioInButtomRightFilter.Init.InjectedParam.ScanMode       = DISABLE;
    hAudioInButtomRightFilter.Init.InjectedParam.DmaMode        = DISABLE;
    hAudioInButtomRightFilter.Init.InjectedParam.ExtTrigger     = DFSDM_FILTER_EXT_TRIG_TIM1_TRGO;
    hAudioInButtomRightFilter.Init.InjectedParam.ExtTriggerEdge = DFSDM_FILTER_EXT_TRIG_RISING_EDGE;
    hAudioInButtomRightFilter.Init.FilterParam.SincOrder        = DFSDM_FILTER_ORDER(AudioFreq);
    /* Set the DFSDM Filters Oversampling to have correct sample rate */
    hAudioInButtomRightFilter.Init.FilterParam.Oversampling     = DFSDM_OVER_SAMPLING(AudioFreq);
    hAudioInButtomRightFilter.Init.FilterParam.IntOversampling  = 1;
    if(HAL_OK != HAL_DFSDM_FilterInit(&hAudioInButtomRightFilter))
    {
      return AUDIO_ERROR;
    }
    /* Configure injected channel */
    if(HAL_OK != HAL_DFSDM_FilterConfigRegChannel(&hAudioInButtomRightFilter, AUDIO_DFSDMx_BUTTOM_RIGHT_CHANNEL, DFSDM_CONTINUOUS_CONV_ON))
    {
      return AUDIO_ERROR;
    }
  }
  return AUDIO_OK;
}


/**
  * @brief  De-initialize the Digital Filter for Sigma-Delta Modulators interface (DFSDM).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
static uint8_t DFSDMx_DeInit(void)
{
  /* De-initializes the DFSDM filters to allow access to DFSDM internal registers */
  if(HAL_OK != HAL_DFSDM_FilterDeInit(&hAudioInTopLeftFilter))
  {
    return AUDIO_ERROR;
  }

  if(HAL_OK != HAL_DFSDM_FilterDeInit(&hAudioInTopRightFilter))
  {
    return AUDIO_ERROR;
  }

  /* De-initializes the DFSDM channels to allow access to DFSDM internal registers */
  if(HAL_OK != HAL_DFSDM_ChannelDeInit(&hAudioInTopLeftChannel))
  {
    return AUDIO_ERROR;
  }

  if(HAL_OK != HAL_DFSDM_ChannelDeInit(&hAudioInTopRightChannel))
  {
    return AUDIO_ERROR;
  }

  if(AudioIn_ChannelNumber > 2)
  {
    /* De-initializes the DFSDM filters to allow access to DFSDM internal registers */
    if(HAL_OK != HAL_DFSDM_FilterDeInit(&hAudioInButtomLeftFilter))
    {
      return AUDIO_ERROR;
    }

    if(HAL_OK != HAL_DFSDM_FilterDeInit(&hAudioInButtomRightFilter))
    {
      return AUDIO_ERROR;
    }

    /* De-initializes the DFSDM channels to allow access to DFSDM internal registers */
    if(HAL_OK != HAL_DFSDM_ChannelDeInit(&hAudioInButtomLeftChannel))
    {
      return AUDIO_ERROR;
    }

    if(HAL_OK != HAL_DFSDM_ChannelDeInit(&hAudioInButtomRightChannel))
    {
      return AUDIO_ERROR;
    }
  }

  return AUDIO_OK;
}



/**
  * @brief  Initialize the DFSDM channel MSP.
  * @retval None
  */
static void DFSDMx_ChannelMspInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Enable DFSDM clock */
  AUDIO_DFSDMx_CLK_ENABLE();

  /* Enable GPIO clock */
  AUDIO_DFSDMx_DMIC_DATIN_GPIO_CLK_ENABLE();
  AUDIO_DFSDMx_CKOUT_DMIC_GPIO_CLK_ENABLE();

  /* DFSDM pins configuration: DFSDM_CKOUT, DMIC_DATIN1 pins ------------------*/
  GPIO_InitStruct.Pin = AUDIO_DFSDMx_CKOUT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = AUDIO_DFSDMx_CKOUT_AF;
  HAL_GPIO_Init(AUDIO_DFSDMx_CKOUT_DMIC_GPIO_PORT, &GPIO_InitStruct);

  /* DFSDM pin configuration: DMIC_DATIN1 pin --------------------------------*/
  GPIO_InitStruct.Pin = AUDIO_DFSDMx_DMIC_DATIN1_PIN;
  GPIO_InitStruct.Alternate = AUDIO_DFSDMx_DMIC_DATIN_AF;
  HAL_GPIO_Init(AUDIO_DFSDMx_DMIC_DATIN_GPIO_PORT, &GPIO_InitStruct);

  if(AudioIn_ChannelNumber > 2)
  {
  /* DFSDM pin configuration: DMIC_DATIN5 pin --------------------------------*/
    GPIO_InitStruct.Pin = AUDIO_DFSDMx_DMIC_DATIN5_PIN;
    GPIO_InitStruct.Alternate = AUDIO_DFSDMx_DMIC_DATIN_AF;
    HAL_GPIO_Init(AUDIO_DFSDMx_DMIC_DATIN_GPIO_PORT, &GPIO_InitStruct);
  }
}


/**
  * @brief  DeInitialize the DFSDM channel MSP.
  * @retval None
  */
static void DFSDMx_ChannelMspDeInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* DFSDM pin configuration: DMIC_DATIN1 pin --------------------------------*/
  GPIO_InitStruct.Pin = AUDIO_DFSDMx_CKOUT_PIN;
  HAL_GPIO_DeInit(AUDIO_DFSDMx_CKOUT_DMIC_GPIO_PORT, GPIO_InitStruct.Pin);
  GPIO_InitStruct.Pin = AUDIO_DFSDMx_DMIC_DATIN1_PIN;
  HAL_GPIO_DeInit(AUDIO_DFSDMx_DMIC_DATIN_GPIO_PORT, GPIO_InitStruct.Pin);

  if(AudioIn_ChannelNumber > 2)
  {
    /* DFSDM pin configuration: DMIC_DATIN5 pin ------------------------------*/
    GPIO_InitStruct.Pin = AUDIO_DFSDMx_CKOUT_PIN;
    HAL_GPIO_DeInit(AUDIO_DFSDMx_CKOUT_DMIC_GPIO_PORT, GPIO_InitStruct.Pin);
    GPIO_InitStruct.Pin = AUDIO_DFSDMx_DMIC_DATIN5_PIN;
    HAL_GPIO_DeInit(AUDIO_DFSDMx_DMIC_DATIN_GPIO_PORT, GPIO_InitStruct.Pin);
  }
}


/**
  * @brief  Initialize the DFSDM filter MSP.
  * @retval None
  */
static void DFSDMx_FilterMspInit(void)
{
  /* Enable DFSDM clock */
  AUDIO_DFSDMx_CLK_ENABLE();

  /* Enable the DMA clock */
  AUDIO_DFSDMx_DMAx_CLK_ENABLE();

  /*********** Configure DMA stream for TOP LEFT microphone *******************/
  hDmaTopLeft.Init.Request             = DMA_REQUEST_DFSDM1_FLT0;
  hDmaTopLeft.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hDmaTopLeft.Init.PeriphInc           = DMA_PINC_DISABLE;
  hDmaTopLeft.Init.MemInc              = DMA_MINC_ENABLE;
  hDmaTopLeft.Init.PeriphDataAlignment = AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE;
  hDmaTopLeft.Init.MemDataAlignment    = AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE;
  hDmaTopLeft.Init.Mode                = DMA_CIRCULAR;
  hDmaTopLeft.Init.Priority            = DMA_PRIORITY_HIGH;
  hDmaTopLeft.Instance                 = AUDIO_DFSDMx_DMAx_TOP_LEFT_STREAM;


  /* Associate the DMA handle */
  __HAL_LINKDMA(&hAudioInTopLeftFilter, hdmaReg, hDmaTopLeft);

  /* Reset DMA handle state */
  __HAL_DMA_RESET_HANDLE_STATE(&hDmaTopLeft);

  /* Configure the DMA Channel */
  HAL_DMA_Init(&hDmaTopLeft);

  /* DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_DFSDMx_DMAx_TOP_LEFT_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
  HAL_NVIC_EnableIRQ(AUDIO_DFSDMx_DMAx_TOP_LEFT_IRQ);


  /*********** Configure DMA stream for TOP RIGHT microphone ******************/
  hDmaTopRight.Init.Request             = DMA_REQUEST_DFSDM1_FLT1;
  hDmaTopRight.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hDmaTopRight.Init.PeriphInc           = DMA_PINC_DISABLE;
  hDmaTopRight.Init.MemInc              = DMA_MINC_ENABLE;
  hDmaTopRight.Init.PeriphDataAlignment = AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE;
  hDmaTopRight.Init.MemDataAlignment    = AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE;
  hDmaTopRight.Init.Mode                = DMA_CIRCULAR;
  hDmaTopRight.Init.Priority            = DMA_PRIORITY_HIGH;
  hDmaTopRight.Instance                 = AUDIO_DFSDMx_DMAx_TOP_RIGHT_STREAM;

  /* Associate the DMA handle */
  __HAL_LINKDMA(&hAudioInTopRightFilter, hdmaReg, hDmaTopRight);

  /* Reset DMA handle state */
  __HAL_DMA_RESET_HANDLE_STATE(&hDmaTopRight);

  /* Configure the DMA Channel */
  HAL_DMA_Init(&hDmaTopRight);

  /* DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(AUDIO_DFSDMx_DMAx_TOP_RIGHT_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
  HAL_NVIC_EnableIRQ(AUDIO_DFSDMx_DMAx_TOP_RIGHT_IRQ);

  if(AudioIn_ChannelNumber > 2)
  {
    /*********** Configure DMA stream for BUTTOM LEFT microphone ****************/
	hDmaButtomLeft.Init.Request             = DMA_REQUEST_DFSDM1_FLT2;
    hDmaButtomLeft.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hDmaButtomLeft.Init.PeriphInc           = DMA_PINC_DISABLE;
    hDmaButtomLeft.Init.MemInc              = DMA_MINC_ENABLE;
    hDmaButtomLeft.Init.PeriphDataAlignment = AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE;
    hDmaButtomLeft.Init.MemDataAlignment    = AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE;
    hDmaButtomLeft.Init.Mode                = DMA_CIRCULAR;
    hDmaButtomLeft.Init.Priority            = DMA_PRIORITY_HIGH;
    hDmaButtomLeft.Instance                 = AUDIO_DFSDMx_DMAx_BUTTOM_LEFT_STREAM;

    /* Associate the DMA handle */
    __HAL_LINKDMA(&hAudioInButtomLeftFilter, hdmaReg, hDmaButtomLeft);

    /* Reset DMA handle state */
    __HAL_DMA_RESET_HANDLE_STATE(&hDmaButtomLeft);

    /* Configure the DMA Channel */
    HAL_DMA_Init(&hDmaButtomLeft);

    /* DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_DFSDMx_DMAx_BUTTOM_LEFT_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
    HAL_NVIC_EnableIRQ(AUDIO_DFSDMx_DMAx_BUTTOM_LEFT_IRQ);


    /*********** Configure DMA stream for BUTTOM RIGHT microphone ***************/
    hDmaButtomRight.Init.Request             = DMA_REQUEST_DFSDM1_FLT3;
    hDmaButtomRight.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hDmaButtomRight.Init.PeriphInc           = DMA_PINC_DISABLE;
    hDmaButtomRight.Init.MemInc              = DMA_MINC_ENABLE;
    hDmaButtomRight.Init.PeriphDataAlignment = AUDIO_DFSDMx_DMAx_PERIPH_DATA_SIZE;
    hDmaButtomRight.Init.MemDataAlignment    = AUDIO_DFSDMx_DMAx_MEM_DATA_SIZE;
    hDmaButtomRight.Init.Mode                = DMA_CIRCULAR;
    hDmaButtomRight.Init.Priority            = DMA_PRIORITY_HIGH;
    hDmaButtomRight.Instance                 = AUDIO_DFSDMx_DMAx_BUTTOM_RIGHT_STREAM;

    /* Associate the DMA handle */
    __HAL_LINKDMA(&hAudioInButtomRightFilter, hdmaReg, hDmaButtomRight);

    /* Reset DMA handle state */
    __HAL_DMA_RESET_HANDLE_STATE(&hDmaButtomRight);

    /* Configure the DMA Channel */
    HAL_DMA_Init(&hDmaButtomRight);

    /* DMA IRQ Channel configuration */
    HAL_NVIC_SetPriority(AUDIO_DFSDMx_DMAx_BUTTOM_RIGHT_IRQ, AUDIO_IN_IRQ_PREPRIO, 0);
    HAL_NVIC_EnableIRQ(AUDIO_DFSDMx_DMAx_BUTTOM_RIGHT_IRQ);
  }
}


/**
  * @brief  DeInitialize the DFSDM filter MSP.
  * @retval None
  */
static void DFSDMx_FilterMspDeInit(void)
{
  /* Configure the DMA Channel */
  HAL_DMA_DeInit(&hDmaTopLeft);
  HAL_DMA_DeInit(&hDmaTopRight);
  if(AudioIn_ChannelNumber > 2)
  {
    HAL_DMA_DeInit(&hDmaButtomLeft);
    HAL_DMA_DeInit(&hDmaButtomRight);
  }
}
