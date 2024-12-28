//
// Created by volt on 12/21/2024.
//

#ifndef LPG_ENGINE_WORLD_HPP
#define LPG_ENGINE_WORLD_HPP

#include "Registry.hpp"
#include "entity.hpp"

#include <list>

namespace lpg {

    class World {
    public:

        template<typename T>
        void registerEntity() {
            //TODO
        }

        template<typename T>
        void registerSystem() {
            //TODO
        }

        template<typename TEntity>
        void spawnEntity(const TEntity& entity) {

        }

    private:



        Registry registry;
        std::list<std::unique_ptr<BaseEntityHandle>> entities;
    };

} // lpg

#endif //LPG_ENGINE_WORLD_HPP
