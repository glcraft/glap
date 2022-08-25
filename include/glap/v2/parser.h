#pragma once

#include "../common/expected.h"
#include "../common/error.h"
#include "../common/utils.h"
#include "../common/utf8.h"
#include <utility>
#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <tuple>
#include <string_view>
#include <concepts>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>
namespace glap::v2 
{
    template<size_t N>
    struct StringLiteral {
        constexpr StringLiteral(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
        constexpr operator std::string_view() const {
            return std::string_view(value, N - 1);
        }
        
        char value[N];
    };
    

    struct Discard {};
    static constexpr Discard discard = {};

    template <auto Value, size_t Default>
    struct value_or {
        static constexpr auto value = Default;
    };
    template <auto Value, size_t Default>
        requires std::convertible_to<decltype(Value), size_t>
    struct value_or<Value, Default> {
        static constexpr size_t value = Value;
    };
    template <auto Value, size_t Default>
    constexpr auto value_or_v = value_or<Value, Default>::value;

    template <class Ty, auto I, size_t V>
    struct is_value
    {
        static constexpr bool value = false;
    };
    template <class Ty, size_t V>
    struct is_value<Ty, V, V>
    {
        static constexpr bool value = true;
    };
    template <class Ty, auto I, size_t V>
    constexpr bool is_value_v = is_value<Ty, I, V>::value;

    template<typename T, size_t N>
    class FixedVector {
        T m_data[N];
        size_t m_size = 0;
        
        template <class Ty>
        class _iterator {
            FixedVector<Ty, N>& array;
            size_t index;
        public:
            constexpr _iterator(FixedVector<T, N>& array, size_t index = 0) noexcept : array(array), index(index) 
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
        ~FixedVector() requires std::destructible<T> {
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
            return size;
        }
        [[nodiscard]] constexpr auto capacity() const noexcept {
            return N;
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
        static constexpr auto is_n_zero = is_value_v<n_type, N, 0>;
        using dynamic_vector = std::vector<value_type>;
        using fixed_vector = FixedVector<value_type, value_or_v<N, 0>>;
    public:
        using container_type = std::conditional_t<is_n_discard || is_n_zero, dynamic_vector, fixed_vector>;
        container_type values;

        constexpr auto size() const noexcept {
            return values.size();
        }

        [[nodiscard]]constexpr const auto& get(size_t i) const noexcept {
            return this->values[i];
        }
        [[nodiscard]]constexpr auto& get(size_t i) noexcept {
            return this->values[i];
        }
        template <size_t I>
        [[nodiscard]]constexpr auto& get() noexcept requires (I < value_or<N, std::numeric_limits<size_t>::max()>::value) {
            return this->values[I];
        }
        template <size_t I>
        [[nodiscard]]constexpr const auto& get() const noexcept requires (I < value_or<N, std::numeric_limits<size_t>::max()>::value) {
            return this->values[I];
        }
    };

    template <StringLiteral LongName, auto ShortName = discard>
    struct Names {
    private:
        template <auto SN>
        struct _short_name {
            static constexpr auto value = std::optional<char32_t>{SN};
        };
        template<> struct _short_name<discard> {
            static constexpr auto value = std::optional<char32_t>{};
        };
        template<> struct _short_name<0> {
            static constexpr auto value = std::optional<char32_t>{};
        };
    public:
        static constexpr std::string_view Longname = LongName;
        static constexpr std::optional<char32_t> Shortname = _short_name<ShortName>::value;

        constexpr auto longname() const noexcept {
            return Longname;
        }
        constexpr auto shortname() const noexcept {
            return Shortname;
        }
    };
    template <typename T>
    concept HasNames = requires {
        std::same_as<std::remove_cvref_t<decltype(T::Longname)>, std::string_view>;
        std::same_as<std::remove_cvref_t<decltype(T::Shortname)>, std::optional<char32_t>>;
    };
   
    template <class ...ArgN>
    struct NameChecker 
    {
        static constexpr bool has_duplicate_longname = false;
        static constexpr bool has_duplicate_shortname = false;
    };
    template <HasNames Arg1, HasNames Arg2, class ...ArgN>
    struct NameChecker<Arg1, Arg2, ArgN...> 
    {
        static constexpr bool has_duplicate_longname = Arg1::Longname == Arg2::Longname || NameChecker<Arg1, ArgN...>::has_duplicate_longname || NameChecker<Arg2, ArgN...>::has_duplicate_longname;
        static constexpr bool has_duplicate_shortname = Arg1::Shortname.has_value() && Arg2::Shortname.has_value() && Arg1::Shortname.value() == Arg2::Shortname.value() || NameChecker<Arg1, ArgN...>::has_duplicate_shortname || NameChecker<Arg2, ArgN...>::has_duplicate_shortname;
    };
    template <typename Arg1, typename Arg2, class ...ArgN>
        requires HasNames<Arg1> && (!HasNames<Arg2>)
    struct NameChecker<Arg1, Arg2, ArgN...>
    {
        static constexpr bool has_duplicate_longname = NameChecker<Arg1, ArgN...>::has_duplicate_longname;
        static constexpr bool has_duplicate_shortname = NameChecker<Arg1, ArgN...>::has_duplicate_shortname;
    };
    template <typename Arg1, class ...ArgN>
    struct NameChecker<Arg1, Arg1, ArgN...>
    {
        static_assert(!std::is_same_v<Arg1, Arg1>, "Duplicate parameter");
    };
    template <auto Resolver = discard, auto Validator = discard>
    struct Value {
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;

        std::string_view value;
        [[nodiscard]]constexpr auto resolve() const requires (!std::same_as<decltype(Resolver), Discard>) {
            static_assert(std::invocable<decltype(Resolver), std::string_view>, "Resolver must be callable with std::string_view");
            return Resolver(value);
        }
        [[nodiscard]]constexpr auto validate() const requires (!std::same_as<decltype(Validator), Discard>) {
            static_assert(std::invocable<decltype(Validator), std::string_view>, "Validator must be callable with std::string_view");
            return Validator(value);
        }
    };

    enum class ParameterType {
        Argument,
        Flag,
        Input
    };

    template <class ArgNames, auto Resolver = discard, auto Validator = discard>
    struct Argument : public ArgNames, public Value<Resolver, Validator> {
        static constexpr auto type = ParameterType::Argument;
    };
    
    template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Arguments : public ArgNames, public Container<Argument<ArgNames, Resolver, Validator>, N> {
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Argument;
    };
    template <class ArgNames>
    struct Flag : public ArgNames {
        size_t occurences = 0;
        static constexpr auto type = ParameterType::Flag;
    };
    template <auto Resolver = discard, auto Validator = discard>
    struct Input : public Value<Resolver, Validator> {
        static constexpr auto type = ParameterType::Input;
    };
    template <auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Inputs : public Container<Input<Resolver, Validator>, N> {
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Input;
    };

    template <class T>
    concept IsParameter = requires {
        std::same_as<std::remove_cvref_t<decltype(T::type)>, ParameterType>;
    };
    
    template <class CommandNames, IsParameter... P>
    class Command : public CommandNames {
        using NameCheck = NameChecker<P...>;
        static_assert(!NameCheck::has_duplicate_longname, "parameters has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "parameters has duplicate short name");

        using Params = std::tuple<P...>;
        static constexpr size_t NbParams = sizeof...(P);
        template <size_t I>
        using Param = std::tuple_element_t<I, Params>;
        
        template <size_t i, StringLiteral lit>
        static consteval size_t _get_parameter_id() noexcept {
            static_assert((i < NbParams), "Parameter not found");
            if constexpr (Param<i>::Longname == lit) {
                return i;
            } else {
                return _get_parameter_id<i + 1, lit>();
            }
        }
    public:
        Params params;
        template <StringLiteral lit>
        constexpr auto& get_parameter() noexcept requires (NbParams > 0) {
            return std::get<_get_parameter_id<0, lit>()>(params);
        }
        template <StringLiteral lit>
        constexpr const auto& get_parameter() const noexcept requires (NbParams > 0) {
            return std::get<_get_parameter_id<0, lit>()>(params);
        }
    };
    template<class... Commands>
    struct Program {
        std::string_view program;
        std::variant<Commands...> command;
    };

    template<class... Commands>
    class Parser {
        using NameCheck = NameChecker<Commands...>;
        static_assert(!NameCheck::has_duplicate_longname, "commands has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "commands has duplicate short name");
    public:
        template <utils::Iterator<std::string_view> T>
        struct BiIterator {
            T begin;
            T end;
        };
        template <class T>
        BiIterator(T,T) -> BiIterator<T>;
        template <class C>
        struct ParseItem {
            constexpr auto operator()(utils::Iterable<std::string_view> auto args) const -> PosExpected<C>;
        };
        constexpr auto parse(utils::Iterable<std::string_view> auto args) const -> PosExpected<Program<Commands...>> {
            if (args.size() == 0) 
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .argument = "",
                        .value = std::nullopt,
                        .type = Error::Type::None,
                        .code = Error::Code::NoArgument
                    },
                    .position = 0
                });
            auto itarg = args.begin();
            Program<Commands...> program;
            program.program = *itarg++;
            
            //TODO: implement global command

            if (itarg == args.end()) {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .argument = "",
                        .value = std::nullopt,
                        .type = Error::Type::None,
                        .code = Error::Code::NoGlobalCommand
                    },
                    .position = 0
                });
            }
            auto arg = std::string_view{*itarg};
            if (arg.starts_with("-")) {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .argument = arg,
                        .value = std::nullopt,
                        .type = Error::Type::None,
                        .code = Error::Code::NoGlobalCommand
                    },
                    .position = 1
                });
            }
            auto found_command = find_and_parse<decltype(program.command)>(args);
            if (!found_command) {
                return make_unexpected(found_command.error());
            }
            program.command = found_command.value();
            
            return program;
        }
    private:
        template <HasNames Ty>
        static constexpr auto find_longname(std::string_view name) -> bool {
            return name == Ty::Longname;
        }
        template <HasNames Ty>
        static constexpr auto find_shortname(std::string_view name) -> Expected<bool> {
            auto exp_codepoint = glap::utils::uni::codepoint(name);
            if (!exp_codepoint) {
                return glap::make_unexpected(Error {
                    .argument = name,
                    .value = std::nullopt,
                    .type = Error::Type::Command,
                    .code = Error::Code::BadString
                });
            }
            auto codepoint = exp_codepoint.value();
            return codepoint == Ty::Shortname;
        }
        template <HasNames Ty>
        static constexpr auto find_name(std::string_view name) -> Expected<bool> {
            if (auto value = find_longname<Ty>(name); value)
                return value;
            return find_shortname<Ty>(name);
        }
        template <class T>
        struct FindAndParse {
            constexpr auto operator()(utils::Iterable<std::string_view> auto args) const -> PosExpected<T>;
        };
        template <HasNames ...T>
        struct FindAndParse<std::variant<T...>> {
            constexpr auto operator()(utils::Iterable<std::string_view> auto args) const -> PosExpected<std::variant<T...>> {
                std::optional<PosExpected<std::variant<T...>>> result = std::nullopt;
                auto cmd_name = *args.begin();
                ([&] {
                    auto exp_found = find_name<T>(cmd_name);
                    if (!exp_found) 
                        result = make_unexpected(PositionnedError {
                            .error = std::move(exp_found.error()),
                            .position = 0
                        });
                    else if (*exp_found) 
                        result = parse_item<T>(args);
                    return !exp_found || *exp_found;
                }() || ...);

                if (result.has_value()) {
                    return make_unexpected(PositionnedError {
                        .error = Error {
                            .argument = cmd_name,
                            .value = std::nullopt,
                            .type = Error::Type::Command,
                            .code = Error::Code::BadCommand
                        },
                        .position = 0
                    });
                }
                return result.value();
            }
        };
        template <class ...T>
            requires (std::same_as<decltype(T::type), ParameterType> && ...)
        struct FindAndParse<std::tuple<T...>> 
        {
            using tuple_type = std::tuple<T...>;
            constexpr auto operator()(utils::Iterable<std::string_view> auto args) const -> PosExpected<tuple_type> {
                PosExpected<tuple_type> result;

                for (auto itarg = args.begin(); itarg != args.end() && result;) {
                    bool maybe_flag = false, maybe_arg = false;
                    auto arg = *itarg;
                    std::string_view name;
                    std::optional<std::string_view> value;

                    if (arg.starts_with("---")) {
                        result = make_unexpected(PositionnedError {
                            .error = Error{
                                *itarg,
                                std::nullopt,
                                Error::Type::None,
                                Error::Code::SyntaxError
                            },
                            .position = std::distance(args.begin(), itarg)
                        });
                        break;
                    } else if (arg.starts_with("--")) {
                        auto pos_equal = arg.find('=', 2);
                        name = arg.substr(2, pos_equal);
                        if (pos_equal != std::string_view::npos)
                            value = arg.substr(pos_equal+1);
                        maybe_arg = value.has_value();
                        maybe_flag = !maybe_arg;
                    } else if (arg.starts_with("-")) {
                        name = arg.substr(1);
                        maybe_arg = maybe_flag = true;
                    } else {
                        //is input

                    }

                    ([&] {
                        Expected<bool> exp_found;
                        if (maybe_arg && maybe_flag) { // == is short
                            exp_found = find_shortname<T>(name);
                        }
                        else if (maybe_flag) {
                            if constexpr(T::type == ParameterType::Flag) {
                                exp_found = find_longname<T>(name);
                            }
                        }
                        else if (maybe_arg) {
                            if constexpr(T::type == ParameterType::Argument) {
                                exp_found = find_longname<T>(name);
                            }
                        }
                        if (!exp_found) 
                            result = make_unexpected(PositionnedError {
                                .error = std::move(exp_found.error()),
                                .position = std::distance(args.begin(), itarg)
                            });
                        else if (*exp_found) {
                            // std::get<T>(result) = parse_item<T>(args);
                        }
                        return !result // is error
                            || *exp_found; // is found or not
                    }() || ...);
                    itarg++;
                }

                return result;
            }
        };
        template <class C>
        static constexpr auto find_and_parse = FindAndParse<C>{};
        
        template <class ...P>
        struct ParseItem<Command<P...>> {
            constexpr auto operator()(utils::Iterable<std::string_view> auto args) const -> PosExpected<Command<P...>> {

            }
        };
        template <class C>
        static constexpr auto parse_item = ParseItem<C>{};

        template <class Current, class ...Others>
        constexpr auto find_by_name(std::string_view cmd_name) const noexcept -> PosExpected<std::variant<Commands...>> {
            if (cmd_name == Current::Longname) {
                return Current{};
            } 
            auto exp_codepoint = glap::utils::uni::codepoint(cmd_name);
            if (!exp_codepoint) {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .argument = cmd_name,
                        .value = std::nullopt,
                        .type = Error::Type::Command,
                        .code = Error::Code::BadString
                    },
                    .position = 0
                });
            }
            auto codepoint = exp_codepoint.value();
            if (codepoint == Current::Shortname) {
                return Current{};
            }
            if constexpr(sizeof...(Others) == 0) {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .argument = cmd_name,
                        .value = std::nullopt,
                        .type = Error::Type::Command,
                        .code = Error::Code::BadCommand
                    },
                    .position = 0
                });
            } else {
                return find_by_name<Others...>(cmd_name);
            }
        }
    };
}