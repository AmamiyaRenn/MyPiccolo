#pragma once

#include "core/base/macro.h"
#include "core/meta/serializer/serializer.h"
#include "function/global/global_context.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace Piccolo
{
    class AssetManager
    {
    public:
        // 获得relative path的absolute path
        static std::filesystem::path getFullPath(const std::string& relative_path)
        {
            return std::filesystem::absolute(g_runtime_global_context.m_config_manager->getRootFolder() /
                                             relative_path);
        }

        template<typename AssetType>
        bool loadAsset(const std::string& asset_url, AssetType& out_asset) const
        {
            // read json file to string
            std::filesystem::path asset_path = getFullPath(asset_url);
            std::ifstream         asset_json_file(asset_path);
            if (!asset_json_file)
            {
                LOG_ERROR("open file : {} failed !", asset_path.generic_string());
                return false;
            }

            std::stringstream buffer;
            buffer << asset_json_file.rdbuf();

            // parse to json object and read to runtime res object
            std::string error;
            auto&&      asset_json = Json::parse(buffer.str(), error);
            if (!error.empty())
            {
                LOG_ERROR("parse json file {} failed!", asset_url);
                return false;
            }

            Serializer::read(asset_json, out_asset);
            return true;
        }

        template<typename AssetType>
        bool saveAsset(const AssetType& out_asset, const std::string& asset_url) const
        {
            std::ofstream asset_json_file(getFullPath(asset_url));
            if (!asset_json_file)
            {
                LOG_ERROR("open file {} failed!", asset_url);
                return false;
            }

            // write to json object and dump to string
            std::string&& asset_json_text = Serializer::write(out_asset).dump();

            // write to file
            asset_json_file << asset_json_text;
            asset_json_file.flush();

            return true;
        }
    };
} // namespace Piccolo