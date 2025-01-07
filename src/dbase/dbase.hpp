#pragma once

#include "config/config.hpp" // IWYU pragma: keep
#include "pqxx/pqxx"
#include <array>
#include <cstdint>
#include <mutex>

namespace DBASE {
class Dbase {
public:
  Dbase(std::mutex &mutex, nlohmann::json &conf_db);
  void run(float ph, float cod, float tss, float nh3n, float flow);
  int get_interval();
  ~Dbase();

protected:
  // clang-format off
  const std::array<int, 30> schedule{
      0,2,4,6,8,
      10,12,14,16,18,
      20,22,24,26,28,
      30,32,34,36,38,
      40,42,44,46,48,
      50,52,54,56,58};
  // clang-format on

  pqxx::connection *db_connection;
  nlohmann::json &conf;
  std::mutex &mutex;
  std::string dbase_url;
  int interval;
  uint8_t dones = 0;
};

} // namespace DBASE