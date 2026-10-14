# Contributing to SimOS

Thank you for helping improve SimOS. Small, focused pull requests are easiest to review.

## Development setup

You need a C++17 compiler and CMake 3.20 or newer.

```bash
git clone https://github.com/jasonzop/simos-project.git
cd simos-project
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

## Guidelines

- Preserve deterministic behavior; simulation results should not depend on timing.
- Add or update a regression test for every behavior change.
- Keep the public API small and document new state transitions.
- Build without warnings on the supported GCC, Clang, and MSVC configurations.
- Follow the repository's `.clang-format` and `.editorconfig` settings.

## Pull requests

In the pull-request description, explain the state transition being changed, why the change is
needed, and how it was tested. All CI checks should pass before review.
