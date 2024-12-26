//
// Created by volt on 12/23/2024.
//

#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "../core/entity.hpp"

namespace lpg {

    struct Character {
        al::Vec3f position;
        al::Vec3f cam_offset;
    };

}



#endif //CHARACTER_HPP
