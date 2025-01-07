#include "crow/app.h"
#include "server.hpp" // IWYU pragma: keep
#include <string>

namespace SERVER {
using namespace std::literals;

void Server::crow_route_ph() {
  CROW_ROUTE(app, "/ph")
  ([&](const crow::request &req) {
    auto url_param = req.url_params;
    auto read_en = url_param.get("read_en");
    auto read_cmd = url_param.get("read_cmd");
    auto config_en = url_param.get("calib_en");
    auto config_cmd = url_param.get("calib_cmd");
    auto offset_cmd = url_param.get("offset_cmd");
    auto offset_a_url = url_param.get("offset_a");
    auto offset_b_url = url_param.get("offset_b");
    auto slope_cmd = url_param.get("slope_cmd");

    if (read_cmd != nullptr && read_cmd == "set"s) {
      if (read_en == ("1"s)) {
        ph.set_enable_read(true);
      } else {
        ph.set_enable_read(false);
      }
    }

    if (config_cmd != nullptr && config_cmd == "set"s) {
      if (config_en == ("1"s)) {
        ph.set_calib_boqu(true);
      } else {
        ph.set_calib_boqu(false);
      }
    }

    if (offset_cmd != nullptr && offset_cmd == "set"s) {
      auto a = std::stof(offset_a_url);
      auto b = std::stof(offset_b_url);
      ph.set_offset(a, b);
    }

    auto offset = ph.get_offset();
    auto offset_a = offset[0];
    auto offset_b = offset[1];

    auto slope_val = int(0);
    if (slope_cmd != nullptr && slope_cmd == "get"s) {
      slope_val = ph.get_slope_boqu();
    }

    // clang-format off
    auto ctx = crow::mustache::context({
      {"header_text","Halaman PH"},
      {"title","PH"},
      {"read_en",ph.get_enable_read()},
      {"calib_en",ph.get_calib_boqu()},
      {"offset_a",offset_a},
      {"offset_b",offset_b},
      {"slope_val",slope_val}}
      );
    // clang-format on

    auto page = join_html_files("ph.mst");
    return page.render(ctx);
  });
}
} // namespace SERVER