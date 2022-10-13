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
            requires (Named::longname == T::name)
        struct FindByName<Named, T, Others...>
        {
        public:
            using type = T;
        };
        template <class Named, class T, class ...Others>
            requires (Named::longname != T::name)
        struct FindByName<Named, T, Others...> : FindByName<Named, Others...>
        {};
        template <class Named>
        struct FindByName<Named> 
        {
        public:
            using type = void;
        };
        template <HasNames... Nameds>
        constexpr auto max_length(size_t padding) -> size_t {
            size_t max = 0;
            (..., (max = std::max(max, Nameds::longname.size()+(Nameds::shortname ? 1+padding : 0))));
            return max;
        }
        template <class>
        static constexpr bool always_false_v = false;
        template<class T, class>
        struct BasicHelp
        {
            static_assert(always_false_v<T>, "non instanciable");
        };
        template<help::model::IsDescription FromHelp, HasLongName FromParser>
        struct BasicHelp<FromHelp, FromParser>
        {
            template <class OutputIt, bool Fullname = false>
            OutputIt name(OutputIt it) const noexcept {
                if constexpr(Fullname && HasShortName<FromParser>)
                    return glap::format_to(it, "{}, {}", glap::utils::uni::codepoint_to_utf8(FromParser::shortname.value()), FromParser::longname);
                else
                    return glap::format_to(it, "{}", FromParser::longname);
            }
            template <class OutputIt, bool FullDescription = false>
            OutputIt description(OutputIt it) const noexcept {
                if constexpr(help::model::IsFullDescription<FromHelp>)
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

    template<StringLiteral NameHelp, help::model::IsDescription Desc, class ...CommandsHelp, StringLiteral NameParser, DefaultCommand def_cmd, class... CommandsParser>
    struct Help<help::model::Program<NameHelp, Desc, CommandsHelp...>, Parser<NameParser, def_cmd, CommandsParser...>> : impl::BasicHelp<help::model::Program<NameHelp, Desc, CommandsHelp...>, Parser<NameParser, def_cmd, CommandsParser...>> {
        using ProgramHelp = help::model::Program<NameHelp, Desc, CommandsHelp...>;
        using ProgramParser = Parser<NameParser, def_cmd, CommandsParser...>;
        [[nodiscard]] constexpr std::string operator()() const noexcept {
            std::string result;
            this->operator()(std::back_inserter(result));
            return result;
        }
        template <class OutputIt>
        constexpr OutputIt operator()(OutputIt it) const noexcept {
            it = this->template identity<OutputIt, false, false>(it);
            it = glap::format_to(it, "\n\n");
            it = glap::format_to(it, "Commands:\n");
            constexpr auto max_cmd_name_length = impl::max_length<CommandsParser...>(2)+2;
            ([&] {
                constexpr auto basic_help = impl::basic_help<typename impl::FindByName<CommandsParser, CommandsHelp...>::type, CommandsParser>;
                constexpr auto spacing = max_cmd_name_length - impl::max_length<CommandsParser>(2);
                it = glap::format_to(it, "{0:>{1}}", "", spacing);
                it = basic_help.template identity<OutputIt, true, false>(it);
                it = glap::format_to(it, "\n");
            }(), ...);
            return it;
        }
    private:
    };
}