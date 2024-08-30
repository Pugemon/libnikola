/********************************************************************************
 * File: ini_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file provides functions for working with INI (Initialization)
 *files in C++. It includes functions for reading, parsing, and editing INI
 *files, as well as cleaning INI file formatting.
 *
 *   For the latest updates and contributions, visit the project's GitHub
 *repository. (GitHub Repository:
 *https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *   Note: Please be aware that this notice cannot be altered or removed. It is
 *a part of the project's documentation and must remain intact.
 *
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/

#pragma once
#include <cstdio>
#include <cstring>  // For std::string, strlen(), etc.
#include <map>  // For std::map
#include <string>  // For std::string
#include <vector>  // For std::vector

namespace nikola::utils
{

/**
 * @brief Splits a string into a vector of substrings using a specified
 * delimiter.
 *
 * This function splits a given string into multiple substrings based on the
 * specified delimiter.
 *
 * @param str The input string to be split.
 * @param delim The delimiter character used for splitting (default is space '
 * ').
 * @return A vector of substrings obtained by splitting the input string.
 */
static std::vector<std::string> split(const std::string& str, char delim = ' ');

/**
 * @brief Parses an INI-formatted string into a map of sections and key-value
 * pairs.
 *
 * This function parses an INI-formatted string and organizes the data into a
 * map, where sections are keys and key-value pairs are stored within each
 * section.
 *
 * @param str The INI-formatted string to parse.
 * @return A map representing the parsed INI data.
 */
static std::map<std::string, std::map<std::string, std::string>> parseIni(
    const std::string& str);

/**
 * @brief Parses an INI file and returns its content as a map of sections and
 * key-value pairs.
 *
 * This function reads the contents of an INI file located at the specified
 * path, parses it into a map structure, where section names are keys and
 * key-value pairs are stored within each section.
 *
 * @param configIniPath The path to the INI file to be parsed.
 * @return A map representing the parsed INI data.
 */
std::map<std::string, std::map<std::string, std::string>>
getParsedDataFromIniFile(const std::string& configIniPath);

/**
 * @brief Parses sections from an INI file and returns them as a list of
 * strings.
 *
 * This function reads an INI file and extracts the section names from it.
 *
 * @param filePath The path to the INI file.
 * @return A vector of section names.
 */
std::vector<std::string> parseSectionsFromIni(const std::string& filePath);

std::string parseValueFromIniSection(const std::string& filePath,
                                     const std::string& sectionName,
                                     const std::string& keyName);

std::string parseValueFromIniSectionF(FILE*& file,
                                      const std::string& filePath,
                                      const std::string& sectionName,
                                      const std::string& keyName);

/**
 * @brief Cleans the formatting of an INI file by removing empty lines and
 * standardizing section formatting.
 *
 * This function takes an INI file located at the specified path, removes empty
 * lines, and standardizes the formatting of sections by ensuring that there is
 * a newline between each section's closing ']' and the next section's opening
 * '['.
 *
 * @param filePath The path to the INI file to be cleaned.
 */
void cleanIniFormatting(const std::string& filePath);

/**
 * @brief Modifies or creates an INI file by adding or updating key-value pairs
 * in the specified section.
 *
 * This function attempts to open the specified INI file for reading. If the
 * file doesn't exist, it creates a new file and adds the specified section and
 * key-value pair. If the file exists, it reads its contents, modifies or adds
 * the key-value pair in the specified section, and saves the changes back to
 * the original file.
 *
 * @param fileToEdit      The path to the INI file to be modified or created.
 * @param desiredSection  The name of the section in which the key-value pair
 * should be added or updated.
 * @param desiredKey      The key for the key-value pair to be added or updated.
 * @param desiredValue    The new value for the key-value pair.
 * @param desiredNewKey   (Optional) If provided, the function will rename the
 * key while preserving the original value.
 */
void setIniFile(const std::string& fileToEdit,
                const std::string& desiredSection,
                const std::string& desiredKey,
                const std::string& desiredValue,
                const std::string& desiredNewKey);

/**
 * @brief Sets the value of a key in an INI file within the specified section
 * and cleans the formatting.
 *
 * This function sets the value of the specified key within the given section of
 * the INI file. If the key or section does not exist, it creates them. After
 * updating the INI file, it cleans the formatting to ensure proper INI file
 * structure.
 *
 * @param fileToEdit      The path to the INI file to be modified or created.
 * @param desiredSection  The name of the section in which the key-value pair
 * should be added or updated.
 * @param desiredKey      The key for the key-value pair to be added or updated.
 * @param desiredValue    The new value for the key-value pair.
 */
void setIniFileValue(const std::string& fileToEdit,
                     const std::string& desiredSection,
                     const std::string& desiredKey,
                     const std::string& desiredValue);

/**
 * @brief Sets the key name to a new name in an INI file within the specified
 * section and cleans the formatting.
 *
 * This function sets the key name to a new name within the given section of the
 * INI file. If the key or section does not exist, it creates them. After
 * updating the INI file, it cleans the formatting to ensure proper INI file
 * structure.
 *
 * @param fileToEdit      The path to the INI file to be modified or created.
 * @param desiredSection  The name of the section in which the key-name change
 * should occur.
 * @param desiredKey      The key name to be changed.
 * @param desiredNewKey   The new key name to replace the original key name.
 */
void setIniFileKey(const std::string& fileToEdit,
                   const std::string& desiredSection,
                   const std::string& desiredKey,
                   const std::string& desiredNewKey);

/**
 * @brief Adds a new section to an INI file.
 *
 * This function adds a new section with the specified name to the INI file
 * located at the specified path. If the section already exists, it does
 * nothing.
 *
 * @param filePath The path to the INI file.
 * @param sectionName The name of the section to add.
 */
void addIniSection(const char* filePath, const char* sectionName);

/**
 * @brief Renames a section in an INI file.
 *
 * This function renames the section with the specified current name to the
 * specified new name in the INI file located at the specified path. If the
 * current section does not exist, or if the new section name already exists, it
 * does nothing.
 *
 * @param filePath The path to the INI file.
 * @param currentSectionName The name of the section to rename.
 * @param newSectionName The new name for the section.
 */
void renameIniSection(const std::string& filePath,
                      const std::string& currentSectionName,
                      const std::string& newSectionName);

/**
 * @brief Removes a section from an INI file.
 *
 * This function removes the section with the specified name, including all its
 * associated key-value pairs, from the INI file located at the specified path.
 * If the section does not exist in the file, it does nothing.
 *
 * @param filePath The path to the INI file.
 * @param sectionName The name of the section to remove.
 */
void removeIniSection(const std::string& filePath,
                      const std::string& sectionName);
} // namespace nikola::utils