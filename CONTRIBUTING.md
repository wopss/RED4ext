# Contributing

You can contribuite to RED4ext with issues, ideas and PRs. Simply filling issues or propose new ideas is a great way to contribute.

## Content guidelines

The issue tracker is exclusively for filling and discussing bugs and featurerequests, it is not for scripting support (e.g. `How do I get X handler?`, `How can I call X function?`, etc.). For any other kind of discussion use [discussions](https://github.com/WopsS/RED4ext/discussions).

### Reporting a bug or requesting a feature

Before filing a bug report or requesting a new features ensure the bug reproduces on the latest version of the library and search existing issues and make sure this issue is not already filed.

## Code guidelines

### Language

The project is using [C++20](https://en.cppreference.com/w/cpp/20), any C++20 features are allowed.

### General

* Use [EditorConfig](https://editorconfig.org/) and [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to style your code before pushing.
  * Ideally, set up pre-commit hook using [pre-commit](https://pre-commit.com).
    All that should be required to set it up is to have [Python 3.x](https://www.python.org/) with [pip](https://pypi.org/project/pip/) installed and run the following command inside of the cloned repository folder: `pip install pre-commit && pre-commit install`
    * Note that first commit after setting up the hook will take longer as it has to download required packages.
    * You don't have to run it every single time there is some update, hook is auto-updated after initial installation.
* Use names that describe the purpose or intent of the object. Names should be self-explanatory and easily recognizable in the context.
* Minimize the use of abbreviations that would likely be unknown to someone outside of the project.
* Use `auto` whenever it is possible, if the intended type is the correct one (in case of primitive types).
* Brackets (`{}`) must be on new line.

### Namespaces

* Namespaces must be `PascalCase`.

### Classes

* Class names must be `PascalCase`.
* Use `class` and `struct` acordingly.
  * A rule of thumb is to use `struct` for reversed types or objects that carry data, for everything else use `class`.

### Functions

* Function names should be `PascalCase`.

### Variables

* Class data members should be prefixed with `m_`.
* Function arguments should be prefixed with an `a` (**a**rgument).
* Local variables should be `camelCase`.
* Structs data members are named like ordinary variables.

### Enumerators

* Enumerators names must be `PascalCase`.
* Always use `enum class` over `enum`.

### Macros

* Marcos should named with uppercase letters.
* Prefix macros with `RED4EXT_`.

## Commit messages

Please try to format commit messages as follows (based on [A Note About Git Commit Messages](http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html)):

```text
Short (50 characters or less) summary

Provide a more detailed (72 characters or so) text after the first line,
if necessary, leave one blank line below the summary.

If the commit fixes an issue, leave another blank line after the final paragraph
and indicate which issue is fixed.

Fix #1
```

## License

The project is licensed under the MIT license detailed in [LICENSE.md](/LICENSE.md) file. All incoming code is subject to the project's [LICENSE](/LICENSE.md).

**Do not** use code that is not yours or is not compatible with the project's [LICENSE](/LICENSE.md).
