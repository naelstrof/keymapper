
#include "Settings.h"
#include "common/output.h"

#if defined(_WIN32)
bool interpret_commandline(Settings& settings, int argc, wchar_t* argv[]) {
#  define T(text) L##text
  for (auto i = 1; i < argc; i++) {
    const auto argument = std::wstring_view(argv[i]);
#else
bool interpret_commandline(Settings& settings, int argc, char* argv[]) {
#  define T(text) text
  for (auto i = 1; i < argc; i++) {
    const auto argument = std::string_view(argv[i]);
#endif

    if (argument == T("-d") || argument == T("--debounce")) {
      settings.debounce = true;
    }
    else if (argument == T("-v") || argument == T("--verbose")) {
      settings.verbose = true;
    }
    else {
      return false;
    }
  }
  return true;
}

void print_help_message() {
  message(
    "keymapperd %s\n"
    "\n"
    "Usage: keymapperd [-options]\n"
    "  -v, --verbose        enable verbose output.\n"
    "  -d, --debounce       enable mouse button debouncing.\n"
    "  -h, --help           print this help.\n"
    "\n"
    "%s\n"
    "\n", about_header, about_footer);
}
