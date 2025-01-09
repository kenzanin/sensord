#include "fmtlog/fmtlog.h"

#include "total.hpp"

namespace PROBES {

using namespace nlohmann;

Total::Total(std::mutex &mutex, MODBUS::Modbus &modbus, json &ph_conf)
    : modbus(modbus), conf(ph_conf), mutex(mutex) {

  name = conf.value("name", "total");
  addr = conf.value("addr", 1);
  value_reg = conf.value("value_reg", 0);
  loop = conf.value("interval", 5000);
  enable = conf.value("enable", true);

  logi("{} probe, addr: {}", name, addr);
  fmtlog::poll();
  modbus.set_slave(addr);
  std::srand(std::time(0));
}

void Total::set_enable_read(bool en) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    enable = en;
  }
}

bool Total::get_enable_read() { return enable; }

uint32_t Total::get_total_kacise() { return total; };

void Total::update_value_kacise() {
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
    logi("{}, addr: {}, reg: {}, disabled", name, addr, value_reg);
    fmtlog::poll();
    return;
  }
  logi("reading {}, addr: {}, reg: {}", name, addr, value_reg);

  auto v = modbus.get_data(addr, 2);
  if (!v.size()) {
    logi("error reading {}, addr: {}, reg: {}", name, addr, value_reg);
    return;
  }

  uint32_t v_value = v[1] | (v[0] << 16);

  logi("success reading {}, addr: {}, reg: {}, value: {}", name, addr,
       value_reg, v_value);
  {
    const std::lock_guard<std::mutex> lock(mutex);
    total = v_value;
  }
  sleep(start, loop);
}

} // namespace PROBES