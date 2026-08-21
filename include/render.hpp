#pragma once
#include "knowledge.hpp"
#include "search.hpp"

class Renderer {
public:
    // Zeigt einen gefundenen Eintrag vollständig an (Titel, Content, Related)
    static void renderEntry(const KnowledgeEntry& entry, const KnowledgeBase& kb);

    // Wird angezeigt wenn es gar keine Treffer gibt
    static void renderNoMatch(const std::string& query);

    // Wird angezeigt wenn es nur schwache Treffer gibt (score < STRONG_MATCH)
    static void renderSuggestions(const std::string& query,
                                   const std::vector<SearchResult>& weakResults);
};
