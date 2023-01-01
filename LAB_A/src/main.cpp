/*
 * @Author       : Chivier Humber
 * @Date         : 2021-08-30 14:29:14
 * @LastEditors  : lining
 * @LastEditTime : 2022-12-30 15:16:53
 * @Description  : A small assembler for LC-3
 */

#include "assembler.h"

bool gIsErrorLogMode = false;
bool gIsHexMode = false;

// 参数解析
std::pair<bool, std::string> getCmdOption(char **begin, char **end, const std::string &option)
{
    char **itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return std::make_pair(true, *itr);
    }
    return std::make_pair(false, "");
}

// 判断某个参数是否存在
bool cmdOptionExists(char **begin, char **end, const std::string &option)
{
    return std::find(begin, end, option) != end;
}

int main(int argc, char **argv)
{
    // 输出汇编器的基本信息
    if (cmdOptionExists(argv, argv + argc, "-h"))
    {
        std::cout << "This is a simple assembler for LC-3." << std::endl << std::endl;
        std::cout << "\e[1mUsage\e[0m" << std::endl;
        std::cout << "./assembler \e[1m[OPTION]\e[0m ... \e[1m[FILE]\e[0m ..." << std::endl << std::endl;
        std::cout << "\e[1mOptions\e[0m" << std::endl;
        std::cout << "-h : print out help information" << std::endl;
        std::cout << "-f : the path for the input file" << std::endl;
        std::cout << "-e : print out error information" << std::endl;
        std::cout << "-o : the path for the output file" << std::endl;
        std::cout << "-s : hex mode" << std::endl;
        return 0;
    }

    // 获取输入输出文件参数
    auto input_info = getCmdOption(argv, argv + argc, "-f");
    std::string input_filename;
    auto output_info = getCmdOption(argv, argv + argc, "-o");
    std::string output_filename;

    // 检查输入文件名
    if (input_info.first)
    {
        input_filename = input_info.second;
    }
    else
    {
        input_filename = "input.txt";
    }

    if (output_info.first)
    {
        output_filename = output_info.second;
    }
    else
    {
        output_filename = "";
    }

    // 检查输出文件名
    if (output_filename.empty())
    {
        output_filename = input_filename;
        if (output_filename.find('.') == std::string::npos)
        {
            output_filename = output_filename + ".asm";
        }
        else
        {
            output_filename =
                output_filename.substr(0, output_filename.rfind('.'));
            output_filename = output_filename + ".asm";
        }
    }

    // 日志模式
    if (cmdOptionExists(argv, argv + argc, "-e"))
    {
        SetErrorLogMode(true);
    }
    // 十六进制模式
    if (cmdOptionExists(argv, argv + argc, "-s"))
    {
        SetHexMode(true);
    }

    auto ass = assembler();
    auto status = ass.assemble(input_filename, output_filename);

    if (gIsErrorLogMode)
    {
        std::cout << std::dec << status << std::endl;
    }
    return 0;
}
