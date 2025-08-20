#pragma once
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>

namespace test_framework {
    struct TestCase {
        std::string name;
        std::function<void()> func;
    };

    inline std::vector<TestCase>& registry() {
        static std::vector<TestCase> tests;
        return tests;
    }

    inline void register_test(const std::string& name, std::function<void()> func) {
        registry().push_back({name, std::move(func)});
    }

    inline int run_all() {
        int failures = 0;
        for (auto& t : registry()) {
            try {
                t.func();
                std::cout << "[PASS] " << t.name << "\n";
            } catch (const std::exception& e) {
                std::cout << "[FAIL] " << t.name << ": " << e.what() << "\n";
                failures++;
            }
        }
        std::cout << registry().size() - failures << " tests passed, " << failures << " failed."
                  << std::endl;
        return failures;
    }

    inline std::string location(const char* file, int line) {
        std::ostringstream oss;
        oss << file << ":" << line << " ";
        return oss.str();
    }

    template <typename A, typename B>
    void expect_eq(const A& a, const B& b, const char* as, const char* bs, const char* file,
                   int line) {
        if (!(a == b)) {
            std::ostringstream oss;
            oss << location(file, line) << "Expected " << as << " == " << bs;
            throw std::runtime_error(oss.str());
        }
    }

    template <typename A, typename B>
    void expect_ne(const A& a, const B& b, const char* as, const char* bs, const char* file,
                   int line) {
        if (!(a != b)) {
            std::ostringstream oss;
            oss << location(file, line) << "Expected " << as << " != " << bs;
            throw std::runtime_error(oss.str());
        }
    }

    template <typename A, typename B>
    void expect_gt(const A& a, const B& b, const char* as, const char* bs, const char* file,
                   int line) {
        if (!(a > b)) {
            std::ostringstream oss;
            oss << location(file, line) << "Expected " << as << " > " << bs;
            throw std::runtime_error(oss.str());
        }
    }

    template <typename A, typename B>
    void expect_ge(const A& a, const B& b, const char* as, const char* bs, const char* file,
                   int line) {
        if (!(a >= b)) {
            std::ostringstream oss;
            oss << location(file, line) << "Expected " << as << " >= " << bs;
            throw std::runtime_error(oss.str());
        }
    }

    inline void expect_true(bool cond, const char* expr, const char* file, int line) {
        if (!cond) {
            std::ostringstream oss;
            oss << location(file, line) << "Expected " << expr;
            throw std::runtime_error(oss.str());
        }
    }

    inline void expect_false(bool cond, const char* expr, const char* file, int line) {
        if (cond) {
            std::ostringstream oss;
            oss << location(file, line) << "Expected !(" << expr << ")";
            throw std::runtime_error(oss.str());
        }
    }

    inline void expect_no_throw(const std::function<void()>& fn, const char* expr, const char* file,
                                int line) {
        try {
            fn();
        } catch (const std::exception& e) {
            std::ostringstream oss;
            oss << location(file, line) << "Unexpected exception in " << expr << ": " << e.what();
            throw std::runtime_error(oss.str());
        } catch (...) {
            std::ostringstream oss;
            oss << location(file, line) << "Unexpected non-std exception in " << expr;
            throw std::runtime_error(oss.str());
        }
    }

    template <typename E, typename F>
    inline void expect_throw(F&& fn, const char* expr, const char* file, int line) {
        bool caught = false;
        try {
            fn();
        } catch (const E&) {
            caught = true;
        } catch (...) {
        }
        if (!caught) {
            std::ostringstream oss;
            oss << location(file, line) << "Expected " << expr << " to throw " << typeid(E).name();
            throw std::runtime_error(oss.str());
        }
    }
}

#define TEST(suite, name)                                                \
    static void suite##_##name();                                        \
    static bool suite##_##name##_registered = []() {                     \
        test_framework::register_test(#suite "." #name, suite##_##name); \
        return true;                                                     \
    }();                                                                 \
    static void suite##_##name()

#define EXPECT_EQ(a, b)                                                \
    do {                                                               \
        const auto& _a = (a);                                          \
        const auto& _b = (b);                                          \
        test_framework::expect_eq(_a, _b, #a, #b, __FILE__, __LINE__); \
    } while (0)
#define ASSERT_EQ EXPECT_EQ
#define EXPECT_NE(a, b)                                                \
    do {                                                               \
        const auto& _a = (a);                                          \
        const auto& _b = (b);                                          \
        test_framework::expect_ne(_a, _b, #a, #b, __FILE__, __LINE__); \
    } while (0)
#define ASSERT_NE EXPECT_NE
#define EXPECT_GT(a, b)                                                \
    do {                                                               \
        const auto& _a = (a);                                          \
        const auto& _b = (b);                                          \
        test_framework::expect_gt(_a, _b, #a, #b, __FILE__, __LINE__); \
    } while (0)
#define EXPECT_GE(a, b)                                                \
    do {                                                               \
        const auto& _a = (a);                                          \
        const auto& _b = (b);                                          \
        test_framework::expect_ge(_a, _b, #a, #b, __FILE__, __LINE__); \
    } while (0)
#define ASSERT_GE EXPECT_GE
#define EXPECT_TRUE(cond) test_framework::expect_true((cond), #cond, __FILE__, __LINE__)
#define EXPECT_FALSE(cond) test_framework::expect_false((cond), #cond, __FILE__, __LINE__)
#define ASSERT_TRUE EXPECT_TRUE
#define ASSERT_FALSE EXPECT_FALSE
#define EXPECT_NO_THROW(expr) \
    test_framework::expect_no_throw([&]() { expr; }, #expr, __FILE__, __LINE__)
#define EXPECT_THROW(expr, exc) \
    test_framework::expect_throw<exc>([&]() { expr; }, #expr, __FILE__, __LINE__)