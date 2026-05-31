#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>
#include <stddef.h>

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────

#define FAT32_EOC           0x0FFFFFFF   // end of cluster chain
#define FAT32_FREE_CLUSTER  0x00000000   // cluster is free
#define FAT32_BAD_CLUSTER   0x0FFFFFF7   // bad sector marker

// Directory entry attribute flags
#define FAT_ATTR_READ_ONLY  0x01
#define FAT_ATTR_HIDDEN     0x02
#define FAT_ATTR_SYSTEM     0x04
#define FAT_ATTR_VOLUME_ID  0x08
#define FAT_ATTR_DIRECTORY  0x10
#define FAT_ATTR_ARCHIVE    0x20
#define FAT_ATTR_LFN        0x0F         // long filename entry (all 4 low bits set)

// ─────────────────────────────────────────────
//  BPB — BIOS Parameter Block (lives at LBA 0)
//  All fields are little-endian
// ─────────────────────────────────────────────
typedef struct __attribute__((packed)) {
    uint8_t  jump_boot[3];          // Boot jump instruction (ignored after bootloader stage)
    uint8_t  oem_name[8];           // OEM label string (cosmetic, informational only)

    uint16_t bytes_per_sector;      // Sector size (usually 512 bytes)
    uint8_t  sectors_per_cluster;   // Allocation unit size (clusters = sectors * this)
    uint16_t reserved_sector_count; // Sectors before FAT starts (boot + FS metadata)
    uint8_t  num_fats;              // Number of FAT tables (usually 2 for redundancy)

    uint16_t root_entry_count;      // Legacy (FAT12/16 only), must be 0 in FAT32
    uint16_t total_sectors_16;      // Total sectors (if small volume, else 0)
    uint8_t  media_type;            // Media descriptor (0xF8 = fixed disk)
    uint16_t fat_size_16;           // Legacy FAT size (unused in FAT32)

    uint16_t sectors_per_track;     // CHS legacy info (ignored in modern OS)
    uint16_t num_heads;             // CHS legacy info (ignored in modern OS)
    uint32_t hidden_sectors;        // Sectors before partition start (LBA offset base)
    uint32_t total_sectors_32;      // Total disk size in sectors (main value)

    // FAT32 extended BPB
    uint32_t fat_size_32;           // Size of one FAT table in sectors
    uint16_t ext_flags;             // FAT mirroring / active FAT settings
    uint16_t fs_version;            // FAT32 version (usually 0.0)
    uint32_t root_cluster;          // Starting cluster of root directory
    uint16_t fs_info_sector;        // FSInfo sector (free space tracking)
    uint16_t backup_boot_sector;    // Backup boot sector location

    uint8_t  reserved[12];          // Reserved space (must be ignored)
    uint8_t  drive_number;          // BIOS drive number (0x80 = HDD)
    uint8_t  reserved1;             // Reserved
    uint8_t  boot_signature;        // Signature (0x29 = extended fields valid)

    uint32_t volume_id;             // Serial number of volume
    uint8_t  volume_label[11];      // Human-readable volume name
    uint8_t  fs_type_label[8];      // "FAT32" label (DO NOT trust for detection)
} fat32_bpb_t;

// ─────────────────────────────────────────────
//  Directory entry — 32 bytes, packed
// ─────────────────────────────────────────────

typedef struct __attribute__((packed)) {
    uint8_t  name[8];               // 0x00  filename, space-padded (NOT null terminated)
    uint8_t  ext[3];                // 0x08  extension, space-padded
    uint8_t  attributes;            // 0x0B  see FAT_ATTR_* constants above
    uint8_t  reserved;              // 0x0C  always 0
    uint8_t  create_time_tenth;     // 0x0D  creation time, 10ms units
    uint16_t create_time;           // 0x0E  hour:minute:second packed
    uint16_t create_date;           // 0x10  year:month:day packed
    uint16_t access_date;           // 0x12  last access date
    uint16_t first_cluster_high;    // 0x14  high 16 bits of first cluster number
    uint16_t write_time;            // 0x16  last write time
    uint16_t write_date;            // 0x18  last write date
    uint16_t first_cluster_low;     // 0x1A  low 16 bits of first cluster number
    uint32_t file_size;             // 0x1C  file size in bytes (0 for directories)
} fat32_dir_entry_t;

// ─────────────────────────────────────────────
//  Runtime state — computed once in fat32_init,
//  then used by every other function
// ─────────────────────────────────────────────

typedef struct {
    uint32_t fat_lba;           // LBA of FAT table #1 -> where the FAT table is on disk
    uint32_t data_lba;          // LBA of first data sector (cluster 2)
    uint32_t root_cluster;      // cluster number of root directory
    uint8_t  sectors_per_cluster;
    uint16_t bytes_per_sector;
} fat32_fs_t;

// ─────────────────────────────────────────────
//  File handle — returned by fat32_open
// ─────────────────────────────────────────────

typedef struct {
    uint32_t first_cluster;     // starting cluster
    uint32_t file_size;         // bytes
    uint32_t current_cluster;   // where we are right now while reading
    uint32_t bytes_read;        // how far into the file we've read
} fat32_file_t;

// ─────────────────────────────────────────────
//  Return codes
// ─────────────────────────────────────────────

#define FAT32_OK            0
#define FAT32_ERR_NOT_FOUND 1
#define FAT32_ERR_NOT_FAT32 2
#define FAT32_ERR_READ_FAIL 3
#define FAT32_ERR_EOF       4

// ─────────────────────────────────────────────
//  Public API
// ─────────────────────────────────────────────

// Must call first — reads BPB, fills internal fat32_fs_t
int      fat32_init(void);

// Convert cluster number → LBA address
uint32_t fat32_cluster_to_lba(uint32_t cluster);

// Read one FAT entry — returns next cluster in chain
// Returns FAT32_EOC if end of chain, FAT32_FREE_CLUSTER if empty
uint32_t fat32_next_cluster(uint32_t cluster);

// Find a file by name in the root directory
// name should be "FILENAME", ext should be "EXT" (no dot, uppercase, space-padded)
// Returns FAT32_OK and fills out_entry on success
int      fat32_find_in_root(const char* name, const char* ext,
                             fat32_dir_entry_t* out_entry);

// Open a file — fills fat32_file_t handle
int      fat32_open(const char* name, const char* ext, fat32_file_t* out_file);

// Read up to 'size' bytes from an open file into buffer
// Returns number of bytes actually read
uint32_t fat32_read(fat32_file_t* file, uint8_t* buffer, uint32_t size);

#endif