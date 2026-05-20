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
 * @file digital_out.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class implementation for wrapping zephyr os display
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#if CONFIG_DISPLAY

#include "zpp_include/display.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zpp_drivers, CONFIG_ZPP_DRIVERS_LOG_LEVEL);

namespace zpp_lib {

K_HEAP_DEFINE(BUF_HEAP, 1024);

// we define our own min
// NOLINTNEXTLINE(build/include_what_you_use)
uint16_t min(uint32_t a, uint16_t b) { return (b < a) ? b : static_cast<uint16_t>(a); }


// Returns the char width or 0
inline uint8_t getCharWidth(const Display::Font* pFont,
                            uint32_t unicode) {
    __ASSERT(pFont != nullptr, "Font not set");
    for (uint32_t i = 0; i < pFont->char_count; i++) {
        if (pFont->chars[i].unicode == unicode) {
          return pFont->chars[i].width;
        }
    }
    return 0;
}

// Lookup a character's bitmap by Unicode code point
// Returns nullptr if the character is not in the font
inline const uint8_t* FontGetGlyph(const Display::Font* pFont,
                                   uint32_t unicode,
                                   uint8_t& charWidth) {
    __ASSERT(pFont != nullptr, "Font not set");
    for (uint32_t i = 0; i < pFont->char_count; i++) {
        if (pFont->chars[i].unicode == unicode) {
            charWidth = pFont->chars[i].width;
            return pFont->table + pFont->chars[i].offset;
        }
    }
    return nullptr;
}
 
ZephyrResult Display::initialize() {
  ZephyrResult res;
  _displayDevice = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
  if (!device_is_ready(_displayDevice)) {
    LOG_ERR("Device %s not found", _displayDevice->name);
    res.assign_error(zpp_lib::ZephyrErrorCode::k_nodev);
    return res;
  }

  LOG_INF("Display sample for %s", _displayDevice->name);
  display_get_capabilities(_displayDevice, &_displayCapabilities);

  _lcdXsize = _displayCapabilities.x_resolution;
  _lcdYsize = _displayCapabilities.y_resolution;

  _lineBufferSize = _lcdXsize;

  switch (_displayCapabilities.current_pixel_format) {
    case PIXEL_FORMAT_ARGB_8888: {
      _fillColorFunction = Display::fillColorArgb8888;
      _fillLineFunction  = Display::fillLineArgb8888;
      _lineBufferSize *= 4;
    } break;

    case PIXEL_FORMAT_RGB_888: {
      _fillColorFunction = Display::fillColorRgb888;
      _fillLineFunction  = Display::fillLineRgb888;
      _lineBufferSize *= 3;
    } break;

    default:
      LOG_ERR("Unsupported pixel format. Aborting sample.");
      res.assign_error(zpp_lib::ZephyrErrorCode::k_nosys);
      return res;
  }

  k_timeout_t timeout = {0};
  printk("line buffer size %d\n", _lineBufferSize);
  _lineBuffer = static_cast<uint8_t*>(k_heap_alloc(&BUF_HEAP, _lineBufferSize, timeout));
  if (_lineBuffer == nullptr) {
    LOG_ERR("Could not allocate memory of size %d. Aborting sample.", _lineBufferSize);
    res.assign_error(zpp_lib::ZephyrErrorCode::k_nomem);
    return res;
  }

  LOG_INF("Display capabilities: x_res %d y_res %d",
          _displayCapabilities.x_resolution,
          _displayCapabilities.y_resolution);
  LOG_INF("Display size: x %d, y %d", _lcdXsize, _lcdYsize);
  LOG_INF("Buffer size: %d", _lineBufferSize);

  return res;
}

uint32_t Display::getWidth() const { return _lcdXsize; }

uint32_t Display::getHeight() const { return _lcdYsize; }

void Display::fillDisplay(Color color) {
  fillRectangle(color, 0, 0, _lcdXsize, _lcdYsize);
  display_blanking_off(_displayDevice);
}

void Display::setBackgroundColor(Color color) { _backgroundColor = color; }

void Display::fillRectangle(
    Color color, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height) {
  _fillColorFunction(color, _lineBuffer, _lineBufferSize);

  struct display_buffer_descriptor bufDesc = {0};
  bufDesc.buf_size                         = _lineBufferSize;
  xPos           = zpp_lib::min(_displayCapabilities.x_resolution - 1, xPos);
  bufDesc.width  = zpp_lib::min(_displayCapabilities.x_resolution - xPos, width);
  bufDesc.pitch  = bufDesc.width;
  bufDesc.height = H_STEP;
  bufDesc.frame_incomplete = true;
  uint16_t firstLine       = zpp_lib::min(yPos, _displayCapabilities.y_resolution);
  uint16_t lastLine = zpp_lib::min(yPos + height, _displayCapabilities.y_resolution);
  for (uint32_t line = firstLine; line < lastLine; line += H_STEP) {
    // LOG_DBG("Writing buffer at post (%d - %d), width(%d)",getCharWidth xPos, line, bufDesc.width);
    auto ret = display_write(_displayDevice, xPos, line, &bufDesc, _lineBuffer);
    if (ret != 0) {
      LOG_ERR("Cannot write to display: %d", ret);
    }
  }
}

void Display::drawVerticalLine(
    Color color, uint32_t xPos, uint32_t yPos, uint32_t lineWidth, uint32_t thickness) {
  // LOG_DBG("Fill rectangle with color 0x%08x starting at (%d, %d) - (width %d, height
  // %d)",
  //        static_cast<uint32_t>(color),
  //        xPos,
  //        yPos,
  //        thickness,
  //        lineWidth);
  fillRectangle(color, xPos, yPos, thickness, lineWidth);
}

void Display::drawHorizontalLine(
    Color color, uint32_t xPos, uint32_t yPos, uint32_t lineWidth, uint32_t thickness) {
  // LOG_DBG("Fill rectangle with color 0x%08x starting at (%d, %d) - (width %d, height
  // %d)",
  //        static_cast<uint32_t>(color),
  //        xPos,
  //        yPos,
  //        lineWidth,
  //        thickness);
  fillRectangle(color, xPos, yPos, lineWidth, thickness);
}

void Display::drawLogo(uint16_t xPos,
                       uint16_t yPos,
                       const uint32_t* pSrc,
                       uint16_t logoWidth,
                       uint16_t logoHeight) {
  struct display_buffer_descriptor bufDesc = {0};
  xPos             = zpp_lib::min(_displayCapabilities.x_resolution - 1, xPos);
  bufDesc.buf_size = _lineBufferSize;
  bufDesc.width  = zpp_lib::min(_displayCapabilities.x_resolution - xPos - 1, logoWidth);
  bufDesc.pitch  = bufDesc.width;
  bufDesc.height = H_STEP;
  bufDesc.frame_incomplete = true;
  // draw the picture line by line
  uint16_t firstLine = zpp_lib::min(yPos, _displayCapabilities.y_resolution);
  uint16_t lastLine  = zpp_lib::min(yPos + logoHeight, _displayCapabilities.y_resolution);
  for (uint32_t line = firstLine; line < lastLine; line += H_STEP) {
    _fillLineFunction(pSrc, logoWidth, _lineBuffer);
    auto ret = display_write(_displayDevice, xPos, line, &bufDesc, _lineBuffer);
    if (ret != 0) {
      LOG_ERR("Cannot write to display: %d", ret);
    }
    pSrc += logoWidth;
  }
}

void Display::setFont(const Font* pFont) { _pFont = pFont; }

const Display::Font* Display::getFont() const { return _pFont; }

uint16_t Display::getStringWidth(const char* text) const {
  __ASSERT(_pFont != nullptr, "Font not set");
  uint16_t stringWidth = 0;
  while (*text != 0) {
    stringWidth += getCharWidth(_pFont, *text);
    text++;
  }
  return stringWidth;
}

void Display::drawStringAtLine(Color color,
                               uint32_t line,
                               const char* text,                  
                               AlignMode mode) {
  uint16_t nbrOfPixels = getStringWidth(text);
  uint32_t xPos = 0;
  switch (mode) { 
    case AlignMode::CENTER_MODE: {
      xPos = getWidth() / 2 - nbrOfPixels / 2;
    }
    break;

    case AlignMode::LEFT_MODE: {
      xPos = 0;
    }
    break;

    case AlignMode::RIGHT_MODE: {
      xPos = getWidth() - nbrOfPixels;
    }
    break;

    default: 
    break;
  }                             
  drawStringAt(color, xPos, computeYPosFromLineNumber(line), text);
}

void Display::drawStringAt(Color color, uint32_t xPos, uint32_t yPos, const char* text) {
  __ASSERT(_pFont != nullptr, "Font not set");

  // Get the text size
  uint32_t nbrOfChars = 0;
  char* ptr           = const_cast<char*>(text);
  while (*ptr++) {
    nbrOfChars++;
  }

  // get the number of pixels required for displaying the text
  uint16_t nbrOfPixels = getStringWidth(text);

  // compute the starting column
  int32_t refcolumn = xPos;
  LOG_DBG("Drawing %s at pos %d - %d (#pixels = %d, refcolumn %d)", text, xPos, yPos, nbrOfPixels, refcolumn);

  // Check that the Start column is located in the screenascii
  if ((refcolumn < 1) || (refcolumn >= 0x8000)) {
    refcolumn = 1;
  }

  // Send the string character by character on display
  uint32_t totalWidth = refcolumn;
  char c = *text;
  while (c != 0) {
    uint16_t charWidth = getCharWidth(_pFont, c);
    totalWidth += charWidth;
    if (totalWidth > _lcdXsize) {
      break;
    }
    
    // Display one character on display
    displayChar(color, refcolumn, yPos, *text);
    // Increment the column position by the width
    refcolumn += charWidth;

    // Point on the next character
    text++;
    c = *text;
  }
}

uint32_t Display::computeYPosFromLineNumber(uint32_t line) {
  return line * getFont()->height;
}

void Display::displayChar(Color color, uint32_t xPos, uint32_t yPos, uint32_t unicode) {
  __ASSERT(_pFont != nullptr, "Font not set");
  uint8_t charWidth = 0;
  const uint8_t* pData = FontGetGlyph(_pFont, unicode, charWidth);
  
  // LOG_DBG("Displaying char %c at %d - %d", unicode, xPos, yPos);

  // compute the bit offset in each line
  // nbrOfBytesPerLine represents the number of bytes
  // stored for each line of the character font
  uint32_t offset            = 8 * ((charWidth + 7) / 8) - charWidth;
  uint32_t nbrOfBytesPerLine = (charWidth + 7) / 8;

  // draw each line of the char stored in table
  for (uint32_t i = 0; i < _pFont->height; i++) {
    // get the start address of the line
    uint8_t* pchar = (const_cast<uint8_t*>(pData) + nbrOfBytesPerLine * i);

    // line represents the ith line of the character (as a uint64_t)
    uint64_t line = 0;
    for (uint32_t j = 0; j < nbrOfBytesPerLine; j++) {
      line <<= 8;
      line |= pchar[j];
    }

    if (_displayCapabilities.current_pixel_format == PIXEL_FORMAT_ARGB_8888 ||
        _displayCapabilities.current_pixel_format == PIXEL_FORMAT_RGB_888) {
      uint32_t argb8888[48] = {0};
      for (uint32_t j = 0; j < charWidth; j++) {
        // check whether the j^th bit in line is on or off
        uint64_t bitInPixel = static_cast<uint64_t>(1)
                              << static_cast<uint64_t>(charWidth - j + offset - 1);
        if (line & bitInPixel) {
          argb8888[j] = static_cast<uint32_t>(color);
        } else {
          argb8888[j] = static_cast<uint32_t>(_backgroundColor);
        }
      }
      fillRgbRect(color, xPos, yPos++, argb8888, charWidth, 1);  // NOLINT
    } else {
      LOG_ERR("Not implemented");
      __ASSERT(false, "Not implemented");
    }
  }
}

void Display::fillColorArgb8888(Color color, uint8_t* pBuffer, size_t bufferSize) {
  // LOG_INF("Filling buffer of size %d with value %d", buf_size, color);
  for (size_t idx = 0; idx < bufferSize; idx += 4) {
    // static_cast<uint32_t*> is not accepted here, reinterpret_cast is not supported
    // cppcheck-suppress cstyleCast
    // NOLINTNEXTLINE(readability/casting)
    *((uint32_t*)(pBuffer + idx)) = static_cast<uint32_t>(color);
  }
}
void Display::fillColorRgb888(Color color, uint8_t* pBuffer, size_t bufferSize) {
  uint32_t c = static_cast<uint32_t>(color);
  for (size_t idx = 0; idx < bufferSize; idx += 3) {
    *(pBuffer + idx + 0) = (uint8_t)((c >> 16) & 0xFF);
    *(pBuffer + idx + 1) = (uint8_t)((c >> 8) & 0xFF);
    *(pBuffer + idx + 2) = (uint8_t)((c >> 0) & 0xFF);
  }
}

void Display::fillLineArgb8888(const uint32_t* pSrc,
                               size_t srcSize,
                               uint8_t* pBuffer) {
  for (size_t idx = 0; idx < srcSize; idx += 4) {
    // static_cast<uint32_t*> is not accepted here, reinterpret_cast is not supported
    // cppcheck-suppress cstyleCast
    *((uint32_t*)(pBuffer + idx)) = *pSrc;  // NOLINT(readability/casting)
    pSrc++;
  }
}

void Display::fillLineRgb888(const uint32_t* pSrc,
                             size_t srcSize,
                             uint8_t* pBuffer) {
  for (size_t idx = 0; idx < srcSize; idx++) {
    uint32_t color           = *pSrc;
    *(pBuffer + 3 * idx + 0) = (uint8_t)((color >> 16) & 0xFF);
    *(pBuffer + 3 * idx + 1) = (uint8_t)((color >> 8) & 0xFF);
    *(pBuffer + 3 * idx + 2) = (uint8_t)((color >> 0) & 0xFF);
    pSrc++;
  }
}

void Display::fillRgbRect(Color color,
                          uint32_t xPos,
                          uint32_t yPos,
                          uint32_t* pData,
                          uint32_t width,
                          uint32_t height) {
  struct display_buffer_descriptor bufDesc = {0};
  xPos             = zpp_lib::min(_displayCapabilities.x_resolution - 1, xPos);
  bufDesc.buf_size = _lineBufferSize;
  bufDesc.width    = zpp_lib::min(_displayCapabilities.x_resolution - xPos - 1, width);
  bufDesc.pitch    = bufDesc.width;
  bufDesc.height   = H_STEP;
  bufDesc.frame_incomplete = true;
  // draw the rect line by line
  uint16_t firstLine = zpp_lib::min(yPos, _displayCapabilities.y_resolution);
  uint16_t lastLine  = zpp_lib::min(yPos + height, _displayCapabilities.y_resolution);
  for (uint32_t line = firstLine; line < lastLine; line += H_STEP) {
    _fillLineFunction(pData, width, _lineBuffer);
    auto ret = display_write(_displayDevice, xPos, line, &bufDesc, _lineBuffer);
    if (ret != 0) {
      LOG_ERR("Cannot write to display: %d", ret);
    }
    pData += width;
  }
}

}  // namespace zpp_lib

#endif  // CONFIG_DISPLAY
