//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_IMPL_HPP
#define LIBNIKOLA_IMPL_HPP

namespace nikola::tsl::impl
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

}  // namespace nikola::tsl::impl

#endif  // LIBNIKOLA_IMPL_HPP
