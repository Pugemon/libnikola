//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_ELM_HPP
#define LIBNIKOLA_ELM_HPP

#include "gfx.hpp"
#include <algorithm>
#include <cstring>
#include <cwctype>
#include <string>

#include <cmath>
#include <cstdlib>
#include <strings.h>
#include <switch.h>

namespace nikola::tsl::elm
{

/**
 * @brief The top level Element of the libtesla UI library
 * @note When creating your own elements, extend from this or one of it's sub
 * classes
 */
class Element
{
public:
  Element() {}
  virtual ~Element() {}

  std::string highlightColor1Str = parseValueFromIniSection(
      "/config/ultrahand/theme.ini", "theme", "highlight_color_1");
  std::string highlightColor2Str = parseValueFromIniSection(
      "/config/ultrahand/theme.ini", "theme", "highlight_color_2");

  tsl::gfx::Color highlightColor1 =
      tsl::gfx::RGB888(highlightColor1Str, "#2288CC");
  tsl::gfx::Color highlightColor2 =
      tsl::gfx::RGB888(highlightColor2Str, "#88FFFF");

  /**
   * @brief Handles focus requesting
   * @note This function should return the element to focus.
   *       When this element should be focused, return `this`.
   *       When one of it's child should be focused, return
   * `this->child->requestFocus(oldFocus, direction)` When this element is not
   * focusable, return `nullptr`
   *
   * @param oldFocus Previously focused element
   * @param direction Direction in which focus moved. \ref FocusDirection::None
   * is passed for the initial load
   * @return Element to focus
   */
  virtual Element* requestFocus(Element* oldFocus, FocusDirection direction)
  {
    return nullptr;
  }

  /**
   * @brief Function called when a joycon button got pressed
   *
   * @param keys Keys pressed in the last frame
   * @return true when button press has been consumed
   * @return false when button press should be passed on to the parent
   */
  virtual bool onClick(u64 keys) { return m_clickListener(keys); }

  /**
   * @brief Function called when the element got touched
   * @todo Not yet implemented
   *
   * @param x X pos
   * @param y Y pos
   * @return true when touch input has been consumed
   * @return false when touch input should be passed on to the parent
   */
  virtual bool onTouch(u32 x, u32 y) { return false; }

  /**
   * @brief Called once per frame to draw the element
   * @warning Do not call this yourself. Use \ref Element::frame(gfx::Renderer
   * *renderer)
   *
   * @param renderer Renderer
   */
  virtual void draw(gfx::Renderer* renderer) = 0;

  /**
   * @brief Called when the underlying Gui gets created and after calling \ref
   * Gui::invalidate() to calculate positions and boundaries of the element
   * @warning Do not call this yourself. Use \ref Element::invalidate()
   *
   * @param parentX Parent X pos
   * @param parentY Parent Y pos
   * @param parentWidth Parent Width
   * @param parentHeight Parent Height
   */
  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) = 0;

  /**
   * @brief Draws highlighting and the element itself
   * @note When drawing children of a element in \ref
   * Element::draw(gfx::Renderer *renderer), use `this->child->frame(renderer)`
   * instead of calling draw directly
   *
   * @param renderer
   */
  virtual void frame(gfx::Renderer* renderer) final
  {
    if (this->m_focused)
      this->drawHighlight(renderer);

    this->draw(renderer);
  }

  /**
   * @brief Forces a layout recreation of a element
   *
   */
  virtual void invalidate() final
  {
    const auto& parent = this->getParent();

    if (parent == nullptr)
      this->layout(0, 0, cfg::FramebufferWidth, cfg::FramebufferHeight);
    else
      this->layout(parent->getX(),
                   parent->getY(),
                   parent->getWidth(),
                   parent->getHeight());
  }

  /**
   * @brief Shake the highlight in the given direction to signal that the focus
   * cannot move there
   *
   * @param direction Direction to shake highlight in
   */
  virtual void shakeHighlight(FocusDirection direction) final
  {
    this->m_highlightShaking = true;
    this->m_highlightShakingDirection = direction;
    this->m_highlightShakingStartTime = std::chrono::system_clock::now();
  }

  /**
   * @brief Draws the blue boarder when a element is highlighted
   * @note Override this if you have a element that e.g requires a
   * non-rectangular focus
   *
   * @param renderer Renderer
   */
  virtual void drawHighlight(gfx::Renderer* renderer)
  {
    // Get the current time
    auto currentTime = std::chrono::system_clock::now();
    auto timeInSeconds =
        std::chrono::duration<double>(currentTime.time_since_epoch()).count();

    // Calculate the progress for one full sine wave per second
    const double cycleDuration = 1.0;  // 1 second for one full sine wave
    double timeCounter = fmod(timeInSeconds, cycleDuration);
    float progress = (std::sin(2 * M_PI * timeCounter / cycleDuration) + 1) / 2;

    tsl::gfx::Color highlightColor = {
        static_cast<u8>((highlightColor1.r - highlightColor2.r) * progress
                        + highlightColor2.r),
        static_cast<u8>((highlightColor1.g - highlightColor2.g) * progress
                        + highlightColor2.g),
        static_cast<u8>((highlightColor1.b - highlightColor2.b) * progress
                        + highlightColor2.b),
        0xF};
    s32 x = 0, y = 0;

    if (this->m_highlightShaking) {
      auto t = (std::chrono::system_clock::now()
                - this->m_highlightShakingStartTime);
      if (t >= 100ms)
        this->m_highlightShaking = false;
      else {
        s32 amplitude = std::rand() % 5 + 5;

        switch (this->m_highlightShakingDirection) {
          case FocusDirection::Up:
            y -= shakeAnimation(t, amplitude);
            break;
          case FocusDirection::Down:
            y += shakeAnimation(t, amplitude);
            break;
          case FocusDirection::Left:
            x -= shakeAnimation(t, amplitude);
            break;
          case FocusDirection::Right:
            x += shakeAnimation(t, amplitude);
            break;
          default:
            break;
        }

        x = std::clamp(x, -amplitude, amplitude);
        y = std::clamp(y, -amplitude, amplitude);
      }
    }

    renderer->drawRect(
        this->m_x, this->m_y, this->m_width, this->m_height, a(0xF000));

    renderer->drawRect(this->m_x + x - 4,
                       this->m_y + y - 4,
                       this->m_width + 8,
                       4,
                       a(highlightColor));
    renderer->drawRect(this->m_x + x - 4,
                       this->m_y + y + this->m_height,
                       this->m_width + 8,
                       4,
                       a(highlightColor));
    renderer->drawRect(
        this->m_x + x - 4, this->m_y + y, 4, this->m_height, a(highlightColor));
    renderer->drawRect(this->m_x + x + this->m_width,
                       this->m_y + y,
                       4,
                       this->m_height,
                       a(highlightColor));
  }

  /**
   * @brief Sets the boundaries of this view
   *
   * @param x Start X pos
   * @param y Start Y pos
   * @param width Width
   * @param height Height
   */
  virtual void setBoundaries(u16 x, u16 y, u16 width, u16 height) final
  {
    this->m_x = x;
    this->m_y = y;
    this->m_width = width;
    this->m_height = height;
  }

  /**
   * @brief Adds a click listener to the element
   *
   * @param clickListener Click listener called with keys that were pressed last
   * frame. Callback should return true if keys got consumed
   */
  virtual void setClickListener(std::function<bool(u64 keys)> clickListener)
  {
    this->m_clickListener = clickListener;
  }

  /**
   * @brief Gets the element's X position
   *
   * @return X position
   */
  virtual inline u16 getX() final { return this->m_x; }
  /**
   * @brief Gets the element's Y position
   *
   * @return Y position
   */
  virtual inline u16 getY() final { return this->m_y; }
  /**
   * @brief Gets the element's Width
   *
   * @return Width
   */
  virtual inline u16 getWidth() final { return this->m_width; }
  /**
   * @brief Gets the element's Height
   *
   * @return Height
   */
  virtual inline u16 getHeight() final { return this->m_height; }

  /**
   * @brief Sets the element's parent
   * @note This is required to handle focus and button downpassing properly
   *
   * @param parent Parent
   */
  virtual inline void setParent(Element* parent) final
  {
    this->m_parent = parent;
  }

  /**
   * @brief Get the element's parent
   *
   * @return Parent
   */
  virtual inline Element* getParent() final { return this->m_parent; }

  /**
   * @brief Marks this element as focused or unfocused to draw the highlight
   *
   * @param focused Focused
   */
  virtual inline void setFocused(bool focused) { this->m_focused = focused; }

protected:
  constexpr static inline auto a = &gfx::Renderer::a;

private:
  friend class Gui;

  u16 m_x = 0, m_y = 0, m_width = 0, m_height = 0;
  Element* m_parent = nullptr;
  bool m_focused = false;

  std::function<bool(u64 keys)> m_clickListener = [](u64) { return false; };

  // Highlight shake animation
  bool m_highlightShaking = false;
  std::chrono::system_clock::time_point m_highlightShakingStartTime;
  FocusDirection m_highlightShakingDirection;

  /**
   * @brief Shake animation callculation based on a damped sine wave
   *
   * @param t Passed time
   * @param a Amplitude
   * @return Damped sine wave output
   */
  int shakeAnimation(std::chrono::system_clock::duration t, float a)
  {
    float w = 0.2F;
    float tau = 0.05F;

    int t_ = t.count() / 1'000'000;

    return roundf(a * exp(-(tau * t_) * sin(w * t_)));
  }
};

/**
 * @brief The base frame which can contain another view
 *
 */
class OverlayFrame : public Element
{
public:
  std::string defaultTextColorStr = parseValueFromIniSection(
      "/config/ultrahand/theme.ini", "theme", "text_color");
  tsl::gfx::Color defaultTextColor = tsl::gfx::RGB888(defaultTextColorStr);
  std::string clockColorStr = parseValueFromIniSection(
      "/config/ultrahand/theme.ini", "theme", "clock_color");
  tsl::gfx::Color clockColor = tsl::gfx::RGB888(clockColorStr);
  std::string batteryColorStr = parseValueFromIniSection(
      "/config/ultrahand/theme.ini", "theme", "battery_color");
  tsl::gfx::Color batteryColor = tsl::gfx::RGB888(batteryColorStr);

  /**
   * @brief Constructor
   *
   * @param title Name of the Overlay drawn bolt at the top
   * @param subtitle Subtitle drawn bellow the title e.g version number
   */
  OverlayFrame(std::string title, std::string subtitle)
      : Element()
      , m_title(title)
      , m_subtitle(subtitle)
  {
  }
  virtual ~OverlayFrame()
  {
    if (this->m_contentElement != nullptr)
      delete this->m_contentElement;
  }

  virtual void draw(gfx::Renderer* renderer) override
  {
    renderer->fillScreen(a({0x0, 0x0, 0x0, alphabackground}));

    renderer->drawString(
        this->m_title.c_str(), false, 20, 50, 30, a(defaultTextColor));
    renderer->drawString(
        this->m_subtitle.c_str(), false, 20, 70, 15, a(defaultTextColor));

    if (FullMode == true)
      renderer->drawRect(15,
                         720 - 73,
                         tsl::cfg::FramebufferWidth - 30,
                         1,
                         a(defaultTextColor));
    if (!deactivateOriginalFooter)
      renderer->drawString("\uE0E1  Back     \uE0E0  OK",
                           false,
                           30,
                           693,
                           23,
                           a(defaultTextColor));

    if (this->m_contentElement != nullptr)
      this->m_contentElement->frame(renderer);
  }

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override
  {
    this->setBoundaries(parentX, parentY, parentWidth, parentHeight);

    if (this->m_contentElement != nullptr) {
      this->m_contentElement->setBoundaries(
          parentX + 35,
          parentY + 140,
          parentWidth - 85,
          parentHeight - 73 - 105);  // CUSTOM MODIFICATION
      this->m_contentElement->invalidate();
    }
  }

  virtual Element* requestFocus(Element* oldFocus,
                                FocusDirection direction) override
  {
    if (this->m_contentElement != nullptr)
      return this->m_contentElement->requestFocus(oldFocus, direction);
    else
      return nullptr;
  }

  /**
   * @brief Sets the content of the frame
   *
   * @param content Element
   */
  virtual void setContent(Element* content) final
  {
    if (this->m_contentElement != nullptr)
      delete this->m_contentElement;

    this->m_contentElement = content;

    if (content != nullptr) {
      this->m_contentElement->setParent(this);
      this->invalidate();
    }
  }

protected:
  Element* m_contentElement = nullptr;

  std::string m_title, m_subtitle;
};

/**
 * @brief Single color rectangle element mainly used for debugging to visualize
 * boundaries
 *
 */
class DebugRectangle : public Element
{
public:
  /**
   * @brief Constructor
   *
   * @param color Color of the rectangle
   */
  DebugRectangle(gfx::Color color)
      : Element()
      , m_color(color)
  {
  }
  virtual ~DebugRectangle() {}

  virtual void draw(gfx::Renderer* renderer) override
  {
    renderer->drawRect(this->getX(),
                       this->getY(),
                       this->getWidth(),
                       this->getHeight(),
                       a(this->m_color));
  }

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override
  {
  }

private:
  gfx::Color m_color;
};

/**
 * @brief A item that goes into a list
 *
 */
class ListItem : public Element
{
public:
  std::string defaultTextColorStr = parseValueFromIniSection(
      "/config/ultrahand/theme.ini", "theme", "text_color");
  tsl::gfx::Color defaultTextColor = tsl::gfx::RGB888(defaultTextColorStr);

  /**
   * @brief Constructor
   *
   * @param text Initial description text
   */
  ListItem(std::string text)
      : Element()
      , m_text(text)
  {
  }
  virtual ~ListItem() {}

  virtual void draw(gfx::Renderer* renderer) override
  {
    if (this->m_valueWidth == 0) {
      auto [width, height] =
          renderer->drawString(this->m_value.c_str(),
                               false,
                               0,
                               0,
                               20,
                               tsl::style::color::ColorTransparent);
      this->m_valueWidth = width;
    }

    renderer->drawRect(this->getX(),
                       this->getY(),
                       this->getWidth(),
                       1,
                       a({0x4, 0x4, 0x4, 0xF}));
    renderer->drawRect(this->getX(),
                       this->getY() + this->getHeight(),
                       this->getWidth(),
                       1,
                       a({0x0, 0x0, 0x0, 0xD}));

    renderer->drawString(this->m_text.c_str(),
                         false,
                         this->getX() + 20,
                         this->getY() + 45,
                         23,
                         a(defaultTextColor));

    renderer->drawString(
        this->m_value.c_str(),
        false,
        this->getX() + this->getWidth() - this->m_valueWidth - 20,
        this->getY() + 45,
        20,
        this->m_faint ? a({0x6, 0x6, 0x6, 0xF}) : a({0x5, 0xC, 0xA, 0xF}));
  }

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override
  {
  }

  virtual Element* requestFocus(Element* oldFocus,
                                FocusDirection direction) override
  {
    return this;
  }

  /**
   * @brief Sets the left hand description text of the list item
   *
   * @param text Text
   */
  virtual inline void setText(std::string text) final { this->m_text = text; }

  /**
   * @brief Sets the right hand value text of the list item
   *
   * @param value Text
   * @param faint Should the text be drawn in a glowing green or a faint gray
   */
  virtual inline void setValue(std::string value, bool faint = false)
  {
    this->m_value = value;
    this->m_faint = faint;
    this->m_valueWidth = 0;
  }

protected:
  std::string m_text;
  std::string m_value = "";
  bool m_faint = false;

  u16 m_valueWidth = 0;
};

/**
 * @brief A toggleable list item that changes the state from On to Off when the
 * A button gets pressed
 *
 */
class ToggleListItem : public ListItem
{
public:
  /**
   * @brief Constructor
   *
   * @param text Initial description text
   * @param initialState Is the toggle set to On or Off initially
   * @param onValue Value drawn if the toggle is on
   * @param offValue Value drawn if the toggle is off
   */
  ToggleListItem(std::string text,
                 bool initialState,
                 std::string onValue = "On",
                 std::string offValue = "Off")
      : ListItem(text)
      , m_state(initialState)
      , m_onValue(onValue)
      , m_offValue(offValue)
  {
    this->setState(this->m_state);
  }

  virtual ~ToggleListItem() {}

  virtual bool onClick(u64 keys)
  {
    if (keys & KEY_A) {
      this->m_state = !this->m_state;

      this->setState(this->m_state);
      this->m_stateChangedListener(this->m_state);

      return true;
    }

    return false;
  }

  /**
   * @brief Gets the current state of the toggle
   *
   * @return State
   */
  virtual inline bool getState() { return this->m_state; }

  /**
   * @brief Sets the current state of the toggle. Updates the Value
   *
   * @param state State
   */
  virtual void setState(bool state)
  {
    this->m_state = state;

    if (state)
      this->setValue(this->m_onValue, false);
    else
      this->setValue(this->m_offValue, true);
  }

  /**
   * @brief Adds a listener that gets called whenever the state of the toggle
   * changes
   *
   * @param stateChangedListener Listener with the current state passed in as
   * parameter
   */
  void setStateChangedListener(std::function<void(bool)> stateChangedListener)
  {
    this->m_stateChangedListener = stateChangedListener;
  }

protected:
  bool m_state = true;
  std::string m_onValue, m_offValue;

  std::function<void(bool)> m_stateChangedListener = [](bool) {};
};

/**
 * @brief A List containing list items
 *
 */
class List : public Element
{
public:
  /**
   * @brief Constructor
   *
   * @param entriesShown Amount of items displayed in the list at once before
   * scrolling starts
   */
  List(u16 entriesShown = 6)
      : Element()
      , m_entriesShown(entriesShown)
  {
  }
  virtual ~List()
  {
    for (auto& item : this->m_items)
      delete item.element;
  }

  virtual void draw(gfx::Renderer* renderer) override
  {
    u16 i = 0;
    for (auto& entry : this->m_items) {
      if (i >= this->m_offset && i < this->m_offset + this->m_entriesShown) {
        entry.element->frame(renderer);
      }
      i++;
    }
  }

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override
  {
    u16 y = this->getY();
    u16 i = 0;
    for (auto& entry : this->m_items) {
      if (i >= this->m_offset && i < this->m_offset + this->m_entriesShown) {
        entry.element->setBoundaries(
            this->getX(), y, this->getWidth(), entry.height);
        entry.element->invalidate();
        y += entry.height;
      }
      i++;
    }
  }

  /**
   * @brief Adds a new item to the list
   *
   * @param element Element to add
   * @param height Height of the element. Don't set this parameter for libtesla
   * to try and figure out the size based on the type
   */
  virtual void addItem(Element* element, u16 height = 0) final
  {
    if (height == 0) {
      if (dynamic_cast<ListItem*>(element) != nullptr)
        height = tsl::style::ListItemDefaultHeight;
    }

    if (element != nullptr && height > 0) {
      element->setParent(this);
      this->m_items.push_back({element, height});
      this->invalidate();
    }

    if (this->m_items.size() == 1)
      this->requestFocus(nullptr, FocusDirection::None);
  }

  /**
   * @brief Removes all children from the list
   */
  virtual void clear() final
  {
    for (auto& item : this->m_items)
      delete item.element;

    this->m_items.clear();
  }

  virtual Element* requestFocus(Element* oldFocus,
                                FocusDirection direction) override
  {
    if (this->m_items.size() == 0)
      return nullptr;

    auto it = std::find(this->m_items.begin(), this->m_items.end(), oldFocus);

    if (it == this->m_items.end() || direction == FocusDirection::None)
      return this->m_items[0].element;

    if (direction == FocusDirection::Up) {
      if (it == this->m_items.begin())
        return this->m_items[0].element;
      else {
        // old focus on the second item, and has offset
        if (oldFocus == (this->m_items.begin() + this->m_offset + 1)->element) {
          if (this->m_offset > 0) {
            this->m_offset--;
            this->invalidate();
          }
        }
        return (it - 1)->element;
      }
    } else if (direction == FocusDirection::Down) {
      if (it == (this->m_items.end() - 1)) {
        return this->m_items[this->m_items.size() - 1].element;
      } else {
        // old focus on second to last item, and has more items hidden
        if (oldFocus
            == (this->m_items.begin() + this->m_offset + this->m_entriesShown
                - 2)
                   ->element)
        {
          if (this->m_items.size() > this->m_offset + this->m_entriesShown) {
            this->m_offset++;
            this->invalidate();
          }
        }
        return (it + 1)->element;
      }
    }

    return it->element;
  }

protected:
  struct ListEntry
  {
    Element* element;
    u16 height;

    bool operator==(Element* other) { return this->element == other; }
  };

  std::vector<ListEntry> m_items;
  u16 m_focusedElement = 0;

  u16 m_offset = 0;
  u16 m_entriesShown = 5;
};

/**
 * @brief A Element that exposes the renderer directly to draw custom views
 * easily
 */
class CustomDrawer : public Element
{
public:
  /**
   * @brief Constructor
   * @note This element should only be used to draw static things the user
   * cannot interact with e.g info text, images, etc.
   *
   * @param renderFunc Callback that will be called once every frame to draw
   * this view
   */
  CustomDrawer(std::function<void(gfx::Renderer*, u16 x, u16 y, u16 w, u16 h)>
                   renderFunc)
      : Element()
      , m_renderFunc(renderFunc)
  {
  }
  virtual ~CustomDrawer() {}

  virtual void draw(gfx::Renderer* renderer) override
  {
    this->m_renderFunc(renderer,
                       this->getX(),
                       this->getY(),
                       this->getWidth(),
                       this->getHeight());
  }

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override
  {
  }

private:
  std::function<void(gfx::Renderer*, u16 x, u16 y, u16 w, u16 h)> m_renderFunc;
};

}

#endif  // LIBNIKOLA_ELM_HPP
