/**
 * @file game_log.cpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */
#include "game_log.hpp"

#include "constants.hpp"
#include <sstream>

namespace rglike {
    auto LogEntry::Color(ftxui::Color color) -> LogEntry& {
        m_current_color = color;
        return *this;
    }

    auto LogEntry::BgColor(ftxui::Color color) -> LogEntry& {
        m_current_bg_color = color;
        return *this;
    }

    auto LogEntry::Bold(bool bold) -> LogEntry& {
        m_current_bold = bold;
        return *this;
    }

    auto LogEntry::Dim(bool dim) -> LogEntry& {
        m_current_dim = dim;
        return *this;
    }

    auto LogEntry::Underline(bool underline) -> LogEntry& {
        m_current_underlined = underline;
        return *this;
    }

    auto LogEntry::Blink(bool blink) -> LogEntry& {
        m_current_blinking = blink;
        return *this;
    }

    auto LogEntry::Text(const std::string_view& text) -> LogEntry& {
        m_fragments.emplace_back(
            m_current_color, m_current_bg_color, m_current_bold, m_current_dim,
            m_current_underlined, m_current_blinking, text
        );
        return *this;
    }

    void LogEntry::Log() const { m_log->Log(*this); }

    auto GameLog::GetInstance() -> GameLog& {
        static GameLog instance{};
        return instance;
    }

    void GameLog::Log(const std::string_view& text) { Entry().Text(text).Log(); }

    void GameLog::Log(const LogEntry& entry) {
        if (m_log.size() == MAX_LOG_LINES) { m_log.pop_back(); }
        m_log.push_front(entry.Fragments());
    }

    auto GameLog::Entry() -> LogEntry { return LogEntry(this); }
} // namespace rglike
