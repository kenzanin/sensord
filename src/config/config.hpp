#pragma once

#include "nlohmann/json.hpp" // IWYU pragma: keep
#include "string"

namespace CONFIG {

class Config {
public:
  Config(std::string file);
  ~Config();
  void save();

  nlohmann::json *config;

protected:
  std::string path;
};

} // namespace CONFIG