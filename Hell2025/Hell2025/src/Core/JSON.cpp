#pragma once
#include "JSON.h"
#include "AssetManagement/AssetManager.h"
#include "Util.h"
#include <fstream>

namespace nlohmann {
    void to_json(nlohmann::json& j, const glm::vec3& v) {
        j = json::array({ v.x, v.y, v.z });
    }

    void to_json(nlohmann::json& j, const MeshRenderingInfo& info) {
        Material* material = AssetManager::GetMaterialByIndex(info.materialIndex);
        Mesh* mesh = AssetManager::GetMeshByIndex(info.meshIndex);

        if (!material) return;
        if (!mesh) return;

        j = nlohmann::json{
            {"meshName", mesh->GetName()},
            {"materialName", material ? material->m_name : DEFAULT_MATERIAL_NAME },
            {"blendingMode", Util::BlendingModeToString(info.blendingMode)}
        };
    }

    void to_json(nlohmann::json& j, const std::map<ivecXZ, std::string>& mapData) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& m : mapData) {
            nlohmann::json item;
            item["name"] = m.second;   // sector name
            item["x"] = m.first.x;     // x coordinate
            item["z"] = m.first.z;     // z coordinate
            arr.push_back(item);
        }
        j = arr;
    }

    void from_json(const nlohmann::json& j, glm::vec3& v) {
        v.x = j.at(0).get<float>();
        v.y = j.at(1).get<float>();
        v.z = j.at(2).get<float>();
    }

    void from_json(const nlohmann::json& j, MeshRenderingInfo& info) {
        std::string meshName;
        std::string materialName;
        std::string blendingModeString;

        j.at("meshName").get_to(meshName);
        j.at("materialName").get_to(materialName);
        j.at("blendingMode").get_to(blendingModeString);

        info.meshIndex = AssetManager::GetMeshIndexByName(meshName);
        info.materialIndex = AssetManager::GetMaterialIndexByName(materialName);
        info.blendingMode = Util::StringToBlendingMode(blendingModeString);
    }

    void from_json(const nlohmann::json& j, std::map<ivecXZ, std::string>& mapData) {
        mapData.clear();
        for (const auto& item : j) {
            int x = item.at("x").get<int>();
            int z = item.at("z").get<int>();
            std::string sectorName = item.at("name").get<std::string>();
            ivecXZ key(x, z);
            mapData[key] = sectorName;
        }
    }
}

namespace JSON {

    bool LoadJsonFromFile(nlohmann::json& json, const std::string filepath) {
        // Open the file
        std::ifstream file(filepath);
        if (!file) {
            std::cout << "JSON::LoadJsonFromFile() failed to open file: " << filepath << "\n";
            return false;
        }

        // Read the entire file into a string stream
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        // Try to parse the JSON
        try {
            json = nlohmann::json::parse(buffer.str());
            return true;
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON::LoadJsonFromFile() failed to parse the file " << filepath << ": " << e.what() << "\n";
            return false;
        }

    }

    SectorCreateInfo LoadSector(const std::string& filepath) {
        SectorCreateInfo info;

        // Try to parse the JSON
        nlohmann::json json;
        if (!LoadJsonFromFile(json, filepath)) {
            std::cerr << "JSON::LoadSector() failed to open file: " << filepath << "\n";
            return info;
        }

        FileInfo fileInfo = Util::GetFileInfoFromPath(filepath);

        SectorCreateInfo sectorCreateInfo;
        sectorCreateInfo.sectorName = fileInfo.name;

        sectorCreateInfo.heightMapName = json.value("HeightMapName", "");
        if (sectorCreateInfo.heightMapName == "None") {
            sectorCreateInfo.heightMapName = "";
        }

        // Load Game Objects
        for (auto& jsonObject : json["GameObjects"]) {
            GameObjectCreateInfo& createInfo = sectorCreateInfo.gameObjects.emplace_back();
            createInfo.position = jsonObject["position"];
            createInfo.rotation = jsonObject["rotation"];
            createInfo.scale = jsonObject["scale"];
            createInfo.modelName = jsonObject["modelName"];
            createInfo.meshRenderingInfoSet = jsonObject["meshRenderingInfo"];
        }

        // Load lights
        for (auto& jsonObject : json["Lights"]) {
            LightCreateInfo& createInfo = sectorCreateInfo.lights.emplace_back();
            createInfo.position = jsonObject["position"];
            createInfo.color = jsonObject["color"];
            createInfo.radius = jsonObject["radius"];
            createInfo.strength = jsonObject["strength"];
            createInfo.type = jsonObject["type"];
        }

        // Load Pickups
        for (auto& jsonObject : json["PickUps"]) {
            PickUpCreateInfo& createInfo = sectorCreateInfo.pickUps.emplace_back();
            createInfo.position = jsonObject["position"];
            createInfo.rotation = jsonObject["rotation"];
            createInfo.pickUpType = jsonObject["type"];
        }

        // Load Trees
        for (auto& jsonObject : json["Trees"]) {
            TreeCreateInfo& createInfo = sectorCreateInfo.trees.emplace_back();
            createInfo.position = jsonObject["position"];
            createInfo.rotation = jsonObject["rotation"];
            createInfo.scale = jsonObject["scale"];
            createInfo.type = jsonObject["type"];
        }

        return sectorCreateInfo;
    }

    void SaveSector(const std::string& filepath, SectorCreateInfo& sectorCreateInfo) {

        nlohmann::json json;

        json["HeightMapName"] = sectorCreateInfo.heightMapName;
        json["Lights"] = nlohmann::json::array();
        json["GameObjects"] = nlohmann::json::array();

        // Save Game Objects
        for (const GameObjectCreateInfo& createInfo : sectorCreateInfo.gameObjects) {
            json["GameObjects"].push_back(nlohmann::json{
                { "position", createInfo.position },
                { "rotation", createInfo.rotation },
                { "scale", createInfo.scale },
                { "modelName", createInfo.modelName },
                { "meshRenderingInfo", createInfo.meshRenderingInfoSet },
            });
        }

        // Save lights
        for (const LightCreateInfo& createInfo : sectorCreateInfo.lights) {
            json["Lights"].push_back(nlohmann::json{
                { "position", createInfo.position },
                { "color", createInfo.color },
                { "type", createInfo.type },
                { "radius", createInfo.radius },
                { "strength", createInfo.strength }
            });
        }

        // Save PickUps
        for (const PickUpCreateInfo& createInfo : sectorCreateInfo.pickUps) {
            json["PickUps"].push_back(nlohmann::json{
                { "position", createInfo.position },
                { "rotation", createInfo.rotation },
                { "type", createInfo.pickUpType }
            });
        }

        // Save Trees
        for (const TreeCreateInfo& createInfo : sectorCreateInfo.trees) {
            json["Trees"].push_back(nlohmann::json{
                { "position", createInfo.position },
                { "rotation", createInfo.rotation },
                { "scale", createInfo.scale },
                { "type", (int)createInfo.type }
            });
        }

        JSON::SaveToFile(json, filepath);
    }

    MapCreateInfo LoadMap(const std::string& filepath) {
        MapCreateInfo mapCreateInfo;
        // TODO: 
        std::cout << "TODO: JSON::LoadMap()\n";
        return mapCreateInfo;
    }

    void SaveMap(const std::string& filepath, MapCreateInfo& mapCreateInfo) {
        nlohmann::json json;
        json["Name"] = mapCreateInfo.name;
        json["Width"] = mapCreateInfo.width;
        json["Depth"] = mapCreateInfo.depth;
        //json["SectorLocations"] = mapCreateInfo.sectorLocations;
        JSON::SaveToFile(json, filepath);

        std::cout << "TODO: JSON::SaveMap()\n";
    }

    void SaveToFile(nlohmann::json& json, const std::string& filepath) {
        std::ofstream file(filepath);
        if (file.is_open()) {
            file << json.dump(4);
            file.close();
            std::cout << "Saved " << filepath << "\n";
        }
    }
}