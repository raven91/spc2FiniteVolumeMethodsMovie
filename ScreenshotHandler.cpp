//
// Created by Nikita Kruk on 16.07.18.
//

#include "ScreenshotHandler.hpp"

//#define SAVE_WITH_ALPHA_CHANNEL

ScreenshotHandler::ScreenshotHandler()
{

}

ScreenshotHandler::~ScreenshotHandler()
{

}

void ScreenshotHandler::TakeScreenshotPpm(int width, int height)
{
  const std::string spc2ScreenshotFileNamePpm("/Users/nikita/Documents/Projects/spc2/PngStorage/screenshot.ppm");
  GLubyte *pixels = NULL;
  size_t i, j, cur;
  GLubyte r, g, b;
  const size_t format_nchannels = 3;
  std::ofstream screenshot_file(spc2ScreenshotFileNamePpm, std::ios::out | std::ios::trunc);
  if (screenshot_file.is_open())
  {
    screenshot_file << "P3\n" << width << " " << height << "\n" << 255 << "\n";
    pixels = (GLubyte *)realloc(pixels, format_nchannels * sizeof(GLubyte) * width * height);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    for (i = 0; i < height; ++i)
    {
      for (j = 0; j < width; ++j)
      {
        cur = format_nchannels * ((height - i - 1) * width + j);
        r = (pixels)[cur];
        g = (pixels)[cur + 1];
        b = (pixels)[cur + 2];
        screenshot_file << std::setw(3) << static_cast<unsigned>(r) << " " << std::setw(3) << static_cast<unsigned>(g) << " " << std::setw(3) << static_cast<unsigned>(b) << " ";
      }
      screenshot_file << "\n";
    }
    screenshot_file.close();
  }
  free(pixels);
}

void ScreenshotHandler::TakeScreenshotPng(unsigned int width, unsigned int height, int image_index)
{
  const std::string spc2ScreenshotFileNamePng("/Users/nikita/Documents/Projects/spc2/PngStorage/screenshot.png");
  const std::string spc2ScreenshotStoragePng("/Users/nikita/Documents/Projects/spc2/PngStorage/");

  GLubyte *pixels = NULL;
  png_byte *png_bytes = NULL;
  png_byte **png_rows = NULL;

  size_t i, nvals;
#if defined(SAVE_WITH_ALPHA_CHANNEL)
  const size_t format_nchannels = 4;
#else
  const size_t format_nchannels = 3;
#endif
  std::ostringstream name_buffer;
  name_buffer << spc2ScreenshotStoragePng << image_index << ".png";
  FILE *f = fopen(name_buffer.str().c_str(), "wb");
  nvals = format_nchannels * width * height;
  pixels = (GLubyte *)realloc(pixels, nvals * sizeof(GLubyte));
  png_bytes = (GLubyte *)realloc(png_bytes, nvals * sizeof(png_byte));
  png_rows = (GLubyte **)realloc(png_rows, height * sizeof(png_byte*));
#if defined(SAVE_WITH_ALPHA_CHANNEL)
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#else
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
#endif
  for (i = 0; i < nvals; i++)
    (png_bytes)[i] = (pixels)[i];
  for (i = 0; i < height; i++)
    (png_rows)[height - i - 1] = &(png_bytes)[i * width * format_nchannels];
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) abort();
  png_infop info = png_create_info_struct(png);
  if (!info) abort();
  if (setjmp(png_jmpbuf(png))) abort();
  png_init_io(png, f);
#if defined(SAVE_WITH_ALPHA_CHANNEL)
  png_set_IHDR(
				 png,
				 info,
				 width,
				 height,
				 8,
				 PNG_COLOR_TYPE_RGBA,
				 PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT,
				 PNG_FILTER_TYPE_DEFAULT
				 );
#else
  png_set_IHDR(
      png,
      info,
      width,
      height,
      8,
      PNG_COLOR_TYPE_RGB,
      PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT
  );
#endif
  png_write_info(png, info);
  png_write_image(png, png_rows);
  png_write_end(png, NULL);
  fclose(f);

  free(pixels);
  free(png_bytes);
  free(png_rows);
}