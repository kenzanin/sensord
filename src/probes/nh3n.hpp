#pragma once

#include "probes.hpp"

namespace PROBES {
class Nh3n : _probes {
public:
  Nh3n(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &nh3n_conf);
  void update_value_boqu();
  void set_enable_read(bool en);
  bool get_enable_read();
  float get_value_boqu();
  std::vector<float> get_offset();
  int set_offset(float a, float b);
  void update_value_kacise();
  float get_value_kacise();

protected:
  MODBUS::Modbus &modbus;
  nlohmann::json &conf;
  std::mutex &mutex;

private:
  int get_slope_boqu() { return {}; };
  void impl_set_calib_boqu_first_sequence() {};
  void update_calib_slope_boqu() {};
  void set_calib_boqu(bool en) {};
  bool get_calib_boqu() { return {}; };
};
} // namespace PROBES