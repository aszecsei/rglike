/**
 * @file formatting.cpp
 * @author Alic Szecsei
 * @date 6/6/2023
 */

#include "rglike/formatting.hpp"
#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <stack>
#include <utility>
#include <variant>

namespace rglike {
    using namespace formatting;

    // AST
    namespace ast {
        struct Element {
            using TextNode = std::variant<std::string, Element>;

            std::string name;
            std::optional<std::string> value;
            std::optional<std::vector<TextNode>> children;

            Element(
                std::string name, std::optional<std::string> value,
                std::optional<std::vector<TextNode>> children
            )
                : name(std::move(name))
                , value(std::move(value))
                , children(std::move(children)) { }
        };

        using Node = std::variant<std::string, Element>;

        struct Document {
            std::vector<Node> nodes;

            explicit Document(std::vector<Node>&& nodes)
                : nodes(std::move(nodes)) { }
        };
    } // namespace ast

    namespace grammar {
        namespace dsl = lexy::dsl;

        struct invalid_character {
            static constexpr auto name() { return "invalid character"; }
        };

        constexpr auto ws = dsl::whitespace(dsl::ascii::space / dsl::ascii::newline);

        struct text : lexy::token_production {
            static constexpr auto rule = [] {
                auto char_ = dsl::code_point - dsl::lit_c<'['> - dsl::lit_c<']'>;
                auto escaped_open = LEXY_LIT("[[");
                auto escaped_close = LEXY_LIT("]]");
                auto allowed = dsl::token(dsl::while_one(char_ | escaped_open | escaped_close));
                return dsl::capture(allowed);
            }();
            static constexpr auto value = lexy::as_string<std::string>;
        };

        // The name of a tag.
        constexpr auto name = [] {
            // We only support ASCII here, as I'm too lazy to type all the code point ranges out.
            auto head_char = dsl::ascii::alpha;
            auto trailing_char = dsl::ascii::alpha_digit_underscore;

            return dsl::identifier(head_char.error<invalid_character>, trailing_char);
        }();

        struct parameter {
            static constexpr auto rule = [] {
                return LEXY_LIT("=") >> ws + dsl::p<text> + ws;
            }();
            static constexpr auto value = lexy::as_string<std::string>;
        };

        struct element {
            struct tag_mismatch {
                static LEXY_CONSTEVAL auto name() { return "closing tag doesn't match"; }
            };

            static constexpr auto whitespace = dsl::ascii::space;
            static constexpr auto rule = [] {
                // The brackets for surrounding the opening and closing tag
                auto open_tagged = dsl::brackets(LEXY_LIT("["), LEXY_LIT("]"));
                auto close_tagged = dsl::brackets(LEXY_LIT("[/"), LEXY_LIT("]"));

                auto name_var = dsl::context_identifier<struct name_var_tag>(name);

                auto open_tag = open_tagged(name_var.capture() + dsl::if_(dsl::p<parameter>));

                auto close_tag = close_tagged(name_var.rematch().error<tag_mismatch>);

                auto content =
                    dsl::peek(LEXY_LIT("[") + dsl::ascii::alpha) >> dsl::recurse<element> |
                    dsl::p<text>;

                return name_var.create() + dsl::brackets(open_tag, close_tag).opt_list(content);
            }();

            static constexpr auto value =
                lexy::as_list<std::vector<ast::Node>> >>
                lexy::callback<ast::Element>(
                    [](auto&& name, std::string&& param, std::vector<ast::Node>&& children) {
                        return ast::Element(
                            lexy::as_string<std::string>(name), LEXY_MOV(param), LEXY_MOV(children)
                        );
                    },
                    [](auto&& name, std::string&& param, lexy::nullopt children = {}) {
                        return ast::Element(
                            lexy::as_string<std::string>(name), LEXY_MOV(param), {}
                        );
                    },
                    [](auto&& name, lexy::nullopt children = {}) {
                        return ast::Element(lexy::as_string<std::string>(name), {}, {});
                    },
                    [](auto&& name, std::vector<ast::Node>&& children) {
                        return ast::Element(
                            lexy::as_string<std::string>(name), {}, LEXY_MOV(children)
                        );
                    }
                );
        };

        struct node {
            static constexpr auto rule = dsl::p<text> | dsl::peek(LEXY_LIT("[")) >> dsl::p<element>;
            static constexpr auto value = lexy::construct<ast::Node>;
        };

        struct document {
            static constexpr auto rule = dsl::list(dsl::p<node>);
            static constexpr auto value =
                lexy::as_list<std::vector<ast::Node>> >> lexy::construct<ast::Document>;
        };
    } // namespace grammar

    auto update_formatting_from_element(const Format& format, const ast::Element& element)
        -> Format {
        Format result = format;

        std::unordered_map<char, ftxui::Color> color_map{
            {'k', ftxui::Color::Black       },
            {'K', ftxui::Color::GrayDark    },
            {'w', ftxui::Color::GrayLight   },
            {'W', ftxui::Color::White       },
            {'b', ftxui::Color::Blue        },
            {'B', ftxui::Color::BlueLight   },
            {'c', ftxui::Color::Cyan        },
            {'C', ftxui::Color::CyanLight   },
            {'g', ftxui::Color::Green       },
            {'G', ftxui::Color::GreenLight  },
            {'m', ftxui::Color::Magenta     },
            {'M', ftxui::Color::MagentaLight},
            {'r', ftxui::Color::Red         },
            {'R', ftxui::Color::RedLight    },
            {'y', ftxui::Color::Yellow      },
            {'Y', ftxui::Color::YellowLight },
        };

        if (element.name == "color") {
            if (element.value.has_value() && element.value.value().length() == 1) {
                auto ch = element.value.value()[0];
                if (color_map.contains(ch)) {
                    result.color = color_map.at(ch);
                    return result;
                }
            }
            // TODO: Parse color hex?
        }

        if (element.name == "bg") {
            if (element.value.has_value() && element.value.value().length() == 1) {
                auto ch = element.value.value()[0];
                if (color_map.contains(ch)) {
                    result.bg_color = color_map.at(ch);
                    return result;
                }
            }
        }

        if (element.name == "b") {
            result.bold = true;
            return result;
        }

        if (element.name == "u") {
            result.underlined = true;
            return result;
        }

        if (element.name == "d") {
            result.dim = true;
            return result;
        }

        if (element.name == "blink") {
            result.blinking = true;
            return result;
        }

        return result;
    }

    auto format_text(const std::string_view& text) -> Text {
        Text output{};

        auto input = lexy::string_input<lexy::utf8_char_encoding>(text);
        auto document = lexy::parse<grammar::document>(input, lexy_ext::report_error);

        if (!document.has_value()) { return output; }

        std::stack<std::pair<Format, ast::Node>> formatting{};
        for (const auto& node : document.value().nodes) {
            formatting.emplace(Format{}, node);

            while (!formatting.empty()) {
                auto [fmt, curnode] = formatting.top();
                formatting.pop();

                if (std::holds_alternative<std::string>(curnode)) {
                    auto t = std::get<std::string>(curnode);
                    output.emplace_back(fmt, t);
                } else {
                    auto elem = std::get<ast::Element>(node);
                    if (elem.children.has_value()) {
                        Format child_format = update_formatting_from_element(fmt, elem);
                        for (const auto& child_node : elem.children.value()) {
                            formatting.emplace(child_format, child_node);
                        }
                    }
                }
            }
        }

        return output;
    }
} // namespace rglike