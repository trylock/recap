#ifndef RECAP_ASSIGNMENT_KERNEL_HPP_
#define RECAP_ASSIGNMENT_KERNEL_HPP_

#include <cuda_runtime.h>
#include <cstdint>

#include "assignment.hpp"
#include "resistance.hpp"
#include "recipe.hpp"

namespace recap
{
    namespace cuda
    {
        using resistance_t = resistance::item_t;

        // maximal number of equipment slots
        inline static constexpr std::size_t MAX_SLOT_COUNT = 16;

        template<typename T>
        struct vector4
        {
            T x, y, z, w;
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
            // Resistances providied by current recipe
            vector4<resistance_t> recipe_resist;
            // Cost of this recipe
            recipe::cost_t recipe_cost;
            // index of this recipe 
            std::uint8_t recipe_index;
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