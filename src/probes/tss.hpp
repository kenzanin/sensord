#pragma once

#include "probes.hpp"

namespace PROBES {
class Tss : _probes {
public:
  Tss(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &tss_conf);
  bool get_enable_read();
  void set_calib_boqu(bool en);
  int set_offset(float a, float b);
  void set_enable_read(bool en);
  bool get_calib_boqu();
  std::vector<float> get_offset();
  void update_value_boqu();
  float get_value_boqu();
  void update_value_kacise();
  float get_value_kacise();

protected:
  MODBUS::Modbus &modbus;
  nlohmann::json &conf;
  std::mutex &mutex;

private:
};

}; // namespace PROBES