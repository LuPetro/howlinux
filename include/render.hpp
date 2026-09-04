#pragma once

#include "concepts.hpp"
#include "knowledge.hpp"
#include "search.hpp"

#include <cstddef>
#include <iosfwd>
#include <string>

namespace howlinux {

class Renderer {
public:
    static void entry(std::ostream& output,
                      const KnowledgeEntry& value,
                      const KnowledgeBase& knowledge);
    static void suggestions(std::ostream& output,
                            const SearchResponse& response,
                            std::size_t limit,
                            bool explain);
    static void explain(std::ostream& output,
                        const SearchResponse& response,
                        std::size_t limit);
    static void noMatch(std::ostream& output, const std::string& query);
    static void list(std::ostream& output, const KnowledgeBase& knowledge);
    static void validation(std::ostream& output,
                           const KnowledgeLoadReport& knowledge_report,
                           const ConceptLoadReport& concept_report,
                           const KnowledgeLintReport& lint_report = {});
    static void searchJson(std::ostream& output,
                           const SearchResponse& response,
                           const PolicyDecision& decision,
                           std::size_t limit,
                           bool explain);
    static void entryJson(std::ostream& output,
                          const KnowledgeEntry& value,
                          const KnowledgeBase& knowledge);
    static void listJson(std::ostream& output, const KnowledgeBase& knowledge);
    static void validationJson(std::ostream& output,
                               const KnowledgeLoadReport& knowledge_report,
                               const ConceptLoadReport& concept_report,
                               const KnowledgeLintReport& lint_report = {});
};

[[nodiscard]] std::string escapeJson(const std::string& value);

}  // namespace howlinux
