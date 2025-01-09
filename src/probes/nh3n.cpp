#include "fmtlog/fmtlog.h" // IWYU pragma: keep

#include "nh3n.hpp"

namespace PROBES {
Nh3n::Nh3n(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &nh3n_conf)
    : modbus(modbus), conf(nh3n_conf), mutex(mutex) {
  name = conf.value("name", "nh3n");
  addr = conf.value("addr", 1);
  value_reg = conf.value("value_reg", 0);
  loop = conf.value("interval", 5000);
  offset_a = conf.value("offset_a", 1.0f);
  offset_b = conf.value("offset_b", 0.0f);
  value_min = conf.value("value_min", 0.015f);
  value_max = conf.value("value_max", 1.0f);
  random_fact = conf.value("random_fact", 0.015f);
  enable = conf.value("enable", true);

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
    logi("{}, addr:{}, reg: {}, disabled", name, addr, value_reg);
    fmtlog::poll();
    sleep(start, loop);
    return;
  }
  logi("reading {}, addr:{}, reg: {}", name, addr, value_reg);
  // auto v = modbus.get_value_float_cdab(value_reg);
  auto v = modbus.get_data(value_reg, 6 * 2);
  if (!v.size()) {
    logi("error reading {}, addr: {}, reg: {}", name, addr, value_reg);
    sleep(start, loop);
    return;
  }

  auto val = modbus.dcba_to_float(v, 2 * 2);
  logi("success reading {}, addr:{}, reg: {}, value: {}", name, addr, value_reg,
       val);
  {
    const std::lock_guard<std::mutex> lock(mutex);
    value = val;
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

  v = (v + offset_b) * offset_a;

  if (v < value_min) {
    v = value_min + random;
  } else if (v > value_max) {
    v = value_max - random;
  }

  return v;
}

void Nh3n::set_enable_read(bool en) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    enable = en;
  }
}

bool Nh3n::get_enable_read() { return enable; }

std::vector<float> Nh3n::get_offset() { return {offset_a, offset_b}; };

int Nh3n::set_offset(float a, float b) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    offset_a = a;
    offset_b = b;
    conf.at("offset_a") = offset_a;
    conf.at("offset_b") = offset_b;
  }
  return 0;
};

void Nh3n::update_value_boqu() {}
float Nh3n::get_value_boqu() { return {}; }
}; // namespace PROBES