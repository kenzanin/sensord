#include "server.hpp"
#include "crow.h" // IWYU pragma: keep
#include "crow/app.h"
#include "crow/mustache.h"
#include "cstring"
#include "ctime"
#include "fstream"
#include "iterator"
#include "probes/nh3n.hpp"
#include "probes/ph.hpp"
#include "probes/temp.hpp"
#include "probes/total.hpp"

namespace SERVER {

using namespace std::string_literals;
using namespace nlohmann;

Server::Server(std::mutex &mutex, nlohmann::json &server_conf, PROBES::Ph &ph,
               PROBES::Temp &temp, PROBES::Cod &cod, PROBES::Tss &tss,
               PROBES::Nh3n &nh3n, PROBES::Flow &flow, PROBES::Total &total)
    : mutex(mutex), conf(server_conf), ph(ph), temp(temp), cod(cod), tss(tss),
      nh3n(nh3n), flow(flow), total(total) {

  path = conf.value("path", "templates/"s);
  port = conf.value("port", 8080);

  crow_route_menu();
  crow_route_ph();
  crow_route_index();
  crow_route_read();
  crow_route_temp();
  crow_route_cod();
}

void Server::run() { app.port(port).run(); }

void Server::stop() { app.stop(); }

crow::mustache::template_t Server::join_html_files(std::string const &p) {
  auto header = std::ifstream(path + "header.mst");
  auto body = std::ifstream(path + p);
  auto footer = std::ifstream(path + "footer.mst");
  auto page_str = std::string(std::istreambuf_iterator<char>{header}, {});
  page_str += std::string(std::istreambuf_iterator<char>{body}, {});
  page_str += std::string(std::istreambuf_iterator<char>{footer}, {});

  return crow::mustache::compile(page_str);
}

} // namespace SERVER