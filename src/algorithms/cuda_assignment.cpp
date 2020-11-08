#include "cuda_assignment.hpp"

recap::cuda_assignment::cuda_assignment()
{
}

const char* recap::cuda_assignment::name() const 
{
    return "cuda";
}

void recap::cuda_assignment::initialize(resistance max_res, std::size_t max_recipes)
{
    auto value_count = count_values(max_res);

    // allocate CPU buffers where we will store the result
    output_cost_.resize(value_count);
    output_assignment_.resize(value_count * MAX_SLOT_COUNT);

    // allocate memory on the GPU
    best_cost_.allocate(value_count);
    next_best_cost_.allocate(value_count);
    best_assignment_.allocate(value_count * MAX_SLOT_COUNT);
    next_best_assignment_.allocate(value_count * MAX_SLOT_COUNT);

    // allocate memory for recipes
    buffer_cost_.resize(max_recipes);
    buffer_slot_.resize(max_recipes);
    buffer_resist_.resize(max_recipes);

    // allocate memory for recipes on GPU
    recipe_cost_.allocate(max_recipes);
    recipe_slot_.allocate(max_recipes);
    recipe_resist_.allocate(max_recipes);
}

void recap::cuda_assignment::set_recipes(cuda::input_data& input, const std::vector<recipe>& recipes)
{
    // copy recipes to GPU
    for (std::size_t i = 0; i < recipes.size(); ++i)
    {
        const auto& recipe = recipes[i];

        buffer_cost_[i] = recipe.cost();
        buffer_slot_[i] = recipe.slots();
        buffer_resist_[i].x = recipe.resistances().fire();
        buffer_resist_[i].y = recipe.resistances().cold();
        buffer_resist_[i].z = recipe.resistances().lightning();
        buffer_resist_[i].w = recipe.resistances().chaos();
    }
    recipe_cost_.copy_to_gpu(buffer_cost_, recipes.size());
    recipe_slot_.copy_to_gpu(buffer_slot_, recipes.size());
    recipe_resist_.copy_to_gpu(buffer_resist_, recipes.size());

    // update input data
    input.recipes.costs = recipe_cost_.get();
    input.recipes.slots = recipe_slot_.get();
    input.recipes.resistances = recipe_resist_.get();
    input.recipes.count = recipes.size();
}

void recap::cuda_assignment::set_table_size(cuda::input_data& input, resistance req)
{
    auto value_count = count_values(req);

    input.table_size = value_count;
    input.table_dim.x = req.fire() + 1;
    input.table_dim.y = req.cold() + 1;
    input.table_dim.z = req.lightning() + 1;
    input.table_dim.w = req.chaos() + 1;
}

void recap::cuda_assignment::set_table_buffers(cuda::input_data& input, std::size_t value_count)
{
    // initialize cost to MAX_COST
    std::fill(output_cost_.begin(), output_cost_.begin() + value_count, recipe::MAX_COST);
    output_cost_[0] = 0;

    // copy current cost to GPU
    best_cost_.copy_to_gpu(output_cost_, value_count);

    // update input data
    input.best_cost = best_cost_.get();
    input.best_assignment = best_assignment_.get();
}

recap::assignment recap::cuda_assignment::find_minimal_assignment(
    resistance required, 
    const std::vector<recipe::slot_t>& slots, 
    const std::vector<recipe>& recipes)
{
    // if we need to allocate more memory
    auto value_count = count_values(required);
    if (value_count > output_cost_.size() || 
        buffer_cost_.size() > recipes.size())
    {
        initialize(required, recipes.size());
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

    // construct kernel arguments
    cuda::input_data input;
    set_table_size(input, required);
    set_table_buffers(input, value_count);
    set_recipes(input, recipes);

    cuda::output_data output;
    output.best_cost = next_best_cost_.get();
    output.best_assignment = next_best_assignment_.get();

    for (std::uint8_t i = 0; i < slots.size(); ++i)
    {
        // set current slot
        input.slot_index = i;
        input.slot = slots[i];

        // compute table with i slots
        cuda::run_assignment_kernel(input, output);
        CUCHECK(cudaDeviceSynchronize());

        // swap buffers
        std::swap(next_best_cost_, best_cost_);
        std::swap(next_best_assignment_, best_assignment_);

        input.best_cost = best_cost_.get();
        input.best_assignment = best_assignment_.get();
        output.best_cost = next_best_cost_.get();
        output.best_assignment = next_best_assignment_.get();
    }

    // get results from GPU
    best_cost_.copy_from_gpu(output_cost_, value_count);
    best_assignment_.copy_from_gpu(output_assignment_, value_count * MAX_SLOT_COUNT);

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
