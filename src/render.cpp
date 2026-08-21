#include "render.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace {

std::string toUpper(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::toupper);
    return out;
}

// Sucht einen Eintrag anhand seiner ID (für "related") und gibt ein
// gut lesbares Label dafür zurück (bevorzugt den ersten Alias).
std::string labelForId(const std::string& id, const KnowledgeBase& kb) {
    auto it = std::find_if(kb.entries().begin(), kb.entries().end(),
                            [&](const KnowledgeEntry& e) { return e.id == id; });

    if (it == kb.entries().end()) {
        return id; // Eintrag nicht gefunden -> ID selbst als Fallback anzeigen
    }
    return it->aliases.empty() ? it->id : it->aliases.front();
}

std::string removeMarkdownMarkers(const std::string& line) {
    std::string output;
    output.reserve(line.size());

    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '`') {
            continue;
        }
        if (line[i] == '*' && i + 1 < line.size() && line[i + 1] == '*') {
            ++i;
            continue;
        }
        output += line[i];
    }

    return output;
}

std::string renderMarkdown(const std::string& markdown) {
    std::istringstream input(markdown);
    std::ostringstream output;
    std::string line;
    bool inCodeBlock = false;

    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.rfind("```", 0) == 0) {
            inCodeBlock = !inCodeBlock;
            continue;
        }

        if (!inCodeBlock && line.rfind("## ", 0) == 0) {
            output << "\n" << line.substr(3) << "\n";
            continue;
        }

        output << removeMarkdownMarkers(line) << "\n";
    }

    return output.str();
}

} // namespace

void Renderer::renderEntry(const KnowledgeEntry& entry, const KnowledgeBase& kb) {
    std::cout << "\n" << toUpper(entry.title) << "\n";
    std::cout << std::string(40, '-') << "\n\n";
    std::cout << renderMarkdown(entry.content) << "\n";

    if (!entry.related.empty()) {
        std::cout << "RELATED\n";
        for (const auto& relatedId : entry.related) {
            std::cout << "  howlinux " << labelForId(relatedId, kb) << "\n";
        }
        std::cout << "\n";
    }
}

void Renderer::renderNoMatch(const std::string& query) {
    std::cout << "\nNo local knowledge found for: " << query << "\n\n";
}

void Renderer::renderSuggestions(const std::string& query,
                                  const std::vector<SearchResult>& weakResults) {
    std::cout << "\nNo confident local match found for: " << query << "\n\n";

    if (!weakResults.empty()) {
        std::cout << "Did you mean:\n";
        int shown = 0;
        for (const auto& result : weakResults) {
            if (shown >= 3) break;
            std::string label = result.entry->aliases.empty()
                                     ? result.entry->id
                                     : result.entry->aliases.front();
            std::cout << "  howlinux " << label << "\n";
            shown++;
        }
        std::cout << "\n";
    }
}
