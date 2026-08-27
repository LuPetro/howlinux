#include "fixtures.hpp"
#include "test_harness.hpp"

#include "concepts.hpp"
#include "query.hpp"

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

using namespace howlinux;

HL_TEST(query_tokenizer_preserves_linux_special_tokens) {
    const auto actual = tokenize(
        "How, PLEASE rename_the-folder TAR.GZ v1.2.3 755 -r "
        "--recursive 2> MV mv");
    const std::vector<std::string> expected = {
        "rename", "folder",      "tar.gz", "v1.2.3", "755",
        "-r",     "--recursive", "2>",     "mv",
    };
    HL_REQUIRE_EQ(actual, expected);

    const auto shell_boundaries = tokenize("mv|cp;rm&&chmod/foo\\bar");
    const std::vector<std::string> expected_boundaries = {
        "mv", "cp", "rm", "chmod", "foo", "bar"};
    HL_REQUIRE_EQ(shell_boundaries, expected_boundaries);

    const auto utf8 = tokenize("Größe ÄPFEL");
    const std::vector<std::string> expected_utf8 = {"größe", "Äpfel"};
    HL_REQUIRE_EQ(utf8, expected_utf8);
}

HL_TEST(query_stopwords_and_stable_deduplication) {
    const auto filtered = tokenize(
        "a an the i do does can how please to of is for me my "
        "mv cp rm chmod tar ssh sudo");
    const std::vector<std::string> expected = {
        "mv", "cp", "rm", "chmod", "tar", "ssh", "sudo"};
    HL_REQUIRE_EQ(filtered, expected);

    const auto deduplicated = tokenize("mv rename mv folder rename");
    const std::vector<std::string> expected_deduplicated = {
        "mv", "rename", "folder"};
    HL_REQUIRE_EQ(deduplicated, expected_deduplicated);

    const auto ordered = tokenize("mv rename mv", true, false);
    const std::vector<std::string> expected_ordered = {"mv", "rename", "mv"};
    HL_REQUIRE_EQ(ordered, expected_ordered);
    HL_REQUIRE(tokenize("how can i please").empty());
}

HL_TEST(query_context_contains_sequence_normalized_phrases_and_types) {
    QueryProcessor processor(nullptr, {"mv", "chmod"});

    const auto context = processor.process("How can I rename rename a folder?");
    HL_REQUIRE_EQ(context.original, "How can I rename rename a folder?");
    const std::vector<std::string> expected_sequence = {
        "rename", "rename", "folder"};
    const std::vector<std::string> expected_tokens = {"rename", "folder"};
    HL_REQUIRE_EQ(context.sequence_tokens, expected_sequence);
    HL_REQUIRE_EQ(context.tokens, expected_tokens);
    HL_REQUIRE_EQ(context.normalized_query, "rename folder");
    HL_REQUIRE(context.type == QueryType::how_to);
    HL_REQUIRE(std::find(context.phrases.begin(), context.phrases.end(),
                         "rename folder") != context.phrases.end());

    HL_REQUIRE(processor.process("what does chmod mean").type ==
               QueryType::explain);
    HL_REQUIRE(processor.process("explain chmod").type == QueryType::explain);
    HL_REQUIRE(processor.process("why can't chmod work").type == QueryType::why);
    HL_REQUIRE(processor.process("chmod 755").type == QueryType::command);
    HL_REQUIRE(processor.process("--recursive").type == QueryType::command);
    HL_REQUIRE(processor.process("ordinary filesystem question").type ==
               QueryType::general);
    HL_REQUIRE_EQ(queryTypeName(QueryType::how_to), "how_to");
}

HL_TEST(concepts_missing_file_is_valid_and_clears_previous_state) {
    hltest::TemporaryDirectory temporary;
    const auto valid_file = temporary.path() / "valid.yaml";
    hltest::writeText(valid_file, R"yaml(concepts:
  folder:
    - directory
)yaml");

    ConceptDictionary dictionary;
    const auto loaded = dictionary.load(valid_file);
    HL_REQUIRE(loaded.file_present);
    HL_REQUIRE(loaded.usable);
    HL_REQUIRE(!dictionary.empty());

    const auto missing = dictionary.load(temporary.path() / "missing.yaml");
    HL_REQUIRE(!missing.file_present);
    HL_REQUIRE(missing.usable);
    HL_REQUIRE(!missing.hasIssues());
    HL_REQUIRE(dictionary.empty());
}

HL_TEST(concepts_support_single_and_longest_multiword_matches) {
    hltest::TemporaryDirectory temporary;
    const auto file = temporary.path() / "concepts.yaml";
    hltest::writeText(file, R"yaml(concepts:
  folder:
    - folder
    - directory
    - dir
  rename:
    - rename
    - change name
    - give another name
)yaml");

    ConceptDictionary dictionary;
    const auto report = dictionary.load(file);
    HL_REQUIRE(report.usable);
    HL_REQUIRE_EQ(report.concepts_loaded, std::size_t{2});
    HL_REQUIRE(!report.hasIssues());
    HL_REQUIRE_EQ(dictionary.canonicalForPhrase("DIRECTORY").value(), "folder");
    HL_REQUIRE_EQ(dictionary.canonicalForPhrase("change the name").value(),
                  "rename");

    const std::vector<std::string> tokens = {
        "give", "another", "name", "directory"};
    const auto matches = dictionary.detect(tokens);
    HL_REQUIRE_EQ(matches.size(), std::size_t{2});
    HL_REQUIRE_EQ(matches[0].canonical, "rename");
    HL_REQUIRE_EQ(matches[0].token_offset, std::size_t{0});
    HL_REQUIRE_EQ(matches[0].token_length, std::size_t{3});
    HL_REQUIRE_EQ(matches[1].canonical, "folder");
    HL_REQUIRE_EQ(matches[1].token_offset, std::size_t{3});

    QueryProcessor processor(&dictionary);
    const auto context = processor.process("change the name of a directory");
    const std::vector<std::string> expected_concepts = {"rename", "folder"};
    HL_REQUIRE_EQ(context.concepts, expected_concepts);
}

HL_TEST(concepts_collision_makes_the_complete_dictionary_unusable) {
    hltest::TemporaryDirectory temporary;
    const auto file = temporary.path() / "concepts.yaml";
    hltest::writeText(file, R"yaml(concepts:
  first:
    - shared phrase
  second:
    - shared phrase
)yaml");

    ConceptDictionary dictionary;
    const auto report = dictionary.load(file);
    HL_REQUIRE(report.file_present);
    HL_REQUIRE(!report.usable);
    HL_REQUIRE(report.hasIssues());
    HL_REQUIRE_EQ(report.concepts_loaded, std::size_t{0});
    HL_REQUIRE(dictionary.empty());
    HL_REQUIRE(hltest::diagnosticsContain(report.diagnostics, "collision"));
}

HL_TEST(concepts_cycle_and_duplicates_are_reported) {
    hltest::TemporaryDirectory temporary;
    const auto cycle_file = temporary.path() / "cycle.yaml";
    hltest::writeText(cycle_file, R"yaml(concepts:
  alpha:
    - beta
  beta:
    - alpha
)yaml");

    ConceptDictionary dictionary;
    const auto cycle = dictionary.load(cycle_file);
    HL_REQUIRE(!cycle.usable);
    HL_REQUIRE(dictionary.empty());
    HL_REQUIRE(hltest::diagnosticsContain(cycle.diagnostics, "circular"));

    const auto duplicate_file = temporary.path() / "duplicate.yaml";
    hltest::writeText(duplicate_file, R"yaml(concepts:
  folder:
    - directory
    - directory
)yaml");
    const auto duplicate = dictionary.load(duplicate_file);
    HL_REQUIRE(!duplicate.usable);
    HL_REQUIRE(hltest::diagnosticsContain(duplicate.diagnostics, "duplicate"));
}

HL_TEST(concepts_validate_yaml_root_and_alias_types) {
    hltest::TemporaryDirectory temporary;
    ConceptDictionary dictionary;

    const auto root_file = temporary.path() / "root.yaml";
    hltest::writeText(root_file, "concepts: []\n");
    const auto root_report = dictionary.load(root_file);
    HL_REQUIRE(!root_report.usable);
    HL_REQUIRE(hltest::diagnosticsContain(root_report.diagnostics, "mapping"));

    const auto alias_file = temporary.path() / "alias.yaml";
    hltest::writeText(alias_file, "concepts:\n  folder: directory\n");
    const auto alias_report = dictionary.load(alias_file);
    HL_REQUIRE(!alias_report.usable);
    HL_REQUIRE(hltest::diagnosticsContain(alias_report.diagnostics, "sequence"));
}
