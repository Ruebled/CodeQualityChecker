#include <iostream>
#include <cstdio>
#include <sstream>
#include <vector>
#include <fstream>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Structure to store errors
struct Error {
    int line;
    std::string message;
    std::string code;
};

// Function to run the lexer app and capture its output
std::vector<Error> run_lexer(const std::string &filename) {
    std::vector<Error> errors;

    // Command to invoke the lexer
    std::string command = "./../../analyzer " + filename;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        perror("Failed to run lexer");
        return errors;
    }

    // Read JSON output from the lexer
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        try {
			//printf("buffer: %s\n", buffer);
            json j = json::parse(buffer);
            errors.push_back({j["message"], j["line"]});
        } catch (...) {
            std::cerr << "Failed to parse JSON: " << buffer << std::endl;
        }
    }
    pclose(pipe);
    return errors;
}

// Main function
int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    auto errors = run_lexer(filename);

    // Create the FTXUI interface
    using namespace ftxui;
	auto screen = ScreenInteractive::Fullscreen();

	auto middle = Renderer([] { return text("middle") | center; });
	auto left = Renderer([] { return text("left") | center; });
	auto bottom = Renderer([] { return text("bottom") | center; });

	int left_size = 40;
	int bottom_size = 8;

	auto container = middle;
	container = ResizableSplitBottom(bottom, container, &bottom_size);
	container = ResizableSplitLeft(left, container, &left_size);

	// Wrap the container to provide a Node
    auto mainWindowContent = Renderer(container, [&] {
        return container->Render();
    });

    // Title for the main window
    auto mainWindowTitle = text("Main Window") | hcenter | bold | color(Color::Blue);

    // Create the main window with the title and content
    auto mainWindow = window(mainWindowTitle, mainWindowContent->Render());

    // Renderer for the main application
    auto layout = Renderer([&] {
        return vbox({
            mainWindow | flex,
        });
    });

	screen.Loop(layout);

    // Display the errors in a list
   // std::vector<std::string> error_list;
   // for (const auto &error : errors) {
   //     error_list.push_back("Line " + std::to_string(error.line) + ": " + error.message);
   // }

   // int selected_error = 0;
   // auto list = Menu(&error_list, &selected_error);

	


    // Display the file content with highlighted errors
//    std::ifstream file(filename);
//    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//    auto editor = Renderer([&] {
//        Elements lines;
//        std::istringstream iss(content);
//        std::string line;
//        int line_number = 1;
//        while (std::getline(iss, line)) {
//            // Highlight the line if it has an error
//            bool has_error = false;
//            for (const auto &error : errors) {
//                if (error.line == line_number) {
//                    has_error = true;
//                    break;
//                }
//            }
//            if (has_error) {
//                lines.push_back(text(line) | color(Color::Red));
//            } else {
//                lines.push_back(text(line));
//            }
//            line_number++;
//        }
//        return vbox(std::move(lines));
//    });
//
//    auto layout = Renderer([&] {
//			return hbox({
//					list->Render() | flex, 
//					editor->Render() | flex
//					});
//	});
//
//
//    auto screen = ScreenInteractive::FitComponent();
//    screen.Loop(layout);

    return 0;
}

