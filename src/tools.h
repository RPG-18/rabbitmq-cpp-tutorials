#ifndef TOOLS_H_
#define TOOLS_H_

#include <string>
#include <sstream>

#include "kashmir/uuid.h"
#include "kashmir/devrand.h"


template<typename Iterator>
std::string join(Iterator first, const Iterator last,
        const std::string& separator)
{
    std::string str;
    for (; first != last; ++first)
    {
        str.append(*first);
        if (first != (last - 1))
        {
            str.append(separator);
        }

    }
    return str;
}

std::string uuid()
{
    using kashmir::uuid_t;
    using kashmir::system::DevRand;
    DevRand devrandom;
    DevRand& in = devrandom;
    uuid_t uuid;
    in >> uuid;
    std::stringstream sstr;
    sstr<<uuid;
    return sstr.str();
}

#endif /* TOOLS_H_ */
