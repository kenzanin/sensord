#include "fmtlog/fmtlog.h"

#include "tss.hpp"

namespace PROBES {

Tss::Tss(std::mutex &mutex, MODBUS::Modbus &modbus, nlohmann::json &tss_conf)
    : modbus(modbus), conf(tss_conf), mutex(mutex) {
  name = conf.value("name", "tss");
  addr = conf.value("addr", 1);
  value_reg = conf.value("value_reg", 0);
  loop = conf.value("interval", 5000);
  offset_a = conf.value("offset_a", 1.0f);
  offset_b = conf.value("offset_b", 0.0f);
  value_min = conf.value("value_min", 0.5f);
  value_max = conf.value("value_max", 100.0f);
  random_fact = conf.value("random_fact", 0.5f);
  enable = conf.value("enable", true);

  logi("{} probe, addr: {}", name, addr);

  modbus.set_slave(addr);
  std::srand(std::time(0));
  fmtlog::poll();
}

void Tss::set_enable_read(bool en) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    enable = en;
  }
}

bool Tss::get_enable_read() { return enable; }

int Tss::set_offset(float a, float b) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    offset_a = a;
    offset_b = b;
    conf.at("offset_a") = offset_a;
    conf.at("offset_b") = offset_b;
  }
  return 0;
}

void Tss::set_calib_boqu(bool en) {
  {
    const std::lock_guard<std::mutex> lock(mutex);
    calib = en;
  }
}

bool Tss::get_calib_boqu() { return calib; }

std::vector<float> Tss::get_offset() { return {offset_a, offset_b}; }

void Tss::update_value_boqu() {
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

float Tss::get_value_boqu() {
  auto v = std::floor(value) / 100.0;
  auto random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

  if (enable == false) {
    v = value_min + (random * random_fact);
  }

  v = (v + offset_a) * offset_b;

  if (v < value_min) {
    v = value_min + (random * random_fact);
  } else if (v > value_max) {
    v = value_max - (random * random_fact);
  }

  return v;
}

void Tss::update_value_kacise() {
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
    sleep(start, loop);
    return;
  }
  logi("reading {}, addr: {}, reg: {}", name, addr, value_reg);
  auto v = modbus.get_data(value_reg, 6 * 2);

  if (!v.size()) {
    logi("error reading {}, addr: {}, reg: {}", name, addr, value_reg);
    sleep(start, loop);
    return;
  }

  auto val = modbus.dcba_to_float(v, 0);
  logi("success reading {}, addr: {}, reg: {}, value: {}", name, addr,
       value_reg, val);
  {
    const std::lock_guard<std::mutex> lock(mutex);
    value = val;
  }
  sleep(start, loop);
}

float Tss::get_value_kacise() {
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
};

} // namespace PROBES
