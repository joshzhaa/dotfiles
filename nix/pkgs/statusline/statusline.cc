// a basic swaybar status line program.

#include <spawn.h>
// <stdio.h> needed to get `stdout` which standardized as a preprocessor macro, shockingly
#include <stdio.h>

import std;

/* CONFIGURATION */
constexpr std::string_view BATTERY_CAPACITY = "/sys/class/power_supply/BAT1/capacity";
constexpr std::string_view BATTERY_STATUS = "/sys/class/power_supply/BAT1/status";
constexpr std::string_view VOLUME_COMMAND = "wpctl get-volume @DEFAULT_AUDIO_SINK@";
constexpr auto TZ_OFFSET = std::chrono::hours{4}; // from system_clock::now()

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
auto battery_charge() -> std::string {
  constexpr static auto ICONS = std::array{"", "", "", "", ""};
  constexpr static auto CHARGING_ICON = " 󱐋 ";

  auto read_capacity_file = []() {
    auto file = File{BATTERY_CAPACITY};
    usize capacity;
    i32 n = std::fscanf(file.fp, "%zu", &capacity);
    assert(n == 1);
    return capacity;
  };

  auto read_status_file = []() {
    auto file = File{BATTERY_STATUS};
    constexpr usize BUFSIZE = 32;
    std::array<char, BUFSIZE> buffer;
    i32 n = std::fscanf(file.fp, "%31s", buffer.data());
    assert(n == 1);
    return std::string{buffer.data()};
  };

  usize capacity = read_capacity_file();
  auto status = read_status_file();

  auto icon_index = std::clamp(capacity / 20uz, 0uz, ICONS.size());
  bool is_charging = status == "Charging";

  return std::format(
    "{}{}{}%",
    ICONS.at(icon_index),
    is_charging ? CHARGING_ICON : " ",
    capacity //
  );
}

[[nodiscard]]
auto date_time() -> std::string {
  auto system_time = std::chrono::system_clock::now();
  // TODO: replace with std whenever libc++ actually implements it
  auto local_time = system_time - TZ_OFFSET;
  return std::format(" {:%m/%d %H:%M}", local_time);
}

/* I/O */

// from the catppuccin mocha theme
namespace mocha {
  constexpr std::string_view MAUVE = "#cba6f7";
  constexpr std::string_view CRUST = "#11111b";
  constexpr std::string_view GREEN = "#a6e3a1";
  constexpr std::string_view YELLOW = "#f9e2af";
  constexpr std::string_view TEAL = "#94e2d5";
  constexpr std::string_view ROSEWATER = "#f5e0dc";
  constexpr std::string_view RED = "#f38ba8";
} // namespace mocha

/// a "status line" for swaybar is a JSON array of objects. each object is a "block" for swaybar.
void print_status_line() {
  std::println("["); // start status line

  // battery block
  std::println(
    R"({{ "full_text": " {} ", "background": "{}", "color": "{}", "separator_block_width": 0 }},)",
    battery_charge(),
    mocha::RED,
    mocha::CRUST //
  );

  // date time block
  std::println(
    R"({{ "full_text": " {} ", "background": "{}", "color": "{}", "separator_block_width": 0 }})",
    date_time(),
    mocha::MAUVE,
    mocha::CRUST //
  );

  std::println("],"); // end status line
  std::fflush(stdout);
}

auto main(int argc, char **argv) -> int {
  using namespace std::chrono_literals;
  // TODO: do we actually need args?
  auto args = std::span{argv, static_cast<std::size_t>(argc)};

  constexpr std::string_view PROTOCOL_HEADER = R"({ "version": 1 })";
  std::println("{}", PROTOCOL_HEADER);
  std::println("["); // start the infinite JSON list
  std::fflush(stdout);

  while (true) {
    print_status_line();
    // TODO: listen to some events in addition to polling
    std::this_thread::sleep_for(60s);
  }
}
