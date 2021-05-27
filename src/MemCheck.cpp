#include "MemCheck.h"

// 构造函数
MemCheck::MemCheck()
{}

// 析构函数
MemCheck::~MemCheck()
{}

// 存储分配内存信息
int32_t MemCheck::New(void *addr, size_t size)
{
    // 空指针判断
    if (nullptr == addr) 
        return MC_FAIL;

    // 分配情况统计
    m_useBytes += size;
    ++m_useBlocks;
    ++m_allocs;
    m_allocated += size;

    // 获取栈信息
    void *buf[BUFFER_SIZE];
    int32_t stack_size = backtrace(buf, BUFFER_SIZE);
    void **trace = (void**)backtrace_symbols(buf, stack_size);
    if (trace == nullptr)
        return MC_FAIL;
  
    // 计算map的键值
    uint32_t uMapKey = (uint64_t)addr % NS_MC::MAP_SIZE;
    std::map<uint32_t, NS_MC::MemSlot>::iterator iter = m_AddrMap.find(uMapKey);
    if (iter != m_AddrMap.end()) {
        // 查看结点是否存在，存在直接返回
        std::list<NS_MC::MemNode>::iterator it = iter->second.m_MemList.begin();
        for (; it != iter->second.m_MemList.end(); ++it)
        {
            if (it->addr == addr)
                return MC_OK;
        }

        // 构建新的结点，插入list当中
        NS_MC::MemNode _temp;
        _temp.addr = addr;
        _temp.addr_size = size;
        _temp.stack = trace;
        _temp.stack_size = stack_size;
        iter->second.m_MemList.push_back(_temp);
    }

    // 没找到map结点，则插入新的map结点
    NS_MC::MemSlot _slot;
    NS_MC::MemNode _temp;
    _temp.addr = addr;
    _temp.addr_size = size;
    _temp.stack = trace;
    _temp.stack_size = size;
    _slot.m_MemList.push_back(_temp);
    m_AddrMap[uMapKey] = _slot;

    return MC_OK;
}

// 释放分配内存信息
int32_t MemCheck::Delete(void *addr)
{
    // 空指针判断
    if (nullptr == addr)
        return MC_FAIL;

    // 释放信息统计
    --m_useBlocks;
    --m_frees;

    // 计算map的键值，并找到该地址对应结点，将其删除    
    uint32_t uMapKey = (uint64_t)addr % NS_MC::MAP_SIZE;
    std::map<uint32_t, NS_MC::MemSlot>::iterator iter = m_AddrMap.find(uMapKey);
    if (iter != m_AddrMap.end()) {
        std::list<NS_MC::MemNode>::iterator it = iter->second.m_MemList.begin();
        for (; it != iter->second.m_MemList.end(); ++it)
        {
            if (it->addr == addr) {
                m_useBytes -= it->addr_size;
                iter->second.m_MemList.erase(it);
                return MC_OK;
            }
        }
    }

    return MC_OK;
}

// 输出内存泄漏的日志
int32_t MemCheck::WriteLog()
{
    // 拼接输出文件的名称
    char file_name[BUFFER_SIZE];
    time_t nowtime = time(nullptr);
    struct tm *local = localtime(&nowtime);
    snprintf(file_name, BUFFER_SIZE, "MemCheck_%04d%02d%02d%2d%2d%2d_%d.log", 
					local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, 
					local->tm_hour, local->tm_min, local->tm_sec, getpid());

    // 打开文件
    FILE *fd = fopen(file_name, "a+");
    if (nullptr == fd)
        return MC_FAIL;

    // 输出文件头
    fwrite(NS_MC::LOG_HEAD, NS_MC::LOG_HEAD_LEN, 1, fd);
    // 输出进程id
    char buf[BUFFER_SIZE];
    uint8_t len = snprintf(buf, BUFFER_SIZE, "ParentId: %d\n\n", getpid());
    fwrite(buf, len, 1, fd);
    len = snprintf(buf, BUFFER_SIZE, "HEAP SUMMARY:\n \
					in use at now: %d bytes in %d blocks\n \
					total heap usage: %d allocs, %d frees, %d bytes allocated\n\n\n",
					m_useBytes, m_useBlocks, m_allocs, m_frees, m_allocated);
    fwrite(buf, len, 1, fd);

    // 将存储信息map当前泄漏的点转移到临时map上
    std::map<uint32_t, NS_MC::MemSlot> m_AddrMapTemp;
    std::map<uint32_t, NS_MC::MemSlot>::iterator iter = m_AddrMap.begin();
    for (; iter != m_AddrMap.end(); ++iter) {
        std::list<NS_MC::MemNode>::iterator it = iter->second.m_MemList.begin();
        for (; it != iter->second.m_MemList.end(); ++it)
        {
            NS_MC::MemNode _temp;
            _temp.addr = it->addr;
            _temp.addr_size = it->addr_size;
            _temp.stack = it->stack;
            _temp.stack_size = it->stack_size;
            std::map<uint32_t, NS_MC::MemSlot>::iterator it_temp;
            it_temp = m_AddrMapTemp.find(iter->first);
            if (it_temp != m_AddrMapTemp.end()) {
                it_temp->second.m_MemList.push_back(_temp);
                continue;
            }

            NS_MC::MemSlot _slot;
            _slot.m_MemList.push_back(_temp);
            m_AddrMapTemp[iter->first] = _slot;
        }
    }

    // 输出泄漏信息
    iter = m_AddrMapTemp.begin();
    for (; iter != m_AddrMapTemp.end(); ++iter) {
        std::list<NS_MC::MemNode>::iterator output = iter->second.m_MemList.begin();
        for (; output != iter->second.m_MemList.end(); ++output)
        {
            // 将栈信息做处理（系统返回的带乱码）
            OutputStack(fd, *output);
        }
    }

    len = snprintf(buf, BUFFER_SIZE, "LEAK SUMMARY:\n \
					total lost at now: %d bytes in %d blocks\n",
					m_useBytes, m_useBlocks);
    fwrite(buf, len, 1, fd);

    // 关闭文件
    fclose(fd);
    return MC_OK;
}

void MemCheck::OutputStack(FILE *fd, NS_MC::MemNode _addr)
{
    // 空指针判断
    if (fd == nullptr || _addr.stack == nullptr || _addr.addr == nullptr)
        return;
    
    for (int32_t Index = 0; Index < _addr.stack_size; ++Index)
    {
        char buf[BUFFER_SIZE];
        int32_t len = snprintf(buf, BUFFER_SIZE, "%s\n", _addr.stack[Index]);
        fwrite(buf, len, 1, fd);
    }
    fwrite("\n\n", 2, 1, fd);

    free(_addr.stack);
    return;
}
