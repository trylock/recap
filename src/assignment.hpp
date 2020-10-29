#ifndef RECAP_ASSIGNMENT_HPP_
#define RECAP_ASSIGNMENT_HPP_

#include <vector>
#include <cassert>
#include <cstdint>
#include <array>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range3d.h>

#include "recipe.hpp"
#include "resistance.hpp"

namespace recap
{
    /** Assignment of recipes to equipment slots
     */
    class assignment
    {
    public:
        assignment() = default;

        // Copyable
        assignment(const assignment&) = default;
        assignment& operator=(const assignment&) = default;

        // Movable
        assignment(assignment&&) = default;
        assignment& operator=(assignment&&) = default;

        /** Assignment of recipes to slots
         * 
         * @returns recipe assignment (maps slot index to recipe)
         */
        inline std::vector<recipe>& recipes()
        {
            return recipes_;
        }

        /** Assignment of recipes to slots
         * 
         * @returns recipe assignment (maps slot index to recipe)
         */
        inline const std::vector<recipe>& recipes() const
        {
            return recipes_;
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
        std::vector<recipe> recipes_;
        // Actual cost (precomputed sum of recipe costs)
        recipe::cost_t cost_;
    };

    /** Find assignment of @p recipes to equipment @p slots which minimizes cost and 
     * has at least @p required resistances.
     * 
     * @param required Required resistances 
     * @param slots Free equipment slots where we can apply recipes
     * @param recipes Available recipes
     * 
     * @return assignment of recipes to slots or invalid assingment object if assignment is not possible.
     */
    assignment find_assignment(
        resistance required, 
        const std::vector<recipe::slot_t>& slots, 
        const std::vector<recipe>& recipes);
}

#endif // RECAP_ASSIGNMENT_HPP_