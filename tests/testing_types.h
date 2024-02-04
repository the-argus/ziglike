#pragma once
#include <cstdint>
#include <utility>

enum class StatusCodeA : uint8_t
{
    Okay,
    ResultReleased,
    Whatever,
    OOMIGuess,
    BadAccess,
};

enum class StatusCodeB : uint8_t
{
    Okay = 0,
    ResultReleased,
    Nothing = 250,
    MoreNothing = 100,
};

struct trivial_t
{
    int whatever;
    const char *nothing;
};

struct moveable_t
{
    int whatever;
    char *nothing;

    moveable_t() { nothing = new char[150]; }
    ~moveable_t() { delete[] nothing; };

    // no copying
    moveable_t &operator=(const moveable_t &other) = delete;
    moveable_t(const moveable_t &other) = delete;

    // yay moving
    moveable_t &operator=(moveable_t &&other)
    {
        nothing = other.nothing;
        whatever = other.whatever;
        other.nothing = nullptr;
        other.whatever = 0;
        return *this;
    }
    moveable_t(moveable_t &&other) { *this = std::move(other); }
};

struct nonmoveable_t
{
    int whatever;
    const char *nothing;

    nonmoveable_t() { nothing = new char[150]; }
    ~nonmoveable_t() { delete[] nothing; };

    // trivially copyable
    nonmoveable_t &operator=(const nonmoveable_t &other) = default;
    nonmoveable_t(const nonmoveable_t &other) = default;

    // yay moving
    nonmoveable_t &operator=(nonmoveable_t &&other) = delete;
    nonmoveable_t(nonmoveable_t &&other) = delete;
};
