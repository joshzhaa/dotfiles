// a basic swaybar status line program.

#include <fcntl.h>     // O_CREAT and friends
#include <spawn.h>     // posix_spawn
#include <string.h>    // for strerror
#include <sys/types.h> // ssize_t
#include <sys/wait.h>  // waitpid()
// <stdio.h> needed to get `stdout` which is standardized as a preprocessor macro, shockingly
#include <stdio.h>
#include <unistd.h> // for pipe2

import std;

/* CONFIGURATION */
constexpr std::string_view BATTERY_CAPACITY = "/sys/class/power_supply/BAT1/capacity";
constexpr std::string_view BATTERY_STATUS = "/sys/class/power_supply/BAT1/status";
constexpr auto VOLUME_COMMAND = std::array{
  "wpctl",
  "get-volume",
  "@DEFAULT_AUDIO_SINK@",
  static_cast<char const*>(nullptr), // needed to terminate an argv
};
constexpr auto TZ_OFFSET = std::chrono::hours{4}; // from system_clock::now()

/* HELPERS */
template <typename T>
void assert_eq(
  T const& value, //
  T const& expected,
  std::source_location const location =
    std::source_location::current() // default arg gives caller instead of callee
) {
  if (value == expected) {
    return;
  }
  std::println(
    "ERROR at {}:{}:{} in {}. Received value '{}', expected '{}'",
    location.file_name(),
    location.line(),
    location.column(),
    location.function_name(),
    value,
    expected
  );
  std::exit(1);
}

template <typename T, typename U>
void assert_neq(
  T const& value, //
  U const& expected,
  std::source_location const location =
    std::source_location::current() // default arg gives caller instead of callee
) {
  if (value != expected) {
    return;
  }
  std::println(
    "ERROR at {}:{}:{} in {}. Value was unexpectedly {}",
    location.file_name(),
    location.line(),
    location.column(),
    location.function_name(),
    expected
  );
  std::exit(1);
}

// RAII wrapper over std::fopen
struct File {
  std::FILE* fp;

  // this is technically not safe, but I hate fstream and c strings
  File(std::string_view filename) : fp{std::fopen(filename.data(), "r")} {
    assert_neq(fp, nullptr);
  }

  ~File() { std::fclose(fp); }
};

using u8 = std::uint8_t;
using i32 = std::int32_t;
using usize = std::size_t;
using isize = ssize_t;

// as a reminder, argv[0] is the name of the program and argv[-1] is a nullptr.
// we use posix_spawnp, so argv[0] is looked up in PATH.
[[nodiscard]]
auto spawn_process(std::span<char const* const> argv) -> std::string {
  // TODO: make this more beautiful
  struct SpawnParams {
    posix_spawn_file_actions_t file_actions;
    posix_spawnattr_t attr;

    std::array<i32, 2> pipe_fd;

    SpawnParams() {
      posix_spawn_file_actions_init(&this->file_actions);
      posix_spawnattr_init(&this->attr);

      // set up one way pipe for output from child proc to this proc
      i32 error = pipe2(pipe_fd.data(), O_CLOEXEC);
      assert_eq(error, 0);

      error = posix_spawn_file_actions_adddup2(
        &this->file_actions, //
        pipe_fd.at(1),
        STDOUT_FILENO
      );
      assert_eq(error, 0);
      posix_spawn_file_actions_addclose(&this->file_actions, pipe_fd.at(0));

      // I think this one is automatically closed by O_CLOEXEC.
      // posix_spawn_file_actions_addclose(&this->file_actions, pipe_fd.at(1));
    }
    ~SpawnParams() {
      close(this->pipe_fd.at(0));
      close(this->pipe_fd.at(1));

      posix_spawnattr_destroy(&this->attr);
      posix_spawn_file_actions_destroy(&this->file_actions);
    }
  };

  auto params = SpawnParams{};

  auto read_pipe = [](i32 pipe_fd) -> std::string {
    auto output = std::string{};
    constexpr usize BUFSIZE = 128; // expecting this to not be very large
    std::array<char, BUFSIZE> buffer;

    while (true) {
      isize const n = read(pipe_fd, buffer.data(), buffer.size());
      assert_neq(n, -1);

      if (n == 0)
        break;

      output.append(buffer.data(), static_cast<usize>(n));
    }
    return output;
  };

  extern char** environ;
  auto pid = pid_t{};
  i32 error = posix_spawnp(
    &pid,
    argv.at(0),
    &params.file_actions,
    &params.attr,
    const_cast<char* const*>(argv.data()), // posix_spawn takes non-const char pointers, so evil.
    environ
  );
  assert_eq(error, 0);
  close(params.pipe_fd.at(1)); // not writing anything to the child proc
  params.pipe_fd.at(1) = -1;

  i32 status;
  error = waitpid(pid, &status, 0);
  assert_neq(error, -1);
  return read_pipe(params.pipe_fd.at(0));
}

/* "BUSINESS-LOGIC" COMPONENTS */

[[nodiscard]]
auto audio_volume() -> std::string {
  auto output = spawn_process(VOLUME_COMMAND);
  constexpr usize PREFIX_LEN = sizeof "Volume:";
  double volume;
  auto [_, error] = std::from_chars( //
    output.data() + PREFIX_LEN,
    output.data() + output.size(),
    volume
  );
  // TODO: std::print doesn't know how to format errc, fix somehow
  // assert_eq(error, std::errc());

  // TODO: add support for   and    and 
  return std::format(" {:.0f}%", volume * 100);
}

[[nodiscard]]
auto battery_charge() -> std::string {
  constexpr static auto ICONS = std::array{"", "", "", "", ""};
  constexpr static auto CHARGING_ICON = " 󱐋 ";

  auto read_capacity_file = []() {
    auto file = File{BATTERY_CAPACITY};
    usize capacity;
    i32 n = std::fscanf(file.fp, "%zu", &capacity);
    assert_eq(n, 1);
    return capacity;
  };

  auto read_status_file = []() {
    auto file = File{BATTERY_STATUS};
    constexpr usize BUFSIZE = 32;
    std::array<char, BUFSIZE> buffer;
    i32 n = std::fscanf(file.fp, "%31s", buffer.data());
    assert_eq(n, 1);
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

/// a "status line" for swaybar is a JSON array of objects. each object is a
/// "block" for swaybar.
void print_status_line() {
  std::println("["); // start status line

  // volume block
  std::println(
    R"({{ "full_text": " {} ", "background": "{}", "color": "{}", "separator_block_width": 0 }},)",
    audio_volume(),
    mocha::YELLOW,
    mocha::CRUST //
  );

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

auto main(int argc, char** argv) -> int {
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
