#include "cuda_assignment_algorithm.hpp"

recap::cuda_assignment_algorithm::cuda_assignment_algorithm()
{
    initialize(resistance::make_zero());
}

void recap::cuda_assignment_algorithm::initialize(resistance max_res)
{
    auto value_count = parallel_assignment_algorithm::count_values(max_res);

    // allocate CPU buffers where we will store the result
    output_cost_.resize(value_count);
    output_assignment_.resize(value_count * MAX_SLOT_COUNT);

    // allocate memory on the GPU
    best_cost_.allocate(value_count);
    next_best_cost_.allocate(value_count);
    best_assignment_.allocate(value_count * MAX_SLOT_COUNT);
    next_best_assignment_.allocate(value_count * MAX_SLOT_COUNT);

    max_res_ = max_res;
}

recap::assignment recap::cuda_assignment_algorithm::run(
    resistance required, 
    const std::vector<recipe::slot_t>& slots, 
    const std::vector<recipe>& recipes)
{
    // if we need to allocate more memory
    if (parallel_assignment_algorithm::count_values(required) > output_cost_.size())
    {
        initialize(required);
    }

    // Check that we can fit all recipes into index type
    if (recipes.size() > std::numeric_limits<recipe_index_t>::max())
    {
        throw std::runtime_error{ "Recipes won't fit into used index type." };
    }

    // Check number of slots
    if (slots.size() > MAX_SLOT_COUNT)
    {
        throw std::runtime_error{ 
            "Internal type has memory only for " + 
            std::to_string(MAX_SLOT_COUNT) +  " slots." };
    }

    // initialize cost to MAX_COST
    std::fill(output_cost_.begin(), output_cost_.end(), recipe::MAX_COST);

    // 0 is always satisfiable
    output_cost_[0] = 0;

    // copy current cost to GPU
    best_cost_.copy_to_gpu(output_cost_);
    next_best_cost_.copy_to_gpu(output_cost_);

    // construct kernel arguments
    cuda::input_data input;
    input.best_cost = best_cost_.get();
    input.best_assignment = best_assignment_.get();
    input.table_size = output_cost_.size();
    input.table_dim.x = required.fire() + 1;
    input.table_dim.y = required.cold() + 1;
    input.table_dim.z = required.lightning() + 1;
    input.table_dim.w = required.chaos() + 1;

    cuda::output_data output;
    output.best_cost = next_best_cost_.get();
    output.best_assignment = next_best_assignment_.get();

    // fill output_cost_ with MAX_COST so we can use it to fill next_best_cost_ each iteration
    std::fill(output_cost_.begin(), output_cost_.end(), recipe::MAX_COST);

    for (std::uint8_t i = 0; i < slots.size(); ++i)
    {
        // reset output costs to MAX_COST
        next_best_cost_.copy_to_gpu(output_cost_);

        // set current slot
        input.slot_index = i;

        for (recipe_index_t j = 0; j < recipes.size(); ++j)
        {
            const auto& recipe = recipes[j];

            // if this recipe is not aplicable for slot i
            if ((recipe.slots() & slots[i]) == 0)
            {
                continue; // skip this recipe
            }

            // set current recipe
            input.recipe_resist.x = recipe.resistances().fire();
            input.recipe_resist.y = recipe.resistances().cold();
            input.recipe_resist.z = recipe.resistances().lightning();
            input.recipe_resist.w = recipe.resistances().chaos();
            input.recipe_cost = recipe.cost();
            input.recipe_index = j;

            cuda::run_assignment_kernel(input, output);
            CUCHECK(cudaDeviceSynchronize());
        }

        // swap buffers
        std::swap(next_best_cost_, best_cost_);
        std::swap(next_best_assignment_, best_assignment_);

        input.best_cost = best_cost_.get();
        input.best_assignment = best_assignment_.get();
        output.best_cost = next_best_cost_.get();
        output.best_assignment = next_best_assignment_.get();
    }

    // get results from GPU
    best_cost_.copy_from_gpu(output_cost_);
    best_assignment_.copy_from_gpu(output_assignment_);

    // Count number of distinct resistance values <= required
    const resistance res_count{ 
        static_cast<resistance::item_t>(required.fire() + 1), 
        static_cast<resistance::item_t>(required.cold() + 1), 
        static_cast<resistance::item_t>(required.lightning() + 1), 
        static_cast<resistance::item_t>(required.chaos() + 1) 
    };

    // Convert resistance object to a linear index.
    // This is a one-to-one mapping from resistances < res_count to [0, value_count - 1]
    auto to_index = [res_count](resistance res)
    {
        std::size_t index = res.fire();
        index = index * res_count.cold() + res.cold();
        index = index * res_count.lightning() + res.lightning();
        index = index * res_count.chaos() + res.chaos();
        return index;
    };

    // lookup the solution in the table
    auto result_index = to_index(required);
    auto result_cost = output_cost_[result_index];
    auto result_assignment = &output_assignment_[result_index * MAX_SLOT_COUNT];
    
    // convert it to the output type
    assignment result;
    result.cost() = result_cost;

    if (result.cost() != recipe::MAX_COST)
    {
        for (std::size_t i = 0; i < slots.size(); ++i)
        {
            auto& used_recipe = recipes[result_assignment[i]];
            if (used_recipe.resistances() != resistance::make_zero())
            {
                result.assignments().push_back(recipe_assignment{ slots[i], used_recipe });
            }
        }
    }

    return result;
}
