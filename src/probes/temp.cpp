#include "fmtlog/fmtlog.h" // IWYU pragma: keep

#include "cstdlib"
#include "mutex"
#include "optional"
#include "temp.hpp"

namespace PROBES {

using namespace nlohmann;
using namespace std::literals;

Temp::Temp(std::mutex &mutex, MODBUS::Modbus &modbus, json &temp_conf)
    : modbus(modbus), conf(temp_conf), mutex(mutex) {

  name = conf.value("name", "temp");
  addr = conf.value("addr", 1);
  value_reg = conf.value("value_addr", 0);
  loop = conf.value("interval", 5000);
  offset_a = conf.value("offset_a", 1.0f);
  offset_b = conf.value("offset_b", 0.0f);
  value_min = conf.value("value_min", 20.0f);
  value_max = conf.value("value_max", 50.0f);
  enable = conf.value("enable",true);

  logi("{} probe, addr: {}", name, addr);

  modbus.set_slave(addr);
  std::srand(std::time(0));
}

void Temp::update_value_kacise() {
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
  logi("success reading {}: reg: {}, value: {}", name, addr, v.value());
  {
    const std::lock_guard<std::mutex> lock(mutex);
    value = v.value();
  }
  sleep(start, loop);
}

void Temp::update_value_boqu() {
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

  logi("success reading {}, reg: {}, value: {}", name, addr, v.value());

  {
    const std::lock_guard<std::mutex> lock(mutex);
    value = static_cast<float>(v.value()) * 1.0;
  }

  fmtlog::poll();
  sleep(start, loop);
}

void Temp::set_enable_read(bool en) {
  const std::lock_guard<std::mutex> lock(mutex);
  enable = en;
}

bool Temp::get_enable_read() { return enable; }

float Temp::get_value_kacise() { return value; }

float Temp::get_value_boqu() {
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

} // namespace PROBES
