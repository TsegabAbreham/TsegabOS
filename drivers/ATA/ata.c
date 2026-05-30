#include "ata.h"
#include "../../drivers/Serial/libk/kprintf/kprintf.h"


// inb reads 8-bits while inw reads 16-bits
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile ("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static void ata_wait() {
    while (1) {
        uint8_t status = inb(ATA_STATUS);

        // BSY = BUSY and DRQ = DATA READY
        // Check if it isn't busy and is ready
        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ)) {
            break; // Exits the loops after disk is ready is detected
        }
    }
}

void ata_read_sectors(uint32_t lba, uint8_t count, uint8_t *buffer) {
    // Inform disk that we are using LBA mode and select master drive
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    // Tell the disk how many sectors to read
    outb(ATA_SECTOR_COUNT, count);

    // Send the LBA address in 3 parts since the outb port only allows to send 8-bit each time
    // and LBA is 28-bits
    outb(ATA_LBA_LOW,  (uint8_t)(lba));          // bits 0-7
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));     // bits 8-15
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));    // bits 16-23

    // Send the READ command
    outb(ATA_COMMAND, ATA_CMD_READ);

    // Read the data sector by sector
    for (uint8_t i = 0; i < count; i++) {

        // wait for the disk to be ready
        ata_wait();

        // read 256 words (256 x 2 bytes = 512 bytes = 1 sector)
        for (int j = 0; j < 256; j++) {
            ((uint16_t*)buffer)[i * 256 + j] = inw(ATA_DATA);
        }
    }
}

void ata_write_sectors(uint32_t lba, uint8_t count, uint8_t *buffer) {
    // Inform disk that we are using LBA mode and select master drive
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    // Tell the disk how many sectors to read
    outb(ATA_SECTOR_COUNT, count);

    // Send the LBA address in 3 parts since the outb port only allows to send 8-bit each time
    // and LBA is 28-bits
    outb(ATA_LBA_LOW,  (uint8_t)(lba));          // bits 0-7
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));     // bits 8-15
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));    // bits 16-23

    // Send the READ command
    outb(ATA_COMMAND, ATA_CMD_WRITE);

    // Read the data sector by sector
    for (uint8_t i = 0; i < count; i++) {

        // wait for the disk to be ready
        ata_wait();

        // send 256 words
        for (int j = 0; j < 256; j++) {
            outw(ATA_DATA, ((uint16_t*)buffer)[i * 256 + j]);
        }

        // Flush the disk cache after each sector
        outb(ATA_COMMAND, 0xE7);

        // wait for flush to finish
        while (inb(ATA_STATUS) & ATA_STATUS_BSY);
    }
}