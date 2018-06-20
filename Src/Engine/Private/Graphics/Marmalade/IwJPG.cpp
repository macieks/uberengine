/*
 * This file is part of the Marmalade SDK Code Samples.
 *
 * Copyright (C) 2001-2011 Ideaworks3D Ltd.
 * All Rights Reserved.
 *
 * This source code is intended only as a supplement to Ideaworks Labs
 * Development Tools and/or on-line documentation.
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

// Includes

#include "IwTexture.h"
#include "Base/ueBase.h"

extern "C"
{
#include "jpeglib.h"
}

//-----------------------------------------------------------------------------

struct buf_source_mgr
{
    jpeg_source_mgr pub;
    char*           buf;
    char            buf_term[2];
    long            buf_size;
    long            pos;
    bool            read_started;
};

void init_source_from_buf(j_decompress_ptr cinfo){
  buf_source_mgr* src = (buf_source_mgr*) cinfo->src;
  src->read_started = true;
}

void skip_input_data_from_buf(j_decompress_ptr cinfo, long nbytes){
  buf_source_mgr* src = (buf_source_mgr*) cinfo->src;
  if (nbytes > 0) {
    src->pub.next_input_byte += (size_t) nbytes;
    src->pub.bytes_in_buffer -= (size_t) nbytes;
  }
}

boolean fill_input_buffer_from_buf(j_decompress_ptr cinfo){
  buf_source_mgr* src = (buf_source_mgr*) cinfo->src;

  if (src->pos == src->buf_size){
    src->buf_term[0] = (JOCTET) 0xFF;
    src->buf_term[1] = (JOCTET) JPEG_EOI;
    src->pub.next_input_byte = (JOCTET*)src->buf_term;
    src->pub.bytes_in_buffer = 2;
    src->read_started = false;
    return TRUE;
  }

  src->pub.next_input_byte = (JOCTET*)src->buf;
  src->pub.bytes_in_buffer = src->buf_size;
  src->pos = src->buf_size;
  src->read_started = false;

  return TRUE;
}

void term_source_from_buf(j_decompress_ptr cinfo){
}

void jpeg_buf_src (j_decompress_ptr cinfo, char* buf,long size){
  buf_source_mgr* src = (buf_source_mgr*) cinfo->src;
  if (cinfo->src == NULL) {
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                 sizeof(buf_source_mgr));
    src = (buf_source_mgr*) cinfo->src;
  }

  src = (buf_source_mgr*) cinfo->src;
  src->pub.init_source = init_source_from_buf;
  src->pub.fill_input_buffer = fill_input_buffer_from_buf;
  src->pub.skip_input_data = skip_input_data_from_buf;
  src->pub.resync_to_restart = jpeg_resync_to_restart;
  src->pub.term_source = term_source_from_buf;
  src->pub.bytes_in_buffer = 0;
  src->pub.next_input_byte = (JOCTET*)NULL;

  src->buf = buf;
  src->read_started = false;
  src->buf_size = size;
  src->pos = 0;
}

bool IsJPEG(const char * buf, int len)
{
    const char pJPEGSignature[] =
        { 0xFF, 0xD8, 0xFF, 0xE0, 0, 0, 0x4A, 0x46, 0x49, 0x46, 0x00 };

    if (len > (int)sizeof(pJPEGSignature))
    {
        if (!memcmp(buf, pJPEGSignature, 4) &&
            !memcmp(buf+6, pJPEGSignature+6, 5))
        {
            return true;
        }
    }

    return false;
}

bool IwImageUtil_LoadJPG(const char* buf, int len, CIwImage& img, bool downscale2x, u32 maxWidth, u32 maxHeight, u32& originalWidth, u32& originalHeight)
{
    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct cinfo;
    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);
    jpeg_buf_src(&cinfo, (char*) buf, len);
    jpeg_read_header(&cinfo, TRUE);
    cinfo.out_color_space = JCS_RGB;
    jpeg_start_decompress(&cinfo);

    int newlen = cinfo.image_width * cinfo.image_height * 3;
	u32 width = cinfo.image_width;
	u32 height = cinfo.image_height;

	originalWidth = cinfo.image_width;
	originalHeight = cinfo.image_height;

	const u32 maxPixels = maxWidth * maxHeight;

    u8* data = NULL;
	if (!downscale2x && width * height <= maxPixels)
		data = (u8*) s3eMalloc(newlen);
	if (!data)
	{
		// Simulatenously load and downsize the image (so, we're able to load even extremely large photos, e.g. 16000 x 14000)

		u32 tmpBufferSize = 0;
		u32 downScale = 1;
		do
		{
			do
			{
				downScale *= 2;
				width = cinfo.image_width / downScale;
				height = cinfo.image_height / downScale;
				tmpBufferSize = 3 * cinfo.image_width * downScale;
			}
			while (width * height > maxPixels);

			newlen = width * height * 3;
			data = (u8*) s3eMalloc(newlen + tmpBufferSize);

		} while (!data);

		// Memory allocated - now decode & downsize at the same time

		u8* tmpBuffer = data + newlen;

		while (cinfo.output_scanline < cinfo.output_height)
		{
			const u32 startScanLine = cinfo.output_scanline;

			// Load lines

			u32 blockHeight = 0;
			while (blockHeight < downScale && cinfo.output_scanline < cinfo.output_height)
			{
				u8* line = &tmpBuffer[blockHeight * cinfo.image_width * 3];
				jpeg_read_scanlines(&cinfo, &line, 1);
				blockHeight++;
			}

			// Average vertically

			u8* dstPtr = tmpBuffer;

			for (u32 x = 0; x < cinfo.image_width; x++)
			{
				for (u32 c = 0; c < 3; c++)
				{
					const u8* srcPtr = tmpBuffer + x * 3 + c;

					u32 value = 0;
					for (u32 y = 0; y < blockHeight; y++)
					{
						value += *srcPtr;
						srcPtr += cinfo.image_width * 3;
					}
					value /= blockHeight;

					dstPtr[c] = (u8) value;
				}

				dstPtr += 3;
			}

			// Average horizontally

			dstPtr = data + (startScanLine / downScale) * width * 3;
			for (u32 x = 0; x < width; x++)
			{
				const u32 blockWidth = ueMin(downScale, x * downScale - cinfo.image_width);

				for (u32 c = 0; c < 3; c++)
				{
					const u8* srcPtr = tmpBuffer + x * downScale * 3 + c;

					u32 value = 0;
					for (u32 xb = 0; xb < blockWidth; xb++)
						value += srcPtr[xb * 3];
					value /= blockWidth;

					dstPtr[c] = (u8) value;
				}
				dstPtr += 3;
			}
		}
	}

	// Load the image without resizing

	else
	{
		u8* linha = data;
		while (cinfo.output_scanline < cinfo.output_height)
		{
			linha = data + 3 * cinfo.image_width * cinfo.output_scanline;
			jpeg_read_scanlines(&cinfo, &linha, 1);
		}
	}

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

	img.SetFormat(CIwImage::BGR_888);
	img.SetWidth(width);
	img.SetHeight(height);
	img.SetPitch(width * 3);
	img.SetOwnedBuffers((uint8*) data, NULL);

	return true;
}

bool IwImageUtil_SaveJPG(u32 width, u32 height, u32 bytesPerPixel, const void* data, const char* filename)
{
	FILE* outfile = fopen(filename, "wb");
	if (!outfile)
		return false;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = width;	
	cinfo.image_height = height;
	cinfo.input_components = bytesPerPixel;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo );
	jpeg_start_compress(&cinfo, TRUE);

	JSAMPROW row_pointer[1];
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = (u8*) data + cinfo.next_scanline * cinfo.image_width * cinfo.input_components;
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(outfile);

	return true;
}