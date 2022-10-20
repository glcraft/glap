#pragma once
#include "glap/core/utf8.h"
#include "../model.h"
#include "../parser.h" // for lsp
#include <optional>
#include <string_view>
#include <type_traits>
#include <variant>

namespace glap
{
    namespace impl
    {
        template <class T>
        struct FindAndParse {
            template <class Iter>
            constexpr auto operator()(utils::BiIterator<Iter> args) const -> PosExpected<T>;
        };
        template <class C>
        static constexpr auto find_and_parse = FindAndParse<C>{};
    }
    template <class ...Args>
    struct ParseCommand<model::Command<Args...>> {
        using item_type = model::Command<Args...>;
        template <class Iter>
        constexpr auto operator()(item_type& command, utils::BiIterator<Iter> args) const -> PosExpected<void> {
            auto parsed = impl::find_and_parse<decltype(command.params)>(args);
            if (!parsed) [[unlikely]] {
                return make_unexpected(parsed.error());
            }
            command.params = *parsed;
            return {};
        }
    };
    template <class Valued>
        requires requires(Valued a) {
            {a.value} -> std::convertible_to<std::optional<std::string_view>>;
        }
    struct ParseArgument<Valued> {
        using item_type = Valued;
        static constexpr auto error_type = (Valued::type == model::ArgumentType::Input) ? Error::Type::Input : Error::Type::Parameter;
        static constexpr auto validate(std::string_view value) requires std::invocable<decltype(Valued::validator), std::string_view> {
            return Valued::validator(value);
        }
        static constexpr auto validate(std::string_view value) {
            return true;
        }
        constexpr auto operator()(item_type& arg, std::optional<std::string_view> value) const -> Expected<void> {
            if (arg.value) {
                return make_unexpected(Error{
                    std::string_view{},
                    value,
                    error_type,
                    Error::Code::AlreadySet
                });
            }
            if (!ParseArgument::validate(value.value())) [[unlikely]] {
                return make_unexpected(Error{
                    std::string_view{},
                    value,
                    error_type,
                    Error::Code::InvalidValue
                });
            }
            arg.value = value.value();
            return {};
        }
    };
    template <class Contained>
        requires std::constructible_from<typename Contained::value_type, std::string_view> && requires (Contained a, typename Contained::value_type v) {
            {a.values.push_back(v)};
        }
    struct ParseArgument<Contained> {
        using container_type = Contained;
        static constexpr auto error_type = (Contained::type == model::ArgumentType::Input) ? Error::Type::Input : Error::Type::Parameter;
        static constexpr auto validate(std::string_view value) requires std::invocable<decltype(Contained::validator), std::string_view> {
            return Contained::validator(value);
        }
        static constexpr auto validate(std::string_view value) {
            return true;
        }
        constexpr auto operator()(container_type& arg, std::optional<std::string_view> value) const -> Expected<void> {
            if (!ParseArgument::validate(value.value())) [[unlikely]] {
                return make_unexpected(Error{
                    std::string_view{},
                    value,
                    error_type,
                    Error::Code::InvalidValue
                });
            }
            arg.values.push_back(typename container_type::value_type(value.value()));
            return {};
        }
    };
    template <class ...Args>
    struct ParseArgument<model::Flag<Args...>> {
        using item_type = model::Flag<Args...>;
        constexpr auto operator()(item_type& arg, std::optional<std::string_view> value) const -> Expected<void> {
            arg.occurences += 1;
            return {};
        }
    };
    namespace impl 
    {
        template <HasNames Ty>
        constexpr auto find_longname(std::string_view name) -> bool {
            return name == Ty::longname;
        }
        template <HasNames Ty>
        constexpr auto find_shortname(std::string_view name) -> Expected<bool> {
            auto exp_codepoint = glap::utils::uni::codepoint(name);
            if (!exp_codepoint) [[unlikely]] {
                return glap::make_unexpected(Error {
                    .parameter = name,
                    .value = std::nullopt,
                    .type = Error::Type::Command,
                    .code = Error::Code::BadString
                });
            }
            auto codepoint = exp_codepoint.value();
            return codepoint == Ty::shortname;
        }
        template <HasNames Ty>
        constexpr auto find_name(std::string_view name) -> Expected<bool> {
            if (auto value = find_longname<Ty>(name); value)
                return value;
            return find_shortname<Ty>(name);
        }
        
        template <HasNames Com>
        struct FindAndParse<Com> {
            template <class Iter>
            constexpr auto operator()(utils::BiIterator<Iter> args) const -> PosExpected<Com> {
                Com result;
                auto res = parse_command<Com>(result, args);
                if (!res) [[unlikely]] {
                    return make_unexpected(res.error());
                }
                return result;
            }
        };
        template <HasNames ...T>
        struct FindAndParse<std::variant<T...>> {
            template <class Iter>
            constexpr auto operator()(utils::BiIterator<Iter> args) const -> PosExpected<std::variant<T...>> {
                std::optional<PosExpected<std::variant<T...>>> result = std::nullopt;
                auto cmd_name = *args.begin;
                ([&] {
                    auto exp_found = find_name<T>(cmd_name);
                    if (!exp_found) [[unlikely]] 
                        result = make_unexpected(PositionnedError {
                            .error = std::move(exp_found.error()),
                            .position = 0
                        });
                    else if (*exp_found) {
                        result.emplace(T{});
                        auto res = parse_command<T>(std::get<T>(result.value().value()), utils::BiIterator{std::next(args.begin), args.end});
                        if (!res) [[unlikely]] {
                            res.error().position+=1;
                            result = make_unexpected(std::move(res.error()));
                            return true;
                        }
                    }
                    return !exp_found || *exp_found;
                }() || ...);

                if (!result.has_value()) [[unlikely]] {
                    return make_unexpected(PositionnedError {
                        .error = Error {
                            .parameter = cmd_name,
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
            requires (std::same_as<std::decay_t<decltype(T::type)>, model::ArgumentType> && ...)
        class FindAndParse<std::tuple<T...>> 
        {
            using tuple_type = std::tuple<T...>;
            struct ParamInfo {
                struct Long {
                    struct Iterator {
                        std::string_view name;
                        constexpr auto operator*() noexcept -> std::optional<std::string_view> {
                            if (name.empty())
                                return std::nullopt;
                            auto result = name;
                            name = std::string_view();
                            return result;
                        }
                    };

                    std::string_view name;

                    constexpr auto iter() noexcept -> Iterator {
                        return Iterator{name};
                    }
                    constexpr auto iter() const noexcept -> Iterator {
                        return Iterator{name};
                    }
                };
                struct Short {
                    struct Iterator {
                        std::string_view shortnames;
                        constexpr auto operator*() noexcept -> std::optional<char32_t> {
                            if (shortnames.empty())
                                return std::nullopt;
                            auto len = glap::utils::uni::utf8_char_length(shortnames);
                            auto result = glap::utils::uni::codepoint(shortnames);
                            if (!result || !len)
                                return std::nullopt;
                            shortnames = std::string_view(shortnames.begin()+len.value(), shortnames.end());
                            return result.value();
                        }
                    };
                    std::string_view names;

                    constexpr auto iter() noexcept -> Iterator {
                        return Iterator{names};
                    }
                    constexpr auto iter() const noexcept -> Iterator {
                        return Iterator{names};
                    }
                };
                
                std::optional<std::variant<Long, Short>> name = std::nullopt;
                std::optional<std::string_view> value = std::nullopt;

                ParamInfo() = default;
                static constexpr auto Parse(std::string_view arg) -> Expected<ParamInfo> {
                    ParamInfo result;
                    if (auto exp_res = result.parse(arg); !exp_res) [[unlikely]] {
                        return make_unexpected(exp_res.error());
                    }
                    return result;
                }

                constexpr auto parse(std::string_view arg) -> Expected<void> {
                    if (arg.starts_with("---")) {
                        return make_unexpected(Error{
                            arg,
                            std::nullopt,
                            Error::Type::None,
                            Error::Code::SyntaxError
                        });
                    } else if (arg.starts_with("--")) {
                        auto pos_equal = arg.find('=', 2);
                        if (pos_equal != std::string_view::npos) {
                            value = arg.substr(pos_equal+1);
                            name = Long{arg.substr(2, pos_equal-2)};
                        } else {
                            name = Long{arg.substr(2)};
                        }
                    } else if (arg.starts_with("-")) {
                        name = Short{arg.substr(1)};
                    } else {
                        value = arg;
                    }
                    return {};
                }
            };
            constexpr auto find_parse_longname(tuple_type& arguments, std::string_view longname, std::optional<std::string_view> value) const -> Expected<void> {
                auto result = Expected<void>{};
                auto found = ([&] {
                    if constexpr(!HasNames<T>)
                        return false;
                    else {
                        if constexpr(!std::same_as<std::decay_t<decltype(T::longname)>, std::string_view>)
                            return false;
                        if (T::longname != longname)
                            return false;
                    }
                    result = parse_argument<T>(std::get<T>(arguments), value);
                    return true;
                }() || ...);
                if (!found)
                    return make_unexpected(Error{
                        longname,
                        std::nullopt,
                        Error::Type::None,
                        Error::Code::UnknownArgument
                    });
                return result;
            }
            template <class Iter>
            constexpr auto find_parse_shortname(tuple_type& arguments, Iter& itarg, Iter itend, char32_t shortname) const -> Expected<void> {
                auto result = Expected<void>{};
                auto found = ([&] {
                    if constexpr(!HasNames<T>)
                        return false;
                    else {
                        if constexpr(!std::same_as<std::decay_t<decltype(T::shortname)>, std::optional<char32_t>>)
                            return false;
                        if constexpr(!T::shortname.has_value())
                            return false;
                        if (T::shortname.value() != shortname)
                            return false;
                    }
                    std::optional<std::string_view> value = std::nullopt;
                    if constexpr(T::type == model::ArgumentType::Parameter) {
                        if (++itarg == itend) [[unlikely]] {
                            result = make_unexpected(Error{
                                std::string_view{},
                                std::nullopt,
                                Error::Type::Parameter,
                                Error::Code::MissingValue
                            });
                            return true;
                        }
                        value = *itarg;
                    }
                    result = parse_argument<T>(std::get<T>(arguments), value);
                    return true;
                }() || ...);
                if (!found)
                    return make_unexpected(Error{
                        *itarg,
                        std::nullopt,
                        Error::Type::None,
                        Error::Code::UnknownArgument
                    });
                return result;
            }
            constexpr auto find_parse_input(tuple_type& arguments, std::optional<std::string_view> value) const -> Expected<void> {
                auto result = Expected<void>{};
                ([&] {
                    if constexpr(T::type != model::ArgumentType::Input)
                        return false;
                    result = parse_argument<T>(std::get<T>(arguments), value);
                    return true;
                }() || ...);
                return result;
            }
        public:
            template <class Iter>
            constexpr auto operator()(utils::BiIterator<Iter> args) const -> PosExpected<tuple_type> {
                PosExpected<tuple_type> result;

                for (auto itarg = args.begin; itarg != args.end && result;) {
                    auto arg = *itarg;
                    
                    auto exp_param_info = ParamInfo::Parse(arg);
                    if (!exp_param_info) [[unlikely]] {
                        return make_unexpected(PositionnedError {
                            .error = exp_param_info.error(),
                            .position = std::distance(args.begin, itarg)
                        });
                    }
                    auto param_info = std::move(exp_param_info.value());
                    auto exp_ok = [&] {
                        if (param_info.name) {
                            // std::variant<ParamInfo::Long, ParamInfo::Short>& 
                            if (std::holds_alternative<typename ParamInfo::Long>(*param_info.name)) {
                                return find_parse_longname(result.value(), std::get<typename ParamInfo::Long>(param_info.name.value()).name, param_info.value);
                            } else {
                                auto& shortnames= std::get<typename ParamInfo::Short>(param_info.name.value());
                                auto itshorts = shortnames.iter();
                                while (auto name = *itshorts) {
                                    auto exp_short = find_parse_shortname(result.value(), itarg, args.end, *name);
                                    if (!exp_short) {
                                        exp_short.error().parameter = shortnames.names;
                                        return exp_short;
                                    }
                                }
                            }
                        } else if (param_info.value) {
                            return find_parse_input(result.value(), param_info.value);
                        }
                        return Expected<void>{};
                    }();
                    if (!exp_ok) {
                        return make_unexpected(PositionnedError {
                            .error = exp_ok.error(),
                            .position = std::distance(args.begin, itarg)
                        });
                    }
                    itarg++;
                }

                return result;
            }
        };
        
    }
    template<StringLiteral Name, DefaultCommand def_cmd, class... Commands>
    constexpr auto Parser<Name, def_cmd, Commands...>::parse(glap::utils::Iterable<std::string_view> auto args) const -> PosExpected<model::Program<Commands...>> {
        if (args.size() == 0) [[unlikely]] {
            return make_unexpected(PositionnedError{
                .error = Error{
                    .parameter = "",
                    .value = std::nullopt,
                    .type = Error::Type::None,
                    .code = Error::Code::NoParameter
                },
                .position = 0
            });
        }
        auto itarg = args.begin();
        model::Program<Commands...> program;
        program.program = *itarg++;

        auto default_command = [&] () mutable {
            if (itarg == args.end()) {
                return true;
            }
            auto arg = std::string_view{*itarg};
            if (arg.starts_with("-")) {
                return true;
            }
            return false;
        }();
        auto call_command = [&] <class T> () -> PosExpected<model::Program<Commands...>> {
            auto found_command = impl::find_and_parse<T>(utils::BiIterator{itarg, args.end()});
            if (!found_command) [[unlikely]] {
                found_command.error().position += std::distance(args.begin(), itarg);
                return make_unexpected(found_command.error());
            }
            program.command = found_command.value();
            return program;
        };
        if (default_command) {
            if constexpr (def_cmd == DefaultCommand::None) {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .parameter = "",
                        .value = std::nullopt,
                        .type = Error::Type::None,
                        .code = Error::Code::NoGlobalCommand
                    },
                    .position = 0
                });
            } else {
                return call_command.template operator()<default_command_type>();
            }
        } else {
            return call_command.template operator()<decltype(program.command)>();
        }
    }
}