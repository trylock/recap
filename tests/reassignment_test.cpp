#include "catch_amalgamated.hpp"
#include "reassignment.hpp"
#include "parallel_assignment_algorithm.hpp"
#include "cuda_assignment_algorithm.hpp"

void verify_reassignment(
    const std::vector<recap::equipment>& items, 
    const recap::assignment& result,
    recap::resistance current,
    recap::resistance req)
{
    using namespace recap;

    for (auto& item : items)
    {
        auto it = std::find_if(items.begin(), items.end(), [item](auto&& other_item) 
        {
            return other_item.is_new() && other_item.slot() == item.slot();
        });

        // this item has been replaced => update current resistances
        if (it != items.end())
        {
            current = current - item.all_resistances();
            current = current + it->all_resistances();
        }
    }

    std::uint64_t used_slots = recipe::SLOT_NONE;

    recipe::cost_t total_cost = 0;
    resistance res_after_application = current;
    for (auto assign : result.assignments())
    {
        res_after_application = res_after_application + assign.used_recipe().resistances();
        total_cost += assign.used_recipe().cost();

        // check that we don't assign to a slot twice
        REQUIRE((assign.slot() & used_slots) == 0);
        used_slots |= assign.slot();
    }

    REQUIRE(res_after_application >= req);
    REQUIRE(result.cost() == total_cost);
}

TEST_CASE("Replace an item with the same resistances", "[reassignment]")
{
    using namespace recap;

    std::vector<equipment> items{
        equipment{ recipe::SLOT_HELMET, resistance{ 10, 0, 0, 0 }, resistance{ 0, 10, 0, 0 }, true, false },
        equipment{ recipe::SLOT_BODY, resistance{ 0, 10, 0, 0 }, resistance{ 0, 0, 10, 0 }, true, false },
        equipment{ recipe::SLOT_GLOVES, resistance{ 0, 0, 10, 0 }, resistance{ 10, 0, 0, 0 }, true, false },
        equipment{ recipe::SLOT_GLOVES, resistance{ 0, 0, 0, 0 }, resistance{ 10, 0, 0, 0 }, true, true },
    };

    std::vector<recipe> recipes{
        recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
        recipe{ resistance{ 5, 5, 0, 0 }, 1, recipe::SLOT_ALL },
        recipe{ resistance{ 5, 0, 5, 0 }, 1, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 5, 5, 0 }, 1, recipe::SLOT_ALL },
        recipe{ resistance{ 10, 0, 0, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 10, 0, 0 }, 10, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 0, 10, 0 }, 10, recipe::SLOT_ALL },
    };

    resistance current{ 20, 20, 20, 0 };
    resistance req{ 20, 20, 20, 0 };

    auto result = find_minimal_reassignment<parallel_assignment_algorithm>(current, req, items, recipes);
    REQUIRE(result.cost() == 2);
    verify_reassignment(items, result, current, req);
}

TEST_CASE("No solution", "[reassignment]")
{
    using namespace recap;
    
    std::vector<equipment> items{
        equipment{ recipe::SLOT_HELMET, resistance{ 10, 0, 0, 0 }, resistance{ 0, 10, 0, 0 }, true, false },
        equipment{ recipe::SLOT_BODY, resistance{ 0, 10, 0, 0 }, resistance{ 0, 0, 10, 0 }, true, false },
        equipment{ recipe::SLOT_GLOVES, resistance{ 0, 0, 10, 0 }, resistance{ 10, 0, 0, 0 }, true, false },
        equipment{ recipe::SLOT_GLOVES, resistance{ 0, 0, 0, 0 }, resistance{ 0, 0, 0, 0 }, true, true },
    };

    std::vector<recipe> recipes{
        recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
        recipe{ resistance{ 4, 4, 0, 0 }, 1, recipe::SLOT_ALL },
        recipe{ resistance{ 4, 0, 4, 0 }, 1, recipe::SLOT_ALL },
        recipe{ resistance{ 0, 4, 4, 0 }, 1, recipe::SLOT_ALL },
    };

    resistance current{ 20, 20, 20, 0 };
    resistance req{ 20, 20, 20, 0 };

    auto result = find_minimal_reassignment<parallel_assignment_algorithm>(current, req, items, recipes);
    REQUIRE(result.cost() == recipe::MAX_COST);
    REQUIRE(result.assignments().size() == 0);
}