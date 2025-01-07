#include "fmtlog/fmtlog.h" // IWYU pragma: keep

#include "config.hpp"
#include "filesystem"
#include "string"
#include <fstream>

namespace CONFIG {

using namespace nlohmann;

Config::Config(std::string file) : path(file) {
  if (!std::filesystem::is_regular_file(path)) {
    loge("{} is not valid file", path);
    fmtlog::poll();
    exit(-1);
  };
  auto s_file = std::ifstream(path);
  config = new json;
  s_file >> *config;
  s_file.close();
  logi("json config:\n{}", config->dump(2));
  fmtlog::poll();
}

void Config::save() {
  logi("json config:\n{}", config->dump(2));
  fmtlog::poll();
  auto file = std::ofstream(path);
  if (!file.is_open()) {
    logi("error write config");
    fmtlog::poll();
  }
  file << config->dump(2);
  file.close();
}

Config::~Config() { delete config; }
} // namespace CONFIG