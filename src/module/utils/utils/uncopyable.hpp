/// \file
#pragma once

/// Base class that cannot be moved or copied
/// Useful for inheriting
class Uncopyable {
public:
    Uncopyable() = default;
    Uncopyable(const Uncopyable &) = delete;
    Uncopyable(Uncopyable &&) = delete;

    Uncopyable operator=(const Uncopyable &) = delete;
    Uncopyable operator=(Uncopyable &&) = delete;
};
