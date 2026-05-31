import struct
import os

DISK_SIZE    = 100 * 1024 * 1024
SECTOR_SIZE  = 512
CLUSTER_SIZE = 8

disk = bytearray(DISK_SIZE)

TOTAL_SECTORS    = DISK_SIZE // SECTOR_SIZE
RESERVED_SECTORS = 32
NUM_FATS         = 2
FAT_SIZE         = (TOTAL_SECTORS - RESERVED_SECTORS) // (CLUSTER_SIZE * 2)

fat_start  = RESERVED_SECTORS * SECTOR_SIZE
data_start = (RESERVED_SECTORS + NUM_FATS * FAT_SIZE) * SECTOR_SIZE

# ── BPB ──────────────────────────────────────────
bpb = struct.pack(
    '<3s8sHBHBHHBHHHIIIHHIHH12sBBBI11s8s',
    b'\xEB\x58\x90',
    b'MSWIN4.1',
    SECTOR_SIZE,
    CLUSTER_SIZE,
    RESERVED_SECTORS,
    NUM_FATS,
    0,
    0,
    0xF8,
    0,
    63,
    255,
    0,
    TOTAL_SECTORS,
    FAT_SIZE,
    0,
    0,
    2,                  # root cluster = 2
    1,
    6,
    b'\x00' * 12,
    0x80,
    0,
    0x29,
    0x12345678,
    b'MYOS       ',
    b'FAT32   '
)

disk[0:len(bpb)] = bpb
disk[510] = 0x55
disk[511] = 0xAA

# ── FAT entries ───────────────────────────────────
def write_fat_entry(disk, base, cluster, value):
    offset = base + cluster * 4
    struct.pack_into('<I', disk, offset, value)

for base in [fat_start, fat_start + FAT_SIZE * SECTOR_SIZE]:
    write_fat_entry(disk, base, 0, 0x0FFFFFF8)  # reserved
    write_fat_entry(disk, base, 1, 0xFFFFFFFF)  # reserved
    write_fat_entry(disk, base, 2, 0x0FFFFFFF)  # root dir, end of chain
    write_fat_entry(disk, base, 3, 0x0FFFFFFF)  # TEST.TXT, end of chain

# ── Root directory at cluster 2 ───────────────────
# 32-byte directory entry for TEST.TXT
dir_entry = struct.pack(
    '<8s3sBBBHHHHHHHI',
    b'TEST    ',   # name, space padded to 8
    b'TXT',        # extension
    0x20,          # attribute: archive
    0,             # reserved
    0,             # create time tenth
    0,             # create time
    0,             # create date
    0,             # access date
    0,             # first cluster high
    0,             # write time
    0,             # write date
    3,             # first cluster low (cluster 3)
    18             # file size in bytes
)

root_dir_offset = data_start  # cluster 2
disk[root_dir_offset:root_dir_offset + len(dir_entry)] = dir_entry

# ── File contents at cluster 3 ────────────────────
file_contents = b"Hello from FAT32!\x00"
cluster3_offset = data_start + CLUSTER_SIZE * SECTOR_SIZE  # cluster 3
disk[cluster3_offset:cluster3_offset + len(file_contents)] = file_contents

# ── Write image ───────────────────────────────────
with open('disk.img', 'wb') as f:
    f.write(disk)

print("disk.img created!")
print(f"  FAT start:  sector {RESERVED_SECTORS}")
print(f"  Data start: sector {RESERVED_SECTORS + NUM_FATS * FAT_SIZE}")
print(f"  Root dir:   cluster 2")
print(f"  TEST.TXT:   cluster 3")