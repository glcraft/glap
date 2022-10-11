#pragma once
#include "glap/common/utf8.h"
#include "glap/v2/discard.h"
#include "glap/v2/utils.h"
#include "help.h"

#include "model.h"
#include "parser.h"
#include <concepts>
#include <cstddef>
#include <algorithm>
namespace glap::v2 {
    namespace impl {
        template <class Named, class ...Others>
        struct FindByName 
        {};
        template <class Named, class T, class ...Others>
            requires (Named::Longname == T::name)
        struct FindByName<Named, T, Others...>
        {
        public:
            using type = T;
        };
        template <class Named, class T, class ...Others>
            requires (Named::Longname != T::name)
        struct FindByName<Named, T, Others...> : FindByName<Named, Others...>
        {};
        template <class Named>
        struct FindByName<Named> 
        {
        public:
            using type = void;
        };
        template <HasNames... Nameds>
        constexpr auto max_length(size_t spacing) -> size_t {
            size_t max = 0;
            (..., (max = std::max(max, Nameds::Longname.size()+(Nameds::Shortname ? 1+spacing : 0))));
            return max;
        }
    }

    template<StringLiteral Name, help::model::IsDescription Desc, class ...CommandsHelp, DefaultCommand def_cmd, class... CommandsParser>
    struct Help<help::model::Program<Name, Desc, CommandsHelp...>, Parser<def_cmd, CommandsParser...>> {
        using ProgramHelp = help::model::Program<Name, Desc, CommandsHelp...>;
        using ProgramParser = Parser<def_cmd, CommandsParser...>;
        [[nodiscard]] constexpr std::string operator()() const noexcept {
            std::string result;
            this->operator()(std::back_inserter(result));
            return result;
        }
        template <class OutputIt>
        constexpr OutputIt operator()(OutputIt it) const noexcept {
            it = display_full_description<OutputIt, ProgramHelp>(it);
            it = glap::format_to(it, "Commands:\n");
            constexpr auto max_cmd_name_length = impl::max_length<CommandsParser...>(2)+2;
            ([&] {
                it = display_name<OutputIt, CommandsParser>(it, max_cmd_name_length);
                if constexpr(std::same_as<typename impl::FindByName<CommandsParser, CommandsHelp...>::type, void>)
                    it = glap::format_to(it, " - (no description)\n");
                else
                    it = display_description<OutputIt, typename impl::FindByName<CommandsParser, CommandsHelp...>::type>(it);
            }(), ...);
            return it;
        }
    private:
        template <class OutputIt, class CommandParser>
        OutputIt display_name(OutputIt it, std::size_t width) const noexcept {
            auto spacing = width - impl::max_length<CommandParser>(2);
            if constexpr(CommandParser::Shortname.has_value())
                it = glap::format_to(it, "{0:>{1}}{2}, {3}", "", spacing, glap::utils::uni::codepoint_to_utf8(CommandParser::Shortname.value()), CommandParser::Longname);
            else
                it = glap::format_to(it, "{0:>{1}}{2}", "", spacing, CommandParser::Longname);
            return it;
        }
        template <class OutputIt, class CommandHelp>
        OutputIt display_description(OutputIt it) const noexcept {
            return glap::format_to(it, " - {}\n", CommandHelp::short_description);
        }
        template <class OutputIt, class Descr>
            requires help::model::IsDescription<Descr>
        OutputIt display_full_description(OutputIt it) const noexcept {
            if constexpr(help::model::IsFullDescription<Descr>)
                it = fmt::format_to(it, "{} - {}\n\n{}\n\n", Name, Descr::short_description, Descr::long_description);
            else
                it = glap::format_to(it, "{} - {}\n\n", Name, Descr::short_description);
            return it;
        }
    };
}