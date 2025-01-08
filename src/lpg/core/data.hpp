//
// Created by volt on 12/22/2024.
//

#ifndef LPG_ENGINE_DATA_HPP
#define LPG_ENGINE_DATA_HPP

#include <reflect>
#include <string>
#include <variant>
#include <tuple>


#include <axxegro/com/math/math.hpp>


#ifdef _MSC_VER
#define LPG_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define LPG_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#define LPG_EXPAND(x) x

#define LPG_CONCAT_HELPER(x, y) x ## y
#define LPG_CONCAT(x, y) LPG_CONCAT_HELPER(x, y)


namespace lpg {
    enum class AssetType {
        Texture,
        Model,
        Material,
        Shader,
        SFX,
        Music,
        Script,
        Font,
        Blob
    };

    template<AssetType TPAssetType>
    struct AssetPath {
        static constexpr AssetType Type = TPAssetType;
        std::string path;
    };

    namespace detail {
        template<typename... Args>
        struct DataUnitHelper {
            using DataUnitType = std::variant<Args...>;
            using PtrToDataUnitType = std::variant<Args*...>;
        };

        using DataUnitTypes = DataUnitHelper<
            bool,
            int8_t,
            int16_t,
            int32_t,
            int64_t,
            float,
            double,
            al::Vec2i,
            al::Vec3i,
            al::Vec4i,
            al::Vec2f,
            al::Vec3f,
            al::Vec4f,
            std::string
        >;
    }

    using DataUnit = typename detail::DataUnitTypes::DataUnitType;
    using PtrToDataUnit = typename detail::DataUnitTypes::PtrToDataUnitType;

    [[nodiscard]] std::string DataUnitToStr(const DataUnit& dataUnit);
    std::string DataUnitToStr(PtrToDataUnit dataUnit);
     // DataUnit StrToDataUnit(const std::string_view data);



    inline namespace attr {

        struct IsComponent {};
        struct DoNotSerialize {};

        struct MinValue {double val;};
        struct MaxValue {double val;};
        struct HelpText {std::string text;};
        struct DefaultValue {DataUnit value;};
        struct RValFlags {
            bool isCheat = false;
            bool replicate = false;
            bool archive = true;
        };
    }



    namespace refl {

        namespace base_refl = reflect::v1_2_4;

        /*
         * So basically, since we can reflect on data members of aggregates, we also want to be able to annotate them
         * with some sort of compile-time metadata:
         *
         * struct Test {
         *     LPG_ATTR(MinValue{0}, MaxValue{100})
         *     int a;
         * };
         *
         * LPG Engine does this by actually inserting an additional data member inside of the expansion of LPG_ATTR.
         * It is an empty struct with [[no_unique_address]] and does not take any additional space.
         * The attribute is always applied to the data member that immediately follows the attribute.
         *
         * Since we have more data members than it appears, we need an additional layer on top of qlibs/reflect
         * to map apparent member indices to real member indices, which is what the code below does.
         *
         * This approach also **breaks structured bindings** for obvious reasons.
         * We could potentially provide our own std::get that is constrained to types that have attributes,
         * which would solve the problem -- TODO try it once this incompatibility becomes an issue
         */

        template<auto... TPValues>
        struct AttributeList {
            using LPGEngineAttributeTag = void;
            static constexpr auto Values = std::tuple{TPValues...};
        };
#define LPG_ATTR(...) LPG_EXPAND(LPG_NO_UNIQUE_ADDRESS) ::lpg::refl::AttributeList<__VA_ARGS__> LPG_CONCAT(lpg____attr_, __COUNTER__) = {};


        namespace detail {

            template<int N, typename T>
            inline constexpr bool is_member_attr_list() {
                return requires{typename base_refl::member_type<N, T>::LPGEngineAttributeTag;};
            }

            template<int N, typename T>
            using real_member_type = std::remove_cvref_t<decltype(reflect::get<N, T>(std::declval<T>()))>;

            template<int N, typename T>
            inline constexpr bool num_attributes_until() {
                if constexpr (N < 0) {
                    return 0;
                } else {
                    return is_member_attr_list<N, T>() + num_attributes_until<N-1, T>();
                }
            }

            template<typename T>
            inline constexpr auto num_non_attr_members() {
                return reflect::size<T>() - num_attributes_until<reflect::size<T>() - 1, T>();
            }

            template<int N, typename T>
            inline constexpr int idx_real_to_reduced() {
                return N - num_attributes_until<N, T>();
            }

            //TODO binary search here
            template<int N, typename T, int X>
                requires (X >= 0)
            inline constexpr int idx_reduced_to_real_impl() {
                if constexpr (idx_real_to_reduced<X, T>() == N) {
                    return X;
                } else {
                    return idx_reduced_to_real_impl<N, T, X-1>();
                }
            }

            template<int N, typename T>
                requires (N < num_non_attr_members<T>())
            inline constexpr int idx_reduced_to_real() {
                return idx_reduced_to_real_impl<N, T, reflect::size<T>()-1>();
            }

        } //namespace detail

        template<typename T>
        inline constexpr auto num_data_members() {
            return detail::num_non_attr_members<T>();
        }

        template<typename T>
        inline constexpr auto type_name() {
            return base_refl::type_name<T>();
        }

        template<typename T>
        inline constexpr auto enum_name() {
            return base_refl::enum_name<T>();
        }

        template<typename T>
        inline constexpr auto enum_name(T&&) {
            return enum_name<T>();
        }

        template<int N, typename T>
        inline constexpr auto member_name() {
            return base_refl::member_name<detail::idx_reduced_to_real<N, T>(), T>();
        }

        template<int N, typename T>
        inline constexpr auto member_name(T&&) {
            return member_name<N, T>();
        }

        template<int N, typename T>
        inline constexpr decltype(auto) get(T&& value) noexcept {
            return base_refl::get<detail::idx_reduced_to_real<N, T>()>(value);
        }

        template<int N, typename T>
        using member_type = std::remove_cvref_t<decltype(get<N>(std::declval<T>()))>;

        template<int N, typename T>
        inline constexpr bool has_attributes() {
            static constexpr int real_idx = detail::idx_reduced_to_real<N, T>();
            if constexpr (real_idx == 0) {
                return false;
            } else {
                return detail::is_member_attr_list<real_idx - 1, T>();
            }
        }

        template<int N, typename T>
            requires (has_attributes<N, T>())
        inline constexpr auto member_attr_list() {
            static constexpr int real_idx = detail::idx_reduced_to_real<N, T>();
            return detail::real_member_type<real_idx-1, T>::Values;
        }

        template<typename A, int N, typename T>
        inline constexpr auto member_attr() {
            return std::get<A>(member_attr_list<N, T>());
        }

        template<typename A, int N, typename T>
        inline constexpr bool has_member_attr() {
            return requires {std::get<A>(member_attr_list<N, T>());};
        }


        template<typename T, typename Fn>
        inline constexpr void for_each_decl(Fn&& fn) {
            reflect::for_each<T>([&](auto I) {
                if constexpr (not detail::is_member_attr_list<I, T>()) {
                    std::integral_constant<int, detail::idx_real_to_reduced<I, T>()> I1;
                    fn(I1);
                }
            });
        }



    } //namespace refl


} // lpg

#endif //LPG_ENGINE_DATA_HPP
