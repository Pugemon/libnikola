//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_NIKOLA_HPP
#define LIBNIKOLA_NIKOLA_HPP

#include <string>
#include "nikola/get_funcs.hpp"
#include "nikola/ini_funcs.hpp"
#include "nikola/string_funcs.hpp"
#include "nikola/tesla.hpp"


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

#endif  // LIBNIKOLA_NIKOLA_HPP
