//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_ELM_HPP
#define LIBNIKOLA_ELM_HPP

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cwctype>
#include <functional>
#include <string>

#include <strings.h>
#include <switch.h>

#include "gfx.hpp"
#include "nikola/utils/ini_funcs.hpp"
#include "focus_direction.hpp"


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

  gfx::Color highlightColor1 =
      gfx::RGB888(highlightColor1Str, "#2288CC");
  gfx::Color highlightColor2 =
      gfx::RGB888(highlightColor2Str, "#88FFFF");

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
  virtual Element* requestFocus(Element* oldFocus, FocusDirection direction);

  /**
   * @brief Function called when a joycon button got pressed
   *
   * @param keys Keys pressed in the last frame
   * @return true when button press has been consumed
   * @return false when button press should be passed on to the parent
   */
  virtual bool onClick(u64 keys);

  /**
   * @brief Function called when the element got touched
   * @todo Not yet implemented
   *
   * @param x X pos
   * @param y Y pos
   * @return true when touch input has been consumed
   * @return false when touch input should be passed on to the parent
   */
  virtual bool onTouch(u32 x, u32 y);

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
  virtual void frame(gfx::Renderer* renderer) final;

  /**
   * @brief Forces a layout recreation of a element
   *
   */
  virtual void invalidate() final;

  /**
   * @brief Shake the highlight in the given direction to signal that the focus
   * cannot move there
   *
   * @param direction Direction to shake highlight in
   */
  virtual void shakeHighlight(FocusDirection direction) final;

  /**
   * @brief Draws the blue boarder when a element is highlighted
   * @note Override this if you have a element that e.g requires a
   * non-rectangular focus
   *
   * @param renderer Renderer
   */
  virtual void drawHighlight(gfx::Renderer* renderer);

  /**
   * @brief Sets the boundaries of this view
   *
   * @param x Start X pos
   * @param y Start Y pos
   * @param width Width
   * @param height Height
   */
  virtual void setBoundaries(u16 x, u16 y, u16 width, u16 height) final;

  /**
   * @brief Adds a click listener to the element
   *
   * @param clickListener Click listener called with keys that were pressed last
   * frame. Callback should return true if keys got consumed
   */
  virtual void setClickListener(std::function<bool(u64 keys)> clickListener);

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
  virtual inline void setParent(Element* parent) final;

  /**
   * @brief Get the element's parent
   *
   * @return Parent
   */
  virtual inline Element* getParent() final;

  /**
   * @brief Marks this element as focused or unfocused to draw the highlight
   *
   * @param focused Focused
   */
  virtual inline void setFocused(bool focused);

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
  int shakeAnimation(std::chrono::system_clock::duration t, float a);
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
  gfx::Color defaultTextColor = gfx::RGB888(defaultTextColorStr);
  std::string clockColorStr = parseValueFromIniSection(
      "/config/ultrahand/theme.ini", "theme", "clock_color");
  gfx::Color clockColor = gfx::RGB888(clockColorStr);
  std::string batteryColorStr = parseValueFromIniSection(
      "/config/ultrahand/theme.ini", "theme", "battery_color");
  gfx::Color batteryColor = gfx::RGB888(batteryColorStr);

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
  virtual ~OverlayFrame();

  virtual void draw(gfx::Renderer* renderer) override;

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override;

  virtual Element* requestFocus(Element* oldFocus,
                                FocusDirection direction) override;

  /**
   * @brief Sets the content of the frame
   *
   * @param content Element
   */
  virtual void setContent(Element* content) final;

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

  virtual void draw(gfx::Renderer* renderer) override;

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override {}

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
  gfx::Color defaultTextColor = gfx::RGB888(defaultTextColorStr);

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

  virtual void draw(gfx::Renderer* renderer) override;

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override
  {
  }

  virtual Element* requestFocus(Element* oldFocus,
                                FocusDirection direction) override;

  /**
   * @brief Sets the left hand description text of the list item
   *
   * @param text Text
   */
  virtual inline void setText(std::string text) final;

  /**
   * @brief Sets the right hand value text of the list item
   *
   * @param value Text
   * @param faint Should the text be drawn in a glowing green or a faint gray
   */
  virtual inline void setValue(std::string value, bool faint = false);

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
                 std::string offValue = "Off");

  virtual ~ToggleListItem() {}

  virtual bool onClick(u64 keys);

  /**
   * @brief Gets the current state of the toggle
   *
   * @return State
   */
  virtual inline bool getState();

  /**
   * @brief Sets the current state of the toggle. Updates the Value
   *
   * @param state State
   */
  virtual void setState(bool state);

  /**
   * @brief Adds a listener that gets called whenever the state of the toggle
   * changes
   *
   * @param stateChangedListener Listener with the current state passed in as
   * parameter
   */
  void setStateChangedListener(std::function<void(bool)> stateChangedListener);

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
  virtual ~List();

  virtual void draw(gfx::Renderer* renderer) override;

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override;

  /**
   * @brief Adds a new item to the list
   *
   * @param element Element to add
   * @param height Height of the element. Don't set this parameter for libtesla
   * to try and figure out the size based on the type
   */
  virtual void addItem(Element* element, u16 height = 0) final;

  /**
   * @brief Removes all children from the list
   */
  virtual void clear() final;

  virtual Element* requestFocus(Element* oldFocus,
                                FocusDirection direction) override;

protected:
  struct ListEntry
  {
    Element* element;
    u16 height;

    bool operator==(Element* other);
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

  virtual void draw(gfx::Renderer* renderer) override;

  virtual void layout(u16 parentX,
                      u16 parentY,
                      u16 parentWidth,
                      u16 parentHeight) override {}

private:
  std::function<void(gfx::Renderer*, u16 x, u16 y, u16 w, u16 h)> m_renderFunc;
};

}  // namespace nikola::tsl::elm

#endif  // LIBNIKOLA_ELM_HPP
