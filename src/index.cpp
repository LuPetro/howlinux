#include "index.hpp"

#include "concepts.hpp"
#include "knowledge.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>
#include <utility>

namespace howlinux {
namespace {

void addTokens(std::unordered_set<std::string>& target,
               const std::vector<std::string>& values) {
    target.insert(values.begin(), values.end());
}

std::string normalizeIntent(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char byte) {
        if (byte >= 'A' && byte <= 'Z') {
            return static_cast<char>(byte - 'A' + 'a');
        }
        if (byte == '-') {
            return '_';
        }
        return static_cast<char>(byte);
    });
    return value;
}

void addConcepts(std::unordered_set<std::string>& target,
                 const ConceptDictionary& concepts,
                 const std::string& value) {
    const auto tokens = tokenize(value, true, false);
    for (const auto& match : concepts.detect(tokens)) {
        target.insert(match.canonical);
    }
}

bool isProtectedFromFuzzy(const std::string& token,
                          const std::unordered_set<std::string>& known_commands) {
    if (token.empty() || token.front() == '-' || known_commands.contains(token)) {
        return true;
    }
    return std::all_of(token.begin(), token.end(), [](unsigned char byte) {
        return (byte >= '0' && byte <= '9') || byte == '.' || byte == '>' ||
               byte == '<';
    });
}

}  // namespace

void InvertedIndex::build(const KnowledgeBase& knowledge,
                          const ConceptDictionary& concepts) {
    documents_.clear();
    postings_.clear();
    idf_.clear();
    known_commands_.clear();
    vocabulary_.clear();
    vocabulary_by_length_.clear();

    documents_.reserve(knowledge.entries().size());

    for (const auto& entry : knowledge.entries()) {
        EntryDocument document;

        addTokens(document.id_tokens, tokenize(entry.id));
        addTokens(document.title_tokens, tokenize(entry.title));
        addConcepts(document.concept_tokens, concepts, entry.id);
        addConcepts(document.concept_tokens, concepts, entry.title);

        for (const auto& alias : entry.aliases) {
            const auto alias_tokens = tokenize(alias);
            addTokens(document.alias_tokens, alias_tokens);
            const auto normalized = joinTokens(alias_tokens);
            if (!normalized.empty() &&
                std::find(document.aliases_normalized.begin(),
                          document.aliases_normalized.end(),
                          normalized) == document.aliases_normalized.end()) {
                document.aliases_normalized.push_back(normalized);
            }
            addConcepts(document.concept_tokens, concepts, alias);
        }

        for (const auto& keyword : entry.keywords) {
            addTokens(document.keyword_tokens, tokenize(keyword));
            addConcepts(document.concept_tokens, concepts, keyword);
        }

        if (!entry.command.empty()) {
            const auto command_tokens = tokenize(entry.command, false);
            addTokens(document.command_tokens, command_tokens);
            addConcepts(document.concept_tokens, concepts, entry.command);
            known_commands_.insert(command_tokens.begin(), command_tokens.end());
        }

        for (const auto& intent : entry.intents) {
            const auto normalized = normalizeIntent(intent);
            if (!normalized.empty()) {
                document.intents.insert(normalized);
            }
        }
        const auto normalized_type = normalizeIntent(entry.type);
        if (normalized_type == "howto" || normalized_type == "how_to") {
            document.intents.insert("how_to");
        } else if (normalized_type == "command") {
            document.intents.insert("command");
        } else if (normalized_type == "explain" || normalized_type == "explanation") {
            document.intents.insert("explain");
        }

        addTokens(document.all_tokens,
                  std::vector<std::string>(document.id_tokens.begin(),
                                           document.id_tokens.end()));
        document.all_tokens.insert(document.title_tokens.begin(), document.title_tokens.end());
        document.all_tokens.insert(document.alias_tokens.begin(), document.alias_tokens.end());
        document.all_tokens.insert(document.keyword_tokens.begin(), document.keyword_tokens.end());
        document.all_tokens.insert(document.command_tokens.begin(), document.command_tokens.end());
        document.all_tokens.insert(document.concept_tokens.begin(), document.concept_tokens.end());

        documents_.push_back(std::move(document));
    }

    for (std::size_t index = 0; index < documents_.size(); ++index) {
        for (const auto& token : documents_[index].all_tokens) {
            postings_[token].push_back(index);
        }
    }

    vocabulary_.reserve(postings_.size());
    const auto document_count = static_cast<double>(documents_.size());
    for (auto& [token, indices] : postings_) {
        std::sort(indices.begin(), indices.end());
        indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
        const auto frequency = static_cast<double>(indices.size());
        idf_[token] = std::log((document_count + 1.0) / (frequency + 1.0)) + 1.0;
        vocabulary_.push_back(token);
    }
    std::sort(vocabulary_.begin(), vocabulary_.end());
    for (const auto& token : vocabulary_) {
        vocabulary_by_length_[token.size()].push_back(token);
    }
}

CandidateSet InvertedIndex::candidates(const QueryContext& query,
                                       const SearchConfig& config) const {
    CandidateSet result;
    std::set<std::size_t> candidate_indices;
    std::unordered_set<std::string> direct_tokens;

    auto add_postings = [&](const std::string& token) {
        const auto found = postings_.find(token);
        if (found == postings_.end()) {
            return false;
        }
        candidate_indices.insert(found->second.begin(), found->second.end());
        direct_tokens.insert(token);
        return true;
    };

    for (const auto& token : query.tokens) {
        add_postings(token);
    }
    for (const auto& canonical_concept : query.concepts) {
        add_postings(canonical_concept);
    }

    for (const auto& query_token : query.tokens) {
        if (direct_tokens.contains(query_token) ||
            query_token.size() < config.fuzzy_minimum_length ||
            isProtectedFromFuzzy(query_token, known_commands_)) {
            continue;
        }

        const std::size_t maximum_distance = query_token.size() >= 7 ? 2 : 1;
        std::vector<std::pair<std::string, std::size_t>> fuzzy_tokens;
        const std::size_t minimum_length =
            query_token.size() > maximum_distance
                ? query_token.size() - maximum_distance
                : 0;
        const std::size_t maximum_length = query_token.size() + maximum_distance;
        for (std::size_t length = minimum_length; length <= maximum_length; ++length) {
            const auto bucket = vocabulary_by_length_.find(length);
            if (bucket == vocabulary_by_length_.end()) {
                continue;
            }
            for (const auto& indexed_token : bucket->second) {
                if (isProtectedFromFuzzy(indexed_token, known_commands_)) {
                    continue;
                }
                const auto distance = damerauLevenshteinDistance(
                    query_token, indexed_token, maximum_distance);
                if (distance <= maximum_distance) {
                    fuzzy_tokens.emplace_back(indexed_token, distance);
                }
            }
        }
        std::sort(fuzzy_tokens.begin(), fuzzy_tokens.end(),
                  [](const auto& left, const auto& right) {
                      if (left.second != right.second) {
                          return left.second < right.second;
                      }
                      return left.first < right.first;
                  });
        if (fuzzy_tokens.size() > config.fuzzy_match_limit_per_token) {
            fuzzy_tokens.resize(config.fuzzy_match_limit_per_token);
        }

        for (const auto& [indexed_token, distance] : fuzzy_tokens) {
            const auto posting = postings_.find(indexed_token);
            if (posting == postings_.end()) {
                continue;
            }
            for (const auto entry_index : posting->second) {
                candidate_indices.insert(entry_index);
                auto& matches = result.fuzzy_matches[entry_index];
                const auto duplicate = std::find_if(
                    matches.begin(), matches.end(), [&](const FuzzyTokenMatch& match) {
                        return match.query_token == query_token &&
                               match.indexed_token == indexed_token;
                    });
                if (duplicate == matches.end()) {
                    matches.push_back({query_token, indexed_token, distance});
                }
            }
        }
    }

    result.entry_indices.assign(candidate_indices.begin(), candidate_indices.end());
    return result;
}

double InvertedIndex::idf(const std::string& token) const {
    const auto found = idf_.find(token);
    return found == idf_.end() ? 1.0 : found->second;
}

const EntryDocument& InvertedIndex::document(std::size_t index) const {
    if (index >= documents_.size()) {
        throw std::out_of_range("entry document index is out of range");
    }
    return documents_[index];
}

std::size_t damerauLevenshteinDistance(const std::string& left,
                                       const std::string& right,
                                       std::size_t maximum_distance) {
    if (left == right) {
        return 0;
    }
    const auto length_difference = left.size() > right.size()
                                       ? left.size() - right.size()
                                       : right.size() - left.size();
    if (length_difference > maximum_distance) {
        return maximum_distance + 1;
    }

    thread_local std::vector<std::size_t> previous_previous;
    thread_local std::vector<std::size_t> previous;
    thread_local std::vector<std::size_t> current;
    previous_previous.resize(right.size() + 1);
    previous.resize(right.size() + 1);
    current.resize(right.size() + 1);
    for (std::size_t column = 0; column <= right.size(); ++column) {
        previous[column] = column;
        previous_previous[column] = column;
    }

    for (std::size_t row = 1; row <= left.size(); ++row) {
        current[0] = row;
        std::size_t row_minimum = current[0];
        for (std::size_t column = 1; column <= right.size(); ++column) {
            const std::size_t substitution_cost = left[row - 1] == right[column - 1] ? 0 : 1;
            current[column] = std::min({
                previous[column] + 1,
                current[column - 1] + 1,
                previous[column - 1] + substitution_cost,
            });

            if (row > 1 && column > 1 && left[row - 1] == right[column - 2] &&
                left[row - 2] == right[column - 1]) {
                current[column] = std::min(current[column],
                                           previous_previous[column - 2] + 1);
            }
            row_minimum = std::min(row_minimum, current[column]);
        }

        if (row_minimum > maximum_distance) {
            return maximum_distance + 1;
        }
        previous_previous.swap(previous);
        previous.swap(current);
    }
    return previous[right.size()];
}

}  // namespace howlinux
