﻿#include "resource/config_manager/config_manager.h"

#include <fstream>
#include <iostream>
#include <string>

namespace Piccolo
{
    void ConfigManager::initialize(const std::filesystem::path& config_file_path)
    {
        // read configs
        std::ifstream config_file(config_file_path); // 从路径中读取文件

        std::string config_line;
        while (std::getline(config_file, config_line)) // 一行一行读ini
        {
            size_t seperate_pos = config_line.find_first_of('=');
            if (seperate_pos > 0 && seperate_pos < (config_line.length() - 1))
            {
                std::string name  = config_line.substr(0, seperate_pos);
                std::string value = config_line.substr(seperate_pos + 1, config_line.length() - seperate_pos - 1);

                if (name == "BinaryRootFolder")
                    m_root_folder = config_file_path.parent_path() / value;
                else if (name == "AssetFolder")
                    m_asset_folder = m_root_folder / value;
                else if (name == "SchemaFolder")
                    m_schema_folder = m_root_folder / value;
                else if (name == "DefaultWorld")
                    m_default_world_url = value;
                else if (name == "BigIconFile")
                    m_editor_big_icon_path = m_root_folder / value;
                else if (name == "SmallIconFile")
                    m_editor_small_icon_path = m_root_folder / value;
                else if (name == "FontFile")
                    m_editor_font_path = m_root_folder / value;
                else if (name == "GlobalRenderingRes")
                    m_global_rendering_res_url = value;
                else if (name == "GlobalParticleRes")
                    m_global_particle_res_url = value;
            }
        }
    }
} // namespace Piccolo