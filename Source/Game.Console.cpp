#include "Game.h"

#include <algorithm>
#include <cctype>
#include <sstream>

#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Font.h>

Vector3 Game::UserToWorldCoords(float x, float y, float z) const
{
    // User space: X/Y on ground plane, Z is height.
    // World space: X/Z on ground plane, Y is height.
    return Vector3(x, z, y);
}

Vector3 Game::WorldToUserCoords(const Vector3& world) const
{
    return Vector3(world.x_, world.z_, world.y_);
}

void Game::CreateConsoleUI()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    if (!ui)
        return;

    Font* uiFont = cache ? cache->GetResource<Font>("Fonts/Arial Unicode.ttf") : nullptr;
    if (!uiFont && cache)
        uiFont = cache->GetResource<Font>("Fonts/Calibri.ttf");

    consoleRoot_ = new UIElement(context_);
    consoleRoot_->SetName("DeveloperConsole");
    consoleRoot_->SetAlignment(HA_LEFT, VA_TOP);
    consoleRoot_->SetPosition(16, 16);
    consoleRoot_->SetMinSize(640, 260);
    consoleRoot_->SetVisible(false);
    consoleRoot_->SetEnabled(false);

    auto* panel = consoleRoot_->CreateChild<BorderImage>();
    panel->SetMinSize(640, 260);
    panel->SetColor(Color(0.03f, 0.03f, 0.03f, 0.88f));

    consoleOutput_ = panel->CreateChild<Text>();
    consoleOutput_->SetPosition(12, 12);
    consoleOutput_->SetMinSize(616, 194);
    consoleOutput_->SetColor(Color(0.86f, 0.95f, 0.86f));
    consoleOutput_->SetWordwrap(true);
    if (uiFont)
        consoleOutput_->SetFont(uiFont, 16);

    consoleInput_ = panel->CreateChild<LineEdit>();
    consoleInput_->SetName("ConsoleInput");
    consoleInput_->SetPosition(12, 220);
    consoleInput_->SetMinSize(616, 30);
    consoleInput_->SetStyleAuto();
    consoleInput_->SetColor(Color(0.96f, 0.96f, 0.96f));
    if (UIElement* lineTextElement = consoleInput_->GetChild("LineEditText", true))
    {
        if (Text* textChild = lineTextElement->Cast<Text>())
        {
            textChild->SetColor(Color::BLACK);
            if (uiFont)
                textChild->SetFont(uiFont, 16);
        }
    }

    ui->GetRoot()->AddChild(consoleRoot_);
    AddConsoleLine("Console ready. xyz mode: Z=height. Commands: teleport x y z, speed value, coordinate");
}

void Game::ToggleConsole()
{
    UI* ui = GetSubsystem<UI>();
    Input* in = GetSubsystem<Input>();
    consoleVisible_ = !consoleVisible_;

    if (consoleRoot_)
    {
        consoleRoot_->SetVisible(consoleVisible_);
        consoleRoot_->SetEnabled(consoleVisible_);
    }

    if (consoleVisible_)
    {
        if (consoleInput_)
        {
            consoleInput_->SetText("");
            consoleInput_->SetFocus(true);
        }
        if (ui && consoleInput_)
            ui->SetFocusElement(consoleInput_);
        if (in)
        {
            in->SetMouseVisible(true);
            in->SetMouseMode(MM_ABSOLUTE);
            in->SetMouseGrabbed(false);
        }
    }
    else
    {
        if (ui)
            ui->SetFocusElement(nullptr);
        if (in && gameState_ == GameState::Playing)
        {
            in->SetMouseVisible(false);
            in->SetMouseMode(MM_RELATIVE);
            in->SetMouseGrabbed(true);
        }
    }
}

void Game::AddConsoleLine(const ea::string& line)
{
    if (!consoleOutput_)
        return;

    consoleLines_.push_back(line);
    if (consoleLines_.size() > 10)
        consoleLines_.erase(consoleLines_.begin());

    ea::string merged;
    for (const auto& l : consoleLines_)
    {
        if (!merged.empty())
            merged += '\n';
        merged += l;
    }
    consoleOutput_->SetText(merged);
}

void Game::ExecuteConsoleCommand(const ea::string& commandLine)
{
    const ea::string trimmed = commandLine.trimmed();
    if (trimmed.empty())
        return;

    AddConsoleLine("> " + trimmed);

    std::istringstream iss(trimmed.c_str());
    std::string command;
    iss >> command;
    std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (command == "teleport")
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        if (!(iss >> x >> y >> z))
        {
            AddConsoleLine("Usage: teleport <x> <y> <z>");
            return;
        }
        if (!cameraNode_)
        {
            AddConsoleLine("Player not initialized.");
            return;
        }
        cameraNode_->SetWorldPosition(UserToWorldCoords(x, y, z));
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
        moveSpeed_ = value;
        AddConsoleLine("Speed set to " + ea::to_string(value));
        return;
    }

    if (command == "coordinate")
    {
        if (!cameraNode_)
        {
            AddConsoleLine("Player not initialized.");
            return;
        }
        const Vector3 userPos = WorldToUserCoords(cameraNode_->GetWorldPosition());
        AddConsoleLine("xyz: " + ea::to_string(userPos.x_) + " " + ea::to_string(userPos.y_) + " " + ea::to_string(userPos.z_));
        return;
    }

    AddConsoleLine("Unknown command. Available: teleport, speed, coordinate");
}

void Game::HandleConsoleTextFinished(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    (void)eventData;
    if (!consoleInput_)
        return;

    ExecuteConsoleCommand(consoleInput_->GetText());
    consoleInput_->SetText("");
    consoleInput_->SetFocus(true);
}

void Game::HandleConsoleTextInput(StringHash eventType, VariantMap& eventData)
{
    (void)eventType;
    using namespace TextInput;
    const ea::string text = eventData[P_TEXT].GetString();
    if (text.empty())
        return;

    // Open console from different layouts using the same physical key.
    if (text == "`" || text == u8"ё" || text == u8"Ё" || text == u8"ë" || text == u8"Ë")
    {
        if (!consoleVisible_)
            ToggleConsole();
    }
}
