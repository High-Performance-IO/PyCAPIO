/**
 * @file libsyscall_intercept_hook_point.h
 * @brief Stand-in for the syscall_intercept header, used when building PyCAPIO.
 *
 * Since the CALF logger refactor (CAPIO#239) the CAPIO POSIX headers are only meant to be
 * compiled with @c __CAPIO_POSIX defined, which includes <libsyscall_intercept_hook_point.h>.
 * PyCAPIO calls the POSIX handlers directly and never hooks system calls, so it does not link
 * syscall_intercept: this header provides the two symbols the handlers use, with the same
 * semantics as the real library.
 */
#ifndef PYCAPIO_COMPAT_LIBSYSCALL_INTERCEPT_HOOK_POINT_H
#define PYCAPIO_COMPAT_LIBSYSCALL_INTERCEPT_HOOK_POINT_H

#include <cerrno>
#include <type_traits>

#include <sys/syscall.h>
#include <unistd.h>

namespace pycapio::compat {

template <typename T> inline long syscall_arg(T value) {
    if constexpr (std::is_null_pointer_v<T>) {
        return 0;
    } else if constexpr (std::is_pointer_v<T>) {
        return reinterpret_cast<long>(value);
    } else {
        return static_cast<long>(value);
    }
}

/**
 * @brief Issue a raw system call.
 *
 * Like syscall_intercept's syscall_no_intercept(), and unlike glibc's syscall(), failures are
 * reported by returning the negated error code (-errno) instead of -1.
 */
template <typename... Args> inline long syscall_no_intercept(long syscall_number, Args... args) {
    const long result = ::syscall(syscall_number, syscall_arg(args)...);
    return result == -1 ? -errno : result;
}

} // namespace pycapio::compat

#define syscall_no_intercept pycapio::compat::syscall_no_intercept

/// @brief Error code of a raw system call result, or 0 if the call succeeded.
inline int syscall_error_code(const long result) {
    return (result < 0 && result >= -4095) ? static_cast<int>(-result) : 0;
}

#endif // PYCAPIO_COMPAT_LIBSYSCALL_INTERCEPT_HOOK_POINT_H
