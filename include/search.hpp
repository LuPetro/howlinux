#pragma once

#include "config.hpp"
#include "index.hpp"
#include "knowledge.hpp"
#include "query.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace howlinux {

class ConceptDictionary;

struct ScoreBreakdown {
    double exact_alias{0.0};
    double phrase{0.0};
    double command{0.0};
    double keywords{0.0};
    double concepts{0.0};
    double intent{0.0};
    double title{0.0};
    double token_idf{0.0};
    double fuzzy{0.0};

    [[nodiscard]] double total() const noexcept;
    [[nodiscard]] double exactAndPhrase() const noexcept {
        return exact_alias + phrase;
    }
};

struct SearchResult {
    const KnowledgeEntry* entry{nullptr};
    double score{0.0};
    ScoreBreakdown breakdown;
    bool fuzzy_used{false};
    std::vector<std::string> match_reasons;
};

struct SearchResponse {
    QueryContext query;
    std::vector<SearchResult> results;
};

class SearchEngine {
public:
    SearchEngine(const KnowledgeBase& knowledge,
                 const ConceptDictionary& concepts,
                 SearchConfig config = {});

    [[nodiscard]] SearchResponse search(const std::string& raw_query,
                                        std::size_t limit = 0) const;
    [[nodiscard]] const InvertedIndex& index() const noexcept { return index_; }

private:
    [[nodiscard]] SearchResult score(std::size_t entry_index,
                                     const QueryContext& query,
                                     const CandidateSet& candidates) const;

    const KnowledgeBase& knowledge_;
    const ConceptDictionary& concepts_;
    SearchConfig config_;
    InvertedIndex index_;
    QueryProcessor query_processor_;
};

enum class ResultStatus {
    confident,
    uncertain,
    no_match,
};

struct PolicyDecision {
    ResultStatus status{ResultStatus::no_match};
    const SearchResult* selected{nullptr};
};

class ResultPolicy {
public:
    explicit ResultPolicy(SearchConfig config = {}) : config_(config) {}
    [[nodiscard]] PolicyDecision decide(const std::vector<SearchResult>& results) const;

private:
    SearchConfig config_;
};

[[nodiscard]] std::string resultStatusName(ResultStatus status);

}  // namespace howlinux
