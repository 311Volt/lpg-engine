//
// Created by volt on 12/22/2024.
//

#ifndef LPG_ENGINE_ENTITY_HPP
#define LPG_ENGINE_ENTITY_HPP

#include <string_view>
#include <memory>
#include <array>
#include <axxegro/com/math/math.hpp> //TODO get rid of this dependency eventually
#include <axxegro/core/Transform.hpp>

#include "data.hpp"

#include "../util/strided_span.hpp"

namespace lpg {

    struct BaseEntity {};

    template<typename T>
    concept Entity = std::is_aggregate_v<T>;

    using EntityID = uint32_t;
    using EntityVersionNumber = uint32_t;
    using EntityDescriptor = uint32_t;

    namespace detail {
        template<typename T>
        struct HandlesMessage {
            using LPGHandlesMessageTag = void;
            using Type = T;
        };

        inline int32_t EntityTypeIdCounter = 0;
        inline int32_t MessageTypeIdCounter = 0;

        template<typename TEntity>
        int32_t GetEntityTypeId() {
            static int32_t id = EntityTypeIdCounter++;
            return id;
        }

        template<typename TMessage>
        int32_t GetMessageTypeId() {
            static int32_t id = MessageTypeIdCounter++;
            return id;
        }
    }




#define LPG_MESSAGE_HANDLER(Type) \
    LPG_NO_UNIQUE_ADDRESS ::lpg::refl::TypeTag<::lpg::detail::HandlesMessage<Type>{}> LPG_CONCAT(lpg___msg_tag_, __LINE__)

    struct ComponentInfo {
        int entityTypeId;
        std::string name;
        int position;
        int offset;
    };

    struct PropertyInfo {
        std::string name;
        int position;
        int offset;
    };

    struct EntityInterface {

        std::string name;
        int32_t entitySize;
        int32_t entityAlign;

        std::vector<ComponentInfo> embeddedComponents;
        std::vector<ComponentInfo> managedComponents;
        std::vector<PropertyInfo> properties;

        void (*destroy)(void*);
        void (*swap)(void*, void*);
        void (*move)(void*, void*);
        void (*copy)(void*, void*);

        al::Vec3f (*getPosition)(void*);
        al::Vec3f (*getRotation)(void*);
        al::Vec3f (*getScale)(void*);
        al::Vec3f (*getLocalPosition)(void*);
        al::Vec3f (*getLocalRotation)(void*);
        al::Vec3f (*getLocalScale)(void*);
        void (*accumulateTransform)(void*, al::Transform&);
        void (*accumulateLocalTransform)(void*, al::Transform&);

        std::vector<int32_t> (*getChildrenIds)(void*);
        int32_t (*getParentId)(void*);
        int32_t (*getId)(void*);

        void (*toJSON)(void*, std::string&);
        void (*fromJSON)(void*, std::string&);

        int32_t (*propertyNamePerfectHash)(std::string_view);
        std::vector<void(*)(void* ent, void* prop)> setProperty;
        std::vector<void*(*)(void* ent)> getProperty;

        std::vector<void(*)(void* msg, void* ent)> sendMessage;
        std::vector<void(*)(void* msg, TypeErasedStridedSpan ent)> sendMessageToMany;
        std::vector<void(*)(void* msg, void* ent, size_t)> sendMessageToManyContiguous;

    };

    template<Entity TEntity>
    struct EntityQueryResult {
        std::vector<std::span<TEntity>> results;
    };


    struct AnyRef {
        using IsEntityReference = void;
        EntityID id = 0;
        EntityVersionNumber version = 0;
    };

    template<Entity TEntity>
    struct Ref {
        using IsEntityReference = void;
        using EntityType = TEntity;

        EntityID id = 0;
        EntityVersionNumber version = 0;
    };

    struct AnyChild {
        EntityID id = 0;
    };

    template<Entity TEntity>
    struct Child {
        using IsEntityChild = void;
        using EntityType = TEntity;
        EntityID id = 0;
    };

    template<Entity TEntity>
    struct Managed {
        using IsManagedComponent = void;
        using EntityType = TEntity;
    };


    struct BaseGameObject {
        EntityID id;
        EntityID parentId;

        al::Vec3f position{}, rotation{}, scale{1,1,1};
        al::Vec3f localPosition{}, localRotation{}, localScale{1,1,1};
    };


}


#endif //LPG_ENGINE_ENTITY_HPP
