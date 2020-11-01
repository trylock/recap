#include "reassignment.hpp"

recap::assignment recap::find_minimal_reassignment(
    resistance current_resistances, 
    resistance max_resistances, 
    const std::vector<equipment>& items,
    const std::vector<recipe>& recipes)
{
    // construct list of only the new items
    std::vector<equipment> new_items;
    new_items.reserve(items.size());

    resistance new_resistances = current_resistances;
    for (auto& item : items)
    {
        if (!item.is_new())
        {
            auto item_slot = item.slot();
            auto it = std::find_if(items.begin(), items.end(), [item_slot](auto&& other_item) 
            {
                return other_item.is_new() && other_item.slot() == item_slot;
            });

            // if this item has been replaced
            if (it != items.end())
            {
                new_resistances = new_resistances - item.all_resistances(); 
                new_resistances = new_resistances + it->all_resistances();
            }
            else // item hasn't been replaced
            {
                new_items.push_back(item);
            }
        }
        else  
        {
            new_items.push_back(item);
        }
    }

    // find the new requirements
    resistance new_req_resistances = max_resistances - new_resistances;

    // if the requirements are trivially satisfied, end here
    if (new_req_resistances <= resistance::make_zero())
    {
        recap::assignment result;
        result.cost() = 0;
        return result;
    }

    // try all subsets of new items
    std::size_t subset_count = 1 << new_items.size();
    std::vector<recipe::slot_t> slots;
    slots.reserve(new_items.size());

    // remember minimal cost assignment and which slots we've used
    assignment min_assignment;
    min_assignment.cost() = recipe::MAX_COST;

    for (std::size_t i = 1; i < subset_count; ++i)
    {
        resistance req = new_req_resistances;

        // gather item slots from this subset to an array
        slots.clear();
        for (std::size_t j = 0; j < new_items.size(); ++j)
        {
            if ((i & (1 << j)) != 0) 
            {
                slots.push_back(new_items[j].slot());

                // We're gonna potentially change crafted recipe this item so we have to 
                // assume resistances crafted on it won't be available.
                req = req + new_items[j].crafted_resistances();
            }
        }

        // find minimal cost assignment using current subset of items
        auto assign = find_assignment(req, slots, recipes);
        if (assign.cost() < min_assignment.cost())
        {
            min_assignment = assign;
        }
    }

    return min_assignment;
}