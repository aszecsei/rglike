/**r
 * @file formatting.hpp
 * @author Alic Szecsei
 * @date 6/6/2023
 */

#pragma once

#include <deque>
#include <ftxui/screen/color.hpp>
#include <iterator>

namespace rglike {
    namespace formatting {
        struct Format {
            ftxui::Color color;
            ftxui::Color bg_color;
            bool bold = false;
            bool dim = false;
            bool underlined = false;
            bool blinking = false;
        };

        struct Fragment {
            Format format;
            std::string text;

            Fragment(const Format& format, const std::string_view& text)
                : format(format)
                , text(text) { }
        };

        class Text {
        private:
            std::deque<Fragment> m_fragments;

        public:
            Text()
                : m_fragments({}) { }

            void push_front(const Fragment& fragment) { m_fragments.push_front(fragment); }

            void push_front(Fragment&& fragment) { m_fragments.push_front(std::move(fragment)); }

            void push_back(const Fragment& fragment) { m_fragments.push_back(fragment); }

            void push_back(Fragment&& fragment) { m_fragments.push_back(std::move(fragment)); }

            template<class... Args> void emplace_front(Args&&... args) {
                m_fragments.emplace_front(args...);
            }

            template<class... Args> void emplace_back(Args&&... args) {
                m_fragments.emplace_back(args...);
            }

            auto plain_text() -> std::string {
                std::string result{};
                for (const auto& frag : m_fragments) { result += frag.text; }
                return result;
            }
        };
    } // namespace formatting

    auto format_text(const std::string_view& text) -> formatting::Text;
} // namespace rglike