#include "search.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>

namespace {

// Wörter die für die Suche keine Rolle spielen
const std::unordered_set<std::string> stopwords = {
    "a", "an", "the", "i", "do", "does", "can", "how",
    "please", "to", "of", "is", "for", "me", "my"
};

std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return out;
}

bool isPunct(char c) {
    return std::ispunct(static_cast<unsigned char>(c));
}

std::string joinTokens(const std::vector<std::string>& tokens) {
    std::string out;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) out += " ";
        out += tokens[i];
    }
    return out;
}

} // namespace

std::vector<std::string> normalize(const std::string& input) {
    // 1. Satzzeichen entfernen
    std::string cleaned;
    for (char c : input) {
        if (!isPunct(c)) {
            cleaned += c;
        }
    }

    // 2. lowercase
    cleaned = toLower(cleaned);

    // 3. in Wörter zerlegen + Stopwörter rausfiltern
    std::vector<std::string> tokens;
    std::istringstream iss(cleaned);
    std::string word;
    while (iss >> word) {
        if (stopwords.count(word) == 0) {
            tokens.push_back(word);
        }
    }
    return tokens;
}

namespace {

// Bewertet wie gut ein Eintrag zur Query passt.
// Die Punktewerte entsprechen "Ranking v1" aus den Notizen.
double scoreEntry(const std::vector<std::string>& queryTokens,
                   const std::string& queryJoined,
                   const KnowledgeEntry& entry) {
    double score = 0.0;

    // Exact alias match: irgendein Alias entspricht normalisiert genau der Query
    for (const auto& alias : entry.aliases) {
        if (joinTokens(normalize(alias)) == queryJoined) {
            score += 100.0;
        }
    }

    // Command match: Query enthält z.B. "mv" und entry.command == "mv"
    if (!entry.command.empty()) {
        for (const auto& token : queryTokens) {
            if (token == entry.command) {
                score += 25.0;
            }
        }
    }

    // Keyword match: jedes treffende Keyword gibt Punkte
    for (const auto& token : queryTokens) {
        for (const auto& keyword : entry.keywords) {
            if (token == keyword) {
                score += 20.0;
                break;
            }
        }
    }

    // Title match: Wörter aus dem Titel die auch in der Query vorkommen
    auto titleTokens = normalize(entry.title);
    for (const auto& token : queryTokens) {
        if (std::find(titleTokens.begin(), titleTokens.end(), token) != titleTokens.end()) {
            score += 10.0;
        }
    }

    return score;
}

} // namespace

std::vector<SearchResult> SearchEngine::search(const std::string& rawQuery) const {
    auto queryTokens = normalize(rawQuery);
    std::string queryJoined = joinTokens(queryTokens);

    std::vector<SearchResult> results;
    for (const auto& entry : kb_.entries()) {
        double score = scoreEntry(queryTokens, queryJoined, entry);
        if (score > 0.0) {
            results.push_back({&entry, score});
        }
    }

    std::sort(results.begin(), results.end(),
              [](const SearchResult& a, const SearchResult& b) {
                  return a.score > b.score;
              });

    return results;
}
