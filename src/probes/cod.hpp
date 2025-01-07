#pragma once

#include "probes.hpp" // IWYU pragma: keep

namespace PROBES {
class Cod : _probes {
public:
  Cod(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &cod_conf);
  void update_value_kacise();
  void update_value_boqu();
  void set_enable_read(bool en);
  bool get_enable_read();
  float get_value_kacise();
  float get_value_boqu();
  std::vector<float> get_offset();
  int set_offset(float a, float b);

  int get_slope_boqu();
  void impl_set_calib_boqu_first_sequence();
  void update_calib_slope_boqu();
  void set_calib_boqu(bool en);
  bool get_calib_boqu();

protected:
  MODBUS::Modbus &modbus;
  nlohmann::json &conf;
  std::mutex &mutex;
};

} // namespace PROBES
