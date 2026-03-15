#include "Game.h"

#include <AzCore/Debug/Trace.h>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace TheLimitOfDreams
{
    AZ::Vector3 Game::UserToWorldCoords(float x, float y, float z) const
    {
        return AZ::Vector3(x, z, y);
    }

    AZ::Vector3 Game::WorldToUserCoords(const AZ::Vector3& world) const
    {
        return AZ::Vector3(world.GetX(), world.GetZ(), world.GetY());
    }

    void Game::CreateConsoleUI()
    {
        AddConsoleLine(
            "Console ready. Commands: teleport x y z, speed value, coordinate, fullscreen, aspect 16:9|16:10, viewdistance 1..8");
    }

    void Game::ToggleConsole()
    {
        m_consoleVisible = !m_consoleVisible;

        if (m_consoleVisible)
        {
            SetGameplayCursorMode(false);
            AddConsoleLine("Console opened.");
        }
        else
        {
            if (m_gameState == GameState::Playing)
            {
                SetGameplayCursorMode(true);
            }
            AddConsoleLine("Console closed.");
        }
    }

    void Game::AddConsoleLine(const AZStd::string& line)
    {
        m_consoleLines.push_back(line);
        if (m_consoleLines.size() > 20)
        {
            m_consoleLines.erase(m_consoleLines.begin());
        }

        AZ_Printf("TheLimitOfDreams", "%s", line.c_str());
    }

    void Game::ExecuteConsoleCommand(const AZStd::string& commandLine)
    {
        AZStd::string trimmed = commandLine;
        const auto firstNonWhitespace = trimmed.find_first_not_of(" \t\n\r");
        if (firstNonWhitespace == AZStd::string::npos)
        {
            return;
        }
        const auto lastNonWhitespace = trimmed.find_last_not_of(" \t\n\r");
        trimmed = trimmed.substr(firstNonWhitespace, lastNonWhitespace - firstNonWhitespace + 1);
        if (trimmed.empty())
        {
            return;
        }

        AddConsoleLine(AZStd::string::format("> %s", trimmed.c_str()));

        std::istringstream iss(trimmed.c_str());
        std::string command;
        iss >> command;

        std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            });

        if (command == "teleport")
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            if (!(iss >> x >> y >> z))
            {
                AddConsoleLine("Usage: teleport <x> <y> <z>");
                return;
            }

            m_playerWorldPosition = UserToWorldCoords(x, y, z);
            AddConsoleLine("Teleported.");
            return;
        }

        if (command == "speed")
        {
            float value = 0.0f;
            if (!(iss >> value) || value <= 0.0f)
            {
                AddConsoleLine("Usage: speed <positive_value>");
                return;
            }

            m_moveSpeed = value;
            AddConsoleLine(AZStd::string::format("Speed set to %.2f", m_moveSpeed));
            return;
        }

        if (command == "coordinate")
        {
            const AZ::Vector3 user = WorldToUserCoords(m_playerWorldPosition);
            AddConsoleLine(AZStd::string::format("xyz: %.2f %.2f %.2f", user.GetX(), user.GetY(), user.GetZ()));
            return;
        }

        if (command == "fullscreen")
        {
            ToggleFullscreenMode();
            AddConsoleLine(m_fullscreenMode ? "Fullscreen mode enabled." : "Windowed mode enabled.");
            return;
        }

        if (command == "aspect")
        {
            std::string value;
            iss >> value;
            if (value != "16:9" && value != "16:10")
            {
                AddConsoleLine("Usage: aspect 16:9|16:10");
                return;
            }

            const AspectRatioMode targetMode = value == "16:10" ? AspectRatioMode::Ratio16x10 : AspectRatioMode::Ratio16x9;
            if (m_aspectRatioMode != targetMode)
            {
                ToggleAspectRatioMode();
            }

            AddConsoleLine(value == "16:10" ? "Aspect ratio set to 16:10." : "Aspect ratio set to 16:9.");
            return;
        }

        if (command == "viewdistance")
        {
            int value = 0;
            if (!(iss >> value))
            {
                AddConsoleLine("Usage: viewdistance <1..8>");
                return;
            }

            m_viewDistance = std::clamp(value, 1, 8);
            m_pauseMenu.SetViewDistance(m_viewDistance);
            AddConsoleLine(AZStd::string::format("View distance set to %d.", m_viewDistance));
            UpdateChunks();
            return;
        }

        if (command == "terrainpath")
        {
            std::string path;
            iss >> path;
            if (path.empty())
            {
                AddConsoleLine("Usage: terrainpath <absolute_or_relative_path_to_r16>");
                return;
            }

            m_heightMapPath = path.c_str();
            AddConsoleLine(AZStd::string::format("Terrain path set: %s", m_heightMapPath.c_str()));
            return;
        }

        if (command == "reloadterrain")
        {
            CreateLandscape();
            PositionPlayerOnTerrain();
            return;
        }

        if (command == "help")
        {
            AddConsoleLine("Commands: teleport, speed, coordinate, fullscreen, aspect, viewdistance, terrainpath, reloadterrain");
            return;
        }

        AddConsoleLine("Unknown command. Use: help");
    }
}
