#include "PauseMenu.h"

#include <AzCore/std/algorithm.h>
#include <AzCore/std/string/string_view.h>

namespace TheLimitOfDreams
{
    void PauseMenu::ShowMainMenuPage()
    {
        m_page = Page::Main;
    }

    void PauseMenu::ShowVideoMenuPage()
    {
        m_page = Page::Video;
    }

    bool PauseMenu::IsVideoMenuPage() const
    {
        return m_page == Page::Video;
    }

    void PauseMenu::SetVisible(bool visible)
    {
        m_visible = visible;
    }

    bool PauseMenu::IsVisible() const
    {
        return m_visible;
    }

    void PauseMenu::SetViewDistance(int viewDistance)
    {
        m_viewDistance = AZStd::clamp(viewDistance, 1, 8);
        RefreshViewDistanceText();
    }

    int PauseMenu::GetViewDistance() const
    {
        return m_viewDistance;
    }

    void PauseMenu::SetChunkCount(int chunkCount)
    {
        m_chunkCount = AZStd::max(chunkCount, 0);
        RefreshChunkCountText();
    }

    const AZStd::string& PauseMenu::GetViewDistanceText() const
    {
        return m_viewDistanceText;
    }

    const AZStd::string& PauseMenu::GetChunkCountText() const
    {
        return m_chunkCountText;
    }

    const AZStd::string& PauseMenu::GetDisplayModeText() const
    {
        return m_displayModeText;
    }

    const AZStd::string& PauseMenu::GetAspectRatioText() const
    {
        return m_aspectRatioText;
    }

    void PauseMenu::SetDisplayModeText(bool isFullscreen)
    {
        m_displayModeText = isFullscreen ? "Display mode: Fullscreen" : "Display mode: Windowed";
    }

    void PauseMenu::SetAspectRatioText(bool is16x10)
    {
        m_aspectRatioText = is16x10 ? "Aspect ratio: 16:10" : "Aspect ratio: 16:9";
    }

    void PauseMenu::RefreshViewDistanceText()
    {
        m_viewDistanceText = AZStd::string::format("View distance: %d", m_viewDistance);
    }

    void PauseMenu::RefreshChunkCountText()
    {
        m_chunkCountText = AZStd::string::format("Visible chunks: %d", m_chunkCount);
    }
} // namespace TheLimitOfDreams
