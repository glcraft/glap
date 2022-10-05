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
    }

    template<StringLiteral Name, help::model::IsDescription Desc, class ...CommandsDesc>
    class Help<help::model::Program<Name, Desc, CommandsDesc...>>
    {
        using program_t = help::model::Program<Name, Desc, CommandsDesc...>;
        template <class>
        struct GetHelp
        {};
        template<DefaultCommand def_cmd, class... Commands> 
        struct GetHelp<Parser<def_cmd, Commands...>>
        {
            using parser_t = Parser<def_cmd, Commands...>;
            [[nodiscard]] constexpr std::string operator()() const noexcept {
                std::string result;
                this->operator()(std::back_inserter(result));
                return result;
            }
            template <class OutputIt>
            constexpr OutputIt operator()(OutputIt it) const noexcept {
                if constexpr(help::model::IsFullDescription<Desc>)
                    it = fmt::format_to(it, "{} - {}\n\n{}\n", Name, program_t::short_description, program_t::long_description);
                else
                    it = glap::format_to(it, "{} - {}\n", Name, program_t::short_description);
                it = glap::format_to(it, "Commands:\n");
                constexpr auto max_cmd_name_length = max_length<Commands...>(2)+2;
                it = display<EmbedArgs<Commands...>, EmbedArgs<CommandsDesc...>>(it, max_cmd_name_length);
                return it;
            }
        private:
            template <class ...Args>
            struct EmbedArgs
            {};
            template <class EmbedParser, class EmbedHelp>
            struct Display
            {};
            template <HasNames... Nameds>
            static constexpr auto max_length(size_t spacing) -> size_t {
                size_t max = 0;
                (..., (max = std::max(max, Nameds::Longname.size()+(Nameds::Shortname ? 1+spacing : 0))));
                return max;
            }

            template <class EmbedParser, class EmbedHelp>
            static constexpr auto display = Display<EmbedParser, EmbedHelp>{};

            template <class... ArgsParser, class... ArgsHelp>
            struct Display<EmbedArgs<ArgsParser...>, EmbedArgs<ArgsHelp...>>
            {
                template <class OutputIt>
                constexpr OutputIt operator()(OutputIt it, size_t width) const {
                    ((it = display<ArgsParser, EmbedArgs<ArgsHelp...>>(it, width)),...);
                    return it;
                }
            };
            template <class ArgParser, class... ArgsHelp>
            struct Display<ArgParser, EmbedArgs<ArgsHelp...>>
            {
                template <class OutputIt>
                constexpr OutputIt operator()(OutputIt it, size_t width) const {
                    bool found = ([&] {
                        if constexpr (ArgParser::Longname == ArgsHelp::name){
                            it = display<ArgParser, ArgsHelp>(it, width);
                            return true;
                        } 
                        return false;
                    }() || ...);
                    if (!found) {
                        return display<ArgParser, Discard>(it, width);
                    }
                    return it;
                }
            };

            template <class ArgParser, class ArgHelp>
                requires requires {
                    ArgParser::Longname == ArgHelp::name;
                }
            struct Display<ArgParser, ArgHelp>
            {
                template <class OutputIt>
                constexpr OutputIt operator()(OutputIt it, size_t width) const {
                    auto spacing = width - max_length<ArgParser>(2);
                    if constexpr(ArgParser::Shortname.has_value())
                        it = glap::format_to(it, "{0:>{1}}{2}, {3}", "", spacing, glap::utils::uni::codepoint_to_utf8(ArgParser::Shortname.value()), ArgParser::Longname);
                    else
                        it = glap::format_to(it, "{0:>{1}}{2}", "", spacing, ArgParser::Longname);
                    return glap::format_to(it, ": {}\n", ArgHelp::short_description);
                } 
            };
            template <class ArgParser>
            struct Display<ArgParser, Discard>
            {
                template <class OutputIt>
                constexpr OutputIt operator()(OutputIt it, size_t width) const {
                    auto spacing = width - max_length<ArgParser>(2);
                    if constexpr(ArgParser::Shortname.has_value())
                        it = glap::format_to(it, "{0:>{1}}{2}, {3}", "", spacing, glap::utils::uni::codepoint_to_utf8(ArgParser::Shortname.value()), ArgParser::Longname);
                    else
                        it = glap::format_to(it, "{0:>{1}}{2}", "", spacing, ArgParser::Longname);
                    return glap::format_to(it, ": (no description)\n");
                } 
            };

        };
    public:
        template<class P> 
        static constexpr auto get_help = GetHelp<P>{};
    };
}