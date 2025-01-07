#include "dbase.hpp"  // IWYU pragma: keep

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iterator>

#include "chrono"
#include "cstdint"
#include "cstdlib"
#include "exception"
#include "fmtlog/fmtlog.h"
#include "mutex"
#include "pqxx/pqxx"

namespace DBASE {

using namespace nlohmann;
using namespace std::literals;

Dbase::Dbase(std::mutex &mutex, json &conf_dbase)
    : conf(conf_dbase), mutex(mutex) {
  // clang-format off
  dbase_url = conf.value("dbase_url",
    "dbname=sparingdb user=postgres "
    "password=123 hostaddr=127.0.0.1 port=5432"s
    );
  // clang-format on
  interval = conf.value("interval", 120);

  try {
    db_connection = new pqxx::connection(dbase_url);

    if (db_connection->is_open()) {
      logi("dbase open success");
      fmtlog::poll();
    }
  } catch (const std::exception &err) {
    loge("open database error: {}", err.what());
    fmtlog::poll();
    exit(-1);
  }
}

void Dbase::run(float ph, float cod, float tss, float nh3n, float flow) {
  auto insert_db = [&]() {
    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
    const auto data_sql = fmt::format(
        "INSERT INTO sparing (time,ph,cod,tss,nh3n,flow) VALUES "
        "({},{},{},{},{},{});",
        now, ph, cod, tss, nh3n, flow);
    logi("{}", data_sql);
    fmtlog::poll();
    try {
      pqxx::work w(*db_connection);
      w.exec(data_sql);
      w.commit();
    } catch (const std::exception &err) {
      logi("error executing sql: {}", err.what());
      fmtlog::poll();
      exit(-1);
    }
  };

  auto get_minutes = []() {
    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
    const uint16_t minute = (now / 60) % 60;
    return minute;
  };

  const auto m = get_minutes();
  const auto it = std::upper_bound(schedule.begin(), schedule.end(), m);
  const auto index = std::distance(schedule.begin(), it);
  const bool index_b = index & 1;
  if (index_b && dones != 1) {
    insert_db();
    dones = 1;
  } else if (!index_b && dones != 2) {
    insert_db();
    dones = 2;
  }
}

int Dbase::get_interval() { return interval; }

Dbase::~Dbase() {
  // if (db_connection->is_open) db_connection->disconnect();
  if (db_connection->is_open()) db_connection->close();
  delete db_connection;
}

}  // namespace DBASE
