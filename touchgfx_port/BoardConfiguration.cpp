/*
 * touchgfxInit.cpp
 *
 *  Created on: 2018Äê2ÔÂ3ÈÕ
 *      Author: caoyuan9642
 */

#include "mbed.h"
#include "rtos.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dsi.h"
#include "stm32469i_discovery_sdram.h"
#include "stm32469i_discovery_qspi.h"
#include "stm32469i_discovery.h"
#include "stm32469i_discovery_lcd.h"

#include <common/TouchGFXInit.hpp>
#include <touchgfx/hal/BoardConfiguration.hpp>
#include <touchgfx/hal/GPIO.hpp>
#include <platform/driver/lcd/LCD16bpp.hpp>
#include <platform/driver/lcd/LCD24bpp.hpp>
#include <touchgfx_port/CortexMMCUInstrumentation.hpp>
#include <touchgfx_port/OTM8009TouchController.hpp>
#include <touchgfx_port/STM32F4DMA.hpp>
#include <touchgfx_port/STM32F4HAL_DSI.hpp>

extern LTDC_HandleTypeDef hltdc_eval;
extern DSI_HandleTypeDef hdsi_eval;
DSI_CmdCfgTypeDef CmdCfg;
DSI_LPCmdTypeDef LPCmd;
DSI_PLLInitTypeDef dsiPllInit;
static RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

/* Private define ------------------------------------------------------------*/
#define VSYNC           1
#define VBP             1
#define VFP             1
#define VACT            800
#define HSYNC           1
#define HBP             1
#define HFP             1
#define USE_DOUBLE_BUFFERING

#define HACT            480

LCD24bpp display;
const uint16_t bitDepth = 24;

#ifdef USE_DOUBLE_BUFFERING
	#define FBSIZE (800*480*3) * 3
#else
	#define FBSIZE (800*480*3) * 2
#endif

#define CANVAS_BUFFER_SIZE 262144

// Allocate the frame buffer
__attribute__((section (".sdram")))
              uint8_t frameBuf0[FBSIZE];

// Allocate buffer for Canvas widgets
__attribute__((section (".sdram")))
              uint8_t canvasBuffer[CANVAS_BUFFER_SIZE];

uint8_t pCol[] =
{ 0x00, 0x00, 0x01, 0xDF }; /* 0->479 */

uint8_t pPage[] =
{ 0x00, 0x00, 0x03, 0x1F }; /*   0 -> 799 */

static void LTDC_Init();
static uint8_t LCD_Init(void);
static void LCD_LayerInit(uint16_t LayerIndex, uint32_t *Address);

/**
 * Request TE at column 570.
 */
void LCD_ReqTear(void)
{
	static uint8_t ScanLineParams[2];

	static const uint16_t scanline = 570;
	ScanLineParams[0] = scanline >> 8;
	ScanLineParams[1] = scanline & 0x00FF;

	HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 2, 0x44, ScanLineParams);
	// set_tear_on
	HAL_DSI_ShortWrite(&hdsi_eval, 0, DSI_DCS_SHORT_PKT_WRITE_P1, 0x35, 0x00);
}

namespace touchgfx
{

STM32F4DMA dma;
OTM8009TouchController tc;
CortexMMCUInstrumentation mcuInstr;

void hw_init()
{

	LCD_Init();

	/* Initialize LTDC layer 0 iused for Hint */
	LCD_LayerInit(0, (uint32_t*) frameBuf0);
	BSP_LCD_SelectLayer(0);

	HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 4,
	OTM8009A_CMD_CASET, pCol);

	HAL_DSI_LongWrite(&hdsi_eval, 0, DSI_DCS_LONG_PKT_WRITE, 4,
	OTM8009A_CMD_PASET, pPage);

	HAL_DSI_ShortWrite(&hdsi_eval, 0, DSI_DCS_SHORT_PKT_WRITE_P0,
	OTM8009A_CMD_DISPOFF, 0);

	GPIO::init();

	tc.init();
}

static unsigned int xsize()
{
	return BSP_LCD_GetYSize();
}

static unsigned int ysize()
{
	return BSP_LCD_GetXSize();
}

//KeySampler btnctrl;

HAL *hal;

void touchgfx_init()
{
	uint16_t dispWidth = xsize();
	uint16_t dispHeight = ysize();
	hal = &touchgfx_generic_init<STM32F4HAL_DSI>(dma, display, tc, dispWidth, dispHeight, 0, 0);

#ifdef USE_DOUBLE_BUFFERING
	hal->setFrameBufferStartAddress((uint16_t*) frameBuf0, bitDepth, true, true);
#else
	hal->setFrameBufferStartAddress((uint16_t*) frameBuf0, bitDepth, false, true);
#endif

	// By default frame rate compensation is off.
	// Enable frame rate compensation to smooth out animations in case there is periodic slow frame rates.
	hal->setFrameRateCompensation(false);
//    hal.setButtonController(&btnctrl);
	hal->setTouchSampleRate(1);
	hal->setFingerSize(35);

	// This platform can handle simultaneous DMA and TFT accesses to SDRAM, so disable lock to increase performance.
	hal->lockDMAToFrontPorch(false);

	mcuInstr.init();

	//Set MCU instrumentation and Load calculation
	hal->setMCUInstrumentation(&mcuInstr);
	hal->enableMCULoadCalculation(true);

	hal->enableInterrupts(); // Turn on the interrupts

	// Initialize Canvas buffer
	CanvasWidgetRenderer::setupBuffer(canvasBuffer, CANVAS_BUFFER_SIZE);

}
}
/**
 * @brief  Initializes the DSI LCD.
 * The ititialization is done as below:
 *     - DSI PLL ititialization
 *     - DSI ititialization
 *     - LTDC ititialization
 *     - OTM8009A LCD Display IC Driver ititialization
 * @param  None
 * @retval LCD state
 */
static uint8_t LCD_Init(void)
{

	GPIO_InitTypeDef GPIO_Init_Structure;

	/* Toggle Hardware Reset of the DSI LCD using
	 * its XRES signal (active low) */
	BSP_LCD_Reset();

	/* Call first MSP Initialize only in case of first initialization
	 * This will set IP blocks LTDC, DSI and DMA2D
	 * - out of reset
	 * - clocked
	 * - NVIC IRQ related to IP blocks enabled
	 */
	BSP_LCD_MspInit();

	/*Disable interrupts for now*/
	NVIC_DisableIRQ(DMA2D_IRQn);
	NVIC_DisableIRQ(DSI_IRQn);

	/* LCD clock configuration */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC | RCC_PERIPHCLK_CLK48;
	/* use 384/5/2 = 38.4MHz */
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
	PeriphClkInitStruct.PLLSAI.PLLSAIP = 8;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 3;
	PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLSAIP;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

	/* Base address of DSI Host/Wrapper registers to be set before calling De-Init */
	hdsi_eval.Instance = DSI;

	HAL_DSI_DeInit(&(hdsi_eval));

	/* Disco board has 8Mhz crystal. */
	dsiPllInit.PLLNDIV = 125;
	dsiPllInit.PLLIDF = DSI_PLL_IN_DIV2;
	dsiPllInit.PLLODF = DSI_PLL_OUT_DIV1;

	hdsi_eval.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
	hdsi_eval.Init.TXEscapeCkdiv = 0x4;
	HAL_DSI_Init(&(hdsi_eval), &(dsiPllInit));

	/* Configure the DSI for Command mode */
	CmdCfg.VirtualChannelID = 0;
	CmdCfg.HSPolarity = DSI_HSYNC_ACTIVE_HIGH;
	CmdCfg.VSPolarity = DSI_VSYNC_ACTIVE_HIGH;
	CmdCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
	CmdCfg.CommandSize = HACT;
	CmdCfg.TearingEffectSource = DSI_TE_EXTERNAL;
	CmdCfg.TearingEffectPolarity = DSI_TE_RISING_EDGE;
	CmdCfg.VSyncPol = DSI_VSYNC_FALLING;
	CmdCfg.AutomaticRefresh = DSI_AR_DISABLE;
	CmdCfg.TEAcknowledgeRequest = DSI_TE_ACKNOWLEDGE_ENABLE;

	CmdCfg.ColorCoding = DSI_RGB888;

	HAL_DSI_ConfigAdaptedCommandMode(&hdsi_eval, &CmdCfg);

	LPCmd.LPGenShortWriteNoP = DSI_LP_GSW0P_ENABLE;
	LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_ENABLE;
	LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_ENABLE;
	LPCmd.LPGenShortReadNoP = DSI_LP_GSR0P_ENABLE;
	LPCmd.LPGenShortReadOneP = DSI_LP_GSR1P_ENABLE;
	LPCmd.LPGenShortReadTwoP = DSI_LP_GSR2P_ENABLE;
	LPCmd.LPGenLongWrite = DSI_LP_GLW_ENABLE;
	LPCmd.LPDcsShortWriteNoP = DSI_LP_DSW0P_ENABLE;
	LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_ENABLE;
	LPCmd.LPDcsShortReadNoP = DSI_LP_DSR0P_ENABLE;
	LPCmd.LPDcsLongWrite = DSI_LP_DLW_ENABLE;
	HAL_DSI_ConfigCommand(&hdsi_eval, &LPCmd);

	/* Initialize LTDC */
	LTDC_Init();

	/* Start DSI */
	HAL_DSI_Start(&(hdsi_eval));

	/* Initialize the OTM8009A LCD Display IC Driver (KoD LCD IC Driver)
	 *  depending on configuration set in 'hdsivideo_handle'.
	 */
	OTM8009A_Init(OTM8009A_FORMAT_RGB888, LCD_ORIENTATION_PORTRAIT);

	LPCmd.LPGenShortWriteNoP = DSI_LP_GSW0P_DISABLE;
	LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_DISABLE;
	LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_DISABLE;
	LPCmd.LPGenShortReadNoP = DSI_LP_GSR0P_DISABLE;
	LPCmd.LPGenShortReadOneP = DSI_LP_GSR1P_DISABLE;
	LPCmd.LPGenShortReadTwoP = DSI_LP_GSR2P_DISABLE;
	LPCmd.LPGenLongWrite = DSI_LP_GLW_DISABLE;
	LPCmd.LPDcsShortWriteNoP = DSI_LP_DSW0P_DISABLE;
	LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_DISABLE;
	LPCmd.LPDcsShortReadNoP = DSI_LP_DSR0P_DISABLE;
	LPCmd.LPDcsLongWrite = DSI_LP_DLW_DISABLE;
	HAL_DSI_ConfigCommand(&hdsi_eval, &LPCmd);

	HAL_DSI_ConfigFlowControl(&hdsi_eval, DSI_FLOW_CONTROL_BTA);

	/* Enable GPIOJ clock */
	__HAL_RCC_GPIOJ_CLK_ENABLE()
	;

	/* Configure DSI_TE pin from MB1166 : Tearing effect on separated GPIO from KoD LCD */
	/* that is mapped on GPIOJ2 as alternate DSI function (DSI_TE)                      */
	/* This pin is used only when the LCD and DSI link is configured in command mode    */
	/* Not used in DSI Video mode.                                                      */

	GPIO_Init_Structure.Pin = GPIO_PIN_2;
	GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Pull = GPIO_NOPULL;
	GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
	GPIO_Init_Structure.Alternate = GPIO_AF13_DSI;
	HAL_GPIO_Init(GPIOJ, &GPIO_Init_Structure);

	/* Refresh the display */
	HAL_DSI_Refresh(&hdsi_eval);

	return LCD_OK;
}

/**
 * @brief  Initialize the LTDC
 * @param  None
 * @retval None
 */
static void LTDC_Init(void)
{
	/* DeInit */
	HAL_LTDC_DeInit(&hltdc_eval);

	/* LTDC Config */
	/* Timing and polarity */
	hltdc_eval.Init.HorizontalSync = HSYNC;
	hltdc_eval.Init.VerticalSync = VSYNC;
	hltdc_eval.Init.AccumulatedHBP = HSYNC + HBP;
	hltdc_eval.Init.AccumulatedVBP = VSYNC + VBP;
	hltdc_eval.Init.AccumulatedActiveH = VSYNC + VBP + VACT;
	hltdc_eval.Init.AccumulatedActiveW = HSYNC + HBP + HACT;
	hltdc_eval.Init.TotalHeigh = VSYNC + VBP + VACT + VFP;
	hltdc_eval.Init.TotalWidth = HSYNC + HBP + HACT + HFP;

	/* background value */
	hltdc_eval.Init.Backcolor.Blue = 0;
	hltdc_eval.Init.Backcolor.Green = 0;
	hltdc_eval.Init.Backcolor.Red = 0;

	/* Polarity */
	hltdc_eval.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc_eval.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc_eval.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc_eval.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	hltdc_eval.Instance = LTDC;

	HAL_LTDC_Init(&hltdc_eval);
}

/**
 * @brief  Initializes the LCD layers.
 * @param  LayerIndex: Layer foreground or background
 * @param  FB_Address: Layer frame buffer
 * @retval None
 */
static void LCD_LayerInit(uint16_t LayerIndex, uint32_t *Address)
{
	LCD_LayerCfgTypeDef Layercfg;

	/* Layer Init */
	Layercfg.WindowX0 = 0;
	Layercfg.WindowY0 = 0;
	Layercfg.WindowX1 = xsize();
	Layercfg.WindowY1 = ysize();
	Layercfg.FBStartAdress = (uint32_t) Address;
	Layercfg.Alpha = 255;
	Layercfg.Alpha0 = 0;
	Layercfg.Backcolor.Blue = 0;
	Layercfg.Backcolor.Green = 0;
	Layercfg.Backcolor.Red = 0;
	Layercfg.ImageHeight = ysize();
	Layercfg.ImageWidth = xsize();

	Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;
	Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;

	HAL_LTDC_ConfigLayer(&hltdc_eval, &Layercfg, LayerIndex);
}
