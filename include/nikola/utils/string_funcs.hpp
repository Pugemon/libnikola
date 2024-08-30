/********************************************************************************
 * File: string_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains function declarations and utility functions for
 *string manipulation. These functions are used in the Ultrahand Overlay project
 *to perform operations like trimming whitespaces, removing quotes, replacing
 *   multiple slashes, and more.
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

#include <string>
#include <vector>

namespace nikola::utils
{

/**
 * @brief Trims leading and trailing whitespaces from a string.
 *
 * This function removes leading and trailing whitespaces, tabs, newlines,
 * carriage returns, form feeds, and vertical tabs from the input string.
 *
 * @param str The input string to trim.
 * @return The trimmed string.
 */
std::string trim(const std::string& str);

/**
 * @brief Removes all white spaces from a string.
 *
 * This function removes all white spaces, including spaces, tabs, newlines,
 * carriage returns, form feeds, and vertical tabs from the input string.
 *
 * @param str The input string to remove white spaces from.
 * @return The string with white spaces removed.
 */
std::string removeWhiteSpaces(const std::string& str);

/**
 * @brief Removes quotes from a string.
 *
 * This function removes single and double quotes from the beginning and end of
 * the input string.
 *
 * @param str The input string to remove quotes from.
 * @return The string with quotes removed.
 */
std::string removeQuotes(const std::string& str);

/**
 * @brief Replaces multiple consecutive slashes with a single slash in a string.
 *
 * This function replaces sequences of two or more consecutive slashes with a
 * single slash in the input string.
 *
 * @param input The input string to process.
 * @return The string with multiple slashes replaced.
 */
std::string replaceMultipleSlashes(const std::string& input);

/**
 * @brief Removes the leading slash from a string if present.
 *
 * This function removes the leading slash ('/') from the input string if it
 * exists.
 *
 * @param pathPattern The input string representing a path pattern.
 * @return The string with the leading slash removed.
 */
std::string removeLeadingSlash(const std::string& pathPattern);

/**
 * @brief Removes the trailing slash from a string if present.
 *
 * This function removes the trailing slash ('/') from the input string if it
 * exists.
 *
 * @param pathPattern The input string representing a path pattern.
 * @return The string with the trailing slash removed.
 */
std::string removeEndingSlash(const std::string& pathPattern);

/**
 * @brief Preprocesses a path string by replacing multiple slashes and adding
 * "sdmc:" prefix.
 *
 * This function preprocesses a path string by removing multiple consecutive
 * slashes, adding the "sdmc:" prefix if not present, and returning the
 * resulting string.
 *
 * @param path The input path string to preprocess.
 * @return The preprocessed path string.
 */
std::string preprocessPath(const std::string& path);

/**
 * @brief Drops the file extension from a filename.
 *
 * This function removes the file extension (characters after the last dot) from
 * the input filename string.
 *
 * @param filename The input filename from which to drop the extension.
 * @return The filename without the extension.
 */
std::string dropExtension(const std::string& filename);

/**
 * @brief Checks if a string starts with a given prefix.
 *
 * This function checks if the input string starts with the specified prefix.
 *
 * @param str The input string to check.
 * @param prefix The prefix to check for.
 * @return True if the string starts with the prefix, false otherwise.
 */
bool startsWith(const std::string& str, const std::string& prefix);

/**
 * @brief Checks if a path points to a directory.
 *
 * This function checks if the specified path points to a directory.
 *
 * @param path The path to check.
 * @return True if the path is a directory, false otherwise.
 */
bool isDirectory(const std::string& path);

/**
 * @brief Checks if a path points to a file or directory.
 *
 * This function checks if the specified path points to either a file or a
 * directory.
 *
 * @param path The path to check.
 * @return True if the path points to a file or directory, false otherwise.
 */
bool isFileOrDirectory(const std::string& path);

/**
 * @brief Splits a string into a vector of strings using a delimiter.
 *
 * This function splits the input string into multiple strings using the
 * specified delimiter.
 *
 * @param str The input string to split.
 * @return A vector of strings containing the split values.
 */
std::vector<std::string> stringToList(const std::string& str);
}  // namespace nikola::utils
