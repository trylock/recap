#include "assignment.hpp"

template<typename IndexType>
struct min_cost_assign
{
    using index_t = IndexType;

    // Maximal number of equipment slots 
    inline static constexpr std::size_t MAX_SLOT_COUNT = 10;

    // indices of recipes used in this assignment
    std::array<index_t, MAX_SLOT_COUNT> recipes;
    // total cost of this assignment
    recap::recipe::cost_t cost;
};

recap::assignment recap::find_assignment(
    resistance required, 
    const std::vector<recipe::slot_t>& slots, 
    const std::vector<recipe>& recipes)
{
    using min_cost_assign_t = min_cost_assign<std::uint8_t>;

    // Check that we can fit all recipes into index type
    if (recipes.size() > std::numeric_limits<min_cost_assign_t::index_t>::max())
    {
        throw std::runtime_error{ "Recipes won't fit into used index type." };
    }

    // Check number of slots
    if (slots.size() > min_cost_assign_t::MAX_SLOT_COUNT)
    {
        throw std::runtime_error{ 
            "Internal type has memory only for " + 
            std::to_string(min_cost_assign_t::MAX_SLOT_COUNT) +  " slots." };
    }

    // Count number of distinct resistance values <= required
    const auto res_count = resistance{ 
        static_cast<resistance::item_t>(required.fire() + 1), 
        static_cast<resistance::item_t>(required.cold() + 1), 
        static_cast<resistance::item_t>(required.lightning() + 1), 
        static_cast<resistance::item_t>(required.chaos() + 1) 
    };
    const auto value_count = static_cast<std::size_t>(res_count.fire()) * res_count.cold() * 
        res_count.lightning() * res_count.chaos();

    // initialize table with MAX_COSTs
    min_cost_assign_t unsatisfiable;
    unsatisfiable.cost = recipe::MAX_COST;

    // allocate memory for 2 tables
    std::vector<min_cost_assign_t> best_cost(value_count);
    std::fill(best_cost.begin(), best_cost.end(), unsatisfiable);
    std::vector<min_cost_assign_t> next_best_cost = best_cost;

    // Convert resistance object to a linear index.
    // This is a one-to-one mapping from resistances < res_count to [0, value_count - 1]
    auto to_index = [res_count](resistance res)
    {
        std::size_t index = res.fire();
        index = index * res_count.cold() + res.cold();
        index = index * res_count.lightning() + res.lightning();
        index = index * res_count.chaos() + res.chaos();
        return index;
    };

    // we can always satisfy the requirement of 0 resistances
    best_cost[0].cost = 0;

    for (std::size_t i = 0; i < slots.size(); ++i)
    {
        // compute next best costs (with 1 more item)
        tbb::blocked_range3d<resistance::item_t> range(
            0, res_count.fire(), 
            0, res_count.cold(), 
            0, res_count.lightning());
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
                            current_node.cost = recipe::MAX_COST;

                            // try all recipes for current resistance
                            for (std::size_t recipe_index = 0; recipe_index < recipes.size(); ++recipe_index)
                            {
                                const auto& recipe = recipes[recipe_index];

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
                                if (prev_node.cost + recipe.cost() < current_node.cost)
                                {
                                    // replace the recipe
                                    for (std::size_t j = 0; j < i; ++j)
                                    {
                                        current_node.recipes[j] = prev_node.recipes[j];
                                    }
                                    current_node.recipes[i] = static_cast<min_cost_assign_t::index_t>(recipe_index);

                                    // update the cost
                                    current_node.cost = prev_node.cost + recipe.cost();
                                }
                            }
                        }
                    }
                }
            }
        });

        std::swap(next_best_cost, best_cost);
    }

    // lookup the solution in the table
    const auto& best_node = best_cost[to_index(required)];
    
    // convert it to the output type
    assignment result;
    result.cost() = best_node.cost;

    if (result.cost() != recipe::MAX_COST)
    {
        result.recipes().resize(slots.size());
        for (std::size_t i = 0; i < slots.size(); ++i)
        {
            result.recipes()[i] = recipes[best_node.recipes[i]];
        }
    }
    
    return result;
}