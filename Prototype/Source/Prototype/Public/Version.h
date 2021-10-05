#pragma once

/*
template <class Class>
struct ClassVersion
{
    static const unsigned sMajor;
    static const unsigned sMinor;
};


#define DECLARE_CLASS_VERSION(type, major, minor) template <class Class> struct ClassVersion{ const unsigned type##_VERSION_MAJOR = major; const unsigned type##_VERSION_MINOR = minor;
#define CHECK_CLASS_VERSION(type, major, minor) static_assert(type##_VERSION_MAJOR == major && type##_VERSION_MINOR <= minor, #type ## " version " ## #major "." ## #minor ## " is out of date.");
#define CLASS_VERSION_MAJOR(type) type##_VERSION_MAJOR
#define CLASS_VERSION_MINOR(type) type##_VERSION_MINOR
*/


#define DECLARE_CLASS_VERSION(major, minor) public: static constexpr unsigned VERSION_MAJOR = major; static constexpr unsigned VERSION_MINOR = minor;
#define CHECK_CLASS_VERSION(type, major, minor) static_assert(type::VERSION_MAJOR == major && type::VERSION_MINOR <= minor, #type ## " version " ## #major "." ## #minor ## " is out of date.");
#define CHECK_CLASS_VERSION_EQUAL(type, major, minor) static_assert(type::VERSION_MAJOR == major && type::VERSION_MINOR == minor, #type ## " version " ## #major "." ## #minor ## " is not equal.");
#define CLASS_VERSION_MAJOR(type) type::VERSION_MAJOR
#define CLASS_VERSION_MINOR(type) type::VERSION_MINOR
