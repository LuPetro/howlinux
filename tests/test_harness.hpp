#pragma once

#include "diagnostics.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace hltest {

class Failure final : public std::runtime_error {
public:
    explicit Failure(const std::string& message) : std::runtime_error(message) {}
};

[[noreturn]] inline void fail(const char* expression,
                              const char* file,
                              int line,
                              const std::string& detail = {}) {
    std::ostringstream message;
    message << file << ':' << line << ": assertion failed: " << expression;
    if (!detail.empty()) {
        message << " (" << detail << ')';
    }
    throw Failure(message.str());
}

inline void requireContains(std::string_view haystack,
                            std::string_view needle,
                            const char* expression,
                            const char* file,
                            int line) {
    if (haystack.find(needle) == std::string_view::npos) {
        fail(expression, file, line,
             "missing substring '" + std::string(needle) + "'");
    }
}

inline void requireNear(double actual,
                        double expected,
                        double tolerance,
                        const char* expression,
                        const char* file,
                        int line) {
    if (std::abs(actual - expected) > tolerance) {
        std::ostringstream detail;
        detail << "actual=" << actual << ", expected=" << expected
               << ", tolerance=" << tolerance;
        fail(expression, file, line, detail.str());
    }
}

using TestFunction = void (*)();

struct TestCase {
    std::string name;
    TestFunction function;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

class Registrar {
public:
    Registrar(const char* name, TestFunction function) {
        registry().push_back({name, function});
    }
};

class TemporaryDirectory {
public:
    TemporaryDirectory() {
        static std::atomic<unsigned long long> sequence{0};
        const auto stamp = std::chrono::high_resolution_clock::now()
                               .time_since_epoch()
                               .count();
        path_ = std::filesystem::temp_directory_path() /
                ("howlinux-tests-" + std::to_string(stamp) + '-' +
                 std::to_string(sequence.fetch_add(1)));
        std::error_code error;
        if (!std::filesystem::create_directories(path_, error) || error) {
            throw Failure("cannot create temporary directory '" +
                          path_.string() + "': " + error.message());
        }
    }

    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;

    ~TemporaryDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(path_, ignored);
    }

    [[nodiscard]] const std::filesystem::path& path() const noexcept {
        return path_;
    }

private:
    std::filesystem::path path_;
};

inline void writeText(const std::filesystem::path& path,
                      const std::string& content) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        throw Failure("cannot create fixture directory '" +
                      path.parent_path().string() + "': " + error.message());
    }

    std::ofstream output(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!output) {
        throw Failure("cannot write fixture file '" + path.string() + "'");
    }
    output.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (!output) {
        throw Failure("failed while writing fixture file '" + path.string() + "'");
    }
}

inline bool diagnosticsContain(const std::vector<howlinux::Diagnostic>& diagnostics,
                               std::string_view fragment) {
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.message.find(fragment) != std::string::npos ||
            diagnostic.entry_id.find(fragment) != std::string::npos ||
            diagnostic.path.generic_string().find(fragment) != std::string::npos) {
            return true;
        }
    }
    return false;
}

class EnvironmentGuard {
public:
    explicit EnvironmentGuard(std::string name) : name_(std::move(name)) {
        if (const char* current = std::getenv(name_.c_str()); current != nullptr) {
            previous_ = std::string(current);
        }
        unset();
    }

    EnvironmentGuard(const EnvironmentGuard&) = delete;
    EnvironmentGuard& operator=(const EnvironmentGuard&) = delete;

    ~EnvironmentGuard() {
        if (previous_) {
            set(*previous_);
        } else {
            unset();
        }
    }

private:
    void set(const std::string& value) const {
#ifdef _WIN32
        (void)_putenv_s(name_.c_str(), value.c_str());
#else
        (void)setenv(name_.c_str(), value.c_str(), 1);
#endif
    }

    void unset() const {
#ifdef _WIN32
        (void)_putenv_s(name_.c_str(), "");
#else
        (void)unsetenv(name_.c_str());
#endif
    }

    std::string name_;
    std::optional<std::string> previous_;
};

class JsonParser {
public:
    explicit JsonParser(std::string_view input) : input_(input) {}

    [[nodiscard]] bool parse() {
        skipWhitespace();
        if (!parseValue()) {
            return false;
        }
        skipWhitespace();
        return position_ == input_.size();
    }

private:
    void skipWhitespace() {
        while (position_ < input_.size() &&
               (input_[position_] == ' ' || input_[position_] == '\n' ||
                input_[position_] == '\r' || input_[position_] == '\t')) {
            ++position_;
        }
    }

    [[nodiscard]] bool consume(char expected) {
        skipWhitespace();
        if (position_ >= input_.size() || input_[position_] != expected) {
            return false;
        }
        ++position_;
        return true;
    }

    [[nodiscard]] bool parseValue() {
        skipWhitespace();
        if (position_ >= input_.size()) {
            return false;
        }
        switch (input_[position_]) {
            case '{':
                return parseObject();
            case '[':
                return parseArray();
            case '"':
                return parseString();
            case 't':
                return parseLiteral("true");
            case 'f':
                return parseLiteral("false");
            case 'n':
                return parseLiteral("null");
            default:
                return parseNumber();
        }
    }

    [[nodiscard]] bool parseObject() {
        if (!consume('{')) {
            return false;
        }
        skipWhitespace();
        if (position_ < input_.size() && input_[position_] == '}') {
            ++position_;
            return true;
        }
        while (true) {
            if (!parseString() || !consume(':') || !parseValue()) {
                return false;
            }
            skipWhitespace();
            if (position_ < input_.size() && input_[position_] == '}') {
                ++position_;
                return true;
            }
            if (!consume(',')) {
                return false;
            }
        }
    }

    [[nodiscard]] bool parseArray() {
        if (!consume('[')) {
            return false;
        }
        skipWhitespace();
        if (position_ < input_.size() && input_[position_] == ']') {
            ++position_;
            return true;
        }
        while (true) {
            if (!parseValue()) {
                return false;
            }
            skipWhitespace();
            if (position_ < input_.size() && input_[position_] == ']') {
                ++position_;
                return true;
            }
            if (!consume(',')) {
                return false;
            }
        }
    }

    [[nodiscard]] bool parseString() {
        skipWhitespace();
        if (position_ >= input_.size() || input_[position_] != '"') {
            return false;
        }
        ++position_;
        while (position_ < input_.size()) {
            const unsigned char character =
                static_cast<unsigned char>(input_[position_++]);
            if (character == static_cast<unsigned char>('"')) {
                return true;
            }
            if (character < 0x20U) {
                return false;
            }
            if (character != static_cast<unsigned char>('\\')) {
                continue;
            }
            if (position_ >= input_.size()) {
                return false;
            }
            const char escape = input_[position_++];
            if (escape == '"' || escape == '\\' || escape == '/' ||
                escape == 'b' || escape == 'f' || escape == 'n' ||
                escape == 'r' || escape == 't') {
                continue;
            }
            if (escape != 'u' || position_ + 4 > input_.size()) {
                return false;
            }
            for (int digit = 0; digit < 4; ++digit) {
                const char value = input_[position_++];
                const bool hexadecimal =
                    (value >= '0' && value <= '9') ||
                    (value >= 'a' && value <= 'f') ||
                    (value >= 'A' && value <= 'F');
                if (!hexadecimal) {
                    return false;
                }
            }
        }
        return false;
    }

    [[nodiscard]] bool parseLiteral(std::string_view literal) {
        if (input_.substr(position_, literal.size()) != literal) {
            return false;
        }
        position_ += literal.size();
        return true;
    }

    [[nodiscard]] bool parseNumber() {
        const std::size_t beginning = position_;
        if (position_ < input_.size() && input_[position_] == '-') {
            ++position_;
        }
        if (position_ >= input_.size()) {
            return false;
        }
        if (input_[position_] == '0') {
            ++position_;
        } else {
            if (input_[position_] < '1' || input_[position_] > '9') {
                return false;
            }
            while (position_ < input_.size() && input_[position_] >= '0' &&
                   input_[position_] <= '9') {
                ++position_;
            }
        }
        if (position_ < input_.size() && input_[position_] == '.') {
            ++position_;
            const std::size_t fractional_beginning = position_;
            while (position_ < input_.size() && input_[position_] >= '0' &&
                   input_[position_] <= '9') {
                ++position_;
            }
            if (position_ == fractional_beginning) {
                return false;
            }
        }
        if (position_ < input_.size() &&
            (input_[position_] == 'e' || input_[position_] == 'E')) {
            ++position_;
            if (position_ < input_.size() &&
                (input_[position_] == '+' || input_[position_] == '-')) {
                ++position_;
            }
            const std::size_t exponent_beginning = position_;
            while (position_ < input_.size() && input_[position_] >= '0' &&
                   input_[position_] <= '9') {
                ++position_;
            }
            if (position_ == exponent_beginning) {
                return false;
            }
        }
        return position_ > beginning;
    }

    std::string_view input_;
    std::size_t position_{0};
};

inline bool isValidJson(const std::string& value) {
    return JsonParser(value).parse();
}

}  // namespace hltest

#define HL_TEST(name)                                                        \
    static void name();                                                      \
    namespace {                                                              \
    const ::hltest::Registrar name##_registrar{#name, &name};               \
    }                                                                        \
    static void name()

#define HL_REQUIRE(expression)                                               \
    do {                                                                     \
        if (!(expression)) {                                                 \
            ::hltest::fail(#expression, __FILE__, __LINE__);                \
        }                                                                    \
    } while (false)

#define HL_REQUIRE_EQ(actual, expected)                                      \
    do {                                                                     \
        const auto hl_actual = (actual);                                     \
        const auto hl_expected = (expected);                                 \
        if (!(hl_actual == hl_expected)) {                                   \
            ::hltest::fail(#actual " == " #expected, __FILE__, __LINE__);  \
        }                                                                    \
    } while (false)

#define HL_REQUIRE_CONTAINS(haystack, needle)                                \
    ::hltest::requireContains((haystack), (needle),                          \
                              #haystack " contains " #needle, __FILE__,     \
                              __LINE__)

#define HL_REQUIRE_NEAR(actual, expected, tolerance)                         \
    ::hltest::requireNear((actual), (expected), (tolerance),                 \
                          #actual " near " #expected, __FILE__, __LINE__)
