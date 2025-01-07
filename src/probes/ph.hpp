#pragma once

#include "probes.hpp"

namespace PROBES {

class Ph : _probes {
public:
  Ph(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &ph_conf);
  void update_value_kacise();
  void update_value_boqu();
  void set_enable_read(bool en);
  bool get_enable_read();
  float get_value_kacise();
  float get_value_boqu();
  void update_calib_slope_boqu();
  int set_offset(float a, float b);
  int get_slope_boqu();
  void set_calib_boqu(bool en);
  bool get_calib_boqu();
  std::vector<float> get_offset();

protected:
  void impl_set_calib_boqu_first_sequence();
  MODBUS::Modbus &modbus;
  nlohmann::json &conf;
  std::mutex &mutex;
};

} // namespace PROBES
