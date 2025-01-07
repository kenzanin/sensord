#include "fmtlog/fmtlog.h" // IWYU pragma: keep

#include "cod.hpp"
#include "cstdlib"
#include "mutex"
#include "optional"
#include "vector"

namespace PROBES {

using namespace nlohmann;
using namespace std::literals;

Cod::Cod(std::mutex &mutex, MODBUS::Modbus &modbus, json &cod_conf)
    : modbus(modbus), conf(cod_conf), mutex(mutex) {

  name = conf.value("name", "cod");
  addr = conf.value("addr", 1);
  value_reg = conf.value("value_reg", 0);
  loop = conf.value("interval", 5000);
  offset_a = conf.value("offset_a", 1.0f);
  offset_b = conf.value("offset_b", 0.0f);
  value_min = conf.value("value_min", 0.5f);
  value_max = conf.value("value_max", 100.0f);
  random_fact = conf.value("random_fact", 0.5f);
  enable = conf.value("enable",true);

  logi("{} probe, addr: {}", name, addr);

  modbus.set_slave(addr);
  std::srand(std::time(0));
}

void Cod::update_value_kacise() {
  modbus.set_slave(addr);

  auto sleep =
      [](std::chrono::time_point<std::chrono::high_resolution_clock> start,
         int loop) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count();
        auto delay = ((loop - duration) > 0) ? loop - duration : 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
      };

  auto start = std::chrono::high_resolution_clock::now();
  if (!enable) {
    logi("{}, reg: {}, disabled", name, addr);
    fmtlog::poll();
    sleep(start, loop);
    return;
  }
  logi("reading {}, reg: {}", name, addr);
  auto v = modbus.get_value_float_cdab(value_reg);
  if (!v.has_value()) {
    logi("error reading {}, reg: {}", name, addr);
    sleep(start, loop);
    return;
  }
  logi("success reading {}, reg: {}, value: {}", name, addr, v.value());
  {
    const std::lock_guard<std::mutex> lock(mutex);
    value = v.value();
  }
  sleep(start, loop);
}

void Cod::update_value_boqu() {
  modbus.set_slave(addr);

  auto sleep =
      [](std::chrono::time_point<std::chrono::high_resolution_clock> start,
         int loop) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                .count();
        auto delay = ((loop - duration) > 0) ? loop - duration : 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
      };

  auto start = std::chrono::high_resolution_clock::now();
  if (!enable) {
    logi("{}, reg: {}, disabled", name, addr);
    fmtlog::poll();
    sleep(start, loop);
    return;
  }
  logi("reading {}, reg: {}", name, addr);

  auto v = modbus.get_value_int16_ab(value_reg);

  if (!v.has_value()) {
    logi("error reading {}, reg: {}", name, addr);
    fmtlog::poll();
    sleep(start, loop);
    return;
  }

  logi("success reading {}: reg: {}, value: {}", name, addr, v.value());

  {
    const std::lock_guard<std::mutex> lock(mutex);
    value = static_cast<float>(v.value()) * 1.0;
  }

  fmtlog::poll();
  sleep(start, loop);
}

void Cod::set_enable_read(bool en) {
  const std::lock_guard<std::mutex> lock(mutex);
  enable = en;
}

bool Cod::get_enable_read() { return enable; }

float Cod::get_value_kacise() {
  auto v = value;

  auto random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  random *= random_fact;

  if (enable == false) {
    v = value_min + random;
  }

  v = (v + offset_a) * offset_b;

  if (v < value_min) {
    v = value_min + random;
  } else if (v > value_max) {
    v = value_max - random;
  }

  return v;
}

float Cod::get_value_boqu() {
  auto v = std::floor(value) / 100.0;
  auto random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

  if (enable == false) {
    v = value_min + (random * 0.5);
  }

  v = (v + offset_a) * offset_b;

  if (v < value_min) {
    v = value_min + (random * 0.5);
  } else if (v > value_max) {
    v = value_max - (random * 0.5);
  }

  return v;
}

int Cod::set_offset(float a, float b) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    offset_a = a;
    offset_b = b;
    conf.at("offset_a") = offset_a;
    conf.at("offset_b") = offset_b;
  }
  return 0;
}

void Cod::impl_set_calib_boqu_first_sequence() {
  if (calib_first_sequence == true) {
    modbus.set_slave(addr);
    // Standard Solution
    // address 20
    // 0:Standard Solution
    // 4.00,7.00,10.00;
    // 1:Standard Solution
    // 4.00,6.86,9.18
    modbus.set_data_int_ab(20, 1);
    // Instrument Calibration
    // address 13
    modbus.set_data_int_ab(13, 2);
    calib_first_sequence = false;
  }
}

void Cod::update_calib_slope_boqu() {
  auto v = std::optional<int>{0};
  if (calib == true)
    v = modbus.get_value_int16_ab(15);

  if (v.has_value()) {
    const std::lock_guard<std::mutex> lock(mutex);
    slope_boqu = v.value();
  }
}

int Cod::get_slope_boqu() { return slope_boqu; }

void Cod::set_calib_boqu(bool en) {
  const std::lock_guard<std::mutex> lock(mutex);
  if (en) {
    calib_first_sequence = true;
    calib = true;
  } else {
    calib_first_sequence = false;
    calib = false;
  }
}

bool Cod::get_calib_boqu() { return calib; }

std::vector<float> Cod::get_offset() { return {offset_a, offset_b}; }
} // namespace PROBES
