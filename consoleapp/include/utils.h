#pragma once
#include <iterator>
#include <string_view>
#include "expected.h"
namespace utils {
    namespace uni
    {
        struct UnicodeError {
            std::string_view str;
            size_t pos;
            enum class Error {
                InvalidUtf8Char,
            } error;
        };
        template <class T>
        using Expected = expected<T, UnicodeError>;
        [[nodiscard]] constexpr Expected<uint8_t> utf8_char_length(std::string_view str) noexcept {
            auto c = str.front();
            if ((c & 0x80) == 0) {
                return 1;
            } else if ((c & 0xE0) == 0xC0) {
                return 2;
            } else if ((c & 0xF0) == 0xE0) {
                return 3;
            } else if ((c & 0xF8) == 0xF0) {
                return 4;
            } else {
                return unexpected<UnicodeError>(UnicodeError{
                    .str = str,
                    .pos = 0,
                    .error = UnicodeError::Error::InvalidUtf8Char
                });
            }
        }
        [[nodiscard]] constexpr Expected<size_t> utf8_length(std::string_view str) noexcept {
            uint8_t len = 0;
            for (auto itChar = str.begin(); itChar != str.end();) {
                auto c = *itChar;
                auto char_len = utf8_char_length(std::string_view{itChar, 1});
                if (!char_len) {
                    auto error = std::move(char_len.error());
                    error.pos = std::distance(str.begin(), itChar);
                    return unexpected<UnicodeError>(std::move(error));
                }
                len += 1;
                itChar += char_len.value();
            }
            return len;
        }
        [[nodiscard]] constexpr Expected<uint8_t> codepoint(std::string_view utf8) noexcept {
            char32_t codepoint = 0;
            auto it = utf8.begin();
            if ((*it & 0x80) == 0) {
                codepoint = *it;
            } else if ((*it & 0xE0) == 0xC0) {
                codepoint = (*it & 0x1F) << 6;
                codepoint |= *(++it) & 0x3F;
            } else if ((*it & 0xF0) == 0xE0) {
                codepoint = (*it & 0x0F) << 12;
                codepoint |= (*(++it) & 0x3F) << 6;
                codepoint |= *(++it) & 0x3F;
            } else if ((*it & 0xF8) == 0xF0) {
                codepoint = (*it & 0x07) << 18;
                codepoint |= (*(++it) & 0x3F) << 12;
                codepoint |= (*(++it) & 0x3F) << 6;
                codepoint |= *(++it) & 0x3F;
            } else {
                return unexpected<UnicodeError>(UnicodeError{
                    .str = utf8,
                    .pos = 0,
                    .error = UnicodeError::Error::InvalidUtf8Char
                });
            }
            return codepoint;
        }
    }
}