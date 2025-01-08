//
// Created by volt on 12/21/2024.
//

#ifndef LPG_ENGINE_WORLD_HPP
#define LPG_ENGINE_WORLD_HPP

#include <stdint.h>
#include <string>
#include <any>
#include <reflect>
#include "SysCounter.hpp"

namespace lpg {


    struct EntityTypeDecl {
        int32_t id;
        std::string name;
        const std::type_info& type;
    };

    class World;

    struct SystemDecl {
        int32_t id;
        std::string name;
        std::string regNamespace;
        SysFreq trigger;
        void (callback*)(World*);
    };






    class World {
    public:

        template<typename T>
        void registerEntityType() {

        }

        template<typename T>
        void registerSystem(T&& system) {
            //TODO
        }

        template<typename TEntity>
        Ref<TEntity> spawnEntity(const TEntity& entity) {
            refl::for_each_decl<TEntity>([](auto I) {

            });
        }

        template<typename TEntity>
        std::vector<TEntity*> queryEntity(auto&&... predicates) {
            int entType = entityTypeNames_.at(refl::type_name<TEntity>());
            std::vector<TEntity*> entities;

        }

        template<typename TEntity>
        auto&& getEntity(this auto&& self, Ref<TEntity> entity) {

        }



    private:

        std::any impl_;
    };

    namespace global {
        inline World TheWorld;
    }

} // lpg

#endif //LPG_ENGINE_WORLD_HPP
