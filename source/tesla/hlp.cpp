//
// Created by pugemon on 28.08.24.
//

#include "nikola/tesla/hlp.hpp"
#include "nikola/tesla.hpp"

#include <functional>

namespace nikola::tsl::hlp
{

void doWithSmSession(std::function<void()> f)
{
  smInitialize();
  f();
  smExit();
}

Result hidsysEnableAppletToGetInput(bool enable, u64 aruid)
{
  const struct
  {
    u8 permitInput;
    u64 appletResourceUserId;
  } in = {enable != 0, aruid};

  return serviceDispatchIn(hidsysGetServiceSession(), 503, in);
}

Result viAddToLayerStack(ViLayer* layer, ViLayerStack stack)
{
  const struct
  {
    u32 stack;
    u64 layerId;
  } in = {stack, layer->layer_id};

  return serviceDispatchIn(viGetSession_IManagerDisplayService(), 6000, in);
}

void requestForeground(bool enabled)
{
  u64 applicationAruid = 0, appletAruid = 0;

  for (u64 programId = 0x0100000000001000ul; programId < 0x0100000000001020ul;
       programId++)
  {
    pmdmntGetProcessId(&appletAruid, programId);

    if (appletAruid != 0)
      hidsysEnableAppletToGetInput(!enabled, appletAruid);
  }

  pmdmntGetApplicationProcessId(&applicationAruid);
  hidsysEnableAppletToGetInput(!enabled, applicationAruid);

  hidsysEnableAppletToGetInput(true, 0);
}
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
u64 stringToKeyCode(std::string& value)
{
  if (strcasecmp(value.c_str(), "A") == 0)
    return KEY_A;
  else if (strcasecmp(value.c_str(), "B") == 0)
    return KEY_B;
  else if (strcasecmp(value.c_str(), "X") == 0)
    return KEY_X;
  else if (strcasecmp(value.c_str(), "Y") == 0)
    return KEY_Y;
  else if (strcasecmp(value.c_str(), "LS") == 0)
    return KEY_LSTICK;
  else if (strcasecmp(value.c_str(), "RS") == 0)
    return KEY_RSTICK;
  else if (strcasecmp(value.c_str(), "L") == 0)
    return KEY_L;
  else if (strcasecmp(value.c_str(), "R") == 0)
    return KEY_R;
  else if (strcasecmp(value.c_str(), "ZL") == 0)
    return KEY_ZL;
  else if (strcasecmp(value.c_str(), "ZR") == 0)
    return KEY_ZR;
  else if (strcasecmp(value.c_str(), "PLUS") == 0)
    return KEY_PLUS;
  else if (strcasecmp(value.c_str(), "MINUS") == 0)
    return KEY_MINUS;
  else if (strcasecmp(value.c_str(), "DLEFT") == 0)
    return KEY_DLEFT;
  else if (strcasecmp(value.c_str(), "DUP") == 0)
    return KEY_DUP;
  else if (strcasecmp(value.c_str(), "DRIGHT") == 0)
    return KEY_DRIGHT;
  else if (strcasecmp(value.c_str(), "DDOWN") == 0)
    return KEY_DDOWN;
  else if (strcasecmp(value.c_str(), "SL") == 0)
    return KEY_SL;
  else if (strcasecmp(value.c_str(), "SR") == 0)
    return KEY_SR;
  else
    return 0;
}

namespace ini
{
IniData parseIni(const std::string& str)
{
  IniData iniData;

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
}

}