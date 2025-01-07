#include "fmtlog/fmtlog.h" // IWYU pragma: keep
#include "server.hpp"      // IWYU pragma: keep

#include "crow/app.h"
#include "crow/http_response.h"

namespace SERVER {
using namespace nlohmann;
void Server::crow_route_read() {
  CROW_ROUTE(app, "/read/json")
  ([&](const crow::request &req, crow::response &res) {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");

    // clang-format off
    const json data{
      {"ph", ph.get_value_kacise()},
      {"cod", cod.get_value_kacise()},
      {"tss", tss.get_value_kacise()},
      {"nh3n", nh3n.get_value_kacise()},
      {"flow", flow.get_value_kacise()},
      {"temp", temp.get_value_kacise()},
      {"total",total.get_total_kacise()},
      {"time",ss.str()},
      {"unix",static_cast<uint64_t>(now_time)},
    };
    // clang-format on

    res.add_header("Content-Type", "application/json");
    res.write(data.dump(2));
    res.end();
  });
}
} // namespace SERVER
