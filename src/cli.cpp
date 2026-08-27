#include "cli.hpp"

#include "config.hpp"

#include <charconv>
#include <cstdlib>
#include <filesystem>
#include <ostream>
#include <sstream>
#include <system_error>

namespace howlinux {
namespace {

std::string join(const std::vector<std::string>& values,
                 std::size_t first,
                 const char* separator = " ") {
    std::ostringstream output;
    for (std::size_t index = first; index < values.size(); ++index) {
        if (index > first) {
            output << separator;
        }
        output << values[index];
    }
    return output.str();
}

bool parseLimit(const std::string& value, std::size_t& output) {
    if (value.empty() || value.front() == '-' || value.front() == '+') {
        return false;
    }
    std::size_t parsed = 0;
    const auto* begin = value.data();
    const auto* end = value.data() + value.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (result.ec != std::errc{} || result.ptr != end || parsed == 0 ||
        parsed > SearchConfig{}.maximum_limit) {
        return false;
    }
    output = parsed;
    return true;
}

std::filesystem::path normalizedAbsolute(const std::filesystem::path& value,
                                         const std::filesystem::path& current_directory) {
    const auto combined = value.is_absolute() ? value : current_directory / value;
    std::error_code error;
    const auto canonical = std::filesystem::weakly_canonical(combined, error);
    return error ? combined.lexically_normal() : canonical;
}

bool isDirectory(const std::filesystem::path& value) {
    std::error_code error;
    return std::filesystem::is_directory(value, error) && !error;
}

}  // namespace

CliParseResult parseCommandLine(const std::vector<std::string>& arguments) {
    CliParseResult result;
    result.options.limit = SearchConfig{}.default_limit;
    std::vector<std::string> positional;
    bool parse_options = true;
    bool force_implicit_query = false;

    for (std::size_t index = 0; index < arguments.size(); ++index) {
        const auto& argument = arguments[index];
        if (parse_options && argument == "--") {
            force_implicit_query = positional.empty();
            parse_options = false;
            continue;
        }

        if (parse_options && (argument == "--help" || argument == "-h")) {
            result.ok = true;
            result.options.command = CliCommand::help;
            return result;
        }
        if (parse_options && (argument == "--version" || argument == "-V")) {
            result.ok = true;
            result.options.command = CliCommand::version;
            return result;
        }
        if (parse_options && argument == "--explain") {
            result.options.explain = true;
            continue;
        }
        if (parse_options && argument == "--json") {
            result.options.json = true;
            continue;
        }

        if (parse_options &&
            (argument == "--knowledge" || argument.rfind("--knowledge=", 0) == 0)) {
            std::string value;
            if (argument == "--knowledge") {
                if (++index >= arguments.size()) {
                    result.error = "--knowledge requires a path";
                    return result;
                }
                value = arguments[index];
            } else {
                value = argument.substr(std::string("--knowledge=").size());
            }
            if (value.empty()) {
                result.error = "--knowledge requires a non-empty path";
                return result;
            }
            if (result.options.knowledge_path_explicit) {
                result.error = "--knowledge may only be specified once";
                return result;
            }
            result.options.knowledge_path = value;
            result.options.knowledge_path_explicit = true;
            continue;
        }

        if (parse_options &&
            (argument == "--limit" || argument.rfind("--limit=", 0) == 0)) {
            std::string value;
            if (argument == "--limit") {
                if (++index >= arguments.size()) {
                    result.error = "--limit requires an integer";
                    return result;
                }
                value = arguments[index];
            } else {
                value = argument.substr(std::string("--limit=").size());
            }
            if (!parseLimit(value, result.options.limit)) {
                result.error = "--limit must be an integer from 1 to " +
                               std::to_string(SearchConfig{}.maximum_limit);
                return result;
            }
            continue;
        }

        if (parse_options && argument.size() > 1 && argument.front() == '-') {
            result.error = "unknown option: " + argument +
                           " (use -- before query flags)";
            return result;
        }
        positional.push_back(argument);
    }

    if (positional.empty()) {
        result.error = "missing query or command";
        return result;
    }

    const auto& command = positional.front();
    if (force_implicit_query) {
        result.options.command = CliCommand::search;
        result.options.query = join(positional, 0);
    } else if (command == "search") {
        if (positional.size() == 1) {
            result.error = "search requires a query";
            return result;
        }
        result.options.command = CliCommand::search;
        result.options.query = join(positional, 1);
    } else if (command == "list") {
        if (positional.size() != 1) {
            result.error = "list does not accept positional arguments";
            return result;
        }
        result.options.command = CliCommand::list;
    } else if (command == "show") {
        if (positional.size() != 2) {
            result.error = "show requires exactly one entry ID";
            return result;
        }
        result.options.command = CliCommand::show;
        result.options.entry_id = positional[1];
    } else if (command == "validate") {
        if (positional.size() > 2) {
            result.error = "validate accepts at most one knowledge path";
            return result;
        }
        if (positional.size() == 2) {
            if (result.options.knowledge_path_explicit) {
                result.error = "specify the validation path either positionally or with --knowledge";
                return result;
            }
            result.options.knowledge_path = positional[1];
            result.options.knowledge_path_explicit = true;
        }
        result.options.command = CliCommand::validate;
    } else {
        result.options.command = CliCommand::search;
        result.options.query = join(positional, 0);
    }

    result.ok = true;
    return result;
}

std::filesystem::path resolveKnowledgePath(
    const CliOptions& options,
    const std::filesystem::path& executable_path,
    const std::filesystem::path& current_directory) {
    if (options.knowledge_path_explicit) {
        return normalizedAbsolute(options.knowledge_path, current_directory);
    }

    if (const char* environment = std::getenv("HOWLINUX_KNOWLEDGE");
        environment != nullptr && *environment != '\0') {
        return normalizedAbsolute(environment, current_directory);
    }

    const auto executable_input = executable_path.is_absolute()
                                      ? executable_path
                                      : current_directory / executable_path;
    std::error_code error;
    auto executable = std::filesystem::weakly_canonical(executable_input, error);
    if (error) {
        executable = executable_input.lexically_normal();
    }
    const auto executable_directory = executable.parent_path();
    const std::filesystem::path configured_data_directory =
        kInstallDataDirectory;
    const std::vector<std::filesystem::path> candidates = {
        executable_directory / "knowledge",
        executable_directory.parent_path() / configured_data_directory /
            "howlinux" / "knowledge",
        current_directory / "knowledge",
    };
    for (const auto& candidate : candidates) {
        if (isDirectory(candidate)) {
            return normalizedAbsolute(candidate, current_directory);
        }
    }
    return normalizedAbsolute(current_directory / "knowledge", current_directory);
}

void printHelp(std::ostream& output) {
    output << "howlinux " << kVersion << " - offline Linux knowledge search\n\n"
           << "Usage:\n"
           << "  howlinux [options] <query...>\n"
           << "  howlinux [options] search <query...>\n"
           << "  howlinux [options] list\n"
           << "  howlinux [options] show <entry-id>\n"
           << "  howlinux [options] validate [path]\n\n"
           << "Options:\n"
           << "  -h, --help              Show this help and exit\n"
           << "  -V, --version           Show the version and exit\n"
           << "      --knowledge <path>  Use this knowledge directory\n"
           << "      --limit <n>         Return at most n results (1-100, default 5)\n"
           << "      --explain           Show ranking components and match reasons\n"
           << "      --json              Emit stable, ANSI-free JSON\n"
           << "      --                  Treat all following arguments as query text\n\n"
           << "Examples:\n"
           << "  howlinux rename folder\n"
           << "  howlinux --explain 'what does chmod 755 mean'\n"
           << "  howlinux --knowledge /srv/howlinux/knowledge validate\n"
           << "  howlinux search -- --recursive\n\n"
           << "howlinux reads local YAML/Markdown only. It never executes shown commands\n"
           << "and never uses a network or generative AI.\n";
}

}  // namespace howlinux
