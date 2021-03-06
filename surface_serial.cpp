﻿#include "crt.h"
#include "io.h"
#include "surface.h"

#pragma pack(push)
#pragma pack(1)
struct bmp_header
{
	unsigned short	signature; // for bitmap 4D42
	unsigned		size; // size in bytes all file
	unsigned short	reserved[2]; // reserved, must be null
	unsigned		bits; // offset to bitmap bits from top of this structure
};
struct bmp_info
{
	unsigned		size; // The number of bytes required by the structure.
	int				width; // The width of the bitmap, in pixels. If biCompression is BI_JPEG or BI_PNG, the biWidth member specifies the width of the decompressed JPEG or PNG image file, respectively.
	int				height; // The height of the bitmap, in pixels. If biHeight is positive, the bitmap is a bottom-up DIB and its origin is the lower-left corner. If biHeight is negative, the bitmap is a top-down DIB and its origin is the upper-left corner.
	unsigned short	planes; // The number of planes for the target device. This value must be set to 1.
	unsigned short	bpp; // Bits per pixels. Can be 8, 16, 24, 32
	unsigned		compression;
	unsigned		size_image; // The size, in bytes, of the image.This may be set to zero for BI_RGB bitmaps.
	int				pels_per_meter_x; // The horizontal resolution, in pixels-per-meter, of the target device for the bitmap. An application can use this value to select a bitmap from a resource group that best matches the characteristics of the current device.
	int				pels_per_meter_y; // The vertical resolution, in pixels-per-meter, of the target device for the bitmap.
	unsigned		color_used; // Used mainly with 8-bit per pixel image format.
	unsigned		color_important;
};
#pragma pack(pop)

//void io::stream::writescan(void* p, int width, int height, int scan_line, int element_size)
//{
//	char* pc = (char*)p;
//	while(height > 0)
//	{
//		write(pc, width*element_size);
//		pc += scan_line;
//		height--;
//	}
//}

void draw::write(const char* url, unsigned char* bits, int width, int height, int bpp, int scanline, color* pallette)
{
	bmp_header bmf = {0};
	bmp_info bmi = {0};
	//
	bmf.size = sizeof(bmf);
	bmf.signature = 0x4D42;
	bmf.bits = sizeof(bmp_header) + sizeof(bmp_info);
	//
	bmi.size = sizeof(bmi);
	bmi.width = width;
	bmi.height = height;
	bmi.planes = 1;
	bmi.bpp = bpp;
	bmi.pels_per_meter_x = 96;
	bmi.pels_per_meter_y = 96;
	switch(bpp)
	{
	case 8:
		bmi.color_used = 256;
		if(!pallette)
			return;
		break;
	default:
		bmi.color_used = 0;
		break;
	}
	if(!scanline)
		scanline = color::scanline(bmi.width, bmi.bpp);
	int wscn = color::scanline(bmi.width, bmi.bpp);
	int size = height * scanline;
	io::file file(url, StreamWrite);
	if(!file)
		return;
	file.write(&bmf, sizeof(bmf));
	file.write(&bmi, sizeof(bmi));
	if(pallette && bmi.color_used)
		file.write(pallette, 256 * sizeof(color));
	// bits
	int pixbytes = bmi.bpp / 8;
	for(int y = 0; y < height; y++)
	{
		int w = pixbytes*width;
		unsigned char* p0 = (unsigned char*)bits + (height - y - 1)*scanline;
		file.write(p0, w);
		if(w < wscn)
		{
			char temp[16] = {0};
			file.write(p0, wscn - w);
		}
	}
}

void draw::surface::write(const char* url, color* pallette)
{
	draw::write(url, bits, width, height, bpp, 0, pallette);
}

draw::surface::surface(const char* url, color* pallette) :surface()
{
	read(url, pallette);
}

int draw::surface::read(const char* url, color* pallette, int need_bpp)
{
	unsigned size;
	resize(0, 0, 0, true);
	unsigned char* pin = (unsigned char*)loadb(url, (int*)&size);
	if(!pin)
		return ErrorFileNotFound;
	int e = NoCrtError;
	for(auto pv = surface::plugin::first; pv; pv = pv->next)
	{
		int width, height, bpp;
		if(pv->inspect(width, height, bpp, pin, size))
		{
			resize(width, height, bpp, true);
			e = pv->decode(bits, pin, size, scanline);
			if(e)
				break;
			if(need_bpp)
				convert(need_bpp, pallette);
			break;
		}
	}
	delete pin;
	return e;
}

static struct bmp_bitmap_plugin : public draw::surface::plugin
{

	bmp_bitmap_plugin() : plugin("bmp", "BMP images\0*.bmp\0")
	{
	}

	int decode(unsigned char* output, const unsigned char* input, unsigned input_size, int& output_scanline) override
	{
		int width, height, bpp;
		if(!output)
			return ErrorInvalidInputParameters;
		if(!inspect(width, height, bpp, input, input_size))
			return ErrorIllegalByteSequence;
		bpp = iabs(bpp);
		auto ph = (bmp_header*)input;
		auto pi = (bmp_info*)(input + sizeof(bmp_header));
		unsigned char* ppal = (unsigned char*)pi + sizeof(bmp_info);
		unsigned char* pb = (unsigned char*)input + ph->bits;
		int output_bpp = bpp;
		int input_bpp = bpp;
		int input_scanline = color::scanline(width, input_bpp);
		color e;
		for(int y = 0; y < height; y++)
		{
			unsigned char* d = output + y*output_scanline;
			unsigned char* s = pb + ((pi->height < 0) ? y*input_scanline : (pi->height - y - 1)*input_scanline);
			for(int x = 0; x < width; x++)
			{
				e.read(s, x, input_bpp, ppal);
				e.write(d, x, output_bpp, 0);
			}
		}
		return NoCrtError;
	}

	bool inspect(int& width, int& height, int& bpp, const unsigned char* input, unsigned size) override
	{
		if(input[1] != 0x4D || input[0] != 0x42)
			return false;
		auto ph = (bmp_header*)input;
		auto pi = (bmp_info*)(input + sizeof(bmp_header));
		width = pi->width;
		height = pi->height;
		bpp = -pi->bpp;
		return true;
	}

} instance;