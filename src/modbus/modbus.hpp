#pragma once

#include <cstdint>

#include "config/config.hpp" // IWYU pragma: keep
#include "cstdint"
#include "modbus/modbus.h"
#include "mutex"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include "optional"
#include "string"

namespace MODBUS {

class Modbus {
public:
  Modbus(std::mutex &mutex, std::string const device = "/dev/tnt1",
         uint32_t baud = 9600, char parity = 'N', uint8_t data_bit = 8,
         uint8_t stop_bit = 1);
  Modbus(std::mutex &mutex, nlohmann::json &modbus_conf);
  ~Modbus();

  void set_slave(uint8_t slave);
  std::vector<uint16_t> get_data(int addr, int n);
  int set_data(int addr, std::vector<uint16_t> &data);
  int set_data_float_abcd(int addr, float data);
  int set_data_float_badc(int addr, float data);
  int set_data_float_cdab(int addr, float data);
  int set_data_float_dcba(int addr, float data);
  int set_data_int_ab(int addr, uint16_t data);
  int set_data_int_ba(int addr, uint16_t data);
  std::optional<float> get_value_float_abcd(int addr);
  std::optional<float> get_value_float_badc(int addr);
  std::optional<float> get_value_float_cdab(int addr);
  std::optional<float> get_value_float_dcba(int addr);
  float abcd_to_float(const std::vector<uint16_t> &in, int index);
  float badc_to_float(const std::vector<uint16_t> &in, int index);
  float cdba_to_float(const std::vector<uint16_t> &in, int index);
  float dcba_to_float(const std::vector<uint16_t> &in, int index);
  std::optional<int> get_value_int16_ab(int addr);
  std::optional<int> get_value_int16_ba(int addr);

protected:
  modbus_t *ctx;
  std::mutex &mutex;
  nlohmann::json *conf;
  std::string device{};
  int baud;
  char parity;
  int data_bit;
  int stop_bit;
  int debug;
  int retry_delay;
  int response_timeout;
  int byte_timeout;
};

} // namespace MODBUS