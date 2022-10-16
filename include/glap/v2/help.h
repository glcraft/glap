#pragma once
#include "glap/v2/discard.h"
#include "utils.h"
#include <concepts>
#include <optional>
#include <string_view>
#include <type_traits>
namespace glap::v2 {
    namespace help {
        namespace model 
        {
            static constexpr std::string_view INPUTS_NAME = "INPUTS";
            static constexpr int PADDING = 4;
            template <class T>
            concept IsDescription = std::same_as<std::remove_cvref_t<decltype(T::short_description)>, std::string_view>;
            template <class T>
            concept IsFullDescription = IsDescription<T> 
                && std::same_as<std::remove_cvref_t<decltype(T::long_description)>, std::string_view>;
            template <class T>
            concept IsInputs = (T::name == INPUTS_NAME);
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
            template<StringLiteral Name, IsDescription Desc, class ...Params>
            struct Command : Desc {
                static constexpr auto name = Name;
            };
            template<StringLiteral Name, IsDescription Desc, class ...Commands>
            struct Program : Desc
            {};
        }
    }
    template<class FromHelp, class FromParser> 
    struct Help
    {};

    template<class FromHelp, class FromParser>
    static constexpr auto get_help = Help<FromHelp, FromParser>{};
}
#include "help.inl"