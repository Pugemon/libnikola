//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_GFX_HPP
#define LIBNIKOLA_GFX_HPP

#include <string>

#include <switch/display/framebuffer.h>
#include <switch/kernel/event.h>
#include <switch/services/vi.h>
#include <switch/types.h>

#include "../stb_truetype.h"

namespace tsl
{
class Overlay;
} // namespace tsl

namespace tsl::gfx
{

extern "C" u64 __nx_vi_layer_id;

bool isValidHexColor(const std::string& hexColor);

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

Color RGB888(std::string hexColor, std::string defaultHexColor = "#FFFFFF");

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
  static Color a(const Color& c);

  /**
   * @brief Enables scissoring, discarding of any draw outside the given
   * boundaries
   *
   * @param x x pos
   * @param y y pos
   * @param w Width
   * @param h Height
   */
  void enableScissoring(u16 x, u16 y, u16 w, u16 h);

  /**
   * @brief Disables scissoring
   */
  void disableScissoring();

  // Drawing functions

  /**
   * @brief Draw a single pixel onto the screen
   *
   * @param x X pos
   * @param y Y pos
   * @param color Color
   */
  void setPixel(s16 x, s16 y, Color color);

  /**
   * @brief Blends two colors
   *
   * @param src Source color
   * @param dst Destination color
   * @param alpha Opacity
   * @return Blended color
   */
  u8 blendColor(u8 src, u8 dst, u8 alpha);

  /**
   * @brief Draws a single source blended pixel onto the screen
   *
   * @param x X pos
   * @param y Y pos
   * @param color Color
   */
  void setPixelBlendSrc(s16 x, s16 y, Color color);

  /**
   * @brief Draws a single destination blended pixel onto the screen
   *
   * @param x X pos
   * @param y Y pos
   * @param color Color
   */
  void setPixelBlendDst(s16 x, s16 y, Color color);

  /**
   * @brief Draws a rectangle of given sizes
   *
   * @param x X pos
   * @param y Y pos
   * @param w Width
   * @param h Height
   * @param color Color
   */
  void drawRect(s16 x, s16 y, s16 w, s16 h, Color color);

  /**
   * @brief Draws a rectangle of given sizes with empty filling
   *
   * @param x X pos
   * @param y Y pos
   * @param w Width
   * @param h Height
   * @param color Color
   */
  void drawEmptyRect(s16 x, s16 y, s16 w, s16 h, Color color);

  /**
   * @brief Draws a line
   *
   * @param x0 Start X pos
   * @param y0 Start Y pos
   * @param x1 End X pos
   * @param y1 End Y pos
   * @param color Color
   */
  void drawLine(s16 x0, s16 y0, s16 x1, s16 y1, Color color);

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
  void drawDashedLine(
      s16 x0, s16 y0, s16 x1, s16 y1, s16 line_width, Color color);

  /**
   * @brief Draws a RGBA8888 bitmap from memory
   *
   * @param x X start position
   * @param y Y start position
   * @param w Bitmap width
   * @param h Bitmap height
   * @param bmp Pointer to bitmap data
   */
  void drawBitmap(s16 x, s16 y, s16 w, s16 h, const u8* bmp);

  /**
   * @brief Fills the entire layer with a given color
   *
   * @param color Color
   */
  void fillScreen(Color color);

  /**
   * @brief Clears the layer (With transparency)
   *
   */
  void clearScreen();

  void setLayerPos(u16 x, u16 y);

  static Renderer& getRenderer();

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
                                 Color color);

private:
  Renderer() {}

  /**
   * @brief Provides access to the single instance of the Renderer.
   *
   * This method returns a reference to the sole instance of the Renderer.
   * If the instance does not yet exist, it will be created at the time of the
   * first call.
   *
   * @return Renderer& Reference to the single instance of the Renderer.
   */
  static Renderer& get();

  /**
   * @brief Sets the opacity of the layer
   *
   * @param opacity Opacity
   */
  static void setOpacity(float opacity);

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
  void* getCurrentFramebuffer();

  /**
   * @brief Get the next framebuffer address
   *
   * @return Next framebuffer address
   */
  void* getNextFramebuffer();

  /**
   * @brief Get the framebuffer size
   *
   * @return Framebuffer size
   */
  size_t getFramebufferSize();

  /**
   * @brief Get the number of framebuffers in use
   *
   * @return Number of framebuffers
   */
  size_t getFramebufferCount();

  /**
   * @brief Get the currently used framebuffer's slot
   *
   * @return Slot
   */
  u8 getCurrentFramebufferSlot();

  /**
   * @brief Get the next framebuffer's slot
   *
   * @return Next slot
   */
  u8 getNextFramebufferSlot();

  /**
   * @brief Waits for the vsync event
   *
   */
  void waitForVSync();

  /**
   * @brief Decodes a x and y coordinate into a offset into the swizzled
   * framebuffer
   *
   * @param x X pos
   * @param y Y Pos
   * @return Offset
   */
  const u32 getPixelOffset(u32 x, u32 y);

  /**
   * @brief Initializes the renderer and layers
   *
   */
  void init();

  /**
   * @brief Exits the renderer and layer
   *
   */
  void exit();

  /**
   * @brief Initializes Nintendo's shared fonts. Default and Extended
   *
   * @return Result
   */
  Result initFonts();

  /**
   * @brief Start a new frame
   * @warning Don't call this more than once before calling \ref endFrame
   */
  void startFrame();

  /**
   * @brief End the current frame
   * @warning Don't call this before calling \ref startFrame once
   */
  void endFrame();

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
  void drawGlyph(s32 codepoint,
                 s32 x,
                 s32 y,
                 Color color,
                 stbtt_fontinfo* font,
                 float fontSize);

  void setLayerPosImpl(u16 x, u16 y);
};

}  // namespace tsl::gfx

#endif  // LIBNIKOLA_GFX_HPP
