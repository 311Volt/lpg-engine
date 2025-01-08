//
// Created by volt on 2025-01-07.
//




#ifndef LPG_ENGINE_SRC_LPG_CORE_ENTITY_CODEGEN_HPP_
#define LPG_ENGINE_SRC_LPG_CORE_ENTITY_CODEGEN_HPP_

#include <reflect>
#include <ranges>
#include <vector>
#include <typeindex>
#include "data.hpp"

namespace lpg {


    template<typename TEntity>
    struct EntityActions {

        //generate components metadata
        //set/get pos/rot/scale
        //set/get transform
        //property name perfect hash function
        //set/get property
        //get id and world
        //get parent, get children
        //send messages

        static inline std::vector<std::type_index> GetComponentTypeIndices() {
            std::vector<std::type_index> result;
            refl::for_each_decl<TEntity>([&](auto I) {
                using FieldType = refl::member_type<I, TEntity>;

                if constexpr(refl::has_member_attr<IsComponent, I, TEntity>() || requires{typename FieldType::IsEntity;}) {
                    result.push_back(std::type_index(typeid(TEntity)));

                    auto subResult = EntityActions<FieldType>::GetComponentTypeIndices();
                    result.insert(result.end(), subResult.begin(), subResult.end());
                }

            });
            return result;
        }

        static inline std::vector<void*> GetComponentPointers(TEntity* entity) {
            std::vector<void*> result;
            refl::for_each_decl<TEntity>([&](auto I) {
                using FieldType = refl::member_type<I, TEntity>;

                if constexpr(refl::has_member_attr<IsComponent, I, TEntity>() || requires{typename FieldType::IsEntity;}) {
                    result.push_back(static_cast<void*>(&refl::get<I, TEntity>(*entity)));

                    auto subResult = EntityActions<FieldType>::GetComponentPointers();
                    result.insert(result.end(), subResult.begin(), subResult.end());
                }

            });
            return result;
        }






    };

    template<typename TEntity>
    struct EntityGroupActions {

    };

}

#endif //LPG_ENGINE_SRC_LPG_CORE_ENTITY_CODEGEN_HPP_
