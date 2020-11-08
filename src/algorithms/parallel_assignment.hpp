#ifndef RECAP_PARALLEL_ASSIGNMENT_HPP_
#define RECAP_PARALLEL_ASSIGNMENT_HPP_

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
#include "assignment.hpp"
#include "assignment_algorithm.hpp"

namespace recap
{
    /** Dynamic programming algorithm which uses TBB to parallelize the computation.
     */
    class parallel_assignment : public assignment_algorithm
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

        parallel_assignment();

        virtual ~parallel_assignment() {}

        // Non-copyable
        parallel_assignment(const parallel_assignment&) = delete;
        parallel_assignment& operator=(const parallel_assignment&) = delete;

        // Movable
        parallel_assignment(parallel_assignment&&) = default;
        parallel_assignment& operator=(parallel_assignment&&) = default;

        /** Identifier of this algorithms
         * 
         * @returns name of this algorithm
         */
        const char* name() const override;

        /** Allocate memory for problem instances
         * 
         * @param max_resistances Maximal number of resistances
         * @param max_recipes Maximal number of recipes
         */
        void initialize(resistance max_resistances, std::size_t max_recipes) override;

        /** Find assignment of @p recipes to equipment @p slots which minimizes cost and 
         * has at least @p required resistances.
         * 
         * @param required Required resistances 
         * @param slots Free equipment slots where we can apply recipes
         * @param recipes Available recipes
         * 
         * @return assignment of recipes to slots or invalid assingment object if assignment is not possible.
         */
        assignment find_minimal_assignment(
            resistance required, 
            const std::vector<recipe::slot_t>& slots, 
            const std::vector<recipe>& recipes) override;

    private:
        std::vector<cost_t> best_cost_;
        std::vector<cost_t> next_best_cost_;
        std::vector<internal_assignment_t> best_assignment_;
        std::vector<internal_assignment_t> next_best_assignment_;
    };
}

#endif // RECAP_PARALLEL_ASSIGNMENT_HPP_