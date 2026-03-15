#include "Game.h"
#include <cmath>
#include <string>
#include <vector>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include "PauseMenu.h"

Game::Game(Context* context) :
    Application(context)
{
}

void Game::Setup()
{
    engineParameters_["WindowTitle"] = ea::string("The Limit Of Dreams");
    engineParameters_["FullScreen"] = false;
    engineParameters_["WindowWidth"] = 1280;
    engineParameters_["WindowHeight"] = 720;
}

void Game::Start()
{
    InitializeGame();
    if (Graphics* graphics = GetSubsystem<Graphics>())
    {
        fullscreenMode_ = graphics->GetFullscreen();
        const int h = graphics->GetHeight();
        const int w = graphics->GetWidth();
        if (h > 0)
        {
            const float ratio = static_cast<float>(w) / static_cast<float>(h);
            aspectRatioMode_ = std::abs(ratio - (16.0f / 10.0f)) < std::abs(ratio - (16.0f / 9.0f))
                ? AspectRatioMode::Ratio16x10
                : AspectRatioMode::Ratio16x9;
        }
    }
    CreateScene();
    // Create pause menu and wire UI events
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    pauseMenu_ = new PauseMenu(context_);
    pauseMenu_->Create(ui, cache);
    CreateConsoleUI();
    CreateViewport();
    SubscribeToEvents();
    // Subscribe to pause menu button events
    if (pauseMenu_)
    {
        SubscribeToEvent(pauseMenu_->GetCloseButton(), E_RELEASED, URHO3D_HANDLER(Game, HandleCloseMenu));
        SubscribeToEvent(pauseMenu_->GetExitButton(), E_RELEASED, URHO3D_HANDLER(Game, HandleExitButton));
        SubscribeToEvent(pauseMenu_->GetVideoButton(), E_RELEASED, URHO3D_HANDLER(Game, HandleVideoButton));
        SubscribeToEvent(pauseMenu_->GetVideoBackButton(), E_RELEASED, URHO3D_HANDLER(Game, HandleVideoBackButton));
        SubscribeToEvent(pauseMenu_->GetDisplayModeButton(), E_RELEASED, URHO3D_HANDLER(Game, HandleDisplayModeButton));
        SubscribeToEvent(pauseMenu_->GetAspectRatioButton(), E_RELEASED, URHO3D_HANDLER(Game, HandleAspectRatioButton));
        SubscribeToEvent(pauseMenu_->GetViewDistanceSlider(), E_SLIDERCHANGED, URHO3D_HANDLER(Game, HandleViewDistanceSlider));
        // For UI display, never show less than 1 (visual-only change)
        int displayViewDistance = viewDistance_ < 1 ? 1 : viewDistance_;
        if (pauseMenu_->GetViewDistanceSlider())
            // Slider internal range is 0..7 so map display 1..8 -> slider 0..7
            pauseMenu_->GetViewDistanceSlider()->SetValue(static_cast<float>(displayViewDistance - 1));
        if (pauseMenu_->GetViewDistanceText())
        {
            std::string txt = std::string("Дальность прорисовки: ") + std::to_string(displayViewDistance);
            pauseMenu_->GetViewDistanceText()->SetText(txt.c_str());
        }
        RefreshDisplaySettingsUi();
    }
    if (consoleInput_)
        SubscribeToEvent(consoleInput_, E_TEXTFINISHED, URHO3D_HANDLER(Game, HandleConsoleTextFinished));
    StartGame();
}

void Game::InitializeGame()
{
    Input* input = GetSubsystem<Input>();
    input->SetMouseVisible(false);
    input->SetMouseMode(MM_RELATIVE);
    input->SetMouseGrabbed(true);
}

void Game::StartGame()
{
    gameState_ = GameState::Playing;
}

void Game::PauseGame()
{
    gameState_ = GameState::Paused;
}

void Game::ResumeGame()
{
    gameState_ = GameState::Playing;
    // Ensure input is returned to gameplay state
    Input* in = GetSubsystem<Input>();
    if (in)
    {
        in->SetMouseVisible(false);
        in->SetMouseMode(MM_RELATIVE);
        in->SetMouseGrabbed(true);
    }
}

void Game::ShowPauseMenu()
{
    PauseGame();
    UI* ui = GetSubsystem<UI>();
    Input* in = GetSubsystem<Input>();
    if (pauseMenu_ && pauseMenu_->GetRoot())
    {
        pauseMenu_->ShowMainMenuPage();
        auto r = pauseMenu_->GetRoot();
        r->SetVisible(true);
        r->SetEnabled(true);
        if (in)
        {
            in->SetMouseVisible(true);
            in->SetMouseMode(MM_ABSOLUTE);
            in->SetMouseGrabbed(false);
        }
        if (ui)
            ui->SetFocusElement(r);
    }
}

void Game::HidePauseMenuAndResume()
{
    UI* ui = GetSubsystem<UI>();
    if (pauseMenu_ && pauseMenu_->GetRoot())
    {
        pauseMenu_->ShowMainMenuPage();
        auto r = pauseMenu_->GetRoot();
        r->SetVisible(false);
        r->SetEnabled(false);
    }
    if (ui)
        ui->SetFocusElement(nullptr);
    ResumeGame();
}

void Game::ToggleFullscreenMode()
{
    fullscreenMode_ = !fullscreenMode_;
    ApplyDisplaySettings();
}

void Game::ToggleAspectRatioMode()
{
    aspectRatioMode_ = aspectRatioMode_ == AspectRatioMode::Ratio16x9
        ? AspectRatioMode::Ratio16x10
        : AspectRatioMode::Ratio16x9;
    ApplyDisplaySettings();
}

void Game::ApplyDisplaySettings()
{
    Graphics* graphics = GetSubsystem<Graphics>();
    UI* ui = GetSubsystem<UI>();
    if (!graphics)
        return;

    const float ratio = aspectRatioMode_ == AspectRatioMode::Ratio16x9 ? 16.0f / 9.0f : 16.0f / 10.0f;
    int targetHeight = aspectRatioMode_ == AspectRatioMode::Ratio16x9 ? 720 : 800;
    int targetWidth = static_cast<int>(targetHeight * ratio + 0.5f);

    WindowSettings settings = graphics->GetWindowSettings();
    settings.mode_ = fullscreenMode_ ? WindowMode::Fullscreen : WindowMode::Windowed;

    if (fullscreenMode_)
    {
        const IntVector2 desktopSize = graphics->GetDesktopResolution(settings.monitor_);
        if (desktopSize.x_ > 0 && desktopSize.y_ > 0)
        {
            targetHeight = desktopSize.y_;
            targetWidth = static_cast<int>(targetHeight * ratio + 0.5f);
            if (targetWidth > desktopSize.x_)
            {
                targetWidth = desktopSize.x_;
                targetHeight = static_cast<int>(targetWidth / ratio + 0.5f);
            }
        }
    }

    if (targetWidth < 320)
        targetWidth = 320;
    if (targetHeight < 240)
        targetHeight = 240;

    settings.size_ = IntVector2(targetWidth, targetHeight);
    if (!graphics->SetScreenMode(settings))
    {
        fullscreenMode_ = graphics->GetFullscreen();
        RefreshDisplaySettingsUi();
        return;
    }

    if (!fullscreenMode_)
    {
        const IntVector2 desktopSize = graphics->GetDesktopResolution(settings.monitor_);
        if (desktopSize.x_ > 0 && desktopSize.y_ > 0)
        {
            const int posX = Max((desktopSize.x_ - targetWidth) / 2, 0);
            const int posY = Max((desktopSize.y_ - targetHeight) / 2, 0);
            graphics->SetWindowPosition(posX, posY);
        }
    }

    if (cameraNode_)
    {
        if (Camera* camera = cameraNode_->GetComponent<Camera>())
            camera->SetAspectRatio(ratio);
    }

    if (pauseMenu_ && pauseMenu_->GetRoot() && ui)
        pauseMenu_->GetRoot()->SetSize(ui->GetRoot()->GetSize());
    RefreshDisplaySettingsUi();
}

void Game::RefreshDisplaySettingsUi()
{
    if (!pauseMenu_)
        return;

    if (pauseMenu_->GetDisplayModeText())
        pauseMenu_->GetDisplayModeText()->SetText(fullscreenMode_ ? "Режим: Полноэкранный" : "Режим: Оконный");

    if (pauseMenu_->GetAspectRatioText())
        pauseMenu_->GetAspectRatioText()->SetText(
            aspectRatioMode_ == AspectRatioMode::Ratio16x9 ? "Соотношение: 16:9" : "Соотношение: 16:10");
}

void Game::CreateScene()
{
    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();

    // Player placeholder without model. Height = 1.8 units (180 cm).
    cameraNode_ = scene_->CreateChild("PlayerCharacter");
    cameraNode_->SetPosition(Vector3(0.0f, 1.8f, 0.0f));
    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
    cameraNode_->CreateComponent<Camera>()->SetFarClip(12000.0f);

    CreateLight();
    CreateLandscape();
    PositionPlayerOnTerrain();
}

void Game::CreateLight()
{
    Node* zoneNode = scene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-2000.0f, 2000.0f));
    zone->SetAmbientColor(Color(0.25f, 0.25f, 0.28f));

    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.3f, -1.0f, 0.5f));

    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetBrightness(1.2f);
    light->SetColor(Color(1.0f, 0.95f, 0.85f));
    light->SetCastShadows(true);
    light->SetShadowDistance(220.0f);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    light->SetShadowCascade(CascadeParameters(20.0f, 45.0f, 90.0f, 180.0f, 0.8f));

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    if (!cache)
        return;

    // Visual sun object (sphere) so the light source is visible in scene.
    Node* sunNode = scene_->CreateChild("SunSphere");
    sunNode->SetPosition(Vector3(-60.0f, 70.0f, -40.0f));
    sunNode->SetScale(8.0f);

    Model* sunModel = cache->GetResource<Model>("Models/Sphere.mdl");
    if (!sunModel)
        sunModel = cache->GetResource<Model>("Models/Box.mdl");

    if (sunModel)
    {
        StaticModel* sunStaticModel = sunNode->CreateComponent<StaticModel>();
        sunStaticModel->SetModel(sunModel);
        sunStaticModel->SetCastShadows(false);

        SharedPtr<Material> sunMaterial(new Material(context_));
        Technique* unlitTech = cache->GetResource<Technique>("Techniques/DiffUnlit.xml");
        Technique* fallbackLitTech = cache->GetResource<Technique>("Techniques/Diff.xml");
        if (unlitTech)
            sunMaterial->SetTechnique(0, unlitTech);
        else if (fallbackLitTech)
            sunMaterial->SetTechnique(0, fallbackLitTech);

        sunMaterial->SetShaderParameter("MatDiffColor", Vector4(1.0f, 0.82f, 0.25f, 1.0f));
        sunMaterial->SetShaderParameter("MatEmissiveColor", Vector3(1.2f, 0.75f, 0.2f));
        sunStaticModel->SetMaterial(0, sunMaterial);
    }

}

void Game::CreateWorldGeometry()
{
    // World geometry is now managed by the chunk system
    // The static geometry generation has been moved to GenerateChunkGeometry()
    // to avoid conflicts and allow dynamic loading/unloading
}

void Game::CreateViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();
    renderer->SetViewport(0, new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
}

void Game::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Game, HandleUpdate));
    SubscribeToEvent(E_INPUTFOCUS, URHO3D_HANDLER(Game, HandleInputFocus));
    SubscribeToEvent(E_TEXTINPUT, URHO3D_HANDLER(Game, HandleConsoleTextInput));
}

void Game::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    (void)eventType;

    const float timeStep = eventData[P_TIMESTEP].GetFloat();
    Input* input = GetSubsystem<Input>();
    const bool escPressed = input && input->GetKeyPress(KEY_ESCAPE);

    if (consoleVisible_)
    {
        if (escPressed)
            ToggleConsole();
        return;
    }

    if (escPressed)
    {
        bool menuVisible = pauseMenu_ && pauseMenu_->GetRoot() && pauseMenu_->GetRoot()->IsVisible();
        if (menuVisible || gameState_ == GameState::Paused)
        {
            if (pauseMenu_ && pauseMenu_->IsVideoMenuPage())
                pauseMenu_->ShowMainMenuPage();
            else
                HidePauseMenuAndResume();
        }
        else
            ShowPauseMenu();
        return;
    }

    if (gameState_ != GameState::Playing)
        return;

    HandleCameraLook();
    HandleMovement(timeStep);
}

// Pause menu has been moved to PauseMenu class in PauseMenu.h/.cpp

void Game::HandleCloseMenu(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;
    HidePauseMenuAndResume();
}

void Game::HandleDisplayModeButton(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;
    ToggleFullscreenMode();
}

void Game::HandleVideoButton(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;
    if (pauseMenu_)
        pauseMenu_->ShowVideoMenuPage();
}

void Game::HandleVideoBackButton(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;
    if (pauseMenu_)
        pauseMenu_->ShowMainMenuPage();
}

void Game::HandleAspectRatioButton(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;
    ToggleAspectRatioMode();
}

void Game::HandleViewDistanceSlider(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    using namespace SliderChanged;
    float rawValue = eventData[P_VALUE].GetFloat();
    // rawValue is in 0..7 (slider). Map to 1..8 for display/internal chunk distance
    int nextViewDistance = Clamp(static_cast<int>(rawValue + 0.5f) + 1, 1, 8); // chunk load distance in chunks, rounded to nearest int
    viewDistance_ = nextViewDistance;

    if (pauseMenu_ && pauseMenu_->GetViewDistanceSlider())
        // Keep slider consistent with visual mapping (internal value = display - 1)
        pauseMenu_->GetViewDistanceSlider()->SetValue(static_cast<float>(viewDistance_ - 1));

    if (pauseMenu_ && pauseMenu_->GetViewDistanceText())
    {
        std::string tmp = std::string("Дальность прорисовки: ") + std::to_string(viewDistance_);
        pauseMenu_->GetViewDistanceText()->SetText(tmp.c_str());
    }
    if (pauseMenu_ && pauseMenu_->GetChunkCountText())
        pauseMenu_->GetChunkCountText()->SetText("Прорисовано чанков: 1");
}

void Game::HandleExitButton(StringHash eventType, VariantMap& eventData)
{
    engine_->Exit();
}

void Game::HandleCameraLook()
{
    if (!cameraNode_)
        return;  // Safety check - camera not initialized

    Input* input = GetSubsystem<Input>();
    const IntVector2 mouseMove = input->GetMouseMove();

    yaw_ += mouseSensitivity_ * mouseMove.x_;
    pitch_ += mouseSensitivity_ * mouseMove.y_;
    pitch_ = Clamp(pitch_, -80.0f, 80.0f);

    cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
}

void Game::HandleMovement(float timeStep)
{
    if (!cameraNode_)
        return;  // Safety check - camera not initialized

    Input* input = GetSubsystem<Input>();
    const float moveStep = moveSpeed_ * timeStep;

    if (input->GetKeyDown(KEY_W) || input->GetScancodeDown(SCANCODE_W))
        cameraNode_->Translate(Vector3::FORWARD * moveStep);

    if (input->GetKeyDown(KEY_S) || input->GetScancodeDown(SCANCODE_S))
        cameraNode_->Translate(Vector3::BACK * moveStep);

    if (input->GetKeyDown(KEY_A) || input->GetScancodeDown(SCANCODE_A))
        cameraNode_->Translate(Vector3::LEFT * moveStep);

    if (input->GetKeyDown(KEY_D) || input->GetScancodeDown(SCANCODE_D))
        cameraNode_->Translate(Vector3::RIGHT * moveStep);

    if (input->GetKeyDown(KEY_SPACE))
        cameraNode_->Translate(Vector3::UP * moveStep);

    if (input->GetKeyDown(KEY_LCTRL))
        cameraNode_->Translate(Vector3::DOWN * moveStep);
}

void Game::HandleInputFocus(StringHash eventType, VariantMap& eventData)
{
    using namespace InputFocus;
    (void)eventType;

    bool minimized = eventData[P_MINIMIZED].GetBool();
    bool focus = eventData[P_FOCUS].GetBool();

    Input* input = GetSubsystem<Input>();
    UI* ui = GetSubsystem<UI>();
    bool menuVisible = pauseMenu_ && pauseMenu_->GetRoot() && pauseMenu_->GetRoot()->IsVisible();

    if (minimized || !focus)
    {
        URHO3D_LOGINFOF("Input focus lost or minimized: focus=%d minimized=%d", focus ? 1 : 0, minimized ? 1 : 0);
        ShowPauseMenu();
    }
    else
    {
        URHO3D_LOGINFOF("Input focus regained: focus=%d minimized=%d", focus ? 1 : 0, minimized ? 1 : 0);
        // Keep game paused until user explicitly closes menu.
        if (gameState_ == GameState::Paused)
        {
            if (!menuVisible)
                ShowPauseMenu();
            else
            {
                if (input)
                {
                    input->SetMouseVisible(true);
                    input->SetMouseMode(MM_ABSOLUTE);
                }
                if (ui && pauseMenu_ && pauseMenu_->GetRoot())
                    ui->SetFocusElement(pauseMenu_->GetRoot());
            }
        }
        else
        {
            if (input)
            {
                input->SetMouseVisible(false);
                input->SetMouseMode(MM_RELATIVE);
                input->SetMouseGrabbed(true);
            }
            if (ui)
                ui->SetFocusElement(nullptr);
        }
    }
}

std::pair<int, int> Game::GetChunkCoordinates(const Vector3& position) const
{
    int x = static_cast<int>(floor(position.x_ / CHUNK_SIZE));
    int z = static_cast<int>(floor(position.z_ / CHUNK_SIZE));
    return {x, z};
}

void Game::UpdateChunks()
{
    if (!cameraNode_ || !scene_)
        return;

    Vector3 cameraPos = cameraNode_->GetWorldPosition();
    std::pair<int, int> newChunkCoords = GetChunkCoordinates(cameraPos);

    // Only update if camera moved to a different chunk
    if (!chunksInitialized_ || newChunkCoords != currentChunkCoords_)
    {
        chunksInitialized_ = true;
        currentChunkCoords_ = newChunkCoords;

        // Load chunks within view distance
        for (int x = newChunkCoords.first - viewDistance_; x <= newChunkCoords.first + viewDistance_; ++x)
        {
            for (int z = newChunkCoords.second - viewDistance_; z <= newChunkCoords.second + viewDistance_; ++z)
            {
                auto key = std::make_pair(x, z);
                if (chunks_.find(key) == chunks_.end())
                {
                    LoadChunk(x, z);
                }
            }
        }

        // Unload chunks outside view distance
            {
                // Collect keys to unload first to avoid modifying the map while iterating
                std::vector<std::pair<int, int>> keysToUnload;
                for (const auto& kv : chunks_)
                {
                    int dx = abs(kv.first.first - newChunkCoords.first);
                    int dz = abs(kv.first.second - newChunkCoords.second);
                    if (dx > viewDistance_ || dz > viewDistance_)
                        keysToUnload.push_back(kv.first);
                }

                for (const auto& key : keysToUnload)
                {
                    UnloadChunk(key.first, key.second);
                }
            }
    }

    // Update chunk count text (how many chunks are currently rendered)
    if (pauseMenu_ && pauseMenu_->GetChunkCountText())
    {
        int visibleCount = 0;
        for (const auto& kv : chunks_)
        {
            if (kv.second.node && kv.second.node->IsEnabled())
                ++visibleCount;
        }
        std::string cnt = std::string("Прорисовано чанков: ") + std::to_string(visibleCount);
        pauseMenu_->GetChunkCountText()->SetText(cnt.c_str());
    }
}

void Game::LoadChunk(int x, int z)
{
    if (!scene_)
        return;

    auto key = std::make_pair(x, z);
    if (chunks_.find(key) != chunks_.end())
        return;  // Already loaded

    Chunk chunk;
    chunk.x = x;
    chunk.z = z;
    chunk.isLoaded = false;  // Set to true only after successful geometry generation

    // Create a node for this chunk
    std::string chunkName = "Chunk_" + std::to_string(x) + "_" + std::to_string(z);
    chunk.node = scene_->CreateChild(chunkName.c_str());
    
    if (!chunk.node)
        return;  // Failed to create node

    chunk.node->SetPosition(Vector3(x * CHUNK_SIZE, 0.0f, z * CHUNK_SIZE));

    chunks_[key] = chunk;
    
    // Generate geometry after safely storing chunk
    GenerateChunkGeometry(x, z);
    
    // Mark as loaded only after geometry generation completes
    chunks_[key].isLoaded = true;
}

void Game::UnloadChunk(int x, int z)
{
    auto key = std::make_pair(x, z);
    auto it = chunks_.find(key);
    if (it == chunks_.end())
        return;

    if (it->second.node)
    {
        it->second.node->Remove();
    }
    chunks_.erase(it);
}

void Game::GenerateChunkGeometry(int x, int z)
{
    auto key = std::make_pair(x, z);
    auto it = chunks_.find(key);
    if (it == chunks_.end() || !it->second.node || !scene_)
        return;

    Node* chunkNode = it->second.node;
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    if (!cache)
        return;

    // Check if Box model exists before attempting to use it
    Model* boxModel = cache->GetResource<Model>("Models/Box.mdl");
    
    if (!boxModel)
    {
        // Resource not available, skip geometry generation for this chunk
        return;
    }

    // Try to load stone material, if not available use a simple white fallback
    Material* material = cache->GetResource<Material>("Materials/Stone.xml");
    if (!material)
    {
        // Use platform default material (should be safe)
        material = cache->GetResource<Material>("Materials/DefaultGrey.xml");
    }
    
    // If still no material, skip the SetMaterial call and let it use default Box material
    // This prevents segfault from null material assignment

    // Generate a simple grid of boxes for chunk geometry (can be replaced with actual terrain)
    // Larger step = fewer objects = better performance (currently step of 5 = only 16 boxes per chunk)
    const float step = 5.0f;
    int gridSize = (int)(CHUNK_SIZE / step);

    for (int i = 0; i < gridSize; ++i)
    {
        for (int j = 0; j < gridSize; ++j)
        {
            if (!chunkNode)
                return;  // Safety check

            Node* boxNode = chunkNode->CreateChild("Box");
            if (!boxNode)
                continue;

            boxNode->SetPosition(Vector3(i * step, 0.5f, j * step));
            boxNode->SetScale(0.9f);  // Slightly smaller to show grid

            StaticModel* boxModelComp = boxNode->CreateComponent<StaticModel>();
            if (boxModelComp)
            {
                boxModelComp->SetModel(boxModel);
                boxModelComp->SetCastShadows(true);
                // Only set material if we successfully loaded it
                if (material)
                    boxModelComp->SetMaterial(0, material);  // Material index 0 (first submesh)
            }
        }
    }
}
