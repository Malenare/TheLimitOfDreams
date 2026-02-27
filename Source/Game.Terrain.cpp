#include "Game.h"

#include <limits>
#include <vector>

#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>

namespace
{
constexpr int kHeightMapWidth = 4097;
constexpr int kHeightMapHeight = 4097;
constexpr float kWorldSize = 10000.0f; // 10x10 km, 1 meter = 1 unit
constexpr float kMaxTerrainHeight = 600.0f;
}

void Game::CreateLandscape()
{
    if (!scene_)
        return;

    const ea::string rawHeightMapPath = "/Users/marat/Documents/map/map.r16";

    std::vector<unsigned short> rawHeight;
    if (!LoadRawHeightMap(rawHeightMapPath, kHeightMapWidth, kHeightMapHeight, rawHeight))
    {
        URHO3D_LOGERRORF("Failed to import landscape from %s", rawHeightMapPath.c_str());
        return;
    }

    SharedPtr<Image> heightMapImage(new Image(context_));
    if (!heightMapImage->SetSize(kHeightMapWidth, kHeightMapHeight, 1))
    {
        URHO3D_LOGERROR("Failed to allocate height map image.");
        return;
    }

    unsigned char* imageData = heightMapImage->GetData();
    if (!imageData)
    {
        URHO3D_LOGERROR("Height map image data buffer is null.");
        return;
    }

    const unsigned samplesCount = static_cast<unsigned>(kHeightMapWidth * kHeightMapHeight);
    unsigned short minHeight = std::numeric_limits<unsigned short>::max();
    unsigned short maxHeight = std::numeric_limits<unsigned short>::min();
    for (unsigned i = 0; i < samplesCount; ++i)
    {
        minHeight = Min(minHeight, rawHeight[i]);
        maxHeight = Max(maxHeight, rawHeight[i]);
    }

    const unsigned short heightRange = maxHeight > minHeight ? static_cast<unsigned short>(maxHeight - minHeight) : 1;
    for (unsigned i = 0; i < samplesCount; ++i)
    {
        const unsigned normalized = static_cast<unsigned>(rawHeight[i] - minHeight) * 255u / static_cast<unsigned>(heightRange);
        imageData[i] = static_cast<unsigned char>(normalized);
    }

    Node* terrainNode = scene_->CreateChild("Landscape");
    terrainNode->SetPosition(Vector3(-kWorldSize * 0.5f, 0.0f, -kWorldSize * 0.5f));

    terrain_ = terrainNode->CreateComponent<Terrain>();
    terrain_->SetPatchSize(64);
    terrain_->SetSpacing(Vector3(kWorldSize / static_cast<float>(kHeightMapWidth - 1),
                                 kMaxTerrainHeight / 255.0f,
                                 kWorldSize / static_cast<float>(kHeightMapHeight - 1)));
    terrain_->SetSmoothing(true);
    terrain_->SetHeightMap(heightMapImage);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    if (cache)
    {
        if (Material* terrainMaterial = cache->GetResource<Material>("Materials/Terrain.xml"))
            terrain_->SetMaterial(terrainMaterial);
        else if (Material* fallbackMaterial = cache->GetResource<Material>("Materials/DefaultGrey.xml"))
            terrain_->SetMaterial(fallbackMaterial);
    }
}

void Game::PositionPlayerOnTerrain()
{
    if (!cameraNode_ || !terrain_)
        return;

    const IntVector2 verts = terrain_->GetNumVertices();
    const IntVector2 centerPixel(Max(0, verts.x_ / 2), Max(0, verts.y_ / 2));
    Vector3 worldPos = terrain_->HeightMapToWorld(centerPixel);

    const float terrainHeight = terrain_->GetHeight(worldPos);
    worldPos.y_ = terrainHeight + 1.8f;
    cameraNode_->SetWorldPosition(worldPos);
}

bool Game::LoadRawHeightMap(const ea::string& path, int width, int height, std::vector<unsigned short>& outData) const
{
    outData.clear();

    if (width <= 0 || height <= 0)
        return false;

    FileSystem* fileSystem = GetSubsystem<FileSystem>();
    if (!fileSystem || !fileSystem->FileExists(path))
        return false;

    File file(context_, path, FILE_READ);
    if (!file.IsOpen())
        return false;

    const unsigned expectedBytes = static_cast<unsigned>(width * height * 2);
    if (file.GetSize() < expectedBytes)
        return false;

    std::vector<unsigned char> rawBytes(expectedBytes);
    const unsigned bytesRead = file.Read(rawBytes.data(), expectedBytes);
    if (bytesRead != expectedBytes)
        return false;

    outData.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    for (unsigned i = 0; i < static_cast<unsigned>(outData.size()); ++i)
    {
        const unsigned lo = rawBytes[i * 2];
        const unsigned hi = rawBytes[i * 2 + 1];
        outData[i] = static_cast<unsigned short>((hi << 8u) | lo);
    }

    return true;
}
