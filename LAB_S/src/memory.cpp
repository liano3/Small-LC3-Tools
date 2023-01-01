/*
 * @Author       : Chivier Humber
 * @Date         : 2021-09-15 21:15:24
 * @LastEditors  : lining
 * @LastEditTime : 2022-12-30 19:27:55
 * @Description  : file content
 */
#include "common.h"
#include "memory.h"

namespace virtual_machine_nsp
{
    // 将程序内容保存在对应内存中
    void memory_tp::ReadMemoryFromFile(std::string filename, int beginning_address)
    {
        // 打开文件
        std::string line;
        std::ifstream input_file(filename);
        if (!input_file.is_open())
        {
            std::cout << "Unable to open file" << std::endl;
            exit(-1);
        }
        // 按行读取
        while (std::getline(input_file, line))
        {
            int num = stoi(line, NULL, 2);
            memory[beginning_address++] = (int16_t)num;
        }
    }

    // 读取内存
    int16_t memory_tp::GetContent(int address) const
    {
        return memory[address];
    }

    // 重载[]操作符
    int16_t &memory_tp::operator[](int address)
    {
        if (address > 0xffff)
        {
            // @read memory error!
            std::cout << "read memory error!" << std::endl;
            exit(-2);
        }
        else
            return memory[address];
    }
}; // virtual machine namespace
