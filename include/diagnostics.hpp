#pragma once

#include <filesystem>
#include <string>

namespace howlinux {

enum class DiagnosticSeverity {
    warning,
    error,
};

struct Diagnostic {
    DiagnosticSeverity severity{DiagnosticSeverity::warning};
    std::filesystem::path path;
    std::string entry_id;
    std::string message;
};

std::string formatDiagnostic(const Diagnostic& diagnostic);

}  // namespace howlinux
