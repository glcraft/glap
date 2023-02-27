# Utils

## Names

### Definition

```cpp
/// In namespace glap
template <StringLiteral Name, auto ShortName = discard>
struct Names {
    static constexpr std::string_view name = Name;
    static constexpr std::optional<char32_t> shortname = impl::Optional<char32_t, ShortName>;
};
```

### Description

This class is used to define argument long name and short name in the model.

Long name is required, whereas short name is optional.

Long name is a utf8 string, write the string character in the template.
Short name on the other hand is a unicode code point (or [`discard`](#discard)). This means it expects a number, not a 
string. Use `''` annotation to define the character easily. 

You can define one and only unicode code point in a short name. Composed code points like multiple accented character 
(for example [*glitched*](https://lingojam.com/GlitchTextGenerator) strings) or color specified emojis 
(for example 👩🏿🧓🏻) won't work.

## Handle errors

### Definition

```cpp
/// in namespace glap
struct Error {
    std::string_view parameter;
    std::optional<std::string_view> value;
    enum class Type {
        Command,
        Parameter,
        Flag,
        Input,
        None,
        Unknown
    } type;
    enum class Code {
        NoParameter,
        NoGlobalCommand,
        BadCommand,
        UnknownArgument,
        BadResolution,
        BadValidation,
        DuplicateParameter,
        TooManyParameters,
        MissingValue,
        SyntaxError,
        BadString
    } code;

    std::string to_string() const;
};
template <class T>
using Expected = glap::expected<T, Error>;
struct PositionnedError {
    using difference_type = decltype(std::distance(std::span<std::string>().begin(), std::span<std::string>().end()));
    Error error;
    difference_type position;
    auto to_string() const;
};
template<class T>
using PosExpected = expected<T, PositionnedError>;
```

### Description

The library does not rely on C++ exceptions. Instead, `tl::expected` (or `std::expected` 
[if configured](README.md#xmake-configuration)) is used. If no issues is raised from the parser, the expected value 
embedded in expected is returned. Otherwise, an `PositionnedError` is returned.

In `PositionnedError`, there is the argument position and the details of the error: 
* `Error::Type`: which kind of argument was parsing
* `Error::Code`: the kind of error

I advise you to read [the C++ documentation about `std::expected`](https://en.cppreference.com/w/cpp/utility/expected)
to understand how to work with expected if you're not familiar with.

## Discard

### Definition

```cpp
/// In namespace glap
struct Discard {};
static constexpr Discard discard = {};
```

### Description

The type `Discard` and value `discard` is used to apply the default behaviour or to discard something in templates.
For example, Resolvers and Validators may be discarded so no resolution nor validations is processed.

## Value

### Definition

```cpp
template <auto Resolver = discard, auto Validator = discard>
struct Value {
    using value_type = typename impl::ResolverReturnType<decltype(Resolver)>::type;
    static constexpr auto resolver = Resolver;
    static constexpr auto validator = Validator;
    std::optional<value_type> value;
};
```

### Description

This is the base model to capture the value of [Input](#single-parameter-argument) and 
[Parameter](#single-parameter-argument).

`Resolver` is either a function or [*discard*](#discard). Here is The list of signature expected :
* `(std::string_view) -> T` where T is whatever type.
* `(std::string_view) -> tl::expected<T, Discard>` where T is whatever type. If a Discard error is raised, it is 
intercepted and raised by the parser.

The type of `value` is based on the output of the `Resolver`.

`Validator` is either a function or [*discard*](#discard). The signature expected is `(std::string_view) -> bool`. When
`Validator` returns false, an error is raised by the parser.

By default, there is no value. It accepts only one value. It means if a value is set whereas a value is already setted, 
an error is raised by the parser.

## Container

### Definition

```cpp
/// In namespace glap
template <class T, auto N = discard>
class Container {
public: 
    using value_type = T;
private:
    using n_type = std::remove_cvref_t<decltype(N)>;
    static constexpr auto is_n_discard = std::is_same_v<n_type, Discard>;
    static constexpr auto is_n_zero = impl::is_equal_v<N, 0>;
    using dynamic_vector = std::vector<value_type>;
    using stack_vector = StackVector<value_type, impl::value_or_v<N, 0>>;
public:
    using container_type = std::conditional_t<is_n_discard || is_n_zero, dynamic_vector, stack_vector>;
    container_type values;

    /// Returns the size of the container
    /// The size is the number of argument stored in the container (so not the capacity)
    [[nodiscard]]constexpr auto size() const noexcept;
    /// Get const value at index `i`
    [[nodiscard]]constexpr const auto& get(size_t i) const /*noexcept is stack vector*/;
    /// Get value at index `i`
    [[nodiscard]]constexpr auto& get(size_t i) /*noexcept is stack vector*/;
    /// Get const value at index `I`
    /// In case container_type is a stack vector, I is constrained about N. 
    template <size_t I>
    [[nodiscard]]constexpr const auto& get() const /*noexcept is stack vector*/;
    /// Get value at index `I`
    /// In case container_type is a stack vector, I is constrained about N. 
    template <size_t I>
    [[nodiscard]]constexpr auto& get() /*noexcept is stack vector*/;
    /// Get value at index `i`
    [[nodiscard]]constexpr auto& operator[](size_t i) /*noexcept is stack vector*/;
    /// Get const value at index `i`
    [[nodiscard]]constexpr const auto& operator[](size_t i) const /*noexcept is stack vector*/;
};
```
### Description

This is the base model to capture one or several values of [Inputs](#single-parameter-argument) and 
[Parameters](#single-parameter-argument).

`N` is either a integer or [*discard*]. 
If N is 0 or [*discard*], the container type will be a dynamic vector (aka
`std::vector<T>`). 
Otherwise, it will be a stack vector, which is a custom class acting like a vector but with a fixed size array on the stack.

The type `T` refer directly to [`Value`], so check out [`Value`] type for more explanation about how to 
work with values inside the container.

`get` and `operator[]` retreive value inside of the [`Value`] type. so they will return an `std::optional<X>` wxhere X 
is the type inferred from Resolver or std::string_view if no Resolver is specified.

[*discard*]: #discard
[`Value`]: #value