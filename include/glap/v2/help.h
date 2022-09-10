#pragma once
#include "glap/v2/discard.h"
#include "utils.h"
#include <optional>
#include <string_view>
#include <type_traits>
namespace glap::v2 {
    namespace help {
        namespace model 
        {
            template <class T>
            concept IsDescription = requires (T a) {
                {a.short_description} -> std::convertible_to<std::string_view>;
            };
            template <class T>
            concept IsFullDescription = IsDescription<T> && requires (T a) {
                {a.long_description} -> std::convertible_to<std::string_view>;
            };

            template<StringLiteral Short, auto Long = discard>
            struct Description 
            {};
            template<StringLiteral Short, StringLiteral Long>
            struct Description<Short, Long> {
                static constexpr std::string_view long_description = Long;
                static constexpr std::string_view short_description = Short;
            };
            template<StringLiteral Short>
            struct Description<Short, discard> {
                static constexpr std::string_view short_description = Short;
            };
            template<StringLiteral Short, StringLiteral Long>
            using FullDescription = Description<Short, Long>;

            template<StringLiteral Name, IsDescription Desc> 
            struct Parameter : Desc {
                static constexpr std::string_view name = Name;
            };
            template<StringLiteral Name, IsDescription Desc, Parameter ...Params>
            struct Command : Desc {
                static constexpr auto name = Name;
            };
            template<StringLiteral Name, IsDescription Desc, class ...Commands>
            struct Program : Desc
            {};
        }
    }

    template<class> 
    class Help 
    {};
}
#include "help.inl"