//
// Created by volt on 12/23/2024.
//

#ifndef LPG_ENGINE_ENT_CHARACTER_HPP
#define LPG_ENGINE_ENT_CHARACTER_HPP

#include "../core/entity.hpp"

namespace lpg {

    struct Character: BaseEntity {
        al::Vec3f position;
        al::Vec3f scale;
        al::Vec3f eyesOffset;
    };

}



#endif //LPG_ENGINE_ENT_CHARACTER_HPP
