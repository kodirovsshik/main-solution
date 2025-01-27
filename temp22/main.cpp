
#include <string_view>
#include <type_traits>
#include <utility>
#include <string>

template<typename T>
struct is_const_char_array : std::false_type {};

template<size_t N>
struct is_const_char_array<const char[N]> : std::true_type {};

template<typename T>
inline constexpr bool IsCharArray_v = is_const_char_array<T>::value;

template<typename Arg>
auto process_arg(Arg&& arg)
{
    if constexpr (IsCharArray_v<std::remove_reference_t<Arg>>)
    {
        constexpr size_t array_size = sizeof(Arg) / sizeof(char);
        return std::string_view(arg, array_size - 1);
    }
    else {
        return std::string_view(std::forward<Arg>(arg));
    }
}

template<class... Args> requires(std::is_same_v<Args, std::string_view> && ...)
void foo2(Args ...ma_strings)
{

}

template<std::convertible_to<std::string_view>... Args>
void foo(Args&&... args) 
{
    foo2(process_arg(std::forward<Args>(args))...);
}


int main()
{
    const char* a = "a";
    std::string b = "b";

    foo(a, b, "Hi");
}