#include "quickbuildls.hpp"

#include <string>

size_t get_origin_index(const Origin &origin)
{
    if (std::holds_alternative<InputStreamPos>(origin))
        return std::get<InputStreamPos>(origin).index;
    if (std::holds_alternative<ObjectReference>(origin))
        return std::get<ObjectReference>(origin).size();

    return SIZE_MAX;
}

