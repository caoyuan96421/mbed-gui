// -alpha_dither yes -dither 2 -non_opaque_image_format ARGB8888 -opaque_image_format RGB888 0x24381026
// Generated by imageconverter. Please, do not edit!

#include <touchgfx/Bitmap.hpp>
#include <BitmapDatabase.hpp>

#ifndef NO_USING_NAMESPACE_TOUCHGFX
using namespace touchgfx;
#endif

extern const unsigned char _button1[]; 
extern const unsigned char _button1_pressed[]; 
extern const unsigned char _button2[]; 
extern const unsigned char _button2_pressed[]; 
extern const unsigned char _button3[]; 
extern const unsigned char _button3_pressed[]; 
extern const unsigned char _dark_icons_down_arrow_48[]; 
extern const unsigned char _dark_icons_up_arrow_48[]; 
extern const unsigned char _home_button[]; 
extern const unsigned char _home_button2[]; 
extern const unsigned char _moon_small[]; 
extern const unsigned char _stars[]; 
extern const unsigned char _telescope[]; 
extern const unsigned char _utility[]; 
extern const unsigned char _wrench[]; 

const touchgfx::Bitmap::BitmapData bitmap_database[] = {
    { _button1, 0, 200, 200, 0, 0, 200, 200, touchgfx::Bitmap::RGB888 },
    { _button1_pressed, 0, 200, 200, 0, 0, 200, 200, touchgfx::Bitmap::RGB888 },
    { _button2, 0, 170, 60, 0, 0, 170, 60, touchgfx::Bitmap::RGB888 },
    { _button2_pressed, 0, 170, 60, 0, 0, 170, 60, touchgfx::Bitmap::RGB888 },
    { _button3, 0, 60, 60, 0, 0, 60, 60, touchgfx::Bitmap::RGB888 },
    { _button3_pressed, 0, 60, 60, 0, 0, 60, 60, touchgfx::Bitmap::RGB888 },
    { _dark_icons_down_arrow_48, 0, 48, 28, 18, 15, 12, 7, touchgfx::Bitmap::ARGB8888 },
    { _dark_icons_up_arrow_48, 0, 48, 28, 18, 4, 12, 7, touchgfx::Bitmap::ARGB8888 },
    { _home_button, 0, 47, 34, 12, 16, 22, 8, touchgfx::Bitmap::ARGB8888 },
    { _home_button2, 0, 47, 34, 12, 16, 22, 8, touchgfx::Bitmap::ARGB8888 },
    { _moon_small, 0, 165, 165, 24, 24, 116, 116, touchgfx::Bitmap::ARGB8888 },
    { _stars, 0, 200, 200, 123, 127, 15, 15, touchgfx::Bitmap::ARGB8888 },
    { _telescope, 0, 104, 158, 53, 85, 1, 72, touchgfx::Bitmap::ARGB8888 },
    { _utility, 0, 200, 200, 116, 63, 49, 69, touchgfx::Bitmap::ARGB8888 },
    { _wrench, 0, 146, 147, 75, 30, 1, 17, touchgfx::Bitmap::ARGB8888 }
};

namespace BitmapDatabase
{
  const touchgfx::Bitmap::BitmapData* getInstance()
  {
    return bitmap_database;
  }
  uint16_t getInstanceSize()
  {
    return (uint16_t)(sizeof(bitmap_database) / sizeof(touchgfx::Bitmap::BitmapData));
  }
}

