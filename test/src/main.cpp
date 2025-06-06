#include <csignal>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include "ts_transmit.h"

struct Param
{
    std::string input_file = "input.json";
};

bool app_running = true;

static void SigIntHandler(int sig_num)
{
    signal(SIGINT, SigIntHandler);
    app_running = false;
}

static std::shared_ptr<Param> ParseParam(int argc, char **argv)
{
    std::shared_ptr<Param> param = std::make_shared<Param>();
    for (int index = 1; index < argc; ++index)
    {
        std::string curr_arg = argv[index];

        if ("--help" == curr_arg)
        {
            return nullptr;
        }

        if ("-i" == curr_arg)
        {
            if (index + 1 >= argc)
            {
                return nullptr;
            }

            std::string next_arg = argv[++index];
            param->input_file = next_arg;
        }
    }

    return param;
}

static void ShowUsage()
{
    std::cerr << "Usage: ./ts_transmit_test [-i input file]" << std::endl;
}

std::string ReadFileToString(const std::string &file_name)
{
    std::ifstream fin(file_name.c_str());
    if (!fin.is_open())
    {
        return "";
    }

    std::ostringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

int main(int argc, char **argv)
{
    signal(SIGINT, SigIntHandler);

    std::shared_ptr<Param> param = ParseParam(argc, argv);
    if (!param)
    {
        ShowUsage();
        return 0;
    }

    std::string json_param = ReadFileToString(param->input_file);
    if (0 == json_param.length())
    {
        std::cerr << "Can't not read file: " << param->input_file << std::endl;
        return 0;
    }

    std::shared_ptr<tstm::ITransmitter> transmitter = tstm::CreateTransmitter(json_param.c_str());
    if (!transmitter)
    {
        return 0;
    }

    while (app_running)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
