#pragma once
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace sjtu {

using sv_t = std::string_view;

struct format_error : std::exception {
public:
    format_error(const char *msg = "invalid format") : msg(msg) {}
    auto what() const noexcept -> const char * override {
        return msg;
    }

private:
    const char *msg;
};

template <typename Tp>
struct formatter;

struct format_info {
    inline static constexpr auto npos = static_cast<std::size_t>(-1);
    std::size_t position; // where is the specifier
    std::size_t consumed; // how many characters consumed
};

template <typename... Args>
struct format_string {
public:
    // must be constructed at compile time, to ensure the format string is valid
    consteval format_string(const char *fmt);
    constexpr auto get_format() const -> std::string_view {
        return fmt_str;
    }
    constexpr auto get_index() const -> std::span<const format_info> {
        return fmt_idx;
    }

private:
    inline static constexpr auto Nm = sizeof...(Args);
    std::string_view fmt_str;            // the format string
    std::array<format_info, Nm> fmt_idx; // where are the specifiers
};

template <typename... Args>
consteval auto compile_time_format_check(sv_t fmt, std::span<format_info> idx) -> void {
    std::size_t n = 0;
    std::size_t pos = 0;

    auto find_next_spec = [&]() -> bool {
        while (pos < fmt.size()) {
            if (fmt[pos] == '%') {
                if (pos + 1 >= fmt.size()) {
                    throw format_error{"missing specifier after '%'"};
                }
                if (fmt[pos + 1] == '%') {
                    pos += 2;  // skip escaped %%
                    continue;
                }
                pos++;  // skip '%'
                return true;
            }
            pos++;
        }
        return false;
    };

    auto parse_fn = [&](const auto &parser) {
        if (!find_next_spec()) {
            idx[n++] = {
                .position = format_info::npos,
                .consumed = 0,
            };
            return;
        }

        const auto consumed = parser.parse(fmt.substr(pos));

        idx[n++] = {
            .position = pos - 1,  // position of '%'
            .consumed = consumed,
        };

        if (consumed > 0) {
            pos += consumed;
        } else if (pos < fmt.size() && fmt[pos] == '_') {
            pos++;
        } else {
            throw format_error{"invalid specifier"};
        }
    };

    (parse_fn(formatter<Args>{}), ...);

    // Check for extra specifiers
    if (find_next_spec()) {
        throw format_error{"too many specifiers"};
    }
}

template <typename... Args>
consteval format_string<Args...>::format_string(const char *fmt) :
    fmt_str(fmt), fmt_idx() {
    compile_time_format_check<Args...>(fmt_str, fmt_idx);
}

// Formatter for string-like types
template <typename StrLike>
    requires(
        std::same_as<StrLike, std::string> ||      //
        std::same_as<StrLike, std::string_view> || //
        std::same_as<StrLike, char *> ||           //
        std::same_as<StrLike, const char *>        //
    )
struct formatter<StrLike> {
    static constexpr auto parse(sv_t fmt) -> std::size_t {
        if (fmt.starts_with("s")) {
            return 1;
        } else if (fmt.starts_with("_")) {
            return 0;  // default format, will be handled separately
        }
        return 0;
    }

    template <typename T>
    static auto format_to(std::ostream &os, const T &val, sv_t fmt = "s") -> void {
        if (fmt.starts_with("s") || fmt.starts_with("_")) {
            os << static_cast<sv_t>(val);
        } else {
            throw format_error{};
        }
    }
};

// Formatter for signed integer types
template <typename Int>
    requires std::signed_integral<Int>
struct formatter<Int> {
    static constexpr auto parse(sv_t fmt) -> std::size_t {
        if (fmt.starts_with("d")) {
            return 1;
        } else if (fmt.starts_with("u")) {
            return 1;
        } else if (fmt.starts_with("_")) {
            return 0;  // default format
        }
        return 0;
    }
    static auto format_to(std::ostream &os, const Int &val, sv_t fmt = "d") -> void {
        if (fmt.starts_with("d") || fmt.starts_with("_")) {
            os << static_cast<int64_t>(val);
        } else if (fmt.starts_with("u")) {
            os << static_cast<uint64_t>(val);
        } else {
            throw format_error{};
        }
    }
};

// Formatter for unsigned integer types
template <typename UInt>
    requires std::unsigned_integral<UInt>
struct formatter<UInt> {
    static constexpr auto parse(sv_t fmt) -> std::size_t {
        if (fmt.starts_with("d")) {
            return 1;
        } else if (fmt.starts_with("u")) {
            return 1;
        } else if (fmt.starts_with("_")) {
            return 0;  // default format
        }
        return 0;
    }
    static auto format_to(std::ostream &os, const UInt &val, sv_t fmt = "u") -> void {
        if (fmt.starts_with("u") || fmt.starts_with("_")) {
            os << static_cast<uint64_t>(val);
        } else if (fmt.starts_with("d")) {
            os << static_cast<int64_t>(val);
        } else {
            throw format_error{};
        }
    }
};

// Formatter for vector types
template <typename T>
struct formatter<std::vector<T>> {
    static constexpr auto parse(sv_t fmt) -> std::size_t {
        if (fmt.starts_with("_")) {
            return 0;
        }
        return 0;
    }
    static auto format_to(std::ostream &os, const std::vector<T> &val, sv_t fmt = "_") -> void {
        os << "[";
        for (size_t i = 0; i < val.size(); ++i) {
            if (i > 0) os << ",";
            formatter<T>::format_to(os, val[i], "_");
        }
        os << "]";
    }
};

template <typename... Args>
using format_string_t = format_string<std::decay_t<Args>...>;

template <typename... Args>
inline auto printf(format_string_t<Args...> fmt, const Args &...args) -> void {
    auto fmt_str = fmt.get_format();
    auto fmt_idx = fmt.get_index();

    // Process the format string
    std::size_t str_pos = 0;
    std::size_t arg_idx = 0;

    auto process_arg = [&]<typename T>(const T &arg) {
        if (arg_idx >= fmt_idx.size()) {
            return;
        }

        const auto &info = fmt_idx[arg_idx];

        // Output characters up to the specifier
        while (str_pos < fmt_str.size()) {
            if (fmt_str[str_pos] == '%') {
                if (str_pos + 1 < fmt_str.size() && fmt_str[str_pos + 1] == '%') {
                    // Escaped %%
                    std::cout << '%';
                    str_pos += 2;
                } else {
                    // This is our specifier
                    str_pos++; // skip '%'
                    sv_t spec = fmt_str.substr(str_pos, info.consumed > 0 ? info.consumed : 1);
                    // Use decay_t to match the formatter
                    using ArgType = std::decay_t<T>;
                    formatter<ArgType>::format_to(std::cout, arg, spec);
                    str_pos += (info.consumed > 0 ? info.consumed : 1);
                    arg_idx++;
                    return;
                }
            } else {
                std::cout << fmt_str[str_pos];
                str_pos++;
            }
        }
        arg_idx++;
    };

    (process_arg(args), ...);

    // Output remaining characters
    while (str_pos < fmt_str.size()) {
        if (fmt_str[str_pos] == '%' && str_pos + 1 < fmt_str.size() && fmt_str[str_pos + 1] == '%') {
            std::cout << '%';
            str_pos += 2;
        } else {
            std::cout << fmt_str[str_pos];
            str_pos++;
        }
    }
}

} // namespace sjtu
