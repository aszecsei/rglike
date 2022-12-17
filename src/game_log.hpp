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

        [[nodiscard]] inline auto Fragments() const -> std::vector<GameLogFragment> {
            return m_fragments;
        }

    public:
        [[nodiscard]] auto Color(ftxui::Color color) -> LogEntry&;
        [[nodiscard]] auto BgColor(ftxui::Color color) -> LogEntry&;
        [[nodiscard]] auto Bold(bool bold) -> LogEntry&;
        [[nodiscard]] auto Dim(bool dim) -> LogEntry&;
        [[nodiscard]] auto Underline(bool underline) -> LogEntry&;
        [[nodiscard]] auto Blink(bool blink) -> LogEntry&;
        [[nodiscard]] auto Text(const std::string_view& text) -> LogEntry&;
        void Log() const;

        friend class GameLog;
    };

    class GameLog {
    private:
        GameLog() = default;
        ~GameLog() = default;

        std::deque<std::vector<GameLogFragment>> m_log;

        void Log(const LogEntry& entry);

    public:
        static auto GetInstance() -> GameLog&;

        GameLog(const GameLog&) = delete;
        auto operator=(const GameLog&) -> GameLog& = delete;
        GameLog(GameLog&&) = delete;
        auto operator=(GameLog&&) -> GameLog& = delete;

        auto Entry() -> LogEntry;
        void Log(const std::string_view& text);

        [[nodiscard]] inline auto Size() const -> size_t { return m_log.size(); }

        [[nodiscard]] inline auto Entries() const
            -> const std::deque<std::vector<GameLogFragment>>& {
            return m_log;
        }

        friend class LogEntry;
    };
} // namespace rglike