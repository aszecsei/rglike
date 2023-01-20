/**
 * @file resource.cpp
 * @author Alic Szecsei
 * @date 12/18/2022
 */

#include "fluent/resource.hpp"

#include <lexy/action/parse.hpp>
#include <lexy/action/trace.hpp>
#include <lexy/callback.hpp>
#include <lexy/dsl.hpp>
#include <lexy/input/file.hpp>
#include <lexy/input/string_input.hpp>
#include <lexy_ext/report_error.hpp>

namespace fluent {
    namespace dsl = lexy::dsl;

#pragma region Forward Declarations
    struct InlineExpression;
    struct Pattern;
    struct Selector;
#pragma endregion

#pragma region Whitespace
    // blank_inline ::= "\u0020"+
    static constexpr auto blank_inline = dsl::while_one(dsl::ascii::space);
    // blank_block ::= (blank_inline? line_end)+
    static constexpr auto blank_block = dsl::while_one(
        dsl::peek(dsl::while_(dsl::ascii::space) + dsl::newline) >>
        dsl::opt(blank_inline) + dsl::newline
    );
    // blank ::= (blank_inline | line_end)+
    static constexpr auto blank = dsl::while_one(blank_inline | dsl::newline);
#pragma endregion

#pragma region Numbers
    // digits ::= [0-9]+
    static constexpr auto digits = dsl::while_one(dsl::ascii::digit);
#pragma endregion

#pragma region String Literals

    /*
     * For special-purpose content, quoted string literals can be used where text
     * elements are not a good fit. String literals are delimited with double
     * quotes and may not contain line breaks. String literals use the backslash
     * (\) as the escape character. The literal double quote can be inserted via
     * the \" escape sequence. The literal backslash can be inserted with \\. The
     * literal opening brace ({) is allowed in string literals because they may not
     * comprise placeables.
     */
    struct StringLiteral : lexy::token_production {
        struct invalid_char {
            static LEXY_CONSTEVAL auto name() { return "invalid character in string literal"; }
        };

        static constexpr auto escaped_symbols = lexy::symbol_table<char> //
                                                    .map<'"'>('"')
                                                    .map<'\\'>('\\');
        static constexpr auto escape = dsl::backslash_escape //
                                           .symbol<escaped_symbols>()
                                           .rule(dsl::lit_c<'u'> >> dsl::code_point_id<4>)
                                           .rule(dsl::lit_c<'U'> >> dsl::code_point_id<6>);

        static constexpr auto rule = [] {
            auto code_point =
                (-dsl::unicode::control)
                    .error<invalid_char>; // NOLINT(readability-static-accessed-through-instance)

            return dsl::quoted.limit(dsl::ascii::newline)(code_point, escape);
        }();

        static constexpr auto value = lexy::as_string<ast::StringLiteral, lexy::utf8_encoding>;
    };

#pragma endregion

#pragma region Number Literals

    struct NumberLiteral : lexy::token_production {
        struct integer : lexy::transparent_production {
            static constexpr auto rule =
                dsl::opt(LEXY_LIT("-")) + dsl::integer<int64_t>(dsl::digits<>.no_leading_zero());
            static constexpr auto value = lexy::as_integer<std::int64_t>;
        };

        struct fraction : lexy::transparent_production {
            static constexpr auto rule = dsl::lit_c<'.'> >> dsl::capture(dsl::digits<>);
            static constexpr auto value = lexy::as_string<std::string>;
        };

        static constexpr auto rule = dsl::peek(dsl::opt(LEXY_LIT("-")) + dsl::digit<>) >>
                                     dsl::p<integer> + dsl::opt(dsl::p<fraction>);

        static constexpr auto value = [](int64_t integer, std::optional<std::string> fraction
                                      ) -> ast::NumberLiteral {
            if (!fraction.has_value()) { return integer; }
            auto result = static_cast<double>(integer);
            result += std::stod("0." + fraction.value());
            return result;
        };
    };

#pragma endregion

#pragma region Text Elements
    // special_text_char ::= "{" | "}"
    static constexpr auto special_text_char = LEXY_LITERAL_SET(LEXY_LIT("{"), LEXY_LIT("}"));
    // text_char ::= any_char - special_text_char - line_end
    static constexpr auto text_char =
        dsl::code_point - LEXY_LIT("{") - LEXY_LIT("}") - LEXY_LIT("\n");
    // indented_char ::= text_char - "[" - "*" - "."
    static constexpr auto indented_char = text_char - LEXY_LIT("[") - LEXY_LIT("*") - LEXY_LIT(".");
#pragma endregion

#pragma region Content Characters
    /*
     * Translation content can be written using any Unicode characters. However,
     * some characters are considered special depending on the type of content
     * they're in. See text_char and quoted_char for more information.
     *
     * Some Unicode characters, even if allowed, should be avoided in Fluent
     * resources. See spec/recommendations.md.
     */
    // TODO: FIXME
    // any_char ::= [\\u{0}-\\u{10FFFF}]
    static constexpr auto any_char = dsl::code_point;
#pragma endregion

#pragma region Identifier

    struct Identifier : lexy::token_production {
        static constexpr auto rule = [] {
            auto head = dsl::ascii::alpha_underscore;
            auto tail = dsl::ascii::alpha_digit_underscore / LEXY_LIT("-");
            return dsl::identifier(head, tail);
        }();
        static constexpr auto value = lexy::as_string<std::string, lexy::utf8_encoding>;
    };

#pragma endregion

#pragma region Block Expressions
    // VariantKey ::= "[" blank? (NumberLiteral | Identifier) blank? "]"
    struct VariantKey {
        static constexpr auto rule = dsl::square_bracketed.try_(
            dsl::opt(blank) + (dsl::p<NumberLiteral> | dsl::p<Identifier>)+dsl::opt(blank)
        );
    };

    // DefaultVariant ::= line_end blank? "*" VariantKey blank_inline? Pattern
    struct DefaultVariant {
        static constexpr auto rule =
            dsl::peek(dsl::newline + dsl::opt(blank) + LEXY_LIT("*")) >>
            (dsl::newline + dsl::opt(blank) + LEXY_LIT("*") + dsl::p<VariantKey> + dsl::opt(blank_inline) + dsl::recurse<Pattern>);
    };

    // Variant ::= line_end blank? VariantKey blank_inline? Pattern
    struct Variant {
        static constexpr auto rule =
            dsl::peek(dsl::newline + dsl::opt(blank) + LEXY_LIT("[")) >>
            (dsl::newline + dsl::opt(blank) + dsl::p<VariantKey> + dsl::opt(blank_inline) + dsl::recurse<Pattern>);
    };

    // variant_list ::= Variant* DefaultVariant Variant* line_end
    struct Variants {
        static constexpr auto rule = dsl::opt(dsl::list(dsl::p<Variant>));
    };

    struct SelectExpression {
        static constexpr auto rule = [] {
            auto prefix = dsl::p<Selector> + dsl::opt(blank) + LEXY_LIT("->");
            auto variant_list = dsl::p<Variants> + dsl::p<DefaultVariant> + dsl::p<Variants> + dsl::eol;
            return dsl::peek(prefix) >> (prefix + dsl::if_(blank_inline) + variant_list);
        };
    };

#pragma endregion

    struct pattern;
    struct inline_expression;

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
        static constexpr auto rule =
            dsl::square_bracketed.try_(dsl::p<identifier> | dsl::p<number_literal>);
    };

    struct variant {
        static constexpr auto rule = dsl::p<variant_key> + dsl::recurse<pattern>;
    };

    struct default_variant {
        static constexpr auto rule = dsl::lit_c<'*'> + dsl::p<variant>;
    };

    struct variant_list_member {
        static constexpr auto rule =
            (dsl::peek(LEXY_LIT("*")) >> dsl::p<default_variant>) | dsl::else_ >> dsl::p<variant>;
    };

    struct variant_list {
        static constexpr auto rule = dsl::list(
            dsl::peek(dsl::lit_c<'['> / dsl::lit_c<'*'>) >> dsl::p<variant_list_member>,
            dsl::trailing_sep(dsl::newline)
        );
    };

    struct select_expression {
        static constexpr auto rule =
            dsl::recurse<inline_expression> + LEXY_LIT("->") + dsl::p<variant_list>;
    };

    struct named_argument {
        static constexpr auto rule = dsl::p<identifier> >> dsl::lit_c<':'> + dsl::p<literal>;
    };

    struct argument {
        static constexpr auto rule =
            dsl::p<named_argument> | dsl::else_ >> dsl::recurse<inline_expression>;
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
        static constexpr auto rule =
            dsl::p<identifier> + (dsl::peek(LEXY_LIT("(")) >> dsl::p<call_arguments>);
    };

    struct msg_function_reference {
        static constexpr auto rule = dsl::p<identifier> + (dsl::peek(LEXY_LIT("(")) >> dsl::p <)
    };

    struct inline_placeable {
        static constexpr auto rule = LEXY_LIT("{") >>
                                     ((dsl::p<select_expression> |
                                       dsl::recurse<inline_expression>)+LEXY_LIT("}"));
    };

    struct inline_expression {
        static constexpr auto rule = dsl::p<string_literal> | dsl::p<number_literal> |
                                     dsl::p<term_reference> | dsl::p<variable_reference> |
                                     dsl::p<inline_placeable> | dsl::p<msg_function_reference>;
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

    struct message {
        static constexpr auto rule =
            dsl::p<identifier> + LEXY_LIT("=") +
            ((dsl::p<pattern> + dsl::list(dsl::p<attribute>) | dsl::while_one(dsl::p<attribute>)));
    };

    FluentResource::FluentResource(std::string source) {
        // TODO
    }
} // namespace fluent
