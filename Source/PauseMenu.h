#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Slider.h>

using namespace Urho3D;

class PauseMenu : public Object
{
    URHO3D_OBJECT(PauseMenu, Object);

public:
    explicit PauseMenu(Context* context);

    void Create(UI* ui, ResourceCache* cache);
    void ShowMainMenuPage();
    void ShowVideoMenuPage();
    bool IsVideoMenuPage() const;
    WeakPtr<UIElement> GetRoot() const { return root_; }
    WeakPtr<Button> GetCloseButton() const { return closeButton_; }
    WeakPtr<Button> GetExitButton() const { return exitButton_; }
    WeakPtr<Button> GetVideoButton() const { return videoButton_; }
    WeakPtr<Button> GetVideoBackButton() const { return videoBackButton_; }
    WeakPtr<Button> GetDisplayModeButton() const { return displayModeButton_; }
    WeakPtr<Button> GetAspectRatioButton() const { return aspectRatioButton_; }
    WeakPtr<Slider> GetViewDistanceSlider() const { return viewDistanceSlider_; }
    WeakPtr<Text> GetViewDistanceText() const { return viewDistanceText_; }
    WeakPtr<Text> GetChunkCountText() const { return chunkCountText_; }
    WeakPtr<Text> GetDisplayModeText() const { return displayModeText_; }
    WeakPtr<Text> GetAspectRatioText() const { return aspectRatioText_; }
    void SetVisible(bool v) { if (root_) { root_->SetVisible(v); root_->SetEnabled(v); } }

private:
    WeakPtr<UIElement> root_;
    WeakPtr<UIElement> mainPage_;
    WeakPtr<UIElement> videoPage_;
    WeakPtr<Button> closeButton_;
    WeakPtr<Button> exitButton_;
    WeakPtr<Button> videoButton_;
    WeakPtr<Button> videoBackButton_;
    WeakPtr<Button> displayModeButton_;
    WeakPtr<Button> aspectRatioButton_;
    WeakPtr<Slider> viewDistanceSlider_;
    WeakPtr<Text> viewDistanceText_;
    WeakPtr<Text> chunkCountText_;
    WeakPtr<Text> displayModeText_;
    WeakPtr<Text> aspectRatioText_;
};
