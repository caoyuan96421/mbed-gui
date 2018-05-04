#ifndef MBED_SDIO_BLOCK_DEVICE_H
#define MBED_SDIO_BLOCK_DEVICE_H

#include "BlockDevice.h"
#include "mbed.h"
#include "stm32469i_discovery_sd.h"
#include "platform/PlatformMutex.h"

class SDIOBlockDevice: public BlockDevice
{
public:
	/** Initialize a block device
	 *
	 *  @return         0 on success or a negative error code on failure
	 */
	virtual int init();

	/** Deinitialize a block device
	 *
	 *  @return         0 on success or a negative error code on failure
	 */
	virtual int deinit();

	/** Read blocks from a block device
	 *
	 *  @param buffer   Buffer to write blocks to
	 *  @param addr     Address of block to begin reading from
	 *  @param size     Size to read in bytes, must be a multiple of read block size
	 *  @return         0 on success, negative error code on failure
	 */
	virtual int read(void *buffer, bd_addr_t addr, bd_size_t size);

	/** Program blocks to a block device
	 *
	 *  The blocks must have been erased prior to being programmed
	 *
	 *  @param buffer   Buffer of data to write to blocks
	 *  @param addr     Address of block to begin writing to
	 *  @param size     Size to write in bytes, must be a multiple of program block size
	 *  @return         0 on success, negative error code on failure
	 */
	virtual int program(const void *buffer, bd_addr_t addr, bd_size_t size);

	/** Get the size of a readable block
	 *
	 *  @return         Size of a readable block in bytes
	 */
	virtual bd_size_t get_read_size() const;

	/** Get the size of a programable block
	 *
	 *  @return         Size of a programable block in bytes
	 *  @note Must be a multiple of the read size
	 */
	virtual bd_size_t get_program_size() const;

	/** Get the total size of the underlying device
	 *
	 *  @return         Size of the underlying device in bytes
	 */
	virtual bd_size_t size() const;

	static SDIOBlockDevice &getInstance()
	{
		static SDIOBlockDevice inst;
		return inst;
	}

private:
	Mutex mutex;

	SDIOBlockDevice();
	virtual ~SDIOBlockDevice();

	virtual void lock()
	{
		mutex.lock();
	}

	virtual void unlock()
	{
		mutex.unlock();
	}
};

#endif  /* MBED_SD_BLOCK_DEVICE_H */
