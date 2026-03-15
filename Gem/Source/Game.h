#pragma once

#include "PauseMenu.h"

#include <AzCore/Math/Vector3.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzFramework/Windowing/WindowBus.h>

namespace AzFramework
{
    class InputChannel;
}

namespace TheLimitOfDreams
{
    class Game
        : private AzFramework::InputChannelEventListener
    {
    public:
        Game();
        ~Game() override;

        void Activate();
        void Deactivate();
        void OnTick(float timeStep);

        void ExecuteConsoleCommand(const AZStd::string& commandLine);
        AZ::Vector3 GetPlayerWorldPosition() const;

    private:
        enum class GameState
        {
            MainMenu,
            Playing,
            Paused
        };

        enum class AspectRatioMode
        {
            Ratio16x9,
            Ratio16x10
        };

        struct ChunkCoord
        {
            int m_x{};
            int m_z{};

            bool operator==(const ChunkCoord& rhs) const
            {
                return m_x == rhs.m_x && m_z == rhs.m_z;
            }
        };

        struct ChunkCoordHash
        {
            size_t operator()(const ChunkCoord& coord) const
            {
                const AZ::s64 packed = (static_cast<AZ::s64>(coord.m_x) << 32) ^ static_cast<AZ::u32>(coord.m_z);
                return AZStd::hash<AZ::s64>{}(packed);
            }
        };

        struct Chunk
        {
            ChunkCoord m_coord;
            bool m_isLoaded{ false };
            AZStd::vector<AZ::Vector3> m_boxPositions;
        };

        bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;

        void InitializeGame();
        void StartGame();
        void PauseGame();
        void ResumeGame();
        void ShowPauseMenu();
        void HidePauseMenuAndResume();
        void ToggleFullscreenMode();
        void ToggleAspectRatioMode();
        void ApplyDisplaySettings();
        void RefreshDisplaySettingsUi();

        void CreateScene();
        void CreateWorldGeometry();
        void CreateLight();
        void CreateViewport();
        void SubscribeToEvents();

        void CreateConsoleUI();
        void ToggleConsole();
        void AddConsoleLine(const AZStd::string& line);

        AZ::Vector3 UserToWorldCoords(float x, float y, float z) const;
        AZ::Vector3 WorldToUserCoords(const AZ::Vector3& world) const;

        void CreateLandscape();
        void PositionPlayerOnTerrain();
        bool LoadRawHeightMap(const AZStd::string& path, int width, int height, AZStd::vector<AZ::u16>& outData) const;

        void HandleCameraLook();
        void HandleMovement(float timeStep);

        void UpdateChunks();
        void LoadChunk(int x, int z);
        void UnloadChunk(int x, int z);
        ChunkCoord GetChunkCoordinates(const AZ::Vector3& position) const;
        void GenerateChunkGeometry(int x, int z);

        AzFramework::NativeWindowHandle GetDefaultWindowHandle() const;
        void SetGameplayCursorMode(bool gameplayMode);

        static constexpr float ChunkSize = 20.0f;

        GameState m_gameState{ GameState::MainMenu };
        AspectRatioMode m_aspectRatioMode{ AspectRatioMode::Ratio16x9 };

        AZ::Vector3 m_playerWorldPosition{ 0.0f, 1.8f, 0.0f };

        float m_moveSpeed{ 8.0f };
        float m_mouseSensitivity{ 0.1f };
        float m_yawDegrees{ 0.0f };
        float m_pitchDegrees{ 10.0f };

        AZStd::unordered_map<ChunkCoord, Chunk, ChunkCoordHash> m_chunks;
        ChunkCoord m_currentChunkCoords{ 0, 0 };
        bool m_chunksInitialized{ false };
        int m_viewDistance{ 1 };

        PauseMenu m_pauseMenu;

        AZStd::vector<AZStd::string> m_consoleLines;
        bool m_consoleVisible{ false };

        bool m_fullscreenMode{ false };

        bool m_moveForward{ false };
        bool m_moveBack{ false };
        bool m_moveLeft{ false };
        bool m_moveRight{ false };
        bool m_moveUp{ false };
        bool m_moveDown{ false };
        float m_pendingMouseDeltaX{ 0.0f };
        float m_pendingMouseDeltaY{ 0.0f };

        static constexpr int HeightMapWidth = 4097;
        static constexpr int HeightMapHeight = 4097;
        static constexpr float MaxTerrainHeight = 600.0f;

        AZStd::string m_heightMapPath{ "/Users/marat/Documents/map/map.r16" };
        AZStd::vector<AZ::u16> m_rawHeightData;
        AZ::u16 m_heightMin{ 0 };
        AZ::u16 m_heightMax{ 0 };
    };
} // namespace TheLimitOfDreams
