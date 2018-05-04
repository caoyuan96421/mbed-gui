#include <SDIOBlockDevice.h>
#include "mbed.h"

static const int SD_TIMEOUT = 500;

SDIOBlockDevice::SDIOBlockDevice()
{
}

SDIOBlockDevice::~SDIOBlockDevice()
{
}

extern SD_HandleTypeDef uSdHandle;

extern "C" void BSP_SD_IRQHandler()
{
	HAL_SD_IRQHandler(&uSdHandle);
}

int SDIOBlockDevice::init()
{
	lock();
	if (BSP_SD_Init() != MSD_OK)
	{
		unlock();
		return BD_ERROR_DEVICE_ERROR;
	}
	unlock();
	return BD_ERROR_OK;
}

int SDIOBlockDevice::deinit()
{
	lock();
	BSP_SD_DeInit();
	unlock();
	return 0;
}

int SDIOBlockDevice::program(const void *b, bd_addr_t addr, bd_size_t size)
{
	if (!is_valid_program(addr, size))
	{
		return -EINVAL;
	}

	lock();

	uint32_t *buffer = const_cast<uint32_t *>((const uint32_t *) b);
	if (BSP_SD_WriteBlocks(buffer, addr / get_program_size(),
			size / get_program_size(), SD_TIMEOUT) != MSD_OK)
	{
		unlock();
		return -ENODEV;
	}

	while (BSP_SD_GetCardState() != MSD_OK)
		;

	unlock();
	return 0;
}

int SDIOBlockDevice::read(void *b, bd_addr_t addr, bd_size_t size)
{
	if (!is_valid_read(addr, size))
	{
		return -EINVAL;
	}

	lock();

	uint32_t *buffer = static_cast<uint32_t *>(b);
	if (BSP_SD_ReadBlocks(buffer, addr / get_program_size(),
			size / get_program_size(), SD_TIMEOUT) != MSD_OK)
	{
		unlock();
		return -ENODEV;
	}
	while (BSP_SD_GetCardState() != MSD_OK)
		;

	unlock();
	return 0;
}

bd_size_t SDIOBlockDevice::get_read_size() const
{
	BSP_SD_CardInfo CardInfo;
	BSP_SD_GetCardInfo(&CardInfo);
	return CardInfo.LogBlockSize;
}

bd_size_t SDIOBlockDevice::get_program_size() const
{

	BSP_SD_CardInfo CardInfo;
	BSP_SD_GetCardInfo(&CardInfo);
	return CardInfo.LogBlockSize;
}

bd_size_t SDIOBlockDevice::size() const
{

	BSP_SD_CardInfo CardInfo;
	BSP_SD_GetCardInfo(&CardInfo);
	return CardInfo.LogBlockSize * CardInfo.LogBlockNbr;
}


