//
// Created by pugemon on 27.08.24.
//

#include <string>

#include "libnikola/libnikola.hpp"

LibNikola::LibNikola()
    : m_name {"libnikola"}
{
}

auto LibNikola::name() const -> char const*
{
  return m_name.c_str();
}
