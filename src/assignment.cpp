#include "assignment.hpp"

recap::assignment recap::find_assignment(
    resistance required, 
    const std::vector<recipe::slot_t>& slots, 
    const std::vector<recipe>& recipes)
{
    static_assert(std::numeric_limits<recipe::cost_t>::has_infinity, "Cost type has to support infinity");

    // Count number of 4-tuples of resistances where no value is higher then the required 
    // resistance fo that type.
    auto max_res = resistance{ 
        static_cast<resistance::item_t>(required.fire() + 1), 
        static_cast<resistance::item_t>(required.cold() + 1), 
        static_cast<resistance::item_t>(required.lightning() + 1), 
        static_cast<resistance::item_t>(required.chaos() + 1) 
    };
    auto resistance_count = static_cast<std::size_t>(max_res.fire()) * max_res.cold() * 
        max_res.lightning() * max_res.chaos();

    assignment invalid_node;
    invalid_node.cost() = recipe::MAX_COST;
    invalid_node.recipes().resize(slots.size());

    // allocate memory for the table
    std::vector<assignment> best_cost(resistance_count);
    std::fill(best_cost.begin(), best_cost.end(), invalid_node);

    // copy the table so that we can parallelize each iteration
    std::vector<assignment> next_best_cost = best_cost;

    // Convert resistance object to a linear index.
    // This is a one-to-one mapping from resistances < max_res to [0, resistance_count - 1]
    auto to_index = [max_res](resistance res)
    {
        std::size_t index = res.fire();
        index = index * max_res.cold() + res.cold();
        index = index * max_res.lightning() + res.lightning();
        index = index * max_res.chaos() + res.chaos();
        return index;
    };

    // we can always satisfy the requirement of 0 resistances
    best_cost[0].cost() = 0;

    for (std::size_t i = 0; i < slots.size(); ++i)
    {
        // compute next best costs (with 1 more item)
        tbb::blocked_range3d<resistance::item_t> range(
            0, max_res.fire(), 
            0, max_res.cold(), 
            0, max_res.lightning());
        tbb::parallel_for(range, [&](auto& local_range) 
        {
            for (resistance::item_t fire = local_range.pages().begin(); fire != local_range.pages().end(); ++fire)
            {
                for (resistance::item_t cold = local_range.rows().begin(); cold != local_range.rows().end(); ++cold)
                {
                    for (resistance::item_t lightning = local_range.cols().begin(); lightning != local_range.cols().end(); ++lightning)
                    {
                        for (resistance::item_t chaos = 0; chaos <= required.chaos(); ++chaos)
                        {
                            // construct resistance object from the values and find its index
                            resistance current_resist{ fire, cold, lightning, chaos };
                            auto current_index = to_index(current_resist);
                            auto& current_node = next_best_cost[current_index];
                            current_node.cost() = recipe::MAX_COST;

                            // try all recipes for current resistance
                            for (auto& recipe : recipes)
                            {
                                // if this recipe is not aplicable for slot i
                                if ((recipe.slots() & slots[i]) == 0)
                                {
                                    continue; // skip this recipe
                                }

                                // find required resistances if we use this recipe
                                resistance prev_resist = current_resist - recipe.resistances();
                                auto prev_index = to_index(prev_resist);
                                const auto& prev_node = best_cost[prev_index];

                                // if this path is better
                                if (prev_node.cost() + recipe.cost() < current_node.cost())
                                {
                                    // replace the recipe
                                    for (std::size_t j = 0; j < i; ++j)
                                    {
                                        current_node.recipes()[j] = prev_node.recipes()[j];
                                    }
                                    current_node.recipes()[i] = recipe;

                                    // update the cost
                                    current_node.cost() = prev_node.cost() + recipe.cost();
                                }
                            }
                        }
                    }
                }
            }
        });
        std::swap(next_best_cost, best_cost);
    }
    
    return std::move(best_cost[to_index(required)]);
}