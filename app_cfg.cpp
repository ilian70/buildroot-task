#include "app.h"
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

bool Application::Config::loadFromFile(const std::string &filename) 
{
  std::ifstream file(filename);
  if (!file.is_open()) {
    return false;
  }

  try {
    nlohmann::json j;
    file >> j;

    if (j.contains("RedisHostIP"))
      RedisHostIP = j["RedisHostIP"].get<std::string>();
    if (j.contains("RedisPort"))
      RedisPort = j["RedisPort"].get<int>();
    if (j.contains("KEY"))
      KEY = j["KEY"].get<std::string>();
    if (j.contains("RefreshTimeGET_sec"))
      RefreshTimeGET_sec = j["RefreshTimeGET_sec"].get<int>();
    if (j.contains("ImageFolder"))
      ImageFolder = j["ImageFolder"].get<std::string>();
    if (j.contains("ImageExtension"))
      ImageExtension = j["ImageExtension"].get<std::string>();
    if (j.contains("ImagePrefix"))
      ImagePrefix = j["ImagePrefix"].get<std::string>();
    if (j.contains("screen_width"))
      screen_width = j["screen_width"].get<int>();
    if (j.contains("screen_height"))
      screen_height = j["screen_height"].get<int>();
    if (j.contains("WindowTitle"))
      WindowTitle = j["WindowTitle"].get<std::string>();

    return true;
  } catch (...) {
    return false;
  }
}
