#include "query.hpp"

#include "concepts.hpp"

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace howlinux {
namespace {

const std::unordered_set<std::string>& stopwords() {
    // Search vocabulary belongs here rather than being spread across the
    // tokenizer, ranker and CLI.  Keep Linux commands out of this list.
    static const std::unordered_set<std::string> words = {
        "a",       "an",      "and",      "are",     "be",
        "can",     "could",   "define",   "describe", "did",
        "do",      "does",    "explain",  "for",      "from",
        "help",    "how",     "i",        "in",       "is",
        "it",      "me",      "mean",     "means",    "my",
        "of",      "on",      "or",       "please",   "should",
        "show",    "that",    "the",      "this",     "to",
        "was",     "were",    "what",     "why",      "with",
        "would",
    };
    return words;
}

[[nodiscard]] bool isAsciiDigit(unsigned char value) noexcept {
    return value >= static_cast<unsigned char>('0') &&
           value <= static_cast<unsigned char>('9');
}

[[nodiscard]] bool isAsciiLetter(unsigned char value) noexcept {
    return (value >= static_cast<unsigned char>('a') &&
            value <= static_cast<unsigned char>('z')) ||
           (value >= static_cast<unsigned char>('A') &&
            value <= static_cast<unsigned char>('Z'));
}

[[nodiscard]] bool isWordByte(unsigned char value) noexcept {
    // Bytes belonging to a UTF-8 sequence are deliberately copied verbatim.
    // Locale-dependent byte-wise case conversion would corrupt them.
    return isAsciiLetter(value) || isAsciiDigit(value) || value >= 0x80U;
}

[[nodiscard]] char asciiLower(char value) noexcept {
    const auto byte = static_cast<unsigned char>(value);
    if (byte >= static_cast<unsigned char>('A') &&
        byte <= static_cast<unsigned char>('Z')) {
        return static_cast<char>(byte +
                                 (static_cast<unsigned char>('a') -
                                  static_cast<unsigned char>('A')));
    }
    return value;
}

[[nodiscard]] bool isChunkByte(unsigned char value) noexcept {
    return isWordByte(value) || value == static_cast<unsigned char>('.') ||
           value == static_cast<unsigned char>('-') ||
           value == static_cast<unsigned char>('_');
}

[[nodiscard]] bool isAllDigits(std::string_view value) noexcept {
    return !value.empty() &&
           std::all_of(value.begin(), value.end(), [](char character) {
               return isAsciiDigit(static_cast<unsigned char>(character));
           });
}

[[nodiscard]] bool isFlagChunk(std::string_view chunk) noexcept {
    if (chunk.size() < 2 || chunk.front() != '-') {
        return false;
    }

    std::size_t prefix_length = 1;
    if (chunk.size() >= 2 && chunk[1] == '-') {
        prefix_length = 2;
    }
    if (chunk.size() <= prefix_length ||
        (prefix_length == 2 && chunk.size() >= 3 && chunk[2] == '-')) {
        return false;
    }

    return std::any_of(chunk.begin() + static_cast<std::ptrdiff_t>(prefix_length),
                       chunk.end(), [](char character) {
                           return isWordByte(
                               static_cast<unsigned char>(character));
                       });
}

void appendDotAware(std::string_view segment,
                    std::vector<std::string>& output) {
    std::string token;
    token.reserve(segment.size());

    const auto flush = [&]() {
        if (!token.empty()) {
            output.push_back(std::move(token));
            token.clear();
        }
    };

    for (std::size_t index = 0; index < segment.size(); ++index) {
        const auto value = static_cast<unsigned char>(segment[index]);
        if (isWordByte(value)) {
            token.push_back(segment[index]);
            continue;
        }

        if (value == static_cast<unsigned char>('.') && !token.empty() &&
            index + 1 < segment.size() &&
            isWordByte(static_cast<unsigned char>(segment[index + 1]))) {
            token.push_back('.');
            continue;
        }

        flush();
    }
    flush();
}

void appendChunk(std::string_view chunk, std::vector<std::string>& output) {
    if (chunk.empty()) {
        return;
    }

    // A leading one- or two-dash option is kept as one search token.  This
    // also preserves common long options containing additional dashes.
    if (isFlagChunk(chunk)) {
        std::string flag(chunk);
        while (!flag.empty() && flag.back() == '.') {
            flag.pop_back();
        }
        if (!flag.empty()) {
            output.push_back(std::move(flag));
        }
        return;
    }

    // Hyphens and underscores inside ordinary words are word boundaries.
    std::size_t begin = 0;
    for (std::size_t index = 0; index <= chunk.size(); ++index) {
        if (index != chunk.size() && chunk[index] != '-' &&
            chunk[index] != '_') {
            continue;
        }
        appendDotAware(chunk.substr(begin, index - begin), output);
        begin = index + 1;
    }
}

[[nodiscard]] std::vector<std::string> lexicalTokens(
    const std::string& input) {
    std::vector<std::string> output;
    std::size_t index = 0;

    while (index < input.size()) {
        if (!isChunkByte(static_cast<unsigned char>(input[index]))) {
            ++index;
            continue;
        }

        std::string chunk;
        while (index < input.size() &&
               isChunkByte(static_cast<unsigned char>(input[index]))) {
            chunk.push_back(asciiLower(input[index]));
            ++index;
        }

        // File-descriptor redirections such as 2> and 10>> are meaningful
        // shell tokens.  Other shell operators remain word boundaries.
        if (isAllDigits(chunk) && index < input.size() &&
            (input[index] == '>' || input[index] == '<')) {
            const char direction = input[index];
            do {
                chunk.push_back(input[index]);
                ++index;
            } while (index < input.size() && input[index] == direction);
            output.push_back(std::move(chunk));
            continue;
        }

        appendChunk(chunk, output);
    }

    return output;
}

[[nodiscard]] bool isStopword(const std::string& token) {
    return stopwords().find(token) != stopwords().end();
}

[[nodiscard]] bool beginsWith(const std::vector<std::string>& tokens,
                              std::initializer_list<std::string_view> prefix) {
    if (tokens.size() < prefix.size()) {
        return false;
    }

    return std::equal(prefix.begin(), prefix.end(), tokens.begin(),
                      [](std::string_view expected, const std::string& actual) {
                          return expected == actual;
                      });
}

[[nodiscard]] bool looksLikeShellSyntax(const std::string& token) noexcept {
    if (isFlagChunk(token)) {
        return true;
    }

    std::size_t index = 0;
    while (index < token.size() &&
           isAsciiDigit(static_cast<unsigned char>(token[index]))) {
        ++index;
    }
    return index > 0 && index < token.size() &&
           (token[index] == '>' || token[index] == '<');
}

[[nodiscard]] std::vector<std::string> buildPhrases(
    const std::vector<std::string>& sequence_tokens) {
    std::vector<std::string> phrases;
    if (sequence_tokens.size() < 2) {
        return phrases;
    }

    std::unordered_set<std::string> seen;
    constexpr std::size_t maximum_ngram_length = 5;

    // Preserve the complete ordered phrase even for unusually long queries.
    if (sequence_tokens.size() > maximum_ngram_length) {
        auto complete = joinTokens(sequence_tokens);
        seen.insert(complete);
        phrases.push_back(std::move(complete));
    }

    const std::size_t largest =
        std::min(sequence_tokens.size(), maximum_ngram_length);
    for (std::size_t length = largest; length >= 2; --length) {
        for (std::size_t offset = 0;
             offset + length <= sequence_tokens.size(); ++offset) {
            std::vector<std::string> part(
                sequence_tokens.begin() + static_cast<std::ptrdiff_t>(offset),
                sequence_tokens.begin() +
                    static_cast<std::ptrdiff_t>(offset + length));
            auto phrase = joinTokens(part);
            if (seen.insert(phrase).second) {
                phrases.push_back(std::move(phrase));
            }
        }
    }
    return phrases;
}

}  // namespace

std::vector<std::string> tokenize(const std::string& input,
                                  bool remove_stopwords,
                                  bool deduplicate) {
    auto lexical = lexicalTokens(input);
    std::vector<std::string> output;
    output.reserve(lexical.size());
    std::unordered_set<std::string> seen;

    for (auto& token : lexical) {
        if (remove_stopwords && isStopword(token)) {
            continue;
        }
        if (deduplicate && !seen.insert(token).second) {
            continue;
        }
        output.push_back(std::move(token));
    }
    return output;
}

std::string joinTokens(const std::vector<std::string>& tokens) {
    std::string output;
    std::size_t required = tokens.empty() ? 0 : tokens.size() - 1;
    for (const auto& token : tokens) {
        required += token.size();
    }
    output.reserve(required);

    for (const auto& token : tokens) {
        if (!output.empty()) {
            output.push_back(' ');
        }
        output += token;
    }
    return output;
}

std::string queryTypeName(QueryType type) {
    switch (type) {
        case QueryType::explain:
            return "explain";
        case QueryType::how_to:
            return "how_to";
        case QueryType::why:
            return "why";
        case QueryType::command:
            return "command";
        case QueryType::general:
            return "general";
    }
    return "general";
}

QueryProcessor::QueryProcessor(
    const ConceptDictionary* concepts,
    std::unordered_set<std::string> known_commands)
    : concepts_(concepts) {
    for (const auto& command : known_commands) {
        auto command_tokens = tokenize(command, false, false);
        if (!command_tokens.empty()) {
            known_commands_.insert(joinTokens(command_tokens));
        }
    }
}

QueryContext QueryProcessor::process(const std::string& raw_query) const {
    QueryContext context;
    context.original = raw_query;

    // Intent is detected from the pre-stopword sequence.  Otherwise the
    // defining words in "how can I" and "what does" would already be gone.
    const auto intent_tokens = tokenize(raw_query, false, false);
    context.type = detectType(raw_query, intent_tokens);

    context.sequence_tokens = tokenize(raw_query, true, false);
    context.tokens = tokenize(raw_query, true, true);
    context.normalized_query = joinTokens(context.tokens);
    context.phrases = buildPhrases(context.sequence_tokens);

    if (concepts_ != nullptr && !concepts_->empty()) {
        std::unordered_set<std::string> seen_concepts;
        for (const auto& match : concepts_->detect(context.sequence_tokens)) {
            if (seen_concepts.insert(match.canonical).second) {
                context.concepts.push_back(match.canonical);
            }
        }
    }

    return context;
}

QueryType QueryProcessor::detectType(
    const std::string& raw_query,
    const std::vector<std::string>& tokens) const {
    (void)raw_query;
    if (tokens.empty()) {
        return QueryType::general;
    }

    if (tokens.front() == "why") {
        return QueryType::why;
    }

    if (tokens.front() == "what" || tokens.front() == "explain" ||
        tokens.front() == "define" || tokens.front() == "describe" ||
        beginsWith(tokens, {"meaning", "of"})) {
        return QueryType::explain;
    }

    if (tokens.front() == "how" || beginsWith(tokens, {"show", "me"}) ||
        beginsWith(tokens, {"help", "me"})) {
        return QueryType::how_to;
    }

    std::vector<std::string> significant_tokens;
    significant_tokens.reserve(tokens.size());
    for (const auto& token : tokens) {
        if (!isStopword(token)) {
            significant_tokens.push_back(token);
        }
    }

    if (std::any_of(significant_tokens.begin(), significant_tokens.end(),
                    looksLikeShellSyntax)) {
        return QueryType::command;
    }

    const auto normalized = joinTokens(significant_tokens);
    for (const auto& command : known_commands_) {
        if (normalized == command ||
            (normalized.size() > command.size() &&
             normalized.compare(0, command.size(), command) == 0 &&
             normalized[command.size()] == ' ')) {
            return QueryType::command;
        }
    }

    static const std::unordered_set<std::string> action_verbs = {
        "add",     "change", "copy",   "create", "delete", "extract",
        "find",    "install", "list",  "make",   "move",   "remove",
        "rename",  "replace", "set",   "unpack", "update",
    };
    if (!significant_tokens.empty() &&
        action_verbs.contains(significant_tokens.front())) {
        return QueryType::how_to;
    }

    return QueryType::general;
}

}  // namespace howlinux
