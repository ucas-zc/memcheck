#ifndef _MEM_TEST_H_
#define _MEM_TEST_H_

#include <iostream>
#include <cstdint>
#include <cstdlib>

#ifdef _DEBUG_
#include "MemCheck.h"
#endif

class MemTest
{
    public:
        /**
         * @brief 构造函数
         */
        MemTest();

        /**
         * @brief 析构函数
         */
        ~MemTest();

        /**
         * @brief 测试接口
         */
        int32_t TestFunction();

        /**
         * @brief 重载operator new
         *
         * @prame size 内存大小
         *
         * @return 对象指针
         */
        static void * operator new(size_t size)
        {
            void *ptr = (void*)malloc(size);

#ifdef _DEBUG_
            MemCheck::getInstance().New(ptr, size);
#endif

            return ptr;
        }

        /**
         * @brief 重载operator delete
         *
         * @prame ptr 释放指针
         */
        static void operator delete(void *ptr)
        {
#ifdef _DEBUG_
            MemCheck::getInstance().Delete(ptr);
#endif

            free(ptr);
        }

    public:
        int32_t iTest;
};

#endif
