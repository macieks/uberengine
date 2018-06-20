#include "TexLib_Private.h"

#include "png.h"

static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	TexLib_Context* ctx = (TexLib_Context*) png_ptr->io_ptr;
	if (!ctx->m_read(ctx, data, length))
	{
		// png_error
	}
}

bool TexLib_LoadPNG(TexLib_Context* ctx)
{
	// Set up a read buffer and check the library version
	png_structp png_ptr;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		//	nvDebug( "*** LoadPNG: Error allocating read buffer in file '%s'.\n", name );
		return false;
	}

	// Allocate/initialize a memory block for the image information
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		//	nvDebug( "*** LoadPNG: Error allocating image information for '%s'.\n", name );
		return false;
	}

	// Set up the error handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		//	nvDebug( "*** LoadPNG: Error reading png file '%s'.\n", name );
		return false;
	}

	// Set up the I/O functions.
	png_set_read_fn(png_ptr, ctx, user_read_data);


	// Retrieve the image header information
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);


	if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8) {
		// Convert indexed images to RGB.
		png_set_expand(png_ptr);
	}
	else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
		// Convert grayscale to RGB.
		png_set_expand(png_ptr);
	}
	else if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		// Expand images with transparency to full alpha channels
		// so the data will be available as RGBA quartets.
		png_set_expand(png_ptr);
	}
	else if (bit_depth < 8) {
		// If we have < 8 scale it up to 8.
		//png_set_expand(png_ptr);
		png_set_packing(png_ptr);
	}

	// Reduce bit depth.
	if (bit_depth == 16) {
		png_set_strip_16(png_ptr);
	}

	// Represent gray as RGB
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
	}

	// Convert to RGBA filling alpha with 0xFF.
//	if (!(color_type & PNG_COLOR_MASK_ALPHA)) {
//		png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
//	}

	// @todo Choose gamma according to the platform?
	double screen_gamma = 2.2;
	int intent;
	if (png_get_sRGB(png_ptr, info_ptr, &intent)) {
		png_set_gamma(png_ptr, screen_gamma, 0.45455);
	}
	else {
		double image_gamma;
		if (png_get_gAMA(png_ptr, info_ptr, &image_gamma)) {
			png_set_gamma(png_ptr, screen_gamma, image_gamma);
		}
		else {
			png_set_gamma(png_ptr, screen_gamma, 0.45455);
		}
	}

	// Perform the selected transforms.
	png_read_update_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

	TexLib_Header texHeader;
	texHeader.m_hasColor = (color_type & PNG_COLOR_MASK_COLOR) != 0;
	texHeader.m_hasAlpha = (color_type & PNG_COLOR_MASK_ALPHA) != 0;
	if (texHeader.m_hasColor && texHeader.m_hasAlpha)
		texHeader.m_format = TexLib_Format_ARGB;
	else if (texHeader.m_hasColor)
		texHeader.m_format = TexLib_Format_RGB;
	else
		texHeader.m_format = TexLib_Format_A;
	texHeader.m_width = width;
	texHeader.m_height = height;
	texHeader.m_depth = 0;
	texHeader.m_numFaces = 1;
	texHeader.m_numLevels = 1;

	if (!ctx->m_onHeader(ctx, &texHeader))
	{
		png_read_end(png_ptr, info_ptr);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}

	int numComponents = (texHeader.m_hasColor ? 3 : 0) + (texHeader.m_hasAlpha ? 1 : 0);
	int pixelsSize = width * height * numComponents;
	unsigned char* pixels = (unsigned char*) ctx->m_alloc(ctx, pixelsSize);
	if (!pixels)
	{
		png_read_end(png_ptr, info_ptr);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}

	// Read the image
	png_bytep * row_data = (png_bytep *) ctx->m_alloc(ctx, sizeof(png_bytep) * height);
	if (!row_data)
	{
		png_read_end(png_ptr, info_ptr);
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return false;
	}
	for (unsigned int y = 0; y < height; y++)
		row_data[y] = pixels + width * numComponents * (height - y - 1);

	png_read_image(png_ptr, row_data);
	ctx->m_free(ctx, row_data);

	// Finish things up
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	// Convert to BGR(A)

	const unsigned int numPixels = width * height;
	if (numComponents == 3)
	{
		unsigned char* curr = pixels;
		for (unsigned int i = 0; i < numPixels; i++)
		{
			const unsigned char rgb[3] = {curr[0], curr[1], curr[2]};
			curr[0] = rgb[2];
			curr[2] = rgb[0];
			curr += 3;
		}
	}
	else if (numComponents == 4)
	{
		unsigned char* curr = pixels;
		for (unsigned int i = 0; i < numPixels; i++)
		{
			const unsigned char rgba[4] = {curr[0], curr[1], curr[2], curr[3]};
			curr[0] = rgba[3];
			curr[1] = rgba[2];
			curr[2] = rgba[1];
			curr[3] = rgba[0];
			curr += 4;
		}
	}

	TexLib_Data_RAW rawData;
	rawData.m_curr = pixels;
	rawData.m_end = pixels + pixelsSize;

	const bool result = TexLib_TransformRawImage(ctx, &texHeader, &rawData);
	ctx->m_free(ctx, pixels);

	return result;
}