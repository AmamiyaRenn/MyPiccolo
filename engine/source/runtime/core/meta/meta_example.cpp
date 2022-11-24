#include "runtime/core/meta/meta_example.h"

#include "runtime/core/base/macro.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace Piccolo
{
    void metaExample()
    {
        Test1 test1_in;
        test1_in.m_int  = 12;
        test1_in.m_char = 'g';
        int i           = 1;
        test1_in.m_int_vector.emplace_back(&i);

        // reflection
        // auto meta = TypeMetaDef(Test2, &test2_out);
    }
} // namespace Piccolo