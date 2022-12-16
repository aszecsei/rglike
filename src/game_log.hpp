/**
 * @file game_log.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include "ftxui/screen/color.hpp"
#include <deque>
#include <string>
#include <utility>
#include <vector>

namespace rglike {
    struct GameLogFragment {
        ftxui::Color color;
        ftxui::Color bg_color;
        bool bold;
        bool dim;
        bool underlined;
        bool blinking;
        std::string text;

        GameLogFragment(
            ftxui::Color color, ftxui::Color bg_color, bool bold, bool dim, bool underlined,
            bool blinking, const std::string_view& text
        )
            : color(color)
            , bg_color(bg_color)
            , bold(bold)
            , dim(dim)
            , underlined(underlined)
            , blinking(blinking)
            , text(text) { }
    };

    class GameLog;

    class LogEntry {
    private:
        ftxui::Color m_current_color = ftxui::Color::Default;
        ftxui::Color m_current_bg_color = ftxui::Color::Default;
        bool m_current_bold = false;
        bool m_current_dim = false;
        bool m_current_underlined = false;
        bool m_current_blinking = false;

        std::vector<GameLogFragment> m_fragments;
        GameLog* m_log;

        explicit LogEntry(GameLog* log)
            : m_log(log) { }

        [[nodiscard]] inline auto fragments() const -> std::vector<GameLogFragment> {
            return m_fragments;
        }

    public:
        [[nodiscard]] auto color(ftxui::Color color) -> LogEntry&;
        [[nodiscard]] auto bg_color(ftxui::Color color) -> LogEntry&;
        [[nodiscard]] auto bold(bool bold) -> LogEntry&;
        [[nodiscard]] auto dim(bool dim) -> LogEntry&;
        [[nodiscard]] auto underline(bool underline) -> LogEntry&;
        [[nodiscard]] auto blink(bool blink) -> LogEntry&;
        [[nodiscard]] auto text(const std::string_view& text) -> LogEntry&;
        void log() const;

        friend class GameLog;
    };

    class GameLog {
    private:
        GameLog() = default;
        ~GameLog() = default;

        std::deque<std::vector<GameLogFragment>> m_log;

        void log(const LogEntry& entry);

    public:
        static auto GetInstance() -> GameLog&;

        GameLog(const GameLog&) = delete;
        auto operator=(const GameLog&) -> GameLog& = delete;
        GameLog(GameLog&&) = delete;
        auto operator=(GameLog&&) -> GameLog& = delete;

        auto entry() -> LogEntry;
        void log(const std::string_view& text);

        [[nodiscard]] inline auto size() const -> size_t { return m_log.size(); }

        [[nodiscard]] inline auto entries() const
            -> const std::deque<std::vector<GameLogFragment>>& {
            return m_log;
        }

        friend class LogEntry;
    };
} // namespace rglike