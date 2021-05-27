#ifndef _MEM_CHECK_H_
#define _MEM_CHECK_H_

#include <iostream>
#include <map>
#include <list>
#include <execinfo.h>
#include <unistd.h>
#include <sys/types.h>
#include <cxxabi.h>
#include <string.h>

// 返回成功标志
#define MC_OK 0
// 返回失败标志
#define MC_FAIL -1
// 栈信息缓冲区大小
#define BUFFER_SIZE 128

namespace NS_MC
{
    class MemNode
    {
        public:
            /**
             * @brief 构造函数
             */
            MemNode()
            {
                addr_size = 0;
                stack_size = 0;
                addr = nullptr;
                stack = nullptr;
            }

            /**
             * @brief 析构函数
             */
            ~MemNode()
            {
                addr_size = 0;
                stack_size = 0;
                addr = nullptr;
                stack = nullptr;
            }

        public:
            void *addr;         // 地址信息
            int32_t addr_size;  // 地址大小
            void **stack;       // 栈信息
            int32_t stack_size;  // 栈层数
    };

    class MemSlot
    {
        public:
            /**
             * @brief 构造函数
             */
            MemSlot()
            {
            }

            /**
             * @brief 析构函数
             */
            ~MemSlot()
            {
            }

        public:
            std::list<NS_MC::MemNode> m_MemList;
    };

    // map大小
    const int32_t MAP_SIZE = 17777;
    // 日志文件头
    const char * const LOG_HEAD = "Memcheck, a memory error detector\n";
    // 日志头长度
    const int32_t LOG_HEAD_LEN = 34;
}

class MemCheck
{
    public:
        /**
         * @brief 析构函数
         */
        ~MemCheck();

        /**
         * @brief 删除拷贝构造和重载赋值
         */
        MemCheck(MemCheck const &) = delete;
        void operator=(MemCheck const&) = delete;

        /**
         * @brief 返回单例函数
         *
         * @return 单例
         */
        static MemCheck &getInstance()
        {
            static MemCheck _instance;
            return _instance;
        }

        /**
         * @brief new内存存储栈信息
         *
         * @prame p 分配的地址，size 内存长度
         *
         * @return MC_OK 成功；MC_FAIL 失败
         */
        int32_t New(void *addr, size_t size);

        /**
         * @breif delete内存释放存储信息
         *
         * @prame p 分配的地址
         *
         * @return MC_OK 成功；MC_FAIL 失败
         */
        int32_t Delete(void *addr);

        /**
         * @brief 输出日志
         *
         * @return MC_OK 成功；MC_FAIL 失败
         */
        int32_t WriteLog();

    private:
        /**
         * @brief 构造函数
         */
        MemCheck();

        /**
         * @brief 对栈信息做相应处理
         *
         * @prame fd 文件指针
         */
        void OutputStack(FILE *fd, NS_MC::MemNode _addr);

    private:
        /**
         * @brief 存储内存信息结果
         */
        std::map<uint32_t, NS_MC::MemSlot> m_AddrMap;

        /**
         * @brief 当前还在使用的总内存 
         */
        int32_t m_useBytes;

        /**
         * @brief 当前使用的内存总块数
         */
        int32_t m_useBlocks;

        /**
         * @brief 共分配总数
         */
        int32_t m_allocs;

        /**
         * @brief 共释放总数
         */
        int32_t m_frees;

        /**
         * @brief 已经分配了多少内存
         */
        int32_t m_allocated;
};

#endif
