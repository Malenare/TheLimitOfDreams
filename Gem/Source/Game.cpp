#include "Game.h"

#include <AzCore/Math/MathUtils.h>
#include <AzCore/std/algorithm.h>
#include <AzFramework/Input/Buses/Requests/InputSystemCursorRequestBus.h>
#include <AzFramework/Input/Channels/InputChannel.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzFramework/Windowing/WindowBus.h>

#include <cmath>

namespace TheLimitOfDreams
{
    Game::Game()
        : AzFramework::InputChannelEventListener(AzFramework::InputChannelEventListener::GetPriorityDefault())
    {
    }

    Game::~Game()
    {
        Deactivate();
    }

    void Game::Activate()
    {
        InitializeGame();
        CreateScene();
        CreateConsoleUI();
        CreateViewport();
        SubscribeToEvents();
        StartGame();
    }

    void Game::Deactivate()
    {
        Disconnect();
        m_chunks.clear();
        m_rawHeightData.clear();
        m_consoleLines.clear();
        m_chunksInitialized = false;
    }

    void Game::OnTick(float timeStep)
    {
        if (m_consoleVisible)
        {
            HandleCameraLook();
            return;
        }

        if (m_gameState != GameState::Playing)
        {
            HandleCameraLook();
            return;
        }

        HandleCameraLook();
        HandleMovement(timeStep);
        UpdateChunks();
    }

    void Game::InitializeGame()
    {
        SetGameplayCursorMode(true);

        if (const AzFramework::NativeWindowHandle windowHandle = GetDefaultWindowHandle(); windowHandle != nullptr)
        {
            AzFramework::WindowRequestBus::EventResult(
                m_fullscreenMode,
                windowHandle,
                &AzFramework::WindowRequests::GetFullScreenState);

            AzFramework::WindowSize size{};
            AzFramework::WindowRequestBus::EventResult(
                size,
                windowHandle,
                &AzFramework::WindowRequests::GetClientAreaSize);

            if (size.m_height > 0)
            {
                const float ratio = static_cast<float>(size.m_width) / static_cast<float>(size.m_height);
                const float distance169 = std::abs(ratio - (16.0f / 9.0f));
                const float distance1610 = std::abs(ratio - (16.0f / 10.0f));
                m_aspectRatioMode = distance1610 < distance169 ? AspectRatioMode::Ratio16x10 : AspectRatioMode::Ratio16x9;
            }
        }

        RefreshDisplaySettingsUi();
    }

    void Game::StartGame()
    {
        m_gameState = GameState::Playing;
    }

    void Game::PauseGame()
    {
        m_gameState = GameState::Paused;
    }

    void Game::ResumeGame()
    {
        m_gameState = GameState::Playing;
        SetGameplayCursorMode(true);
    }

    void Game::ShowPauseMenu()
    {
        PauseGame();
        m_pauseMenu.ShowMainMenuPage();
        m_pauseMenu.SetVisible(true);
        SetGameplayCursorMode(false);
    }

    void Game::HidePauseMenuAndResume()
    {
        m_pauseMenu.ShowMainMenuPage();
        m_pauseMenu.SetVisible(false);
        ResumeGame();
    }

    void Game::ToggleFullscreenMode()
    {
        m_fullscreenMode = !m_fullscreenMode;
        ApplyDisplaySettings();
    }

    void Game::ToggleAspectRatioMode()
    {
        m_aspectRatioMode =
            m_aspectRatioMode == AspectRatioMode::Ratio16x9 ? AspectRatioMode::Ratio16x10 : AspectRatioMode::Ratio16x9;
        ApplyDisplaySettings();
    }

    void Game::ApplyDisplaySettings()
    {
        const AzFramework::NativeWindowHandle windowHandle = GetDefaultWindowHandle();
        if (windowHandle == nullptr)
        {
            RefreshDisplaySettingsUi();
            return;
        }

        const float ratio = m_aspectRatioMode == AspectRatioMode::Ratio16x9 ? 16.0f / 9.0f : 16.0f / 10.0f;
        uint32_t targetHeight = m_aspectRatioMode == AspectRatioMode::Ratio16x9 ? 720U : 800U;
        uint32_t targetWidth = static_cast<uint32_t>(static_cast<float>(targetHeight) * ratio + 0.5f);

        bool canToggleFullscreen = false;
        AzFramework::WindowRequestBus::EventResult(
            canToggleFullscreen,
            windowHandle,
            &AzFramework::WindowRequests::CanToggleFullScreenState);

        if (canToggleFullscreen)
        {
            AzFramework::WindowRequestBus::Event(
                windowHandle,
                &AzFramework::WindowRequests::SetFullScreenState,
                m_fullscreenMode);
        }

        if (!m_fullscreenMode)
        {
            AzFramework::WindowPosOptions options;
            options.m_ignoreScreenSizeLimit = false;
            AzFramework::WindowSize targetSize(targetWidth, targetHeight);
            AzFramework::WindowRequestBus::Event(
                windowHandle,
                &AzFramework::WindowRequests::ResizeClientArea,
                targetSize,
                options);
            AzFramework::WindowRequestBus::Event(
                windowHandle,
                &AzFramework::WindowRequests::SetRenderResolution,
                targetSize);
        }

        RefreshDisplaySettingsUi();
    }

    void Game::RefreshDisplaySettingsUi()
    {
        m_pauseMenu.SetDisplayModeText(m_fullscreenMode);
        m_pauseMenu.SetAspectRatioText(m_aspectRatioMode == AspectRatioMode::Ratio16x10);
    }

    void Game::CreateScene()
    {
        m_playerWorldPosition = AZ::Vector3(0.0f, 1.8f, 0.0f);
        CreateLight();
        CreateLandscape();
        PositionPlayerOnTerrain();
        CreateWorldGeometry();
    }

    void Game::CreateLight()
    {
        AddConsoleLine("Directional light setup is managed by O3DE project-level rendering pipeline.");
    }

    void Game::CreateWorldGeometry()
    {
        AddConsoleLine("World geometry is generated by chunk logic at runtime.");
    }

    void Game::CreateViewport()
    {
        // O3DE viewport/camera setup is provided by enabled project gems and level content.
    }

    void Game::SubscribeToEvents()
    {
        Connect();
    }

    bool Game::OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel)
    {
        const auto& channelId = inputChannel.GetInputChannelId();

        if (channelId == AzFramework::InputDeviceMouse::Movement::X)
        {
            m_pendingMouseDeltaX += inputChannel.GetValue();
            return false;
        }

        if (channelId == AzFramework::InputDeviceMouse::Movement::Y)
        {
            m_pendingMouseDeltaY += inputChannel.GetValue();
            return false;
        }

        if (channelId == AzFramework::InputDeviceKeyboard::Key::Escape && inputChannel.IsStateBegan())
        {
            if (m_consoleVisible)
            {
                ToggleConsole();
                return true;
            }

            if (m_pauseMenu.IsVisible() || m_gameState == GameState::Paused)
            {
                if (m_pauseMenu.IsVideoMenuPage())
                {
                    m_pauseMenu.ShowMainMenuPage();
                }
                else
                {
                    HidePauseMenuAndResume();
                }
            }
            else
            {
                ShowPauseMenu();
            }
            return true;
        }

        if (channelId == AzFramework::InputDeviceKeyboard::Key::PunctuationTilde && inputChannel.IsStateBegan())
        {
            ToggleConsole();
            return true;
        }

        if (channelId == AzFramework::InputDeviceKeyboard::Key::Function11 && inputChannel.IsStateBegan())
        {
            ToggleFullscreenMode();
            return true;
        }

        if (channelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericW)
        {
            m_moveForward = inputChannel.IsActive();
            return false;
        }

        if (channelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericS)
        {
            m_moveBack = inputChannel.IsActive();
            return false;
        }

        if (channelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericA)
        {
            m_moveLeft = inputChannel.IsActive();
            return false;
        }

        if (channelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericD)
        {
            m_moveRight = inputChannel.IsActive();
            return false;
        }

        if (channelId == AzFramework::InputDeviceKeyboard::Key::EditSpace)
        {
            m_moveUp = inputChannel.IsActive();
            return false;
        }

        if (channelId == AzFramework::InputDeviceKeyboard::Key::ModifierCtrlL ||
            channelId == AzFramework::InputDeviceKeyboard::Key::ModifierCtrlR)
        {
            m_moveDown = inputChannel.IsActive();
            return false;
        }

        return false;
    }

    void Game::HandleCameraLook()
    {
        if (m_consoleVisible || m_pauseMenu.IsVisible())
        {
            m_pendingMouseDeltaX = 0.0f;
            m_pendingMouseDeltaY = 0.0f;
            return;
        }

        m_yawDegrees += m_mouseSensitivity * m_pendingMouseDeltaX;
        m_pitchDegrees += m_mouseSensitivity * m_pendingMouseDeltaY;
        m_pitchDegrees = AZStd::clamp(m_pitchDegrees, -80.0f, 80.0f);

        m_pendingMouseDeltaX = 0.0f;
        m_pendingMouseDeltaY = 0.0f;
    }

    void Game::HandleMovement(float timeStep)
    {
        AZ::Vector3 direction = AZ::Vector3::CreateZero();

        const float yawRadians = AZ::DegToRad(m_yawDegrees);
        const AZ::Vector3 forward(std::sin(yawRadians), 0.0f, std::cos(yawRadians));
        const AZ::Vector3 right(std::cos(yawRadians), 0.0f, -std::sin(yawRadians));

        if (m_moveForward)
        {
            direction += forward;
        }
        if (m_moveBack)
        {
            direction -= forward;
        }
        if (m_moveLeft)
        {
            direction -= right;
        }
        if (m_moveRight)
        {
            direction += right;
        }
        if (m_moveUp)
        {
            direction += AZ::Vector3::CreateAxisY();
        }
        if (m_moveDown)
        {
            direction -= AZ::Vector3::CreateAxisY();
        }

        if (!direction.IsZero())
        {
            direction.Normalize();
            m_playerWorldPosition += direction * (m_moveSpeed * timeStep);
        }
    }

    Game::ChunkCoord Game::GetChunkCoordinates(const AZ::Vector3& position) const
    {
        const int x = static_cast<int>(std::floor(position.GetX() / ChunkSize));
        const int z = static_cast<int>(std::floor(position.GetZ() / ChunkSize));
        return { x, z };
    }

    void Game::UpdateChunks()
    {
        const ChunkCoord newChunkCoords = GetChunkCoordinates(m_playerWorldPosition);
        if (!m_chunksInitialized || !(newChunkCoords == m_currentChunkCoords))
        {
            m_chunksInitialized = true;
            m_currentChunkCoords = newChunkCoords;

            for (int x = newChunkCoords.m_x - m_viewDistance; x <= newChunkCoords.m_x + m_viewDistance; ++x)
            {
                for (int z = newChunkCoords.m_z - m_viewDistance; z <= newChunkCoords.m_z + m_viewDistance; ++z)
                {
                    const ChunkCoord key{ x, z };
                    if (m_chunks.find(key) == m_chunks.end())
                    {
                        LoadChunk(x, z);
                    }
                }
            }

            AZStd::vector<ChunkCoord> keysToUnload;
            keysToUnload.reserve(m_chunks.size());
            for (const auto& [coord, chunk] : m_chunks)
            {
                AZ_UNUSED(chunk);
                const int dx = std::abs(coord.m_x - newChunkCoords.m_x);
                const int dz = std::abs(coord.m_z - newChunkCoords.m_z);
                if (dx > m_viewDistance || dz > m_viewDistance)
                {
                    keysToUnload.push_back(coord);
                }
            }

            for (const ChunkCoord& coord : keysToUnload)
            {
                UnloadChunk(coord.m_x, coord.m_z);
            }
        }

        int visibleCount = 0;
        for (const auto& [coord, chunk] : m_chunks)
        {
            AZ_UNUSED(coord);
            if (chunk.m_isLoaded)
            {
                ++visibleCount;
            }
        }
        m_pauseMenu.SetChunkCount(visibleCount);
    }

    void Game::LoadChunk(int x, int z)
    {
        const ChunkCoord key{ x, z };
        if (m_chunks.find(key) != m_chunks.end())
        {
            return;
        }

        Chunk chunk;
        chunk.m_coord = key;
        chunk.m_isLoaded = false;

        m_chunks.emplace(key, AZStd::move(chunk));
        GenerateChunkGeometry(x, z);
        m_chunks[key].m_isLoaded = true;
    }

    void Game::UnloadChunk(int x, int z)
    {
        const ChunkCoord key{ x, z };
        m_chunks.erase(key);
    }

    void Game::GenerateChunkGeometry(int x, int z)
    {
        const ChunkCoord key{ x, z };
        auto it = m_chunks.find(key);
        if (it == m_chunks.end())
        {
            return;
        }

        Chunk& chunk = it->second;
        chunk.m_boxPositions.clear();

        constexpr float step = 5.0f;
        const int gridSize = static_cast<int>(ChunkSize / step);
        chunk.m_boxPositions.reserve(static_cast<size_t>(gridSize * gridSize));

        for (int i = 0; i < gridSize; ++i)
        {
            for (int j = 0; j < gridSize; ++j)
            {
                chunk.m_boxPositions.emplace_back(
                    static_cast<float>(x) * ChunkSize + i * step,
                    0.5f,
                    static_cast<float>(z) * ChunkSize + j * step);
            }
        }
    }

    AZ::Vector3 Game::GetPlayerWorldPosition() const
    {
        return m_playerWorldPosition;
    }

    AzFramework::NativeWindowHandle Game::GetDefaultWindowHandle() const
    {
        AzFramework::NativeWindowHandle windowHandle = nullptr;
        AzFramework::WindowSystemRequestBus::BroadcastResult(
            windowHandle,
            &AzFramework::WindowSystemRequests::GetDefaultWindowHandle);
        return windowHandle;
    }

    void Game::SetGameplayCursorMode(bool gameplayMode)
    {
        const AzFramework::SystemCursorState cursorState = gameplayMode
            ? AzFramework::SystemCursorState::ConstrainedAndHidden
            : AzFramework::SystemCursorState::UnconstrainedAndVisible;

        AzFramework::InputSystemCursorRequestBus::Broadcast(
            &AzFramework::InputSystemCursorRequests::SetSystemCursorState,
            cursorState);
    }
}
