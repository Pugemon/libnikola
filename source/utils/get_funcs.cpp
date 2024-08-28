//
// Created by pugemon on 27.08.24.
//
#include "nikola/utils/get_funcs.hpp"

#include <dirent.h>
#include <fnmatch.h>
#include <cstdio>
#include <sys/stat.h>
#include <algorithm>
#include <string>

#include "nikola/utils/string_funcs.hpp"

namespace nikola::utils
{

std::string getFileContents(const std::string& filePath)
{
  std::string content;
  FILE* file = fopen(filePath.c_str(), "rb");
  if (file) {
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) == 0 && fileInfo.st_size > 0) {
      content.resize(fileInfo.st_size);
      fread(&content[0], 1, fileInfo.st_size, file);
    }
    fclose(file);

    // Normalize line endings to '\n'
    content.erase(std::remove(content.begin(), content.end(), '\r'),
                  content.end());
  }
  return content;
}

std::string getDestinationPath(const std::string& destinationDir,
                               const std::string& fileName)
{
  return destinationDir + "/" + fileName;
}

std::string getValueFromLine(const std::string& line)
{
  std::size_t equalsPos = line.find('=');
  if (equalsPos != std::string::npos) {
    std::string value = line.substr(equalsPos + 1);
    return trim(value);
  }
  return "";
}

std::string getNameFromPath(const std::string& path)
{
  size_t lastSlash = path.find_last_of('/');
  if (lastSlash != std::string::npos) {
    std::string name = path.substr(lastSlash + 1);
    if (name.empty()) {
      // The path ends with a slash, indicating a directory
      std::string strippedPath = path.substr(0, lastSlash);
      lastSlash = strippedPath.find_last_of('/');
      if (lastSlash != std::string::npos) {
        name = strippedPath.substr(lastSlash + 1);
      }
    }
    return name;
  }
  return path;
}

std::string getFileNameFromURL(const std::string& url)
{
  size_t lastSlash = url.find_last_of('/');
  if (lastSlash != std::string::npos)
    return url.substr(lastSlash + 1);
  return "";
}

std::string getParentDirNameFromPath(const std::string& path)
{
  // Find the position of the last occurrence of the directory separator '/'
  std::size_t lastSlashPos = removeEndingSlash(path).rfind('/');

  // Check if the slash is found and not at the beginning of the path
  if (lastSlashPos != std::string::npos && lastSlashPos != 0) {
    // Find the position of the second last occurrence of the directory
    // separator '/'
    std::size_t secondLastSlashPos = path.rfind('/', lastSlashPos - 1);

    // Check if the second last slash is found
    if (secondLastSlashPos != std::string::npos) {
      // Extract the substring between the second last and last slashes
      std::string subPath = path.substr(secondLastSlashPos + 1,
                                        lastSlashPos - secondLastSlashPos - 1);

      // Check if the substring contains spaces or special characters
      if (subPath.find_first_of(" \t\n\r\f\v") != std::string::npos) {
        // If it does, return the substring within quotes
        return "\"" + subPath + "\"";
      }

      // If it doesn't, return the substring as is
      return subPath;
    }
  }

  // If the path format is not as expected or the parent directory is not found,
  // return an empty string or handle the case accordingly
  return "";
}

std::string getParentDirFromPath(const std::string& path)
{
  size_t lastSlash = path.find_last_of('/');
  if (lastSlash != std::string::npos) {
    std::string parentDir = path.substr(0, lastSlash + 1);
    return parentDir;
  }
  return path;
}

std::vector<std::string> getSubdirectories(const std::string& directoryPath)
{
  std::vector<std::string> subdirectories;

  DIR* dir = opendir(directoryPath.c_str());
  if (dir != nullptr) {
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      std::string entryName = entry->d_name;

      // Exclude current directory (.) and parent directory (..)
      if (entryName != "." && entryName != "..") {
        struct stat entryStat;
        std::string fullPath = directoryPath + "/" + entryName;

        if (stat(fullPath.c_str(), &entryStat) == 0
            && S_ISDIR(entryStat.st_mode))
        {
          subdirectories.push_back(entryName);
        }
      }
    }

    closedir(dir);
  }

  return subdirectories;
}

std::vector<std::string> getFilesListFromDirectory(
    const std::string& directoryPath)
{
  std::vector<std::string> fileList;

  DIR* dir = opendir(directoryPath.c_str());
  if (dir != nullptr) {
    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      std::string entryName = entry->d_name;
      std::string entryPath = directoryPath;
      if (entryPath.back() != '/')
        entryPath += '/';
      entryPath += entryName;

      // Skip directories "." and ".."
      if (entryName != "." && entryName != "..") {
        if (isDirectory(entryPath)) {
          // Recursively retrieve files from subdirectories
          std::vector<std::string> subDirFiles =
              getFilesListFromDirectory(entryPath);
          fileList.insert(
              fileList.end(), subDirFiles.begin(), subDirFiles.end());
        } else {
          fileList.push_back(entryPath);
        }
      }
    }
    closedir(dir);
  }

  return fileList;
}

std::vector<std::string> getFilesListByWildcard(const std::string& pathPattern)
{
  std::string dirPath = "";
  std::string wildcard = "";

  std::size_t wildcardPos = pathPattern.find('*');
  if (wildcardPos != std::string::npos) {
    std::size_t slashPos = pathPattern.rfind('/', wildcardPos);

    if (slashPos != std::string::npos) {
      dirPath = pathPattern.substr(0, slashPos + 1);
      wildcard = pathPattern.substr(slashPos + 1);
    } else {
      dirPath = "";
      wildcard = pathPattern;
    }
  } else {
    dirPath = pathPattern + "/";
  }

  // logMessage("dirPath: " + dirPath);
  // logMessage("wildcard: " + wildcard);

  std::vector<std::string> fileList;

  bool isFolderWildcard = wildcard.back() == '/';
  if (isFolderWildcard) {
    wildcard =
        wildcard.substr(0, wildcard.size() - 1);  // Remove the trailing slash
  }

  // logMessage("isFolderWildcard: " + std::to_string(isFolderWildcard));

  DIR* dir = opendir(dirPath.c_str());
  if (dir != nullptr) {
    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      std::string entryName = entry->d_name;
      std::string entryPath = dirPath + entryName;

      bool isEntryDirectory = isDirectory(entryPath);

      // logMessage("entryName: " + entryName);
      // logMessage("entryPath: " + entryPath);
      // logMessage("isFolderWildcard: " + std::to_string(isFolderWildcard));
      // logMessage("isEntryDirectory: " + std::to_string(isEntryDirectory));

      if (isFolderWildcard && isEntryDirectory
          && fnmatch(wildcard.c_str(), entryName.c_str(), FNM_NOESCAPE) == 0)
      {
        if (entryName != "." && entryName != "..") {
          fileList.push_back(entryPath + "/");
        }
      } else if (!isFolderWildcard && !isEntryDirectory) {
        std::size_t wildcardPos = wildcard.find('*');
        if (wildcardPos != std::string::npos) {
          std::string prefix = wildcard.substr(0, wildcardPos);
          if (entryName.find(prefix) == 0) {
            std::string suffix = wildcard.substr(wildcardPos + 1);
            if (entryName.size() >= suffix.size()
                && entryName.compare(
                       entryName.size() - suffix.size(), suffix.size(), suffix)
                    == 0)
            {
              fileList.push_back(entryPath);
            }
          }
        } else if (fnmatch(wildcard.c_str(), entryName.c_str(), FNM_NOESCAPE)
                   == 0)
        {
          fileList.push_back(entryPath);
        }
      }
    }
    closedir(dir);
  }

  // std::string fileListAsString;
  // for (const std::string& filePath : fileList) {
  //     fileListAsString += filePath + "\n";
  // }
  // logMessage("File List:\n" + fileListAsString);

  return fileList;
}

std::vector<std::string> getFilesListByWildcards(const std::string& pathPattern)
{
  std::vector<std::string> fileList;

  // Check if the pattern contains multiple wildcards
  std::size_t wildcardPos = pathPattern.find('*');
  if (wildcardPos != std::string::npos
      && pathPattern.find('*', wildcardPos + 1) != std::string::npos)
  {
    std::string dirPath = "";
    std::string wildcard = "";

    // Extract the directory path and the first wildcard
    std::size_t slashPos = pathPattern.rfind('/', wildcardPos);
    if (slashPos != std::string::npos) {
      dirPath = pathPattern.substr(0, slashPos + 1);
      wildcard = pathPattern.substr(slashPos + 1, wildcardPos - slashPos - 1);
    } else {
      dirPath = "";
      wildcard = pathPattern.substr(0, wildcardPos);
    }

    // Get the list of directories matching the first wildcard
    std::vector<std::string> subDirs =
        getFilesListByWildcard(dirPath + wildcard + "*/");

    // Process each subdirectory recursively
    for (const std::string& subDir : subDirs) {
      std::string subPattern =
          subDir + removeLeadingSlash(pathPattern.substr(wildcardPos + 1));
      std::vector<std::string> subFileList =
          getFilesListByWildcards(subPattern);
      fileList.insert(fileList.end(), subFileList.begin(), subFileList.end());
    }
  } else {
    // Only one wildcard present, use getFilesListByWildcard directly
    fileList = getFilesListByWildcard(pathPattern);
  }

  return fileList;
}

}