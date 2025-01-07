#pragma once

#include <functional>
#include <vector>

namespace LICENSE {
class license {
public:
  license();
  bool check();
  ~license();

private:
  std::vector<std::function<bool()> *> rcheck{25};
};
} // namespace LICENSE
