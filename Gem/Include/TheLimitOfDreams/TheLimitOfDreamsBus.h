#pragma once

#include <TheLimitOfDreams/TheLimitOfDreamsTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Math/Vector3.h>

namespace TheLimitOfDreams
{
    class TheLimitOfDreamsRequests
    {
    public:
        AZ_RTTI(TheLimitOfDreamsRequests, TheLimitOfDreamsRequestsTypeId);
        virtual ~TheLimitOfDreamsRequests() = default;

        virtual void ExecuteConsoleCommand(const AZStd::string& commandLine) = 0;
        virtual AZ::Vector3 GetPlayerWorldPosition() const = 0;
    };

    class TheLimitOfDreamsBusTraits
        : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };

    using TheLimitOfDreamsRequestBus = AZ::EBus<TheLimitOfDreamsRequests, TheLimitOfDreamsBusTraits>;
    using TheLimitOfDreamsInterface = AZ::Interface<TheLimitOfDreamsRequests>;
} // namespace TheLimitOfDreams
