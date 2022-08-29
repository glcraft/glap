#pragma once

#include "../common/expected.h"
#include "../common/error.h"
#include "../common/utils.h"
#include "../common/utf8.h"
#include "utils.h"
#include "biiterator.h"
#include "model.h"
namespace glap::v2 
{

    template<class... Commands>
    class Parser {
        using NameCheck = NameChecker<Commands...>;
        static_assert(!NameCheck::has_duplicate_longname, "commands has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "commands has duplicate short name");
    public:
        
        template <class P>
        struct ParseParameter {
            constexpr auto operator()(P&, std::optional<std::string_view> value) const -> Expected<void>;
        };
        template <class C>
        static constexpr auto parse_parameter = ParseParameter<C>{};
        template <class C>
        struct ParseCommand {
            template <class Iter>
            constexpr auto operator()(C&, utils::BiIterator<Iter> args) const -> PosExpected<C>;
        };
        template <class C>
        static constexpr auto parse_command = ParseCommand<C>{};

        constexpr auto parse(glap::utils::Iterable<std::string_view> auto args) const -> PosExpected<model::Program<Commands...>> {
            if (args.size() == 0) [[unlikely]] {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .argument = "",
                        .value = std::nullopt,
                        .type = Error::Type::None,
                        .code = Error::Code::NoArgument
                    },
                    .position = 0
                });
            }
            auto itarg = args.begin();
            model::Program<Commands...> program;
            program.program = *itarg++;
            
            //TODO: implement global command

            if (itarg == args.end()) [[unlikely]] {
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
            if (arg.starts_with("-")) [[unlikely]] {
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
            auto found_command = find_and_parse<decltype(program.command)>(utils::BiIterator{itarg, args.end()});
            if (!found_command) [[unlikely]] {
                found_command.error().position += std::distance(args.begin(), itarg);
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
            if (!exp_codepoint) [[unlikely]] {
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
            template <class Iter>
            constexpr auto operator()(utils::BiIterator<Iter> args) const -> PosExpected<T>;
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
                            result = make_unexpected(res.error());
                            return true;
                        }
                    }
                    return !exp_found || *exp_found;
                }() || ...);

                if (!result.has_value()) [[unlikely]] {
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
            requires (std::same_as<std::decay_t<decltype(T::type)>, model::ParameterType> && ...)
        struct FindAndParse<std::tuple<T...>> 
        {
            using tuple_type = std::tuple<T...>;
            struct ParamInfo {
                bool maybe_flag = false, maybe_arg = false;
                std::optional<std::string_view> name = std::nullopt;
                std::optional<std::string_view> value = std::nullopt;
                ParamInfo() = default;
                static constexpr auto Parse(std::string_view arg) -> Expected<ParamInfo> {
                    ParamInfo result;
                    if (auto exp_res = result.parse(arg); !exp_res) [[unlikely]] {
                        return make_unexpected(exp_res.error());
                    }
                    return result;
                }

                auto parse(std::string_view arg) -> Expected<void> {
                    if (arg.starts_with("---")) {
                        return make_unexpected(Error{
                            arg,
                            std::nullopt,
                            Error::Type::None,
                            Error::Code::SyntaxError
                        });
                    } else if (arg.starts_with("--")) {
                        auto pos_equal = arg.find('=', 2);
                        name = arg.substr(2, pos_equal-2);
                        if (pos_equal != std::string_view::npos)
                            value = arg.substr(pos_equal+1);
                        maybe_arg = value.has_value();
                        maybe_flag = !maybe_arg;
                    } else if (arg.starts_with("-")) {
                        name = arg.substr(1);
                        maybe_arg = maybe_flag = true;
                    } else {
                        value = arg;
                    }
                    return {};
                }
            };

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
                    auto found = ([&] {
                        Expected<bool> exp_found;
                        if constexpr (HasNames<T>) {
                            if (param_info.maybe_arg && param_info.maybe_flag) { // == is short
                                exp_found = find_shortname<T>(param_info.name.value());
                            }
                            else if (param_info.maybe_flag) {
                                if constexpr(T::type == model::ParameterType::Flag) {
                                    exp_found = find_longname<T>(param_info.name.value());
                                }
                            }
                            else if (param_info.maybe_arg) {
                                if constexpr(T::type == model::ParameterType::Argument) {
                                    exp_found = find_longname<T>(param_info.name.value());
                                }
                            }
                        } else { // we admit this is has to be an input type
                            exp_found = (!param_info.maybe_arg && !param_info.maybe_flag) && (T::type == model::ParameterType::Input);
                        }
                        if (!exp_found) [[unlikely]] {
                            result = make_unexpected(PositionnedError {
                                .error = std::move(exp_found.error()),
                                .position = std::distance(args.begin, itarg)
                            });
                            return true;
                        }
                        else if (*exp_found) {
                            if constexpr(T::type == model::ParameterType::Argument) {
                                if (param_info.maybe_arg && param_info.maybe_flag) {
                                    if (++itarg == args.end) [[unlikely]] {
                                        result = make_unexpected(PositionnedError {
                                            .error = Error{
                                                *itarg,
                                                std::nullopt,
                                                Error::Type::None,
                                                Error::Code::MissingValue
                                            },
                                            .position = std::distance(args.begin, itarg)
                                        });
                                        return true;
                                    }
                                    param_info.value = *itarg;
                                }
                            }
                            auto exp_res = parse_parameter<T>(std::get<T>(result.value()), param_info.value);
                            if (!exp_res) [[unlikely]] {
                                result = make_unexpected(PositionnedError {
                                    .error = exp_res.error(),
                                    .position = std::distance(args.begin, itarg)
                                });
                            }
                        }
                        return !result // quit if is error...
                            || *exp_found; // or is found.
                    }() || ...);
                    if (!found) {
                        return make_unexpected(PositionnedError {
                            .error = Error{
                                *itarg,
                                std::nullopt,
                                Error::Type::None,
                                Error::Code::UnknownParameter
                            },
                            .position = std::distance(args.begin, itarg)
                        });
                    }
                    itarg++;
                }

                return result;
            }
        };
        template <class C>
        static constexpr auto find_and_parse = FindAndParse<C>{};

        template <class ...Args>
        struct ParseCommand<model::Command<Args...>> {
            using item_type = model::Command<Args...>;
            template <class Iter>
            constexpr auto operator()(item_type& command, utils::BiIterator<Iter> args) const -> PosExpected<bool> {
                auto parsed = find_and_parse<decltype(command.params)>(args);
                if (!parsed) [[unlikely]] {
                    return make_unexpected(parsed.error());
                }
                command.params = *parsed;
                return true;
            }
        };
        template <class Valued>
            requires requires(Valued a) {
                {a.value} -> std::convertible_to<std::optional<std::string_view>>;
            }
        struct ParseParameter<Valued> {
            using item_type = Valued;
            static constexpr auto error_type = (Valued::type == model::ParameterType::Input) ? Error::Type::Input : Error::Type::Argument;
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
                if (!ParseParameter::validate(value.value())) [[unlikely]] {
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
            requires requires (Contained a) {
                {a.values.emplace_back().value} -> std::convertible_to<std::optional<std::string_view>>;
            }
        struct ParseParameter<Contained> {
            using item_type = Contained;
            static constexpr auto error_type = (Contained::type == model::ParameterType::Input) ? Error::Type::Input : Error::Type::Argument;
            static constexpr auto validate(std::string_view value) requires std::invocable<decltype(Contained::validator), std::string_view> {
                return Contained::validator(value);
            }
            static constexpr auto validate(std::string_view value) {
                return true;
            }
            constexpr auto operator()(item_type& arg, std::optional<std::string_view> value) const -> Expected<void> {
                if (!ParseParameter::validate(value.value())) [[unlikely]] {
                    return make_unexpected(Error{
                        std::string_view{},
                        value,
                        error_type,
                        Error::Code::InvalidValue
                    });
                }
                arg.values.emplace_back().value = value.value();
                return {};
            }
        };
        template <class ...Args>
        struct ParseParameter<model::Flag<Args...>> {
            using item_type = model::Flag<Args...>;
            constexpr auto operator()(item_type& arg, std::optional<std::string_view> value) const -> Expected<void> {
                arg.occurences += 1;
                return {};
            }
        };
    };
}