//
// Created by volt on 2025-01-14.
//




#ifndef LPG_ENGINE_SRC_LPG_UTIL_STRIDED_SPAN_HPP_
#define LPG_ENGINE_SRC_LPG_UTIL_STRIDED_SPAN_HPP_

#include <iterator>
#include <cstddef>

namespace lpg {

    template<typename T>
    class StridedSpan;

    class TypeErasedStridedSpan {
    public:
        TypeErasedStridedSpan(std::byte *data, size_t numElements, size_t stride)
            : data_(data),
              numElements_(numElements),
              stride_(stride) {
        }

        auto* nth_element(this auto&& self, size_t n) {
            return self.data_ + n * self.stride_;
        }

        size_t numElements() const {
            return numElements_;
        }

        template<typename T>
        auto interpretAs() const;

    private:
        std::byte* data_;
        size_t numElements_, stride_;
    };



    template<typename T>
    class StridedSpan {
    public:

        StridedSpan(std::byte *data, size_t numElements, size_t stride)
            : underlying_(data, numElements, stride)
        {}

        explicit StridedSpan(const TypeErasedStridedSpan& underlying) : underlying_(underlying) {

        }

        auto&& operator[](this auto&& self, size_t index) {
            return *reinterpret_cast<T*>(self.underlying_.nth_element(index));
        }

        struct iterator {
            StridedSpan *const sspan_;
            size_t index_;

            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = std::remove_reference_t<T>;
            using pointer = value_type*;
            using reference = value_type&;


            auto&& operator*() {
                return *reinterpret_cast<value_type*>(sspan_->underlying_.nth_element(index_));
            }
            iterator& operator++() {++index_; return *this;}
            iterator& operator++(int) {++index_; return *this;}
            iterator& operator+=(int n) {index_ += n; return *this;}
            iterator& operator-=(int n) {index_ -= n; return *this;}
            iterator operator+(int n) {return iterator{sspan_, index_ + n};}
            iterator operator-(int n) {return iterator{sspan_, index_ - n};}
            friend auto operator<=>(const iterator &, const iterator &) = default;
        };

        iterator begin() {
            return {.sspan_ = this, .index_ = 0};
        }
        iterator end() {
            return {.sspan_ = this, .index_ = size()};
        }
        size_t size() {
            return underlying_.numElements();
        }

    private:
        TypeErasedStridedSpan underlying_;
    };

    template<typename T>
    auto TypeErasedStridedSpan::interpretAs() const {
        return StridedSpan<T>{*this};
    }



}

#endif //LPG_ENGINE_SRC_LPG_UTIL_STRIDED_SPAN_HPP_
