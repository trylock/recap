#include "catch_amalgamated.hpp"
#include "assignment.hpp"
#include "parallel_assignment_algorithm.hpp"
#include "cuda_assignment_algorithm.hpp"

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
    best.assignments().resize(slots.size());
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
                current.assignments()[j] = recipe_assignment{ slots[j], recipes[recipe_index] };
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

    REQUIRE(assign.assignments().size() <= slots.size());

    std::size_t used_slots = 0;
    recap::resistance total_res = recap::resistance::make_zero();
    recap::recipe::cost_t cost = 0;
    for (std::size_t i = 0; i < assign.assignments().size(); ++i)
    {
        const auto& item = assign.assignments()[i];
        cost += item.used_recipe().cost();
        total_res = total_res + item.used_recipe().resistances();

        REQUIRE((used_slots & item.slot()) == 0);
        used_slots |= item.slot();
    }

    // check that assignment doesn't lie about its cost
    REQUIRE(cost == assign.cost());
    REQUIRE(total_res >= req);
}

TEST_CASE("Assignment fails if there are no recipes", "[assignment]")
{
    using namespace recap;

    auto run_test = [](auto&& algorithm)
    {
        std::vector<recipe::slot_t> slots{
            recipe::SLOT_ARMOUR
        };

        std::vector<recipe> recipes{};

        auto result = algorithm.run(resistance::make_zero(), slots, recipes);
        REQUIRE(result.cost() == recipe::MAX_COST);
    };

    run_test(parallel_assignment_algorithm{});
#ifdef USE_CUDA
    run_test(cuda_assignment_algorithm{});
#endif // USE_CUDA
}

TEST_CASE("Find assignment if we have 0 requirements", "[assignment]")
{
    using namespace recap;

    auto run_test = [](auto&& algorithm)
    {
        std::vector<recipe::slot_t> slots{
            recipe::SLOT_BODY
        };

        std::vector<recipe> recipes{
            recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL }
        };

        auto result = algorithm.run(resistance::make_zero(), slots, recipes);
        REQUIRE(result.cost() == 0);
        REQUIRE(result.assignments().size() == 0);
    };

    run_test(parallel_assignment_algorithm{});
#ifdef USE_CUDA
    run_test(cuda_assignment_algorithm{});
#endif // USE_CUDA
}

TEST_CASE("No solution with 1 slot", "[assignment]")
{
    using namespace recap;

    auto run_test = [](auto&& algorithm)
    {
        std::vector<recipe::slot_t> slots{
            recipe::SLOT_ARMOUR
        };

        std::vector<recipe> recipes{
            recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
            recipe{ resistance{ 10, 0, 0, 0 }, 0, recipe::SLOT_ALL },
        };

        auto result = algorithm.run(resistance{ 11, 0, 0, 0 }, slots, recipes);
        REQUIRE(result.cost() == recipe::MAX_COST);
    };

    run_test(parallel_assignment_algorithm{});
    
#ifdef USE_CUDA
    run_test(cuda_assignment_algorithm{});
#endif // USE_CUDA
}

TEST_CASE("Only use recipes aplicable to a given slot", "[assignment]")
{
    using namespace recap;

    auto run_test = [](auto&& algorithm)
    {
        std::vector<recipe::slot_t> slots{
            recipe::SLOT_BODY
        };

        std::vector<recipe> recipes{
            recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
            recipe{ resistance{ 10, 0, 0, 0 }, 0, recipe::SLOT_JEWELRY },
        };

        auto result = algorithm.run(resistance{ 5, 0, 0, 0 }, slots, recipes);
        REQUIRE(result.cost() == recipe::MAX_COST);
        REQUIRE(result.assignments().size() == 0);
    };

    run_test(parallel_assignment_algorithm{});
    
#ifdef USE_CUDA
    run_test(cuda_assignment_algorithm{});
#endif // USE_CUDA
}

TEST_CASE("Returned assignment is optimal", "[assignment]")
{
    using namespace recap;

    auto run_test = [](auto&& algorithm)
    {
        std::vector<recipe::slot_t> slots{
            recipe::SLOT_BODY,
            recipe::SLOT_WEAPON1,
            recipe::SLOT_BOOTS,
            recipe::SLOT_GLOVES
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

        auto result_dynamic = algorithm.run(req, slots, recipes);
        verify_assignment(req, slots, result_dynamic);

        auto result_bf = find_assignment_bf(req, slots, recipes);
        verify_assignment(req, slots, result_bf);
        
        REQUIRE(result_dynamic.cost() == result_bf.cost());
    };

    run_test(parallel_assignment_algorithm{});
    
#ifdef USE_CUDA
    run_test(cuda_assignment_algorithm{});
#endif // USE_CUDA
}

TEST_CASE("Different types of slots", "[assignment]")
{
    using namespace recap;

    auto run_test = [](auto&& algorithm)
    {
        std::vector<recipe::slot_t> slots{
            recipe::SLOT_BODY,
            recipe::SLOT_HELMET,
            recipe::SLOT_RING1,
            recipe::SLOT_AMULET
        };

        std::vector<recipe> recipes{
            recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL },
            recipe{ resistance{ 30, 0, 0, 0 }, 30, recipe::SLOT_ALL },
            recipe{ resistance{ 0, 30, 0, 0 }, 30, recipe::SLOT_ALL },
            recipe{ resistance{ 0, 0, 30, 0 }, 30, recipe::SLOT_ALL },
            recipe{ resistance{ 20, 20, 0, 0 }, 10, recipe::SLOT_ALL },
            recipe{ resistance{ 20, 0, 20, 0 }, 10, recipe::SLOT_ALL },
            recipe{ resistance{ 0, 20, 20, 0 }, 10, recipe::SLOT_ALL },
            recipe{ resistance{ 10, 10, 10, 0 }, 9, recipe::SLOT_JEWELRY },
            recipe{ resistance{ 15, 0, 0, 15 }, 30, recipe::SLOT_JEWELRY },
            recipe{ resistance{ 0, 15, 0, 15 }, 30, recipe::SLOT_JEWELRY },
            recipe{ resistance{ 0, 0, 15, 15 }, 30, recipe::SLOT_JEWELRY },
        };
        resistance req{ 29, 37, 23, 17 };

        auto result_dynamic = algorithm.run(req, slots, recipes);
        verify_assignment(req, slots, result_dynamic);

        auto result_bf = find_assignment_bf(req, slots, recipes);
        verify_assignment(req, slots, result_bf);
        
        REQUIRE(result_dynamic.cost() == result_bf.cost());
    };

    run_test(parallel_assignment_algorithm{});
#ifdef USE_CUDA
    run_test(cuda_assignment_algorithm{});
#endif // USE_CUDA
}

TEST_CASE("Exhaustive test", "[assignment][.][slow]")
{
    using namespace recap;

    constexpr resistance::item_t MAX_VALUE = 30;
    constexpr resistance::item_t MAX_CHAOS = 2;

    auto run_test = [](auto&& algorithm)
    {
        std::vector<recipe::slot_t> slots{
            recipe::SLOT_BODY,
            recipe::SLOT_HELMET,
            recipe::SLOT_GLOVES,
            recipe::SLOT_BOOTS
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

        algorithm.initialize(resistance{ MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_CHAOS });

        for (resistance::item_t fire = 0; fire <= MAX_VALUE; ++fire)
        {
            for (resistance::item_t cold = 0; cold <= MAX_VALUE; ++cold)
            {
                for (resistance::item_t lightning = 0; lightning <= MAX_VALUE; ++lightning)
                {
                    for (resistance::item_t chaos = 0; chaos <= MAX_CHAOS; ++chaos)
                    {
                        resistance req{ fire, cold, lightning, chaos };

                        auto result_dynamic = algorithm.run(req, slots, recipes);
                        verify_assignment(req, slots, result_dynamic);

                        auto result_bf = find_assignment_bf(req, slots, recipes);
                        verify_assignment(req, slots, result_bf);
                        
                        REQUIRE(result_dynamic.cost() == result_bf.cost());
                    }
                }
            }
        }
    };
    run_test(parallel_assignment_algorithm{});
#ifdef USE_CUDA
    run_test(cuda_assignment_algorithm{});
#endif // USE_CUDA
}