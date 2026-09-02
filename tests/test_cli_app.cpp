#include "fixtures.hpp"
#include "test_harness.hpp"

#include "app.hpp"
#include "cli.hpp"
#include "config.hpp"

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

using namespace howlinux;

namespace {

struct ApplicationResult {
    int exit_code{0};
    std::string output;
    std::string error;
};

ApplicationResult run(const std::vector<std::string>& arguments,
                      const std::filesystem::path& executable,
                      const std::filesystem::path& current_directory) {
    std::ostringstream output;
    std::ostringstream error;
    const int exit_code = runApplication(arguments, executable,
                                         current_directory, output, error);
    return {exit_code, output.str(), error.str()};
}

std::string pathString(const std::filesystem::path& value) {
    return value.string();
}

}  // namespace

HL_TEST(cli_parses_help_version_queries_subcommands_and_options) {
    const auto help = parseCommandLine({"--help"});
    HL_REQUIRE(help.ok);
    HL_REQUIRE(help.options.command == CliCommand::help);

    const auto version = parseCommandLine({"-V"});
    HL_REQUIRE(version.ok);
    HL_REQUIRE(version.options.command == CliCommand::version);

    const auto direct = parseCommandLine({"rename", "a", "folder"});
    HL_REQUIRE(direct.ok);
    HL_REQUIRE(direct.options.command == CliCommand::search);
    HL_REQUIRE_EQ(direct.options.query, "rename a folder");

    const auto search = parseCommandLine(
        {"--knowledge", "fixture path", "--limit=2", "--explain", "--json",
         "search", "rename", "folder"});
    HL_REQUIRE(search.ok);
    HL_REQUIRE(search.options.command == CliCommand::search);
    HL_REQUIRE(search.options.knowledge_path_explicit);
    HL_REQUIRE_EQ(search.options.knowledge_path,
                  std::filesystem::path("fixture path"));
    HL_REQUIRE_EQ(search.options.limit, std::size_t{2});
    HL_REQUIRE(search.options.explain);
    HL_REQUIRE(search.options.json);
    HL_REQUIRE_EQ(search.options.query, "rename folder");

    const auto validate = parseCommandLine({"validate", "relative/knowledge"});
    HL_REQUIRE(validate.ok);
    HL_REQUIRE(validate.options.command == CliCommand::validate);
    HL_REQUIRE(validate.options.knowledge_path_explicit);

    const auto list = parseCommandLine({"list"});
    HL_REQUIRE(list.ok);
    HL_REQUIRE(list.options.command == CliCommand::list);

    const auto show = parseCommandLine({"show", "rename-folder"});
    HL_REQUIRE(show.ok);
    HL_REQUIRE(show.options.command == CliCommand::show);
    HL_REQUIRE_EQ(show.options.entry_id, "rename-folder");
}

HL_TEST(cli_option_terminator_preserves_query_flags) {
    const auto parsed =
        parseCommandLine({"search", "--", "--recursive", "-r", "2>"});
    HL_REQUIRE(parsed.ok);
    HL_REQUIRE(parsed.options.command == CliCommand::search);
    HL_REQUIRE_EQ(parsed.options.query, "--recursive -r 2>");

    const auto direct = parseCommandLine({"--", "--recursive"});
    HL_REQUIRE(direct.ok);
    HL_REQUIRE_EQ(direct.options.query, "--recursive");
}

HL_TEST(cli_rejects_missing_invalid_and_duplicate_arguments) {
    HL_REQUIRE(!parseCommandLine({}).ok);
    HL_REQUIRE(!parseCommandLine({"--unknown"}).ok);
    HL_REQUIRE(!parseCommandLine({"--limit"}).ok);
    HL_REQUIRE(!parseCommandLine({"--limit", "0", "mv"}).ok);
    HL_REQUIRE(!parseCommandLine({"--limit", "-1", "mv"}).ok);
    HL_REQUIRE(!parseCommandLine({"--limit", "101", "mv"}).ok);
    HL_REQUIRE(!parseCommandLine({"--limit", "word", "mv"}).ok);
    HL_REQUIRE(!parseCommandLine({"--knowledge"}).ok);
    HL_REQUIRE(!parseCommandLine(
                    {"--knowledge", "one", "--knowledge", "two", "mv"})
                    .ok);
    HL_REQUIRE(!parseCommandLine({"search"}).ok);
    HL_REQUIRE(!parseCommandLine({"show"}).ok);
    HL_REQUIRE(!parseCommandLine({"show", "one", "two"}).ok);
    HL_REQUIRE(!parseCommandLine({"list", "extra"}).ok);
    HL_REQUIRE(!parseCommandLine(
                    {"--knowledge", "one", "validate", "two"})
                    .ok);
}

HL_TEST(cli_help_documents_all_v1_modes) {
    std::ostringstream output;
    printHelp(output);
    const auto help = output.str();
    HL_REQUIRE_CONTAINS(help, "howlinux");
    HL_REQUIRE_CONTAINS(help, "--knowledge");
    HL_REQUIRE_CONTAINS(help, "--limit");
    HL_REQUIRE_CONTAINS(help, "--explain");
    HL_REQUIRE_CONTAINS(help, "--json");
    HL_REQUIRE_CONTAINS(help, "validate");
    HL_REQUIRE_CONTAINS(help, "list");
    HL_REQUIRE_CONTAINS(help, "show");
    HL_REQUIRE_CONTAINS(help, "never executes");
}

HL_TEST(cli_resolves_explicit_executable_relative_and_cwd_paths) {
    hltest::EnvironmentGuard environment("HOWLINUX_KNOWLEDGE");
    hltest::TemporaryDirectory temporary;
    const auto current = temporary.path() / "working directory";
    const auto binary_directory = temporary.path() / "bin";
    const auto binary = binary_directory / "howlinux";
    std::filesystem::create_directories(current);
    std::filesystem::create_directories(binary_directory / "knowledge");
    std::filesystem::create_directories(current / "knowledge");
    hltest::writeText(binary, "not an executable; path resolution fixture\n");

    CliOptions explicit_options;
    explicit_options.knowledge_path_explicit = true;
    explicit_options.knowledge_path = "custom knowledge";
    std::filesystem::create_directories(current / explicit_options.knowledge_path);
    const auto explicit_result =
        resolveKnowledgePath(explicit_options, binary, current);
    HL_REQUIRE_EQ(explicit_result,
                  std::filesystem::weakly_canonical(
                      current / explicit_options.knowledge_path));

    CliOptions defaults;
    const auto executable_relative =
        resolveKnowledgePath(defaults, binary, current);
    HL_REQUIRE_EQ(executable_relative,
                  std::filesystem::weakly_canonical(
                      binary_directory / "knowledge"));

    std::filesystem::remove(binary_directory / "knowledge");
    const auto installed_knowledge =
        binary_directory.parent_path() / kInstallDataDirectory / "howlinux" /
        "knowledge";
    std::filesystem::create_directories(installed_knowledge);
    const auto installed_relative =
        resolveKnowledgePath(defaults, binary, current);
    HL_REQUIRE_EQ(installed_relative,
                  std::filesystem::weakly_canonical(installed_knowledge));

    std::filesystem::remove(installed_knowledge);
    const auto cwd_relative = resolveKnowledgePath(defaults, binary, current);
    HL_REQUIRE_EQ(cwd_relative,
                  std::filesystem::weakly_canonical(current / "knowledge"));
}

HL_TEST(app_help_version_invalid_arguments_and_missing_knowledge_exitcodes) {
    hltest::TemporaryDirectory temporary;
    const auto executable = temporary.path() / "bin/howlinux";
    const auto cwd = temporary.path() / "cwd";
    std::filesystem::create_directories(cwd);

    const auto help = run({"--help"}, executable, cwd);
    HL_REQUIRE_EQ(help.exit_code, 0);
    HL_REQUIRE_CONTAINS(help.output, "Usage:");
    HL_REQUIRE(help.error.empty());

    const auto version = run({"--version"}, executable, cwd);
    HL_REQUIRE_EQ(version.exit_code, 0);
    HL_REQUIRE_CONTAINS(version.output, kVersion);
    HL_REQUIRE(version.error.empty());

    const auto invalid = run({}, executable, cwd);
    HL_REQUIRE_EQ(invalid.exit_code, 2);
    HL_REQUIRE_CONTAINS(invalid.error, "missing query or command");

    const auto missing = run(
        {"--knowledge", pathString(temporary.path() / "absent"), "mv"},
        executable, cwd);
    HL_REQUIRE_EQ(missing.exit_code, 3);
    HL_REQUIRE_CONTAINS(missing.error, "knowledge root");
}

HL_TEST(app_confident_markdown_and_json_search_are_clean) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge with spaces";
    const auto cwd = temporary.path() / "unrelated cwd";
    const auto executable = temporary.path() / "elsewhere/howlinux";
    std::filesystem::create_directories(cwd);
    hltest::writeSearchKnowledge(root);

    const auto markdown = run(
        {"--knowledge", pathString(root), "rename", "folder"}, executable, cwd);
    HL_REQUIRE_EQ(markdown.exit_code, 0);
    HL_REQUIRE_CONTAINS(markdown.output, "Rename");
    HL_REQUIRE_CONTAINS(markdown.output, "## Rename safely");
    HL_REQUIRE_CONTAINS(markdown.output, "```bash");
    HL_REQUIRE_CONTAINS(markdown.output, "mv OLD_NAME NEW_NAME");
    HL_REQUIRE_CONTAINS(markdown.output, "Größe");
    HL_REQUIRE(markdown.error.empty());

    const auto json = run(
        {"--knowledge", pathString(root), "--json", "--explain", "rename",
         "folder"},
        executable, cwd);
    HL_REQUIRE_EQ(json.exit_code, 0);
    HL_REQUIRE(hltest::isValidJson(json.output));
    HL_REQUIRE_CONTAINS(json.output, "\"status\":\"confident\"");
    HL_REQUIRE_CONTAINS(json.output, "\"id\":\"rename-folder\"");
    HL_REQUIRE_CONTAINS(json.output, "\"breakdown\"");
    HL_REQUIRE(json.output.find('\x1b') == std::string::npos);
    HL_REQUIRE(json.error.empty());
}

HL_TEST(app_uncertain_and_no_match_return_one_with_stable_json) {
    hltest::TemporaryDirectory temporary;
    const auto tie_root = temporary.path() / "tie";
    const auto cwd = temporary.path() / "cwd";
    const auto executable = temporary.path() / "bin/howlinux";
    std::filesystem::create_directories(cwd);
    hltest::writeTieKnowledge(tie_root);

    const auto uncertain = run(
        {"--knowledge", pathString(tie_root), "sharedtoken"}, executable, cwd);
    HL_REQUIRE_EQ(uncertain.exit_code, 1);
    HL_REQUIRE_CONTAINS(uncertain.output, "alpha");
    HL_REQUIRE_CONTAINS(uncertain.output, "beta");

    const auto no_match = run(
        {"--knowledge", pathString(tie_root), "quantum", "zephyr"}, executable,
        cwd);
    HL_REQUIRE_EQ(no_match.exit_code, 1);
    HL_REQUIRE_CONTAINS(no_match.output, "quantum zephyr");

    const auto no_match_json = run(
        {"--knowledge", pathString(tie_root), "--json", "quantum", "zephyr"},
        executable, cwd);
    HL_REQUIRE_EQ(no_match_json.exit_code, 1);
    HL_REQUIRE(hltest::isValidJson(no_match_json.output));
    HL_REQUIRE_CONTAINS(no_match_json.output, "\"status\":\"no_match\"");
    HL_REQUIRE(no_match_json.output.find('\x1b') == std::string::npos);
}

HL_TEST(app_list_and_show_support_human_and_json_output) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge";
    const auto cwd = temporary.path() / "cwd";
    const auto executable = temporary.path() / "bin/howlinux";
    std::filesystem::create_directories(cwd);
    hltest::writeSearchKnowledge(root);

    const auto list =
        run({"--knowledge", pathString(root), "list"}, executable, cwd);
    HL_REQUIRE_EQ(list.exit_code, 0);
    HL_REQUIRE_CONTAINS(list.output, "rename-folder");
    HL_REQUIRE_CONTAINS(list.output, "chmod-755");

    const auto list_json = run(
        {"--knowledge", pathString(root), "--json", "list"}, executable, cwd);
    HL_REQUIRE_EQ(list_json.exit_code, 0);
    HL_REQUIRE(hltest::isValidJson(list_json.output));
    HL_REQUIRE_CONTAINS(list_json.output, "rename-folder");

    const auto show = run(
        {"--knowledge", pathString(root), "show", "rename-folder"}, executable,
        cwd);
    HL_REQUIRE_EQ(show.exit_code, 0);
    HL_REQUIRE_CONTAINS(show.output, "mv OLD_NAME NEW_NAME");

    const auto show_json = run(
        {"--knowledge", pathString(root), "--json", "show", "rename-folder"},
        executable, cwd);
    HL_REQUIRE_EQ(show_json.exit_code, 0);
    HL_REQUIRE(hltest::isValidJson(show_json.output));
    HL_REQUIRE_CONTAINS(show_json.output, "Größe");

    const auto missing = run(
        {"--knowledge", pathString(root), "show", "../../etc/passwd"},
        executable, cwd);
    HL_REQUIRE_EQ(missing.exit_code, 1);
    HL_REQUIRE_CONTAINS(missing.output, "../../etc/passwd");
}

HL_TEST(app_validate_distinguishes_clean_entry_errors_and_global_config_errors) {
    hltest::TemporaryDirectory temporary;
    const auto cwd = temporary.path() / "cwd";
    const auto executable = temporary.path() / "bin/howlinux";
    std::filesystem::create_directories(cwd);

    const auto valid_root = temporary.path() / "valid";
    hltest::writeSearchKnowledge(valid_root);
    const auto valid = run({"validate", pathString(valid_root)}, executable, cwd);
    HL_REQUIRE_EQ(valid.exit_code, 0);
    HL_REQUIRE(!valid.output.empty());

    const auto valid_json = run(
        {"--json", "validate", pathString(valid_root)}, executable, cwd);
    HL_REQUIRE_EQ(valid_json.exit_code, 0);
    HL_REQUIRE(hltest::isValidJson(valid_json.output));

    const auto broken_root = temporary.path() / "broken";
    auto good = hltest::entrySpec("good", "Good entry");
    hltest::writeEntry(broken_root, "topics/good", good);
    hltest::writeText(broken_root / "topics/broken/meta.yaml",
                      "id: broken\ntitle: Broken\ntype: howto\n");
    const auto broken =
        run({"validate", pathString(broken_root)}, executable, cwd);
    HL_REQUIRE_EQ(broken.exit_code, 1);
    HL_REQUIRE_CONTAINS(broken.output, "broken");

    const auto invalid_concepts_root = temporary.path() / "invalid-concepts";
    hltest::writeEntry(invalid_concepts_root, "topics/good", good);
    hltest::writeText(invalid_concepts_root / "concepts.yaml",
                      "concepts:\n  first: [shared]\n  second: [shared]\n");
    const auto invalid_concepts =
        run({"validate", pathString(invalid_concepts_root)}, executable, cwd);
    HL_REQUIRE_EQ(invalid_concepts.exit_code, 3);
    HL_REQUIRE_CONTAINS(invalid_concepts.output, "collision");

    const auto missing_json = run(
        {"--json", "validate", pathString(temporary.path() / "missing")},
        executable, cwd);
    HL_REQUIRE_EQ(missing_json.exit_code, 3);
    HL_REQUIRE(hltest::isValidJson(missing_json.output));
}

HL_TEST(app_option_terminator_reaches_search_instead_of_cli_error) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge";
    const auto cwd = temporary.path() / "cwd";
    const auto executable = temporary.path() / "bin/howlinux";
    std::filesystem::create_directories(cwd);
    hltest::writeSearchKnowledge(root);

    const auto result = run(
        {"--knowledge", pathString(root), "search", "--", "--recursive"},
        executable, cwd);
    HL_REQUIRE_EQ(result.exit_code, 1);
    HL_REQUIRE(result.error.find("unknown option") == std::string::npos);
}

HL_TEST(app_never_executes_shell_syntax_from_a_query) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge";
    const auto cwd = temporary.path() / "cwd";
    const auto executable = temporary.path() / "bin/howlinux";
    const auto marker = temporary.path() / "must-not-exist";
    std::filesystem::create_directories(cwd);
    hltest::writeSearchKnowledge(root);

    const std::string payload = "$(touch " + marker.string() + ")";
    const auto result = run(
        {"--knowledge", pathString(root), "--", payload}, executable, cwd);
    HL_REQUIRE(result.exit_code == 0 || result.exit_code == 1);
    HL_REQUIRE(!std::filesystem::exists(marker));
}
