//
// Created by pugemon on 30.08.24.
//

#include "nikola/tesla/impl.hpp"
#include <switch/kernel/svc.h>
#include "nikola/tesla.hpp"

namespace nikola::tsl::impl
{

void parseOverlaySettings(u64& launchCombo)
{
  FILE* configFile = fopen("sdmc:/config/tesla/config.ini", "r");

  if (configFile == nullptr)
    return;

  fseek(configFile, 0, SEEK_END);
  size_t configFileSize = ftell(configFile);
  rewind(configFile);

  std::string configFileData(configFileSize, '\0');
  fread(&configFileData[0], sizeof(char), configFileSize, configFile);
  fclose(configFile);

  hlp::ini::IniData parsedConfig = hlp::ini::parseIni(configFileData);

  launchCombo = 0x00;
  size_t max_combo = 4;
  for (std::string key : hlp::split(parsedConfig["tesla"]["key_combo"], '+')) {
    launchCombo |= hlp::stringToKeyCode(key);
    if (!--max_combo) {
      return;
    }
  }
}

void homeButtonDetector(void* args)
{
  SharedThreadData* shData = static_cast<SharedThreadData*>(args);

  // To prevent focus glitchout, close the overlay immediately when the home
  // button gets pressed
  hidsysAcquireHomeButtonEventHandle(&shData->homeButtonPressEvent, false);

  while (shData->running) {
    if (R_SUCCEEDED(eventWait(&shData->homeButtonPressEvent, 100'000'000))) {
      eventClear(&shData->homeButtonPressEvent);

      if (shData->overlayOpen) {
        tsl::Overlay::get()->hide();
        shData->overlayOpen = false;
      }
    }
  }
}
void powerButtonDetector(void* args)
{
  SharedThreadData* shData = static_cast<SharedThreadData*>(args);

  // To prevent focus glitchout, close the overlay immediately when the power
  // button gets pressed
  hidsysAcquireSleepButtonEventHandle(&shData->powerButtonPressEvent, false);

  while (shData->running) {
    if (R_SUCCEEDED(eventWait(&shData->powerButtonPressEvent, 100'000'000))) {
      eventClear(&shData->powerButtonPressEvent);

      if (shData->overlayOpen) {
        tsl::Overlay::get()->hide();
        shData->overlayOpen = false;
      }
    }
  }
}



template<impl::LaunchFlags launchFlags>
void hidInputPoller(void* args)
{
  SharedThreadData* shData = static_cast<SharedThreadData*>(args);

  // Parse Tesla settings
  impl::parseOverlaySettings(shData->launchCombo);

  padInitializeAny(&pad);

  hidInitializeTouchScreen();

  // Drop all inputs from the previous overlay
  padUpdate(&pad);

  while (shData->running) {
    // Scan for input changes
    padUpdate(&pad);

    // Read in HID values
    {
      std::scoped_lock lock(shData->dataMutex);

      shData->keysDown = padGetButtonsDown(&pad);
      shData->keysHeld = padGetButtons(&pad);
      shData->joyStickPosLeft = padGetStickPos(&pad, 0);
      shData->joyStickPosRight = padGetStickPos(&pad, 1);

      // Read in touch positions
      if (hidGetTouchScreenStates(&shData->touchState, 1) == 0)
        shData->touchState = {0};

      if (((shData->keysHeld & shData->launchCombo) == shData->launchCombo)
          && shData->keysDown & shData->launchCombo)
      {
        if (shData->overlayOpen) {
          tsl::Overlay::get()->hide();
          shData->overlayOpen = false;
        } else
          eventFire(&shData->comboEvent);
      }

      shData->keysDownPending |= shData->keysDown;
    }

    // 20 ms
    svcSleepThread(20E6);
  }
}
}