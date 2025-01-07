#pragma once

#include "probes.hpp"

namespace PROBES {
class Temp : _probes {
public:
  Temp(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &ph_conf);
  void update_value_kacise();
  void update_value_boqu();
  void set_enable_read(bool en);
  bool get_enable_read();
  float get_value_kacise();
  float get_value_boqu();

protected:
  MODBUS::Modbus &modbus;
  nlohmann::json &conf;
  std::mutex &mutex;

private:
  std::vector<float> get_offset() { return {}; };
  int set_offset(float a, float b) { return {}; };
  void set_calib_boqu(bool) {}
  bool get_calib_boqu() { return {}; }
};
} // namespace PROBES
