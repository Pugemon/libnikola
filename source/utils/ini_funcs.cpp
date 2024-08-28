//
// Created by pugemon on 28.08.24.
//
#include <algorithm>  // For std::remove_if
#include <cctype>  // For ::isspace
#include <cstdio>  // For FILE*, fopen(), fclose(), fprintf(), etc.
#include <sstream>  // For std::istringstream

#include "nikola/utils/ini_funcs.hpp"

#include "nikola/utils/get_funcs.hpp"
#include "nikola/utils/string_funcs.hpp"

namespace nikola::utils
{

std::vector<std::string> split(const std::string& str, char delim)
{
  std::vector<std::string> out;

  std::size_t current, previous = 0;
  current = str.find(delim);
  while (current != std::string::npos) {
    out.push_back(str.substr(previous, current - previous));
    previous = current + 1;
    current = str.find(delim, previous);
  }
  out.push_back(str.substr(previous, current - previous));

  return out;
}

std::map<std::string, std::map<std::string, std::string>> parseIni(
    const std::string& str)
{
  std::map<std::string, std::map<std::string, std::string>> iniData;

  auto lines = split(str, '\n');

  std::string lastHeader = "";
  for (auto& line : lines) {
    line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

    if (line[0] == '[' && line[line.size() - 1] == ']') {
      lastHeader = line.substr(1, line.size() - 2);
      iniData.emplace(lastHeader, std::map<std::string, std::string> {});
    } else if (auto keyValuePair = split(line, '='); keyValuePair.size() == 2) {
      iniData[lastHeader].emplace(keyValuePair[0], keyValuePair[1]);
    }
  }

  return iniData;
}

[[maybe_unused]] std::map<std::string, std::map<std::string, std::string>>
getParsedDataFromIniFile(const std::string& configIniPath)
{
  std::map<std::string, std::map<std::string, std::string>> parsedData;
  std::string currentSection = "";  // Initialize the current section as empty

  FILE* configFileIn = fopen(configIniPath.c_str(), "rb");
  if (!configFileIn) {
    return parsedData;
  }

  // Determine the size of the INI file
  fseek(configFileIn, 0, SEEK_END);
  long fileSize = ftell(configFileIn);
  rewind(configFileIn);

  // Read the contents of the INI file
  char* fileData = new char[fileSize + 1];
  fread(fileData, sizeof(char), fileSize, configFileIn);
  fileData[fileSize] = '\0';  // Add null-terminator to create a C-string
  fclose(configFileIn);

  // Parse the INI data
  std::string fileDataString(fileData, fileSize);

  // Normalize line endings to \n
  fileDataString.erase(
      std::remove(fileDataString.begin(), fileDataString.end(), '\r'),
      fileDataString.end());

  // Split lines and parse
  std::istringstream fileStream(fileDataString);
  std::string line;
  while (std::getline(fileStream, line)) {
    // Remove leading and trailing whitespace
    line = trim(line);

    // Check if this line is a section
    if (line.size() > 2 && line.front() == '[' && line.back() == ']') {
      // Remove the brackets to get the section name
      currentSection = line.substr(1, line.size() - 2);
    } else {
      // If not a section, parse as key-value pair
      size_t delimiterPos = line.find('=');
      if (delimiterPos != std::string::npos) {
        std::string key = trim(line.substr(0, delimiterPos));
        std::string value = trim(line.substr(delimiterPos + 1));

        // Store in the current section
        parsedData[currentSection][key] = value;
      }
    }
  }

  delete[] fileData;

  return parsedData;
}

[[maybe_unused]] std::vector<std::string> parseSectionsFromIni(
    const std::string& filePath)
{
  std::vector<std::string> sections;

  FILE* file = fopen(filePath.c_str(), "r");
  if (file == nullptr) {
    return sections;  // Return an empty list if the file cannot be opened
  }

  constexpr size_t BufferSize = 131072;
  char line[BufferSize];
  while (fgets(line, sizeof(line), file)) {
    std::string trimmedLine = trim(std::string(line));

    if (!trimmedLine.empty() && trimmedLine[0] == '['
        && trimmedLine.back() == ']')
    {
      // Extract section name and add it to the list
      std::string sectionName = trimmedLine.substr(1, trimmedLine.size() - 2);
      sections.push_back(sectionName);
    }
  }

  fclose(file);
  return sections;
}
std::string parseValueFromIniSection(const std::string& filePath,
                                     const std::string& sectionName,
                                     const std::string& keyName)
{
  std::string value = "";

  FILE* file = fopen(filePath.c_str(), "r");
  if (file == nullptr) {
    return value;  // Return an empty string if the file cannot be opened
  }

  std::string currentSection = "";
  constexpr size_t BufferSize = 131072;
  char line[BufferSize];

  while (fgets(line, sizeof(line), file)) {
    std::string trimmedLine = trim(std::string(line));

    if (!trimmedLine.empty()) {
      if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
        // This line is a section header
        currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
      } else if (currentSection == sectionName) {
        // Check if the line is within the desired section and contains the
        // desired key
        size_t delimiterPos = trimmedLine.find('=');
        if (delimiterPos != std::string::npos) {
          std::string currentKey = trim(trimmedLine.substr(0, delimiterPos));
          if (currentKey == keyName) {
            value = trim(trimmedLine.substr(delimiterPos + 1));
            break;  // Found the key, exit the loop
          }
        }
      }
    }
  }

  fclose(file);

  return value;
}
[[maybe_unused]] std::string parseValueFromIniSectionF(
    FILE*& file,
    const std::string& filePath,
    const std::string& sectionName,
    const std::string& keyName)
{
  std::string value = "";

  // FILE* file = fopen(filePath.c_str(), "r");
  if (file == nullptr) {
    return value;  // Return an empty string if the file cannot be opened
  }

  std::string currentSection = "";
  constexpr size_t BufferSize = 131072;
  char line[BufferSize];

  while (fgets(line, sizeof(line), file)) {
    std::string trimmedLine = trim(std::string(line));

    if (!trimmedLine.empty()) {
      if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
        // This line is a section header
        currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
      } else if (currentSection == sectionName) {
        // Check if the line is within the desired section and contains the
        // desired key
        size_t delimiterPos = trimmedLine.find('=');
        if (delimiterPos != std::string::npos) {
          std::string currentKey = trim(trimmedLine.substr(0, delimiterPos));
          if (currentKey == keyName) {
            value = trim(trimmedLine.substr(delimiterPos + 1));
            break;  // Found the key, exit the loop
          }
        }
      }
    }
  }

  // fclose(file);

  return value;
}
[[maybe_unused]] void cleanIniFormatting(const std::string& filePath)
{
  FILE* inputFile = fopen(filePath.c_str(), "r");
  if (!inputFile) {
    // Failed to open the input file
    // Handle the error accordingly
    return;
  }

  std::string tempPath = filePath + ".tmp";
  FILE* outputFile = fopen(tempPath.c_str(), "w");
  if (!outputFile) {
    // Failed to create the output file
    // Handle the error accordingly
    fclose(inputFile);
    return;
  }

  bool isNewSection = false;

  char line[4096];
  while (fgets(line, sizeof(line), inputFile)) {
    std::string trimmedLine = trim(std::string(line));

    if (!trimmedLine.empty()) {
      if (trimmedLine[0] == '[' && trimmedLine[trimmedLine.length() - 1] == ']')
      {
        if (isNewSection) {
          fprintf(outputFile, "\n");
        }
        isNewSection = true;
      }

      fprintf(outputFile, "%s\n", trimmedLine.c_str());
    }
  }

  fclose(inputFile);
  fclose(outputFile);

  // Remove the original file and rename the temp file
  remove(filePath.c_str());
  rename(tempPath.c_str(), filePath.c_str());
}
void setIniFile(const std::string& fileToEdit,
                const std::string& desiredSection,
                const std::string& desiredKey,
                const std::string& desiredValue,
                const std::string& desiredNewKey)
{
  FILE* configFile = fopen(fileToEdit.c_str(), "r");
  if (!configFile) {
    configFile = fopen(fileToEdit.c_str(), "w");
    if (!configFile) {
      // Handle the error accordingly
      return;
    }
    fprintf(configFile,
            "[%s]\n%s = %s\n",
            desiredSection.c_str(),
            desiredKey.c_str(),
            desiredValue.c_str());
    fclose(configFile);
    return;
  }

  // Create a buffer to store the updated content
  std::string updatedContent;
  std::string currentSection;
  std::string formattedDesiredValue = trim(desiredValue);
  char line[131072];

  bool sectionFound = false;
  bool keyFound = false;
  bool addNewLine = false;

  while (fgets(line, sizeof(line), configFile)) {
    std::string trimmedLine = trim(line);

    if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
      currentSection =
          removeQuotes(trimmedLine.substr(1, trimmedLine.length() - 2));
      if (sectionFound && !keyFound && desiredNewKey.empty()) {
        if (!updatedContent.empty()
            && updatedContent.substr(updatedContent.length() - 2) == "\n\n")
        {
          updatedContent =
              updatedContent.substr(0, updatedContent.length() - 1);
          addNewLine = true;
        }
        updatedContent += desiredKey + " = " + formattedDesiredValue + "\n";

        if (addNewLine) {  // if it ended with \n\n, add one more newline
          updatedContent += "\n";
          addNewLine = false;
        }
        keyFound = true;
      }
    }

    if (sectionFound && !keyFound && desiredNewKey.empty()
        && trim(currentSection) != trim(desiredSection))
    {
      if (!updatedContent.empty()
          && updatedContent.substr(updatedContent.length() - 2) == "\n\n")
      {
        updatedContent = updatedContent.substr(0, updatedContent.length() - 1);
        addNewLine = true;
      }
      updatedContent += desiredKey + " = " + formattedDesiredValue + "\n";
      // Add a newline character if the last part of updatedContent initially
      // had "\n"
      if (addNewLine) {
        updatedContent += "\n";
        addNewLine = false;
      }
      keyFound = true;
    }

    if (trim(currentSection) == trim(desiredSection)) {
      sectionFound = true;
      std::string::size_type delimiterPos = trimmedLine.find('=');

      if (delimiterPos != std::string::npos) {
        std::string lineKey = trim(trimmedLine.substr(0, delimiterPos));

        if (lineKey == desiredKey) {
          keyFound = true;
          std::string originalValue = getValueFromLine(trimmedLine);
          if (!updatedContent.empty()
              && updatedContent.substr(updatedContent.length() - 2) == "\n\n")
          {
            updatedContent =
                updatedContent.substr(0, updatedContent.length() - 1);
            addNewLine = true;
          }

          if (!desiredNewKey.empty()) {
            updatedContent += desiredNewKey + " = " + originalValue + "\n";
          } else {
            updatedContent += desiredKey + " = " + formattedDesiredValue + "\n";
          }

          // Add a newline character if the last part of updatedContent
          // initially had "\n"
          if (addNewLine) {
            updatedContent += "\n";
            addNewLine = false;
          }
          continue;
        }
      }
    }

    updatedContent += line;
  }

  if (sectionFound && !keyFound && desiredNewKey.empty()) {
    if (!updatedContent.empty()
        && updatedContent.substr(updatedContent.length() - 2) == "\n\n")
    {
      updatedContent = updatedContent.substr(0, updatedContent.length() - 1);
      addNewLine = true;
    }
    updatedContent += desiredKey + " = " + formattedDesiredValue + "\n";
    // Add a newline character if the last part of updatedContent initially had
    // "\n"
    if (addNewLine) {
      updatedContent += "\n";
      addNewLine = false;
    }
  }

  if (!sectionFound && !keyFound && desiredNewKey.empty()) {
    updatedContent += "\n[" + desiredSection + "]\n" + desiredKey + " = "
        + formattedDesiredValue + "\n";
  }

  fclose(configFile);

  // Reopen the original file for writing and overwrite its content
  configFile = fopen(fileToEdit.c_str(), "w");
  if (!configFile) {
    // Handle the error accordingly
    return;
  }
  fprintf(configFile, "%s", updatedContent.c_str());
  fclose(configFile);
}
[[maybe_unused]] void setIniFileValue(const std::string& fileToEdit,
                                      const std::string& desiredSection,
                                      const std::string& desiredKey,
                                      const std::string& desiredValue)
{
  setIniFile(fileToEdit, desiredSection, desiredKey, desiredValue, "");
  // cleanIniFormatting(fileToEdit);
}
[[maybe_unused]] void setIniFileKey(const std::string& fileToEdit,
                                    const std::string& desiredSection,
                                    const std::string& desiredKey,
                                    const std::string& desiredNewKey)
{
  setIniFile(fileToEdit, desiredSection, desiredKey, "", desiredNewKey);
  // cleanIniFormatting(fileToEdit);
}
[[maybe_unused]] void addIniSection(const char* filePath,
                                    const char* sectionName)
{
  if (!isFileOrDirectory(filePath)) {
    // INI file doesn't exist, handle the error accordingly
    // std::cerr << "Error: INI file not found." << std::endl;
    return;
  }

  // Read the existing contents of the INI file
  FILE* inputFile = fopen(filePath, "r");
  if (!inputFile) {
    // std::cerr << "Error: Failed to open INI file for reading." << std::endl;
    return;
  }

  FILE* tempFile = fopen("temp.ini", "w");
  if (!tempFile) {
    // std::cerr << "Error: Failed to create a temporary file." << std::endl;
    fclose(inputFile);
    return;
  }

  constexpr size_t BufferSize = 131072;
  char line[BufferSize];
  bool sectionExists = false;
  while (fgets(line, sizeof(line), inputFile)) {
    // Check if the line contains the section
    if (line[0] == '['
        && strncmp(&line[1], sectionName, strlen(sectionName)) == 0)
    {
      sectionExists = true;
      break;  // Section already exists, no need to continue
    }
    fputs(line, tempFile);
  }

  if (!sectionExists) {
    // Section doesn't exist, add it
    fprintf(tempFile, "[%s]\n", sectionName);
  }

  // Copy the rest of the input file to the temp file
  while (fgets(line, sizeof(line), inputFile)) {
    fputs(line, tempFile);
  }

  fclose(inputFile);
  fclose(tempFile);

  // Replace the original file with the temp file
  remove(filePath);  // Delete the old configuration file
  rename("temp.ini", filePath);  // Rename the temp file to the original name

  // std::cout << "Section '" << sectionName << "' added to the INI file." <<
  // std::endl;
}
[[maybe_unused]] void renameIniSection(const std::string& filePath,
                                       const std::string& currentSectionName,
                                       const std::string& newSectionName)
{
  FILE* configFile = fopen(filePath.c_str(), "r");
  if (!configFile) {
    // The INI file doesn't exist, handle the error accordingly
    return;
  }

  std::string tempPath = filePath + ".tmp";
  FILE* tempFile = fopen(tempPath.c_str(), "w");
  if (!tempFile) {
    // Failed to create a temporary file, handle the error accordingly
    fclose(configFile);
    return;
  }

  std::string currentSection;
  bool renaming = false;
  constexpr size_t BufferSize = 131072;
  char line[BufferSize];

  while (fgets(line, sizeof(line), configFile)) {
    std::string currentLine(trim(std::string(line)));

    // Check if the line represents a section
    if (currentLine.length() > 2 && currentLine.front() == '['
        && currentLine.back() == ']')
    {
      std::string sectionName = currentLine.substr(1, currentLine.size() - 2);

      if (sectionName == currentSectionName) {
        // We found the section to rename
        fprintf(tempFile, "[%s]\n", newSectionName.c_str());
        renaming = true;
      } else {
        // Copy the line as is
        fprintf(tempFile, "%s", currentLine.c_str());
        renaming = false;
      }
    } else if (renaming) {
      // Rename the section in the following lines
      fprintf(tempFile, "[%s]\n", newSectionName.c_str());
      renaming = false;
    } else {
      // Copy the line as is
      fprintf(tempFile, "%s", currentLine.c_str());
    }
  }

  fclose(configFile);
  fclose(tempFile);

  // Replace the original file with the temp file
  if (remove(filePath.c_str()) != 0) {
    // Failed to delete the original file, handle the error accordingly
    return;
  }

  if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
    // Failed to rename the temp file, handle the error accordingly
  }
}
[[maybe_unused]] void removeIniSection(const std::string& filePath,
                                       const std::string& sectionName)
{
  FILE* configFile = fopen(filePath.c_str(), "r");
  if (!configFile) {
    // The INI file doesn't exist, or there was an error opening it.
    // Handle the error accordingly or return.
    return;
  }

  std::string tempPath = filePath + ".tmp";
  FILE* tempFile = fopen(tempPath.c_str(), "w");
  if (!tempFile) {
    // Failed to create a temporary file, handle the error accordingly
    fclose(configFile);
    // Handle the error or return.
    return;
  }

  std::string currentSection;
  bool removing = false;
  constexpr size_t BufferSize = 131072;
  char line[BufferSize];

  while (fgets(line, sizeof(line), configFile)) {
    std::string currentLine(trim(std::string(line)));

    // Check if the line represents a section
    if (currentLine.length() > 2 && currentLine.front() == '['
        && currentLine.back() == ']')
    {
      std::string section = currentLine.substr(1, currentLine.size() - 2);

      if (section == sectionName) {
        // We found the section to remove, so skip it and associated key-value
        // pairs
        removing = true;
      } else {
        // Keep other sections
        fprintf(tempFile, "%s\n", currentLine.c_str());
        removing = false;
      }
    } else if (!removing) {
      // Keep lines outside the section
      fprintf(tempFile, "%s\n", currentLine.c_str());
    }
  }

  fclose(configFile);
  fclose(tempFile);

  // Replace the original file with the temp file
  if (remove(filePath.c_str()) != 0) {
    // Failed to delete the original file, handle the error accordingly
    return;
  }

  if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
    // Failed to rename the temp file, handle the error accordingly
  }
}
}