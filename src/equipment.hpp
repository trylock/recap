#ifndef RECAP_EQUIPMENT_HPP_
#define RECAP_EQUIPMENT_HPP_

#include <vector>
#include <string>
#include <algorithm>

#include "resistance.hpp"
#include "recipe.hpp"
#include "assignment.hpp"

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
}

#endif // RECAP_EQUIPMENT_HPP_