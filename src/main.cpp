#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cmath>
#include <chrono>

#include <csv.h>
#include <boost/program_options.hpp>

#include "resistance.hpp"
#include "recipe.hpp"
#include "assignment.hpp"

// exception thrown if input values are invalid
class invalid_input_error : public std::exception
{
public:
    invalid_input_error(std::size_t line_num, const std::string& msg) :
        msg_("Error on line " + std::to_string(line_num) + ": " + msg) {}

    const char* what() const noexcept override 
    {
        return msg_.c_str();
    }
private:
    std::string msg_;
};

/** Read recipes from a CSV file located at @p path
 * 
 * @param path Path to a file with recipes
 * 
 * @returns list of recipes
 */
std::vector<recap::recipe> read_recipes(const std::string& path)
{
    using namespace recap;

    // read file header
    io::CSVReader<8> input(path);
    input.read_header(io::ignore_extra_column, "fire", "cold", "lightning", "chaos", "value_min", "value_max", "cost", "slot");

    // add a null recipe to index 0
    std::vector<recipe> result;
    result.push_back(recipe{ resistance{ 0, 0, 0, 0 }, 0, recipe::SLOT_ALL });

    // read recipes from file
    resistance::item_t fire, cold, lightning, chaos, value_min, value_max;
    recipe::cost_t cost = 0;
    std::string slot_name;
    while (input.read_row(fire, cold, lightning, chaos, value_min, value_max, cost, slot_name))
    {
        if (fire > 1)
        {
            throw invalid_input_error{ 
                input.get_file_line(), 
                "fire value has to be 0 or 1." };
        }

        if (cold > 1)
        {
            throw invalid_input_error{ 
                input.get_file_line(), 
                "cold value has to be 0 or 1." };
        }

        if (lightning > 1)
        {
            throw invalid_input_error{ 
                input.get_file_line(), 
                "lightning value has to be 0 or 1." };
        }

        if (chaos > 1)
        {
            throw invalid_input_error{ 
                input.get_file_line(), 
                "chaos value has to be 0 or 1." };
        }

        if (value_min > value_max)
        {
            throw invalid_input_error{ 
                input.get_file_line(), 
                "minimal value must not be greater than maximal value." };
        }

        recipe::slot_t slot_value = recipe::SLOT_NONE;
        if (slot_name == "armour")
        {
            slot_value = recipe::SLOT_ARMOUR;
        }
        else if (slot_name == "jewelery")
        {
            slot_value = recipe::SLOT_JEWELERY;
        }
        else if (slot_name == "all")
        {
            slot_value = recipe::SLOT_ALL;
        }
        else 
        {
            throw invalid_input_error{ 
                input.get_file_line(), 
                "minimal value must not be greater than maximal value." };
        }

        // generate recipes
        for (resistance::item_t i = value_min; i <= value_max; ++i)
        {
            // compute expected cost of rolling these values
            double instance_cost = cost * (value_max - value_min + 1.0) / (value_max - i + 1.0);
            result.push_back(recipe{
                resistance{
                    static_cast<resistance::item_t>(fire * i),
                    static_cast<resistance::item_t>(cold * i),
                    static_cast<resistance::item_t>(lightning * i),
                    static_cast<resistance::item_t>(chaos * i)
                }, 
                static_cast<recipe::cost_t>(instance_cost),
                slot_value
            });
        }
    }

    return result;
}

// Print table cell
template<typename T>
void print_cell(T&& value, int width)
{
    std::cout << std::left << std::setw(width) << std::setfill(' ') << value;
}

// Print recipe assignment to armour slots
void print_assignment(const std::vector<recap::recipe::slot_t>& slots, recap::assignment& assign)
{
    using namespace recap;

    if (assign.cost() >= recipe::MAX_COST)
    {
        std::cout << "No solution." << std::endl;
    }
    else 
    {
        std::cout << "Found solution with cost " << assign.cost() << ": " << std::endl;

        constexpr int width = 13;
        print_cell("slot", width);
        print_cell("fire%", width);
        print_cell("cold%", width);
        print_cell("lightning%", width);
        print_cell("chaos%", width);
        print_cell("cost", width);
        std::cout << std::endl;

        for (std::size_t i = 0; i < width * 6; ++i)
        {
            std::cout << "-";
        }
        std::cout << std::endl;

        resistance total = resistance::make_zero(); 
        recipe::cost_t total_cost = 0;
        for (std::size_t i = 0; i < assign.recipes().size(); ++i)
        {
            const auto& recipe = assign.recipes()[i];

            if (slots[i] == recipe::SLOT_ARMOUR)
            {
                print_cell("armour", width);
            }
            else if (slots[i] == recipe::SLOT_JEWELERY)
            {
                print_cell("jewelery", width);
            }
            else 
            {
                print_cell("<unknown>", width);
            }

            print_cell(recipe.resistances().fire(), width);
            print_cell(recipe.resistances().cold(), width);
            print_cell(recipe.resistances().lightning(), width);
            print_cell(recipe.resistances().chaos(), width);
            print_cell(recipe.cost(), width);
            std::cout << std::endl;

            total = total + recipe.resistances();
            total_cost += recipe.cost();
        }
        for (std::size_t i = 0; i < width * 6; ++i)
        {
            std::cout << "-";
        }
        std::cout << std::endl;

        print_cell("", width);
        print_cell(total.fire(), width);
        print_cell(total.cold(), width);
        print_cell(total.lightning(), width);
        print_cell(total.chaos(), width);
        print_cell(total_cost, width);
        std::cout << std::endl << std::endl;
    }
}

int main(int argc, char** argv)
{
    using namespace recap;

    namespace po = boost::program_options;

    // input limits
    constexpr std::size_t MAX_ARMOUR_SLOT_COUNT = 7;
    constexpr std::size_t MAX_JEWELERY_SLOT_COUNT = 3;
    constexpr std::size_t MAX_RECIPE_COUNT = 256;

    po::options_description desc{ "Allowed options" };
    desc.add_options()
        ("help,h", "show help message")
        ("input,i", po::value<std::string>(), "file with all available recipes")
        ("armour,a", po::value<std::size_t>()->default_value(7), "number of armour slots")
        ("jewelery,j", po::value<std::size_t>()->default_value(3), "number of jewelery slots")
        ("required,r", po::value<std::vector<resistance::item_t>>()->multitoken(), "list of required resistances (in order: fire, cold, lightning, and chaos");

    po::variables_map vm;

    try 
    {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        po::notify(vm);
    }
    catch (boost::program_options::error& err)
    {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }

    if (!vm.count("input"))
    {
        std::cerr << "Error: specify path to a file with available recipes." << std::endl;
        return 1;
    }

    // validate required resistances
    if (!vm.count("required"))
    {
        std::cerr << "Error: specify required resistances" << std::endl;
        return 1;
    }

    auto input_resistances = vm["required"].as<std::vector<resistance::item_t>>();
    if (input_resistances.size() <= 0 || input_resistances.size() > 4)
    {
        std::cerr 
            << "Error: wrong number of resistances. Expected at most 4, got " 
            << input_resistances.size() << std::endl;
        return 1;
    }

    // normalize input vector size to 4
    while (input_resistances.size() < 4)
    {
        input_resistances.emplace_back(0);
    }

    // read recipes from file
    std::vector<recipe> recipes;
    try 
    {
        recipes = read_recipes(vm["input"].as<std::string>());
        std::cout << "Loaded " << recipes.size() << " recipe variants." << std::endl;

        if (recipes.size() > MAX_RECIPE_COUNT)
        {
            std::cerr << "Error: this tool is limited to " << MAX_RECIPE_COUNT << " recipe variants at the moment." << std::endl;
            return 1;
        }

        // read required resistances
        resistance required{ 
            input_resistances[0], 
            input_resistances[1],
            input_resistances[2],
            input_resistances[3]
        };

        // read slots
        auto armour_slot_cout = vm["armour"].as<std::size_t>();
        if (armour_slot_cout > MAX_ARMOUR_SLOT_COUNT)
        {
            std::cerr << "Error: there can be at most " << MAX_ARMOUR_SLOT_COUNT << " armour slots." << std::endl;
            return 1;
        }

        std::vector<recipe::slot_t> slots;
        for (std::size_t i = 0; i < armour_slot_cout; ++i)
        {
            slots.push_back(recipe::SLOT_ARMOUR);
        }

        auto jewelery_slot_count = vm["jewelery"].as<std::size_t>();
        if (jewelery_slot_count > MAX_JEWELERY_SLOT_COUNT)
        {
            std::cerr << "Error: there can be at most " << MAX_JEWELERY_SLOT_COUNT << " jewelery slots." << std::endl;
            return 1;
        }

        for (std::size_t i = 0; i < jewelery_slot_count; ++i)
        {
            slots.push_back(recipe::SLOT_JEWELERY);
        }

        std::cout 
            << "Armour slots: " << armour_slot_cout << std::endl 
            << "Jewelery slots: " << jewelery_slot_count << std::endl 
            << "Required: " 
                << required.fire() << "% fire, "
                << required.cold() << "% cold, "
                << required.lightning() << "% lightning, "
                << required.chaos() << "% chaos " << std::endl;

        std::cout << std::endl;

        auto begin = std::chrono::steady_clock::now();
        auto result = find_assignment(required, slots, recipes);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

        print_assignment(slots, result);
        std::cout << duration << " ms" << std::endl;
    }
    catch (invalid_input_error& err)
    {
        std::cerr << err.what() << std::endl;
        return 1;
    }
    catch (io::error::base& err)
    {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    return 0;
}