#include "fmtlog/fmtlog.h"

#include "flow.hpp"

namespace PROBES {
using namespace nlohmann;

Flow::Flow(std::mutex &mutex, MODBUS::Modbus &modbus, json &ph_conf)
    : modbus(modbus), conf(ph_conf), mutex(mutex) {

  name = conf.value("name", "flow");
  addr = conf.value("addr", 1);
  value_reg = conf.value("value_reg", 0);
  loop = conf.value("interval", 5000);
  offset_a = conf.value("offset_a", 1.0f);
  offset_b = conf.value("offset_b", 0.0f);
  value_min = conf.value("value_min", 4.0f);
  value_max = conf.value("value_max", 9.0f);

  logi("{} probe, addr: {}", name, addr);
  fmtlog::poll();
  modbus.set_slave(addr);
  std::srand(std::time(0));
}

void Flow::set_enable_read(bool en) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    enable = en;
  }
}

bool Flow::get_enable_read() { return enable; }

float Flow::get_value_kacise() { return value; }

void Flow::update_value_kacise() {
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

}; // namespace PROBES