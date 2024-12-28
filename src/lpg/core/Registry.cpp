//
// Created by volt on 12/22/2024.
//

#include "Registry.hpp"

void lpg::Registry::dump(std::ostream &os) {
    os << "Registry::dump(" << this << "):" << std::endl;
    for (const auto& [key, value]: this->storage_) {
        os << std::format("{} = {}\n", key, DataUnitToStr(value));
    }
}
