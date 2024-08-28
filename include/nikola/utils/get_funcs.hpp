/********************************************************************************
 * File: get_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains functions for retrieving information and data
 *   from various sources, including file system and JSON files. It includes
 *   functions for obtaining overlay module information, reading file contents,
 *   and parsing JSON data.
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
 * @brief Reads the contents of a file and returns it as a string.
 *
 * @param filePath The path to the file to be read.
 * @return The content of the file as a string.
 */
std::string getFileContents(const std::string& filePath);

/**
 * @brief Concatenates the provided directory and file names to form a
 * destination path.
 *
 * @param destinationDir The directory where the file should be placed.
 * @param fileName The name of the file.
 * @return The destination path as a string.
 */
std::string getDestinationPath(const std::string& destinationDir,
                               const std::string& fileName);

/**
 * @brief Extracts the value part from a string line containing a key-value
 * pair.
 *
 * @param line The string line containing a key-value pair (e.g., "key=value").
 * @return The extracted value as a string. If no value is found, an empty
 * string is returned.
 */
std::string getValueFromLine(const std::string& line);

/**
 * @brief Extracts the name from a file path, including handling directories.
 *
 * @param path The file path from which to extract the name.
 * @return The extracted name as a string. If the path indicates a directory, it
 * extracts the last directory name. If the path is empty or no name is found,
 * an empty string is returned.
 */
std::string getNameFromPath(const std::string& path);

/**
 * @brief Extracts the file name from a URL.
 *
 * @param url The URL from which to extract the file name.
 * @return The extracted file name.
 */
std::string getFileNameFromURL(const std::string& url);

/**
 * @brief Extracts the name of the parent directory from a given file path.
 *
 * @param path The file path from which to extract the parent directory name.
 * @return The parent directory name.
 */
std::string getParentDirNameFromPath(const std::string& path);

/**
 * @brief Extracts the parent directory path from a given file path.
 *
 * @param path The file path from which to extract the parent directory path.
 * @return The parent directory path.
 */
std::string getParentDirFromPath(const std::string& path);

/**
 * @brief Gets a list of subdirectories in a directory.
 *
 * @param directoryPath The path of the directory to search.
 * @return A vector of strings containing the names of subdirectories.
 */
std::vector<std::string> getSubdirectories(const std::string& directoryPath);

/**
 * @brief Recursively retrieves a list of files from a directory.
 *
 * @param directoryPath The path of the directory to search.
 * @return A vector of strings containing the paths of the files.
 */
std::vector<std::string> getFilesListFromDirectory(
    const std::string& directoryPath);

/**
 * @brief Gets a list of files and folders based on a wildcard pattern.
 *
 * @param pathPattern The wildcard pattern to match files and folders.
 * @return A vector of strings containing the paths of matching files and
 * folders.
 */
std::vector<std::string> getFilesListByWildcard(const std::string& pathPattern);

/**
 * @brief Gets a list of files and folders based on a wildcard pattern.
 *
 * This function searches for files and folders in a directory that match the
 * specified wildcard pattern.
 *
 * @param pathPattern The wildcard pattern to match files and folders.
 * @return A vector of strings containing the paths of matching files and
 * folders.
 */
std::vector<std::string> getFilesListByWildcards(
    const std::string& pathPattern);

}  // namespace nikola::utils::get
