#include "fmtlog/fmtlog.h" // IWYU pragma: keep

#include "nh3n.hpp"

namespace PROBES {
Nh3n::Nh3n(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &nh3n_conf)
    : modbus(modbus), conf(nh3n_conf), mutex(mutex) {
  name = conf.value("name", "nh3n");
  addr = conf.value("addr", 1);
  value_reg = conf.value("value_addr", 0);
  loop = conf.value("interval", 5000);
  offset_a = conf.value("offset_a", 1.0f);
  offset_b = conf.value("offset_b", 0.0f);
  value_min = conf.value("value_min", 0.015f);
  value_max = conf.value("value_max", 1.0f);
  random_fact = conf.value("random_fact", 0.015f);
  enable = conf.value("enable",true);

  logi("{} probe, addr: {}", name, addr);

  modbus.set_slave(addr);
  std::srand(std::time(0));
}

void Nh3n::update_value_kacise() {
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

float Nh3n::get_value_kacise() {
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

void Nh3n::update_value_boqu() {}
void Nh3n::set_enable_read(bool en) {}
bool Nh3n::get_enable_read() { return {}; }
float Nh3n::get_value_boqu() { return {}; }
std::vector<float> Nh3n::get_offset() { return {}; };
int Nh3n::set_offset(float a, float b) { return {}; };
}; // namespace PROBES