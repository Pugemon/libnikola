//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_CFG_HPP
#define LIBNIKOLA_CFG_HPP

#include <switch.h>

namespace tsl::cfg
{

constexpr u32 ScreenWidth = 1920;  ///< Width of the Screen
constexpr u32 ScreenHeight = 1080;  ///< Height of the Screen
constexpr u32 LayerMaxWidth = 1280;
constexpr u32 LayerMaxHeight = 720;

extern u16 LayerWidth;  ///< Width of the Tesla layer
extern u16 LayerHeight;  ///< Height of the Tesla layer
extern u16 LayerPosX;  ///< X position of the Tesla layer
extern u16 LayerPosY;  ///< Y position of the Tesla layer
extern u16 FramebufferWidth;  ///< Width of the framebuffer
extern u16 FramebufferHeight;  ///< Height of the framebuffer

}  // namespace nikola::tsl::cfg

#endif  // LIBNIKOLA_CFG_HPP
