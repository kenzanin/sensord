#include "crow/app.h"
#include "fmt/core.h"
#include "server.hpp" // IWYU pragma: keep

namespace SERVER {
using namespace std::literals;
void Server::crow_route_ph() {
  CROW_ROUTE(app, "/ph")
  ([&](const crow::request &req) {
    auto probe = &ph;
    const auto header_text = "Halaman PH"s;
    const auto page_template = "ph.mst"s;
    const auto title_text = "PH"s;

    auto url_param = req.url_params;
    auto read_en = url_param.get("read_en");
    auto read_cmd = url_param.get("read_cmd");
    auto offset_cmd = url_param.get("offset_cmd");
    auto offset_a_url = url_param.get("offset_a");
    auto offset_b_url = url_param.get("offset_b");

    if (read_cmd != nullptr && read_cmd == "set"s) {
      if (read_en == ("1"s)) {
        probe->set_enable_read(true);
      } else {
        probe->set_enable_read(false);
      }
    }

    if (offset_cmd != nullptr && offset_cmd == "set"s) {
      auto a = std::stof(offset_a_url);
      auto b = std::stof(offset_b_url);
      probe->set_offset(a, b);
    }

    auto offset = probe->get_offset();
    auto offset_a = offset[0];
    auto offset_b = offset[1];

    // clang-format off
    auto ctx = crow::mustache::context({
      {"header_text",header_text},
      {"title",title_text},
      {"read_en",probe->get_enable_read()},
      {"offset_a",offset_a},
      {"offset_b",offset_b}}
    );
    // clang-format on

    auto page = join_html_files(page_template);
    return page.render(ctx);
  });
}
} // namespace SERVER