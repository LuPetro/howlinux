#pragma once

#include <cstddef>
#include <filesystem>
#include <iosfwd>
#include <string>
#include <vector>

namespace howlinux {

enum class CliCommand {
    search,
    list,
    show,
    validate,
    help,
    version,
};

struct CliOptions {
    CliCommand command{CliCommand::search};
    std::filesystem::path knowledge_path;
    bool knowledge_path_explicit{false};
    std::size_t limit{5};
    bool explain{false};
    bool json{false};
    std::string query;
    std::string entry_id;
};

struct CliParseResult {
    bool ok{false};
    CliOptions options;
    std::string error;
};

[[nodiscard]] CliParseResult parseCommandLine(const std::vector<std::string>& arguments);
[[nodiscard]] std::filesystem::path resolveKnowledgePath(
    const CliOptions& options,
    const std::filesystem::path& executable_path,
    const std::filesystem::path& current_directory);
void printHelp(std::ostream& output);

}  // namespace howlinux
