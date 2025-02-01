//
// Created by volt on 2025-01-07.
//

// Only include this header in translation units that are meant to contain generated code for an entity type.


#ifndef LPG_ENGINE_SRC_LPG_CORE_ENTITY_CODEGEN_HPP_
#define LPG_ENGINE_SRC_LPG_CORE_ENTITY_CODEGEN_HPP_

#include <reflect>
#include <ranges>
#include <vector>
#include <typeindex>
#include <concepts>
#include "data.hpp"
#include "message.hpp"
#include "entity.hpp"

#include <axxegro/com/math/math.hpp>
#include <axxegro/core/Transform.hpp>

namespace lpg {

    namespace detail {


#define LPG_DATA_MEMBER_OR_DEFAULT(Var, MemberName, Default, ...) \
    [&](){ \
        if constexpr(requires{(Var).MemberName;}) { \
            return (Var).MemberName; \
        } \
        return (Default); \
    }()

        template<typename TEntity>
        inline std::vector<ComponentInfo> GetEntityEmbeddedComponentsInfo(int baseOffset = 0) {
            std::vector<ComponentInfo> result;

            refl::for_each_decl<TEntity>([&](auto I) {
                using FieldType = refl::member_type<I, TEntity>;

                if constexpr(refl::has_member_attr<IsComponent, I, TEntity>() || requires{typename FieldType::IsEntity;}) {
                    int fieldOffset = baseOffset + refl::member_offset<I, TEntity>();
                    result.push_back(ComponentInfo {
                        .name = refl::member_name<I, TEntity>(),
                        .offset = fieldOffset,
                        .position = I,
                        .entityTypeId = GetEntityTypeId<TEntity>()
                    });

                    auto subResult = GetEntityEmbeddedComponentsInfo<FieldType>(fieldOffset);
                    result.insert(result.end(), subResult.begin(), subResult.end());
                }

            });

            return result;

        }


        template<typename TEntity>
        al::Vec3f EntGetPosition(const TEntity& entity) {
            return LPG_DATA_MEMBER_OR_DEFAULT(entity, position, al::Vec3f{});
        }

        template<typename TEntity>
        al::Vec3f EntGetLocalPosition(const TEntity& entity) {
            return LPG_DATA_MEMBER_OR_DEFAULT(entity, localPosition, al::Vec3f{});
        }

        template<typename TEntity>
        al::Vec3f EntGetScale(const TEntity& entity) {
            static constexpr al::Vec3f defaultScale {1,1,1};
            return LPG_DATA_MEMBER_OR_DEFAULT(entity, scale, defaultScale);
        }

        template<typename TEntity>
        al::Vec3f EntGetLocalScale(const TEntity& entity) {
            static constexpr al::Vec3f defaultScale {1,1,1};
            return LPG_DATA_MEMBER_OR_DEFAULT(entity, localScale, defaultScale);
        }

        template<typename TEntity>
        al::Vec3f EntGetRotation(const TEntity& entity) {
            return LPG_DATA_MEMBER_OR_DEFAULT(entity, rotation, al::Vec3f{});
        }

        template<typename TEntity>
        al::Vec3f EntGetLocalRotation(const TEntity& entity) {
            return LPG_DATA_MEMBER_OR_DEFAULT(entity, localRotation, al::Vec3f{});
        }

        template<typename TEntity>
        void EntApplyTransform(const TEntity& entity, al::Transform& transform) {
            if constexpr(reflect::has_member_name<TEntity, "rotation">) {
                transform.rotate(EntGetRotation(entity));
            }
            if constexpr(reflect::has_member_name<TEntity, "scale">) {
                transform.scale(EntGetScale(entity));
            }
            if constexpr(reflect::has_member_name<TEntity, "position">) {
                transform.translate(EntGetPosition(entity));
            }
        }

        template<typename TEntity>
        void EntApplyLocalTransform(const TEntity& entity, al::Transform& transform) {
            if constexpr(reflect::has_member_name<TEntity, "rotation">) {
                transform.rotate(EntGetLocalRotation(entity));
            }
            if constexpr(reflect::has_member_name<TEntity, "scale">) {
                transform.scale(EntGetLocalScale(entity));
            }
            if constexpr(reflect::has_member_name<TEntity, "position">) {
                transform.translate(EntGetLocalPosition(entity));
            }
        }

    }



    template<typename TEntity>
    inline EntityInterface CreateEntityInterface(MessageRegistry& registry) {
        EntityInterface result {};

        result.name = reflect::type_name<TEntity>();

        result.entitySize = sizeof(TEntity);
        result.entityAlign = alignof(TEntity);

        result.embeddedComponents = detail::GetEntityEmbeddedComponentsInfo<TEntity>();

        //TODO managed components

        result.destroy = [](void* entity) {
            TEntity* entityPtr = static_cast<TEntity*>(entity);
            std::destroy_at(entityPtr);
        };

        result.swap = [](void* entity1, void* entity2) {
            TEntity* entity1Ptr = static_cast<TEntity*>(entity1);
            TEntity* entity2Ptr = static_cast<TEntity*>(entity2);
            std::swap(*entity1Ptr, *entity2Ptr);
        };

        result.copy = [](void* entity1, void* entity2) {
            TEntity* entity1Ptr = static_cast<TEntity*>(entity1);
            TEntity* entity2Ptr = static_cast<TEntity*>(entity2);
            *entity1Ptr = *entity2Ptr;
        };

        result.move = [](void* entity1, void* entity2) {
            TEntity* entity1Ptr = static_cast<TEntity*>(entity1);
            TEntity* entity2Ptr = static_cast<TEntity*>(entity2);
            *entity1Ptr = std::move(*entity2Ptr);
        };
        
        result.getPosition = [](void* entity) -> al::Vec3f {
            return detail::EntGetPosition(*static_cast<const TEntity*>(entity));
        };
        result.getLocalPosition = [](void* entity) -> al::Vec3f {
            return detail::EntGetLocalPosition(*static_cast<const TEntity*>(entity));
        };

        result.getScale = [](void* entity) -> al::Vec3f {
            return detail::EntGetScale(*static_cast<const TEntity*>(entity));
        };
        result.getLocalScale = [](void* entity) -> al::Vec3f {
            return detail::EntGetLocalScale(*static_cast<const TEntity*>(entity));
        };

        result.getRotation = [](void* entity) -> al::Vec3f {
            return detail::EntGetRotation(*static_cast<const TEntity*>(entity));
        };
        result.getLocalRotation = [](void* entity) -> al::Vec3f {
            return detail::EntGetLocalRotation(*static_cast<const TEntity*>(entity));
        };

        result.getId = [](void* entity) {
            TEntity* entityPtr = static_cast<TEntity*>(entity);
            return LPG_DATA_MEMBER_OR_DEFAULT(*entityPtr, id, -1);
        };

        result.accumulateTransform = [](void* entity, al::Transform& transform) {
            TEntity* entityPtr = static_cast<TEntity*>(entity);
            detail::EntApplyTransform(*entityPtr, transform);
        };

        result.accumulateLocalTransform = [](void* entity, al::Transform& transform) {
            TEntity* entityPtr = static_cast<TEntity*>(entity);
            detail::EntApplyLocalTransform(*entityPtr, transform);
        };


        std::apply([&](auto&&... tpl) {
            ([&]<typename T>(T&&) {
                using ArgT = std::remove_cvref_t<T>;
                if constexpr(requires{typename ArgT::LPGHandlesMessageTag;}) {
                    using MessageType = typename ArgT::Type;
                    if (registry.getMessageTypeId<MessageType>() == -1) {
                        registry.registerMessageType<MessageType>();
                    }
                    int messageTypeId = registry.getMessageTypeId<MessageType>();
                    if (result.sendMessage[messageTypeId] != nullptr) {
                        throw std::runtime_error(
                            "LPG_MESSAGE_HANDLER must appear exactly once for each message type, however, a duplicate was detected for "
                            + std::string(reflect::type_name<MessageType>())
                            + " when generating code for the entity "
                            + std::string(reflect::type_name<TEntity>())
                        );
                    }
                    if (result.sendMessage.size() <= messageTypeId) {
                        result.sendMessage.resize(messageTypeId + 1, nullptr);
                        result.sendMessageToMany.resize(messageTypeId + 1, nullptr);
                        result.sendMessageToManyContiguous.resize(messageTypeId + 1, nullptr);
                    }
                    result.sendMessage[messageTypeId] = [](void* vpMsg, void* vpEnt) {
                        TEntity* entity = static_cast<TEntity*>(vpEnt);
                        MessageType* message = static_cast<MessageType*>(vpMsg);
                        entity->msg(message);
                    };
                    result.sendMessageToMany[messageTypeId] = [](void* vpMsg, TypeErasedStridedSpan tssEntities) {
                        MessageType* message = static_cast<MessageType*>(vpMsg);
                        auto entities = tssEntities.interpretAs<TEntity>();

                        for (auto& entity : entities) {
                            entity->msg(message);
                        }
                    };
                    result.sendMessageToManyContiguous[messageTypeId] = [](void* vpMsg, void* vpArrEnt, size_t arrSize) {
                        MessageType* message = static_cast<MessageType*>(vpMsg);
                        TEntity* firstEntity = static_cast<TEntity*>(vpArrEnt);
                        auto entities = std::span {firstEntity, firstEntity + arrSize};

                        for (auto& entity : entities) {
                            entity->msg(message);
                        }
                    };

                }
            }(tpl), ...);
        }, refl::all_type_tags<TEntity>());

        return result;
    }

}

#endif //LPG_ENGINE_SRC_LPG_CORE_ENTITY_CODEGEN_HPP_
