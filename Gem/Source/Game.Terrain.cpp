#include "Game.h"

#include <AzCore/IO/SystemFile.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/std/containers/vector.h>

namespace TheLimitOfDreams
{
    void Game::CreateLandscape()
    {
        if (!LoadRawHeightMap(m_heightMapPath, HeightMapWidth, HeightMapHeight, m_rawHeightData))
        {
            AddConsoleLine(AZStd::string::format("Failed to import landscape from %s", m_heightMapPath.c_str()));
            return;
        }

        if (m_rawHeightData.empty())
        {
            AddConsoleLine("Loaded heightmap is empty.");
            return;
        }

        auto [minIt, maxIt] = AZStd::minmax_element(m_rawHeightData.begin(), m_rawHeightData.end());
        m_heightMin = *minIt;
        m_heightMax = *maxIt;

        AddConsoleLine(AZStd::string::format(
            "Landscape loaded: %dx%d, min=%u, max=%u",
            HeightMapWidth,
            HeightMapHeight,
            static_cast<unsigned>(m_heightMin),
            static_cast<unsigned>(m_heightMax)));
    }

    void Game::PositionPlayerOnTerrain()
    {
        if (m_rawHeightData.empty())
        {
            return;
        }

        const int centerX = HeightMapWidth / 2;
        const int centerY = HeightMapHeight / 2;
        const size_t index = static_cast<size_t>(centerY) * static_cast<size_t>(HeightMapWidth) + static_cast<size_t>(centerX);
        if (index >= m_rawHeightData.size())
        {
            return;
        }

        const float range = m_heightMax > m_heightMin ? static_cast<float>(m_heightMax - m_heightMin) : 1.0f;
        const float normalized = static_cast<float>(m_rawHeightData[index] - m_heightMin) / range;
        const float terrainHeight = normalized * MaxTerrainHeight;

        m_playerWorldPosition = AZ::Vector3(0.0f, terrainHeight + 1.8f, 0.0f);
    }

    bool Game::LoadRawHeightMap(const AZStd::string& path, int width, int height, AZStd::vector<AZ::u16>& outData) const
    {
        outData.clear();

        if (width <= 0 || height <= 0)
        {
            return false;
        }

        if (!AZ::IO::SystemFile::Exists(path.c_str()))
        {
            return false;
        }

        AZ::IO::SystemFile file;
        if (!file.Open(path.c_str(), AZ::IO::SystemFile::SF_OPEN_READ_ONLY))
        {
            return false;
        }

        const AZ::IO::SystemFile::SizeType expectedBytes =
            static_cast<AZ::IO::SystemFile::SizeType>(width) *
            static_cast<AZ::IO::SystemFile::SizeType>(height) *
            static_cast<AZ::IO::SystemFile::SizeType>(2);

        if (file.Length() < expectedBytes)
        {
            return false;
        }

        AZStd::vector<AZ::u8> rawBytes(expectedBytes);
        const AZ::IO::SystemFile::SizeType bytesRead = file.Read(expectedBytes, rawBytes.data());
        if (bytesRead != expectedBytes)
        {
            return false;
        }

        outData.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
        for (size_t i = 0; i < outData.size(); ++i)
        {
            const AZ::u16 lo = rawBytes[i * 2];
            const AZ::u16 hi = rawBytes[i * 2 + 1];
            outData[i] = static_cast<AZ::u16>((hi << 8u) | lo);
        }

        return true;
    }
}
