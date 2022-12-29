#pragma once

#ifndef GLAP_MODULE
#include "core/base.h"
#include "core/discard.h"
#include "core/utils.h"
#include <concepts>
#include <optional>
#include <string_view>
#include <type_traits>
#endif

GLAP_EXPORT namespace glap {
    namespace help {
        inline constexpr std::string_view INPUTS_NAME = "INPUTS";
        inline constexpr int PADDING = 4;
        template <class T>
        concept IsDescription = std::same_as<std::remove_cvref_t<decltype(T::short_description)>, std::string_view>;
        template <class T>
        concept IsFullDescription = IsDescription<T> 
            && std::same_as<std::remove_cvref_t<decltype(T::long_description)>, std::string_view>;
        template <class T>
        concept IsInputs = (T::name == INPUTS_NAME);
        namespace model 
        {
            template<StringLiteral Short>
            struct Description {
                static constexpr std::string_view short_description = Short;
            };
            template<StringLiteral Short, StringLiteral Long>
            struct FullDescription : Description<Short> {
                static constexpr std::string_view long_description = Long;
            };

            template<StringLiteral Name, IsDescription Desc> 
            struct Argument : Desc {
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
    inline constexpr auto get_help = Help<FromHelp, FromParser>{};
}

#ifndef GLAP_MODULE
#include "impl/help.inl"
#endif