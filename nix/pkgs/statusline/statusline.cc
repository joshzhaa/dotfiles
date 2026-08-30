// a basic swaybar status line program.

#include <spawn.h>
// <stdio.h> needed to get `stdout` which is a preprocessor macro, shockingly
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
using usize = std::size_t;

/* "BUSINESS-LOGIC" COMPONENTS */

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

  usize capacity;
  i32 n = std::fscanf(file.fp, "%zu", &capacity);
  assert(n == 1);
  auto icon_index = std::clamp(capacity / 20uz, 0uz, ICONS.size());
  return {capacity, ICONS.at(icon_index)};
}

/* I/O */

// Needs to be synchronized with the catppuccin mocha theme
namespace mocha {

  constexpr std::string_view MAUVE = "#cba6f7";

}

/// a "status line" for swaybar is a JSON array of objects.
/// each object is a "block" for swaybar.
void print_status_line() {
  std::println("["); // start status line

  // battery block
  auto [charge_percent, charge_icon] = battery_charge();
  std::println(
    R"({{ "full_text": "{} {}%" }})",
    charge_icon,
    charge_percent //
  );

  std::println("],"); // end status line
  std::fflush(stdout);
}

auto main(int argc, char **argv) -> int {
  using namespace std::chrono_literals;
  auto args = std::span{argv, static_cast<std::size_t>(argc)};

  constexpr std::string_view PROTOCOL_HEADER = R"({ "version": 1 })";
  std::println("{}", PROTOCOL_HEADER);
  std::fflush(stdout);
  std::println("["); // start the infinite JSON list
  std::fflush(stdout);

  while (true) {
    print_status_line();
    std::this_thread::sleep_for(10s);
  }
}
