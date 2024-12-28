//
// Created by volt on 12/22/2024.
//

#ifndef LPG_ENGINE_REGISTRY_HPP
#define LPG_ENGINE_REGISTRY_HPP

#include <concepts>
#include <iostream>

#include <map>
#include <string>
#include "../core/data.hpp"

namespace lpg {

    class Registry {
    public:

        template<std::convertible_to<DataUnit> T>
            requires (not std::is_aggregate_v<T>)
        void registerValue(const std::string& key, T& value) {
            storage_[key] = PtrToDataUnit(&value);
        }

        template<typename T>
            requires std::is_aggregate_v<T>
        void registerValue(const std::string& key, T& value) {
            refl::for_each_decl<T>([&](auto I) {
                std::string subkey = std::format("{}.{}", key, refl::member_name<I, T>());
                auto& subvalue = refl::get<I, T&>(value);
                registerValue(subkey, subvalue);
            });
        }

        void dump(std::ostream& os);

    private:
        std::map<std::string, PtrToDataUnit> storage_;
    };


}





#endif //LPG_ENGINE_REGISTRY_HPP
