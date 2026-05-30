#include <stdint.h>


// Ports used to communicate with the disk
#define ATA_DATA        0x1F0   // read/write data here
#define ATA_SECTOR_COUNT 0x1F2  // tell disk how many sectors to read
#define ATA_LBA_LOW     0x1F3   // sector number (bits 0-7)
#define ATA_LBA_MID     0x1F4   // sector number (bits 8-15)
#define ATA_LBA_HIGH    0x1F5   // sector number (bits 16-23)
#define ATA_DRIVE       0x1F6   // select master/slave + LBA bits 24-27
#define ATA_STATUS      0x1F7   // read status of disk
#define ATA_COMMAND     0x1F7   // send commands to disk (same port)

// -----------------------------------
// Status flags (bits in ATA_STATUS)
// -----------------------------------
#define ATA_STATUS_BSY  0x80    // disk is busy
#define ATA_STATUS_DRQ  0x08    // disk is ready to transfer data

// -----------------------------------
// The two commands needed for now
// -----------------------------------
#define ATA_CMD_READ    0x20    // read sectors
#define ATA_CMD_WRITE   0x30    // write sectors



// LBA: the nth number of sector (typically 512 bytes each)
// count: How many sectors to read/write all at once
// buffer: chunk of memory you give the driver to dump the data into
void ata_read_sectors(uint32_t lba, uint8_t count, uint8_t *buffer);
void ata_write_sectors(uint32_t lba, uint8_t count, uint8_t *buffer);
