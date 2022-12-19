/**
 * @file resource.cpp
 * @author Alic Szecsei
 * @date 12/18/2022
 */

#include "fluent/resource.hpp"

#include <lexy/action/parse.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy_ext/report_error.hpp>

namespace fluent {
    namespace dsl = lexy::dsl;

    struct pattern;
    struct inline_expression;

    struct number_literal : lexy::token_production {
        struct integer : lexy::transparent_production {
            static constexpr auto rule =
                dsl::minus_sign + dsl::integer<int64_t>(dsl::digits<>.no_leading_zero());
            static constexpr auto value = lexy::as_integer<std::int64_t>;
        };

        struct fraction : lexy::transparent_production {
            static constexpr auto rule = dsl::lit_c<'.'> >> dsl::capture(dsl::digits<>);
            static constexpr auto value = lexy::as_string<std::string>;
        };

        static constexpr auto rule = dsl::peek(dsl::lit_c<'-'> / dsl::digit<>) >>
                                     dsl::p<integer> + dsl::opt(dsl::p<fraction>);

        static constexpr auto value = [](int64_t integer, std::optional<std::string> fraction
                                      ) -> ast::NumberLiteral {
            if (!fraction.has_value()) { return integer; }
            auto result = static_cast<double>(integer);
            result += std::stod("0." + fraction.value());
            return result;
        };
    };

    struct string_literal : lexy::token_production {
        struct invalid_char {
            static LEXY_CONSTEVAL auto name() { return "invalid character in string literal"; }
        };

        static constexpr auto escaped_symbols = lexy::symbol_table<char>
            .map<'"'>('"')
            .map<'\\'>('\\')
            .map<'/'>('/')
            .map<'b'>('\b')
            .map<'f'>('\f')
            .map<'n'>('\n')
            .map<'r'>('\r')
            .map<'t'>('\t');

        struct code_point_id {
            static constexpr auto rule =
                LEXY_LIT("u") >> dsl::code_unit_id<lexy::utf16_encoding, 4>;
            static constexpr auto value = lexy::construct<lexy::code_point>;
        };

        static constexpr auto rule = [] {
            auto code_point = (-dsl::unicode::control).error<invalid_char>;

            auto escape =
                dsl::backslash_escape.symbol<escaped_symbols>().rule(dsl::p<code_point_id>);

            return dsl::quoted.limit(dsl::ascii::newline)(code_point, escape);
        }();

        static constexpr auto value = lexy::as_string<ast::StringLiteral, lexy::utf8_encoding>;
    };

    struct inline_text : lexy::token_production {
        static constexpr auto rule = [] {
            auto special_text_char = LEXY_LIT("{") / LEXY_LIT("}");
            return dsl::loop((dsl::newline >> dsl::break_) | -special_text_char | dsl::break_);
        }();
    };

    struct block_text : lexy::token_production {
        static constexpr auto rule = [] {
            auto indented_char =
                -(LEXY_LIT("{") / LEXY_LIT("}") / LEXY_LIT("[") / LEXY_LIT("*") / LEXY_LIT("."));
            return dsl::while_one(dsl::newline) + dsl::while_one(dsl::ascii::space) +
                   indented_char + dsl::p<inline_text>;
        }();
    };

    struct literal {
        static constexpr auto rule = dsl::p<string_literal> | dsl::p<number_literal>;
        static constexpr auto value = lexy::construct<ast::Literal>;
    };

    struct identifier : lexy::token_production {
        static constexpr auto rule = [] {
            auto head = dsl::ascii::alpha_underscore;
            auto tail = dsl::ascii::alpha_digit_underscore / dsl::lit_c<'-'>;
            return dsl::peek(dsl::ascii::alpha_underscore) >> dsl::identifier(head, tail);
        }();
        static constexpr auto value = lexy::as_string<std::string>;
    };

    struct variant_key {
        static constexpr auto whitespace = dsl::whitespace(dsl::ascii::space);
        static constexpr auto rule = dsl::lit_c<'['> >>
                                     (dsl::p<identifier> | dsl::p<number_literal>)+dsl::lit_c<']'>;
    };

    struct default_variant {
        static constexpr auto rule = dsl::lit_c<'*'> >> dsl::p<variant_key> + dsl::recurse<pattern>;
    };

    struct variant {
        static constexpr auto rule = dsl::p<variant_key> >> dsl::recurse<pattern>;
    };

    struct variant_list {
        static constexpr auto rule = dsl::list(dsl::p<variant>, dsl::trailing_sep(dsl::newline)) +
                                     dsl::p<default_variant> +
                                     dsl::list(dsl::p<variant>, dsl::trailing_sep(dsl::newline));
    };

    struct select_expression {
        static constexpr auto rule =
            dsl::recurse<inline_expression> + LEXY_LIT("->") >> dsl::p<variant_list>;
    };

    struct named_argument {
        static constexpr auto rule = dsl::p<identifier> + dsl::lit_c<':'> + dsl::p<literal>;
    };

    struct argument {
        static constexpr auto rule = dsl::recurse<inline_expression> | dsl::p<named_argument>;
    };

    struct call_arguments {
        static constexpr auto rule =
            dsl::parenthesized.list(dsl::p<argument>, dsl::trailing_sep(dsl::lit_c<','>));
    };

    struct attribute_accessor {
        static constexpr auto rule = LEXY_LIT(".") >> dsl::p<identifier>;
    };

    struct variable_reference {
        static constexpr auto rule = LEXY_LIT("$") >> dsl::p<identifier>;
    };

    struct term_reference {
        static constexpr auto
            rule = LEXY_LIT("-") >>
                   dsl::p<identifier> +
                       dsl::opt(dsl::p<attribute_accessor>) + dsl::opt(dsl::p<call_arguments>);
    };

    struct message_reference {
        static constexpr auto rule = dsl::p<identifier> + dsl::opt(dsl::p<attribute_accessor>);
    };

    struct function_reference {
        static constexpr auto rule = dsl::p<identifier> + dsl::p<call_arguments>;
    };

    struct inline_placeable {
        static constexpr auto rule = LEXY_LIT("{") >>
                                     ((dsl::p<select_expression> |
                                       dsl::recurse<inline_expression>)+LEXY_LIT("}"));
    };

    struct inline_expression {
        static constexpr auto rule = dsl::p<string_literal> | dsl::p<number_literal> |
                                     dsl::p<function_reference> | dsl::p<message_reference> |
                                     dsl::p<term_reference> | dsl::p<variable_reference> |
                                     dsl::p<inline_placeable>;
    };

    struct comment_line {
        static constexpr auto rule = dsl::lit_c<'#'> + dsl::until(dsl::newline).or_eof();
    };

    struct attribute {
        static constexpr auto rule =
            dsl::newline + dsl::lit_c<'.'> + dsl::p<identifier> + dsl::lit_c<'='> + dsl::p<pattern>;
    };

    struct term {
        static constexpr auto rule = dsl::lit_c<'-'> + dsl::p<identifier> + dsl::lit_c<'='> +
                                     dsl::p<pattern> + dsl::list(dsl::p<attribute>);
    };

    FluentResource::FluentResource(std::string source) {
        // TODO
    }
} // namespace fluent