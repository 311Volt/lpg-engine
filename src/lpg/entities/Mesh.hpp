//
// Created by volt on 12/27/2024.
//




#ifndef LPG_ENGINE_SRC_LPG_ENTITIES_MESH_HPP_
#define LPG_ENGINE_SRC_LPG_ENTITIES_MESH_HPP_
#include <lpg/core/data.hpp>

namespace lpg {

    struct MeshEntity: BaseEntity {
        const AssetPath<AssetType::Model> path;
    };

}

#endif //LPG_ENGINE_SRC_LPG_ENTITIES_MESH_HPP_
