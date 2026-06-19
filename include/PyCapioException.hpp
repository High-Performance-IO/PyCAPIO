#ifndef PYCAPIO_EXCEPTION_HPP
#define PYCAPIO_EXCEPTION_HPP

#include <exception>
#include <string>

class PyCapioException : public std::exception {
    std::string _message;

    static std::string capture_python_stack();

  public:
    explicit PyCapioException(const std::string &error_message);

    [[nodiscard]] const char *what() const noexcept override;
};

#endif // PYCAPIO_EXCEPTION_HPP