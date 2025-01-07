#include "server.hpp" // IWYU pragma: keep

namespace SERVER {
void Server::crow_route_menu() {
  CROW_ROUTE(app, "/menu")
  ([&]() {
    // clang-format off
    auto ctx = crow::mustache::context({
      {"header_text","Warung Sparing"},
      {"title","Menu"}}
      );
    // clang-format on
    auto page = join_html_files("menu.mst");
    return page.render(ctx);
  });
}
} // namespace SERVER