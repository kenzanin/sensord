#pragma once

#include "config/config.hpp" // IWYU pragma: keep
#include "crow/app.h"
#include "crow/mustache.h"
#include "nlohmann/json.hpp" // IWYU pragma: keep

#include "probes/cod.hpp"
#include "probes/flow.hpp"
#include "probes/nh3n.hpp"
#include "probes/ph.hpp"
#include "probes/temp.hpp"
#include "probes/total.hpp"
#include "probes/tss.hpp"

#include <mutex>

namespace SERVER {
class Server {
public:
  Server(std::mutex &mutex, nlohmann::json &server_conf, PROBES::Ph &ph,
         PROBES::Temp &temp, PROBES::Cod &cod, PROBES::Tss &tss,
         PROBES::Nh3n &nh3n, PROBES::Flow &flow, PROBES::Total &total);
  void run();
  void stop();

protected:
  crow::mustache::template_t join_html_files(std::string const &p);

  int port;
  std::mutex &mutex;
  nlohmann::json &conf;
  crow::SimpleApp app;
  std::string path;
  PROBES::Ph &ph;
  PROBES::Temp &temp;
  PROBES::Cod &cod;
  PROBES::Tss &tss;
  PROBES::Nh3n &nh3n;
  PROBES::Flow &flow;
  PROBES::Total &total;
  void crow_route_ph();
  void crow_route_tss();
  void crow_route_cod();
  void crow_route_nh3n();
  void crow_route_flow();
  void crow_route_total();
  void crow_route_read();
  void crow_route_index();
  void crow_route_menu();
  void crow_route_temp();
};
} // namespace SERVER