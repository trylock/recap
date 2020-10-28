#ifndef RECAP_RESISTANCE_HPP_
#define RECAP_RESISTANCE_HPP_

#include <cassert>
#include <cstdint>

namespace recap
{
    // 4-tuple of resistances
    struct resistance
    {
        std::uint64_t value;

        using item_t = std::uint16_t; 

        // DefaultConstructible
        resistance() = default;

        // Copyable
        resistance(const resistance&) = default;
        resistance& operator=(const resistance&) = default;
        
        inline resistance(item_t fire, item_t cold, item_t lightning, item_t chaos)
        {
            assert(sizeof(item_t) == 2);
            assert(sizeof(std::uint64_t) == 8);

            value = 0;
            value |= static_cast<item_t>(fire);
            value <<= 16;
            value |= static_cast<item_t>(cold);
            value <<= 16;
            value |= static_cast<item_t>(lightning);
            value <<= 16;
            value |= static_cast<item_t>(chaos);
        }

        /** Create a 0 resistance object
         * 
         * @return object with all resistances 0
         */
        inline static resistance make_zero()
        {
            return resistance{ 0, 0, 0, 0 };    
        }

        /** Get fire resistance
         * 
         * @return fire resistance value
         */
        inline item_t fire() const
        {
            return static_cast<item_t>((value >> 48) & 0xFFFF);
        }
        
        /** Get cold resistance
         * 
         * @return cold resistance value
         */
        inline item_t cold() const
        {
            return static_cast<item_t>((value >> 32) & 0xFFFF);
        }
        
        /** Get lightning resistance
         * 
         * @return lightning resistance value
         */
        inline item_t lightning() const
        {
            return static_cast<item_t>((value >> 16) & 0xFFFF);
        }
        
        /** Get chaos resistance
         * 
         * @return chaos resistance value
         */
        inline item_t chaos() const
        {
            return static_cast<item_t>(value & 0xFFFF);
        }

        /** Add 2 resistances together
         * 
         * @param other Right hand side of the operator
         * 
         * @returns new resistance
         */
        inline resistance operator+(const resistance& other) const
        {
            return resistance{
                static_cast<item_t>(fire() + other.fire()),
                static_cast<item_t>(cold() + other.cold()),
                static_cast<item_t>(lightning() + other.lightning()),
                static_cast<item_t>(chaos() + other.chaos())
            };
        }

        /** Subtract a resistance object from this object.
         * 
         * If a result becomes negative, it is set to 0.
         * 
         * @param other Right hand side of the operator
         * 
         * @returns new resistance
         */
        inline resistance operator-(const resistance& other) const
        {
            auto cond_sub = [](auto&& lhs, auto&& rhs)
            {
                return static_cast<item_t>(lhs >= rhs ? lhs - rhs : 0);
            };

            return resistance{
                cond_sub(fire(), other.fire()),
                cond_sub(cold(), other.cold()),
                cond_sub(lightning(), other.lightning()),
                cond_sub(chaos(), other.chaos())
            };
        }
        
        // comparison operators

        inline bool operator>(const resistance& other) const
        {
            return fire() > other.fire() && 
                    cold() > other.cold() && 
                    lightning() > other.lightning() && 
                    chaos() > other.chaos();
        }
        
        inline bool operator>=(const resistance& other) const
        {
            return fire() >= other.fire() && 
                    cold() >= other.cold() && 
                    lightning() >= other.lightning() && 
                    chaos() >= other.chaos();
        }

        inline bool operator<=(const resistance& other) const
        {
            return fire() <= other.fire() && 
                    cold() <= other.cold() && 
                    lightning() <= other.lightning() && 
                    chaos() <= other.chaos();
        }
        
        inline bool operator<(const resistance& other) const
        {
            return fire() < other.fire() && 
                    cold() < other.cold() && 
                    lightning() < other.lightning() && 
                    chaos() < other.chaos();
        }
        
        inline bool operator==(const resistance& other) const
        {
            return value == other.value;
        }
        
        inline bool operator!=(const resistance& other) const
        {
    return value != other.value;
        }
    };
}

#endif // RECAP_RESISTANCE_HPP_