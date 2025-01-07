#pragma once

#include "config/config.hpp" // IWYU pragma: keep
#include "modbus/modbus.hpp" // IWYU pragma: keep
#include "mutex"             // IWYU pragma: keep
#include "vector"

namespace PROBES {
class _probes {
public:
  virtual void update_value_kacise() = 0;
  virtual float get_value_kacise() = 0;

  virtual void update_value_boqu() = 0;
  virtual float get_value_boqu() = 0;
  virtual void set_calib_boqu(bool en) = 0;
  virtual bool get_calib_boqu() = 0;

  virtual void set_enable_read(bool en) = 0;
  virtual bool get_enable_read() = 0;
  virtual int set_offset(float a, float b) = 0;
  virtual std::vector<float> get_offset() = 0;

protected:
  bool enable = true;
  bool calib = false;
  bool calib_first_sequence = false;
  int loop = 5000;
  int addr;
  int value_reg;
  int slope_boqu;
  float value = 0.0f;
  float offset_a = 1.0f;
  float offset_b = 0.0f;
  float value_min;
  float value_max;
  float random_fact;
  std::string name;
};

} // namespace PROBES
