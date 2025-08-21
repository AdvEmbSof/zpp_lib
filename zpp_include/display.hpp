#pragma once

// zephyr
#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

// std
#include <functional>

namespace zpp_lib {
  
/** A digital output, used for setting the state of a pin
 *
 * @note Synchronization level: Interrupt safe
 *
 */
#if CONFIG_DISPLAY == 1

class Display : private NonCopyable<Display>
{
public:
    Display() = default;
    [[nodiscard]] ZephyrResult init();
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    void fillDisplay(uint32_t color);
    void fillRectangle(uint32_t color, uint32_t xPos, uint32_t yPos, uint32_t width, uint32_t height);
    void setTextColor(uint32_t color);
    void setBackColor(uint32_t color);
    struct Font {
      // cppcheck-suppress unusedStructMember
      const uint8_t* table;
      // cppcheck-suppress unusedStructMember
      uint16_t width;
      // cppcheck-suppress unusedStructMember
      uint16_t height;
    };
    void setFont(const Font* pFont);
    const Font* getFont() const;
    enum class AlignMode {
        CENTER_MODE = 0x01, /*!< Center mode */
        RIGHT_MODE  = 0x02, /*!< Right mode  */
        LEFT_MODE   = 0x03  /*!< Left mode   */
    };
    void drawTitle(const char* text, AlignMode alignMode);
    void drawPicture(uint16_t xPos, uint16_t yPos, const uint32_t* pSrc, uint16_t pictureWidth, uint16_t pictureHeight);
    void drawStringAtLine(uint32_t line, const char* text, AlignMode alignMode);
    void drawStringAt(uint32_t xPos, uint32_t yPos, const char* text, AlignMode mode);
    
private:
    // private methods
    static void fillColorArgb8888(uint32_t color, uint8_t* pBuffer, size_t bufferSize);
    static void fillColorRgb888(uint32_t color, uint8_t* pBuffer, size_t bufferSize);
    static void fillLineArgb8888(const uint32_t* pSrc, size_t srcSize, uint8_t* pBuffer);
    static void fillLineRgb888(const uint32_t* pSrc, size_t srcSize, uint8_t* pBuffer);
    uint32_t computeDisplayLineNumber(uint32_t line);
    void displayChar(uint32_t xPos, uint32_t yPos, uint8_t ascii);
    void drawChar(uint32_t xPos, uint32_t yPos, const uint8_t* pData);
    void fillRgbRect(uint32_t xPos, uint32_t yPos, uint32_t* pData, uint32_t width, uint32_t height);

    // constants
    static constexpr uint32_t H_STEP = 1;

    // data members
    const struct device* _displayDevice = nullptr;
  	struct display_capabilities _displayCapabilities = {0};
    uint32_t _lcdXsize = 0;
    uint32_t _lcdYsize = 0;
    size_t _lineBufferSize = 0;
    uint8_t* _lineBuffer = nullptr;
    uint32_t _textColor = 0;
    uint32_t _backColor = 0;
    const Font* _pFont = nullptr;
    std::function<void(uint32_t, uint8_t*, size_t)> _fillColorFunction = nullptr;
    std::function<void(const uint32_t*, size_t, uint8_t*)> _fillLineFunction = nullptr;
};

#endif

} // namespace zpp_lib

