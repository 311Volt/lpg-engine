//
// Created by volt on 12/27/2024.
//




#ifndef LPG_ENGINE_SRC_LPG_SYSTEMS_BASERENDER_HPP_
#define LPG_ENGINE_SRC_LPG_SYSTEMS_BASERENDER_HPP_

#include <string>

namespace lpg {

    class BaseRender {
    public:

        struct {
            std::string apiType;
            int apiLevel;
            bool enableInstancing;
        } config;

        virtual void exec(double deltaTime) = 0;

    };

} // lpg

#endif //LPG_ENGINE_SRC_LPG_SYSTEMS_BASERENDER_HPP_
