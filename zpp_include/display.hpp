// Copyright 2025 Haute école d'ingénierie et d'architecture de Fribourg
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/****************************************************************************
 * @file display.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration for wrapping zephyr os display
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

#if CONFIG_DISPLAY

// zephyr
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

// std
#include <functional>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

/** A digital output, used for setting the state of a pin
 *
 * @note Synchronization level: Interrupt safe
 *
 */

class Display : private NonCopyable<Display> {
 public:
  Display() = default;
  [[nodiscard]] ZephyrResult initialize();
  uint32_t getWidth() const;
  uint32_t getHeight() const;
  enum class Color {
    Black,
    White,
    Blue,
    Green
  };
  void fillDisplay(Color color);
  void setBackgroundColor(Color color);
  void fillRectangle(
      Color color, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height);
  
  void drawHorizontalLine(
      Color color, uint32_t xPos, uint32_t yPos, uint32_t lineWidth, uint32_t thickness);
  void drawVerticalLine(
      Color color, uint32_t xPos, uint32_t yPos, uint32_t lineWidth, uint32_t thickness);
  void drawLogo(uint16_t xPos,
                uint16_t yPos,
                const uint32_t* pSrc,
                uint16_t logoWidth,
                uint16_t logoHeight);
  // type definitions for font
  // Character descriptor: {unicode, offset_in_table, width}
  struct FontCharInfo {
    uint32_t unicode;
    uint32_t offset;
    uint8_t  width;
  };
  struct Font {
    uint8_t                    height;
    uint32_t                   char_count;
    const FontCharInfo        *chars;
    const uint8_t              *table;
  };
  
  void setFont(const Font* pFont);
  const Font* getFont() const;
  enum class AlignMode {
    CENTER_MODE = 0x01, /*!< Center mode */
    RIGHT_MODE  = 0x02, /*!< Right mode  */
    LEFT_MODE   = 0x03  /*!< Left mode   */
  };
  // returns the string length in pixels
  uint16_t getStringWidth(const char* text) const;
  void drawStringAtLine(Color color,
                        uint32_t line,
                        const char* text,
                        AlignMode alignMode);
  void drawStringAt(
      Color color, uint32_t xPos, uint32_t yPos, const char* text);

 private:
  // private methods
  static void fillColorArgb8888(uint32_t colorValue, uint8_t* pBuffer, size_t bufferSize);
  static void fillColorRgb888(uint32_t colorValue, uint8_t* pBuffer, size_t bufferSize);
  static void fillLineArgb8888(const uint32_t* pSrc,
                               size_t srcSize,
                               uint8_t* pBuffer);
  static void fillLineRgb888(const uint32_t* pSrc,
                             size_t srcSize,
                             uint8_t* pBuffer);
  uint32_t computeYPosFromLineNumber(uint32_t line);
  void displayChar(Color color, uint32_t xPos, uint32_t yPos, uint32_t unicode);
  void fillRgbRect(Color color,
                   uint32_t xPos,
                   uint32_t yPos,
                   uint32_t* pData,
                   uint32_t width,
                   uint32_t height);
  
  inline uint32_t getColorValue(Color color) const {
    switch (_displayCapabilities.current_pixel_format) {
      case PIXEL_FORMAT_ARGB_8888:
        switch (color) {
          case Color::Black:
            return ARGB8888Colors[0];
          case Color::White:
            return ARGB8888Colors[1];
          case Color::Blue:
            return ARGB8888Colors[2];
          case Color::Green:
            return ARGB8888Colors[3];
          default:
            __ASSERT(false, "Unsupported color");
            return ARGB8888Colors[0];
        }
        break;
      case PIXEL_FORMAT_RGB_888: {
        switch (color) {
          case Color::Black:
            return RGB8888Colors[0];
          case Color::White:
            return RGB8888Colors[1];
          case Color::Blue:
            return RGB8888Colors[2];
          case Color::Green:
            return RGB8888Colors[3];
          default:
            __ASSERT(false, "Unsupported color");
            return RGB8888Colors[0];
        }
        break;
      
      default:
        __ASSERT(false, "Unsupported pixel format");
        return 0;
      }
    }    
  }

  // constants
  static constexpr uint32_t H_STEP = 1;

  // Colors Black, White, Blue and Green in different formats
  // Colors in ARGB8888 format
  static constexpr uint32_t ARGB8888Colors[] = {0xFF000000, 0xFFFFFFFF, 0xFF0000FF, 0xFF00FF00};
  static constexpr uint32_t RGB8888Colors[] = {0x000000, 0xFFFFFF, 0x0000FF, 0x00FF00};
  
  // data members
  const struct device* _displayDevice                             = nullptr;
  struct display_capabilities _displayCapabilities                = {0};
  uint32_t _lcdXsize                                              = 0;
  uint32_t _lcdYsize                                              = 0;
  size_t _lineBufferSize                                          = 0;
  Color _backgroundColor                                          = Color::Black;
  uint32_t _backgroundColorValue                                  = 0;
  uint8_t* _lineBuffer                                            = nullptr;
  const Font* _pFont                                              = nullptr;
  std::function<void(uint32_t, uint8_t*, size_t)> _fillColorFunction = nullptr;
  std::function<void(const uint32_t*, size_t, uint8_t*)> _fillLineFunction =
      nullptr;
};

}  // namespace zpp_lib

#endif  // CONFIG_DISPLAY
