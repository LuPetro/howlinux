#pragma once

#include "config.hpp"
#include "query.hpp"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace howlinux {

class ConceptDictionary;
class KnowledgeBase;

struct EntryDocument {
    std::unordered_set<std::string> id_tokens;
    std::unordered_set<std::string> title_tokens;
    std::unordered_set<std::string> alias_tokens;
    std::vector<std::string> aliases_normalized;
    std::unordered_set<std::string> keyword_tokens;
    std::unordered_set<std::string> command_tokens;
    std::unordered_set<std::string> concept_tokens;
    std::unordered_set<std::string> all_tokens;
    std::unordered_set<std::string> intents;
};

struct FuzzyTokenMatch {
    std::string query_token;
    std::string indexed_token;
    std::size_t distance{0};
};

struct CandidateSet {
    std::vector<std::size_t> entry_indices;
    std::unordered_map<std::size_t, std::vector<FuzzyTokenMatch>> fuzzy_matches;
};

class InvertedIndex {
public:
    void build(const KnowledgeBase& knowledge, const ConceptDictionary& concepts);

    [[nodiscard]] CandidateSet candidates(const QueryContext& query,
                                          const SearchConfig& config) const;
    [[nodiscard]] double idf(const std::string& token) const;
    [[nodiscard]] const EntryDocument& document(std::size_t index) const;
    [[nodiscard]] const std::unordered_set<std::string>& knownCommands() const noexcept {
        return known_commands_;
    }
    [[nodiscard]] std::size_t size() const noexcept { return documents_.size(); }

private:
    std::vector<EntryDocument> documents_;
    std::unordered_map<std::string, std::vector<std::size_t>> postings_;
    std::unordered_map<std::string, double> idf_;
    std::unordered_set<std::string> known_commands_;
    std::vector<std::string> vocabulary_;
    std::unordered_map<std::size_t, std::vector<std::string>> vocabulary_by_length_;
};

[[nodiscard]] std::size_t damerauLevenshteinDistance(
    const std::string& left,
    const std::string& right,
    std::size_t maximum_distance);

}  // namespace howlinux
