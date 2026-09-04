#pragma once

#include "diagnostics.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace howlinux {

class ConceptDictionary;

struct KnowledgeEntry {
    std::string id;
    std::string title;
    std::string type;
    std::string command;

    std::vector<std::string> aliases;
    std::vector<std::string> keywords;
    std::vector<std::string> related;
    std::vector<std::string> intents;
    std::string difficulty;
    std::vector<std::string> platforms;
    std::vector<std::string> tags;
    std::vector<std::string> examples;

    std::string category;
    std::filesystem::path source_directory;
    std::string content;
};

struct KnowledgeLoadReport {
    bool root_available{false};
    std::size_t discovered_entries{0};
    std::size_t loaded_entries{0};
    std::size_t skipped_entries{0};
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool hasIssues() const;
};

struct KnowledgeLintReport {
    bool performed{false};
    std::size_t entries_checked{0};
    std::size_t aliases_checked{0};
    std::size_t keywords_checked{0};
    std::size_t concepts_checked{0};
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool hasIssues() const;
};

class KnowledgeBase {
public:
    KnowledgeLoadReport load(const std::filesystem::path& directory);
    [[nodiscard]] KnowledgeLintReport lint(
        const ConceptDictionary& concepts) const;

    [[nodiscard]] const std::vector<KnowledgeEntry>& entries() const noexcept {
        return entries_;
    }
    [[nodiscard]] const KnowledgeEntry* findById(const std::string& id) const noexcept;
    [[nodiscard]] const std::filesystem::path& root() const noexcept { return root_; }
    [[nodiscard]] const KnowledgeLoadReport& report() const noexcept { return report_; }

private:
    std::optional<KnowledgeEntry> loadEntry(
        const std::filesystem::path& entry_directory,
        const std::string& category,
        std::vector<Diagnostic>& diagnostics) const;
    void rebuildLookup();

    std::filesystem::path root_;
    std::vector<KnowledgeEntry> entries_;
    std::unordered_map<std::string, std::size_t> id_lookup_;
    KnowledgeLoadReport report_;
};

}  // namespace howlinux
