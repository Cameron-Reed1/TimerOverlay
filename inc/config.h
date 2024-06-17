#include <filesystem>
#include <cstdint>
#include <string>


struct Config
{
    uint32_t pos_x = 10;
    uint32_t pos_y = 25;
    std::string font = "/usr/share/fonts/noto/NotoSans-Regular.ttf";
};


std::filesystem::path getConfigPath(const char* const configFolderName);
Config readConfig(std::filesystem::path config_path);

