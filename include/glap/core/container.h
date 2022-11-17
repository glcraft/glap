#pragma once
#include "utils.h"
#include <cstddef>
namespace glap
{
    template<typename T, size_t N>
    class StackVector {
        T m_data[N];
        size_t m_size = 0;
        
        template <class Ty>
        class _iterator {
            StackVector<Ty, N>& array;
            size_t index;
        public:
            constexpr _iterator(StackVector<T, N>& array, size_t index = 0) noexcept : array(array), index(index) 
            {}
            constexpr _iterator& operator++() noexcept {
                ++index;
                return *this;
            }
            constexpr _iterator operator++(int) noexcept {
                _iterator result(*this);
                ++index;
                return result;
            }
            constexpr bool operator==(const _iterator& other) const noexcept {
                return &array == &other.array && index == other.index;
            }
        };
    public:
        using iterator = _iterator<T>;
        using const_iterator = _iterator<const T>;
        using value_type = T;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

        constexpr ~StackVector() requires std::destructible<T> {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (size_t i = 0; i < m_size; ++i) {
                    m_data[i].~T();
                }
            }
            m_size = 0;
        }
        constexpr void push_back(T value) noexcept {
            m_data[m_size++] = value;
        }
        template <class ...Args>
        [[nodiscard]] constexpr T& emplace_back(Args... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
            auto& current_data = m_data[m_size++];
            new (current_data) T(args...);
            return current_data;
        }
        constexpr bool remove(size_t index) noexcept((!std::destructible<T> || std::is_nothrow_destructible_v<T>) && (!std::swappable<T> || std::is_nothrow_swappable_v<T>)) {
            if (index >= m_size) {
                return false;
            }
            if constexpr (std::destructible<T>)
                m_data[index].~T();
            if constexpr(std::swappable<T>) {
                using std::swap;
                swap(m_data[m_size - 1], m_data[index]);
            } else {
                std::copy_n(reinterpret_cast<std::byte*>(&m_data[m_size - 1]), sizeof(T), reinterpret_cast<std::byte*>(&m_data[index]));
            }
            m_size--;
            return true;
        }
        constexpr T& operator[](size_t index) {
            return m_data[index];
        }

        constexpr const T& operator[](size_t index) const {
            return m_data[index];
        }
        [[nodiscard]] constexpr auto size() const noexcept {
            return m_size;
        }
        [[nodiscard]] constexpr auto capacity() const noexcept {
            return N;
        }
        [[nodiscard]] constexpr auto& first() noexcept {
            return m_data[0];
        }
        [[nodiscard]] constexpr const auto& first() const noexcept {
            return m_data[0];
        }
        [[nodiscard]] constexpr auto& last() noexcept {
            return m_data[m_size-1];
        }
        [[nodiscard]] constexpr const auto& last() const noexcept {
            return m_data[m_size-1];
        }

        [[nodiscard]] constexpr auto begin() noexcept -> iterator {
            return iterator(*this);
        }
        [[nodiscard]] constexpr auto end() noexcept -> iterator {
            return iterator(*this, m_size);
        }
        [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator {
            return const_iterator(*this);
        }
        [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
            return const_iterator(*this, m_size);
        }
    };
    template <class T, auto N = discard>
    class Container {
    public: 
        using value_type = T;
    private:
        using n_type = std::remove_cvref_t<decltype(N)>;
        static constexpr auto is_n_discard = std::is_same_v<n_type, Discard>;
        static constexpr auto is_n_zero = impl::is_equal_v<N, 0>;
        using dynamic_vector = std::vector<value_type>;
        using stack_vector = StackVector<value_type, impl::value_or_v<N, 0>>;
    public:
        using container_type = std::conditional_t<is_n_discard || is_n_zero, dynamic_vector, stack_vector>;
        container_type values;

        [[nodiscard]]constexpr auto size() const noexcept {
            return values.size();
        }

        [[nodiscard]]constexpr const auto& get(size_t i) const noexcept(noexcept(this->values[i])) {
            if constexpr(std::same_as<container_type, stack_vector>) {
                assert(i < impl::ValueOr<N, 0>::value, "Index out of bounds");
            }
            return this->values[i].value;
        }
        [[nodiscard]]constexpr auto& get(size_t i) noexcept(noexcept(this->values[i])) {
            if constexpr(std::same_as<container_type, stack_vector>) {
                assert(i < impl::ValueOr<N, 0>::value, "Index out of bounds");
            }
            return this->values[i].value;
        }
        template <size_t I>
        [[nodiscard]]constexpr auto& get() noexcept(noexcept(this->values[I])) {
            static_assert((std::same_as<container_type, stack_vector> && I < impl::ValueOr<N, 0>::value), "Index out of bounds");
            return this->values[I].value;
        }
        template <size_t I>
        [[nodiscard]]constexpr const auto& get() const noexcept(noexcept(this->values[I])) {
            static_assert((std::same_as<container_type, stack_vector> && I < impl::ValueOr<N, 0>::value), "Index out of bounds");
            return this->values[I].value;
        }
        [[nodiscard]]constexpr auto& operator[](size_t i) noexcept(noexcept(get(i))) {
            return get(i);
        }
        [[nodiscard]]constexpr const auto& operator[](size_t i) const noexcept(noexcept(get(i))) {
            return get(i);
        }
    };
}