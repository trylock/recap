#ifndef RECAP_REASSIGNMENT_HPP_
#define RECAP_REASSIGNMENT_HPP_

#include <vector>
#include <string>
#include <algorithm>

#include "resistance.hpp"
#include "recipe.hpp"
#include "assignment.hpp"
#include "parallel_assignment_algorithm.hpp"
#include "cuda_assignment_algorithm.hpp"

namespace recap
{
    // Describes currently equipped item
    class equipment 
    {
    public:
        inline equipment() : is_craftable_(false), is_new_(false) {}

        inline equipment(recipe::slot_t slot, resistance crafted, resistance base, bool is_craftable, bool is_new) : 
            slot_(slot),
            craft_res_(crafted), 
            base_res_(base), 
            is_craftable_(is_craftable),
            is_new_(is_new)
        {}

        /** Crafted resistances
         * 
         * @returns crafted resistances
         */
        inline resistance crafted_resistances() const 
        {
            return craft_res_;
        }

        /** Base resistances of the item
         * 
         * @returns base resistances of the item
         */
        inline resistance base_resistances() const 
        {
            return base_res_;
        }

        /** All resistances provided by this item
         * 
         * @returns crafted and base resistances
         */
        inline resistance all_resistances() const 
        {
            return crafted_resistances() + base_resistances();
        }

        /** Check if you can craft this item.
         * 
         * @returns true iff you can craft resistances on this item
         */
        inline bool is_craftable() const 
        {
            return is_craftable_;
        }

        /** Check if this item is new
         * 
         * @returns true iff this item replaces an item in the same slot
         */
        inline bool is_new() const 
        {
            return is_new_;
        }

        /** Equipment slot of this item
         * 
         * @returns item slot
         */
        inline recipe::slot_t slot() const 
        {
            return slot_;
        }

    private:
        recipe::slot_t slot_;
        resistance craft_res_;
        resistance base_res_;
        bool is_craftable_;
        bool is_new_;
    };

    /** Find a way to reach @p max_resistances if we replace all old items in @p items 
     * 
     * @param current_resistances Current resistances
     * @param max_resistances Resistance threshold we're trying to reach
     * @param items List of all items
     * @param recipes Available crafting recipes
     * 
     * @returns Assignment of crafting recipes to items 
     */
    template<class AssignmentAlgorithm>
    assignment find_minimal_reassignment(
        resistance current_resistances, 
        resistance max_resistances, 
        const std::vector<equipment>& items,
        const std::vector<recipe>& recipes)
    {
        AssignmentAlgorithm algorithm;
        algorithm.initialize(max_resistances);
        
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
            auto assign = algorithm.run(req, slots, recipes);
            
            if (assign.cost() < min_assignment.cost())
            {
                min_assignment = assign;
            }
        }

        return min_assignment;
    }
}

#endif // RECAP_REASSIGNMENT_HPP_