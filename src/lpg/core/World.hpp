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

        World();
        ~World();

        template<typename TEntity>
        void registerEntityType(const EntityInterface& entityInterface) {

            //TODO make sure bases are registered (later)

            int entityTypeId = detail::GetEntityTypeId<TEntity>();
            saveEntityInterface(entityInterface, entityTypeId);

        }

        template<typename TSystem>
        void registerSystem(const std::string& name, TSystem&& system) {
            //TODO
        }

        template<typename TMessage>
        void registerMessageType() {
            int msgTypeID = detail::GetMessageTypeId<TMessage>();
            std::string name = reflect::type_name<TMessage>();

            registerMessageTypeImpl(name, msgTypeID);
        }

        template<typename TEntity>
        EntityDescriptor spawnEntity(auto&&... args) {
            int32_t entTypeId = detail::GetEntityTypeId<TEntity>();
            auto result = reserveEntity(entTypeId);
            TEntity* ent = static_cast<TEntity*>(result.entity);
            std::construct_at(ent, std::forward<decltype(args)>(args)...);


            //TODO create pages for managed components
            //TODO recursively spawn managed components
            //(later)

            return result.descriptor;
        }

        bool World::despawnEntity(EntityDescriptor entityDescriptor);

        /* TODO performance:
         * replace result type with EntityQueryResult */
        template<typename TEntity>
        std::vector<TEntity*> query(auto&&... predicates) {
            int32_t entTypeId = detail::GetEntityTypeId<TEntity>();
            std::vector<TEntity*> result;
            getAll(entTypeId, &result, [](void* userdata, void* vpEntBeg, void* vpEntEnd) {
                auto* vResult = static_cast<std::vector<TEntity*>*>(userdata);
                auto* entBeg = static_cast<TEntity*>(vpEntBeg);
                auto* entEnd = static_cast<TEntity*>(vpEntEnd);
                for (TEntity* ent = entBeg; ent != entEnd; ++ent) {
                    vResult.push_back(ent);
                }
            });
            return result;
        }

        template<typename TEntity, typename TMessage>
        void sendMessageToAll() {

        }

        template<typename TEntity>
        auto&& at(this auto&& self, Ref<TEntity> entity) {

        }

        void finalizeInit();



    private:

        void forEachEntityImpl(int entityTypeId, void* userdata, void (*onEntity)(void* userdata, void* entBegin, void* entEnd));

        void registerMessageTypeImpl(const std::string& name, int messageTypeId);

        EntityVersionNumber getCurVersionNumOf(EntityID entId);
        void saveEntityInterface(const EntityInterface& entityInterface, int32_t entTypeId);
        int registerEntityTypeImpl(const std::string& name, const EntityInterface& entityInterface);
        void relocateEntity(int32_t targetDescriptor, int32_t sourceDescriptor);
        void swapEntities(int32_t targetDescriptor, int32_t sourceDescriptor);
        int32_t createNewPage(int entityTypeId);
        int32_t getFreePage(int entityTypeId);
        detail::ReserveEntityResult reserveEntity(int32_t entityTypeId);


        std::any worldData_;
    };

    namespace global {
        inline World TheWorld;
    }

} // lpg

#endif //LPG_ENGINE_WORLD_HPP
