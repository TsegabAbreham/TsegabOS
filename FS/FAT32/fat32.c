#include "fat32.h"
#include "../../drivers/ATA/ata.h"
#include "../../drivers/Serial/libk/kprintf/kprintf.h"

static fat32_fs_t fs;
static uint8_t sector_buf[512];

int fat32_init(void) {
    // Reads LBA 0 into the buffer, which the first sector is where the BPB (map of the filesystem) lives
    ata_read_sectors(0, 1, sector_buf);

    // Casting sector_buf 512 raw bytes to fat32_bpb_t struct
    fat32_bpb_t *bpb = (fat32_bpb_t*)sector_buf;

    // Checks for two magic bytes 0x55 and 0xAA at pos 510 & 511, if those dont exist then it isn't a
    // valid boot sector
    if (sector_buf[510] != 0x55 || sector_buf[511] != 0xAA) {
        kprintf("FAT32: Invalid boot signature!\n");
        return FAT32_ERR_NOT_FAT32;
    }

    // Checks if the disk is really FAT32 and not FAT16 or FAT12
    if (bpb->fat_size_16 != 0 || bpb->root_entry_count != 0) {
        kprintf("FAT32: not a FAT32 volume!\n");
        return FAT32_ERR_NOT_FAT32;
    }

    // FAT table start (partition offset + reserved area)
    fs.fat_lba = bpb->hidden_sectors + bpb->reserved_sector_count;

    // Data region start (after FAT copies)
    fs.data_lba = fs.fat_lba + (bpb->num_fats * bpb->fat_size_32);

    // Root directory cluster
    fs.root_cluster = bpb->root_cluster;

    // Cluster size in sectors
    fs.sectors_per_cluster = bpb->sectors_per_cluster;

    // Sector size in bytes
    fs.bytes_per_sector = bpb->bytes_per_sector;

    kprintf("FAT32: init ok\n");
    kprintf("FAT32: fat_lba         = %u\n", fs.fat_lba);
    kprintf("FAT32: data_lba        = %u\n", fs.data_lba);
    kprintf("FAT32: root_cluster    = %u\n", fs.root_cluster);
    kprintf("FAT32: secs_per_cluster= %u\n", fs.sectors_per_cluster);

    return FAT32_OK;
}

uint32_t fat32_cluster_to_lba(uint32_t cluster) {
    return fs.data_lba + (cluster-2) * fs.sectors_per_cluster;
}

uint32_t fat32_next_cluster(uint32_t cluster) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fs.fat_lba + (fat_offset / 512);
    uint32_t entry_offset = fat_offset % 512;

    ata_read_sectors(fat_sector, 1, sector_buf);
    uint32_t next = *(uint32_t*)(sector_buf + entry_offset);
    next = next & 0x0FFFFFFF; // Masks since FAT32 uses 28-bit for entries and rest for flags

    return next;
}

int fat32_find_in_root(const char* name, const char* ext,
                             fat32_dir_entry_t* out_entry) {
    uint32_t cluster = fs.root_cluster;

    // Loops until it hits the end of the chain marker
    while (cluster < 0x0FFFFFF8) {
        uint32_t lba = fat32_cluster_to_lba(cluster);

        // Read each sector inside one cluster one by one
        for (uint8_t s = 0; s < fs.sectors_per_cluster; s++) {
            ata_read_sectors(lba + s, 1, sector_buf);

            fat32_dir_entry_t *entries = (fat32_dir_entry_t*)sector_buf;

            for (int i = 0; i < 16; i++) {
                // 0x00 means the entry and every entry after it is empty
                if (entries[i].name[0] == 0x00) {
                    return FAT32_ERR_NOT_FOUND;
                }
                // 0xE5 means the entry was deleted, the file used to exist but removed, therefore skip.
                if (entries[i].name[0] == 0xE5) {
                    continue;
                }
                // For now skip this since this means long filename entry, not real file entry.
                if (entries[i].attributes == FAT_ATTR_LFN) {
                    continue;
                }


                // assume file name matches until proven otherwise
                int name_match = 1;
                int ext_match  = 1;

                // compare 8-char filename (FAT8.3 format)
                for (int j = 0; j < 8; j++) {
                    if (entries[i].name[j] != name[j]) {
                        name_match = 0;
                        break;
                    }
                }

                // compare 3-char extension
                for (int j = 0; j < 3; j++) {
                    if (entries[i].ext[j] != ext[j]) {
                        ext_match = 0;
                        break;
                    }
                }

                // if both name and extension match, return this file entry
                if (name_match && ext_match) {
                    *out_entry = entries[i];
                    return FAT32_OK;
                }
            }
        }
        cluster = fat32_next_cluster(cluster);
    }

    return FAT32_ERR_NOT_FOUND;
}

int fat32_open(const char* name, const char* ext, fat32_file_t* out_file) {
    fat32_dir_entry_t entry;

    int result = fat32_find_in_root(name, ext, &entry);

    if (result != FAT32_OK) {
        kprintf("FAT32: file not found!\n");
        return FAT32_ERR_NOT_FOUND;
    }

    uint32_t first_cluster = ((uint32_t)entry.first_cluster_high << 16) |
                            (uint32_t)entry.first_cluster_low;

    out_file->first_cluster = first_cluster;
    out_file->current_cluster = first_cluster;
    out_file->file_size = entry.file_size;
    out_file->bytes_read = 0;

    return FAT32_OK;
}

uint32_t fat32_read(fat32_file_t* file, uint8_t* buffer, uint32_t size) {
    uint32_t bytes_read = 0;

    while (bytes_read < size) {
        if (file->bytes_read >= file->file_size) {
            break;
        }

        if (file->current_cluster >= 0x0FFFFFF8) {
            break;
        }

        uint32_t lba = fat32_cluster_to_lba(file->current_cluster);

        for (uint8_t s = 0; s < fs.sectors_per_cluster; s++) {
            ata_read_sectors(lba + s, 1, sector_buf);

            for (uint32_t i = 0; i < fs.bytes_per_sector; i++) {

                if (file->bytes_read >= file->file_size) break;
                if (bytes_read >= size) break;

                buffer[bytes_read] = sector_buf[i];
                bytes_read++;
                file->bytes_read++;
            }
        }
        file->current_cluster = fat32_next_cluster(file->current_cluster);
    }
    return bytes_read;
}

int fat32_write_fat_entry(uint32_t cluster, uint32_t value) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fs.fat_lba + (fat_offset / 512);
    uint32_t entry_offset = fat_offset % 512;

    ata_read_sectors(fat_sector, 1, sector_buf);

    // Mask the last 4-bits since they are for flags and leave the remaining 24bits
    uint32_t *entry = (uint32_t*)(sector_buf + entry_offset);
    *entry = (*entry & 0xF0000000) | (value & 0x0FFFFFFF);

    ata_write_sectors(fat_sector, 1, sector_buf);

    return FAT32_OK;
}

uint32_t fat32_find_free_cluster(void) {
    // calculate the sectors the FAT table occupies
    uint32_t fat_sector_count = fs.data_lba - fs.fat_lba;

    for (uint32_t i = 0; i < fat_sector_count; i++) {
        ata_read_sectors(fs.fat_lba + i, 1, sector_buf);

        uint32_t *entries = (uint32_t*)sector_buf;

        for (int j = 0; j < 128; j++) {
            // Converts sector index and entry index into the actual cluster number.
            uint32_t cluster = i * 128 + j;

            if (cluster < 2) continue; // cluster 0 and 1 are NOT for data, reserved.

            // Mask top 4 bits and check if that cluster is free
            if ((entries[j] & 0x0FFFFFFF) == FAT32_FREE_CLUSTER) {
                return cluster;
            }
        }
    }

    kprintf("FAT32: disk is full!\n");
    return 0;
}

uint32_t fat32_write(fat32_file_t* file, const uint8_t* buffer, uint32_t size) {
    uint32_t bytes_written = 0;

    while (bytes_written < size) {
        if (file->current_cluster == 0 ||
            file->current_cluster >= 0x0FFFFFF8) {
            uint32_t new_cluster = fat32_find_free_cluster();

            if (new_cluster == 0) {
                kprintf("FAT32: no free cluster!\n");
                break;
            }
            if (file->current_cluster == 0) {
                file->first_cluster = new_cluster;
                file->current_cluster = new_cluster;
            } else {
                fat32_write_fat_entry(file->current_cluster, new_cluster);
                file->current_cluster = new_cluster;
            }

            fat32_write_fat_entry(new_cluster, FAT32_EOC);
        }
        uint32_t lba = fat32_cluster_to_lba(file->current_cluster);

        for (uint8_t s = 0; s < fs.sectors_per_cluster; s++) {
            ata_read_sectors(lba + s, 1, sector_buf);

            for (uint32_t i = 0; i < fs.bytes_per_sector; i++) {
                if (bytes_written >= size) break;

                sector_buf[i] = buffer[bytes_written];
                bytes_written++;
                file->bytes_read++;
            }

            ata_write_sectors(lba + s, 1, sector_buf);
        }

        file->current_cluster = fat32_next_cluster(file->current_cluster);

    }
    return bytes_written;
}

int fat32_update_dir_entry(const char* name, const char* ext, uint32_t new_size, uint32_t first_cluster) {
    uint32_t cluster = fs.root_cluster;

    // Loops until it hits the end of the chain marker
    while (cluster < 0x0FFFFFF8) {
        uint32_t lba = fat32_cluster_to_lba(cluster);

        // Read each sector inside one cluster one by one
        for (uint8_t s = 0; s < fs.sectors_per_cluster; s++) {
            ata_read_sectors(lba + s, 1, sector_buf);

            fat32_dir_entry_t *entries = (fat32_dir_entry_t*)sector_buf;

            for (int i = 0; i < 16; i++) {
                // 0x00 means the entry and every entry after it is empty
                if (entries[i].name[0] == 0x00) {
                    return FAT32_ERR_NOT_FOUND;
                }
                // 0xE5 means the entry was deleted, the file used to exist but removed, therefore skip.
                if (entries[i].name[0] == 0xE5) {
                    continue;
                }
                // For now skip this since this means long filename entry, not real file entry.
                if (entries[i].attributes == FAT_ATTR_LFN) {
                    continue;
                }


                // assume file name matches until proven otherwise
                int name_match = 1;
                int ext_match  = 1;

                // compare 8-char filename (FAT8.3 format)
                for (int j = 0; j < 8; j++) {
                    if (entries[i].name[j] != name[j]) {
                        name_match = 0;
                        break;
                    }
                }

                // compare 3-char extension
                for (int j = 0; j < 3; j++) {
                    if (entries[i].ext[j] != ext[j]) {
                        ext_match = 0;
                        break;
                    }
                }

                // if both name and extension match, return this file entry
                if (name_match && ext_match) {
                    entries[i].file_size    = new_size;
                    entries[i].first_cluster_low = (uint16_t)(first_cluster & 0xFFFF);
                    entries[i].first_cluster_high = (uint16_t)(first_cluster >> 16);

                    ata_write_sectors(lba + s, 1, sector_buf);
                    return FAT32_OK;
                }
            }
        }
        cluster = fat32_next_cluster(cluster);
    }

    return FAT32_ERR_NOT_FOUND;
}

int fat32_create_file(const char* name, const char* ext) {
    uint32_t cluster = fs.root_cluster;

    while (cluster < 0x0FFFFFF8) {
        uint32_t lba = fat32_cluster_to_lba(cluster);

        for (uint8_t s = 0; s < fs.sectors_per_cluster; s++) {
            ata_read_sectors(lba + s, 1, sector_buf);

            fat32_dir_entry_t* entries = (fat32_dir_entry_t*)sector_buf;

            for (int i = 0; i < 16; i++) {
                if (entries[i].name[0] == 0x00 ||
                    entries[i].name[0] == 0xE5) {

                    // Found an empty slot — build the directory entry
                    fat32_dir_entry_t new_entry;

                    // zero out the entire entry first
                    uint8_t* p = (uint8_t*)&new_entry;
                    for (int k = 0; k < 32; k++) p[k] = 0;

                    // copy name and extension
                    for (int k = 0; k < 8; k++) new_entry.name[k] = name[k];
                    for (int k = 0; k < 3; k++) new_entry.ext[k]  = ext[k];

                    // set attributes, size, and cluster
                    new_entry.attributes         = FAT_ATTR_ARCHIVE;
                    new_entry.file_size          = 0;
                    new_entry.first_cluster_low  = 0;
                    new_entry.first_cluster_high = 0;

                    // write the new entry into the sector
                    entries[i] = new_entry;
                    ata_write_sectors(lba + s, 1, sector_buf);

                    kprintf("FAT32: created %s.%s", name, ext);
                    return FAT32_OK;
                }
            }
        }
        cluster = fat32_next_cluster(cluster);
    }
    kprintf("FAT32: root directory is full!\n");
    return FAT32_ERR_NOT_FOUND;
}