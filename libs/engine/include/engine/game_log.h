#pragma once

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <string>
#include <vector>
#include <memory>

namespace engine {

// Styling for a log fragment
struct LogFragmentStyle {
    ftxui::Color color = ftxui::Color::Default;
    ftxui::Color bg_color = ftxui::Color::Default;
    bool bold = false;
    bool dim = false;
    bool underlined = false;
    bool blink = false;
};

// A fragment of text within a log entry
struct LogFragment {
    std::string text;
    LogFragmentStyle style;
};

// A complete log entry consisting of multiple styled fragments
class LogEntry {
public:
    LogEntry() = default;

    void add_fragment(const std::string& text, const LogFragmentStyle& style);
    [[nodiscard]] ftxui::Element render() const;

private:
    std::vector<LogFragment> fragments_;
};

// Forward declaration
class GameLog;

// Fluent builder for creating log entries
class LogEntryBuilder {
public:
    explicit LogEntryBuilder(GameLog* log) : log_(log) {}

    LogEntryBuilder& text(const std::string& text);
    LogEntryBuilder& color(ftxui::Color color);
    LogEntryBuilder& bold(bool bold = true);
    LogEntryBuilder& dim(bool dim = true);
    LogEntryBuilder& underlined(bool underlined = true);
    LogEntryBuilder& blink(bool blink = true);
    LogEntryBuilder& reset_style();

    void log();

private:
    GameLog* log_;
    LogEntry entry_;
    LogFragmentStyle current_style_;
};

// The game log that manages multiple entries
class GameLog {
public:
    GameLog() = default;

    // Start a new log entry with fluent interface
    LogEntryBuilder entry();

    // Get all entries
    [[nodiscard]] const std::vector<LogEntry>& get_entries() const { return entries_; }

    // Get total number of entries
    [[nodiscard]] int get_entry_count() const { return static_cast<int>(entries_.size()); }

private:
    friend class LogEntryBuilder;
    void add_entry(LogEntry entry);

    std::vector<LogEntry> entries_;
};

} // namespace engine
