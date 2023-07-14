#pragma once

#include <type_traits>
#include <tuple>

template<typename Class, typename Ret, typename... Args>
constexpr auto function_pointer(Ret (Class::*)(Args...)) -> Ret (*)(Args...);

template<typename Class, typename Ret, typename... Args>
constexpr auto function_pointer(Ret (Class::*)(Args...) const) -> Ret (*)(Args...);

template<auto Func>
using function_signature = std::remove_pointer_t<decltype(function_pointer(Func))>;

template<typename T>
struct function_traits;

template<typename R, typename ...Args>
struct function_traits<R(Args...)>
{
	static constexpr size_t args_size = sizeof...(Args);

	using result_type = R;

	template <size_t i>
	struct arg
	{
		static_assert(i >= 0 && i < args_size, "Invalid range!");
		using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
	};
};

