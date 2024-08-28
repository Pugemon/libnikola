//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_NIKOLA_HPP
#define LIBNIKOLA_NIKOLA_HPP

#include <string>

#include "nikola/tesla.hpp"
#include "nikola/utils/get_funcs.hpp"
#include "nikola/utils/ini_funcs.hpp"
#include "nikola/utils/string_funcs.hpp"

namespace nikola {

/**
 * @brief Reports the name of the library
 */
class LibNikola
{
public:
  /**
   * @brief Initializes the name field to the name of the project
   */
  LibNikola();

  /**
   * @brief Returns a non-owning pointer to the string stored in this class
   */
  auto name() const -> char const*;

private:
  std::string m_name;
};

} // namespace nikola
#endif  // LIBNIKOLA_NIKOLA_HPP
