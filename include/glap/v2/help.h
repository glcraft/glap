#pragma once
#include "glap/v2/discard.h"
#include "utils.h"
#include <optional>
#include <string_view>
namespace glap::v2 {
    namespace help {

        template<auto Short, auto Long>
        struct Description {
            static constexpr std::optional<std::string_view> long_description = impl::optional_value<std::string_view, Long>;
            static constexpr std::string_view short_description = Short;
        };
        template <StringLiteral Short, StringLiteral Long>
        static constexpr auto description = Description<Long, Short>{};
        template <StringLiteral Short>
        static constexpr auto short_description = Description<Short, discard>{};

        template<StringLiteral Name, Description Desc> 
        struct CommandDescription
        {
            static constexpr auto name = Name;
            static constexpr auto description = Desc;
        };
    }

    template<class ...> 
    class Help 
    {};

    template<class Parser, class ...CommandDesc> 
    class Help <Parser, CommandDesc...>
    {

    };
}