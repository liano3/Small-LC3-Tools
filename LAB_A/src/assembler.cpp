/*
 * @Author       : Chivier Humber
 * @Date         : 2021-08-30 15:10:31
 * @LastEditors  : lining
 * @LastEditTime : 2022-12-30 15:07:45
 * @Description  : content for samll assembler
 */

#include "assembler.h"
#include <string>

// 添加 label 至 label_map
void LabelMapType::AddLabel(const std::string &str, const unsigned address)
{
    labels_.insert({str, address});
}

// 查找 label_map
unsigned LabelMapType::GetAddress(const std::string &str) const
{
    if (labels_.find(str) == labels_.end())
    {
        // not found
        return -1;
    }
    return labels_.at(str);
}

// 翻译操作对象
std::string assembler::TranslateOprand(unsigned int current_address, std::string str, int opcode_length)
{
    // 去除首尾空白
    str = Trim(str);
    auto item = label_map.GetAddress(str);
    // 是 label
    if (item != -1)
    {
        int offset = item - current_address - 1;
        std::string tmp = NumberToAssemble(offset);
        return tmp.substr(16 - opcode_length);
    }
    // 是寄存器
    if (str[0] == 'R')
    {
        std::string tmp = NumberToAssemble(str[1] - '0');
        return tmp.substr(13);
    }
    // 是立即数
    else
    {
        std::string tmp = NumberToAssemble(str);
        return tmp.substr(16 - opcode_length);
    }
}

// 处理 label
std::string assembler::LineLabelSplit(const std::string &line, int current_address)
{
    auto first_whitespace_position = line.find(' ');
    auto first_token = line.substr(0, first_whitespace_position);
    // 是 label
    if (IsLC3Pseudo(first_token) == -1 && IsLC3Command(first_token) == -1 && IsLC3TrapRoutine(first_token) == -1)
    {
        // 添加至 label_map
        label_map.AddLabel(first_token, current_address);
        // 删去 label
        if (first_whitespace_position == std::string::npos)
        {
            return "";
        }
        auto command = line.substr(first_whitespace_position + 1);
        return Trim(command);
    }
    return line;
}

// 第一遍扫描，保存指令和 label 地址
int assembler::firstPass(std::string &input_filename)
{
    std::string line;
    std::ifstream input_file(input_filename);
    if (!input_file.is_open())
    {
        std::cout << "Unable to open file" << std::endl;
        // @ Input file read error
        return -1;
    }
    int orig_address = -1;
    int current_address = -1;
    // 按行处理
    while (std::getline(input_file, line))
    {
        line = FormatLine(line);
        if (line.empty())
            continue;

        auto command = LineLabelSplit(line, current_address);
        if (command.empty())
            continue;

        // 获取指令类型
        auto first_whitespace_position = command.find(' ');
        auto first_token = command.substr(0, first_whitespace_position);

        // 特判 .ORIG and .END
        if (first_token == ".ORIG")
        {
            std::string orig_value = command.substr(first_whitespace_position + 1);
            orig_address = RecognizeNumberValue(orig_value);
            if (orig_address == std::numeric_limits<int>::max())
            {
                // @ Error address
                return -2;
            }
            current_address = orig_address;
            continue;
        }

        if (orig_address == -1)
        {
            // @ Error Program begins before .ORIG
            return -3;
        }

        if (first_token == ".END")
        {
            break;
        }

        // 根据指令内容修改当前地址
        // 操作指令
        if (IsLC3Command(first_token) != -1 || IsLC3TrapRoutine(first_token) != -1)
        {
            commands.push_back({current_address, command, CommandType::OPERATION});
            current_address += 1;
            continue;
        }

        // 伪操作
        commands.push_back({current_address, command, CommandType::PSEUDO});
        auto operand = command.substr(first_whitespace_position + 1);
        if (first_token == ".FILL")
        {
            auto num_temp = RecognizeNumberValue(operand);
            if (num_temp == std::numeric_limits<int>::max())
            {
                // @ Error Invalid Number input @ FILL
                return -4;
            }
            if (num_temp > 65535 || num_temp < -65536)
            {
                // @ Error Too large or too small value  @ FILL
                return -5;
            }
            current_address += 1;
        }
        if (first_token == ".BLKW")
        {
            int num_temp = RecognizeNumberValue(operand);
            current_address += num_temp;
        }
        if (first_token == ".STRINGZ")
        {
            current_address += (command.find_last_of("\"") - command.find_first_of("\""));
        }
    }
    // OK flag
    return 0;
}

// 翻译伪操作
std::string assembler::TranslatePseudo(std::stringstream &command_stream)
{
    std::string pseudo_opcode;
    std::string output_line;
    command_stream >> pseudo_opcode;
    // .FILL
    if (pseudo_opcode == ".FILL")
    {
        // 填充 .FILL 后面的值
        std::string number_str;
        command_stream >> number_str;
        output_line = NumberToAssemble(number_str);
        if (gIsHexMode)
            output_line = ConvertBin2Hex(output_line);
    }
    // .BLKW
    else if (pseudo_opcode == ".BLKW")
    {
        // 填充 0
        std::string number_str;
        command_stream >> number_str;
        int num = RecognizeNumberValue(number_str);
        std::string temp = NumberToAssemble(0);
        if (gIsHexMode)
            temp = ConvertBin2Hex(output_line);
        output_line = temp;
        // 填充个数由 .BLKW 后面的值决定
        while (--num)
        {
            output_line += "\n" + temp;
        }
    }
    else if (pseudo_opcode == ".STRINGZ")
    {
        // 填充 .STRINGZ 后面的字符串
        std::string str;
        command_stream >> str;
        str = str.substr(str.find_first_of("\"") + 1, str.find_last_of("\"") - str.find_first_of("\"") - 1);
        for (int i = 0; i <= str.size(); ++i)
        {
            output_line += NumberToAssemble(str[i]);
            if (i < str.size())
                output_line += "\n";
        }
    }
    return output_line;
}

// 翻译操作指令
std::string assembler::TranslateCommand(std::stringstream &command_stream, unsigned int current_address)
{
    // 获取指令编号
    std::string opcode;
    command_stream >> opcode;
    auto command_tag = IsLC3Command(opcode);
    
    // 获取操作对象
    std::vector<std::string> operand_list;
    std::string operand;
    while (command_stream >> operand)
    {
        operand_list.push_back(operand);
    }
    auto operand_list_size = operand_list.size();

    std::string output_line;
    
    // Trap 指令
    if (command_tag == -1)
    {
        command_tag = IsLC3TrapRoutine(opcode);
        output_line = kLC3TrapMachineCode[command_tag];
    }
    // 操作指令
    else
    {
        switch (command_tag)
        {
        case 0:
            // "ADD"
            output_line += "0001";
            if (operand_list_size != 3)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            // 第三个操作对象是寄存器
            if (operand_list[2][0] == 'R')
            {
                output_line += "000";
                output_line += TranslateOprand(current_address, operand_list[2]);
            }
            // 是立即数
            else
            {
                output_line += "1";
                output_line += TranslateOprand(current_address, operand_list[2], 5);
            }
            break;
        case 1:
            // "AND"
            output_line += "0101";
            if (operand_list_size != 3)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            if (operand_list[2][0] == 'R')
            {
                // 第三个操作对象是寄存器
                output_line += "000";
                output_line += TranslateOprand(current_address, operand_list[2]);
            }
            else
            {
                // 是立即数
                output_line += "1";
                output_line += TranslateOprand(current_address, operand_list[2], 5);
            }
            break;
        case 2:
            // "BR"
            output_line += "0000111";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 3:
            // "BRN"
            output_line += "0000100";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 4:
            // "BRZ"
            output_line += "0000010";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 5:
            // "BRP"
            output_line += "0000001";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 6:
            // "BRNZ"
            output_line += "0000110";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 7:
            // "BRNP"
            output_line += "0000101";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 8:
            // "BRZP"
            output_line += "0000011";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 9:
            // "BRNZP"
            output_line += "0000111";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 9);
            break;
        case 10:
            // "JMP"
            output_line += "1100000";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += "000000";
            break;
        case 11:
            // "JSR"
            output_line += "01001";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 11);
            break;
        case 12:
            // "JSRR"
            output_line += "0100000";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += "000000";
            break;
        case 13:
            // "LD"
            output_line += "0010";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 14:
            // "LDI"
            output_line += "1010";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 15:
            // "LDR"
            output_line += "0110";
            if (operand_list_size != 3)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            output_line += TranslateOprand(current_address, operand_list[2], 6);
            break;
        case 16:
            // "LEA"
            output_line += "1110";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 17:
            // "NOT"
            output_line += "1001";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            output_line += "111111";
            break;
        case 18:
            // RET
            output_line += "1100000111000000";
            if (operand_list_size != 0)
            {
                // @ Error operand numbers
                exit(-30);
            }
            break;
        case 19:
            // RTI
            output_line += "1000000000000000";
            if (operand_list_size != 0)
            {
                // @ Error operand numbers
                exit(-30);
            }
            break;
        case 20:
            // ST
            output_line += "0011";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 21:
            // STI
            output_line += "1011";
            if (operand_list_size != 2)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1], 9);
            break;
        case 22:
            // STR
            output_line += "0111";
            if (operand_list_size != 3)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0]);
            output_line += TranslateOprand(current_address, operand_list[1]);
            output_line += TranslateOprand(current_address, operand_list[2], 6);
            break;
        case 23:
            // TRAP
            output_line += "11110000";
            if (operand_list_size != 1)
            {
                // @ Error operand numbers
                exit(-30);
            }
            output_line += TranslateOprand(current_address, operand_list[0], 8);
            break;
        default:
            // Unknown opcode
            // @ Error
            break;
        }
    }

    if (gIsHexMode)
        output_line = ConvertBin2Hex(output_line);

    return output_line;
}

// 第二遍扫描，把格式化的 commands 翻译为机器码
int assembler::secondPass(std::string &output_filename)
{
    std::ofstream output_file;
    output_file.open(output_filename);
    if (!output_file)
    {
        // @ Error at output file
        return -20;
    }

    for (const auto &command : commands)
    {
        const unsigned address = std::get<0>(command);
        const std::string command_content = std::get<1>(command);
        const CommandType command_type = std::get<2>(command);
        auto command_stream = std::stringstream(command_content);
        // 翻译为机器码并写进输出文件
        if (command_type == CommandType::PSEUDO)
        {
            output_file << TranslatePseudo(command_stream) << std::endl;
        }
        else
        {
            output_file << TranslateCommand(command_stream, address) << std::endl;
        }
    }

    output_file.close();
    // OK flag
    return 0;
}

// 主函数，两遍扫描
int assembler::assemble(std::string &input_filename, std::string &output_filename)
{
    auto first_scan_status = firstPass(input_filename);
    if (first_scan_status != 0)
    {
        return first_scan_status;
    }
    auto second_scan_status = secondPass(output_filename);
    if (second_scan_status != 0)
    {
        return second_scan_status;
    }
    // OK flag
    return 0;
}
