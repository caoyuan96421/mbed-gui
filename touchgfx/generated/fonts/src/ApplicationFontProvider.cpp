/* DO NOT EDIT THIS FILE */
/* This file is autogenerated by the text-database code generator */

#include <fonts/ApplicationFontProvider.hpp>
#include <touchgfx/InternalFlashFont.hpp>

#ifndef NO_USING_NAMESPACE_TOUCHGFX
using namespace touchgfx;
#endif

extern touchgfx::InternalFlashFont& getFont_trebuc_24_4bpp();
extern touchgfx::InternalFlashFont& getFont_trebucbd_40_4bpp();
extern touchgfx::InternalFlashFont& getFont_trebuc_32_4bpp();
extern touchgfx::InternalFlashFont& getFont_DejaVuSans_Bold_40_4bpp();
extern touchgfx::InternalFlashFont& getFont_trebuc_16_4bpp();

touchgfx::Font* ApplicationFontProvider::getFont(touchgfx::FontId fontId)
{
  switch(fontId)
  {
    
    case Typography::SMALL:
      return &(getFont_trebuc_24_4bpp());
    
    case Typography::LARGE:
      return &(getFont_trebucbd_40_4bpp());
    
    case Typography::MEDIUM:
      return &(getFont_trebuc_32_4bpp());
    
    case Typography::SYMBOL:
      return &(getFont_DejaVuSans_Bold_40_4bpp());
    
    case Typography::TINY:
      return &(getFont_trebuc_16_4bpp());
    
    default:
      return 0;
  }
}