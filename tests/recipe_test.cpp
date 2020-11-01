#include "catch_amalgamated.hpp"
#include "reassignment.hpp"

TEST_CASE("Serialize and deserialize slot", "[recipe]")
{
    using namespace recap;

    std::array<recipe::slot_t, 14> slots{
        recipe::SLOT_NONE,
        recipe::SLOT_ALL,
        recipe::SLOT_ARMOUR,
        recipe::SLOT_JEWELRY,
        recipe::SLOT_WEAPON1,
        recipe::SLOT_WEAPON2,
        recipe::SLOT_HELMET,
        recipe::SLOT_BODY,
        recipe::SLOT_GLOVES,
        recipe::SLOT_BOOTS,
        recipe::SLOT_BELT,
        recipe::SLOT_RING1,
        recipe::SLOT_RING2,
        recipe::SLOT_AMULET,
    };

    for (auto slot : slots)
    {
        auto slot_str = to_string(slot);
        auto slot_parsed = parse_slot(slot_str);
        REQUIRE(slot_parsed == slot);
    }
    REQUIRE(to_string(recipe::slot_t{ 17 }) == "<unknown>");
}