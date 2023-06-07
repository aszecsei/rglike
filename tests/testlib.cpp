#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <rglike/formatting.hpp>

TEST_CASE("Quick check", "[main]") { REQUIRE(true == true); }

TEST_CASE("Strip formatting", "[formatting]") {
    auto txt = rglike::format_text("Hell[b]o[/b], w[i][/i]orld!");
    auto stripped = txt.plain_text();
    REQUIRE(stripped == "Hello, world!");
}