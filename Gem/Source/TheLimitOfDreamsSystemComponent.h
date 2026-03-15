#pragma once

#include "Game.h"

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <TheLimitOfDreams/TheLimitOfDreamsBus.h>

namespace TheLimitOfDreams
{
    class TheLimitOfDreamsSystemComponent
        : public AZ::Component
        , protected TheLimitOfDreamsRequestBus::Handler
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(TheLimitOfDreamsSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        TheLimitOfDreamsSystemComponent();
        ~TheLimitOfDreamsSystemComponent() override;

    protected:
        void Init() override;
        void Activate() override;
        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void ExecuteConsoleCommand(const AZStd::string& commandLine) override;
        AZ::Vector3 GetPlayerWorldPosition() const override;

    private:
        AZStd::unique_ptr<Game> m_game;
    };
} // namespace TheLimitOfDreams
