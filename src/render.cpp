#include "render.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <locale>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace howlinux {
namespace {

constexpr char kHexDigits[] = "0123456789ABCDEF";

std::string humanSafe(const std::string& value) {
    std::string output;
    output.reserve(value.size());

    for (const auto character : value) {
        const auto byte = static_cast<unsigned char>(character);
        switch (byte) {
            case '\b':
                output += "\\b";
                break;
            case '\f':
                output += "\\f";
                break;
            case '\n':
                output += "\\n";
                break;
            case '\r':
                output += "\\r";
                break;
            case '\t':
                output += "\\t";
                break;
            default:
                if (byte < 0x20U || byte == 0x7FU) {
                    output += "\\x";
                    output += kHexDigits[(byte >> 4U) & 0x0FU];
                    output += kHexDigits[byte & 0x0FU];
                } else {
                    output.push_back(character);
                }
                break;
        }
    }
    return output;
}

bool isContinuationByte(unsigned char byte) {
    return byte >= 0x80U && byte <= 0xBFU;
}

bool decodeUtf8(const std::string& value,
                std::size_t offset,
                std::size_t& length,
                std::uint32_t& code_point) {
    const auto remaining = value.size() - offset;
    const auto first = static_cast<unsigned char>(value[offset]);

    if (first <= 0x7FU) {
        length = 1;
        code_point = first;
        return true;
    }

    if (first >= 0xC2U && first <= 0xDFU && remaining >= 2) {
        const auto second = static_cast<unsigned char>(value[offset + 1]);
        if (!isContinuationByte(second)) {
            return false;
        }
        length = 2;
        code_point = (static_cast<std::uint32_t>(first & 0x1FU) << 6U) |
                     static_cast<std::uint32_t>(second & 0x3FU);
        return true;
    }

    if (first >= 0xE0U && first <= 0xEFU && remaining >= 3) {
        const auto second = static_cast<unsigned char>(value[offset + 1]);
        const auto third = static_cast<unsigned char>(value[offset + 2]);
        const bool valid_second =
            (first == 0xE0U && second >= 0xA0U && second <= 0xBFU) ||
            (first == 0xEDU && second >= 0x80U && second <= 0x9FU) ||
            ((first >= 0xE1U && first <= 0xECU) && isContinuationByte(second)) ||
            ((first >= 0xEEU && first <= 0xEFU) && isContinuationByte(second));
        if (!valid_second || !isContinuationByte(third)) {
            return false;
        }
        length = 3;
        code_point = (static_cast<std::uint32_t>(first & 0x0FU) << 12U) |
                     (static_cast<std::uint32_t>(second & 0x3FU) << 6U) |
                     static_cast<std::uint32_t>(third & 0x3FU);
        return true;
    }

    if (first >= 0xF0U && first <= 0xF4U && remaining >= 4) {
        const auto second = static_cast<unsigned char>(value[offset + 1]);
        const auto third = static_cast<unsigned char>(value[offset + 2]);
        const auto fourth = static_cast<unsigned char>(value[offset + 3]);
        const bool valid_second =
            (first == 0xF0U && second >= 0x90U && second <= 0xBFU) ||
            (first == 0xF4U && second >= 0x80U && second <= 0x8FU) ||
            ((first >= 0xF1U && first <= 0xF3U) && isContinuationByte(second));
        if (!valid_second || !isContinuationByte(third) ||
            !isContinuationByte(fourth)) {
            return false;
        }
        length = 4;
        code_point = (static_cast<std::uint32_t>(first & 0x07U) << 18U) |
                     (static_cast<std::uint32_t>(second & 0x3FU) << 12U) |
                     (static_cast<std::uint32_t>(third & 0x3FU) << 6U) |
                     static_cast<std::uint32_t>(fourth & 0x3FU);
        return true;
    }

    return false;
}

void appendUnicodeEscape(std::string& output, std::uint32_t code_point) {
    output += "\\u";
    output += kHexDigits[(code_point >> 12U) & 0x0FU];
    output += kHexDigits[(code_point >> 8U) & 0x0FU];
    output += kHexDigits[(code_point >> 4U) & 0x0FU];
    output += kHexDigits[code_point & 0x0FU];
}

void writeJsonString(std::ostream& output, const std::string& value) {
    output << '"' << escapeJson(value) << '"';
}

void writeJsonStringArray(std::ostream& output,
                          const std::vector<std::string>& values) {
    output << '[';
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        writeJsonString(output, values[index]);
    }
    output << ']';
}

std::string humanNumber(double value) {
    if (!std::isfinite(value)) {
        return "n/a";
    }
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::fixed << std::setprecision(2) << value;
    return output.str();
}

void writeJsonNumber(std::ostream& output, double value) {
    if (!std::isfinite(value)) {
        output << "null";
        return;
    }
    if (value == 0.0) {
        output << '0';
        return;
    }
    std::ostringstream formatted;
    formatted.imbue(std::locale::classic());
    formatted << std::setprecision(15) << value;
    output << formatted.str();
}

double breakdownTotal(const ScoreBreakdown& breakdown) {
    return breakdown.exact_alias + breakdown.phrase + breakdown.command +
           breakdown.keywords + breakdown.concepts + breakdown.intent +
           breakdown.title + breakdown.token_idf + breakdown.fuzzy;
}

void writeJsonBreakdown(std::ostream& output, const ScoreBreakdown& breakdown) {
    output << '{';
    output << "\"exact_alias\":";
    writeJsonNumber(output, breakdown.exact_alias);
    output << ",\"phrase\":";
    writeJsonNumber(output, breakdown.phrase);
    output << ",\"command\":";
    writeJsonNumber(output, breakdown.command);
    output << ",\"keywords\":";
    writeJsonNumber(output, breakdown.keywords);
    output << ",\"concepts\":";
    writeJsonNumber(output, breakdown.concepts);
    output << ",\"intent\":";
    writeJsonNumber(output, breakdown.intent);
    output << ",\"title\":";
    writeJsonNumber(output, breakdown.title);
    output << ",\"token_idf\":";
    writeJsonNumber(output, breakdown.token_idf);
    output << ",\"fuzzy\":";
    writeJsonNumber(output, breakdown.fuzzy);
    output << ",\"total\":";
    writeJsonNumber(output, breakdownTotal(breakdown));
    output << '}';
}

void writeJsonEntryValue(std::ostream& output, const KnowledgeEntry& entry) {
    output << '{';
    output << "\"id\":";
    writeJsonString(output, entry.id);
    output << ",\"title\":";
    writeJsonString(output, entry.title);
    output << ",\"type\":";
    writeJsonString(output, entry.type);
    output << ",\"command\":";
    writeJsonString(output, entry.command);
    output << ",\"aliases\":";
    writeJsonStringArray(output, entry.aliases);
    output << ",\"keywords\":";
    writeJsonStringArray(output, entry.keywords);
    output << ",\"related\":";
    writeJsonStringArray(output, entry.related);
    output << ",\"intent\":";
    writeJsonStringArray(output, entry.intents);
    output << ",\"difficulty\":";
    writeJsonString(output, entry.difficulty);
    output << ",\"platforms\":";
    writeJsonStringArray(output, entry.platforms);
    output << ",\"tags\":";
    writeJsonStringArray(output, entry.tags);
    output << ",\"examples\":";
    writeJsonStringArray(output, entry.examples);
    output << ",\"category\":";
    writeJsonString(output, entry.category);
    output << ",\"content\":";
    writeJsonString(output, entry.content);
    output << '}';
}

void writeJsonEntrySummary(std::ostream& output, const KnowledgeEntry& entry) {
    output << '{';
    output << "\"id\":";
    writeJsonString(output, entry.id);
    output << ",\"title\":";
    writeJsonString(output, entry.title);
    output << ",\"type\":";
    writeJsonString(output, entry.type);
    output << ",\"command\":";
    writeJsonString(output, entry.command);
    output << ",\"category\":";
    writeJsonString(output, entry.category);
    output << '}';
}

void writeJsonSearchResult(std::ostream& output,
                           const SearchResult& result,
                           bool explain) {
    output << '{';
    output << "\"id\":";
    writeJsonString(output, result.entry->id);
    output << ",\"title\":";
    writeJsonString(output, result.entry->title);
    output << ",\"score\":";
    writeJsonNumber(output, result.score);
    output << ",\"fuzzy_used\":" << (result.fuzzy_used ? "true" : "false");
    output << ",\"match_reasons\":";
    writeJsonStringArray(output, result.match_reasons);
    if (explain) {
        output << ",\"breakdown\":";
        writeJsonBreakdown(output, result.breakdown);
    }
    output << '}';
}

std::vector<const SearchResult*> limitedResults(const SearchResponse& response,
                                                std::size_t limit) {
    const std::size_t maximum =
        limit == 0 ? response.results.size() : std::min(limit, response.results.size());
    std::vector<const SearchResult*> results;
    results.reserve(maximum);
    for (const auto& result : response.results) {
        if (result.entry == nullptr) {
            continue;
        }
        results.push_back(&result);
        if (results.size() == maximum) {
            break;
        }
    }
    return results;
}

std::vector<const KnowledgeEntry*> sortedEntries(const KnowledgeBase& knowledge) {
    std::vector<const KnowledgeEntry*> entries;
    entries.reserve(knowledge.entries().size());
    for (const auto& entry : knowledge.entries()) {
        entries.push_back(&entry);
    }
    std::sort(entries.begin(), entries.end(),
              [](const KnowledgeEntry* left, const KnowledgeEntry* right) {
                  if (left->id != right->id) {
                      return left->id < right->id;
                  }
                  if (left->title != right->title) {
                      return left->title < right->title;
                  }
                  if (left->category != right->category) {
                      return left->category < right->category;
                  }
                  return left->source_directory.generic_string() <
                         right->source_directory.generic_string();
              });
    return entries;
}

std::string relatedLabel(const std::string& id, const KnowledgeBase& knowledge) {
    const auto* related = knowledge.findById(id);
    if (related == nullptr || related->title.empty()) {
        return humanSafe(id);
    }
    return humanSafe(related->title) + " (" + humanSafe(related->id) + ")";
}

std::string joinedHuman(const std::vector<std::string>& values,
                        const std::string& empty_label) {
    if (values.empty()) {
        return empty_label;
    }
    std::ostringstream output;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0) {
            output << ", ";
        }
        output << humanSafe(values[index]);
    }
    return output.str();
}

void writeHumanBreakdown(std::ostream& output,
                         const ScoreBreakdown& breakdown) {
    output << "    exact_alias: " << humanNumber(breakdown.exact_alias) << '\n';
    output << "    phrase: " << humanNumber(breakdown.phrase) << '\n';
    output << "    command: " << humanNumber(breakdown.command) << '\n';
    output << "    keywords: " << humanNumber(breakdown.keywords) << '\n';
    output << "    concepts: " << humanNumber(breakdown.concepts) << '\n';
    output << "    intent: " << humanNumber(breakdown.intent) << '\n';
    output << "    title: " << humanNumber(breakdown.title) << '\n';
    output << "    token_idf: " << humanNumber(breakdown.token_idf) << '\n';
    output << "    fuzzy: " << humanNumber(breakdown.fuzzy) << '\n';
    output << "    total: " << humanNumber(breakdownTotal(breakdown)) << '\n';
}

const char* statusName(ResultStatus status) {
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

const char* severityName(DiagnosticSeverity severity) {
    switch (severity) {
        case DiagnosticSeverity::warning:
            return "warning";
        case DiagnosticSeverity::error:
            return "error";
    }
    return "warning";
}

bool hasValidationIssues(const KnowledgeLoadReport& knowledge_report,
                         const ConceptLoadReport& concept_report,
                         const KnowledgeLintReport& lint_report) {
    return !knowledge_report.root_available ||
           knowledge_report.skipped_entries != 0 ||
           !knowledge_report.diagnostics.empty() || !concept_report.usable ||
           !concept_report.diagnostics.empty() || lint_report.hasIssues();
}

void writeJsonDiagnostic(std::ostream& output,
                         const char* scope,
                         const Diagnostic& diagnostic) {
    output << '{';
    output << "\"scope\":";
    writeJsonString(output, scope);
    output << ",\"severity\":";
    writeJsonString(output, severityName(diagnostic.severity));
    output << ",\"path\":";
    writeJsonString(output, diagnostic.path.generic_string());
    output << ",\"entry_id\":";
    writeJsonString(output, diagnostic.entry_id);
    output << ",\"message\":";
    writeJsonString(output, diagnostic.message);
    output << '}';
}

void writeHumanDiagnostic(std::ostream& output,
                          const char* scope,
                          const Diagnostic& diagnostic) {
    output << "  [" << severityName(diagnostic.severity) << "] "
           << scope;
    if (!diagnostic.path.empty()) {
        output << " " << humanSafe(diagnostic.path.generic_string());
    }
    if (!diagnostic.entry_id.empty()) {
        output << " (ID: " << humanSafe(diagnostic.entry_id) << ")";
    }
    output << ": " << humanSafe(diagnostic.message) << '\n';
}

}  // namespace

std::string escapeJson(const std::string& value) {
    std::string output;
    output.reserve(value.size());

    for (std::size_t offset = 0; offset < value.size();) {
        const auto byte = static_cast<unsigned char>(value[offset]);
        if (byte <= 0x7FU) {
            switch (byte) {
                case '"':
                    output += "\\\"";
                    break;
                case '\\':
                    output += "\\\\";
                    break;
                case '\b':
                    output += "\\b";
                    break;
                case '\f':
                    output += "\\f";
                    break;
                case '\n':
                    output += "\\n";
                    break;
                case '\r':
                    output += "\\r";
                    break;
                case '\t':
                    output += "\\t";
                    break;
                default:
                    if (byte < 0x20U || byte == 0x7FU) {
                        appendUnicodeEscape(output, byte);
                    } else {
                        output.push_back(value[offset]);
                    }
                    break;
            }
            ++offset;
            continue;
        }

        std::size_t length = 0;
        std::uint32_t code_point = 0;
        if (!decodeUtf8(value, offset, length, code_point)) {
            output += "\\uFFFD";
            ++offset;
            continue;
        }

        if ((code_point >= 0x80U && code_point <= 0x9FU) ||
            code_point == 0x2028U || code_point == 0x2029U) {
            appendUnicodeEscape(output, code_point);
        } else {
            output.append(value, offset, length);
        }
        offset += length;
    }
    return output;
}

void Renderer::entry(std::ostream& output,
                     const KnowledgeEntry& value,
                     const KnowledgeBase& knowledge) {
    output << humanSafe(value.title) << '\n';
    output << "ID: " << humanSafe(value.id) << "\n\n";

    if (!value.content.empty()) {
        output.write(value.content.data(),
                     static_cast<std::streamsize>(value.content.size()));
    }
    if (value.content.empty() || value.content.back() != '\n') {
        output << '\n';
    }

    if (!value.related.empty()) {
        output << "\nRelated:\n";
        for (const auto& related_id : value.related) {
            output << "  - " << relatedLabel(related_id, knowledge) << '\n';
        }
    }
}

void Renderer::suggestions(std::ostream& output,
                           const SearchResponse& response,
                           std::size_t limit,
                           bool explain_requested) {
    const auto results = limitedResults(response, limit);
    if (results.empty()) {
        noMatch(output, response.query.original);
        if (explain_requested) {
            output << '\n';
            explain(output, response, limit);
        }
        return;
    }

    output << "No confident local match for: "
           << humanSafe(response.query.original) << "\n\n";
    output << "Suggestions:\n";
    for (std::size_t index = 0; index < results.size(); ++index) {
        const auto& result = *results[index];
        output << "  " << index + 1 << ". " << humanSafe(result.entry->title)
               << '\n';
        output << "     ID: " << humanSafe(result.entry->id) << '\n';
        output << "     Score: " << humanNumber(result.score) << '\n';
        output << "     Match: "
               << joinedHuman(result.match_reasons, "candidate match");
        if (result.fuzzy_used) {
            output << " (fuzzy used)";
        }
        output << '\n';
    }

    if (explain_requested) {
        output << '\n';
        explain(output, response, limit);
    }
}

void Renderer::explain(std::ostream& output,
                       const SearchResponse& response,
                       std::size_t limit) {
    const auto results = limitedResults(response, limit);
    output << "Ranking explanation\n";
    output << "  Query: " << humanSafe(response.query.original) << '\n';
    output << "  Query type: " << queryTypeName(response.query.type) << '\n';
    output << "  Normalized: " << humanSafe(response.query.normalized_query) << '\n';
    output << "  Concepts: "
           << joinedHuman(response.query.concepts, "none") << '\n';
    output << "  Tokens: " << joinedHuman(response.query.tokens, "none") << '\n';

    if (results.empty()) {
        output << "  Results: none\n";
        return;
    }

    output << "  Results:\n";
    for (std::size_t index = 0; index < results.size(); ++index) {
        const auto& result = *results[index];
        output << "  " << index + 1 << ". " << humanSafe(result.entry->title)
               << " (" << humanSafe(result.entry->id) << ")\n";
        output << "    score: " << humanNumber(result.score) << '\n';
        output << "    match_reasons: "
               << joinedHuman(result.match_reasons, "none") << '\n';
        output << "    fuzzy_used: " << (result.fuzzy_used ? "yes" : "no")
               << '\n';
        output << "    breakdown:\n";
        writeHumanBreakdown(output, result.breakdown);
    }
}

void Renderer::noMatch(std::ostream& output, const std::string& query) {
    output << "No local knowledge match found for: " << humanSafe(query) << '\n';
    output << "Example: howlinux \"rename a folder\"\n";
}

void Renderer::list(std::ostream& output, const KnowledgeBase& knowledge) {
    const auto entries = sortedEntries(knowledge);
    output << "Knowledge entries: " << entries.size() << '\n';
    if (entries.empty()) {
        output << "No knowledge entries are loaded.\n";
        return;
    }

    for (const auto* entry_value : entries) {
        output << "  - " << humanSafe(entry_value->title)
               << " (ID: " << humanSafe(entry_value->id) << ")";
        if (!entry_value->category.empty()) {
            output << " [" << humanSafe(entry_value->category) << "]";
        }
        output << '\n';
    }
}

void Renderer::validation(std::ostream& output,
                          const KnowledgeLoadReport& knowledge_report,
                          const ConceptLoadReport& concept_report,
                          const KnowledgeLintReport& lint_report) {
    const bool issues =
        hasValidationIssues(knowledge_report, concept_report, lint_report);
    const char* validation_status =
        !knowledge_report.root_available ? "error" : (issues ? "invalid" : "valid");

    output << "Validation status: " << validation_status << '\n';
    output << "Knowledge:\n";
    output << "  Root available: "
           << (knowledge_report.root_available ? "yes" : "no") << '\n';
    output << "  Discovered entries: " << knowledge_report.discovered_entries
           << '\n';
    output << "  Loaded entries: " << knowledge_report.loaded_entries << '\n';
    output << "  Skipped entries: " << knowledge_report.skipped_entries << '\n';
    output << "Concepts:\n";
    output << "  File present: " << (concept_report.file_present ? "yes" : "no")
           << '\n';
    output << "  Usable: " << (concept_report.usable ? "yes" : "no") << '\n';
    output << "  Loaded concepts: " << concept_report.concepts_loaded << '\n';
    output << "Lint:\n";
    output << "  Performed: " << (lint_report.performed ? "yes" : "no") << '\n';
    output << "  Entries checked: " << lint_report.entries_checked << '\n';
    output << "  Aliases checked: " << lint_report.aliases_checked << '\n';
    output << "  Keywords checked: " << lint_report.keywords_checked << '\n';
    output << "  Concepts checked: " << lint_report.concepts_checked << '\n';

    if (knowledge_report.root_available &&
        knowledge_report.loaded_entries == 0) {
        output << "Note: the Knowledge directory contains no valid entries.\n";
    }

    const std::size_t diagnostic_count =
        knowledge_report.diagnostics.size() + concept_report.diagnostics.size() +
        lint_report.diagnostics.size();
    output << "Diagnostics: " << diagnostic_count << '\n';
    for (const auto& diagnostic : knowledge_report.diagnostics) {
        writeHumanDiagnostic(output, "knowledge", diagnostic);
    }
    for (const auto& diagnostic : concept_report.diagnostics) {
        writeHumanDiagnostic(output, "concepts", diagnostic);
    }
    for (const auto& diagnostic : lint_report.diagnostics) {
        writeHumanDiagnostic(output, "lint", diagnostic);
    }
}

void Renderer::searchJson(std::ostream& output,
                          const SearchResponse& response,
                          const PolicyDecision& decision,
                          std::size_t limit,
                          bool explain_requested) {
    const auto results = limitedResults(response, limit);
    output << '{';
    output << "\"status\":";
    writeJsonString(output, statusName(decision.status));
    output << ",\"query\":";
    writeJsonString(output, response.query.original);
    output << ",\"query_type\":";
    writeJsonString(output, queryTypeName(response.query.type));
    output << ",\"concepts\":";
    writeJsonStringArray(output, response.query.concepts);
    output << ",\"results\":[";
    for (std::size_t index = 0; index < results.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        writeJsonSearchResult(output, *results[index], explain_requested);
    }
    output << ']';
    output << ",\"entry\":";

    const SearchResult* selected = decision.selected;
    if (decision.status == ResultStatus::confident && selected == nullptr &&
        !results.empty()) {
        selected = results.front();
    }
    if (decision.status == ResultStatus::confident && selected != nullptr &&
        selected->entry != nullptr) {
        writeJsonEntryValue(output, *selected->entry);
    } else {
        output << "null";
    }
    output << "}\n";
}

void Renderer::entryJson(std::ostream& output,
                         const KnowledgeEntry& value,
                         const KnowledgeBase& /*knowledge*/) {
    output << "{\"status\":\"ok\",\"entry\":";
    writeJsonEntryValue(output, value);
    output << "}\n";
}

void Renderer::listJson(std::ostream& output, const KnowledgeBase& knowledge) {
    const auto entries = sortedEntries(knowledge);
    output << "{\"status\":\"ok\",\"count\":" << entries.size()
           << ",\"entries\":[";
    for (std::size_t index = 0; index < entries.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        writeJsonEntrySummary(output, *entries[index]);
    }
    output << "]}\n";
}

void Renderer::validationJson(
    std::ostream& output,
    const KnowledgeLoadReport& knowledge_report,
    const ConceptLoadReport& concept_report,
    const KnowledgeLintReport& lint_report) {
    const bool issues =
        hasValidationIssues(knowledge_report, concept_report, lint_report);
    const char* validation_status =
        !knowledge_report.root_available ? "error" : (issues ? "invalid" : "valid");

    output << '{';
    output << "\"status\":";
    writeJsonString(output, validation_status);
    output << ",\"knowledge\":{";
    output << "\"root_available\":"
           << (knowledge_report.root_available ? "true" : "false");
    output << ",\"discovered_entries\":"
           << knowledge_report.discovered_entries;
    output << ",\"loaded_entries\":" << knowledge_report.loaded_entries;
    output << ",\"skipped_entries\":" << knowledge_report.skipped_entries;
    output << ",\"empty\":"
           << (knowledge_report.loaded_entries == 0 ? "true" : "false");
    output << '}';
    output << ",\"concepts\":{";
    output << "\"file_present\":"
           << (concept_report.file_present ? "true" : "false");
    output << ",\"usable\":" << (concept_report.usable ? "true" : "false");
    output << ",\"concepts_loaded\":" << concept_report.concepts_loaded;
    output << '}';
    output << ",\"lint\":{";
    output << "\"performed\":" << (lint_report.performed ? "true" : "false");
    output << ",\"entries_checked\":" << lint_report.entries_checked;
    output << ",\"aliases_checked\":" << lint_report.aliases_checked;
    output << ",\"keywords_checked\":" << lint_report.keywords_checked;
    output << ",\"concepts_checked\":" << lint_report.concepts_checked;
    output << '}';
    output << ",\"diagnostics\":[";

    bool first = true;
    for (const auto& diagnostic : knowledge_report.diagnostics) {
        if (!first) {
            output << ',';
        }
        writeJsonDiagnostic(output, "knowledge", diagnostic);
        first = false;
    }
    for (const auto& diagnostic : concept_report.diagnostics) {
        if (!first) {
            output << ',';
        }
        writeJsonDiagnostic(output, "concepts", diagnostic);
        first = false;
    }
    for (const auto& diagnostic : lint_report.diagnostics) {
        if (!first) {
            output << ',';
        }
        writeJsonDiagnostic(output, "lint", diagnostic);
        first = false;
    }
    output << "]}\n";
}

}  // namespace howlinux
