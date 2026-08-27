#include <spawn.h>
#include <stdio.h>

import std;

/* CONFIGURATION */
constexpr std::string_view BATTERY_CAPACITY = "/sys/class/power_supply/BAT1/capacity";
constexpr std::string_view BATTERY_STATUS = "/sys/class/power_supply/BAT1/status";
constexpr std::string_view VOLUME_COMMAND = "wpctl get-volume @DEFAULT_AUDIO_SINK@";

/* HELPERS */
void assert(bool condition) {
  if (!condition) {
    std::println("ERROR: something went wrong");
    std::exit(1);
  }
}

// RAII wrapper over std::fopen
struct File {
  std::FILE *fp;

  // this is technically not safe, but I hate fstream and c strings
  File(std::string_view filename) : fp{std::fopen(filename.data(), "r")} { assert(fp); }

  ~File() { std::fclose(fp); }
};

using i32 = std::int32_t;

/* MAIN LOGIC */

[[nodiscard]]
auto battery_charge() -> std::tuple<i32, std::string_view> {
  constexpr static std::array<char const *, 5> ICONS = std::array{
    "", // 20%
    "", // 40%
    "", // 60%
    "", // 80%
    "", // 100%
  };

  auto file = File{BATTERY_CAPACITY};

  i32 capacity;
  i32 n = std::fscanf(file.fp, "%d", &capacity);
  assert(n == 1);
  auto icon_index = static_cast<std::size_t>(capacity / 20);
  return {capacity, icon_index >= ICONS.size() ? ICONS.back() : ICONS.at(icon_index)};
}

void print_status() {
  auto [charge, charge_icon] = battery_charge();
  std::println("{} {}%", charge_icon, charge);
  std::fflush(stdout);
}

auto main(int argc, char **argv) -> int {
  using namespace std::chrono_literals;
  auto args = std::span{argv, static_cast<std::size_t>(argc)};

  while (true) {
    print_status();
    std::this_thread::sleep_for(30s);
  }
}
