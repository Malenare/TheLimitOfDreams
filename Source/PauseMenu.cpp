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
    panel->SetMinSize(520, 470);
    panel->SetColor(Color(0.05f, 0.05f, 0.06f, 0.9f));
    panel->SetName("PausePanel");
    panel->SetAlignment(HA_CENTER, VA_CENTER);

    if (ui)
    {
        IntVector2 rootSize = ui->GetRoot()->GetSize();
        root_->SetSize(rootSize);
    }

    mainPage_ = panel->CreateChild<UIElement>();
    mainPage_->SetSize(panel->GetSize());

    videoPage_ = panel->CreateChild<UIElement>();
    videoPage_->SetSize(panel->GetSize());

    auto* title = mainPage_->CreateChild<Text>();
    title->SetStyleAuto();
    title->SetText("PAUSED");
    title->SetHorizontalAlignment(HA_CENTER);
    title->SetVerticalAlignment(VA_TOP);
    title->SetPosition(0, 15);
    title->SetColor(Color::WHITE);
    if (uiFont)
        title->SetFont(uiFont, 30);

    // View distance controls
    viewDistanceText_ = mainPage_->CreateChild<Text>();
    viewDistanceText_->SetStyleAuto();
    viewDistanceText_->SetText("Дальность прорисовки: 1");
    viewDistanceText_->SetHorizontalAlignment(HA_CENTER);
    viewDistanceText_->SetPosition(0, 70);
    viewDistanceText_->SetColor(Color::WHITE);
    if (uiFont)
        viewDistanceText_->SetFont(uiFont, 20);

    viewDistanceSlider_ = mainPage_->CreateChild<Slider>();
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
    chunkCountText_ = mainPage_->CreateChild<Text>();
    chunkCountText_->SetStyleAuto();
    chunkCountText_->SetText("Прорисовано чанков: 0");
    chunkCountText_->SetHorizontalAlignment(HA_CENTER);
    chunkCountText_->SetPosition(0, 145);
    chunkCountText_->SetColor(Color::WHITE);
    if (uiFont)
        chunkCountText_->SetFont(uiFont, 18);

    // Video section button
    videoButton_ = mainPage_->CreateChild<Button>();
    videoButton_->SetStyleAuto();
    videoButton_->SetMinSize(360, 40);
    videoButton_->SetPosition(80, 185);
    videoButton_->SetName("VideoButton");

    auto* videoText = videoButton_->CreateChild<Text>();
    videoText->SetName("VideoText");
    videoText->SetStyleAuto();
    videoText->SetText("Видео");
    if (uiFont)
        videoText->SetFont(uiFont, 18);
    videoText->SetHorizontalAlignment(HA_CENTER);
    videoText->SetVerticalAlignment(VA_CENTER);
    videoText->SetColor(Color::BLACK);

    // Close menu button
    closeButton_ = mainPage_->CreateChild<Button>();
    closeButton_->SetStyleAuto();
    closeButton_->SetMinSize(360, 40);
    closeButton_->SetPosition(80, 335);
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
    exitButton_ = mainPage_->CreateChild<Button>();
    exitButton_->SetStyleAuto();
    exitButton_->SetMinSize(360, 40);
    exitButton_->SetPosition(80, 385);
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

    // Video page title
    auto* videoTitle = videoPage_->CreateChild<Text>();
    videoTitle->SetStyleAuto();
    videoTitle->SetText("VIDEO");
    videoTitle->SetHorizontalAlignment(HA_CENTER);
    videoTitle->SetVerticalAlignment(VA_TOP);
    videoTitle->SetPosition(0, 25);
    videoTitle->SetColor(Color::WHITE);
    if (uiFont)
        videoTitle->SetFont(uiFont, 30);

    // Display mode button
    displayModeButton_ = videoPage_->CreateChild<Button>();
    displayModeButton_->SetStyleAuto();
    displayModeButton_->SetMinSize(360, 40);
    displayModeButton_->SetPosition(80, 115);
    displayModeButton_->SetName("DisplayModeButton");

    displayModeText_ = displayModeButton_->CreateChild<Text>();
    displayModeText_->SetName("DisplayModeText");
    displayModeText_->SetStyleAuto();
    displayModeText_->SetText("Режим: Оконный");
    if (uiFont)
        displayModeText_->SetFont(uiFont, 18);
    displayModeText_->SetHorizontalAlignment(HA_CENTER);
    displayModeText_->SetVerticalAlignment(VA_CENTER);
    displayModeText_->SetColor(Color::BLACK);

    // Aspect ratio button
    aspectRatioButton_ = videoPage_->CreateChild<Button>();
    aspectRatioButton_->SetStyleAuto();
    aspectRatioButton_->SetMinSize(360, 40);
    aspectRatioButton_->SetPosition(80, 165);
    aspectRatioButton_->SetName("AspectRatioButton");

    aspectRatioText_ = aspectRatioButton_->CreateChild<Text>();
    aspectRatioText_->SetName("AspectRatioText");
    aspectRatioText_->SetStyleAuto();
    aspectRatioText_->SetText("Соотношение: 16:9");
    if (uiFont)
        aspectRatioText_->SetFont(uiFont, 18);
    aspectRatioText_->SetHorizontalAlignment(HA_CENTER);
    aspectRatioText_->SetVerticalAlignment(VA_CENTER);
    aspectRatioText_->SetColor(Color::BLACK);

    // Back button
    videoBackButton_ = videoPage_->CreateChild<Button>();
    videoBackButton_->SetStyleAuto();
    videoBackButton_->SetMinSize(360, 40);
    videoBackButton_->SetPosition(80, 235);
    videoBackButton_->SetName("VideoBackButton");

    auto* backText = videoBackButton_->CreateChild<Text>();
    backText->SetName("VideoBackText");
    backText->SetStyleAuto();
    backText->SetText("Назад");
    if (uiFont)
        backText->SetFont(uiFont, 18);
    backText->SetHorizontalAlignment(HA_CENTER);
    backText->SetVerticalAlignment(VA_CENTER);
    backText->SetColor(Color::BLACK);

    // Add to UI root
    if (ui)
        ui->GetRoot()->AddChild(root_);

    ShowMainMenuPage();
}

void PauseMenu::ShowMainMenuPage()
{
    if (mainPage_)
    {
        mainPage_->SetVisible(true);
        mainPage_->SetEnabled(true);
    }
    if (videoPage_)
    {
        videoPage_->SetVisible(false);
        videoPage_->SetEnabled(false);
    }
}

void PauseMenu::ShowVideoMenuPage()
{
    if (mainPage_)
    {
        mainPage_->SetVisible(false);
        mainPage_->SetEnabled(false);
    }
    if (videoPage_)
    {
        videoPage_->SetVisible(true);
        videoPage_->SetEnabled(true);
    }
}

bool PauseMenu::IsVideoMenuPage() const
{
    return videoPage_ && videoPage_->IsVisible();
}
