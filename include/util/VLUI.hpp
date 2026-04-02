#ifndef _TRIMEGO_UTILS_VLUI_H
#define _TRIMEGO_UTILS_VLUI_H

#include "Math.hpp"
#include <map>
#include <cstdint>
#include <utility>
#include <functional>
#include <vector>
#include "CTTI.h"

namespace Trimego::Util
{
    class VLUI final //this class is used to represent variable length bitsets.
    {
    public:
        VLUI()
        {

        }

        VLUI(const VLUI& source)
        {
            elements = source.elements;
        }

        /**
         * @brief constructs a VLUI.
         * @param signature a list of bit positions  to set.
         */
        VLUI(std::initializer_list<std::uint64_t> signature)
        {
            for (std::uint64_t bitPosition : signature)
            {
                setBit(bitPosition);
            }
        }

        /**
         * @brief sets a bit to 1.
         * @param position index of the bit to set.
         */
        void setBit(std::uint64_t position)
        {
            uint64_t index = position / 64;
            elements[index] = elements[index] | (std::uint64_t)pow(2, position % 64);
        }

        /**
         * @brief sets a bit to 0.
         * @param bitPosition index of the bit to set.
         */
        void unsetBit(int bitPosition) noexcept
        {
            uint64_t index = bitPosition / 64;
            elements[index] =
                elements[index] & ~(std::uint64_t)pow(2, bitPosition % 64);
            if (elements[index] == 0)
                elements.erase(index);
        }

        /**
         * @brief computes a bitwise OR.
         * @param op a reference the right operand.
         * @return the result of the OR operation as a new VLUI.
         */
        [[nodiscard]] VLUI OR(const VLUI &op) const
        {
            auto it1 = elements.begin();
            auto it2 = op.elements.begin();

            VLUI _OR;

            for (; it1 != elements.end() && it2 != op.elements.end();)
            {
                auto element1 = it1->second;
                auto element2 = it2->second;
                _OR.elements[it1->first] |= element1;
                _OR.elements[it2->first] |= element2;
                it1++;
                it2++;
            }

            if (it2 != op.elements.end())
            {
                for (;it2 != op.elements.end();)
                {
                    auto element2 = it2->second;
                    _OR.elements[it2->first] |= element2;
                    it2++;
                }
            }
            else if(it1 != elements.end())
            {
                for (;it1 != elements.end();)
                {
                    auto element1 = it1->second;
                    _OR.elements[it1->first] |= element1;
                    it1++;
                }
            }

            return _OR;
        }

        /**
         * @brief checks if a VLUI is included in this.
         * @param op a reference the right operand.
         * @return true if op is included in this, false otherwise.
         */
        [[nodiscard]] bool includes(const VLUI &op) const
        {
            auto it1 = elements.begin();
            auto it2 = op.elements.begin();

            for (; it1 != elements.end() && it2 != op.elements.end();)
            {
                if (it1->first < it2->first)
                {
                    it1++;
                }
                else if (it1->first > it2->first)
                    return false;
                else
                {
                    if((~it1->second & it2->second) != 0)
                        return false;
                    else
                    {
                        it1++;
                        it2++;
                    }
                }
            }

            if (it2 != op.elements.end())
                return false;

            return true;
        }

        /**
         * @brief checks if a VLUI is included in this.
         * @param op an r value reference the right operand.
         * @return true if op is included in this, false otherwise.
         */
        [[nodiscard]] bool includes(VLUI &&op) const noexcept
        {
            auto it1 = elements.begin();
            auto it2 = op.elements.begin();

            for (; it1 != elements.end() && it2 != op.elements.end();)
            {
                if (it1->first < it2->first)
                {
                    it1++;
                }
                else if (it1->first > it2->first)
                    return false;
                else
                {
                    if((~it1->second & it2->second) != 0)
                        return false;
                    else
                    {
                        it1++;
                        it2++;
                    }
                }
            }

            if (it2 != op.elements.end())
                return false;

            return true;
        }

        /**
         * @brief computes a bitwise AND NOT.
         * @param op a reference to the bitmask.
         * @return the result of the bitwise AND NOT operation as a new VLUI.
         */
        [[nodiscard]] VLUI excludeBits(const VLUI &op) const
        {
            auto it1 = elements.begin();
            auto it2 = op.elements.begin();

            VLUI _AND_NOT;

            for(;it1 != elements.end() && it2 != op.elements.end();)
            {
                if(it1->first != it2->first)
                {
                    if(it1->first < it2->first)
                    {
                        _AND_NOT.elements[it1->first] = it1->second;
                        it1++;
                    }
                    else if(it2->first < it1->first)
                        it2++;
                }
                else
                {
                    _AND_NOT.elements[it1->first] = it1->second & ~it2->second;
                    if(_AND_NOT.elements[it1->first] == 0)
                        _AND_NOT.elements.erase(it1->first);
                    it1++;
                    it2++;
                }
            }

            return _AND_NOT;
        }

        /**
         * @brief computes a bitwise AND.
         * @param op a reference to the right operand.
         * @return the result of the bitwise AND operation as a new VLUI.
         */
        [[nodiscard]] VLUI AND(const VLUI &op) const
        {
            auto it1 = elements.begin();
            auto it2 = op.elements.begin();

            VLUI _AND;

            for (; it1 != elements.end() && it2 != op.elements.end();)
            {
                if (it1->first != it2->first)
                {
                    if (it1->first > it2->first)
                    {
                        _AND.elements.erase(it2->first);
                        it2++;
                    }
                    else
                    {
                        _AND.elements.erase(it1->first);
                        it1++;
                    }
                }
                else
                {
                    _AND.elements[it1->first] = it1->second & it2->second;
                    if (_AND.elements[it1->first] == 0)
                        _AND.elements.erase(it1->first);
                    it1++;
                    it2++;
                }
            }

            return _AND;
        }

        [[nodiscard]] inline auto  begin() const noexcept
        {
            return elements.begin();
        }

        [[nodiscard]] inline auto end() const noexcept
        {
            return elements.end();
        }

        [[nodiscard]] inline auto size() const noexcept
        {
            return elements.size();
        }

        /**
         * @brief allows the processing of each non zero bit inside this.
         * @param f the procedure iterating on each individual bit.
         */
        void iterateNonZeroBits(std::function<void(std::uint64_t)> &&f) const noexcept
        {
            for (auto [index, element] : elements)
            {
                while(element != 0)
                {
                    std::uint64_t msb = element;
                    msb   = base2log(msb);
                    f(index * 64 + msb);
                    element -= (std::uint64_t)pow(2, msb);
                }
            }
        }

        /**
         * @brief checks if this is less than another VLUI, both are reinterpreted as unsigned integers.
         * @param op a reference to the right operand.
         */
        [[nodiscard]] bool operator<(const VLUI &op) const 
        {
            if(elements.size() == 0 && op.elements.size() == 0)
                return false;
            else if(elements.size() == 0)
                return true;
            else if(op.elements.size() == 0)
                return false;

            auto it1 = elements.end();
            auto it2 = op.elements.end();

            for(; it1 != elements.begin() && it2 != op.elements.begin();)
            {
                it1 = std::prev(it1);
                it2 = std::prev(it2);

                if(it1->first == it2->first) 
                {
                    if (it1->second < it2->second)
                        return true;
                    else if (it1->second > it2->second)
                        return false;
                }
                else if (it1->first < it2->first)
                    return true;
                else
                    return false;
            }

            if(it2 != op.elements.begin())
                return true;

            return false;
        }

        VLUI& operator=(VLUI&& op)noexcept
        {
            elements = std::move(op.elements);
            op.elements.clear();
            return *this;
        }

        /**
         * @brief checks if this equals another VLUI.
         * @param op an r value reference the right operand.
         * @return true if this equals op, false otherwise.
         */
        [[nodiscard]] bool operator==(const VLUI& op) const noexcept
        {
            auto it1 = elements.begin();
            auto it2 = op.elements.begin();

            for(;it1 != elements.end() && it2 != op.elements.end();)
            {
                if(it1->first < it2->first || it2->first < it1->first)
                    return false;
                else if(it1->second != it2->second)
                    return false;
                it1++;
                it2++;
            }

            if(it1 != elements.end() || it2 != op.elements.end())
                return false;
            
            return true;
        }

        /**
         * @brief clears this and stores its elements as garbage to free later.
         */
        void clear()noexcept
        {
            std::map<std::uint64_t, std::uint64_t> garbage;
            elements.swap(garbage);
            bin.push_back(std::move(garbage));
        }

    private:
        std::map<std::uint64_t, std::uint64_t> elements;
        std::vector<std::map<std::uint64_t, std::uint64_t>> bin;
    };
}  // namespace Trimego

template <>
class std::hash<Trimego::Util::VLUI>
{
public:
    std::size_t operator()(const Trimego::Util::VLUI &key) const noexcept
    {
        std::uint64_t seed = key.size();
        for (auto it = key.begin(); it != key.end(); it++)
        {
            auto x = it->second;
            x = ((x >> 32) ^ x) * 0xff51afd7ed558ccd;
            x = ((x >> 32) ^ x) * 0xc4ceb9fe1a85ec53;
            x = (x >> 32) ^ x;
            seed ^= x + 0x9e3779b9 + (seed << 12) + (seed >> 4);
        }
        return seed;
    }
};

#endif
