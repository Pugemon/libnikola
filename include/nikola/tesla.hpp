/**
 * Copyright (C) 2024 Pugemon
 *
 * This file is part of nikola.
 *
 * nikola is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * nikola is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nikola.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define KEY_A HidNpadButton_A
#define KEY_B HidNpadButton_B
#define KEY_X HidNpadButton_X
#define KEY_Y HidNpadButton_Y
#define KEY_L HidNpadButton_L
#define KEY_R HidNpadButton_R
#define KEY_ZL HidNpadButton_ZL
#define KEY_ZR HidNpadButton_ZR
#define KEY_PLUS HidNpadButton_Plus
#define KEY_MINUS HidNpadButton_Minus
#define KEY_DUP HidNpadButton_Up
#define KEY_DDOWN HidNpadButton_Down
#define KEY_DLEFT HidNpadButton_Left
#define KEY_DRIGHT HidNpadButton_Right
#define KEY_SL HidNpadButton_AnySL
#define KEY_SR HidNpadButton_AnySR
#define KEY_LSTICK HidNpadButton_StickL
#define KEY_RSTICK HidNpadButton_StickR
#define KEY_UP \
  (HidNpadButton_Up | HidNpadButton_StickLUp | HidNpadButton_StickRUp)
#define KEY_DOWN \
  (HidNpadButton_Down | HidNpadButton_StickLDown | HidNpadButton_StickRDown)
#define KEY_LEFT \
  (HidNpadButton_Left | HidNpadButton_StickLLeft | HidNpadButton_StickRLeft)
#define KEY_RIGHT \
  (HidNpadButton_Right | HidNpadButton_StickLRight | HidNpadButton_StickRRight)
#define touchPosition const HidTouchState
#define touchInput &touchPos
#define JoystickPosition HidAnalogStickState
float M_PI = 3.14159265358979323846;

#include "tesla/cfg.hpp"
#include "tesla/style.hpp"
#include "tesla/impl.hpp"
#include "tesla/hlp.hpp"
#include "tesla/gfx.hpp"
#include "tesla/elm.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <cwctype>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <stack>
#include <string>
#include <type_traits>

#include <math.h>
#include <stdlib.h>
#include <strings.h>
#include <switch.h>

#include "utils/ini_funcs.hpp"

namespace nikola
{



// Define this makro before including tesla.hpp in your main file. If you intend
// to use the tesla.hpp header in more than one source file, only define it
// once! #define TESLA_INIT_IMPL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#ifdef TESLA_INIT_IMPL
#  define STB_TRUETYPE_IMPLEMENTATION
#endif
#include "stb_truetype.h"

#pragma GCC diagnostic pop

#define ASSERT_EXIT(x) \
  if (R_FAILED(x)) \
  std::exit(1)
#define ASSERT_FATAL(x) \
  if (Result res = x; R_FAILED(res)) \
  fatalThrow(res)

u8 TeslaFPS = 60;
u8 alphabackground = 0xD;
bool FullMode = true;
PadState pad;
uint16_t framebufferWidth = 448;
uint16_t framebufferHeight = 720;
bool deactivateOriginalFooter = false;

using namespace std::literals::chrono_literals;

namespace tsl
{

// Declarations

/**
 * @brief Direction in which focus moved before landing on
 *        the currently focused element
 */
enum class FocusDirection
{
  None,  ///< Focus was placed on the element programatically without user input
  Up,  ///< Focus moved upwards
  Down,  ///< Focus moved downwards
  Left,  ///< Focus moved from left to rigth
  Right  ///< Focus moved from right to left
};

class Overlay;



[[maybe_unused]] static void goBack();

[[maybe_unused]] static void setNextOverlay(std::string ovlPath,
                                            std::string args = "");

template<typename TOverlay,
         impl::LaunchFlags launchFlags = impl::LaunchFlags::CloseOnExit>
int loop(int argc, char** argv);



// Renderer



// Elements



// GUI

/**
 * @brief The top level Gui class
 * @note The main menu and every sub menu are a separate Gui. Create your own
 * Gui class that extends from this one to create your own menus
 *
 */
class Gui
{
public:
  Gui() {}

  virtual ~Gui()
  {
    if (this->m_topElement != nullptr)
      delete this->m_topElement;
  }

  /**
   * @brief Creates all elements present in this Gui
   * @note Implement this function and let it return a heap allocated element
   * used as the top level element. This is usually some kind of frame e.g \ref
   * OverlayFrame
   *
   * @return Top level element
   */
  virtual elm::Element* createUI() = 0;

  /**
   * @brief Called once per frame to update values
   *
   */
  virtual void update() {}

  /**
   * @brief Called once per frame with the latest HID inputs
   *
   * @param keysDown Buttons pressed in the last frame
   * @param keysHeld Buttons held down longer than one frame
   * @param touchInput Last touch position
   * @param leftJoyStick Left joystick position
   * @param rightJoyStick Right joystick position
   * @return Weather or not the input has been consumed
   */
  virtual bool handleInput(u64 keysDown,
                           u64 keysHeld,
                           touchPosition touchInput,
                           JoystickPosition leftJoyStick,
                           JoystickPosition rightJoyStick)
  {
    return false;
  }

  /**
   * @brief Gets the top level element
   *
   * @return Top level element
   */
  virtual elm::Element* getTopElement() final { return this->m_topElement; }

  /**
   * @brief Get the currently focused element
   *
   * @return Focused element
   */
  virtual elm::Element* getFocusedElement() final
  {
    return this->m_focusedElement;
  }

  /**
   * @brief Requests focus to a element
   * @note Use this function when focusing a element outside of a element's
   * requestFocus function
   *
   * @param element Element to focus
   * @param direction Focus direction
   */
  virtual void requestFocus(elm::Element* element,
                            FocusDirection direction) final
  {
    elm::Element* oldFocus = this->m_focusedElement;

    if (element != nullptr) {
      this->m_focusedElement = element->requestFocus(oldFocus, direction);

      if (oldFocus != nullptr)
        oldFocus->setFocused(false);

      if (this->m_focusedElement != nullptr) {
        this->m_focusedElement->setFocused(true);
      }
    }

    if (oldFocus == this->m_focusedElement && this->m_focusedElement != nullptr)
      this->m_focusedElement->shakeHighlight(direction);
  }

  /**
   * @brief Removes focus from a element
   *
   * @param element Element to remove focus from. Pass nullptr to remove the
   * focus unconditionally
   */
  virtual void removeFocus(elm::Element* element = nullptr) final
  {
    if (element == nullptr || element == this->m_focusedElement)
      this->m_focusedElement = nullptr;
  }

protected:
  constexpr static inline auto a = &gfx::Renderer::a;

private:
  elm::Element* m_focusedElement = nullptr;
  elm::Element* m_topElement = nullptr;

  friend class Overlay;
  friend class gfx::Renderer;

  /**
   * @brief Draws the Gui
   *
   * @param renderer
   */
  virtual void draw(gfx::Renderer* renderer) final
  {
    if (this->m_topElement != nullptr)
      this->m_topElement->draw(renderer);
  }
};

// Overlay

/**
 * @brief The top level Overlay class
 * @note Every Tesla overlay should have exactly one Overlay class initializing
 * services and loading the default Gui
 */
class Overlay
{
protected:
  /**
   * @brief Constructor
   * @note Called once when the Overlay gets loaded
   */
  Overlay() {}

public:
  /**
   * @brief Deconstructor
   * @note Called once when the Overlay exits
   *
   */
  virtual ~Overlay() {}

  /**
   * @brief Initializes services
   * @note Called once at the start to initializes services. You have a sm
   * session available during this call, no need to initialize sm yourself
   */
  virtual void initServices() {}

  /**
   * @brief Exits services
   * @note Make sure to exit all services you initialized in \ref
   * Overlay::initServices() here to prevent leaking handles
   */
  virtual void exitServices() {}

  /**
   * @brief Called before overlay changes from invisible to visible state
   *
   */
  virtual void onShow() {}

  /**
   * @brief Called before overlay changes from visible to invisible state
   *
   */
  virtual void onHide() {}

  /**
   * @brief Loads the default Gui
   * @note This function should return the initial Gui to load using the \ref
   * Gui::initially<T>(Args.. args) function e.g `return initially<GuiMain>();`
   *
   * @return Default Gui
   */
  virtual std::unique_ptr<tsl::Gui> loadInitialGui() = 0;

  /**
   * @brief Gets a reference to the current Gui on top of the Gui stack
   *
   * @return Current Gui reference
   */
  virtual std::unique_ptr<tsl::Gui>& getCurrentGui() final
  {
    return this->m_guiStack.top();
  }

  /**
   * @brief Shows the Gui
   *
   */
  virtual void show() final
  {
    if (this->m_disableNextAnimation) {
      this->m_animationCounter = 5;
      this->m_disableNextAnimation = false;
    } else {
      this->m_fadeInAnimationPlaying = false;
      this->m_animationCounter = 0;
    }

    this->onShow();
  }

  /**
   * @brief Hides the Gui
   *
   */
  virtual void hide() final
  {
    if (this->m_disableNextAnimation) {
      this->m_animationCounter = 0;
      this->m_disableNextAnimation = false;
    } else {
      this->m_fadeOutAnimationPlaying = false;
      this->m_animationCounter = 5;
    }

    this->onHide();
  }

  /**
   * @brief Returns whether fade animation is playing
   *
   * @return whether fade animation is playing
   */
  virtual bool fadeAnimationPlaying() final
  {
    return this->m_fadeInAnimationPlaying || this->m_fadeOutAnimationPlaying;
  }

  /**
   * @brief Closes the Gui
   * @note This makes the Tesla overlay exit and return back to the Tesla-Menu
   *
   */
  virtual void close() final { this->m_shouldClose = true; }

  /**
   * @brief Gets the Overlay instance
   *
   * @return Overlay instance
   */
  static inline Overlay* const get() { return Overlay::s_overlayInstance; }

  /**
   * @brief Creates the initial Gui of an Overlay and moves the object to the
   * Gui stack
   *
   * @tparam T
   * @tparam Args
   * @param args
   * @return constexpr std::unique_ptr<T>
   */
  template<typename T, typename... Args>
  constexpr inline std::unique_ptr<T> initially(Args&&... args)
  {
    return std::make_unique<T>(args...);
  }

private:
  using GuiPtr = std::unique_ptr<tsl::Gui>;
  std::stack<GuiPtr, std::list<GuiPtr>> m_guiStack;
  static inline Overlay* s_overlayInstance = nullptr;

  bool m_fadeInAnimationPlaying = true, m_fadeOutAnimationPlaying = false;
  u8 m_animationCounter = 0;

  bool m_shouldHide = false;
  bool m_shouldClose = false;

  bool m_disableNextAnimation = false;

  bool m_closeOnExit;

  /**
   * @brief Initializes the Renderer
   *
   */
  virtual void initScreen() final { gfx::Renderer::get().init(); }

  /**
   * @brief Exits the Renderer
   *
   */
  virtual void exitScreen() final { gfx::Renderer::get().exit(); }

  /**
   * @brief Weather or not the Gui should get hidden
   *
   * @return should hide
   */
  virtual bool shouldHide() final { return this->m_shouldHide; }

  /**
   * @brief Weather or not hte Gui should get closed
   *
   * @return should close
   */
  virtual bool shouldClose() final { return this->m_shouldClose; }

  /**
   * @brief Handles fade in and fade out animations of the Overlay
   *
   */
  virtual void animationLoop() final
  {
    if (this->m_fadeInAnimationPlaying) {
      this->m_animationCounter++;

      if (this->m_animationCounter >= 5)
        this->m_fadeInAnimationPlaying = false;
    }

    if (this->m_fadeOutAnimationPlaying) {
      this->m_animationCounter--;

      if (this->m_animationCounter == 0) {
        this->m_fadeOutAnimationPlaying = false;
        this->m_shouldHide = true;
      }
    }

    gfx::Renderer::setOpacity(0.2 * this->m_animationCounter);
  }

  /**
   * @brief Main loop
   *
   */
  virtual void loop() final
  {
    auto& renderer = gfx::Renderer::get();

    renderer.startFrame();

    this->animationLoop();
    this->getCurrentGui()->update();
    this->getCurrentGui()->draw(&renderer);

    renderer.endFrame();
  }

  /**
   * @brief Called once per frame with the latest HID inputs
   *
   * @param keysDown Buttons pressed in the last frame
   * @param keysHeld Buttons held down longer than one frame
   * @param touchInput Last touch position
   * @param leftJoyStick Left joystick position
   * @param rightJoyStick Right joystick position
   * @return Weather or not the input has been consumed
   */
  virtual void handleInput(u64 keysDown,
                           u64 keysHeld,
                           touchPosition touchPos,
                           JoystickPosition joyStickPosLeft,
                           JoystickPosition joyStickPosRight) final
  {
    auto& currentGui = this->getCurrentGui();
    auto currentFocus = currentGui->getFocusedElement();

    if (currentFocus == nullptr) {
      if (elm::Element* topElement = currentGui->getTopElement();
          topElement == nullptr)
      {
        return;
      } else
        currentFocus = topElement;
    }

    bool handled = false;
    elm::Element* parentElement = currentFocus;
    do {
      handled = parentElement->onClick(keysDown);
      parentElement = parentElement->getParent();
    } while (!handled && parentElement != nullptr);

    if (currentGui != this->getCurrentGui())
      return;

    handled = handled
        | currentGui->handleInput(
            keysDown, keysHeld, touchPos, joyStickPosLeft, joyStickPosRight);

    if (!handled) {
      if (keysDown & KEY_UP)
        currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Up);
      else if (keysDown & KEY_DOWN)
        currentGui->requestFocus(currentFocus->getParent(),
                                 FocusDirection::Down);
      else if (keysDown & KEY_LEFT)
        currentGui->requestFocus(currentFocus->getParent(),
                                 FocusDirection::Left);
      else if (keysDown & KEY_RIGHT)
        currentGui->requestFocus(currentFocus->getParent(),
                                 FocusDirection::Right);
    }
  }

  /**
   * @brief Clears the screen
   *
   */
  virtual void clearScreen() final
  {
    auto& renderer = gfx::Renderer::get();

    renderer.startFrame();
    renderer.clearScreen();
    renderer.endFrame();
  }

  /**
   * @brief Reset hide and close flags that were previously set by \ref
   * Overlay::close() or \ref Overlay::hide()
   *
   */
  virtual void resetFlags() final
  {
    this->m_shouldHide = false;
    this->m_shouldClose = false;
  }

  /**
   * @brief Disables the next animation that would play
   *
   */
  virtual void disableNextAnimation() final
  {
    this->m_disableNextAnimation = true;
  }

  /**
   * @brief Creates a new Gui and changes to it
   *
   * @tparam G Gui to create
   * @tparam Args Arguments to pass to the Gui
   * @param args Arguments to pass to the Gui
   * @return Reference to the newly created Gui
   */
  template<typename G, typename... Args>
  std::unique_ptr<tsl::Gui>& changeTo(Args&&... args)
  {
    auto newGui = std::make_unique<G>(std::forward<Args>(args)...);
    newGui->m_topElement = newGui->createUI();
    newGui->requestFocus(newGui->m_topElement, FocusDirection::None);

    this->m_guiStack.push(std::move(newGui));

    return this->m_guiStack.top();
  }

  /**
   * @brief Changes to a different Gui
   *
   * @param gui Gui to change to
   * @return Reference to the Gui
   */
  std::unique_ptr<tsl::Gui>& changeTo(std::unique_ptr<tsl::Gui>&& gui)
  {
    gui->m_topElement = gui->createUI();
    gui->requestFocus(gui->m_topElement, FocusDirection::None);

    this->m_guiStack.push(std::move(gui));

    return this->m_guiStack.top();
  }

  /**
   * @brief Pops the top Gui from the stack and goes back to the last one
   * @note The Overlay gets closes once there are no more Guis on the stack
   */
  void goBack()
  {
    if (!this->m_closeOnExit && this->m_guiStack.size() == 1) {
      this->hide();
      return;
    }

    if (!this->m_guiStack.empty())
      this->m_guiStack.pop();

    if (this->m_guiStack.empty())
      this->close();
  }

  template<typename G, typename... Args>
  friend std::unique_ptr<tsl::Gui>& changeTo(Args&&... args);

  friend void goBack();

  template<typename, tsl::impl::LaunchFlags>
  friend int loop(int argc, char** argv);

  friend class tsl::Gui;
};

namespace impl
{

/**
 * @brief Data shared between the different threads
 *
 */
struct SharedThreadData
{
  bool running = false;

  Event comboEvent = {0}, homeButtonPressEvent = {0},
        powerButtonPressEvent = {0};

  u64 launchCombo = KEY_L | KEY_DDOWN | KEY_RSTICK;
  bool overlayOpen = false;

  std::mutex dataMutex;
  u64 keysDown = 0;
  u64 keysDownPending = 0;
  u64 keysHeld = 0;
  HidTouchScreenState touchState = {0};
  JoystickPosition joyStickPosLeft = {0}, joyStickPosRight = {0};
};

/**
 * @brief Parses the Tesla settings
 *
 * @param[out] launchCombo Overlay launch button combo
 */
static void parseOverlaySettings(u64& launchCombo)
{
  FILE* configFile = fopen("sdmc:/config/tesla/config.ini", "r");

  if (configFile == nullptr)
    return;

  fseek(configFile, 0, SEEK_END);
  size_t configFileSize = ftell(configFile);
  rewind(configFile);

  std::string configFileData(configFileSize, '\0');
  fread(&configFileData[0], sizeof(char), configFileSize, configFile);
  fclose(configFile);

  hlp::ini::IniData parsedConfig = hlp::ini::parseIni(configFileData);

  launchCombo = 0x00;
  size_t max_combo = 4;
  for (std::string key : hlp::split(parsedConfig["tesla"]["key_combo"], '+')) {
    launchCombo |= hlp::stringToKeyCode(key);
    if (!--max_combo) {
      return;
    }
  }
}

/**
 * @brief Input polling loop thread
 *
 * @tparam launchFlags Launch flags
 * @param args Used to pass in a pointer to a \ref SharedThreadData struct
 */
template<impl::LaunchFlags launchFlags>
static void hidInputPoller(void* args)
{
  SharedThreadData* shData = static_cast<SharedThreadData*>(args);

  // Parse Tesla settings
  impl::parseOverlaySettings(shData->launchCombo);

  padInitializeAny(&pad);

  hidInitializeTouchScreen();

  // Drop all inputs from the previous overlay
  padUpdate(&pad);

  while (shData->running) {
    // Scan for input changes
    padUpdate(&pad);

    // Read in HID values
    {
      std::scoped_lock lock(shData->dataMutex);

      shData->keysDown = padGetButtonsDown(&pad);
      shData->keysHeld = padGetButtons(&pad);
      shData->joyStickPosLeft = padGetStickPos(&pad, 0);
      shData->joyStickPosRight = padGetStickPos(&pad, 1);

      // Read in touch positions
      if (hidGetTouchScreenStates(&shData->touchState, 1) == 0)
        shData->touchState = {0};

      if (((shData->keysHeld & shData->launchCombo) == shData->launchCombo)
          && shData->keysDown & shData->launchCombo)
      {
        if (shData->overlayOpen) {
          tsl::Overlay::get()->hide();
          shData->overlayOpen = false;
        } else
          eventFire(&shData->comboEvent);
      }

      shData->keysDownPending |= shData->keysDown;
    }

    // 20 ms
    svcSleepThread(20E6);
  }
}

/**
 * @brief Home button detection loop thread
 * @note This makes sure that focus cannot glitch out when pressing the home
 * button
 *
 * @param args Used to pass in a pointer to a \ref SharedThreadData struct
 */
static void homeButtonDetector(void* args)
{
  SharedThreadData* shData = static_cast<SharedThreadData*>(args);

  // To prevent focus glitchout, close the overlay immediately when the home
  // button gets pressed
  hidsysAcquireHomeButtonEventHandle(&shData->homeButtonPressEvent, false);

  while (shData->running) {
    if (R_SUCCEEDED(eventWait(&shData->homeButtonPressEvent, 100'000'000))) {
      eventClear(&shData->homeButtonPressEvent);

      if (shData->overlayOpen) {
        tsl::Overlay::get()->hide();
        shData->overlayOpen = false;
      }
    }
  }
}

/**
 * @brief Power button detection loop thread
 * @note This makes sure that focus cannot glitch out when pressing the power
 * button
 *
 * @param args Used to pass in a pointer to a \ref SharedThreadData struct
 */
static void powerButtonDetector(void* args)
{
  SharedThreadData* shData = static_cast<SharedThreadData*>(args);

  // To prevent focus glitchout, close the overlay immediately when the power
  // button gets pressed
  hidsysAcquireSleepButtonEventHandle(&shData->powerButtonPressEvent, false);

  while (shData->running) {
    if (R_SUCCEEDED(eventWait(&shData->powerButtonPressEvent, 100'000'000))) {
      eventClear(&shData->powerButtonPressEvent);

      if (shData->overlayOpen) {
        tsl::Overlay::get()->hide();
        shData->overlayOpen = false;
      }
    }
  }
}

}  // namespace impl

/**
 * @brief Creates a new Gui and changes to it
 *
 * @tparam G Gui to create
 * @tparam Args Arguments to pass to the Gui
 * @param args Arguments to pass to the Gui
 * @return Reference to the newly created Gui
 */
template<typename G, typename... Args>
std::unique_ptr<tsl::Gui>& changeTo(Args&&... args)
{
  return Overlay::get()->changeTo<G, Args...>(std::forward<Args>(args)...);
}

/**
 * @brief Pops the top Gui from the stack and goes back to the last one
 * @note The Overlay gets closes once there are no more Guis on the stack
 */
static void goBack()
{
  Overlay::get()->goBack();
}

static void setNextOverlay(std::string ovlPath, std::string args)
{
  args += " --skipCombo";

  envSetNextLoad(ovlPath.c_str(), args.c_str());
}

/**
 * @brief libtesla's main function
 * @note Call it directly from main passing in argc and argv and returning it
 * e.g `return tsl::loop<OverlayTest>(argc, argv);`
 *
 * @tparam TOverlay Your overlay class
 * @tparam launchFlags \ref LaunchFlags
 * @param argc argc
 * @param argv argv
 * @return int result
 */
template<typename TOverlay, impl::LaunchFlags launchFlags>
static inline int loop(int argc, char** argv)
{
  static_assert(std::is_base_of_v<tsl::Overlay, TOverlay>,
                "tsl::loop expects a type derived from tsl::Overlay");

  impl::SharedThreadData shData;

  shData.running = true;

  Thread hidPollerThread, homeButtonDetectorThread, powerButtonDetectorThread;
  threadCreate(&hidPollerThread,
               impl::hidInputPoller<launchFlags>,
               &shData,
               nullptr,
               0x1000,
               0x2C,
               -2);
  threadCreate(&homeButtonDetectorThread,
               impl::homeButtonDetector,
               &shData,
               nullptr,
               0x1000,
               0x2C,
               -2);
  threadCreate(&powerButtonDetectorThread,
               impl::powerButtonDetector,
               &shData,
               nullptr,
               0x1000,
               0x2C,
               -2);
  threadStart(&hidPollerThread);
  threadStart(&homeButtonDetectorThread);
  threadStart(&powerButtonDetectorThread);

  eventCreate(&shData.comboEvent, false);

  auto& overlay = tsl::Overlay::s_overlayInstance;
  overlay = new TOverlay();
  overlay->m_closeOnExit =
      (u8(launchFlags) & u8(impl::LaunchFlags::CloseOnExit))
      == u8(impl::LaunchFlags::CloseOnExit);

  tsl::hlp::doWithSmSession([&overlay] { overlay->initServices(); });
  overlay->initScreen();
  overlay->changeTo(overlay->loadInitialGui());

  // Argument parsing
  for (u8 arg = 0; arg < argc; arg++) {
    if (strcasecmp(argv[arg], "--skipCombo") == 0) {
      eventFire(&shData.comboEvent);
      overlay->disableNextAnimation();
    }
  }

  while (shData.running) {
    eventWait(&shData.comboEvent, UINT64_MAX);
    eventClear(&shData.comboEvent);
    shData.overlayOpen = true;

    hlp::requestForeground(true);

    overlay->show();
    overlay->clearScreen();

    while (shData.running) {
      overlay->loop();

      {
        std::scoped_lock lock(shData.dataMutex);
        if (!overlay->fadeAnimationPlaying()) {
          overlay->handleInput(shData.keysDownPending,
                               shData.keysHeld,
                               shData.touchState.touches[0],
                               shData.joyStickPosLeft,
                               shData.joyStickPosRight);
        }
        shData.keysDownPending = 0;
      }

      if (overlay->shouldHide())
        break;

      if (overlay->shouldClose())
        shData.running = false;
    }

    overlay->clearScreen();
    overlay->resetFlags();

    hlp::requestForeground(false);

    shData.overlayOpen = false;
    eventClear(&shData.comboEvent);
  }

  eventClose(&shData.homeButtonPressEvent);
  eventClose(&shData.powerButtonPressEvent);
  eventClose(&shData.comboEvent);

  threadWaitForExit(&hidPollerThread);
  threadClose(&hidPollerThread);
  threadWaitForExit(&homeButtonDetectorThread);
  threadClose(&homeButtonDetectorThread);
  threadWaitForExit(&powerButtonDetectorThread);
  threadClose(&powerButtonDetectorThread);

  overlay->exitScreen();
  overlay->exitServices();

  delete overlay;

  return 0;
}

}  // namespace tsl

#ifdef TESLA_INIT_IMPL

namespace tsl::cfg
{

u16 LayerWidth = 0;
u16 LayerHeight = 0;
u16 LayerPosX = 0;
u16 LayerPosY = 0;
u16 FramebufferWidth = 0;
u16 FramebufferHeight = 0;

}  // namespace tsl::cfg

extern "C" void __libnx_init_time(void);

extern "C"
{
u32 __nx_applet_type = AppletType_None;
u32 __nx_nv_transfermem_size = 0x40000;
ViLayerFlags __nx_vi_stray_layer_flags = (ViLayerFlags)0;

/**
 * @brief libtesla service initializing function to override libnx's
 *
 */
void __appInit(void)
{
  tsl::hlp::doWithSmSession(
      []
      {
        ASSERT_FATAL(fsInitialize());
        ASSERT_FATAL(fsdevMountSdmc());
        ASSERT_FATAL(hidInitialize());  // Controller inputs and Touch
        if (hosversionAtLeast(16, 0, 0)) {
          ASSERT_FATAL(plInitialize(PlServiceType_User));  // Font data
        } else
          ASSERT_FATAL(plInitialize(PlServiceType_System));
        ASSERT_FATAL(pmdmntInitialize());  // PID querying
        ASSERT_FATAL(hidsysInitialize());  // Focus control
        ASSERT_FATAL(setsysInitialize());  // Settings querying
      });
  Service* plSrv = plGetServiceSession();
  Service plClone;
  ASSERT_FATAL(serviceClone(plSrv, &plClone));
  serviceClose(plSrv);
  *plSrv = plClone;
}

/**
 * @brief libtesla service exiting function to override libnx's
 *
 */
void __appExit(void)
{
  fsExit();
  hidExit();
  plExit();
  pmdmntExit();
  hidsysExit();
  setsysExit();
}
}

#endif
} // namespace nikola