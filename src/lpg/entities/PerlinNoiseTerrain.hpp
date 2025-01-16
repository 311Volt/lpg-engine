//
// Created by volt on 12/21/2024.
//

#ifndef LPG_ENGINE_TERRAIN_HPP
#define LPG_ENGINE_TERRAIN_HPP

#include <string>

namespace lpg {

    struct PerlinNoiseTerrain {

        al::Vec3f position{}, scale{};
        uint32_t seed {};

        int numOctaves = 8;


    };

} // lpg

#endif //LPG_ENGINE_TERRAIN_HPP
