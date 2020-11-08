#ifndef RECAP_ASSIGNMENT_ALGORITHM_HPP_
#define RECAP_ASSIGNMENT_ALGORITHM_HPP_

#include "recipe.hpp"
#include "resistance.hpp"
#include "assignment.hpp"
#include "equipment.hpp"

namespace recap 
{
    /** Assignment algorithm implementation
     */
    class assignment_algorithm
    {
    public:
        virtual ~assignment_algorithm() {}

        /** Identifier of this algorithms
         * 
         * @returns name of this algorithm
         */
        virtual const char* name() const = 0;

        /** Allocate memory for problem instances
         * 
         * @param max_resistances Maximal number of resistances
         * @param max_recipes Maximal number of recipes
         */
        virtual void initialize(resistance max_resistances, std::size_t max_recipes) = 0;

        /** Find assignment of @p recipes to equipment @p slots which minimizes cost and 
         * has at least @p required resistances.
         * 
         * @param required Required resistances 
         * @param slots Free equipment slots where we can apply recipes
         * @param recipes Available recipes
         * 
         * @return assignment of recipes to slots or invalid assingment object if assignment is not possible.
         */
        virtual assignment find_minimal_assignment(
            resistance required, 
            const std::vector<recipe::slot_t>& slots, 
            const std::vector<recipe>& recipes) = 0;

        /** Find a way to reach @p max_resistances if we replace all old items in @p items 
         * 
         * @param current_resistances Current resistances
         * @param max_resistances Resistance threshold we're trying to reach
         * @param items List of all items
         * @param recipes Available crafting recipes
         * 
         * @returns Assignment of crafting recipes to items 
         */
        virtual assignment find_minimal_reassignment(
            resistance current_resistances, 
            resistance max_resistances, 
            const std::vector<equipment>& items,
            const std::vector<recipe>& recipes);

        /** Count number of distinct values <= res
         * 
         * @param res Resistances
         * 
         * @returns number of distinct resistance values <= @p res
         */
        inline static std::size_t count_values(resistance res) 
        {
            std::size_t value_count = res.fire() + 1;
            value_count *= res.cold() + 1;
            value_count *= res.lightning() + 1;
            value_count *= res.chaos() + 1;
            return value_count;
        }
    };
}

#endif // RECAP_ASSIGNMENT_ALGORITHM_HPP_