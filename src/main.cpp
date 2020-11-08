#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <cmath>
#include <chrono>

#include <csv.h>
#include <rang.hpp>
#include <boost/program_options.hpp>

#include "resistance.hpp"
#include "recipe.hpp"
#include "assignment.hpp"
#include "reassignment.hpp"
#include "cuda_assignment_algorithm.hpp"

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

class invalid_arg_error : public std::exception
{
public:
    invalid_arg_error(const std::string& msg) :
        msg_(msg) {}

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

        recipe::slot_t slot_value = parse_slot(slot_name);
        if (slot_value == recipe::SLOT_NONE)
        {
            throw invalid_input_error{ 
                input.get_file_line(), 
                "invalid slot: " + slot_name };
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

/** Read equipment from @p path
 * 
 * @param path Path to a file
 * 
 * @returns equipment items
 */
std::vector<recap::equipment> read_equipment(const std::string& path)
{
    using namespace recap;

    std::vector<equipment> items;

    // read file header
    io::CSVReader<11> input(path);
    input.read_header(io::ignore_extra_column, 
        "slot", 
        "craft_fire", 
        "craft_cold", 
        "craft_lightning", 
        "craft_chaos", 
        "base_fire", 
        "base_cold", 
        "base_lightning", 
        "base_chaos", 
        "is_craftable", 
        "is_new");

    // read values from file
    resistance::item_t craft_fire = 0, craft_cold = 0, craft_lightning = 0, craft_chaos = 0;
    resistance::item_t base_fire = 0, base_cold = 0, base_lightning = 0, base_chaos = 0;
    int is_craftable, is_new;
    std::string slot_name;
    while (input.read_row(slot_name, 
        craft_fire, craft_cold, craft_lightning, craft_chaos, 
        base_fire, base_cold, base_lightning, base_chaos,
        is_craftable, is_new))
    {
        auto slot_value = parse_slot(slot_name);
        if (slot_value == recipe::SLOT_NONE) 
        {
            throw invalid_input_error{
                input.get_file_line(),
                "Invalid slot name: " + slot_name
            };
        }

        items.push_back(equipment{ slot_value, 
            resistance{ craft_fire, craft_cold, craft_lightning, craft_chaos },
            resistance{ base_fire, base_cold, base_lightning, base_chaos }, 
            !!is_craftable,
            !!is_new });
    }

    return items;
}

/** Print @p assign in a human readable way
 * 
 * @param output Output stream
 * @param assign Assignment of recipes to equipment slots
 */ 
void print_assignment(std::ostream& output, const recap::assignment& assign)
{
    using namespace recap;

    constexpr int width = 13;
    constexpr int col_count = 6;

    // print table cell
    auto print_cell = [&output]() -> std::ostream&
    {
        output << std::left << std::setw(width) << std::setfill(' ');
        return output;
    };

    // print line separator
    auto print_sep = [&output]()
    {
        for (std::size_t i = 0; i < width * col_count; ++i)
        {
            output << "-";
        }
        output << std::endl;
    };

    if (assign.cost() >= recipe::MAX_COST)
    {
        output << "No solution." << std::endl;
    }
    else 
    {
        output << "Found solution with cost " << assign.cost() << ": " << std::endl;

        print_sep();

        output << rang::style::bold;
        print_cell() <<  "slot";
        print_cell() << "fire%";
        print_cell() << "cold%";
        print_cell() << "lightning%";
        print_cell() << "chaos%";
        print_cell() << "cost";
        output << rang::style::reset;
        output << std::endl;

        print_sep();

        resistance total = resistance::make_zero(); 
        recipe::cost_t total_cost = 0;
        for (std::size_t i = 0; i < assign.assignments().size(); ++i)
        {
            const auto& recipe = assign.assignments()[i].used_recipe();
            auto slot = assign.assignments()[i].slot();

            print_cell() << to_string(slot);
            print_cell() << recipe.resistances().fire();
            print_cell() << recipe.resistances().cold();
            print_cell() << recipe.resistances().lightning();
            print_cell() << recipe.resistances().chaos();
            print_cell() << recipe.cost();
            output << std::endl;

            total = total + recipe.resistances();
            total_cost += recipe.cost();
        }

        print_sep();

        print_cell() << "";
        print_cell() << total.fire();
        print_cell() << total.cold();
        print_cell() << total.lightning();
        print_cell() << total.chaos();
        print_cell() << total_cost;
        output << std::endl << std::endl;
    }
}

/** Read resistances from a variable length vector.
 * 
 * If size of the vector is < 4, 0 is used as a default value.
 * 
 * @param args Resistances in order: fire, cold, lightning, and chaos
 * 
 * @returns resistance object
 */
recap::resistance read_resistance_arg(const std::string& name, boost::program_options::variables_map& vm)
{
    using namespace recap;

    if (!vm.count(name))
    {
        throw invalid_arg_error{ "Missing required argument: " + name };
    }

    auto args = vm[name].as<std::vector<resistance::item_t>>();
    if (args.size() <= 0 || args.size() > 4)
    {
        throw invalid_arg_error{ 
            "Wrong number of resistances. Expected at most 4, got " + 
            std::to_string(args.size()) };
    }

    return resistance{ 
        args.size() > 0 ? args[0] : resistance::item_t{ 0 },  
        args.size() > 1 ? args[1] : resistance::item_t{ 0 },
        args.size() > 2 ? args[2] : resistance::item_t{ 0 },
        args.size() > 3 ? args[3] : resistance::item_t{ 0 }
    };
}

int main(int argc, char** argv)
{
    using namespace recap;

    namespace po = boost::program_options;

    // input limits
    constexpr std::size_t MAX_ARMOUR_SLOT_COUNT = 7;
    constexpr std::size_t MAX_JEWELRY_SLOT_COUNT = 3;
    constexpr std::size_t MAX_RECIPE_COUNT = 256;

    po::options_description desc{ "Allowed options" };
    desc.add_options()
        ("help,h", "show help message")
        ("input,i", po::value<std::string>(), "path to a file with all available recipes")
        ("equip,e", po::value<std::string>(), "path to a file with all your equipment")
        ("armour,a", po::value<std::size_t>()->default_value(7), "number of armour slots")
        ("jewelery,j", po::value<std::size_t>()->default_value(3), "number of jewelery slots")
        ("required,r", po::value<std::vector<resistance::item_t>>()->multitoken(), 
            "list of required resistances (in order: fire, cold, lightning, and chaos")
        ("current,c", po::value<std::vector<resistance::item_t>>()->multitoken(), 
            "list of current uncapped resistances (in order: fire, cold, lightning, and chaos");

    // print help as default action
    if (argc == 1)
    {
        std::cerr << desc << std::endl;
        return 1;
    }

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

        // read resistances
        resistance required = read_resistance_arg("required", vm);

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

        auto jewelry_slot_count = vm["jewelery"].as<std::size_t>();
        if (jewelry_slot_count > MAX_JEWELRY_SLOT_COUNT)
        {
            std::cerr << "Error: there can be at most " << MAX_JEWELRY_SLOT_COUNT << " jewelery slots." << std::endl;
            return 1;
        }

        for (std::size_t i = 0; i < jewelry_slot_count; ++i)
        {
            slots.push_back(recipe::SLOT_JEWELRY);
        }

        // Print required resistances
        std::cout << "Required: " 
            << required.fire() << "% fire, "
            << required.cold() << "% cold, "
            << required.lightning() << "% lightning, "
            << required.chaos() << "% chaos " << std::endl;

        if (vm.count("equip"))
        {
            // read current resistances
            resistance current = read_resistance_arg("current", vm);

            // read items from file
            auto items = read_equipment(vm["equip"].as<std::string>());

            // find reassignment
            auto begin = std::chrono::steady_clock::now();
            auto result = find_minimal_reassignment<cuda_assignment_algorithm>(current, required, items, recipes);
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

            print_assignment(std::cout, result);
            std::cout << duration << " ms" << std::endl;
        }
        else 
        {
            std::cout 
                << "Armour slots: " << armour_slot_cout << std::endl 
                << "Jewelery slots: " << jewelry_slot_count << std::endl;

            std::cout << std::endl;

            cuda_assignment_algorithm algorithm;

            auto begin = std::chrono::steady_clock::now();
            auto result = algorithm.run(required, slots, recipes);
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

            print_assignment(std::cout, result);
            std::cout << duration << " ms" << std::endl;
        }
    }
    catch (invalid_input_error& err)
    {
        std::cerr << err.what() << std::endl;
        return 1;
    }
    catch (invalid_arg_error& err)
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