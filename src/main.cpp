#include "fmtlog/fmtlog.h" // IWYU pragma: keep

#include "config/config.hpp"
#include "dbase/dbase.hpp"
#include "lic/license.hpp"
#include "probes/cod.hpp"
#include "probes/flow.hpp"
#include "probes/nh3n.hpp"
#include "probes/ph.hpp"
#include "probes/temp.hpp"
#include "probes/total.hpp"
#include "probes/tss.hpp"
#include "server/server.hpp"

#include "modbus/modbus.hpp" // IWYU pragma: keep

#include "chrono"
#include "csignal"
#include "cstdlib"
#include "functional"
#include "memory"
#include "mutex"
#include "thread"
#include "vector"

int main(int argc, char **argv) {
  auto mtx = std::make_unique<std::mutex>();
  auto conf = std::make_unique<CONFIG::Config>("config.json");

  auto lic = std::make_unique<LICENSE::license>();

  if (!lic->check()) {
    exit(-1);
  }

  auto modbus = std::make_unique<MODBUS::Modbus>(
      *mtx.get(), conf.get()->config->at("modbus"));

  if (!lic->check()) {
    exit(-2);
  }

  auto ph = PROBES::Ph(*mtx.get(), *modbus.get(), conf.get()->config->at("ph"));
  auto temp =
      PROBES::Temp(*mtx.get(), *modbus.get(), conf.get()->config->at("temp"));
  auto cod =
      PROBES::Cod(*mtx.get(), *modbus.get(), conf.get()->config->at("cod"));
  auto tss =
      PROBES::Tss(*mtx.get(), *modbus.get(), conf.get()->config->at("tss"));
  auto nh3n =
      PROBES::Nh3n(*mtx.get(), *modbus.get(), conf.get()->config->at("nh3n"));
  auto flow =
      PROBES::Flow(*mtx.get(), *modbus.get(), conf.get()->config->at("flow"));

  if (!lic->check()) {
    exit(-3);
  }

  auto total =
      PROBES::Total(*mtx.get(), *modbus.get(), conf.get()->config->at("total"));
  auto web = SERVER::Server(*mtx.get(), conf.get()->config->at("server"), ph,
                            temp, cod, tss, nh3n, flow, total);

  if (!lic->check()) {
    exit(-4);
  }

  auto dbase = std::make_unique<DBASE::Dbase>(*mtx.get(),
                                              conf.get()->config->at("dbase"));

  std::vector<std::function<void()>> read_group{
      std::bind(&PROBES::Temp::update_value_kacise, &temp),
      std::bind(&PROBES::Flow::update_value_kacise, &flow),
      std::bind(&PROBES::Cod::update_value_kacise, &cod),
      std::bind(&PROBES::Ph::update_value_kacise, &ph),
      std::bind(&PROBES::Tss::update_value_kacise, &tss),
      std::bind(&PROBES::Nh3n::update_value_kacise, &nh3n),
      std::bind(&PROBES::Total::update_value_kacise, &total)};

  if (!lic->check()) {
    exit(-5);
  }

  // ph.set_enable(false);
  fmtlog::poll();

  std::jthread worker_probe_read([&read_group](std::stop_token token) {
    logi("worker started");
    while (!token.stop_requested()) {
      for (const auto &e : read_group) {
        e();
        if (token.stop_requested())
          break;
        fmtlog::poll();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  });

  if (!lic->check()) {
    exit(-6);
  }

  std::vector<std::function<void()>> write_group{
      std::bind(&PROBES::Ph::update_calib_slope_boqu, &ph)};

  std::jthread worker_probe_write([&write_group](std::stop_token token) {
    while (!token.stop_requested()) {
      for (auto const &e : write_group) {
        e();
        fmtlog::poll();
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
  });

  if (!lic->check()) {
    exit(-7);
  }

  std::jthread worker_db(
      [&dbase, &ph, &cod, &tss, &nh3n, &flow](std::stop_token token) {
        while (!token.stop_requested()) {
          dbase->run(ph.get_value_kacise(), cod.get_value_kacise(),
                     tss.get_value_kacise(), nh3n.get_value_kacise(),
                     flow.get_value_kacise());
          std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
      });

  fmtlog::poll();

  if (!lic->check()) {
    exit(-8);
  }

  web.run();

  worker_db.request_stop();
  worker_probe_read.request_stop();
  worker_probe_write.request_stop();

  worker_db.join();
  worker_probe_read.join();
  worker_probe_write.join();

  fmtlog::poll();
  return 0;
}
