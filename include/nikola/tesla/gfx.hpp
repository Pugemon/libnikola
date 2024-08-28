//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_GFX_HPP
#define LIBNIKOLA_GFX_HPP

#include <algorithm>
#include <cstring>
#include <cwctype>
#include <string>

#include <cmath>
#include <cstdlib>
#include <strings.h>
#include <switch.h>

#include "cfg.hpp"
#include "../stb_truetype.h"
#include "../tesla.hpp"

#define ASSERT_FATAL(x) \
  if (Result res = x; R_FAILED(res)) \
  fatalThrow(res)

namespace nikola::tsl::gfx
{

extern "C" u64 __nx_vi_layer_id;

bool isValidHexColor(const std::string& hexColor)
{
  // Check if the string is a valid hexadecimal color of the format "#RRGGBB"
  if (hexColor.size() != 6) {
    return false;  // Must be exactly 6 characters long
  }

  for (char c : hexColor) {
    if (!isxdigit(c)) {
      return false;  // Must contain only hexadecimal digits (0-9, A-F, a-f)
    }
  }

  return true;
}

/**
 * @brief RGBA4444 Color structure
 */
struct Color
{
  union
  {
    struct
    {
      u16 r : 4, g : 4, b : 4, a : 4;
    } NX_PACKED;
    u16 rgba;
  };

  inline Color(u16 raw)
      : rgba(raw)
  {
  }
  inline Color(u8 r, u8 g, u8 b, u8 a)
      : r(r)
      , g(g)
      , b(b)
      , a(a)
  {
  }
};

Color RGB888(std::string hexColor, std::string defaultHexColor = "#FFFFFF")
{
  // Remove the '#' character if it's present
  if (!hexColor.empty() && hexColor[0] == '#') {
    hexColor = hexColor.substr(1);
  }

  if (isValidHexColor(hexColor)) {
    std::string r = hexColor.substr(
        0, 2);  // Extract the first two characters (red component)
    std::string g = hexColor.substr(
        2, 2);  // Extract the next two characters (green component)
    std::string b = hexColor.substr(
        4, 2);  // Extract the last two characters (blue component)

    // Convert the RGBA8888 strings to RGBA4444
    uint8_t redValue = std::stoi(r, nullptr, 16) >> 4;  // Right-shift by 4 bits
    uint8_t greenValue =
        std::stoi(g, nullptr, 16) >> 4;  // Right-shift by 4 bits
    uint8_t blueValue =
        std::stoi(b, nullptr, 16) >> 4;  // Right-shift by 4 bits

    // Create a Color with the extracted RGB values

    return Color(redValue, greenValue, blueValue, 15);
  }
  return RGB888(defaultHexColor);
}

/**
 * @brief Manages the Tesla layer and draws raw data to the screen
 */
class Renderer final
{
public:
  Renderer& operator=(Renderer&) = delete;

  friend class tsl::Overlay;

  /**
   * @brief Handles opacity of drawn colors for fadeout. Pass all colors through
   * this function in order to apply opacity properly
   *
   * @param c Original color
   * @return Color with applied opacity
   */
  static Color a(const Color& c)
  {
    return (c.rgba & 0x0FFF)
        | (static_cast<u8>(c.a * Renderer::s_opacity) << 12);
  }

  /**
   * @brief Enables scissoring, discarding of any draw outside the given
   * boundaries
   *
   * @param x x pos
   * @param y y pos
   * @param w Width
   * @param h Height
   */
  inline void enableScissoring(u16 x, u16 y, u16 w, u16 h)
  {
    this->m_scissoring = true;

    this->m_scissorBounds[0] = x;
    this->m_scissorBounds[1] = y;
    this->m_scissorBounds[2] = w;
    this->m_scissorBounds[3] = h;
  }

  /**
   * @brief Disables scissoring
   */
  inline void disableScissoring() { this->m_scissoring = false; }

  // Drawing functions

  /**
   * @brief Draw a single pixel onto the screen
   *
   * @param x X pos
   * @param y Y pos
   * @param color Color
   */
  inline void setPixel(s16 x, s16 y, Color color)
  {
    if (x < 0 || y < 0 || x >= cfg::FramebufferWidth
        || y >= cfg::FramebufferHeight)
      return;

    static_cast<Color*>(
        this->getCurrentFramebuffer())[this->getPixelOffset(x, y)] = color;
  }

  /**
   * @brief Blends two colors
   *
   * @param src Source color
   * @param dst Destination color
   * @param alpha Opacity
   * @return Blended color
   */
  inline u8 blendColor(u8 src, u8 dst, u8 alpha)
  {
    u8 oneMinusAlpha = 0x0F - alpha;

    return (dst * alpha + src * oneMinusAlpha) / float(0xF);
  }

  /**
   * @brief Draws a single source blended pixel onto the screen
   *
   * @param x X pos
   * @param y Y pos
   * @param color Color
   */
  inline void setPixelBlendSrc(s16 x, s16 y, Color color)
  {
    if (x < 0 || y < 0 || x >= cfg::FramebufferWidth
        || y >= cfg::FramebufferHeight)
      return;

    Color src((static_cast<u16*>(
        this->getCurrentFramebuffer()))[this->getPixelOffset(x, y)]);
    Color dst(color);
    Color end(0);

    end.r = this->blendColor(src.r, dst.r, dst.a);
    end.g = this->blendColor(src.g, dst.g, dst.a);
    end.b = this->blendColor(src.b, dst.b, dst.a);
    end.a = src.a;

    this->setPixel(x, y, end);
  }

  /**
   * @brief Draws a single destination blended pixel onto the screen
   *
   * @param x X pos
   * @param y Y pos
   * @param color Color
   */
  inline void setPixelBlendDst(s16 x, s16 y, Color color)
  {
    if (x < 0 || y < 0 || x >= cfg::FramebufferWidth
        || y >= cfg::FramebufferHeight)
      return;

    Color src((static_cast<u16*>(
        this->getCurrentFramebuffer()))[this->getPixelOffset(x, y)]);
    Color dst(color);
    Color end(0);

    end.r = this->blendColor(src.r, dst.r, dst.a);
    end.g = this->blendColor(src.g, dst.g, dst.a);
    end.b = this->blendColor(src.b, dst.b, dst.a);
    end.a = dst.a;

    this->setPixel(x, y, end);
  }

  /**
   * @brief Draws a rectangle of given sizes
   *
   * @param x X pos
   * @param y Y pos
   * @param w Width
   * @param h Height
   * @param color Color
   */
  inline void drawRect(s16 x, s16 y, s16 w, s16 h, Color color)
  {
    for (s16 x1 = x; x1 < (x + w); x1++)
      for (s16 y1 = y; y1 < (y + h); y1++)
        this->setPixelBlendDst(x1, y1, color);
  }

  /**
   * @brief Draws a rectangle of given sizes with empty filling
   *
   * @param x X pos
   * @param y Y pos
   * @param w Width
   * @param h Height
   * @param color Color
   */
  inline void drawEmptyRect(s16 x, s16 y, s16 w, s16 h, Color color)
  {
    if (x < 0 || y < 0 || x >= cfg::FramebufferWidth
        || y >= cfg::FramebufferHeight)
      return;

    for (s16 x1 = x; x1 <= (x + w); x1++)
      for (s16 y1 = y; y1 <= (y + h); y1++)
        if (y1 == y || x1 == x || y1 == y + h || x1 == x + w)
          this->setPixelBlendDst(x1, y1, color);
  }

  /**
   * @brief Draws a line
   *
   * @param x0 Start X pos
   * @param y0 Start Y pos
   * @param x1 End X pos
   * @param y1 End Y pos
   * @param color Color
   */
  inline void drawLine(s16 x0, s16 y0, s16 x1, s16 y1, Color color)
  {
    if ((x0 == x1) && (y0 == y1)) {
      this->setPixelBlendDst(x0, y0, color);
      return;
    }

    s16 x_max = std::max(x0, x1);
    s16 y_max = std::max(y0, y1);
    s16 x_min = std::min(x0, x1);
    s16 y_min = std::min(y0, y1);

    if (x_min < 0 || y_min < 0 || x_min >= cfg::FramebufferWidth
        || y_min >= cfg::FramebufferHeight)
      return;

    // y = mx + b
    s16 dy = y_max - y_min;
    s16 dx = x_max - x_min;

    if (dx == 0) {
      for (s16 y = y_min; y <= y_max; y++) {
        this->setPixelBlendDst(x_min, y, color);
      }
      return;
    }

    float m = (float)dy / float(dx);
    float b = y_min - (m * x_min);

    for (s16 x = x_min; x <= x_max; x++) {
      s16 y = std::lround((m * (float)x) + b);
      s16 y_end = std::lround((m * (float)(x + 1)) + b);
      if (y == y_end) {
        if (x <= x_max && y <= y_max)
          this->setPixelBlendDst(x, y, color);
      } else
        while (y < y_end) {
          if (x <= x_max && y <= y_max)
            this->setPixelBlendDst(x, y, color);
          y += 1;
        }
    }
  }

  /**
   * @brief Draws a dashed line
   *
   * @param x0 Start X pos
   * @param y0 Start Y pos
   * @param x1 End X pos
   * @param y1 End Y pos
   * @param line_width How long one line can be
   * @param color Color
   */
  inline void drawDashedLine(
      s16 x0, s16 y0, s16 x1, s16 y1, s16 line_width, Color color)
  {
    // Source of formula:
    // https://www.cc.gatech.edu/grads/m/Aaron.E.McClennen/Bresenham/code.html

    s16 x_min = std::min(x0, x1);
    s16 x_max = std::max(x0, x1);
    s16 y_min = std::min(y0, y1);
    s16 y_max = std::max(y0, y1);

    if (x_min < 0 || y_min < 0 || x_min >= cfg::FramebufferWidth
        || y_min >= cfg::FramebufferHeight)
      return;

    s16 dx = x_max - x_min;
    s16 dy = y_max - y_min;
    s16 d = 2 * dy - dx;
    s16 incrE = 2 * dy;
    s16 incrNE = 2 * (dy - dx);

    this->setPixelBlendDst(x_min, y_min, color);

    s16 x = x_min;
    s16 y = y_min;
    s16 rendered = 0;

    while (x < x1) {
      if (d <= 0) {
        d += incrE;
        x++;
      } else {
        d += incrNE;
        x++;
        y++;
      }
      rendered++;
      if (x < 0 || y < 0 || x >= cfg::FramebufferWidth
          || y >= cfg::FramebufferHeight)
        continue;
      if (x <= x_max && y <= y_max) {
        if (rendered > 0 && rendered < line_width) {
          this->setPixelBlendDst(x, y, color);
        } else if (rendered > 0 && rendered >= line_width) {
          rendered *= -1;
        }
      }
    }
  }

  /**
   * @brief Draws a RGBA8888 bitmap from memory
   *
   * @param x X start position
   * @param y Y start position
   * @param w Bitmap width
   * @param h Bitmap height
   * @param bmp Pointer to bitmap data
   */
  void drawBitmap(s32 x, s32 y, s32 w, s32 h, const u8* bmp)
  {
    for (s32 y1 = 0; y1 < h; y1++) {
      for (s32 x1 = 0; x1 < w; x1++) {
        const Color color = {static_cast<u8>(bmp[1] >> 4),
                             static_cast<u8>(bmp[2] >> 4),
                             static_cast<u8>(bmp[3] >> 4),
                             static_cast<u8>(bmp[0] >> 4)};
        setPixelBlendSrc(x + x1, y + y1, a(color));
        bmp += 4;
      }
    }
  }

  /**
   * @brief Fills the entire layer with a given color
   *
   * @param color Color
   */
  inline void fillScreen(Color color)
  {
    std::fill_n(static_cast<Color*>(this->getCurrentFramebuffer()),
                this->getFramebufferSize() / sizeof(Color),
                color);
  }

  /**
   * @brief Clears the layer (With transparency)
   *
   */
  inline void clearScreen() { this->fillScreen({0x00, 0x00, 0x00, 0x00}); }

  inline void setLayerPos(u32 x, u32 y)
  {
    float ratio = 1.5;
    u32 maxX = cfg::ScreenWidth - (int)(ratio * cfg::FramebufferWidth);
    u32 maxY = cfg::ScreenHeight - (int)(ratio * cfg::FramebufferHeight);
    if (x > maxX || y > maxY) {
      return;
    }
    setLayerPosImpl(x, y);
  }

  static Renderer& getRenderer() { return get(); }

  /**
   * @brief Draws a string
   *
   * @param string String to draw
   * @param monospace Draw string in monospace font
   * @param x X pos
   * @param y Y pos
   * @param fontSize Height of the text drawn in pixels
   * @param color Text color. Use transparent color to skip drawing and only get
   * the string's dimensions
   * @return Dimensions of drawn string
   */
  std::pair<u32, u32> drawString(const char* string,
                                 bool monospace,
                                 u32 x,
                                 u32 y,
                                 float fontSize,
                                 Color color)
  {
    const size_t stringLength = strlen(string);

    u32 maxX = x;
    u32 currX = x;
    u32 currY = y;
    u32 prevCharacter = 0;

    u32 i = 0;

    do {
      u32 currCharacter;
      ssize_t codepointWidth =
          decode_utf8(&currCharacter, reinterpret_cast<const u8*>(string + i));

      if (codepointWidth <= 0)
        break;

      i += codepointWidth;

      stbtt_fontinfo* currFont = nullptr;

      if (stbtt_FindGlyphIndex(&this->m_extFont, currCharacter))
        currFont = &this->m_extFont;
      else
        currFont = &this->m_stdFont;

      float currFontSize = stbtt_ScaleForPixelHeight(currFont, fontSize);
      currX += currFontSize
          * stbtt_GetCodepointKernAdvance(
                   currFont, prevCharacter, currCharacter);

      int bounds[4] = {0};
      stbtt_GetCodepointBitmapBoxSubpixel(currFont,
                                          currCharacter,
                                          currFontSize,
                                          currFontSize,
                                          0,
                                          0,
                                          &bounds[0],
                                          &bounds[1],
                                          &bounds[2],
                                          &bounds[3]);

      int xAdvance = 0, yAdvance = 0;
      stbtt_GetCodepointHMetrics(
          currFont, monospace ? 'W' : currCharacter, &xAdvance, &yAdvance);

      if (currCharacter == '\n') {
        maxX = std::max(currX, maxX);

        currX = x;
        currY += fontSize;

        continue;
      }

      if (!std::iswspace(currCharacter) && fontSize > 0 && color.a != 0x0)
        this->drawGlyph(currCharacter,
                        currX + bounds[0],
                        currY + bounds[1],
                        color,
                        currFont,
                        currFontSize);

      currX += xAdvance * currFontSize;

    } while (i < stringLength);

    maxX = std::max(currX, maxX);

    return {maxX - x, currY - y};
  }

private:
  Renderer() {}

  /**
   * @brief Gets the renderer instance
   *
   * @return Renderer
   */
  static Renderer& get()
  {
    static Renderer renderer;

    return renderer;
  }

  /**
   * @brief Sets the opacity of the layer
   *
   * @param opacity Opacity
   */
  static void setOpacity(float opacity)
  {
    opacity = std::clamp(opacity, 0.0F, 1.0F);

    Renderer::s_opacity = opacity;
  }

  bool m_initialized = false;
  ViDisplay m_display;
  ViLayer m_layer;
  Event m_vsyncEvent;

  NWindow m_window;
  Framebuffer m_framebuffer;
  void* m_currentFramebuffer = nullptr;

  bool m_scissoring = false;
  u16 m_scissorBounds[4];

  stbtt_fontinfo m_stdFont, m_extFont;

  static inline float s_opacity = 1.0F;

  /**
   * @brief Get the current framebuffer address
   *
   * @return Framebuffer address
   */
  inline void* getCurrentFramebuffer() { return this->m_currentFramebuffer; }

  /**
   * @brief Get the next framebuffer address
   *
   * @return Next framebuffer address
   */
  inline void* getNextFramebuffer()
  {
    return static_cast<u8*>(this->m_framebuffer.buf)
        + this->getNextFramebufferSlot() * this->getFramebufferSize();
  }

  /**
   * @brief Get the framebuffer size
   *
   * @return Framebuffer size
   */
  inline size_t getFramebufferSize() { return this->m_framebuffer.fb_size; }

  /**
   * @brief Get the number of framebuffers in use
   *
   * @return Number of framebuffers
   */
  inline size_t getFramebufferCount() { return this->m_framebuffer.num_fbs; }

  /**
   * @brief Get the currently used framebuffer's slot
   *
   * @return Slot
   */
  inline u8 getCurrentFramebufferSlot() { return this->m_window.cur_slot; }

  /**
   * @brief Get the next framebuffer's slot
   *
   * @return Next slot
   */
  inline u8 getNextFramebufferSlot()
  {
    return (this->getCurrentFramebufferSlot() + 1)
        % this->getFramebufferCount();
  }

  /**
   * @brief Waits for the vsync event
   *
   */
  inline void waitForVSync() { eventWait(&this->m_vsyncEvent, UINT64_MAX); }

  /**
   * @brief Decodes a x and y coordinate into a offset into the swizzled
   * framebuffer
   *
   * @param x X pos
   * @param y Y Pos
   * @return Offset
   */
  const u32 getPixelOffset(u32 x, u32 y)
  {
    if (this->m_scissoring) {
      if (x < this->m_scissorBounds[0] || y < this->m_scissorBounds[1]
          || x > this->m_scissorBounds[0] + this->m_scissorBounds[2]
          || y > this->m_scissorBounds[1] + this->m_scissorBounds[3])
        return cfg::FramebufferWidth * cfg::FramebufferHeight * 2 + 1;
    }

    u32 tmpPos = ((y & 127) / 16) + (x / 32 * 8)
        + ((y / 16 / 8) * (((cfg::FramebufferWidth / 2) / 16 * 8)));
    tmpPos *= 16 * 16 * 4;

    tmpPos += ((y % 16) / 8) * 512 + ((x % 32) / 16) * 256 + ((y % 8) / 2) * 64
        + ((x % 16) / 8) * 32 + (y % 2) * 16 + (x % 8) * 2;

    return tmpPos / 2;
  }

  /**
   * @brief Initializes the renderer and layers
   *
   */
  void init()
  {
    cfg::LayerPosX = 0;
    cfg::LayerPosY = 0;
    cfg::FramebufferWidth = framebufferWidth;
    cfg::FramebufferHeight = framebufferHeight;
    cfg::LayerWidth = cfg::ScreenWidth
        * (float(cfg::FramebufferWidth) / float(cfg::LayerMaxWidth));
    cfg::LayerHeight = cfg::ScreenHeight
        * (float(cfg::FramebufferHeight) / float(cfg::LayerMaxHeight));

    if (this->m_initialized)
      return;

    tsl::hlp::doWithSmSession(
        [this]
        {
          ASSERT_FATAL(viInitialize(ViServiceType_Manager));
          ASSERT_FATAL(viOpenDefaultDisplay(&this->m_display));
          ASSERT_FATAL(
              viGetDisplayVsyncEvent(&this->m_display, &this->m_vsyncEvent));
          ASSERT_FATAL(viCreateManagedLayer(&this->m_display,
                                            static_cast<ViLayerFlags>(0),
                                            0,
                                            &__nx_vi_layer_id));
          ASSERT_FATAL(viCreateLayer(&this->m_display, &this->m_layer));
          ASSERT_FATAL(
              viSetLayerScalingMode(&this->m_layer, ViScalingMode_FitToLayer));

          if (s32 layerZ = 0;
              R_SUCCEEDED(viGetZOrderCountMax(&this->m_display, &layerZ))
              && layerZ > 0)
            ASSERT_FATAL(viSetLayerZ(&this->m_layer, layerZ));

          ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer,
                                                   ViLayerStack_Default));
          ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer,
                                                   ViLayerStack_Screenshot));
          ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer,
                                                   ViLayerStack_Recording));
          ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer,
                                                   ViLayerStack_Arbitrary));
          ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer,
                                                   ViLayerStack_LastFrame));
          ASSERT_FATAL(
              tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Null));
          ASSERT_FATAL(tsl::hlp::viAddToLayerStack(
              &this->m_layer, ViLayerStack_ApplicationForDebug));
          ASSERT_FATAL(
              tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Lcd));

          ASSERT_FATAL(viSetLayerSize(
              &this->m_layer, cfg::LayerWidth, cfg::LayerHeight));
          ASSERT_FATAL(viSetLayerPosition(
              &this->m_layer, cfg::LayerPosX, cfg::LayerPosY));
          ASSERT_FATAL(nwindowCreateFromLayer(&this->m_window, &this->m_layer));
          ASSERT_FATAL(framebufferCreate(&this->m_framebuffer,
                                         &this->m_window,
                                         cfg::FramebufferWidth,
                                         cfg::FramebufferHeight,
                                         PIXEL_FORMAT_RGBA_4444,
                                         2));
          ASSERT_FATAL(this->initFonts());
        });

    this->m_initialized = true;
  }

  /**
   * @brief Exits the renderer and layer
   *
   */
  void exit()
  {
    if (!this->m_initialized)
      return;

    framebufferClose(&this->m_framebuffer);
    nwindowClose(&this->m_window);
    viDestroyManagedLayer(&this->m_layer);
    viCloseDisplay(&this->m_display);
    eventClose(&this->m_vsyncEvent);
    viExit();
  }

  /**
   * @brief Initializes Nintendo's shared fonts. Default and Extended
   *
   * @return Result
   */
  Result initFonts()
  {
    Result res;

    static PlFontData stdFontData, extFontData;

    // Nintendo's default font
    if (R_FAILED(res = plGetSharedFontByType(&stdFontData,
                                             PlSharedFontType_Standard)))
      return res;

    u8* fontBuffer = reinterpret_cast<u8*>(stdFontData.address);
    stbtt_InitFont(&this->m_stdFont,
                   fontBuffer,
                   stbtt_GetFontOffsetForIndex(fontBuffer, 0));

    // Nintendo's extended font containing a bunch of icons
    if (R_FAILED(res = plGetSharedFontByType(&extFontData,
                                             PlSharedFontType_NintendoExt)))
      return res;

    fontBuffer = reinterpret_cast<u8*>(extFontData.address);
    stbtt_InitFont(&this->m_extFont,
                   fontBuffer,
                   stbtt_GetFontOffsetForIndex(fontBuffer, 0));

    return res;
  }

  /**
   * @brief Start a new frame
   * @warning Don't call this more than once before calling \ref endFrame
   */
  inline void startFrame()
  {
    this->m_currentFramebuffer =
        framebufferBegin(&this->m_framebuffer, nullptr);
  }

  /**
   * @brief End the current frame
   * @warning Don't call this before calling \ref startFrame once
   */
  inline void endFrame()
  {
    std::memcpy(this->getNextFramebuffer(),
                this->getCurrentFramebuffer(),
                this->getFramebufferSize());
    svcSleepThread(1000 * 1000 * 1000 / TeslaFPS);
    this->waitForVSync();
    framebufferEnd(&this->m_framebuffer);

    this->m_currentFramebuffer = nullptr;
  }

  /**
   * @brief Draws a single font glyph
   *
   * @param codepoint Unicode codepoint to draw
   * @param x X pos
   * @param y Y pos
   * @param color Color
   * @param font STB Font to use
   * @param fontSize Font size
   */
  inline void drawGlyph(s32 codepoint,
                        s32 x,
                        s32 y,
                        Color color,
                        stbtt_fontinfo* font,
                        float fontSize)
  {
    int width = 10, height = 10;

    u8* glyphBmp = stbtt_GetCodepointBitmap(
        font, fontSize, fontSize, codepoint, &width, &height, nullptr, nullptr);

    if (glyphBmp == nullptr)
      return;

    for (s16 bmpY = 0; bmpY < height; bmpY++) {
      for (s16 bmpX = 0; bmpX < width; bmpX++) {
        Color tmpColor = color;
        tmpColor.a =
            (glyphBmp[width * bmpY + bmpX] >> 4) * (float(tmpColor.a) / 0xF);
        this->setPixelBlendSrc(x + bmpX, y + bmpY, tmpColor);
      }
    }

    std::free(glyphBmp);
  }

  inline void setLayerPosImpl(u32 x, u32 y)
  {
    cfg::LayerPosX = x;
    cfg::LayerPosY = y;
    ASSERT_FATAL(
        viSetLayerPosition(&this->m_layer, cfg::LayerPosX, cfg::LayerPosY));
  }
};

}


#endif  // LIBNIKOLA_GFX_HPP
