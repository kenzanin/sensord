#pragma once

#include "probes.hpp"

namespace PROBES {
class Flow : _probes {
public:
  Flow(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &flow_conf);
  void set_enable_read(bool en);
  bool get_enable_read();
  float get_value_kacise();
  void update_value_kacise();

protected:
  MODBUS::Modbus &modbus;
  nlohmann::json &conf;
  std::mutex &mutex;

private:
  inline void update_value_boqu() {};
  inline float get_value_boqu() { return 0; };
  inline int set_offset(float a, float b) { return {}; };
  inline int get_slope_boqu() { return {}; };
  inline void set_calib_boqu(bool en) {};
  inline bool get_calib_boqu() { return {}; };
  inline std::vector<float> get_offset() { return {}; };
};
} // namespace PROBES