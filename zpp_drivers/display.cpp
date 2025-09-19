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

#if CONFIG_DISPLAY == 1

#include "zpp_include/display.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zpp_drivers, CONFIG_ZPP_DRIVERS_LOG_LEVEL);

namespace zpp_lib {

K_HEAP_DEFINE(BUF_HEAP, 200000);

uint16_t min(uint32_t a, uint16_t b) { return (b < a) ? b : (uint16_t)a; }

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

void Display::fillDisplay(uint32_t color) {
  fillRectangle(color, 0, 0, _lcdXsize, _lcdYsize);
  display_blanking_off(_displayDevice);
}

void Display::fillRectangle(
    uint32_t color, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height) {
  _fillColorFunction(color, _lineBuffer, _lineBufferSize);

  struct display_buffer_descriptor bufDesc = {0};
  bufDesc.buf_size                         = _lineBufferSize;
  xPos                     = min(_displayCapabilities.x_resolution - 1, xPos);
  bufDesc.width            = min(_displayCapabilities.x_resolution - xPos - 1, width);
  bufDesc.pitch            = bufDesc.width;
  bufDesc.height           = H_STEP;
  bufDesc.frame_incomplete = true;
  uint16_t firstLine       = min(yPos, _displayCapabilities.y_resolution);
  uint16_t lastLine        = min(yPos + height, _displayCapabilities.y_resolution);
  for (uint32_t line = firstLine; line < lastLine; line += H_STEP) {
    int rc = display_write(_displayDevice, xPos, line, &bufDesc, _lineBuffer);
    if (rc != 0) {
      LOG_ERR("Cannot write to display: %d", rc);
    }
  }
}

void Display::setTextColor(uint32_t color) { _textColor = color; }

void Display::setBackColor(uint32_t color) { _backColor = color; }

void Display::setFont(const Font* pFont) { _pFont = pFont; }

const Display::Font* Display::getFont() const { return _pFont; }

void Display::drawPicture(uint16_t xPos,
                          uint16_t yPos,
                          const uint32_t* pSrc,
                          uint16_t pictureWidth,
                          uint16_t pictureHeight) {
  struct display_buffer_descriptor bufDesc = {0};
  xPos             = min(_displayCapabilities.x_resolution - 1, xPos);
  bufDesc.buf_size = _lineBufferSize;
  bufDesc.width    = min(_displayCapabilities.x_resolution - xPos - 1, pictureWidth);
  bufDesc.pitch    = bufDesc.width;
  bufDesc.height   = H_STEP;
  bufDesc.frame_incomplete = true;
  // draw the picture line by line
  uint16_t firstLine = min(yPos, _displayCapabilities.y_resolution);
  uint16_t lastLine  = min(yPos + pictureHeight, _displayCapabilities.y_resolution);
  for (uint32_t line = firstLine; line < lastLine; line += H_STEP) {
    _fillLineFunction(pSrc, pictureWidth, _lineBuffer);
    int rc = display_write(_displayDevice, xPos, line, &bufDesc, _lineBuffer);
    if (rc != 0) {
      LOG_ERR("Cannot write to display: %d", rc);
    }
    pSrc += pictureWidth;
  }
}

void Display::drawStringAtLine(uint32_t line, const char* text, AlignMode mode) {
  drawStringAt(10, computeDisplayLineNumber(line), text, mode);
}
void Display::drawStringAt(uint32_t xPos,
                           uint32_t yPos,
                           const char* text,
                           AlignMode mode) {
  // Get the text size
  uint32_t nbrOfChars = 0;
  char* ptr           = const_cast<char*>(text);
  while (*ptr++) {
    nbrOfChars++;
  }

  // Get the number of caracters number per line
  uint32_t nbrOfCharPerLine = (_lcdXsize / _pFont->width);
  int32_t refcolumn         = 1;
  switch (mode) {
    case AlignMode::CENTER_MODE: {
      refcolumn =
          xPos + (((int32_t)nbrOfCharPerLine - (int32_t)nbrOfChars) * _pFont->width) / 2;
      break;
    }
    case AlignMode::LEFT_MODE: {
      refcolumn = xPos;
      break;
    }
    case AlignMode::RIGHT_MODE: {
      refcolumn = -xPos + ((nbrOfCharPerLine - nbrOfChars) * _pFont->width);
      break;
    }
    default: {
      refcolumn = xPos;
      break;
    }
  }

  // Check that the Start column is located in the screen
  if ((refcolumn < 1) || (refcolumn >= 0x8000)) {
    refcolumn = 1;
  }

  // Send the string character by character on display
  uint32_t i = 0;
  while ((*text != 0) & (((_lcdXsize - (i * _pFont->width)) & 0xFFFF) >= _pFont->width)) {
    // Display one character on display
    displayChar(refcolumn, yPos, *text);
    // Increment the column position by the width
    refcolumn += _pFont->width;

    // Point on the next character
    text++;
    i++;
  }
}

uint32_t Display::computeDisplayLineNumber(uint32_t line) {
  return line * getFont()->height;
}

void Display::displayChar(uint32_t xPos, uint32_t yPos, uint8_t ascii) {
  uint32_t offsetInTable = (ascii - ' ') * _pFont->height * ((_pFont->width + 7) / 8);
  drawChar(xPos, yPos, &_pFont->table[offsetInTable]);
}

void Display::drawChar(uint32_t xPos, uint32_t yPos, const uint8_t* pData) {
  // get the font height and width in number of pixels/bytes
  uint32_t height = _pFont->height;
  uint32_t width  = _pFont->width;

  // compute the bit offset in each line
  // nbrOfBytesPerLine represents the number of bytes
  // stored for each line of the character font
  uint32_t offset            = 8 * ((width + 7) / 8) - width;
  uint32_t nbrOfBytesPerLine = (width + 7) / 8;

  // draw each line of the char stored in table
  for (uint32_t i = 0; i < height; i++) {
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
      for (uint32_t j = 0; j < width; j++) {
        // check whether the j^th bit in line is on or off
        uint64_t bitInPixel = (uint64_t)1 << (uint64_t)(width - j + offset - 1);
        if (line & bitInPixel) {
          argb8888[j] = _textColor;
        } else {
          argb8888[j] = _backColor;
        }
      }
      fillRgbRect(xPos, yPos++, argb8888, width, 1);  // NOLINT
    } else {
      LOG_ERR("Not implemented");
      __ASSERT(false, "Not implemented");
    }
  }
}

void Display::fillColorArgb8888(uint32_t color, uint8_t* pBuffer, size_t bufferSize) {
  // LOG_INF("Filling buffer of size %d with value %d", buf_size, color);
  for (size_t idx = 0; idx < bufferSize; idx += 4) {
    // static_cast<uint32_t*> is not accepted here, reinterpret_cast is not supported
    // cppcheck-suppress cstyleCast
    *((uint32_t*)(pBuffer + idx)) = color;  // NOLINT(readability/casting)
  }
}
void Display::fillColorRgb888(uint32_t color, uint8_t* pBuffer, size_t bufferSize) {
  for (size_t idx = 0; idx < bufferSize; idx += 3) {
    *(pBuffer + idx + 0) = (uint8_t)((color >> 16) & 0xFF);
    *(pBuffer + idx + 1) = (uint8_t)((color >> 8) & 0xFF);
    *(pBuffer + idx + 2) = (uint8_t)((color >> 0) & 0xFF);
  }
}

void Display::fillLineArgb8888(const uint32_t* pSrc, size_t srcSize, uint8_t* pBuffer) {
  for (size_t idx = 0; idx < srcSize; idx += 4) {
    // static_cast<uint32_t*> is not accepted here, reinterpret_cast is not supported
    // cppcheck-suppress cstyleCast
    *((uint32_t*)(pBuffer + idx)) = *pSrc;  // NOLINT(readability/casting)
    pSrc++;
  }
}

void Display::fillLineRgb888(const uint32_t* pSrc, size_t srcSize, uint8_t* pBuffer) {
  for (size_t idx = 0; idx < srcSize; idx++) {
    uint32_t color           = *pSrc;
    *(pBuffer + 3 * idx + 0) = (uint8_t)((color >> 16) & 0xFF);
    *(pBuffer + 3 * idx + 1) = (uint8_t)((color >> 8) & 0xFF);
    *(pBuffer + 3 * idx + 2) = (uint8_t)((color >> 0) & 0xFF);
    pSrc++;
  }
}

void Display::fillRgbRect(
    uint32_t xPos, uint32_t yPos, uint32_t* pData, uint32_t width, uint32_t height) {
  struct display_buffer_descriptor bufDesc = {0};
  xPos                     = min(_displayCapabilities.x_resolution - 1, xPos);
  bufDesc.buf_size         = _lineBufferSize;
  bufDesc.width            = min(_displayCapabilities.x_resolution - xPos - 1, width);
  bufDesc.pitch            = bufDesc.width;
  bufDesc.height           = H_STEP;
  bufDesc.frame_incomplete = true;
  // draw the rect line by line
  uint16_t firstLine = min(yPos, _displayCapabilities.y_resolution);
  uint16_t lastLine  = min(yPos + height, _displayCapabilities.y_resolution);
  for (uint32_t line = firstLine; line < lastLine; line += H_STEP) {
    _fillLineFunction(pData, width, _lineBuffer);
    int rc = display_write(_displayDevice, xPos, line, &bufDesc, _lineBuffer);
    if (rc != 0) {
      LOG_ERR("Cannot write to display: %d", rc);
    }
    pData += width;
  }
}

}  // namespace zpp_lib

#endif  // CONFIG_DISPLAY == 1
