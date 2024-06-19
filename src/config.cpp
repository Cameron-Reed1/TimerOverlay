#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <string_view>


#include "config.h"


std::filesystem::path getConfigPath(const char* const configFolderName)
{
    std::filesystem::path config_dir = getenv("XDG_CONFIG_HOME");
    if (config_dir.empty()) {
        config_dir = getenv("HOME");
        if (config_dir.empty()) {
            return config_dir;
        }

        config_dir = config_dir / ".config";
    }


    return config_dir / configFolderName / "config.txt";
}

Config readConfig(std::filesystem::path config_path)
{
    Config config{};
    std::ifstream file(config_path);
    if (!file.is_open()) {
        return config;
    }

    int line_num = 0;
    std::string line;
    while (std::getline(file, line)) {
        line_num++;

        if (line.empty()) {
            continue;
        }

        size_t i = 0;
        while (line.at(i) == ' ' || line.at(i) == '\t') {
            i++;
            if (i == line.length()) {
                break;
            }
        }
        if (i == line.length()) {
            continue;
        }

        if (line.at(i) == '#') {
            continue;
        }

        size_t name_start = i;
        while (line.at(i) != ' ' && line.at(i) != '=') {
            i++;
            if (i == line.length()) {
                std::cerr << "Warning: Invalid line in config file" << std::endl;
                std::cerr << line_num << " | " << line << std::endl;
                break;
            }
        }
        if (i == line.length()) {
            continue;
        }
        std::string_view name = std::string_view(line).substr(name_start, i - name_start);

        while (line.at(i) == ' ' || line.at(i) == '\t') {
            i++;
            if (i == line.length()) {
                std::cerr << "Warning: Invalid line in config file" << std::endl;
                std::cerr << line_num << " | " << line << std::endl;
                break;
            }
        }
        if (i == line.length()) {
            continue;
        }

        if (line.at(i) != '=') {
            std::cerr << "Warning: Invalid line in config file" << std::endl;
            std::cerr << line_num << " | " << line << std::endl;
            continue;
        }
        i++;

        while (line.at(i) == ' ' || line.at(i) == '\t') {
            i++;
            if (i == line.length()) {
                std::cerr << "Warning: Invalid line in config file" << std::endl;
                std::cerr << line_num << " | " << line << std::endl;
                break;
            }
        }
        if (i == line.length()) {
            continue;
        }

        size_t value_start = i;
        while (line.at(i) != ' ' && line.at(i) != '\t' && line.at(i) != '#') {
            i++;
            if (i == line.length()) {
                break;
            }
        }
        std::string value = line.substr(value_start, i - value_start);

        if (i != line.length()) {
            while (line.at(i) == ' ' || line.at(i) == '\t') {
                i++;
            }
        }

        if (i != line.length()) {
            if (line.at(i) != '#') {
                std::cerr << "Warning: Invalid line in config file" << std::endl;
                std::cerr << line_num << " | " << line << std::endl;
                continue;
            }
        }

        if (name == "POSX") {
            config.pos_x = std::stoi(value);
        } else if (name == "POSY") {
            config.pos_y = std::stoi(value);
        } else if (name == "FONT") {
            config.font = value;
        } else if (name == "FONT_SIZE") {
            config.font_size = std::stoi(value);
        } else {
            std::cerr << "Warning: Invalid line in config file" << std::endl;
            std::cerr << line_num << " | " << line << std::endl;
        }
    }
    file.close();

    return config;
}


// int main()
// {
//     auto path = getConfigPath("timer_overlay");
//     readConfig(path);
// }

