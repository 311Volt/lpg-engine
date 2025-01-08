//
// Created by volt on 12/27/2024.
//




#ifndef LPG_ENGINE_SRC_LPG_ENTITIES_DIRECTIONALLIGHT_HPP_
#define LPG_ENGINE_SRC_LPG_ENTITIES_DIRECTIONALLIGHT_HPP_

#include <axxegro/com/math/math.hpp>

namespace lpg {
    struct DirectionalLightEntity: BaseEntity {
        al::Vec3f direction;
        al::Vec3f color;
        double intensity;
    };
}


#endif //LPG_ENGINE_SRC_LPG_ENTITIES_DIRECTIONALLIGHT_HPP_
