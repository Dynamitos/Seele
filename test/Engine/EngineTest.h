#pragma once
#include <vld.h>

namespace Seele
{
    struct GlobalFixture
    {
        GlobalFixture()
        {
            _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        }
        ~GlobalFixture();
        void setup()
        {
        }
        void teardown()
        {
        }
    };
};