//
// Created by pugemon on 29.08.24.
//

#include <cmath>

#include "nikola/tesla/elm.hpp"

#include "nikola/tesla.hpp"

constexpr float M_PI = 3.14159265358979323846;

namespace nikola::tsl::elm
{

Element* Element::requestFocus(Element* oldFocus, FocusDirection direction)
{
  return nullptr;
}

bool Element::onClick(u64 keys)
{
  return m_clickListener(keys);
}

bool Element::onTouch(u32 x, u32 y)
{
  return false;
}

void Element::frame(gfx::Renderer* renderer)
{
  if (this->m_focused)
    this->drawHighlight(renderer);

  this->draw(renderer);
}

void Element::invalidate()
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

void Element::shakeHighlight(FocusDirection direction)
{
  this->m_highlightShaking = true;
  this->m_highlightShakingDirection = direction;
  this->m_highlightShakingStartTime = std::chrono::system_clock::now();
}

void Element::drawHighlight(gfx::Renderer* renderer)
{
  using namespace std::literals::chrono_literals;

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
    auto t =
        (std::chrono::system_clock::now() - this->m_highlightShakingStartTime);
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

void Element::setBoundaries(u16 x, u16 y, u16 width, u16 height)
{
  this->m_x = x;
  this->m_y = y;
  this->m_width = width;
  this->m_height = height;
}

void Element::setClickListener(std::function<bool(u64)> clickListener)
{
  this->m_clickListener = clickListener;
}

void Element::setParent(Element* parent)
{
  this->m_parent = parent;
}

Element* Element::getParent()
{
  return this->m_parent;
}

void Element::setFocused(bool focused)
{
  this->m_focused = focused;
}

int Element::shakeAnimation(std::chrono::system_clock::duration t, float a)
{
  float w = 0.2F;
  float tau = 0.05F;

  int t_ = t.count() / 1'000'000;

  return roundf(a * exp(-(tau * t_) * sin(w * t_)));
}

OverlayFrame::~OverlayFrame()
{
  if (this->m_contentElement != nullptr)
    delete this->m_contentElement;
}

void OverlayFrame::draw(gfx::Renderer* renderer)
{
  renderer->fillScreen(a({0x0, 0x0, 0x0, alphabackground}));

  renderer->drawString(
      this->m_title.c_str(), false, 20, 50, 30, a(defaultTextColor));
  renderer->drawString(
      this->m_subtitle.c_str(), false, 20, 70, 15, a(defaultTextColor));

  if (FullMode == true)
    renderer->drawRect(
        15, 720 - 73, tsl::cfg::FramebufferWidth - 30, 1, a(defaultTextColor));
  if (!deactivateOriginalFooter)
    renderer->drawString(
        "\uE0E1  Back     \uE0E0  OK", false, 30, 693, 23, a(defaultTextColor));

  if (this->m_contentElement != nullptr)
    this->m_contentElement->frame(renderer);
}

void OverlayFrame::layout(u16 parentX,
                          u16 parentY,
                          u16 parentWidth,
                          u16 parentHeight)
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

Element* OverlayFrame::requestFocus(Element* oldFocus, FocusDirection direction)
{
  if (this->m_contentElement != nullptr)
    return this->m_contentElement->requestFocus(oldFocus, direction);
  else
    return nullptr;
}

void OverlayFrame::setContent(Element* content)
{
  if (this->m_contentElement != nullptr)
    delete this->m_contentElement;

  this->m_contentElement = content;

  if (content != nullptr) {
    this->m_contentElement->setParent(this);
    this->invalidate();
  }
}

void DebugRectangle::draw(gfx::Renderer* renderer)
{
  renderer->drawRect(this->getX(),
                     this->getY(),
                     this->getWidth(),
                     this->getHeight(),
                     a(this->m_color));
}

void ListItem::draw(gfx::Renderer* renderer)
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

  renderer->drawRect(
      this->getX(), this->getY(), this->getWidth(), 1, a({0x4, 0x4, 0x4, 0xF}));
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

Element* ListItem::requestFocus(Element* oldFocus, FocusDirection direction)
{
  return this;
}

void ListItem::setText(std::string text)
{
  this->m_text = text;
}

void ListItem::setValue(std::string value, bool faint)
{
  this->m_value = value;
  this->m_faint = faint;
  this->m_valueWidth = 0;
}

ToggleListItem::ToggleListItem(std::string text,
                               bool initialState,
                               std::string onValue,
                               std::string offValue)
    : ListItem(text)
    , m_state(initialState)
    , m_onValue(onValue)
    , m_offValue(offValue)
{
  this->setState(this->m_state);
}

bool ToggleListItem::onClick(u64 keys)
{
  if (keys & KEY_A) {
    this->m_state = !this->m_state;

    this->setState(this->m_state);
    this->m_stateChangedListener(this->m_state);

    return true;
  }

  return false;
}

bool ToggleListItem::getState()
{
  return this->m_state;
}

void ToggleListItem::setState(bool state)
{
  this->m_state = state;

  if (state)
    this->setValue(this->m_onValue, false);
  else
    this->setValue(this->m_offValue, true);
}

void ToggleListItem::setStateChangedListener(
    std::function<void(bool)> stateChangedListener)
{
  this->m_stateChangedListener = stateChangedListener;
}

List::~List()
{
  for (auto& item : this->m_items)
    delete item.element;
}

void List::draw(gfx::Renderer* renderer)
{
  u16 i = 0;
  for (auto& entry : this->m_items) {
    if (i >= this->m_offset && i < this->m_offset + this->m_entriesShown) {
      entry.element->frame(renderer);
    }
    i++;
  }
}

void List::layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight)
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

void List::addItem(Element* element, u16 height)
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

void List::clear()
{
  for (auto& item : this->m_items)
    delete item.element;

  this->m_items.clear();
}

Element* List::requestFocus(Element* oldFocus, FocusDirection direction)
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
          == (this->m_items.begin() + this->m_offset + this->m_entriesShown - 2)
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

bool List::ListEntry::operator==(Element* other)
{
  return this->element == other;
}

void CustomDrawer::draw(gfx::Renderer* renderer)
{
  this->m_renderFunc(renderer,
                     this->getX(),
                     this->getY(),
                     this->getWidth(),
                     this->getHeight());
}
}  // namespace nikola::tsl::elm