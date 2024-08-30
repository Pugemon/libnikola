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

#include <switch/services/hid.h>
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
#define TOUCH_POSITION const HidTouchState
#define TOUCH_INPUT &touchPos
#define JoystickPosition HidAnalogStickState

#include <list>
#include <stack>
#include <memory>

#include <switch/types.h>

#include "tesla/cfg.hpp"
#include "tesla/elm.hpp"
#include "tesla/gfx.hpp"
#include "tesla/hlp.hpp"
#include "tesla/impl.hpp"
#include "tesla/style.hpp"


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

inline u8 TeslaFPS = 60;
inline u8 alphabackground = 0xD;
inline bool FullMode = true;
inline PadState pad;
inline uint16_t framebufferWidth = 448;
inline uint16_t framebufferHeight = 720;
inline bool deactivateOriginalFooter = false;

namespace tsl
{

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

  virtual ~Gui();

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
                           TOUCH_POSITION TOUCH_INPUT,
                           JoystickPosition leftJoyStick,
                           JoystickPosition rightJoyStick);

  /**
   * @brief Gets the top level element
   *
   * @return Top level element
   */
  virtual elm::Element* getTopElement() final;

  /**
   * @brief Get the currently focused element
   *
   * @return Focused element
   */
  virtual elm::Element* getFocusedElement() final;

  /**
   * @brief Requests focus to a element
   * @note Use this function when focusing a element outside of a element's
   * requestFocus function
   *
   * @param element Element to focus
   * @param direction Focus direction
   */
  virtual void requestFocus(elm::Element* element,
                            FocusDirection direction) final;

  /**
   * @brief Removes focus from a element
   *
   * @param element Element to remove focus from. Pass nullptr to remove the
   * focus unconditionally
   */
  virtual void removeFocus(elm::Element* element = nullptr) final;

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
  virtual void draw(gfx::Renderer* renderer) final;
};

// Overlay

using OverlayFactory = std::function<std::unique_ptr<Overlay>()>;

template<impl::LaunchFlags launchFlags = impl::LaunchFlags::CloseOnExit>
int loop(OverlayFactory overlay, int argc, char** argv);

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
  virtual std::unique_ptr<tsl::Gui>& getCurrentGui() final;

  /**
   * @brief Shows the Gui
   *
   */
  virtual void show() final;

  /**
   * @brief Hides the Gui
   *
   */
  virtual void hide() final;

  /**
   * @brief Returns whether fade animation is playing
   *
   * @return whether fade animation is playing
   */
  virtual bool fadeAnimationPlaying() final;

  /**
   * @brief Closes the Gui
   * @note This makes the Tesla overlay exit and return back to the Tesla-Menu
   *
   */
  virtual void close() final;

  /**
   * @brief Gets the Overlay instance
   *
   * @return Overlay instance
   */
  static Overlay* const get();

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
  constexpr std::unique_ptr<T> initially(Args&&... args)
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
  virtual void initScreen() final;

  /**
   * @brief Exits the Renderer
   *
   */
  virtual void exitScreen() final;

  /**
   * @brief Weather or not the Gui should get hidden
   *
   * @return should hide
   */
  virtual bool shouldHide() final;

  /**
   * @brief Weather or not hte Gui should get closed
   *
   * @return should close
   */
  virtual bool shouldClose() final;

  /**
   * @brief Handles fade in and fade out animations of the Overlay
   *
   */
  virtual void animationLoop() final;

  /**
   * @brief Main loop
   *
   */
  virtual void loop() final;

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
                           TOUCH_POSITION touchPos,
                           JoystickPosition joyStickPosLeft,
                           JoystickPosition joyStickPosRight) final;

  /**
   * @brief Clears the screen
   *
   */
  virtual void clearScreen() final;

  /**
   * @brief Reset hide and close flags that were previously set by \ref
   * Overlay::close() or \ref Overlay::hide()
   *
   */
  virtual void resetFlags() final;

  /**
   * @brief Disables the next animation that would play
   *
   */
  virtual void disableNextAnimation() final;

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
  std::unique_ptr<tsl::Gui>& changeTo(std::unique_ptr<tsl::Gui>&& gui);

  /**
   * @brief Pops the top Gui from the stack and goes back to the last one
   * @note The Overlay gets closes once there are no more Guis on the stack
   */
  void goBack();

  template<typename G, typename... Args>
  friend std::unique_ptr<tsl::Gui>& changeTo(Args&&... args);
  friend void goBack();

  friend int loop(OverlayFactory overlay, int argc, char** argv);

  friend class tsl::Gui;
};

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
void goBack();

void setNextOverlay(std::string ovlPath, std::string args);

template <typename TOverlay>
concept DerivedFromOverlay = std::is_base_of_v<Overlay, TOverlay>;

/**
 * @brief libtesla's main function
 * @note Call it directly from main passing in argc and argv and returning it
 * e.g `return tsl::initOverlay<OverlayTest>(argc, argv);`
 *
 * @tparam TOverlay Your overlay class
 * @tparam launchFlags \ref LaunchFlags
 * @param argc argc
 * @param argv argv
 * @return int result
 */
template <DerivedFromOverlay TOverlay,
         impl::LaunchFlags launchFlags = impl::LaunchFlags::CloseOnExit>
int initOverlay(int argc, char** argv)
{
  auto factory = []() -> std::unique_ptr<tsl::Overlay> {
    return std::make_unique<TOverlay>();
  };

  return loop<launchFlags>(factory, argc, argv);
};

}  // namespace tsl

#ifdef TESLA_INIT_IMPL

namespace nikola::tsl::cfg
{

u16 LayerWidth = 0;
u16 LayerHeight = 0;
u16 LayerPosX = 0;
u16 LayerPosY = 0;
u16 FramebufferWidth = 0;
u16 FramebufferHeight = 0;

}  // namespace nikola::tsl::cfg

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