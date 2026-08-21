#include "knowledge.hpp"
#include "search.hpp"
#include "render.hpp"

#include <iostream>
#include <sstream>

int main(int argc, char** argv) {
    // Alle Argumente zu einem String zusammenfügen.
    // "howlinux rename folder" -> argv = ["howlinux", "rename", "folder"]
    // -> query = "rename folder"
    std::ostringstream queryStream;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) queryStream << " ";
        queryStream << argv[i];
    }
    std::string query = queryStream.str();

    if (query.empty()) {
        std::cout << "Usage: howlinux <query>\n";
        std::cout << "Example: howlinux rename folder\n";
        return 0;
    }

    // Knowledge Base laden. "knowledge" wird relativ zum aktuellen
    // Arbeitsverzeichnis gesucht -> howlinux muss aus dem Projekt-Root
    // gestartet werden (oder du kopierst den knowledge Ordner neben das Binary).
    KnowledgeBase kb;
    kb.load("knowledge");

    if (kb.entries().empty()) {
        std::cerr << "Warning: No knowledge entries found."
                     "Make sure the 'knowledge/' folder "
                     "is located in the current directory.\n";
    }

    SearchEngine engine(kb);
    auto results = engine.search(query);

    if (results.empty()) {
        Renderer::renderNoMatch(query);
        return 0;
    }

    // Score threshold for a "confident" match.
    // Everything below it is shown only as a suggestion instead of an answer.
    constexpr double STRONG_MATCH = 45.0;

    if (results.front().score >= STRONG_MATCH) {
        Renderer::renderEntry(*results.front().entry, kb);
    } else {
        Renderer::renderSuggestions(query, results);
    }

    return 0;
}
