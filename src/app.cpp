#include "app.hpp"

#include "cli.hpp"
#include "concepts.hpp"
#include "config.hpp"
#include "diagnostics.hpp"
#include "knowledge.hpp"
#include "render.hpp"
#include "search.hpp"

#include <algorithm>
#include <filesystem>
#include <ostream>
#include <string>
#include <vector>

namespace howlinux {
namespace {

constexpr int kSuccess = 0;
constexpr int kNoConfidentResult = 1;
constexpr int kInvalidArguments = 2;
constexpr int kConfigurationError = 3;

void emitDiagnostics(std::ostream& error,
                     const std::vector<Diagnostic>& diagnostics) {
    for (const auto& diagnostic : diagnostics) {
        error << formatDiagnostic(diagnostic) << '\n';
    }
}

void renderConfigurationErrorJson(
    std::ostream& output,
    const std::string& error_code,
    const std::vector<Diagnostic>& diagnostics) {
    output << "{\"status\":\"error\",\"error\":\""
           << escapeJson(error_code) << "\",\"diagnostics\":[";
    for (std::size_t index = 0; index < diagnostics.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        const auto& diagnostic = diagnostics[index];
        output << "{\"severity\":\""
               << (diagnostic.severity == DiagnosticSeverity::error ? "error"
                                                                     : "warning")
               << "\",\"path\":\""
               << escapeJson(diagnostic.path.generic_string())
               << "\",\"entry_id\":\"" << escapeJson(diagnostic.entry_id)
               << "\",\"message\":\"" << escapeJson(diagnostic.message)
               << "\"}";
    }
    output << "]}\n";
}

}  // namespace

int runApplication(const std::vector<std::string>& arguments,
                   const std::filesystem::path& executable_path,
                   const std::filesystem::path& current_directory,
                   std::ostream& output,
                   std::ostream& error) {
    const auto parsed = parseCommandLine(arguments);
    if (!parsed.ok) {
        error << "Error: " << parsed.error << "\n"
              << "Try 'howlinux --help' for usage.\n";
        return kInvalidArguments;
    }
    const auto& options = parsed.options;

    if (options.command == CliCommand::help) {
        printHelp(output);
        return kSuccess;
    }
    if (options.command == CliCommand::version) {
        output << "howlinux " << kVersion << '\n';
        return kSuccess;
    }

    const auto knowledge_path = resolveKnowledgePath(
        options, executable_path, current_directory);
    KnowledgeBase knowledge;
    const auto knowledge_report = knowledge.load(knowledge_path);
    if (!knowledge_report.root_available) {
        emitDiagnostics(error, knowledge_report.diagnostics);
        if (options.command == CliCommand::validate && options.json) {
            Renderer::validationJson(output, knowledge_report, {});
        } else if (options.json) {
            renderConfigurationErrorJson(
                output, "knowledge_unavailable", knowledge_report.diagnostics);
        }
        return kConfigurationError;
    }

    ConceptDictionary concepts;
    const auto concept_report = concepts.load(knowledge_path / "concepts.yaml");

    if (options.command != CliCommand::validate) {
        emitDiagnostics(error, knowledge_report.diagnostics);
        emitDiagnostics(error, concept_report.diagnostics);
    }
    if (!concept_report.usable) {
        if (options.command == CliCommand::validate) {
            if (options.json) {
                Renderer::validationJson(output, knowledge_report, concept_report);
            } else {
                Renderer::validation(output, knowledge_report, concept_report);
            }
        } else if (options.json) {
            renderConfigurationErrorJson(
                output, "concepts_invalid", concept_report.diagnostics);
        }
        return kConfigurationError;
    }

    if (options.command == CliCommand::validate) {
        if (options.json) {
            Renderer::validationJson(output, knowledge_report, concept_report);
        } else {
            Renderer::validation(output, knowledge_report, concept_report);
        }
        return knowledge_report.hasIssues() || concept_report.hasIssues()
                   ? kNoConfidentResult
                   : kSuccess;
    }

    if (knowledge.entries().empty()) {
        error << "Warning: the knowledge directory contains no valid entries: "
              << knowledge_path.string() << '\n';
    }

    if (options.command == CliCommand::list) {
        if (options.json) {
            Renderer::listJson(output, knowledge);
        } else {
            Renderer::list(output, knowledge);
        }
        return kSuccess;
    }

    if (options.command == CliCommand::show) {
        const auto* entry = knowledge.findById(options.entry_id);
        if (entry == nullptr) {
            if (options.json) {
                output << "{\"status\":\"not_found\",\"id\":\""
                       << escapeJson(options.entry_id) << "\"}\n";
            } else {
                output << "No knowledge entry with ID '" << options.entry_id << "'.\n";
            }
            return kNoConfidentResult;
        }
        if (options.json) {
            Renderer::entryJson(output, *entry, knowledge);
        } else {
            Renderer::entry(output, *entry, knowledge);
        }
        return kSuccess;
    }

    SearchEngine engine(knowledge, concepts);
    const auto internal_limit = std::max<std::size_t>(options.limit, 2);
    const auto response = engine.search(options.query, internal_limit);
    const auto decision = ResultPolicy{}.decide(response.results);

    if (options.json) {
        Renderer::searchJson(output, response, decision, options.limit, options.explain);
    } else if (decision.status == ResultStatus::confident) {
        Renderer::entry(output, *decision.selected->entry, knowledge);
        if (options.explain) {
            Renderer::explain(output, response, options.limit);
        }
    } else if (decision.status == ResultStatus::uncertain) {
        Renderer::suggestions(output, response, options.limit, options.explain);
    } else {
        Renderer::noMatch(output, options.query);
    }

    return decision.status == ResultStatus::confident ? kSuccess : kNoConfidentResult;
}

}  // namespace howlinux
