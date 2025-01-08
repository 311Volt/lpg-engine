//
// Created by volt on 12/22/2024.
//

#ifndef LPG_ENGINE_ENTITY_HPP
#define LPG_ENGINE_ENTITY_HPP

#include <memory>
#include <array>
#include <axxegro/com/math/math.hpp>

#include "data.hpp"

namespace lpg {



    template<typename T>
    concept Entity = std::is_aggregate_v<T>;


    struct EntityVT {

    };

    struct EntityGroupVT {

    };

    template<Entity TEntity>
    struct EntityQueryResult {
        std::vector<std::span<TEntity>> results;
    };

    struct AnyEntityRef {
        int32_t id;
        int32_t version;
    };

    inline constexpr AnyEntityRef NullAnyEntityRef = {.id = 0, .version = 0};

    template<Entity TEntity>
    struct Ref {
        using IsEntityReference = void;
        using EntityType = TEntity;

        int32_t id = 0;
        int32_t version = 0;
    };

    template<Entity TEntity>
    struct Managed {
        using IsManagedComponent = void;
        using EntityType = TEntity;

        int32_t id = 0;
        int32_t version = 0;
    };

    struct BaseGameObject {
        int32_t id;


        LPG_ATTR(DoNotSerialize{})
        void* ptrWorld;

        std::array<float, 16> transform;
        std::array<float, 16> localTransform;
        std::string name;
    };


}


#endif //LPG_ENGINE_ENTITY_HPP
