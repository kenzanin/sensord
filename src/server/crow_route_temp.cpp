#include "crow/app.h"
#include "server.hpp" // IWYU pragma: keep
#include <string>

namespace SERVER {
using namespace std::literals;

void Server::crow_route_temp() {
  CROW_ROUTE(app, "/temp")
  ([&](const crow::request &req) {
    auto url_param = req.url_params;
    auto read_en = url_param.get("read_en");
    auto read_cmd = url_param.get("read_cmd");

    if (read_cmd != nullptr && read_cmd == "set"s) {
      if (read_en == ("1"s)) {
        temp.set_enable_read(true);
      } else {
        temp.set_enable_read(false);
      }
    }

    // clang-format off
    auto ctx = crow::mustache::context({
      {"header_text","Halaman Temperature"},
      {"title","Temperature"},
      {"read_en",temp.get_enable_read()}}
      );
    // clang-format on

    auto page = join_html_files("temp.mst");
    return page.render(ctx);
  });
}

} // namespace SERVER
