#pragma once
#include "../../core/utf8.h"
#include "../../core/discard.h"
#include "../../core/utils.h"
#include "../../core/fmt.h"
#include "../help.h"

#include "../../model.h"
#include <concepts>
#include <cstddef>
#include <algorithm>
namespace glap::generators {
    namespace impl {
        template <class P, class H>
        concept IsHelpInputsCompatible = requires { P::type == glap::model::ArgumentType::Input; } && help::IsInputs<H>;
        template <class FromParser, class ...Others>
        struct FindByName 
        {};
        template <class FromParser, class T, class ...Others>
            requires (HasLongName<FromParser> && FromParser::longname == T::name) || (requires {FromParser::name;} && FromParser::name == T::name) || IsHelpInputsCompatible<FromParser, T>
        struct FindByName<FromParser, T, Others...>
        {
        public:
            using type = T;
        };
        template <class Named, class T, class ...Others>
        struct FindByName<Named, T, Others...> : FindByName<Named, Others...> 
        {};
        template <class FromParser>
        struct FindByName<FromParser> 
        {
        public:
            using type = void;
        };
        template <class... FromParser>
        constexpr auto max_length(size_t separator_length) -> size_t {
            size_t max = 0;
            ([&max, separator_length]() {
                size_t len = 0;
                if constexpr (HasNames<FromParser>) {
                    len = FromParser::longname.length();
                    if constexpr (HasShortName<FromParser>) {
                        len += 1 + separator_length;
                    }
                } else if constexpr (glap::model::IsArgumentTyped<FromParser, glap::model::ArgumentType::Input>) {
                    len = help::INPUTS_NAME.length();
                }
                if (len > max) {
                    max = len;
                }
            }(), ...);
            return max;
        }
        template <class>
        static constexpr bool always_false_v = false;
        template<class T, class>
        struct BasicHelp
        {
            static_assert(always_false_v<T>, "unable to match help for this type");
        };
        template<class FromHelp, glap::model::IsArgumentTyped<glap::model::ArgumentType::Input> InputParser>
            requires help::IsInputs<FromHelp>
        struct BasicHelp<FromHelp, InputParser>
        {
            template <class OutputIt, bool Fullname = false>
            OutputIt name(OutputIt it) const noexcept {
                return glap::format_to(it, "INPUTS");
            }
            template <class OutputIt, bool FullDescription = false>
            OutputIt description(OutputIt it) const noexcept {
                if constexpr(help::IsFullDescription<FromHelp>)
                    return glap::format_to(it, "{}\n\n{}", FromHelp::short_description, FromHelp::long_description);
                else
                    return glap::format_to(it, "{}", FromHelp::short_description);
            }
            template <class OutputIt, bool FullName = false, bool FullDescription = false>
            OutputIt identity(OutputIt it) const noexcept {
                it = name<OutputIt, FullName>(it);
                it = glap::format_to(it, " - ");
                it = description<OutputIt, FullDescription>(it);
                return it;
            }
        };
        template<help::IsDescription FromHelp, class FromParser>
            requires (HasLongName<FromParser> || requires { FromParser::name; })
        struct BasicHelp<FromHelp, FromParser>
        {
            template <class OutputIt, bool Fullname = false>
            OutputIt name(OutputIt it) const noexcept {
                if constexpr(Fullname && HasShortName<FromParser>)
                    return glap::format_to(it, "{}, {}", glap::utils::uni::codepoint_to_utf8(FromParser::shortname.value()), FromParser::longname);
                else if constexpr(requires { FromParser::name; })
                    return glap::format_to(it, "{}", FromParser::name);
                else 
                    return glap::format_to(it, "{}", FromParser::longname);
            }
            template <class OutputIt, bool FullDescription = false>
            OutputIt description(OutputIt it) const noexcept {
                if constexpr(FullDescription && help::IsFullDescription<FromHelp>)
                    return glap::format_to(it, "{}\n\n{}", FromHelp::short_description, FromHelp::long_description);
                else
                    return glap::format_to(it, "{}", FromHelp::short_description);
            }
            template <class OutputIt, bool FullName = false, bool FullDescription = false>
            OutputIt identity(OutputIt it) const noexcept {
                it = name<OutputIt, FullName>(it);
                it = glap::format_to(it, " - ");
                it = description<OutputIt, FullDescription>(it);
                return it;
            }
        };
        template<HasNames FromParser>
        struct BasicHelp<void, FromParser>
        {
            template <class OutputIt, bool Fullname = false>
            OutputIt name(OutputIt it) const noexcept {
                if constexpr(Fullname && HasNames<FromParser> && FromParser::shortname.has_value())
                    return glap::format_to(it, "{}, {}", glap::utils::uni::codepoint_to_utf8(FromParser::shortname.value()), FromParser::longname);
                else
                    return glap::format_to(it, "{}", FromParser::longname);
            }
            template <class OutputIt, bool FullDescription = false>
            OutputIt description(OutputIt it) const noexcept {
                return glap::format_to(it, "(no description)");
            }
            template <class OutputIt, bool FullName = false, bool FullDescription = false>
            OutputIt identity(OutputIt it) const noexcept {
                it = name<OutputIt, FullName>(it);
                it = glap::format_to(it, " - (no description)");
                return it;
            }
        };
        template<class FromHelp, class FromParser>
        static constexpr auto basic_help = BasicHelp<FromHelp, FromParser>{};
    }

    template<StringLiteral NameHelp, help::IsDescription Desc, class ...CommandsHelp, StringLiteral NameParser, model::DefaultCommand def_cmd, class... CommandsParser>
    struct Help<help::Program<NameHelp, Desc, CommandsHelp...>, model::Program<NameParser, def_cmd, CommandsParser...>> {
        using ProgramHelp = help::Program<NameHelp, Desc, CommandsHelp...>;
        using ProgramParser = model::Program<NameParser, def_cmd, CommandsParser...>;
        
        [[nodiscard]] constexpr std::string operator()() const noexcept {
            std::string result;
            this->operator()(std::back_inserter(result));
            return result;
        }
        template <class OutputIt>
        constexpr OutputIt operator()(OutputIt it) const noexcept {
            it = this_basic_help.template identity<OutputIt, false, false>(it);
            it = glap::format_to(it, "\n\n");
            it = glap::format_to(it, "Command{}:", sizeof...(CommandsParser) > 1 ? "s" : "");
            // constexpr auto max_cmd_name_length = impl::max_length<CommandsParser...>(2)+2;
            ([&] {
                constexpr auto cmd_basic_help = impl::basic_help<typename impl::FindByName<CommandsParser, CommandsHelp...>::type, CommandsParser>;
                constexpr auto spacing = cmd_name_max_length - impl::max_length<CommandsParser>(2);
                it = glap::format_to(it, "\n{0:>{1}}", "", spacing);
                it = cmd_basic_help.template identity<OutputIt, true, false>(it);
            }(), ...);
            return it;
        }
    private:
        static constexpr auto cmd_name_max_length = impl::max_length<CommandsParser...>(2)+help::PADDING;
        static constexpr auto this_basic_help = impl::basic_help<ProgramHelp, ProgramParser>;
    };

    template<StringLiteral Name, help::IsDescription Desc, class ...ParamsHelp, class CommandNames, model::IsArgument... ParamsParser>
    struct Help<help::Command<Name, Desc, ParamsHelp...>, model::Command<CommandNames, ParamsParser...>> {
        using CommandHelp = help::Command<Name, Desc, ParamsHelp...>;
        using CommandParser = model::Command<CommandNames, ParamsParser...>;
        [[nodiscard]] constexpr std::string operator()() const noexcept {
            std::string result;
            this->operator()(std::back_inserter(result));
            return result;
        }
        template <class OutputIt, bool DisplayUsage = true>
        constexpr OutputIt operator()(OutputIt it) const noexcept {
            it = this_basic_help.template identity<OutputIt, false, false>(it);
            it = glap::format_to(it, "\n\n");
            if constexpr(DisplayUsage) {
                it = glap::format_to(it, "Usage:\n{0:>{1}}" , "", help::PADDING);
                it = this_basic_help.name(it);
                ([&] {
                    constexpr auto param_basic_help = impl::basic_help<typename impl::FindByName<ParamsParser, ParamsHelp...>::type, ParamsParser>;
                    if constexpr(model::IsArgumentTyped<ParamsParser, model::ArgumentType::Parameter>) {
                        it = glap::format_to(it, " [--");
                        it = param_basic_help.template name<OutputIt>(it);
                        it = glap::format_to(it, "=VALUE]");
                    } else if constexpr(model::IsArgumentTyped<ParamsParser, model::ArgumentType::Flag>) {
                        it = glap::format_to(it, " [--");
                        it = param_basic_help.template name<OutputIt>(it);
                        it = glap::format_to(it, "]");
                    } else if constexpr(model::IsArgumentTyped<ParamsParser, model::ArgumentType::Input>) {
                        it = glap::format_to(it, " <INPUTS>");
                    }
                }(), ...);
                it = glap::format_to(it, "\n\n");
            }
            it = glap::format_to(it, "Argument{}:", sizeof...(ParamsParser) > 1 ? "s" : "");
            ([&] {
                constexpr auto param_basic_help = impl::basic_help<typename impl::FindByName<ParamsParser, ParamsHelp...>::type, ParamsParser>;
                constexpr auto spacing = param_name_max_length - impl::max_length<ParamsParser>(2);
                it = glap::format_to(it, "\n{0:>{1}}", "", spacing);
                it = param_basic_help.template identity<OutputIt, true, false>(it);
            }(), ...);
            return it;
        }
        private:
            static constexpr auto param_name_max_length = impl::max_length<ParamsParser...>(2)+help::PADDING;
            static constexpr auto this_basic_help = impl::basic_help<CommandHelp, CommandParser>;
    };
    template<StringLiteral NameHelp, help::IsDescription Desc, class ...CommandsHelp, class CommandNames, model::IsArgument... ParamsParser>
    struct Help<help::Program<NameHelp, Desc, CommandsHelp...>, model::Command<CommandNames, ParamsParser...>> : Help<typename impl::FindByName<CommandNames, CommandsHelp...>::type, model::Command<CommandNames, ParamsParser...>>
    {};
}