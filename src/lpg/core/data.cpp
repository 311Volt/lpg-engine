//
// Created by volt on 12/22/2024.
//

#include "data.hpp"



namespace lpg {

    namespace detail {

    }

    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };

    std::string DataUnitToStr(const DataUnit &dataUnit) {

        return std::visit(overloaded {
            [](std::integral auto i) -> std::string {
                return std::format("{}", i);
            },
            [](std::floating_point auto i) -> std::string {
                return std::format("{}", i);
            },
            [](bool b) -> std::string {
                return std::string(b ? "true" : "false");
            },
            [](al::VectorType auto vec) -> std::string {
                using VecType = std::remove_cvref_t<decltype(vec)>;
                int size = VecType::NumElements;
                std::string result;
                for (int i=0; i<size; i++) {
                    result += std::format("{}{}", vec[i], (i<size-1) ? "," : "");
                }
                return result;
            },
            [](const std::string& str) -> std::string {
                return str;
            }
        }, dataUnit);
    }

    std::string DataUnitToStr(PtrToDataUnit dataUnit) {
        return std::visit(overloaded {
            [](const auto* p) {
                return DataUnitToStr(DataUnit{*p});
            }
        }, dataUnit);
    }

} // lpg