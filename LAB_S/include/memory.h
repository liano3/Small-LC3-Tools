/*
 * @Author       : Chivier Humber
 * @Date         : 2021-09-15 21:15:30
 * @LastEditors  : Chivier Humber
 * @LastEditTime : 2021-09-22 20:02:31
 * @Description  : file content
 */
#include "common.h"

namespace virtual_machine_nsp {
const int kInstructionLength = 16;

// string 转 int16_t
inline int16_t TranslateInstruction(std::string &line) {
    // TODO: translate hex mode
    int16_t result = 0;
    if (line.size() == kInstructionLength) {
        for (int index = 0; index < kInstructionLength; ++index) {
            result = (result << 1) | (line[index] & 1);
        }
    }
    return result;
}

// 内存类
const int kVirtualMachineMemorySize = 0xFFFF;
class memory_tp {
    private:
    // 内存数组
    int16_t memory[kVirtualMachineMemorySize];

    public:
    // 构造函数
    memory_tp() {
        memset(memory, 0, sizeof(int16_t) * kVirtualMachineMemorySize);
    }
    // 读取文件，保存程序
    void ReadMemoryFromFile(std::string filename, int beginning_address=0x3000);
    // 读取内存
    int16_t GetContent(int address) const;
    // []操作符重载
    int16_t& operator[](int address);
};

}; // virtual machine nsp