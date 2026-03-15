#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include "TheLimitOfDreamsSystemComponent.h"

#include <TheLimitOfDreams/TheLimitOfDreamsTypeIds.h>

namespace TheLimitOfDreams
{
    class TheLimitOfDreamsModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(TheLimitOfDreamsModule, TheLimitOfDreamsModuleTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(TheLimitOfDreamsModule, AZ::SystemAllocator);

        TheLimitOfDreamsModule()
            : AZ::Module()
        {
            m_descriptors.insert(m_descriptors.end(), {
                TheLimitOfDreamsSystemComponent::CreateDescriptor(),
            });
        }

        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<TheLimitOfDreamsSystemComponent>(),
            };
        }
    };
} // namespace TheLimitOfDreams

#if defined(O3DE_GEM_NAME)
AZ_DECLARE_MODULE_CLASS(AZ_JOIN(Gem_, O3DE_GEM_NAME), TheLimitOfDreams::TheLimitOfDreamsModule)
#else
AZ_DECLARE_MODULE_CLASS(Gem_TheLimitOfDreams, TheLimitOfDreams::TheLimitOfDreamsModule)
#endif
