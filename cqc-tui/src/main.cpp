#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
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

// Function to replace tabs with spaces in the file content
std::string replace_tabs_with_spaces(const std::string& str,
                                     int tab_width = 4) {
  std::string result;
  for (char ch : str) {
    if (ch == '\t') {
      result.append(tab_width, ' ');
    } else {
      result.push_back(ch);
    }
  }
  return result;
}

// Function to generate file tree for a directory
void build_tree(fs::path dir_path,
                std::vector<std::string>& tree,
                std::string prefix = "") {
  for (const auto& entry : fs::directory_iterator(dir_path)) {
    if (fs::is_directory(entry)) {
      build_tree(entry.path(), tree,
                 prefix + entry.path().filename().string() + "/");
    } else {
      tree.push_back(prefix + entry.path().filename().string());
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file or directory>" << std::endl;
    return 1;
  }

  // Expand the full path of the given argument
  fs::path base_path = fs::absolute(argv[1]);
  if (!fs::exists(base_path)) {
    std::cerr << "Path does not exist: " << argv[1] << std::endl;
    return 1;
  }

  std::string filename = argv[1];
  std::vector<std::string> file_lines;

  // Read file content if it's a file
  if (fs::is_regular_file(filename)) {
    std::ifstream file(filename);
    if (!file) {
      std::cerr << "Error opening file: " << filename << std::endl;
      return 1;
    }
    std::string line;
    while (std::getline(file, line)) {
      file_lines.push_back(line);
    }
  }

  auto errors = run_lexer(filename);

  // Create the FTXUI interface
  using namespace ftxui;
  auto screen = ScreenInteractive::Fullscreen();

  // Track the current highlighted line
  int current_line = -1;
  // Track the current scroll position
  int scroll_offset = 0;
  // Number of visible lines in the editor
  int visible_lines = 25;

  std::string base_path_str = base_path.string();
  std::vector<std::string> dynamic_file_lines;
  std::vector<Error> dynamic_errors;

  auto error_list = Container::Vertical({});
  auto update_error_list = [&] {
    error_list->DetachAllChildren();  // Clear previous error buttons
    for (const auto& error : dynamic_errors) {
      auto error_button = Button(
          "Line " + std::to_string(error.line) + ": " + error.message,
          [&] {
            current_line = error.line - 1;
            scroll_offset = std::max(0, error.line - 1 - visible_lines / 2);
          },
          ButtonOption::Ascii());
      error_list->Add(error_button);
    }
  };

  auto update_editor = [&](const std::string& file_path) {
    // Load file content
    dynamic_file_lines.clear();
    std::ifstream file(file_path);
    if (file) {
      std::string line;
      while (std::getline(file, line)) {
        dynamic_file_lines.push_back(line);
      }
    } else {
      std::cerr << "Failed to open file: " << file_path << std::endl;
      return;
    }

    // Run lexer on the new file
    dynamic_errors = run_lexer(file_path);

    // Reset scroll and highlight positions
    current_line = -1;
    scroll_offset = 0;
    update_error_list();  // Update the error list for the new file
  };

  // Generate the left panel based on the argument (file or directory)
  std::vector<std::string> left_tree;

  // Initialize editor with the provided argument
  if (fs::is_regular_file(base_path)) {
    update_editor(base_path_str);
  }

  auto file_view = Renderer([&] {
    scroll_offset =
        std::max(0, std::min(scroll_offset,
                             (int)dynamic_file_lines.size() - visible_lines));
    Elements elements;
    for (size_t i = scroll_offset;
         i < scroll_offset + visible_lines && i < dynamic_file_lines.size();
         ++i) {
      auto line_number = text(std::to_string(i + 1) + " ") | color(Color::Blue);
      auto line_content = text(replace_tabs_with_spaces(dynamic_file_lines[i]));
      if (static_cast<int>(i) == current_line) {
        elements.push_back(hbox({line_number, line_content}) |
                           bgcolor(Color::GrayDark));
      } else {
        elements.push_back(hbox({line_number, line_content}));
      }
    }
    return vbox(std::move(elements)) | vscroll_indicator | frame | flex;
  });

  // Update the error list whenever the editor is updated
  update_editor(filename);  // Initial load
  update_error_list();

  // Build the tree and buttons
  if (fs::is_directory(base_path)) {
    // If it's a directory, recursively build the tree
    build_tree(base_path, left_tree);
  } else {
    // If it's a file, just show the file name
    left_tree.push_back("File: " + base_path.filename().string());
  }

  auto left_panel_buttons = Container::Vertical({});
  for (const auto& item : left_tree) {
    left_panel_buttons->Add(Button(
        item,
        [&, item] {
          auto file_path = fs::path(base_path) / item;  // Append to base path
          std::cout << "Clicked on " << file_path << std::endl;
          if (fs::is_regular_file(file_path)) {
            update_editor(file_path.string());
          }
        },
        ButtonOption::Ascii()));
  }

  auto left_panel = Renderer(left_panel_buttons, [&] {
    return vbox({
               text("Project Explorer") | bold | hcenter,
               separator(),
               left_panel_buttons->Render(),
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
