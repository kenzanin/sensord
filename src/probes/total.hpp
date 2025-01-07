#pragma once

#include "probes.hpp"

namespace PROBES {
class Total : _probes {
public:
  Total(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &total_conf);
  void set_enable_read(bool en);
  bool get_enable_read();
  uint32_t get_total_kacise();
  void update_value_kacise();

protected:
  MODBUS::Modbus &modbus;
  nlohmann::json &conf;
  std::mutex &mutex;
  uint32_t total = 0;

private:
  float get_value_kacise() { return {}; };
  void update_value_boqu() {};
  float get_value_boqu() { return 0; };
  int set_offset(float a, float b) { return {}; };
  int get_slope_boqu() { return {}; };
  void set_calib_boqu(bool en) {};
  bool get_calib_boqu() { return {}; };
  std::vector<float> get_offset() { return {}; };
};
} // namespace PROBES