//
// Created by volt on 12/21/2024.
//

#ifndef LPG_ENGINE_WORLD_HPP
#define LPG_ENGINE_WORLD_HPP

#include <stdint.h>
#include <string>
#include <any>
#include <string_view>
#include <reflect>
#include "SysCounter.hpp"
#include "entity.hpp"

namespace lpg {

    namespace detail {
        struct ReserveEntityResult {
            void* entity;
            int32_t descriptor;
        };
    }


    class World {
    public:

        template<typename T>
        void registerEntityType(const EntityInterface& entityInterface) {

            //make sure bases are registered
            //save entity interface

        }

        template<typename T>
        void registerSystem(T&& system) {
            //TODO
        }

        template<typename TEntity>
        Ref<TEntity> spawnEntity(auto&&... args) {
            int32_t entTypeId = getEntityTypeId(reflect::type_name<TEntity>());
            auto result = reserveEntity(entTypeId);
            TEntity* ent = static_cast<TEntity*>(result.entity);
            std::construct_at(ent, std::forward<decltype(args)>(args)...);

            //TODO create pages for managed components
            //TODO recursively spawn managed components

            return Ref<TEntity>{
                .id = result.id,
                .version = getCurVersionNumOf(result.id)
            };
        }

        template<typename TEntity>
        Ref<TEntity> despawnEntity() {

            //recursively destroy managed components
            //destroy entity

        }

        // derefEntities(EntityRange)

        template<typename TEntity>
        std::vector<TEntity*> query(auto&&... predicates) {

        }

        template<typename TEntity>
        auto&& ent(this auto&& self, Ref<TEntity> entity) {

        }



    private:

        detail::ReserveEntityResult reserveEntity(int32_t entTypeId);
        int32_t getEntityTypeId(std::string_view name);
        int32_t getCurVersionNumOf(int32_t entId);



        std::any impl_;
    };

    namespace global {
        inline World TheWorld;
    }

} // lpg

#endif //LPG_ENGINE_WORLD_HPP
