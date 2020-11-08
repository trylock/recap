#ifndef RECAP_ASSIGNMENT_HPP_
#define RECAP_ASSIGNMENT_HPP_

#include <vector>
#include <cassert>
#include <cstdint>
#include <array>

#include "recipe.hpp"
#include "resistance.hpp"

namespace recap
{
    class recipe_assignment
    {
    public:
        inline recipe_assignment() : slot_(recipe::SLOT_NONE) {}
        inline recipe_assignment(recipe::slot_t slot, recipe rec) : slot_(slot), recipe_(rec) {}

        /** What recipe is used in this slot.
         * 
         * @return recipe used in this slot
         */
        inline const recipe& used_recipe() const 
        {
            return recipe_;
        }

        /** In which slot is this recipe
         * 
         * @return equipment slot
         */
        inline recipe::slot_t slot() const 
        {
            return slot_;
        }

    private:
        recipe::slot_t slot_;
        recipe recipe_;
    };

    /** Assignment of recipes to equipment slots
     */
    class assignment
    {
    public:
        assignment() : cost_(recipe::MAX_COST) {}

        // Copyable
        assignment(const assignment&) = default;
        assignment& operator=(const assignment&) = default;

        // Movable
        assignment(assignment&&) = default;
        assignment& operator=(assignment&&) = default;

        /** Assignment of recipes to slots
         * 
         * @returns recipe assignment (maps slot to recipe)
         */
        inline std::vector<recipe_assignment>& assignments()
        {
            return assignments_;
        }

        /** Assignment of recipes to slots
         * 
         * @returns recipe assignment (maps slot to recipe)
         */
        inline const std::vector<recipe_assignment>& assignments() const
        {
            return assignments_;
        }

        /** Cost of this assignment (can be recipe::MAX_COST)
         * 
         * @returns cost of this assignment 
         */
        inline recipe::cost_t& cost()
        {
            return cost_;
        }

        /** Cost of this assignment (can be recipe::MAX_COST)
         * 
         * @returns cost of this assignment 
         */
        inline const recipe::cost_t& cost() const
        {
            return cost_;
        }

    private:
        // Used recipes
        std::vector<recipe_assignment> assignments_;
        // Actual cost (precomputed sum of recipe costs)
        recipe::cost_t cost_;
    };
}

#endif // RECAP_ASSIGNMENT_HPP_