#include <cstdint>
#include <iostream>

#include "basic.hpp"
#include "errno.hpp"

int main()
{
    const char *const text = "Command & Conquer";
    if (ft_strlen_size_t(text) != static_cast<ft_size_t>(17U))
        return 1;

    ft_size_t product = 0U;
    const int32_t multiply_error = ft_size_multiply_checked(
        static_cast<ft_size_t>(6U), static_cast<ft_size_t>(7U), &product);
    if (multiply_error != FT_ERR_SUCCESS || product != static_cast<ft_size_t>(42U))
        return 2;

    std::cout << "libft smoke ok (" << CNC_PROJECT_VERSION << ")\n";
    return 0;
}
