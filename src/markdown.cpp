#include "markdown.hpp"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace howlinux {
namespace {

std::string_view trim(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r");
    if (first == std::string_view::npos) return {};
    return text.substr(first, text.find_last_not_of(" \t\r") - first + 1);
}

// Preserve text layout and escape ASCII terminal controls from content.
void literal(std::ostream& output, std::string_view text) {
    constexpr char hex[] = "0123456789ABCDEF";
    for (const unsigned char ch : text) {
        if ((ch < 32 && ch != '\t' && ch != '\n') || ch == 127) {
            output << "\\x" << hex[ch >> 4] << hex[ch & 15];
        } else {
            output << static_cast<char>(ch);
        }
    }
}

void styled(std::ostream& output, std::string_view text,
            const char* style, bool color) {
    if (color) output << style;
    literal(output, text);
    if (color) output << "\x1b[0m";
}

bool whitespace(char ch) {
    return std::isspace(static_cast<unsigned char>(ch)) != 0;
}

void inlineText(std::ostream& output, std::string_view text, bool color) {
    for (std::size_t pos = 0; pos < text.size();) {
        if (text[pos] == '\\' && pos + 1 < text.size() &&
            std::string_view("\\`*_{}[]()#+-.!>").find(text[pos + 1]) !=
                std::string_view::npos) {
            literal(output, text.substr(pos + 1, 1));
            pos += 2;
            continue;
        }
        if (text[pos] == '`') {
            std::size_t count = 1;
            while (pos + count < text.size() && text[pos + count] == '`') ++count;
            const auto delimiter = text.substr(pos, count);
            auto end = text.find(delimiter, pos + count);
            while (end != std::string_view::npos &&
                   ((end > 0 && text[end - 1] == '`') ||
                    (end + count < text.size() && text[end + count] == '`'))) {
                end = text.find(delimiter, end + count);
            }
            if (end != std::string_view::npos) {
                std::string code(text.substr(pos + count, end - pos - count));
                for (char& ch : code) {
                    if (ch == '\n') ch = ' ';
                }
                styled(output, code, "\x1b[36m", color);
                pos = end + count;
                continue;
            }
            literal(output, delimiter);
            pos += count;
            continue;
        }
        if (text[pos] == '[') {
            const auto label_end = text.find("](", pos + 1);
            if (label_end != std::string_view::npos) {
                auto end = label_end + 2;
                unsigned depth = 1;
                for (; end < text.size(); ++end) {
                    if (text[end] == '(') ++depth;
                    if (text[end] == ')' && --depth == 0) break;
                }
                if (end < text.size()) {
                    const auto label = text.substr(pos + 1, label_end - pos - 1);
                    const auto url = text.substr(label_end + 2, end - label_end - 2);
                    styled(output, label, "\x1b[4m", color);
                    if (url != label) {
                        output << " (";
                        literal(output, url);
                        output << ')';
                    }
                    pos = end + 1;
                    continue;
                }
            }
        }
        if (text[pos] == '*' || text[pos] == '_') {
            const char marker = text[pos];
            const std::size_t count = pos + 1 < text.size() &&
                text[pos + 1] == marker ? 2 : 1;
            const auto start = pos + count;
            const bool boundary = pos == 0 || !std::isalnum(
                static_cast<unsigned char>(text[pos - 1]));
            if (boundary && start < text.size() && !whitespace(text[start])) {
                const auto end = text.find(text.substr(pos, count), start);
                if (end != std::string_view::npos && end > start &&
                    !whitespace(text[end - 1]) &&
                    (end + count == text.size() || !std::isalnum(
                        static_cast<unsigned char>(text[end + count])))) {
                    styled(output, text.substr(start, end - start),
                           count == 2 ? "\x1b[1m" : "\x1b[3m", color);
                    pos = end + count;
                    continue;
                }
            }
        }
        literal(output, text.substr(pos++, 1));
    }
}

}  // namespace

bool terminalColorEnabled(std::ostream& output) {
    const char* no_color = std::getenv("NO_COLOR");
    const char* term = std::getenv("TERM");
    if ((no_color != nullptr && *no_color != '\0') || term == nullptr ||
        *term == '\0' || std::string_view(term) == "dumb") return false;
#ifndef _WIN32
    return &output == &std::cout && ::isatty(STDOUT_FILENO) != 0;
#else
    (void)output;
    return false;
#endif
}

void renderMarkdown(std::ostream& output, std::string_view markdown, bool color) {
    char fence = 0;
    std::size_t fence_length = 0;
    std::size_t fence_indent = 0;
    std::string paragraph;
    const auto flushParagraph = [&]() {
        if (!paragraph.empty()) {
            inlineText(output, paragraph, color);
            output << '\n';
            paragraph.clear();
        }
    };
    while (!markdown.empty()) {
        const auto newline = markdown.find('\n');
        auto line = markdown.substr(0, newline);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
        if (newline == std::string_view::npos) markdown = {};
        else markdown.remove_prefix(newline + 1);

        const auto body = trim(line);
        const auto indent = line.find_first_not_of(' ');
        std::size_t run = 0;
        if (!body.empty() && (body.front() == '`' || body.front() == '~')) {
            while (run < body.size() && body[run] == body.front()) ++run;
        }
        const bool fence_candidate = indent <= 3 && run >= 3;
        if (fence != 0) {
            if (fence_candidate && body.front() == fence && run >= fence_length &&
                trim(body.substr(run)).empty()) {
                fence = 0;
            } else {
                // Remove only the opening fence's indentation; code stays literal.
                std::size_t removed = 0;
                while (removed < fence_indent && removed < line.size() &&
                       line[removed] == ' ') ++removed;
                output << "    ";
                styled(output, line.substr(removed), "\x1b[36m", color);
                output << '\n';
            }
            continue;
        }
        if (fence_candidate && (body.front() != '`' ||
            body.substr(run).find('`') == std::string_view::npos)) {
            flushParagraph();
            fence = body.front();
            fence_length = run;
            fence_indent = indent;
            continue;
        }
        if ((indent >= 4 || line.starts_with('\t')) && !body.empty()) {
            flushParagraph();
            styled(output, line, "\x1b[36m", color);
            output << '\n';
            continue;
        }
        std::size_t heading = 0;
        while (heading < body.size() && body[heading] == '#') ++heading;
        if (heading >= 1 && heading <= 6 &&
            (heading == body.size() || whitespace(body[heading]))) {
            flushParagraph();
            auto title = trim(body.substr(heading));
            const auto last = title.find_last_not_of('#');
            if (last != std::string_view::npos && last + 1 < title.size() &&
                whitespace(title[last])) title = trim(title.substr(0, last));
            if (color) output << "\x1b[1;36m";
            inlineText(output, title, false);
            if (color) output << "\x1b[0m";
            output << '\n';
            if (!color) output << std::string(title.size(), heading == 1 ? '=' : '-') << '\n';
        } else if (body.starts_with("> ") || body == ">") {
            flushParagraph();
            output << "  | ";
            inlineText(output, body.size() > 1 ? body.substr(2) : "", color);
            output << '\n';
        } else if (body.starts_with("- ") || body.starts_with("* ") ||
                   body.starts_with("+ ")) {
            flushParagraph();
            output << std::string(indent, ' ') << "  - ";
            inlineText(output, body.substr(2), color);
            output << '\n';
        } else if (body.empty()) {
            flushParagraph();
            output << '\n';
        } else {
            if (!paragraph.empty()) paragraph += '\n';
            paragraph += line;
        }
    }
    flushParagraph();
}

}  // namespace howlinux
