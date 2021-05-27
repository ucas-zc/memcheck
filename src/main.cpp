#include "MemTest.h"

int32_t main(int32_t argc, char **argv)
{
    MemTest *_temp = new MemTest();
    _temp->TestFunction();

    MemCheck::getInstance().WriteLog();
    return 0;
}
