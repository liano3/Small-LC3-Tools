/*
 * @Author       : Chivier Humber
 * @Date         : 2021-08-30 14:36:39
 * @LastEditors  : lining
 * @LastEditTime : 2022-12-30 14:50:42
 * @Description  : header file for small assembler
 */

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <bitset>

const int kLC3LineLength = 16;

extern bool gIsErrorLogMode;
extern bool gIsHexMode;

// 伪操作
const std::vector<std::string> kLC3Pseudos({
    ".ORIG",
    ".END",
    ".STRINGZ",
    ".FILL",
    ".BLKW",
});

// 操作指令
const std::vector<std::string> kLC3Commands({
    "ADD",   // 00: "0001" + reg(line[1]) + reg(line[2]) + op(line[3])
    "AND",   // 01: "0101" + reg(line[1]) + reg(line[2]) + op(line[3])
    "BR",    // 02: "0000111" + pcoffset(line[1],9)
    "BRN",   // 03: "0000100" + pcoffset(line[1],9)
    "BRZ",   // 04: "0000010" + pcoffset(line[1],9)
    "BRP",   // 05: "0000001" + pcoffset(line[1],9)
    "BRNZ",  // 06: "0000110" + pcoffset(line[1],9)
    "BRNP",  // 07: "0000101" + pcoffset(line[1],9)
    "BRZP",  // 08: "0000011" + pcoffset(line[1],9)
    "BRNZP", // 09: "0000111" + pcoffset(line[1],9)
    "JMP",   // 10: "1100000" + reg(line[1]) + "000000"
    "JSR",   // 11: "01001" + pcoffset(line[1],11)
    "JSRR",  // 12: "0100000"+reg(line[1])+"000000"
    "LD",    // 13: "0010" + reg(line[1]) + pcoffset(line[2],9)
    "LDI",   // 14: "1010" + reg(line[1]) + pcoffset(line[2],9)
    "LDR",   // 15: "0110" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "LEA",   // 16: "1110" + reg(line[1]) + pcoffset(line[2],9)
    "NOT",   // 17: "1001" + reg(line[1]) + reg(line[2]) + "111111"
    "RET",   // 18: "1100000111000000"
    "RTI",   // 19: "1000000000000000"
    "ST",    // 20: "0011" + reg(line[1]) + pcoffset(line[2],9)
    "STI",   // 21: "1011" + reg(line[1]) + pcoffset(line[2],9)
    "STR",   // 22: "0111" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "TRAP"   // 23: "11110000" + h2b(line[1],8)
});

// Trap指令
const std::vector<std::string> kLC3TrapRoutine({
    "GETC",  // x20
    "OUT",   // x21
    "PUTS",  // x22
    "IN",    // x23
    "PUTSP", // x24
    "HALT"   // x25
});
const std::vector<std::string> kLC3TrapMachineCode({
    "1111000000100000",
    "1111000000100001",
    "1111000000100010",
    "1111000000100011",
    "1111000000100100",
    "1111000000100101"
});

// 指令类型
enum CommandType
{
    OPERATION,
    PSEUDO
};

// 状态指示
static inline void SetErrorLogMode(bool error)
{
    gIsErrorLogMode = error;
}

// 十六进制形式
static inline void SetHexMode(bool hex)
{
    gIsHexMode = hex;
}

// Label类
class LabelMapType
{
private:
    std::unordered_map<std::string, unsigned> labels_;

public:
    void AddLabel(const std::string &str, unsigned address);    // 添加Label
    unsigned GetAddress(const std::string &str) const;  // 查找 label
};

// 伪操作判断
static inline int IsLC3Pseudo(const std::string &str)
{
    int index = 0;
    for (const auto &command : kLC3Pseudos)
    {
        if (str == command)
        {
            return index;
        }
        ++index;
    }
    return -1;
}

// 操作指令判断
static inline int IsLC3Command(const std::string &str)
{
    int index = 0;
    for (const auto &command : kLC3Commands)
    {
        if (str == command)
        {
            return index;
        }
        ++index;
    }
    return -1;
}

// Trap指令判断
static inline int IsLC3TrapRoutine(const std::string &str)
{
    int index = 0;
    for (const auto &trap : kLC3TrapRoutine)
    {
        if (str == trap)
        {
            return index;
        }
        ++index;
    }
    return -1;
}

// 字符转数字
static inline int CharToDec(const char &ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }
    return -1;
}

// 数字转字符
static inline char DecToChar(const int &num)
{
    if (num <= 9)
    {
        return num + '0';
    }
    return num - 10 + 'A';
}

// 去除首尾空白
static inline std::string &Trim(std::string &s)
{
    if (s.empty())
        return s;
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

// 格式化一行指令
static std::string FormatLine(const std::string &line)
{
    std::string ans = line;
    if (ans.empty())
        return ans;
    // 去除注释
    if (ans.find_first_of(";") != std::string::npos)
        ans.erase(ans.find_first_of(";"));
    for (int i = 0; i < ans.size(); ++i)
    {
        // 小写转大写
        if (ans[i] >= 'a' && ans[i] <= 'z')
            ans[i] += 'A' - 'a';
        // 逗号转空格
        else if (ans[i] == ',')
            ans[i] = ' ';
        // 转义字符转空格
        else if (ans[i] == '\t' || ans[i] == '\n' || ans[i] == '\r' || ans[i] == '\f' || ans[i] == '\v')
            ans[i] = ' ';
    }
    // 去除首尾空格
    return Trim(ans);
}

// 识别 string 中的数字转为 int
static int RecognizeNumberValue(const std::string &str)
{
    // 十进制
    if (str[0] == '#')
        return stoi(str.substr(1));
    // 十六进制
    else if (str[0] == 'X' || str[0] == 'x')
        return stoi(str.substr(1), NULL, 16);
    else
        return -1;
}

// int 转为二进制 string
static std::string NumberToAssemble(const int &number)
{
    std::bitset<16> bit(number);
    return bit.to_string();
}

// string 转为二进制 string
static std::string NumberToAssemble(const std::string &number)
{
    return NumberToAssemble(RecognizeNumberValue(number));
}

// 二进制 string 转为十六进制
static std::string ConvertBin2Hex(const std::string &bin)
{
    std::string tmp = bin;
    // 补位
    if (tmp.size() % 4 == 1)
        tmp = "000" + tmp;
    else if (tmp.size() % 4 == 2)
        tmp = "00" + tmp;
    else if (tmp.size() % 4 == 3)
        tmp = "0" + tmp;
    std::string ans;
    // 四位一转
    for (int i = 0; i < tmp.size(); i = i + 4)
    {
        std::string subtmp = tmp.substr(i, 4);
        int v = 0, w = 1;
        for (int j = 3; j >= 0; --j)
        {
            v += (subtmp[j] - '0') * w;
            w *= 2;
        }
        char c;
        if (v < 10)
            c = v + '0';
        else
            c = v - 10 + 'A';
        ans += c;
    }
    return ans;
}

// 汇编器类
class assembler
{
    // 指令模板(地址，指令，指令类型)
    using Commands = std::vector<std::tuple<unsigned, std::string, CommandType>>;

private:
    LabelMapType label_map; //label表
    Commands commands;  //指令
    
    // 翻译伪操作
    static std::string TranslatePseudo(std::stringstream &command_stream);
    // 翻译指令
    std::string TranslateCommand(std::stringstream &command_stream, unsigned int current_address);
    // 翻译操作对象
    std::string TranslateOprand(unsigned int current_address, std::string str, int opcode_length = 3);
    // 处理label
    std::string LineLabelSplit(const std::string &line, int current_address);
    // 第一遍扫描
    int firstPass(std::string &input_filename);
    // 第二遍扫描
    int secondPass(std::string &output_filename);

public:
    // 主体函数
    int assemble(std::string &input_filename, std::string &output_filename);
};
