#include "crow/app.h"
#include "server.hpp" // IWYU pragma: keep

namespace SERVER {
using namespace std::literals;
void Server::crow_route_flow() {
  CROW_ROUTE(app, "/flow")
  ([&](const crow::request &req) {
    auto probe = &flow;
    const auto header_text = "Halaman FLOW"s;
    const auto page_template = "flow.mst"s;
    const auto title_text = "FLOW"s;

    auto url_param = req.url_params;
    auto read_en = url_param.get("read_en");
    auto read_cmd = url_param.get("read_cmd");

    if (read_cmd != nullptr && read_cmd == "set"s) {
      if (read_en == ("1"s)) {
        probe->set_enable_read(true);
      } else {
        probe->set_enable_read(false);
      }
    }

    // clang-format off
    auto ctx = crow::mustache::context({
      {"header_text",header_text},
      {"title",title_text},
      {"read_en",probe->get_enable_read()}}
      );
    // clang-format on

    auto page = join_html_files(page_template);
    return page.render(ctx);
  });
}
} // namespace SERVER