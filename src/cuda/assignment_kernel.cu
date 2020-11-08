#include "assignment_kernel.hpp"

#include "device_launch_parameters.h"

using index_vector_t = recap::cuda::vector4<recap::cuda::resistance_t>;

/** Convert linear index to table index (the table is 4 dimensional - one for each type of resistance)
 * 
 * @param idx Linear index
 * @param table_dim Dimensions of the table
 * 
 * @returns index for each dimension in the table
 */
__device__ index_vector_t index_to_vector(int idx, index_vector_t table_dim)
{
    index_vector_t result;
    result.w = idx % table_dim.w;
    idx /= table_dim.w;
    result.z = idx % table_dim.z;
    idx /= table_dim.z;
    result.y = idx % table_dim.y;
    idx /= table_dim.y;
    result.x = idx;

    return result;
}

/** Convert table indices into a single linear index
 * 
 * @param indices Indices for each dimension
 * @param table_dim Dimensions of the table
 * 
 * @returns linear index
 */
__device__ int vector_to_index(index_vector_t indices, index_vector_t table_dim)
{
    int index = indices.x;
    index = index * table_dim.y + indices.y;
    index = index * table_dim.z + indices.z;
    index = index * table_dim.w + indices.w;
    return index;
}

/** Subtract @p rhs from @p lhs
 * 
 * If the result is negative, it's clamped to 0.
 * 
 * @param lhs Left-hand side of the expression
 * @param rhs Right-hand side of the expression
 * 
 * @returns result
 */
__device__ index_vector_t index_vector_sub(index_vector_t lhs, index_vector_t rhs)
{
    index_vector_t result;
    result.x = max(lhs.x - (int)rhs.x, 0);
    result.y = max(lhs.y - (int)rhs.y, 0);
    result.z = max(lhs.z - (int)rhs.z, 0);
    result.w = max(lhs.w - (int)rhs.w, 0);
    return result;
}

/** Find best assignment 
 * 
 * @param input Input parameters
 * @param output Output parameters
 */
__global__ void assignment_kernel(
    const recap::cuda::input_data input, 
    recap::cuda::output_data output)
{
    auto current_index = blockIdx.x * blockDim.x + threadIdx.x;
    if (current_index >= input.table_size)
    {
        return;
    }

    // find resistances at current index
    auto current_resist = index_to_vector(current_index, input.table_dim);
    auto current_cost = output.best_cost[current_index];

    // find resistances if we use assigned recipe
    auto prev_resist = index_vector_sub(current_resist, input.recipe_resist);
    auto prev_index = vector_to_index(prev_resist, input.table_dim);
    auto prev_cost = input.best_cost[prev_index];

    // if using the recipe is better then current best solution, update the solution
    if (current_cost > prev_cost + input.recipe_cost)
    {
        output.best_cost[current_index] = prev_cost + input.recipe_cost;

        for (int i = 0; i < recap::cuda::MAX_SLOT_COUNT; ++i)
        {
            auto current_slot_index = current_index * recap::cuda::MAX_SLOT_COUNT + i;
            auto prev_slot_index = prev_index * recap::cuda::MAX_SLOT_COUNT + i;
            output.best_assignment[current_slot_index] = input.best_assignment[prev_slot_index];
        }
        output.best_assignment[current_index * recap::cuda::MAX_SLOT_COUNT + input.slot_index] = input.recipe_index;
    }
}

void recap::cuda::run_assignment_kernel(const input_data input, output_data output)
{
    constexpr int block_size = 256;

    ::assignment_kernel<<<(input.table_size + block_size - 1) / block_size, block_size>>>(input, output);
}