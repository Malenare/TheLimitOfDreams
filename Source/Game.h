#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/Variant.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Engine/EngineEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/Image.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/Graphics/Terrain.h>
#include <vector>
#include <unordered_map>

using namespace Urho3D;

class PauseMenu;

/// ============================================================================
/// CHUNK SYSTEM
/// ============================================================================
/// Configurable parameter: chunk size in world units
/// Adjust this value to change how terrain is divided into chunks
/// Smaller = more chunks (more updates, finer control)
/// Larger = fewer chunks (less overhead, coarser control)
#define CHUNK_SIZE 20.0f

/// Hash function for chunk coordinates pair
struct ChunkCoordHash {
    size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<long long>()(((long long)p.first << 32) | (unsigned int)p.second);
    }
};

/// Represents a single chunk of terrain/world
struct Chunk {
    int x, z;  ///< Chunk grid coordinates
    SharedPtr<Node> node;  ///< Root node for this chunk's visual representation
    bool isLoaded = false;  ///< Whether this chunk is currently loaded in scene
};

class Game : public Application
{
    URHO3D_OBJECT(Game, Application);

public:
    explicit Game(Context* context);

    void Setup() override;
    void Start() override;

private:
    enum class GameState
    {
        MainMenu,
        Playing,
        Paused
    };

    void InitializeGame();
    void StartGame();
    void PauseGame();
    void ResumeGame();
    void ShowPauseMenu();
    void HidePauseMenuAndResume();

    void CreateScene();
    void CreateWorldGeometry();
    void CreateLight();
    void CreateViewport();
    void SubscribeToEvents();
    void CreateConsoleUI();
    void ToggleConsole();
    void AddConsoleLine(const ea::string& line);
    void ExecuteConsoleCommand(const ea::string& commandLine);
    Vector3 UserToWorldCoords(float x, float y, float z) const;
    Vector3 WorldToUserCoords(const Vector3& world) const;
    void CreateLandscape();
    void PositionPlayerOnTerrain();
    bool LoadRawHeightMap(const ea::string& path, int width, int height, std::vector<unsigned short>& outData) const;
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleInputFocus(StringHash eventType, VariantMap& eventData);
    void HandleCameraLook();
    void HandleMovement(float timeStep);

    /// Chunk management methods
    void UpdateChunks();
    void LoadChunk(int x, int z);
    void UnloadChunk(int x, int z);
    std::pair<int, int> GetChunkCoordinates(const Vector3& position) const;
    void GenerateChunkGeometry(int x, int z);

    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<Terrain> terrain_;

    GameState gameState_{GameState::MainMenu};

    float moveSpeed_{8.0f};
    float mouseSensitivity_{0.1f};
    float yaw_{0.0f};
    float pitch_{10.0f};

    /// Chunk system data
    std::unordered_map<std::pair<int, int>, Chunk, ChunkCoordHash> chunks_;
    std::pair<int, int> currentChunkCoords_{0, 0};
    bool chunksInitialized_{false};
    int viewDistance_{1};  ///< How many chunks around player to load (radius); 1 = 3x3 chunks around player

    // UI / menu
    SharedPtr<PauseMenu> pauseMenu_;

    // Menu handlers
    void HandleCloseMenu(StringHash eventType, VariantMap& eventData);
    void HandleExitButton(StringHash eventType, VariantMap& eventData);
    void HandleViewDistanceSlider(StringHash eventType, VariantMap& eventData);
    void HandleConsoleTextFinished(StringHash eventType, VariantMap& eventData);
    void HandleConsoleTextInput(StringHash eventType, VariantMap& eventData);

    SharedPtr<UIElement> consoleRoot_;
    SharedPtr<Text> consoleOutput_;
    SharedPtr<LineEdit> consoleInput_;
    ea::vector<ea::string> consoleLines_;
    bool consoleVisible_{false};
};
