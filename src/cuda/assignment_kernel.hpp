#ifndef RECAP_ASSIGNMENT_KERNEL_HPP_
#define RECAP_ASSIGNMENT_KERNEL_HPP_

#include <cuda_runtime.h>
#include <cstdint>
#include <cmath>

#include "assignment.hpp"
#include "resistance.hpp"
#include "recipe.hpp"

namespace recap
{
    namespace cuda
    {
        using resistance_t = resistance::item_t;

        // maximal number of equipment slots
        inline static constexpr std::size_t MAX_SLOT_COUNT = 10;

        template<typename T>
        struct vector4
        {
            T x, y, z, w;
        };

        struct recipes_data
        {
            // Resistances of provided by recipes
            const vector4<resistance_t>* resistances;
            // Cost of recipes
            const recipe::cost_t* costs;
            // Recipe slots
            const recipe::slot_t* slots;
            // Number of recipes
            std::uint32_t count;
        };

        struct input_data
        {
            // Array of best costs from previous iteration
            const float* best_cost;
            // Best assignments (indices of recipes) from previous iteration
            const std::uint8_t* best_assignment;
            // Table size 
            std::uint32_t table_size;
            // Maximal resistances
            vector4<resistance_t> table_dim;
            // Available recipes
            recipes_data recipes;
            // Current slot mask
            recipe::slot_t slot;
            // index of a slot
            std::uint8_t slot_index;
        };

        struct output_data
        {
            // Array of best costs in this iteration 
            float* best_cost;
            // Best assignemnts (indices of recipes) in this iteration
            std::uint8_t* best_assignment;
        };

        void run_assignment_kernel(const input_data input, output_data output);
    }
}

#endif // RECAP_ASSIGNMENT_KERNEL_HPP_