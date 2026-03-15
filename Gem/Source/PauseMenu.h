#pragma once

#include <AzCore/std/string/string.h>

namespace TheLimitOfDreams
{
    class PauseMenu
    {
    public:
        void ShowMainMenuPage();
        void ShowVideoMenuPage();
        bool IsVideoMenuPage() const;

        void SetVisible(bool visible);
        bool IsVisible() const;

        void SetViewDistance(int viewDistance);
        int GetViewDistance() const;

        void SetChunkCount(int chunkCount);
        const AZStd::string& GetViewDistanceText() const;
        const AZStd::string& GetChunkCountText() const;
        const AZStd::string& GetDisplayModeText() const;
        const AZStd::string& GetAspectRatioText() const;

        void SetDisplayModeText(bool isFullscreen);
        void SetAspectRatioText(bool is16x10);

    private:
        enum class Page
        {
            Main,
            Video
        };

        void RefreshViewDistanceText();
        void RefreshChunkCountText();

        bool m_visible{ false };
        Page m_page{ Page::Main };
        int m_viewDistance{ 1 };
        int m_chunkCount{ 0 };

        AZStd::string m_viewDistanceText{ "View distance: 1" };
        AZStd::string m_chunkCountText{ "Visible chunks: 0" };
        AZStd::string m_displayModeText{ "Display mode: Windowed" };
        AZStd::string m_aspectRatioText{ "Aspect ratio: 16:9" };
    };
} // namespace TheLimitOfDreams
