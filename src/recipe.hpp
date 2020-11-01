#ifndef RECAP_RECIPE_HPP_
#define RECAP_RECIPE_HPP_

#include <cstdint>
#include <limits>
#include <vector>

#include "resistance.hpp"

namespace recap
{
    struct recipe 
    {
        enum slot_t : std::uint32_t
        {
            SLOT_NONE = 0,

            // equipment slots
            SLOT_WEAPON1 = 1,
            SLOT_WEAPON2 = 1 << 1,
            SLOT_HELMET = 1 << 2,
            SLOT_BODY = 1 << 3,
            SLOT_RING1 = 1 << 4,
            SLOT_RING2 = 1 << 5,
            SLOT_AMULET = 1 << 6,
            SLOT_BELT = 1 << 7,
            SLOT_GLOVES = 1 << 8,
            SLOT_BOOTS = 1 << 9,
            
            // applicable to everything but ring and amulet
            SLOT_ARMOUR = SLOT_WEAPON1 | SLOT_WEAPON2 | SLOT_HELMET | SLOT_BODY | SLOT_BOOTS | SLOT_GLOVES | SLOT_BELT,
            // aplicable to ring and amulet
            SLOT_JEWELERY = SLOT_RING1 | SLOT_RING2 | SLOT_AMULET,
            // aplicable to everything
            SLOT_ALL = SLOT_ARMOUR | SLOT_JEWELERY
        };

        using cost_t = float;

        // maximal cost (indicates invalid state)
        inline static cost_t MAX_COST = std::numeric_limits<cost_t>::infinity();

        /** Create a zero cost recipe aplicable to no slot
         */
        inline recipe() : res_(resistance::make_zero()), cost_(0), slots_(SLOT_NONE) {}

        /** Create a new recipe
         * 
         * @param r Resistances granted by the recipe
         * @param cost Cost of the recipe
         * @param s Aplicable slots
         */
        inline recipe(resistance r, cost_t cost, slot_t s) : res_(r), cost_(cost), slots_(s) {}

        /** Get resistances granted by the recipe
         * 
         * @return resistances
         */
        inline resistance resistances() const
        {
            return res_;
        }

        /** Get cost of the recipe (this is an arbitrary value, interpretation is up to the user)
         * 
         * @return cost
         */
        inline cost_t cost() const
        {
            return cost_;
        }

        /** Get aplicalbe slots
         * 
         * @return slots you can use this recipe on
         */
        inline slot_t slots() const
        {
            return slots_;
        }

    private:
        resistance res_;
        cost_t cost_;
        slot_t slots_;
    };
}

#endif // RECAP_RECIPE_HPP_