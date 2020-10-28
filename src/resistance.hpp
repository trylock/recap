#ifndef RECAP_RESISTANCE_HPP_
#define RECAP_RESISTANCE_HPP_

#include <cassert>
#include <cstdint>

namespace recap
{
    // 4-tuple of resistances
    class resistance
    {
    public:
        using item_t = std::uint16_t; 

        // DefaultConstructible
        resistance() = default;

        // Copyable
        resistance(const resistance&) = default;
        resistance& operator=(const resistance&) = default;
        
        inline resistance(item_t fire, item_t cold, item_t lightning, item_t chaos) : 
            fire_(fire), 
            cold_(cold), 
            lightning_(lightning), 
            chaos_(chaos)
        {
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
            return fire_;
        }
        
        /** Get cold resistance
         * 
         * @return cold resistance value
         */
        inline item_t cold() const
        {
            return cold_;
        }
        
        /** Get lightning resistance
         * 
         * @return lightning resistance value
         */
        inline item_t lightning() const
        {
            return lightning_;
        }
        
        /** Get chaos resistance
         * 
         * @return chaos resistance value
         */
        inline item_t chaos() const
        {
            return chaos_;
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
            return fire() == other.fire() && 
                cold() == other.cold() && 
                lightning() == other.lightning() && 
                chaos() == other.chaos();
        }
        
        inline bool operator!=(const resistance& other) const
        {
            return !operator==(other);
        }

    private:
        item_t fire_;
        item_t cold_;
        item_t lightning_;
        item_t chaos_;
    };
}

#endif // RECAP_RESISTANCE_HPP_