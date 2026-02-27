#include "PauseMenu.h"
#include <Urho3D/Resource/ResourceCache.h>

PauseMenu::PauseMenu(Context* context) : Object(context)
{
}

void PauseMenu::Create(UI* ui, ResourceCache* cache)
{
    // Try to load a font
    Font* uiFont = nullptr;
    if (cache)
    {
        uiFont = cache->GetResource<Font>("Fonts/Arial Unicode.ttf");
        if (!uiFont)
            uiFont = cache->GetResource<Font>("Fonts/Calibri.ttf");
    }

    root_ = new UIElement(context_);
    root_->SetName("PauseMenu");
    root_->SetAlignment(HA_CENTER, VA_CENTER);
    root_->SetVisible(false);
    root_->SetEnabled(false);

    auto* panel = root_->CreateChild<BorderImage>();
    panel->SetMinSize(450, 300);
    panel->SetColor(Color(0.05f, 0.05f, 0.06f, 0.9f));
    panel->SetName("PausePanel");
    panel->SetAlignment(HA_CENTER, VA_CENTER);

    if (ui)
    {
        IntVector2 rootSize = ui->GetRoot()->GetSize();
        root_->SetSize(rootSize);
    }

    auto* title = panel->CreateChild<Text>();
    title->SetStyleAuto();
    title->SetText("PAUSED");
    title->SetHorizontalAlignment(HA_CENTER);
    title->SetVerticalAlignment(VA_TOP);
    title->SetPosition(0, 15);
    title->SetColor(Color::WHITE);
    if (uiFont)
        title->SetFont(uiFont, 30);

    // View distance controls
    viewDistanceText_ = panel->CreateChild<Text>();
    viewDistanceText_->SetStyleAuto();
    viewDistanceText_->SetText("Дальность прорисовки: 1");
    viewDistanceText_->SetHorizontalAlignment(HA_CENTER);
    viewDistanceText_->SetPosition(0, 70);
    viewDistanceText_->SetColor(Color::WHITE);
    if (uiFont)
        viewDistanceText_->SetFont(uiFont, 20);

    viewDistanceSlider_ = panel->CreateChild<Slider>();
    viewDistanceSlider_->SetStyleAuto();
    viewDistanceSlider_->SetName("ViewDistanceSlider");
    viewDistanceSlider_->SetMinSize(330, 24);
    viewDistanceSlider_->SetPosition(60, 105);
    // Internally use range 0..7 so displayed distance maps to 1..8 (visual minimum 1)
    viewDistanceSlider_->SetRange(7.0f);
    viewDistanceSlider_->SetValue(0.0f); // display 1 -> slider value 0
    // Make slider clearly visible against dark menu background.
    viewDistanceSlider_->SetColor(Color(0.15f, 0.72f, 0.93f, 1.0f));
    UIElement* sliderKnob = viewDistanceSlider_->GetChild("SliderKnob", true);
    if (!sliderKnob && viewDistanceSlider_->GetNumChildren() > 0)
        sliderKnob = viewDistanceSlider_->GetChild(0);
    if (sliderKnob)
        sliderKnob->SetColor(Color(0.95f, 0.95f, 0.95f, 1.0f));

    // Chunk count label
    chunkCountText_ = panel->CreateChild<Text>();
    chunkCountText_->SetStyleAuto();
    chunkCountText_->SetText("Прорисовано чанков: 0");
    chunkCountText_->SetHorizontalAlignment(HA_CENTER);
    chunkCountText_->SetPosition(0, 145);
    chunkCountText_->SetColor(Color::WHITE);
    if (uiFont)
        chunkCountText_->SetFont(uiFont, 18);

    // Close menu button
    closeButton_ = panel->CreateChild<Button>();
    closeButton_->SetStyleAuto();
    closeButton_->SetMinSize(300, 40);
    closeButton_->SetPosition(75, 185);
    closeButton_->SetName("CloseMenuButton");

    auto* ctxt = closeButton_->CreateChild<Text>();
    ctxt->SetName("CloseMenuText");
    ctxt->SetStyleAuto();
    ctxt->SetText("Закрыть меню");
    if (uiFont)
        ctxt->SetFont(uiFont, 18);
    ctxt->SetHorizontalAlignment(HA_CENTER);
    ctxt->SetVerticalAlignment(VA_CENTER);
    ctxt->SetColor(Color::BLACK);

    // Exit button
    exitButton_ = panel->CreateChild<Button>();
    exitButton_->SetStyleAuto();
    exitButton_->SetMinSize(300, 40);
    exitButton_->SetPosition(75, 235);
    exitButton_->SetName("ExitButton");

    auto* etxt = exitButton_->CreateChild<Text>();
    etxt->SetName("ExitText");
    etxt->SetStyleAuto();
    etxt->SetText("Exit Game");
    if (uiFont)
        etxt->SetFont(uiFont, 18);
    etxt->SetHorizontalAlignment(HA_CENTER);
    etxt->SetVerticalAlignment(VA_CENTER);
    etxt->SetColor(Color::BLACK);

    // Add to UI root
    if (ui)
        ui->GetRoot()->AddChild(root_);
}
