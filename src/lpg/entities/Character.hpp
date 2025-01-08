//
// Created by volt on 12/23/2024.
//

#ifndef LPG_ENGINE_ENT_CHARACTER_HPP
#define LPG_ENGINE_ENT_CHARACTER_HPP

#include "../core/entity.hpp"

namespace lpg {

    struct Character: BaseEntity {
        al::Vec3f pos;
        al::Vec3f size;
        al::Vec3f cam_offset;
    };

}



#endif //LPG_ENGINE_ENT_CHARACTER_HPP
