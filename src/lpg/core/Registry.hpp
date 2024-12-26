//
// Created by volt on 12/22/2024.
//

#ifndef LPG_ENGINE_REGISTRY_HPP
#define LPG_ENGINE_REGISTRY_HPP

#include <map>
#include <string>
#include "../core/data.hpp"

namespace lpg {

    class Registry {
    public:

        template<typename T>
        void registerEntity() {
            //TODO
        }

        template<typename T>
        void registerSystem() {
            //TODO
        }

    private:
        std::map<std::string, PtrToDataUnit> storage_;
    };


}





#endif //LPG_ENGINE_REGISTRY_HPP
