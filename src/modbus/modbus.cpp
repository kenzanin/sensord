#include "modbus/modbus.h"
#include "fmtlog/fmtlog.h" // IWYU pragma: keep

#include "bit"
#include "chrono"
#include "config/config.hpp" // IWYU pragma: keep
#include "cstdint"
#include "cstring"
#include "fmt/core.h"
#include "fmt/ranges.h" // IWYU pragma: keep
#include "modbus.hpp"
#include "mutex"
#include "optional"
#include "string"
#include "thread"
#include "vector"

namespace MODBUS {

using namespace nlohmann;
using namespace std::literals;

Modbus::Modbus(std::mutex &mutex, std::string const device, uint32_t baud,
               char parity, uint8_t data_bit, uint8_t stop_bit)
    : mutex(mutex) {
  ctx = modbus_new_rtu(device.c_str(), baud, parity, data_bit, stop_bit);
  modbus_set_debug(ctx, 10);
  modbus_set_response_timeout(ctx, 0, 1000 * 1000);
  modbus_set_byte_timeout(ctx, 0, 500 * 1000);

  auto recover = static_cast<modbus_error_recovery_mode>(
      MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL);
  modbus_set_error_recovery(ctx, recover);
  logi("modbus trying to connect");

  while (1) {
    auto err = modbus_connect(ctx);
    if (err != -1) {
      break;
    }
    logi("cant connect to modbus: {}", modbus_strerror(errno));
    fmtlog::poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
  fmtlog::poll();
}

Modbus::Modbus(std::mutex &mutex, json &modbus_conf)
    : mutex(mutex), conf(&modbus_conf) {
  device = conf->value("device", "/dev/tnt1"s);
  baud = conf->value("baud", 9600);
  parity = conf->value("parity", "N")[0];
  data_bit = conf->value("data_bit", 8);
  stop_bit = conf->value("stop_bit", 1);
  debug = conf->value("debug", 0);
  retry_delay = conf->value("retry_delay", 1000);

  ctx = modbus_new_rtu(device.c_str(), baud, parity, data_bit, stop_bit);

  if (debug)
    modbus_set_debug(ctx, 1);
  modbus_set_response_timeout(ctx, 0, 1000 * 1000);
  modbus_set_byte_timeout(ctx, 0, 500 * 1000);

  while (1) {
    auto err = modbus_connect(ctx);
    if (err != -1) {
      break;
    }
    logi("cant connect to modbus: {}", modbus_strerror(errno));
    fmtlog::poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay));
  }
}

void Modbus::set_slave(uint8_t slave) {
  if (modbus_set_slave(ctx, slave) == -1) {
    loge("error set slave addr to {}", slave);
  };
}

std::vector<uint16_t> Modbus::get_data(int addr, int n) {
  auto dest = std::vector<uint16_t>(n, 0);
  auto retry = 0;
  {
    for (; retry < 10; retry++) {
      int err = 0;
      { // lock guard
        const std::lock_guard<std::mutex> lock(mutex);
        err = modbus_read_registers(ctx, addr, dest.size(), dest.data());
      }
      if (err != -1) {
        break;
      }
      logi("modbus id: {}, read register: {} error: {}", modbus_get_slave(ctx),
           addr, modbus_strerror(errno));
      fmtlog::poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay));
    }
  }

  if (retry >= 10) {
    logi("modbus reading error");
    return std::vector<uint16_t>{};
  }

  return dest;
}

int Modbus::set_data(int addr, std::vector<uint16_t> &data) {
  int retry = 0;
  int err = 0;

  for (; retry < 10; retry++) {
    { // lock guard
      const std::lock_guard<std::mutex> lock(mutex);
      err = modbus_write_registers(ctx, addr, data.size(), data.data());
    }
    if (err != -1) {
      break;
    }
    logi("modbus id: {}, write error: ", modbus_get_slave(ctx),
         modbus_strerror(errno));
    fmtlog::poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay));
  }

  if (retry >= 10) {
    return -1;
  }

  return 0;
}

std::optional<float> Modbus::get_value_float_abcd(int addr) {
  auto src = get_data(addr, 2);
  if (src.size() == 0) {
    return std::nullopt;
  }

  float res = modbus_get_float_abcd(src.data());
  return res;
}

std::optional<float> Modbus::get_value_float_badc(int addr) {
  auto src = get_data(addr, 2);
  if (src.size() == 0) {
    return std::nullopt;
  }
  float res = modbus_get_float_badc(src.data());
  return res;
}

std::optional<float> Modbus::get_value_float_cdab(int addr) {
  auto src = get_data(addr, 2);
  if (src.size() == 0) {
    return std::nullopt;
  }
  float res = modbus_get_float_cdab(src.data());
  return res;
}

std::optional<float> Modbus::get_value_float_dcba(int addr) {
  auto src = get_data(addr, 2);
  if (src.size() == 0) {
    return std::nullopt;
  }

  float res = modbus_get_float_dcba(src.data());
  return res;
}

std::optional<int> Modbus::get_value_int16_ab(int addr) {
  auto src = get_data(addr, 1);
  if (src.size() == 0) {
    return std::nullopt;
  }
  return src[0];
}

std::optional<int> Modbus::get_value_int16_ba(int addr) {
  auto src = get_data(addr, 1);
  if (src.size() == 0) {
    return std::nullopt;
  }

  int res = std::byteswap(src[0]);
  return res;
}

int Modbus::set_data_int_ab(int addr, uint16_t data) {
  auto d = std::vector<uint16_t>{data};
  return set_data(addr, d);
}

int Modbus::set_data_int_ba(int addr, uint16_t data) {
  auto d = std::vector<uint16_t>{std::byteswap(data)};
  return set_data(addr, d);
}

int Modbus::set_data_float_abcd(int addr, float data) {
  auto buf = std::vector<uint16_t>(2, 0);
  modbus_set_float_abcd(data, buf.data());
  return set_data(addr, buf);
}

int Modbus::set_data_float_badc(int addr, float data) {
  auto buf = std::vector<uint16_t>(2, 0);
  modbus_set_float_badc(data, buf.data());
  return set_data(addr, buf);
}

int Modbus::set_data_float_cdab(int addr, float data) {
  auto buf = std::vector<uint16_t>(2, 0);
  modbus_set_float_cdab(data, buf.data());
  return set_data(addr, buf);
}

int Modbus::set_data_float_dcba(int addr, float data) {
  auto buf = std::vector<uint16_t>(2, 0);
  modbus_set_float_dcba(data, buf.data());
  return set_data(addr, buf);
}

float Modbus::abcd_to_float(const std::vector<uint16_t> &in, int index = 0) {
  return modbus_get_float_abcd(in.data() + index);
}

float Modbus::badc_to_float(const std::vector<uint16_t> &in, int index = 0) {
  return modbus_get_float_badc(in.data() + index);
}

float Modbus::cdba_to_float(const std::vector<uint16_t> &in, int index = 0) {
  return modbus_get_float_cdab(in.data() + index);
}

float Modbus::dcba_to_float(const std::vector<uint16_t> &in, int index = 0) {
  return modbus_get_float_dcba(in.data() + index);
}

Modbus::~Modbus() {
  modbus_close(ctx);
  modbus_free(ctx);
}

} // namespace MODBUS
