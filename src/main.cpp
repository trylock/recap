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

    po::options_description desc{ "Allowed options" };
    desc.add_options()
        ("help,h", "show help message")
        ("input,i", po::value<std::string>(), "file with all available recipes")
        ("armour,a", po::value<std::size_t>()->default_value(7), "number of armour slots")
        ("jewelery,j", po::value<std::size_t>()->default_value(3), "number of jewelery slots")
        ("fire,f", po::value<resistance::item_t>(), "required fire resistance")
        ("cold,c", po::value<resistance::item_t>(), "required cold resistance")
        ("lightning,l", po::value<resistance::item_t>(), "required lightning resistance")
        ("chaos,ch", po::value<resistance::item_t>()->default_value(0), "required chaos resistance");

    po::positional_options_description p;
    p.add("fire", 1);
    p.add("cold", 1);
    p.add("lightning", 1);
    p.add("chaos", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 1;
    }

    if (!vm.count("input"))
    {
        std::cerr << "Error: specify path to a file with available recipes." << std::endl;
        return 2;
    }

    if (!vm.count("fire"))
    {
        std::cerr << "Error: specify required fire resistance" << std::endl;
        return 3;
    }

    if (!vm.count("cold"))
    {
        std::cerr << "Error: specify required cold resistance" << std::endl;
        return 3;
    }

    if (!vm.count("lightning"))
    {
        std::cerr << "Error: specify required lightning resistance" << std::endl;
        return 3;
    }

    if (!vm.count("chaos"))
    {
        std::cerr << "Error: specify required chaos resistance" << std::endl;
        return 3;
    }

    // read recipes from file
    std::vector<recipe> recipes;
    try 
    {
        recipes = read_recipes(vm["input"].as<std::string>());
        std::cout << "Loaded " << recipes.size() << " recipe variants." << std::endl;

        // read required resistances
        resistance required{
            vm["fire"].as<resistance::item_t>(),
            vm["cold"].as<resistance::item_t>(),
            vm["lightning"].as<resistance::item_t>(),
            vm["chaos"].as<resistance::item_t>()
        };

        // read slots
        std::vector<recipe::slot_t> slots;
        for (std::size_t i = 0; i < vm["armour"].as<std::size_t>(); ++i)
        {
            slots.push_back(recipe::SLOT_ARMOUR);
        }

        for (std::size_t i = 0; i < vm["jewelery"].as<std::size_t>(); ++i)
        {
            slots.push_back(recipe::SLOT_JEWELERY);
        }

        std::cout 
            << "Armour slots: " << vm["armour"].as<std::size_t>() << std::endl 
            << "Jewelery slots: " << vm["jewelery"].as<std::size_t>() << std::endl 
            << "Required resistances: " 
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
        return 4;
    }
    catch (io::error::base& err)
    {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    return 0;
}