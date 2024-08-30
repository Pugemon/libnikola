//
// Created by pugemon on 27.08.24.
//

#include "nikola.hpp"

using namespace nikola;

LibNikola::LibNikola()
    : m_name {"nikola"}
{
}

auto LibNikola::name() const -> char const*
{
  return m_name.c_str();
}
