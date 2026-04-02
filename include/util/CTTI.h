#ifndef _GENERIC_CTTI_H_
#define _GENERIC_CTTI_H_

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Generic::Util
{
template <typename T>
[[nodiscard]] constexpr std::string_view
typeName() noexcept
{
    std::string_view view   = __PRETTY_FUNCTION__;
    std::size_t      first  = view.find('=') + 2;
    std::size_t      last   = view.find(']');
    std::size_t      length = last - first;
    view                    = view.substr(first, length);
    return view;
}

[[nodiscard]] constexpr uint64_t
stringId(const std::string_view str) noexcept
{
    uint64_t hash = 14695981039346656037ull; // offset basis

    for (int i = 0; i < str.size(); i++)
    {
        hash ^= static_cast<uint64_t>(str[i]);
        hash *= 1099511628211ull; // FNV prime
    }

    return hash;
}

[[nodiscard]] constexpr uint64_t
typeId(const std::string_view typeName) noexcept
{
    constexpr uint64_t SEED = 0x9E3779B97F4A7C15;
    return stringId(typeName) ^ SEED;
}

template <typename T>
[[nodiscard]] inline constexpr std::uint64_t
typeId() noexcept
{
    return typeId(typeName<T>());
}

template <std::size_t s>
void
checkTypeIdCollision(const std::string_view (&typeNames)[s]) noexcept
{
    static std::unordered_map<std::uint64_t, std::string_view> typeIds;

    for (int i = 0; i < s; i++)
    {
        if (typeIds.find(typeId(typeNames[i])) != typeIds.end())
            assert(false && "checkTypeIdCollision : collision detected between " &&
                   typeNames[i].data() && "and " && typeIds[typeId(typeNames[i])].data());
        typeIds.insert({typeId(typeNames[i]), typeNames[i]});
    }
}

template <typename... Types>
constexpr void
checkTypeIdCollision() noexcept
{
    const std::string_view typeNames[sizeof...(Types)] = {typeName<Types>()...};
    checkTypeIdCollision(typeNames);
}
} // namespace Generic::Util

#endif