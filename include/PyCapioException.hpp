#ifndef PYCAPIO_EXCEPTION_HPP
#define PYCAPIO_EXCEPTION_HPP

#include <exception>
#include <string>

/**
 * @brief Exception type raised by the PyCAPIO native layer.
 *
 * Carries an error message and, when constructed, captures the current Python
 * call stack to make failures that originate inside intercepted I/O easier to
 * diagnose. Surfaced to Python as @c PyCAPIOException.
 */
class PyCapioException : public std::exception {
    std::string _message;

    /// @brief Capture a textual snapshot of the active Python call stack.
    static std::string capture_python_stack();

public:
    /// @brief Construct with a human-readable error message.
    /// @param error_message Description of what went wrong.
    explicit PyCapioException(const std::string &error_message);

    /// @brief Return the stored error message.
    [[nodiscard]] const char *what() const noexcept override;
};

#endif // PYCAPIO_EXCEPTION_HPP
