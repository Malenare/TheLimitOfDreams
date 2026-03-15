#include "TheLimitOfDreamsSystemComponent.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/smart_ptr/make_unique.h>

#include <TheLimitOfDreams/TheLimitOfDreamsTypeIds.h>

namespace TheLimitOfDreams
{
    AZ_COMPONENT_IMPL(TheLimitOfDreamsSystemComponent, "TheLimitOfDreamsSystemComponent", TheLimitOfDreamsSystemComponentTypeId);

    void TheLimitOfDreamsSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TheLimitOfDreamsSystemComponent, AZ::Component>()
                ->Version(0);
        }
    }

    void TheLimitOfDreamsSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("TheLimitOfDreamsService"));
    }

    void TheLimitOfDreamsSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("TheLimitOfDreamsService"));
    }

    void TheLimitOfDreamsSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void TheLimitOfDreamsSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    TheLimitOfDreamsSystemComponent::TheLimitOfDreamsSystemComponent()
    {
        if (TheLimitOfDreamsInterface::Get() == nullptr)
        {
            TheLimitOfDreamsInterface::Register(this);
        }
    }

    TheLimitOfDreamsSystemComponent::~TheLimitOfDreamsSystemComponent()
    {
        if (TheLimitOfDreamsInterface::Get() == this)
        {
            TheLimitOfDreamsInterface::Unregister(this);
        }
    }

    void TheLimitOfDreamsSystemComponent::Init()
    {
    }

    void TheLimitOfDreamsSystemComponent::Activate()
    {
        TheLimitOfDreamsRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();

        m_game = AZStd::make_unique<Game>();
        m_game->Activate();
    }

    void TheLimitOfDreamsSystemComponent::Deactivate()
    {
        if (m_game)
        {
            m_game->Deactivate();
            m_game.reset();
        }

        AZ::TickBus::Handler::BusDisconnect();
        TheLimitOfDreamsRequestBus::Handler::BusDisconnect();
    }

    void TheLimitOfDreamsSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_game)
        {
            m_game->OnTick(deltaTime);
        }
    }

    void TheLimitOfDreamsSystemComponent::ExecuteConsoleCommand(const AZStd::string& commandLine)
    {
        if (m_game)
        {
            m_game->ExecuteConsoleCommand(commandLine);
        }
    }

    AZ::Vector3 TheLimitOfDreamsSystemComponent::GetPlayerWorldPosition() const
    {
        return m_game ? m_game->GetPlayerWorldPosition() : AZ::Vector3::CreateZero();
    }
} // namespace TheLimitOfDreams
