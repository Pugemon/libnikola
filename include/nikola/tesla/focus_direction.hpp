//
// Created by pugemon on 30.08.24.
//

#ifndef LIBNIKOLA_FOCUS_DIRECTION_HPP
#define LIBNIKOLA_FOCUS_DIRECTION_HPP

namespace tsl
{
/**
 * @brief Direction in which focus moved before landing on
 *        the currently focused element
 */
enum class FocusDirection : u8
{
  None,  ///< Focus was placed on the element programmatically without user
         ///< input
  Up,  ///< Focus moved upwards
  Down,  ///< Focus moved downwards
  Left,  ///< Focus moved from left to right
  Right  ///< Focus moved from right to left
};
}  // namespace tsl

#endif  // LIBNIKOLA_FOCUS_DIRECTION_HPP
