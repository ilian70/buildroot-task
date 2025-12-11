#include "app.h"
#include <fstream>
#include <string>

#include "json11.hpp"

bool Application::Config::loadFromFile(const std::string &filename) 
{
  std::ifstream file(filename);
  if (!file.is_open()) {
    return false;
  }

  std::string json_str((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
  
  std::string err;
  json11::Json j = json11::Json::parse(json_str, err);
  
  if (!err.empty()) {
    return false;
  }

  if (j["RedisHostIP"].is_string())
    RedisHostIP = j["RedisHostIP"].string_value();
  if (j["RedisPort"].is_number())
    RedisPort = j["RedisPort"].int_value();
  if (j["KEY"].is_string())
    KEY = j["KEY"].string_value();
  if (j["RefreshTimeGET_sec"].is_number())
    RefreshTimeGET_sec = j["RefreshTimeGET_sec"].int_value();
  if (j["ImageFolder"].is_string())
    ImageFolder = j["ImageFolder"].string_value();
  if (j["ImageExtension"].is_string())
    ImageExtension = j["ImageExtension"].string_value();
  if (j["ImagePrefix"].is_string())
    ImagePrefix = j["ImagePrefix"].string_value();
  if (j["screen_width"].is_number())
    screen_width = j["screen_width"].int_value();
  if (j["screen_height"].is_number())
    screen_height = j["screen_height"].int_value();
  if (j["WindowTitle"].is_string())
    WindowTitle = j["WindowTitle"].string_value();

  return true;
}
