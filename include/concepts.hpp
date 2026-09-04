#pragma once

#include "diagnostics.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace howlinux {

struct ConceptMatch {
    std::string canonical;
    std::string phrase;
    std::size_t token_offset{0};
    std::size_t token_length{0};
};

struct ConceptLoadReport {
    bool file_present{false};
    bool usable{true};
    std::size_t concepts_loaded{0};
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool hasIssues() const;
};

class ConceptDictionary {
public:
    ConceptLoadReport load(const std::filesystem::path& file);

    [[nodiscard]] std::vector<ConceptMatch> detect(
        const std::vector<std::string>& ordered_tokens) const;
    [[nodiscard]] std::optional<std::string> canonicalForPhrase(
        const std::string& normalized_phrase) const;
    [[nodiscard]] const ConceptLoadReport& report() const noexcept { return report_; }
    [[nodiscard]] bool empty() const noexcept { return phrase_to_canonical_.empty(); }
    [[nodiscard]] const std::unordered_map<std::string, std::string>&
    phraseMappings() const noexcept {
        return phrase_to_canonical_;
    }

private:
    struct PhraseDefinition {
        std::string phrase;
        std::string canonical;
        std::vector<std::string> tokens;
    };

    std::unordered_map<std::string, std::string> phrase_to_canonical_;
    std::vector<PhraseDefinition> definitions_;
    ConceptLoadReport report_;
};

}  // namespace howlinux
