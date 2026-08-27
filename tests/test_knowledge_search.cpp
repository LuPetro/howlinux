#include "fixtures.hpp"
#include "test_harness.hpp"

#include "concepts.hpp"
#include "index.hpp"
#include "knowledge.hpp"
#include "query.hpp"
#include "search.hpp"

#include <cmath>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

using namespace howlinux;

HL_TEST(loader_reads_recursive_categories_optional_fields_and_byte_exact_content) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge with spaces";
    const std::string exact_content =
        "## Alpha\n\n```bash\nprintf 'Größe'\n```\n(no final newline)";
    hltest::writeRawEntry(root, "topics/alpha", R"yaml(id: alpha
title: Alpha title
type: howto
command: printf
aliases:
  - alpha alias
keywords:
  - alpha
related:
  - zeta
intent:
  - how_to
difficulty: beginner
platforms:
  - linux
tags:
  - shell
examples:
  - printf alpha
)yaml", exact_content);

    auto zeta = hltest::entrySpec("zeta", "Zeta title");
    zeta.aliases = {"zeta alias"};
    zeta.keywords = {"zeta"};
    zeta.content = "ZETA\n";
    hltest::writeEntry(root, "custom/admin/zeta", zeta);

    KnowledgeBase knowledge;
    const auto report = knowledge.load(root);
    HL_REQUIRE(report.root_available);
    HL_REQUIRE_EQ(report.discovered_entries, std::size_t{2});
    HL_REQUIRE_EQ(report.loaded_entries, std::size_t{2});
    HL_REQUIRE_EQ(report.skipped_entries, std::size_t{0});
    HL_REQUIRE(!report.hasIssues());
    HL_REQUIRE_EQ(knowledge.entries()[0].id, "alpha");
    HL_REQUIRE_EQ(knowledge.entries()[1].id, "zeta");

    const auto* alpha = knowledge.findById("alpha");
    HL_REQUIRE(alpha != nullptr);
    HL_REQUIRE_EQ(alpha->content, exact_content);
    HL_REQUIRE_EQ(alpha->difficulty, "beginner");
    HL_REQUIRE_EQ(alpha->platforms.front(), "linux");
    HL_REQUIRE_EQ(alpha->tags.front(), "shell");
    HL_REQUIRE_EQ(alpha->examples.front(), "printf alpha");

    const auto* loaded_zeta = knowledge.findById("zeta");
    HL_REQUIRE(loaded_zeta != nullptr);
    HL_REQUIRE_EQ(loaded_zeta->category, "custom");
}

HL_TEST(loader_skips_broken_entries_and_reports_specific_causes) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge";

    auto valid = hltest::entrySpec("valid", "Valid entry");
    valid.aliases = {"valid"};
    hltest::writeEntry(root, "topics/valid", valid);

    hltest::writeText(root / "topics/missing-meta/content.md", "content\n");
    hltest::writeText(root / "topics/missing-content/meta.yaml",
                      "id: missing-content\ntitle: Missing content\ntype: howto\n");
    hltest::writeRawEntry(root, "topics/bad-yaml", "id: [unterminated\n",
                          "content\n");
    hltest::writeRawEntry(root, "topics/bad-list", R"yaml(id: bad-list
title: Bad list
type: howto
aliases: not-a-list
)yaml", "content\n");

    KnowledgeBase knowledge;
    const auto report = knowledge.load(root);
    HL_REQUIRE(report.root_available);
    HL_REQUIRE_EQ(report.discovered_entries, std::size_t{5});
    HL_REQUIRE_EQ(report.loaded_entries, std::size_t{1});
    HL_REQUIRE_EQ(report.skipped_entries, std::size_t{4});
    HL_REQUIRE_EQ(knowledge.entries().front().id, "valid");
    HL_REQUIRE(hltest::diagnosticsContain(report.diagnostics, "missing required"));
    HL_REQUIRE(hltest::diagnosticsContain(report.diagnostics, "invalid YAML"));
    HL_REQUIRE(hltest::diagnosticsContain(report.diagnostics,
                                          "must be a list of strings"));

    for (const auto& diagnostic : report.diagnostics) {
        HL_REQUIRE(!diagnostic.path.empty());
        HL_REQUIRE(!diagnostic.entry_id.empty());
        HL_REQUIRE(!diagnostic.message.empty());
    }
}

HL_TEST(loader_duplicate_ids_are_deterministic_and_not_overwritten) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge";

    auto first = hltest::entrySpec("duplicate", "First duplicate");
    first.content = "FIRST-WINS\n";
    hltest::writeEntry(root, "a/first", first);

    auto second = hltest::entrySpec("duplicate", "Second duplicate");
    second.content = "SECOND-LOSES\n";
    hltest::writeEntry(root, "z/second", second);

    KnowledgeBase knowledge;
    const auto report = knowledge.load(root);
    HL_REQUIRE_EQ(report.discovered_entries, std::size_t{2});
    HL_REQUIRE_EQ(report.loaded_entries, std::size_t{1});
    HL_REQUIRE_EQ(report.skipped_entries, std::size_t{1});
    HL_REQUIRE_EQ(knowledge.entries().front().content, "FIRST-WINS\n");
    HL_REQUIRE(hltest::diagnosticsContain(report.diagnostics, "duplicate id"));
    HL_REQUIRE_CONTAINS(formatDiagnostic(report.diagnostics.back()), "duplicate");
}

HL_TEST(loader_reload_replaces_state_and_related_and_unknown_fields_warn) {
    hltest::TemporaryDirectory temporary;
    const auto first_root = temporary.path() / "first";
    auto first = hltest::entrySpec("first", "First");
    hltest::writeEntry(first_root, "topics/first", first);

    const auto second_root = temporary.path() / "second";
    hltest::writeRawEntry(second_root, "topics/second", R"yaml(id: second
title: Second
type: howto
related:
  - absent-entry
future_field: accepted-with-warning
)yaml", "SECOND\n");

    KnowledgeBase knowledge;
    HL_REQUIRE_EQ(knowledge.load(first_root).loaded_entries, std::size_t{1});
    const auto second_report = knowledge.load(second_root);
    HL_REQUIRE_EQ(second_report.loaded_entries, std::size_t{1});
    HL_REQUIRE_EQ(knowledge.entries().size(), std::size_t{1});
    HL_REQUIRE(knowledge.findById("first") == nullptr);
    HL_REQUIRE(knowledge.findById("second") != nullptr);
    HL_REQUIRE(hltest::diagnosticsContain(second_report.diagnostics,
                                          "unknown metadata field"));
    HL_REQUIRE(hltest::diagnosticsContain(second_report.diagnostics,
                                          "does not refer to a loaded entry"));
}

HL_TEST(loader_accepts_an_empty_directory_but_rejects_a_missing_root) {
    hltest::TemporaryDirectory temporary;
    const auto empty_root = temporary.path() / "empty";
    std::filesystem::create_directories(empty_root);

    KnowledgeBase knowledge;
    const auto empty = knowledge.load(empty_root);
    HL_REQUIRE(empty.root_available);
    HL_REQUIRE_EQ(empty.discovered_entries, std::size_t{0});
    HL_REQUIRE_EQ(empty.loaded_entries, std::size_t{0});
    HL_REQUIRE(!empty.hasIssues());
    HL_REQUIRE(knowledge.entries().empty());

    const auto missing = knowledge.load(temporary.path() / "missing");
    HL_REQUIRE(!missing.root_available);
    HL_REQUIRE(missing.hasIssues());
    HL_REQUIRE(hltest::diagnosticsContain(missing.diagnostics,
                                          "knowledge root"));
}

HL_TEST(index_computes_idf_and_damerau_transpositions) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge";
    for (int index = 1; index <= 3; ++index) {
        auto entry = hltest::entrySpec("entry-" + std::to_string(index),
                                       "Entry " + std::to_string(index));
        entry.command = index == 1 ? "samplecmd" : "";
        entry.aliases = {"common"};
        entry.keywords = {"common"};
        if (index == 1) {
            entry.keywords.push_back("rare");
        }
        hltest::writeEntry(root, "topics/entry-" + std::to_string(index), entry);
    }

    KnowledgeBase knowledge;
    HL_REQUIRE_EQ(knowledge.load(root).loaded_entries, std::size_t{3});
    ConceptDictionary concepts;
    HL_REQUIRE(concepts.load(root / "concepts.yaml").usable);

    InvertedIndex index;
    index.build(knowledge, concepts);
    HL_REQUIRE_EQ(index.size(), std::size_t{3});
    HL_REQUIRE(index.knownCommands().contains("samplecmd"));
    HL_REQUIRE_NEAR(index.idf("common"), 1.0, 0.000001);
    HL_REQUIRE_NEAR(index.idf("rare"), std::log(2.0) + 1.0, 0.000001);
    HL_REQUIRE_EQ(damerauLevenshteinDistance("rename", "rename", 1),
                  std::size_t{0});
    HL_REQUIRE_EQ(damerauLevenshteinDistance("renmae", "rename", 1),
                  std::size_t{1});
    HL_REQUIRE_EQ(damerauLevenshteinDistance("fodler", "folder", 1),
                  std::size_t{1});
    HL_REQUIRE(damerauLevenshteinDistance("alpha", "omega", 1) > 1);
}

HL_TEST(ranking_prioritizes_exact_phrase_concept_command_and_fuzzy_signals) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge";
    hltest::writeSearchKnowledge(root);

    KnowledgeBase knowledge;
    HL_REQUIRE_EQ(knowledge.load(root).loaded_entries, std::size_t{6});
    ConceptDictionary concepts;
    HL_REQUIRE(concepts.load(root / "concepts.yaml").usable);
    SearchEngine search(knowledge, concepts);

    const auto exact = search.search("rename folder");
    HL_REQUIRE(!exact.results.empty());
    HL_REQUIRE_EQ(exact.results.front().entry->id, "rename-folder");
    HL_REQUIRE(exact.results.front().breakdown.exact_alias > 0.0);

    const auto phrase = search.search("please rename folder safely");
    HL_REQUIRE(!phrase.results.empty());
    HL_REQUIRE_EQ(phrase.results.front().entry->id, "rename-folder");
    HL_REQUIRE(phrase.results.front().breakdown.phrase > 0.0);

    const auto concept_match = search.search("remove directory");
    HL_REQUIRE(!concept_match.results.empty());
    HL_REQUIRE_EQ(concept_match.results.front().entry->id, "delete-folder");
    HL_REQUIRE(concept_match.results.front().breakdown.concepts > 0.0);

    const auto command = search.search("mv");
    HL_REQUIRE(!command.results.empty());
    HL_REQUIRE_EQ(command.results.front().entry->id, "mv");
    HL_REQUIRE(command.results.front().breakdown.command > 0.0);

    const auto fuzzy = search.search("renmae fodler");
    HL_REQUIRE(!fuzzy.results.empty());
    HL_REQUIRE_EQ(fuzzy.results.front().entry->id, "rename-folder");
    HL_REQUIRE(fuzzy.results.front().fuzzy_used);
    HL_REQUIRE(fuzzy.results.front().breakdown.fuzzy > 0.0);
    HL_REQUIRE(exact.results.front().score > fuzzy.results.front().score);
}

HL_TEST(ranking_intent_ties_repetition_and_irrelevant_queries_are_stable) {
    hltest::TemporaryDirectory temporary;
    const auto root = temporary.path() / "knowledge";
    hltest::writeSearchKnowledge(root);

    KnowledgeBase knowledge;
    HL_REQUIRE(knowledge.load(root).root_available);
    ConceptDictionary concepts;
    HL_REQUIRE(concepts.load(root / "concepts.yaml").usable);
    SearchEngine search(knowledge, concepts);

    const auto explain = search.search("what does chmod 755 mean");
    HL_REQUIRE(!explain.results.empty());
    HL_REQUIRE_EQ(explain.results.front().entry->id, "chmod-755");
    HL_REQUIRE(explain.results.front().breakdown.intent > 0.0);

    const auto repeated =
        search.search("rename rename rename folder folder folder");
    const auto single = search.search("rename folder");
    HL_REQUIRE(!repeated.results.empty());
    HL_REQUIRE(!single.results.empty());
    HL_REQUIRE_EQ(repeated.results.front().entry->id,
                  single.results.front().entry->id);
    HL_REQUIRE_NEAR(repeated.results.front().score,
                    single.results.front().score, 0.000001);

    const auto content_without_intent = search.search("what mv");
    HL_REQUIRE(!content_without_intent.results.empty());
    HL_REQUIRE_EQ(content_without_intent.results.front().entry->id, "mv");

    HL_REQUIRE(search.search("quantum banana zephyr").results.empty());

    const auto tie_root = temporary.path() / "tie";
    hltest::writeTieKnowledge(tie_root);
    KnowledgeBase tied_knowledge;
    HL_REQUIRE_EQ(tied_knowledge.load(tie_root).loaded_entries, std::size_t{2});
    ConceptDictionary no_concepts;
    HL_REQUIRE(no_concepts.load(tie_root / "concepts.yaml").usable);
    SearchEngine tied_search(tied_knowledge, no_concepts);
    const auto tied = tied_search.search("sharedtoken");
    HL_REQUIRE_EQ(tied.results.size(), std::size_t{2});
    HL_REQUIRE_NEAR(tied.results[0].score, tied.results[1].score, 0.000001);
    HL_REQUIRE_EQ(tied.results[0].entry->id, "alpha");
    HL_REQUIRE_EQ(tied.results[1].entry->id, "beta");
}

HL_TEST(result_policy_requires_both_score_and_margin) {
    KnowledgeEntry alpha;
    alpha.id = "alpha";
    KnowledgeEntry beta;
    beta.id = "beta";
    ResultPolicy policy;

    HL_REQUIRE(policy.decide({}).status == ResultStatus::no_match);

    std::vector<SearchResult> low(1);
    low[0].entry = &alpha;
    low[0].score = 7.0;
    HL_REQUIRE(policy.decide(low).status == ResultStatus::no_match);

    std::vector<SearchResult> one(1);
    one[0].entry = &alpha;
    one[0].score = 70.0;
    const auto confident_single = policy.decide(one);
    HL_REQUIRE(confident_single.status == ResultStatus::confident);
    HL_REQUIRE(confident_single.selected == &one.front());

    std::vector<SearchResult> close(2);
    close[0].entry = &alpha;
    close[0].score = 80.0;
    close[1].entry = &beta;
    close[1].score = 70.0;
    HL_REQUIRE(policy.decide(close).status == ResultStatus::uncertain);

    close[1].score = 65.0;
    HL_REQUIRE(policy.decide(close).status == ResultStatus::confident);
}
