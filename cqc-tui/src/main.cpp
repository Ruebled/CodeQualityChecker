#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Structure to store errors
struct Error {
  int line;
  std::string message;
};

// Function to run the lexer app and capture its output
std::vector<Error> run_lexer(const std::string& filename) {
  std::vector<Error> errors;

  // Command to invoke the lexer
  std::string command = "./../../bin/analyzer " + filename;
  FILE* pipe = popen(command.c_str(), "r");
  if (!pipe) {
    perror("Failed to run lexer");
    return errors;
  }

  // Read JSON output from the lexer
  char buffer[256];
  while (fgets(buffer, sizeof(buffer), pipe)) {
    try {
      json j = json::parse(buffer);
      errors.push_back({j["line"], j["message"]});
    } catch (...) {
      std::cerr << "Failed to parse JSON: " << buffer << std::endl;
    }
  }
  pclose(pipe);
  return errors;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::vector<std::string> file_lines;

  // Read the file content into lines
  std::ifstream file(filename);
  if (!file) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return 1;
  }

  std::string line;
  while (std::getline(file, line)) {
    file_lines.push_back(line);
  }

  auto errors = run_lexer(filename);

  // Create the FTXUI interface
  using namespace ftxui;
  auto screen = ScreenInteractive::Fullscreen();

  // Track the current highlighted line
  int current_line = 0;

  // Renderer for the file content
  auto file_view = Renderer([&] {
    Elements elements;
    for (size_t i = 0; i < file_lines.size(); ++i) {
      auto line_number = text(std::to_string(i + 1) + " ") | color(Color::Blue);
      auto line_content = text(file_lines[i]);
      if (static_cast<int>(i) == current_line) {
        elements.push_back(hbox({line_number, line_content}) |
                           bgcolor(Color::GrayDark));
      } else {
        elements.push_back(hbox({line_number, line_content}));
      }
    }
    return vbox(std::move(elements)) | vscroll_indicator | frame | flex;
  });

  // Renderer for the error list with clickable items
  auto error_list = Container::Vertical({});  // Container for the errors

  for (const auto& error : errors) {
    auto error_button = Button(
        "Line " + std::to_string(error.line) + ": " + error.message,
        [&] { current_line = error.line - 1; },
        ButtonOption::Ascii());  // Adjust to zero-based index
    error_list->Add(error_button);
  }

  // Renderer for the left panel
  auto left_panel = Renderer([] {
    return vbox({
               text("Project Explorer") | bold | hcenter,
               separator(),
               text("File 1"),
               text("File 2"),
               text("File 3"),
           }) |
           flex;
  });

  // Combine panels
  auto middle = file_view;
  auto left = left_panel;
  auto bottom = Renderer(error_list, [&] {
    return vbox({
               text("Errors:") | bold | hcenter,
               separator(),
               error_list->Render(),
           }) |
           vscroll_indicator | frame | flex;
  });

  // Initial sizes of the resizable splits
  int left_size = 40;
  int bottom_size = 10;

  // Apply resizable splits
  auto container = ResizableSplitBottom(bottom, middle, &bottom_size);
  container = ResizableSplitLeft(left, container, &left_size);

  // Title bar
  auto title = text("Code Quality Checker") | bold | hcenter;

  int value = 0;
  auto action = [&] { value++; };

  // Menu bar with small buttons
  auto button1 = Button("Run", action, ButtonOption::Ascii());
  auto button2 = Button("Build", action, ButtonOption::Ascii());

  // Menu bar layout
  auto menu_bar = Container::Horizontal({
      button1,
      button2,
  });

  // Final layout
  auto layout =
      Renderer(Container::Vertical({
                   menu_bar,
                   container,
               }),
               [&] {
                 return vbox({
                            title,                       // Title
                            hbox({menu_bar->Render()}),  // Menu bar
                            separator(),                 // Separator
                            container->Render() | flex,  // Resizable content
                        }) |
                        border;
               });

  screen.Loop(layout);
  return 0;
}
