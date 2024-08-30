//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_IMPL_HPP
#define LIBNIKOLA_IMPL_HPP

#include <mutex>

#include <switch/kernel/event.h>
#include <switch/services/hid.h>
#include <switch/types.h>

namespace tsl::impl
{

/**
 * @brief Overlay launch parameters
 */
enum class LaunchFlags : u8
{
  None = 0,  ///< Do nothing special at launch
  CloseOnExit =
      BIT(0)  ///< Close the overlay the last Gui gets poped from the stack
};

[[maybe_unused]] static constexpr LaunchFlags operator|(LaunchFlags lhs,
                                                        LaunchFlags rhs)
{
  return static_cast<LaunchFlags>(u8(lhs) | u8(rhs));
}

/**
 * @brief Data shared between the different threads
 *
 */
struct SharedThreadData
{
  bool running = false;

  Event comboEvent = {0}, homeButtonPressEvent = {0},
        powerButtonPressEvent = {0};

  u64 launchCombo = HidNpadButton_L | HidNpadButton_Down | HidNpadButton_StickR;
  bool overlayOpen = false;

  std::mutex dataMutex;
  u64 keysDown = 0;
  u64 keysDownPending = 0;
  u64 keysHeld = 0;
  HidTouchScreenState touchState = {0};
  HidAnalogStickState joyStickPosLeft = {0}, joyStickPosRight = {0};
};

/**
 * @brief Parses the Tesla settings
 *
 * @param[out] launchCombo Overlay launch button combo
 */
void parseOverlaySettings(u64& launchCombo);

/**
 * @brief Input polling loop thread
 *
 * @tparam launchFlags Launch flags
 * @param args Used to pass in a pointer to a \ref SharedThreadData struct
 */
template<impl::LaunchFlags launchFlags>
void hidInputPoller(void* args);

/**
 * @brief Home button detection loop thread
 * @note This makes sure that focus cannot glitch out when pressing the home
 * button
 *
 * @param args Used to pass in a pointer to a \ref SharedThreadData struct
 */
void homeButtonDetector(void* args);

/**
 * @brief Power button detection loop thread
 * @note This makes sure that focus cannot glitch out when pressing the power
 * button
 *
 * @param args Used to pass in a pointer to a \ref SharedThreadData struct
 */
void powerButtonDetector(void* args);

}  // namespace tsl::impl

#endif  // LIBNIKOLA_IMPL_HPP
