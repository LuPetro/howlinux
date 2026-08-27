#include "search.hpp"

#include "concepts.hpp"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>

namespace howlinux {
namespace {

bool containsNormalizedPhrase(const std::string& text, const std::string& phrase) {
    if (text.empty() || phrase.empty()) {
        return false;
    }
    const std::string padded_text = " " + text + " ";
    const std::string padded_phrase = " " + phrase + " ";
    return padded_text.find(padded_phrase) != std::string::npos;
}

std::string number(double value) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(2) << value;
    return output.str();
}

void addReason(std::vector<std::string>& reasons,
               const std::string& label,
               double score) {
    if (score > 0.0) {
        reasons.push_back(label + " +" + number(score));
    }
}

double idfWeighted(double base_score, double idf) {
    // IDF remains a meaningful rarity signal without allowing one isolated
    // rare token to cross the confidence threshold by itself.
    return base_score * (0.5 + 0.5 * idf);
}

}  // namespace

double ScoreBreakdown::total() const noexcept {
    return exact_alias + phrase + command + keywords + concepts + intent + title +
           token_idf + fuzzy;
}

SearchEngine::SearchEngine(const KnowledgeBase& knowledge,
                           const ConceptDictionary& concepts,
                           SearchConfig config)
    : knowledge_(knowledge),
      concepts_(concepts),
      config_(config),
      index_(),
      query_processor_(&concepts) {
    index_.build(knowledge_, concepts_);
    query_processor_ = QueryProcessor(&concepts_, index_.knownCommands());
}

SearchResponse SearchEngine::search(const std::string& raw_query,
                                    std::size_t limit) const {
    SearchResponse response;
    response.query = query_processor_.process(raw_query);
    if (response.query.tokens.empty() && response.query.concepts.empty()) {
        return response;
    }

    const auto candidate_set = index_.candidates(response.query, config_);
    response.results.reserve(candidate_set.entry_indices.size());
    for (const auto entry_index : candidate_set.entry_indices) {
        auto result = score(entry_index, response.query, candidate_set);
        if (result.score >= config_.meaningful_score) {
            response.results.push_back(std::move(result));
        }
    }

    std::sort(response.results.begin(), response.results.end(),
              [](const SearchResult& left, const SearchResult& right) {
                  if (left.score != right.score) {
                      return left.score > right.score;
                  }
                  const auto left_exact = left.breakdown.exactAndPhrase();
                  const auto right_exact = right.breakdown.exactAndPhrase();
                  if (left_exact != right_exact) {
                      return left_exact > right_exact;
                  }
                  if (left.breakdown.intent != right.breakdown.intent) {
                      return left.breakdown.intent > right.breakdown.intent;
                  }
                  return left.entry->id < right.entry->id;
              });

    const std::size_t effective_limit = limit == 0 ? config_.default_limit : limit;
    if (response.results.size() > effective_limit) {
        response.results.resize(effective_limit);
    }
    return response;
}

SearchResult SearchEngine::score(std::size_t entry_index,
                                 const QueryContext& query,
                                 const CandidateSet& candidates) const {
    SearchResult result;
    result.entry = &knowledge_.entries().at(entry_index);
    const auto& document = index_.document(entry_index);

    if (std::find(document.aliases_normalized.begin(),
                  document.aliases_normalized.end(),
                  query.normalized_query) != document.aliases_normalized.end()) {
        result.breakdown.exact_alias = config_.exact_alias_score;
    } else {
        for (const auto& alias : document.aliases_normalized) {
            if (containsNormalizedPhrase(query.normalized_query, alias)) {
                result.breakdown.phrase = config_.phrase_score;
                break;
            }
        }
    }

    if (result.breakdown.exact_alias == 0.0 && result.breakdown.phrase == 0.0 &&
        query.concepts.size() >= 2 &&
        std::all_of(query.concepts.begin(), query.concepts.end(),
                    [&](const std::string& canonical_concept) {
                        return document.concept_tokens.contains(canonical_concept);
                    })) {
        // A synonym-expanded action/object pair such as "remove directory"
        // should behave like the canonical phrase "delete folder".
        result.breakdown.phrase = config_.phrase_score;
    }

    for (const auto& token : query.tokens) {
        if (document.command_tokens.contains(token)) {
            result.breakdown.command = config_.command_score;
            break;
        }
    }

    for (const auto& token : query.tokens) {
        if (document.keyword_tokens.contains(token)) {
            result.breakdown.keywords +=
                idfWeighted(config_.keyword_score, index_.idf(token));
        }
        if (document.title_tokens.contains(token)) {
            result.breakdown.title +=
                idfWeighted(config_.title_score, index_.idf(token));
        }
        if (document.all_tokens.contains(token)) {
            result.breakdown.token_idf +=
                idfWeighted(config_.token_score, index_.idf(token));
        }
    }

    for (const auto& canonical_concept : query.concepts) {
        if (document.concept_tokens.contains(canonical_concept)) {
            result.breakdown.concepts +=
                idfWeighted(config_.concept_score, index_.idf(canonical_concept));
        }
    }

    const auto fuzzy = candidates.fuzzy_matches.find(entry_index);
    if (fuzzy != candidates.fuzzy_matches.end()) {
        struct BestFuzzyMatch {
            double score{0.0};
            std::string indexed_token;
        };
        std::map<std::string, BestFuzzyMatch> best_matches;
        for (const auto& match : fuzzy->second) {
            double field_weight = 0.4;
            if (document.alias_tokens.contains(match.indexed_token)) {
                field_weight = 1.0;
            } else if (document.keyword_tokens.contains(match.indexed_token)) {
                field_weight = 0.8;
            } else if (document.concept_tokens.contains(match.indexed_token)) {
                field_weight = 0.7;
            } else if (document.title_tokens.contains(match.indexed_token)) {
                field_weight = 0.6;
            } else if (document.id_tokens.contains(match.indexed_token)) {
                field_weight = 0.5;
            }
            const double closeness = match.distance <= 1 ? 1.0 : 0.65;
            const double fuzzy_score = config_.fuzzy_score * closeness * field_weight;
            auto& best = best_matches[match.query_token];
            if (fuzzy_score > best.score ||
                (fuzzy_score == best.score && match.indexed_token < best.indexed_token)) {
                best = {fuzzy_score, match.indexed_token};
            }
        }
        for (const auto& [query_token, best] : best_matches) {
            result.breakdown.fuzzy += best.score;
            result.match_reasons.push_back("fuzzy " + query_token + " -> " +
                                           best.indexed_token);
        }
        result.fuzzy_used = result.breakdown.fuzzy > 0.0;
    }

    const double content_relevance = result.breakdown.exact_alias + result.breakdown.phrase +
                                     result.breakdown.command + result.breakdown.keywords +
                                     result.breakdown.concepts + result.breakdown.title +
                                     result.breakdown.token_idf + result.breakdown.fuzzy;
    const auto intent = queryTypeName(query.type);
    if (content_relevance >= config_.meaningful_score && document.intents.contains(intent)) {
        result.breakdown.intent = config_.intent_score;
    }

    result.score = result.breakdown.total();
    addReason(result.match_reasons, "exact alias", result.breakdown.exact_alias);
    addReason(result.match_reasons, "phrase", result.breakdown.phrase);
    addReason(result.match_reasons, "command", result.breakdown.command);
    addReason(result.match_reasons, "keywords", result.breakdown.keywords);
    addReason(result.match_reasons, "concepts", result.breakdown.concepts);
    addReason(result.match_reasons, "intent", result.breakdown.intent);
    addReason(result.match_reasons, "title", result.breakdown.title);
    addReason(result.match_reasons, "token/idf", result.breakdown.token_idf);
    addReason(result.match_reasons, "fuzzy", result.breakdown.fuzzy);
    return result;
}

PolicyDecision ResultPolicy::decide(const std::vector<SearchResult>& results) const {
    if (results.empty() || results.front().score < config_.meaningful_score) {
        return {ResultStatus::no_match, nullptr};
    }

    const bool sufficient_score = results.front().score >= config_.confident_score;
    const bool sufficient_margin = results.size() == 1 ||
                                   results.front().score - results[1].score >=
                                       config_.confident_margin;
    if (sufficient_score && sufficient_margin) {
        return {ResultStatus::confident, &results.front()};
    }
    return {ResultStatus::uncertain, nullptr};
}

std::string resultStatusName(ResultStatus status) {
    switch (status) {
        case ResultStatus::confident:
            return "confident";
        case ResultStatus::uncertain:
            return "uncertain";
        case ResultStatus::no_match:
            return "no_match";
    }
    return "no_match";
}

}  // namespace howlinux
