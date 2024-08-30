//
// Created by pugemon on 28.08.24.
//

#ifndef LIBNIKOLA_HLP_HPP
#define LIBNIKOLA_HLP_HPP

#include <functional>
#include <map>
#include <string>

#include <switch/services/vi.h>
#include <switch/types.h>

namespace tsl::hlp
{

/**
 * @brief Wrapper for service initialization
 *
 * @param f wrapped function
 */
void doWithSmSession(std::function<void()> f);

/**
 * @brief libnx hid:sys shim that gives or takes away focus to or from the
 * process with the given aruid
 *
 * @param enable Give focus or take focus
 * @param aruid Aruid of the process to focus/unfocus
 * @return Result Result
 */
Result hidsysEnableAppletToGetInput(bool enable, u64 aruid);

Result viAddToLayerStack(ViLayer* layer, ViLayerStack stack);

/**
 * @brief Toggles focus between the Tesla overlay and the rest of the system
 *
 * @param enabled Focus Tesla?
 */
void requestForeground(bool enabled);

/**
 * @brief Splits a string at the given delimeters
 *
 * @param str String to split
 * @param delim Delimeter
 * @return Vector containing the split tokens
 */
std::vector<std::string> split(const std::string& str, char delim = ' ');

/**
 * @brief Decodes a key string into it's key code
 *
 * @param value Key string
 * @return Key code
 */
u64 stringToKeyCode(std::string& value);

namespace ini
{
/**
 * @brief Ini file type
 */
using IniData = std::map<std::string, std::map<std::string, std::string>>;

/**
 * @brief Parses a ini string
 *
 * @param str String to parse
 * @return Parsed data
 * // Modified to be "const std" instead of just "std"
 */
IniData parseIni(const std::string& str);
}  // namespace ini
}  // namespace tsl::hlp

#endif  // LIBNIKOLA_HLP_HPP
