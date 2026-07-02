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
#include <string>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"
#include "zpp_include/zpp_assert.hpp"

namespace zpp_lib {

/** A digital output, used for setting the state of a pin
 *
 * @note Synchronization level: Interrupt safe
 *
 */

class Display final : private NonCopyable {
public:
  Display() = default;
  [[nodiscard]] ZephyrResult initialize();
  [[nodiscard]] uint32_t get_width() const;
  [[nodiscard]] uint32_t get_height() const;
  enum class Color : uint8_t { Black, White, Blue, Green };
  void fill_display(Color color);
  void set_background_color(Color color);
  void fill_rectangle(Color color, uint32_t x_pos, uint32_t y_pos, uint32_t width, uint32_t height);

  void draw_horizontal_line(Color color, uint32_t x_pos, uint32_t y_pos, uint32_t line_length, uint32_t thickness);
  void draw_vertical_line(Color color, uint32_t x_pos, uint32_t y_pos, uint32_t line_length, uint32_t thickness);
  void draw_icon(uint16_t x_pos, uint16_t y_pos, const uint32_t* p_src, uint16_t logo_width, uint16_t logo_height);
  // type definitions for font
  // Character descriptor: {unicode, offset_in_table, width}
  struct FontCharInfo {
    uint32_t unicode = 0;
    uint32_t offset  = 0;
    uint8_t width    = 0;
  };
  struct Font {
    uint8_t height            = 0;
    uint32_t char_count       = 0;
    const FontCharInfo* chars = nullptr;
    const uint8_t* table      = nullptr;
  };

  void set_font(const Font* p_font);
  [[nodiscard]] const Font* get_font() const;
  enum class AlignMode : uint8_t { Center = 0x01, Right = 0x02, Left = 0x03 };
  // returns the string length in pixels
  [[nodiscard]] uint16_t get_string_width(const std::string& text) const;
  void draw_string_at_line(Color color, uint32_t line, const std::string& text, AlignMode mode);
  void draw_string_at(Color color, uint32_t x_pos, uint32_t y_pos, const std::string& text);

private:
  // private methods
  static void fill_color_argb8888(uint32_t color_value, uint8_t* p_buffer, size_t buffer_size);
  static void fill_color_rgb888(uint32_t color_value, uint8_t* p_buffer, size_t buffer_size);
  static void fill_line_argb8888(const uint32_t* p_src, size_t src_size, uint8_t* p_buffer);
  static void fill_line_rgb888(const uint32_t* p_src, size_t src_size, uint8_t* p_buffer);
  [[nodiscard]] uint32_t compute_ypos_from_line_number(uint32_t line) const;
  void display_char(Color color, uint32_t x_pos, uint32_t y_pos, uint32_t unicode);
  void fill_rgb_rect(Color color, uint32_t x_pos, uint32_t y_pos, uint32_t* p_data, uint32_t width, uint32_t height);

  // This function is implicitly inlined since it is defined in the class definition.
  [[nodiscard]] uint32_t get_pixel_format() const { return _display_capabilities.current_pixel_format; }
  [[nodiscard]] uint32_t get_color_value(Color color) const {
    switch (_display_capabilities.current_pixel_format) {
    case PIXEL_FORMAT_ARGB_8888:
      switch (color) {
      case Color::Black:
        return kARGB8888Colors[0];
      case Color::White:
        return kARGB8888Colors[1];
      case Color::Blue:
        return kARGB8888Colors[2];
      case Color::Green:
        return kARGB8888Colors[3];
      default:
        ZPP_ASSERT(false, "Unsupported color");
        return kARGB8888Colors[0];
      }
      break;
    case PIXEL_FORMAT_RGB_888: {
      switch (color) {
      case Color::Black:
        return kRGB8888Colors[0];
      case Color::White:
        return kRGB8888Colors[1];
      case Color::Blue:
        return kRGB8888Colors[2];
      case Color::Green:
        return kRGB8888Colors[3];
      default:
        ZPP_ASSERT(false, "Unsupported color");
        return kRGB8888Colors[0];
      }
      break;

    default:
      ZPP_ASSERT(false, "Unsupported pixel format");
      return 0;
    }
    }
  }

  // constants
  static constexpr uint32_t kHStep = 1;

  // Colors Black, White, Blue and Green in different formats
  // Colors in ARGB8888 format
  // Using a c-array to declare constans is acceptable and more efficient than using std::array, 
  // since the size of the array is known at compile time and does not require dynamic memory allocation.
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  static constexpr uint32_t kARGB8888Colors[] = {0xFF000000, 0xFFFFFFFF, 0xFF0000FF, 0xFF00FF00};
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
  static constexpr uint32_t kRGB8888Colors[]  = {0x000000, 0xFFFFFF, 0x0000FF, 0x00FF00};

  // data members
  const struct device* _display_device                                       = nullptr;
  struct display_capabilities _display_capabilities                          = {.x_resolution = 0, 
                                                                                .y_resolution = 0, 
                                                                                .supported_pixel_formats = 0,
                                                                                .screen_info = 0, 
                                                                                .current_pixel_format = PIXEL_FORMAT_RGB_888, 
                                                                                .current_orientation = DISPLAY_ORIENTATION_NORMAL};
  uint32_t _lcd_xsize                                                        = 0;
  uint32_t _lcd_ysize                                                        = 0;
  size_t _line_buffer_size                                                   = 0;
  Color _background_color                                                    = Color::Black;
  uint32_t _background_color_value                                           = 0;
  uint8_t* _line_buffer                                                      = nullptr;
  const Font* _p_font                                                        = nullptr;
  std::function<void(uint32_t, uint8_t*, size_t)> _fill_color_function       = nullptr;
  std::function<void(const uint32_t*, size_t, uint8_t*)> _fill_line_function = nullptr;
};

}  // namespace zpp_lib

#endif  // CONFIG_DISPLAY
