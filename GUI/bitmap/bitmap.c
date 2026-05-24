#include "bitmap.h"
#include "../../memory/heap/heap.h"

typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} __attribute__((packed)) bmp_header_t;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    int32_t xppm;
    int32_t yppm;
    uint32_t colors_used;
    uint32_t important;
} __attribute__((packed)) dib_header_t;

bitmap_t load_bmp(const uint8_t* data)
{
    bmp_header_t* bmp = (bmp_header_t*)data;
    dib_header_t* dib = (dib_header_t*)(data + 14);

    bitmap_t result;
    result.width = dib->width;
    result.height = dib->height;

    const uint8_t* pixels = data + bmp->offset;

    result.pixels = (uint32_t*)kmalloc(result.width * result.height * 4);

    int padded_row_size = ((result.width * 3 + 3) & ~3);

    for (int y = 0; y < result.height; y++)
    {
        for (int x = 0; x < result.width; x++)
        {
            int src_index =
                (result.height - 1 - y) * padded_row_size + x * 3;

            uint8_t b = pixels[src_index + 0];
            uint8_t g = pixels[src_index + 1];
            uint8_t r = pixels[src_index + 2];

            result.pixels[y * result.width + x] =
                (r << 16) | (g << 8) | b;
        }
    }

    return result;
}

#define TRANSPARENT_IGNORED_COLOR 0xA011A0

void draw_bitmap(int x, int y, bitmap_t* bmp)
{
    for (int j = 0; j < bmp->height; j++)
    {
        for (int i = 0; i < bmp->width; i++)
        {
            uint32_t color = bmp->pixels[j * bmp->width + i];

            if ((color & 0x00FFFFFF) == TRANSPARENT_IGNORED_COLOR)
                continue;

            
            put_pixel(x + i, y + j, color & 0x00FFFFFF);
        }
    }
}