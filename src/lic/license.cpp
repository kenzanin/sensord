#include "license.hpp"

#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <fstream> // IWYU pragma: keep
#include <functional>
#include <string>        // IWYU pragma: keep
#include <unordered_set> // IWYU pragma: keep

#include "fmt/core.h" // IWYU pragma: keep

namespace LICENSE {
bool check_license() {
#if defined(__aarch64__) || defined(__arm__)
  std::ifstream cpuinfo("/proc/cpuinfo");
  std::string line;
  std::string serial;

  if (cpuinfo.is_open()) {
    while (std::getline(cpuinfo, line)) {
      if (line.find("Serial") != std::string::npos) {
        // Split the line to get the serial number
        serial = line.substr(line.find(":") + 2); // +2 to skip ": "
        break;
      }
    }
    cpuinfo.close();
  } else {
    fmt::print("Unable find serial");
  }
  // 100000006d2e852d wtp4-kecil
  std::unordered_set<std::string> serial_set{"100000006d2e852d"};

  return serial_set.contains(serial);
#elif defined(__i386__) || defined(__x86_64__)
  return true;
#else
  return false;
#endif
}

license::license() {
  std::srand(std::time(0));
  rcheck[0] = new std::function<bool()>(check_license);
  for (size_t i = 1; i < rcheck.size(); i++) {
    rcheck[i] = new std::function<bool()>(*rcheck[i - 1]);
  }
}

bool license::check() {
  const int random = std::rand() % (rcheck.size() - 1);
  return (*rcheck[random])();
}

license::~license() {
  for (size_t i = 0; i < rcheck.size(); i++) {
    delete rcheck[i];
  }
}

} // namespace LICENSE
