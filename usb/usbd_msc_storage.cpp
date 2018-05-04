/**
 #include <usbd_msc_storage.h>
 ******************************************************************************
 * @file    usbd_msc_storage_template.c
 * @author  MCD Application Team
 * @version V2.4.2
 * @date    11-December-2015
 * @brief   Memory management layer
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "SDIOBlockDevice.h"
#include "usbd_msc_storage.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#define STORAGE_LUN_NBR                  1  
static const int SD_TIMEOUT = 500;

int8_t STORAGE_Init(uint8_t lun);

int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num,
		uint16_t *block_size);

int8_t STORAGE_IsReady(uint8_t lun);

int8_t STORAGE_IsWriteProtected(uint8_t lun);

int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr,
		uint16_t blk_len);

int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr,
		uint16_t blk_len);

int8_t STORAGE_GetMaxLun(void);

/* USB Mass storage Standard Inquiry Data */
uint8_t STORAGE_Inquirydata[] =
{ //36

		/* LUN 0 */
		0x00, 0x80, 0x02, 0x02, (STANDARD_INQUIRY_DATA_LEN - 5), 0x00, 0x00,
				0x00, 'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
				'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
				' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '0', '.', '0', '1', /* Version      : 4 Bytes */
		};

USBD_StorageTypeDef USBD_MSC_fops =
{ STORAGE_Init, STORAGE_GetCapacity, STORAGE_IsReady, STORAGE_IsWriteProtected,
		STORAGE_Read, STORAGE_Write, STORAGE_GetMaxLun,
		(int8_t*) STORAGE_Inquirydata,

};
/*******************************************************************************
 * Function Name  : Read_Memory
 * Description    : Handle the Read operation from the microSD card.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
int8_t STORAGE_Init(uint8_t lun)
{
	return (0);
}

/*******************************************************************************
 * Function Name  : Read_Memory
 * Description    : Handle the Read operation from the STORAGE card.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num,
		uint16_t *block_size)
{
	*block_num = SDIOBlockDevice::getInstance().size()
			/ SDIOBlockDevice::getInstance().get_program_size();
	*block_size = SDIOBlockDevice::getInstance().get_program_size();
	return (0);
}

/*******************************************************************************
 * Function Name  : Read_Memory
 * Description    : Handle the Read operation from the STORAGE card.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
int8_t STORAGE_IsReady(uint8_t lun)
{
	return (0);
}

/*******************************************************************************
 * Function Name  : Read_Memory
 * Description    : Handle the Read operation from the STORAGE card.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
	return 0;
}

/*******************************************************************************
 * Function Name  : Read_Memory
 * Description    : Handle the Read operation from the STORAGE card.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr,
		uint16_t blk_len)
{
	if (BSP_SD_ReadBlocks((uint32_t*) buf, blk_addr, blk_len,
			SD_TIMEOUT) != MSD_OK)
	{
		return -ENODEV;
	}
	while (BSP_SD_GetCardState() != MSD_OK)
		;
	return 0;
}
/*******************************************************************************
 * Function Name  : Write_Memory
 * Description    : Handle the Write operation to the STORAGE card.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr,
		uint16_t blk_len)
{
	if (BSP_SD_WriteBlocks((uint32_t*) buf, blk_addr, blk_len,
			SD_TIMEOUT) != MSD_OK)
	{
		return -ENODEV;
	}
	while (BSP_SD_GetCardState() != MSD_OK)
		;
	return 0;
}
/*******************************************************************************
 * Function Name  : Write_Memory
 * Description    : Handle the Write operation to the STORAGE card.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
int8_t STORAGE_GetMaxLun(void)
{
	return (STORAGE_LUN_NBR - 1);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

