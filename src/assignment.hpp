#ifndef RECAP_ASSIGNMENT_HPP_
#define RECAP_ASSIGNMENT_HPP_

#include <vector>
#include <cassert>
#include <cstdint>
#include <array>

#include <tbb/partitioner.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range3d.h>
#define TBB_PREVIEW_BLOCKED_RANGE_ND 1
#include <tbb/blocked_rangeNd.h>

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

    /** Dynamic programming algorithm which uses TBB to parallelize the computation.
     */
    class parallel_assignment_algorithm 
    {
    public:
        // maximal number of equipment slots
        inline static constexpr std::size_t MAX_SLOT_COUNT = 16;

        // Type used to index recipes during computation
        using recipe_index_t = std::uint8_t;
        // Recipe cost type
        using cost_t = recipe::cost_t;
        // Type used internally to store assignment
        using internal_assignment_t = std::array<recipe_index_t, MAX_SLOT_COUNT>;

        parallel_assignment_algorithm();

        // Non-copyable
        parallel_assignment_algorithm(const parallel_assignment_algorithm&) = delete;
        parallel_assignment_algorithm& operator=(const parallel_assignment_algorithm&) = delete;

        // Movable
        parallel_assignment_algorithm(parallel_assignment_algorithm&&) = default;
        parallel_assignment_algorithm& operator=(parallel_assignment_algorithm&&) = default;

        /** Allocate memory for the algorithm
         * 
         * @param max_res Maximal used resistances
         */
        void initialize(resistance max_res);

        /** Find assignment of @p recipes to equipment @p slots which minimizes cost and 
         * has at least @p required resistances.
         * 
         * @param required Required resistances 
         * @param slots Free equipment slots where we can apply recipes
         * @param recipes Available recipes
         * 
         * @return assignment of recipes to slots or invalid assingment object if assignment is not possible.
         */
        assignment run(
            resistance required, 
            const std::vector<recipe::slot_t>& slots, 
            const std::vector<recipe>& recipes);

    private:
        resistance max_res_;
        std::vector<cost_t> best_cost_;
        std::vector<cost_t> next_best_cost_;
        std::vector<internal_assignment_t> best_assignment_;
        std::vector<internal_assignment_t> next_best_assignment_;

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

#endif // RECAP_ASSIGNMENT_HPP_