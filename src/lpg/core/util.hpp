//
// Created by volt on 2025-01-29.
//




#ifndef LPG_ENGINE_SRC_LPG_CORE_UTIL_HPP_
#define LPG_ENGINE_SRC_LPG_CORE_UTIL_HPP_

#include <type_traits>

namespace lpg {

    namespace vec {

        template<typename TVecRef>
        auto* TryGet(TVecRef&& vec, size_t index) -> decltype(&vec[0]) {
            if (index >= vec.size()) {
                return nullptr;
            }
            return &vec[index];
        }

        template<typename TVecRef>
        void ResizeFor(TVecRef&& vec, size_t index) {
            if (vec.size() <= index) {
                vec.resize(index + 1);
            }
        }

        template<typename TVecRef>
        void InsertAt(TVecRef&& vec, size_t index, typename std::remove_cvref_t<TVecRef>::value_type const& value) {
            ResizeFor(vec, index);
            vec[index] = value;
        }

    }

}

#endif //LPG_ENGINE_SRC_LPG_CORE_UTIL_HPP_
