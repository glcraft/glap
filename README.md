# glap (2.0) : gly's command line parser

A UTF-8 command line parser written in C++20 respecting conventionals shell arguments.

*Note : as the library is still in development, the API may change in the future.*

## CI
| Platform  | CI                                                                                                                                                         | Prebuilt                                                   | Run tests |
|-----------|------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------|-----------|
| Windows   | [![Windows](https://github.com/glcraft/glap/actions/workflows/Windows.yml/badge.svg)](https://github.com/glcraft/glap/actions/workflows/Windows.yml)       | https://nightly.link/glcraft/glap/workflows/Windows/main   | ❌         |
| Ubuntu    | [![Ubuntu](https://github.com/glcraft/glap/actions/workflows/Ubuntu.yml/badge.svg)](https://github.com/glcraft/glap/actions/workflows/Ubuntu.yml)          | https://nightly.link/glcraft/glap/workflows/Ubuntu/main    | ❌         |
| Archlinux | [![Archlinux](https://github.com/glcraft/glap/actions/workflows/Archlinux.yml/badge.svg)](https://github.com/glcraft/glap/actions/workflows/Archlinux.yml) | https://nightly.link/glcraft/glap/workflows/Archlinux/main | ✔️         |
| Fedora    | [![Fedora](https://github.com/glcraft/glap/actions/workflows/Fedora.yml/badge.svg)](https://github.com/glcraft/glap/actions/workflows/Fedora.yml)          | https://nightly.link/glcraft/glap/workflows/Fedora/main    | ✔️         |
| macOS     | [![macOS](https://github.com/glcraft/glap/actions/workflows/macOS.yml/badge.svg)](https://github.com/glcraft/glap/actions/workflows/macOS.yml)             | https://nightly.link/glcraft/glap/workflows/macOS/main     | ❌         |
| iOS       | [![iOS](https://github.com/glcraft/glap/actions/workflows/iOS.yml/badge.svg)](https://github.com/glcraft/glap/actions/workflows/iOS.yml)                   | https://nightly.link/glcraft/glap/workflows/iOS/main       | ❌         |
| Android   | [![Android](https://github.com/glcraft/glap/actions/workflows/Android.yml/badge.svg)](https://github.com/glcraft/glap/actions/workflows/Android.yml)       | https://nightly.link/glcraft/glap/workflows/Android/main   | ❌         |
| FreeBSD   | [![FreeBSD](https://github.com/glcraft/glap/actions/workflows/FreeBSD.yml/badge.svg)](https://github.com/glcraft/glap/actions/workflows/FreeBSD.yml)       | https://nightly.link/glcraft/glap/workflows/FreeBSD/main   | ❌         |

## Notable features

### Compile time evaluated model

The model to define the command line parser is defined in templates. All definition and verification of the model is evaluated during the compilation.

### Up to date library

The library is written in C++20 with its new language features like concepts, ranges, views... which make the library easier to use and maintain.

### Flexible

The model is easy to update or to personnalize by the user himself.

### Error analysis and validators

Several error types is catch and returned for a better analysis or the arguments. Validators over input and argument values can be set up to check parameter during parsing.

Moreover, the library doesn't use exception but *expected* class in return instead.

### Tested using Google Test

A complete test program is available in "tests" to check the parser. As I'm writing this doc, all of the 50 tests passed successfully !

### UTF-8 text

The library works with UTF-8 text. So if you want to use emoji or non latin alphabet in long or short name, you can !

## Documentation

The whole documentation is available at [docs](docs) folder, let's check it out !

## TODO

- [ ] Help generator
  - [ ] Base
  - [ ] Colored text
  - [ ] Locales
- [ ] Executor (handle invokable objects to execute)
- [ ] Subcommands
- [ ] constexpr hashmap for  long names