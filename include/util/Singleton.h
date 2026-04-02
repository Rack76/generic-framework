#ifndef _TRIMEGO_SINGLETON_H_
#define _TRIMEGO_SINGLETON_H_

namespace Generic::Util
{

template <typename T>
class Singleton
{
public:
    /**
     * @brief For all singleton classes, they must consume this method
     * This will return a static instance to the same object, no matter
     * where it is called
     *
     * @return The object of the class singleton
     */
    [[nodiscard]] static T& getInstance() noexcept
    {
        static T t;
        return t;
    }
};

} // namespace Generic::Util

#endif