#include <sys/inotify.h>

import std;

constexpr std::string_view BATTERY = "/sys/class/power_supply/BAT1";

auto main(int argc, char **argv) -> int {
  auto args = std::span{argv, static_cast<std::size_t>(argc)};

  namespace fs = std::filesystem;

  auto file = std::ifstream{BATTERY / fs::path("capacity")};
  std::string content;
  file >> content;

  std::println("hello world {}", content);

  return 0;
}
