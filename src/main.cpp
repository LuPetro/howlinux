#include "app.hpp"
#include "cli.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::vector<std::string> arguments;
    arguments.reserve(argc > 1 ? static_cast<std::size_t>(argc - 1) : 0);
    for (int index = 1; index < argc; ++index) {
        arguments.emplace_back(argv[index]);
    }

    try {
        std::error_code error;
        auto current_directory = std::filesystem::current_path(error);
        if (error) {
            const auto parsed = howlinux::parseCommandLine(arguments);
            if (parsed.ok && (parsed.options.command == howlinux::CliCommand::help ||
                              parsed.options.command == howlinux::CliCommand::version)) {
                return howlinux::runApplication(arguments,
                                                argc > 0 ? argv[0] : "howlinux",
                                                ".",
                                                std::cout,
                                                std::cerr);
            }
            std::cerr << "Configuration error: cannot determine current directory: "
                      << error.message() << '\n';
            return 3;
        }
        std::error_code executable_error;
        auto executable_path =
            std::filesystem::read_symlink("/proc/self/exe", executable_error);
        if (executable_error || executable_path.empty()) {
            executable_path = argc > 0 ? argv[0] : "howlinux";
        }
        return howlinux::runApplication(arguments,
                                        executable_path,
                                        current_directory,
                                        std::cout,
                                        std::cerr);
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << '\n';
        return 3;
    } catch (...) {
        std::cerr << "Fatal error: unknown exception\n";
        return 3;
    }
}
