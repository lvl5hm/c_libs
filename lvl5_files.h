#ifndef LVL5_FILES
#include <stdio.h>


typedef struct {
  i32 width;
  i32 height;
  byte *data;
} Bitmap;

#pragma pack(push, 1)
typedef struct {
  u32 info_header_size;
  i32 width;
  i32 height;
  i16 planes;
  i16 bits_per_pixel;
  u32 compression;
  u32 image_size;
  i32 x_pixels_per_meter;
  i32 y_pixels_per_meter;
  u32 colors_used;
  u32 important_colors;
  u32 red_mask;
  u32 green_mask;
  u32 blue_mask;
  u32 alpha_mask;
  u32 cs_type;
  i32 red_x;
  i32 red_y;
  i32 red_z;
  i32 green_x;
  i32 green_y;
  i32 green_z;
  i32 blue_x;
  i32 blue_y;
  i32 blue_z;
  u32 gamma_red;
  u32 gamma_green;
  u32 gamma_blue;
} Bmp_Info_Header;

typedef struct {
  u16 signature;
  u32 file_size;
  u32 reserved;
  u32 data_offset;
  Bmp_Info_Header info;
} Bmp_File_Header;

typedef struct {
  byte red;
  byte green;
  byte blue;
  byte reserved;
} Bmp_File_Color;
#pragma pack(pop)

#define BMP_SIGNATURE (('B' << 0) | ('M' << 8))

Bitmap bmp_load(char *file_name) {
  FILE *file;
  fopen_s(&file, file_name, "rb");
  fseek(file, 0, SEEK_END);
  u32 file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  byte *file_data = alloc(file_size);
  fread(file_data, file_size, 1, file);
  fclose(file);
  
  Bmp_File_Header *header = (Bmp_File_Header *)file_data;
  assert(header->signature == BMP_SIGNATURE);
  assert(header->file_size == file_size);
  assert(header->info.planes == 1);
  assert(header->info.bits_per_pixel == 32);
  
  Bitmap result = {0};
  result.data = file_data + header->data_offset;;
  result.width = header->info.width;
  result.height = header->info.height;
  
  return result;
}

void bmp_save(char *file_name, Bitmap bmp) {
  Bmp_Info_Header info = {0};
  info.info_header_size = 124;
  info.width = bmp.width;
  info.height = bmp.height;
  info.planes = 1;
  info.bits_per_pixel = 32;
  info.compression = 3;
  info.image_size = bmp.width*bmp.height*4;
  info.x_pixels_per_meter = 11811;
  info.y_pixels_per_meter = 11811;
  info.colors_used = 0;
  info.important_colors = 0;
  info.red_mask = 0x00FF0000;
  info.green_mask = 0x0000FF00;
  info.blue_mask = 0x000000FF;
  info.alpha_mask = 0xFF000000;
  info.cs_type = 0x73524742;
	
  Bmp_File_Header header = {0};
  header.signature = BMP_SIGNATURE;
  header.data_offset = 138;
  header.file_size = header.data_offset + info.image_size;
  header.reserved = 0;
  header.info = info;
  
  FILE *file;
  fopen_s(&file, file_name, "wb");
  fwrite(&header, sizeof(header), 1, file);
  fseek(file, header.data_offset, SEEK_SET);
  fwrite(bmp.data, info.image_size, 1, file);
  
  fclose(file);
}


#define LVL5_FILES
#endif