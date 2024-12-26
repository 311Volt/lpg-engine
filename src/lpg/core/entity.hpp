//
// Created by volt on 12/22/2024.
//

#ifndef LPG_ENGINE_ENTITY_HPP
#define LPG_ENGINE_ENTITY_HPP

#include <memory>
#include <axxegro/com/math/math.hpp>

namespace lpg {

    struct EntityReference {
        int32_t id;
        int32_t version;
    };


    class EntityInfo {
    public:
        std::string name;
    };

    class BaseEntityHandle {
    public:
        virtual ~BaseEntityHandle() = default;

        virtual void update(double deltaTime) = 0;
        virtual void render() = 0;
        //getProperties()
    };

    template<typename TEntity>
    class EntityHandle : public BaseEntityHandle {
    public:

        template<typename... Args>
        explicit EntityHandle(Args&&... args) {
            entity = std::make_unique<TEntity>(args...);
        }

        void update(double deltaTime) override {
            entity->update(deltaTime);
        }

        void render() override {
            entity->render();
        }
    private:
        std::unique_ptr<TEntity> entity;
    };
}


#endif //LPG_ENGINE_ENTITY_HPP
