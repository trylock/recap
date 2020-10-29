#include "catch_amalgamated.hpp"
#include "assignment.hpp"

// Brute force solution
static recap::assignment find_assignment_bf(
    recap::resistance req, 
    const std::vector<recap::recipe::slot_t>& slots,
    const std::vector<recap::recipe>& recipes)
{
    using namespace recap;

    std::size_t option_count = 1;
    for (auto& slot : slots)
    {
        option_count *= recipes.size();
        (void)slot;
    }

    assignment best;
    best.cost() = recipe::MAX_COST;
    best.recipes().resize(slots.size());
    assignment current = best;

    for (std::size_t i = 0; i < option_count; ++i)
    {
        std::size_t value = i;
        current.cost() = 0;
        recap::resistance current_res = recap::resistance::make_zero();

        for (std::size_t j = 0; j < slots.size(); ++j)
        {
            auto recipe_index = value % recipes.size();

            // check if this recipe is aplicable to the slot j
            if ((recipes[recipe_index].slots() & slots[j]) == 0)
            {
                current.cost() = recipe::MAX_COST;
                break;
            }
            else // recipe is aplicable 
            {
                current.cost() += recipes[recipe_index].cost();
                current.recipes()[j] = recipes[recipe_index];
                current_res = current_res + recipes[recipe_index].resistances();
            }

            value /= recipes.size();
        }

        if (current.cost() < best.cost() && current_res >= req)
        {
            std::swap(current, best);
        }
    }
    return best;
}

// Verify assignment
static void verify_assignment(
    recap::resistance req, 
    const std::vector<recap::recipe::slot_t>& slots, 
    const recap::assignment& assign)
{
    if (assign.cost() >= recap::recipe::MAX_COST)
    {
        return; // no solution is a valid answer
    }

    REQUIRE(slots.size() == assign.recipes().size());

    recap::resistance total_res = recap::resistance::make_zero();
    recap::recipe::cost_t cost = 0;
    for (std::size_t i = 0; i < assign.recipes().size(); ++i)
    {
        const auto& recipe = assign.recipes()[i];
        cost += recipe.cost();
        total_res = total_res + recipe.resistances();

        REQUIRE((recipe.slots() & slots[i]) != 0);
    }

    // check that assignment doesn't lie about its cost
    REQUIRE(cost == assign.cost());
    REQUIRE(total_res >= req);
}

TEST_CASE("Assignment fails if there are no recipes", "[assignment]")
{
    using namespace recap;

    std::vector<recipe::slot_t> slots{
        recipe::SLOT_ARMOUR
    };

    std::vector<recipe> recipes{};

    auto result = find_assignment(resistance::make_zero(), slots, recipes);
    REQUIRE(result.cost() == recipe::MAX_COST);
}

TEST_CASE("Find assignment if we have 0 requirements", "[assignment]")
{
    using namespace recap;

    std::vector<recipe::slot_t> slots{
        recipe::SLOT_ARMOUR
    };

    std::vector<recipe> recipes{
        recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL }
    };

    auto result = find_assignment(resistance::make_zero(), slots, recipes);
    REQUIRE(result.cost() == 0);
    REQUIRE(result.recipes()[0].resistances() == resistance::make_zero());
}

TEST_CASE("No solution with 1 slot", "[assignment]")
{
    using namespace recap;

    std::vector<recipe::slot_t> slots{
        recipe::SLOT_ARMOUR
    };

    std::vector<recipe> recipes{
        recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
        recipe{ resistance{ 10, 0, 0, 0 }, 0, recipe::SLOT_ALL },
    };

    auto result = find_assignment(resistance{ 11, 0, 0, 0 }, slots, recipes);
    REQUIRE(result.cost() == recipe::MAX_COST);
}

TEST_CASE("Only use recipes aplicable to a given slot", "[assignment]")
{
    using namespace recap;

    std::vector<recipe::slot_t> slots{
        recipe::SLOT_ARMOUR
    };

    std::vector<recipe> recipes{
        recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
        recipe{ resistance{ 10, 0, 0, 0 }, 0, recipe::SLOT_JEWELERY },
    };

    auto result = find_assignment(resistance{ 5, 0, 0, 0 }, slots, recipes);
    REQUIRE(result.cost() == recipe::MAX_COST);
}

TEST_CASE("Returned assignment is optimal", "[assignment]")
{
    using namespace recap;

    std::vector<recipe::slot_t> slots{
        recipe::SLOT_ARMOUR,
        recipe::SLOT_ARMOUR,
        recipe::SLOT_ARMOUR,
        recipe::SLOT_ARMOUR
    };

    std::vector<recipe> recipes{
        recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
        recipe{ resistance{ 30, 0, 0, 0 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 30, 0, 0 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 0, 30, 0 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 20, 20, 0, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 20, 0, 20, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 20, 20, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 10, 10, 10, 0 }, 9, recipe::SLOT_ALL },
        recipe{ resistance{ 15, 0, 0, 15 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 15, 0, 15 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 0, 15, 15 }, 30, recipe::SLOT_ALL },
    };
    resistance req{ 29, 37, 23, 17 };

    auto result_dynamic = find_assignment(req, slots, recipes);
    verify_assignment(req, slots, result_dynamic);

    auto result_bf = find_assignment_bf(req, slots, recipes);
    verify_assignment(req, slots, result_bf);
    
    REQUIRE(result_dynamic.cost() == result_bf.cost());
}

TEST_CASE("Different types of slots", "[assignment]")
{
    using namespace recap;

    std::vector<recipe::slot_t> slots{
        recipe::SLOT_ARMOUR,
        recipe::SLOT_ARMOUR,
        recipe::SLOT_JEWELERY,
        recipe::SLOT_JEWELERY
    };

    std::vector<recipe> recipes{
        recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
        recipe{ resistance{ 30, 0, 0, 0 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 30, 0, 0 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 0, 30, 0 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 20, 20, 0, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 20, 0, 20, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 20, 20, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 10, 10, 10, 0 }, 9, recipe::SLOT_JEWELERY },
        recipe{ resistance{ 15, 0, 0, 15 }, 30, recipe::SLOT_JEWELERY },
        recipe{ resistance{ 0, 15, 0, 15 }, 30, recipe::SLOT_JEWELERY },
        recipe{ resistance{ 0, 0, 15, 15 }, 30, recipe::SLOT_JEWELERY },
    };
    resistance req{ 29, 37, 23, 17 };

    auto result_dynamic = find_assignment(req, slots, recipes);
    verify_assignment(req, slots, result_dynamic);

    auto result_bf = find_assignment_bf(req, slots, recipes);
    verify_assignment(req, slots, result_bf);
    
    REQUIRE(result_dynamic.cost() == result_bf.cost());
}

TEST_CASE("Exhaustive test", "[assignment][.][slow]")
{
    using namespace recap;

    std::vector<recipe::slot_t> slots{
        recipe::SLOT_ARMOUR,
        recipe::SLOT_ARMOUR,
        recipe::SLOT_ARMOUR,
        recipe::SLOT_ARMOUR
    };

    std::vector<recipe> recipes{
        recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
        recipe{ resistance{ 30, 0, 0, 0 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 30, 0, 0 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 0, 30, 0 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 20, 20, 0, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 20, 0, 20, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 20, 20, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 10, 10, 10, 0 }, 9, recipe::SLOT_ALL },
        recipe{ resistance{ 15, 0, 0, 15 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 15, 0, 15 }, 30, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 0, 15, 15 }, 30, recipe::SLOT_ALL },
    };

    constexpr resistance::item_t MAX_VALUE = 30;
    constexpr resistance::item_t MAX_CHAOS = 2;

    for (resistance::item_t fire = 0; fire <= MAX_VALUE; ++fire)
    {
        for (resistance::item_t cold = 0; cold <= MAX_VALUE; ++cold)
        {
            for (resistance::item_t lightning = 0; lightning <= MAX_VALUE; ++lightning)
            {
                for (resistance::item_t chaos = 0; chaos <= MAX_CHAOS; ++chaos)
                {
                    resistance req{ fire, cold, lightning, chaos };

                    auto result_dynamic = find_assignment(req, slots, recipes);
                    verify_assignment(req, slots, result_dynamic);

                    auto result_bf = find_assignment_bf(req, slots, recipes);
                    verify_assignment(req, slots, result_bf);
                    
                    REQUIRE(result_dynamic.cost() == result_bf.cost());
                }
            }
        }
    }
}