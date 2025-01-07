#include "server.hpp" // IWYU pragma: keep

namespace SERVER {
using namespace std::literals;
void Server::crow_route_index() {
  CROW_ROUTE(app, "/")
  ([&]() {
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S");

    // clang-format off
    auto ctx = crow::mustache::context({
      {"now", ss.str()},
      {"header_text", "ini header"},
      {"title", "ini title"}
    });
    // clang-format on

    auto page = join_html_files("index.mst");

    return page.render(ctx);
  });
}
} // namespace SERVER
