#pragma once

#include <string>
#include <unordered_set>
#include <vector>

namespace howlinux {

class ConceptDictionary;

enum class QueryType {
    explain,
    how_to,
    why,
    command,
    general,
};

struct QueryContext {
    std::string original;
    std::vector<std::string> sequence_tokens;
    std::vector<std::string> tokens;
    std::string normalized_query;
    std::vector<std::string> phrases;
    std::vector<std::string> concepts;
    QueryType type{QueryType::general};
};

[[nodiscard]] std::vector<std::string> tokenize(
    const std::string& input,
    bool remove_stopwords = true,
    bool deduplicate = true);
[[nodiscard]] std::string joinTokens(const std::vector<std::string>& tokens);
[[nodiscard]] std::string queryTypeName(QueryType type);

class QueryProcessor {
public:
    QueryProcessor(const ConceptDictionary* concepts = nullptr,
                   std::unordered_set<std::string> known_commands = {});

    [[nodiscard]] QueryContext process(const std::string& raw_query) const;

private:
    [[nodiscard]] QueryType detectType(const std::string& raw_query,
                                       const std::vector<std::string>& tokens) const;

    const ConceptDictionary* concepts_;
    std::unordered_set<std::string> known_commands_;
};

}  // namespace howlinux
