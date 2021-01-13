#include <ksn/ksn.hpp>
#include <type_traits>
#include <stddef.h>

_KSN_BEGIN





//_KSN_DETAIL_BEGIN



//_KSN_DETAIL_END


//
//template<size_t N, size_t c, class = void>
//struct _counter_t_helper3_well_checker;

template<size_t N, size_t c>
struct _counter_t_helper3_well_checker_t
{
	template<class T>
	constexpr static bool foo();

	template<>
	constexpr static bool foo<void>();
};

template<size_t N, size_t c>
struct counter_t
{
	/*
	if "detail::_counter_t_foo(my_t())" is well-formed
		return counter_t<N, c+1>::value
	
	instantiate detail::_counter_t_helper1<N, c>
	return c;
	*/

	//template<>
	//friend void detail::_counter_t_foo(counter_t<N, c + 1>);

	//size_t value = std::enable_if_t<, std::integral_constant<size_t, c>>::value;

	constexpr static size_t value = _counter_t_helper2_well_checker_result<N, c, 
		_counter_t_helper3_well_checker_t<N, c>::foo<void>()
	>::value;
};



template<size_t N, size_t c>
struct _counter_t2 {};

template<size_t N, size_t c>
constexpr void _counter_t_foo(_counter_t2<N, c>);

//_KSN_DETAIL_BEGIN




template<size_t N, size_t c>
struct _counter_t_helper1_def_injecter
{
	friend constexpr void _counter_t_foo(_counter_t2<N, c>) {};
};

template<size_t N, size_t c, bool well>
struct _counter_t_helper2_well_checker_result
{
	static constexpr size_t value = counter_t<N, c + 1>::value;
};

template<size_t N, size_t c>
struct _counter_t_helper2_well_checker_result<N, c, false>
{
	template<size_t _c, size_t>
	struct _helper
	{
		static constexpr size_t value = _c;
	};
	//Instantiate _counter_t_helper1<N, c> thus defining _counter_t_foo<N, c>
	//and return c
	static constexpr size_t value = _helper<c, sizeof(_counter_t_helper1_def_injecter<N, c>)>::value;
};


//template<size_t N, size_t c, class>
//struct _counter_t_helper3_well_checker
//{
//	static constexpr bool value = std::false_type::value;
//};
//
//template<size_t N, size_t c>
//struct _counter_t_helper3_well_checker<N, c, void>
//{
//	static constexpr bool value = std::enable_if_t<
//		std::is_same_v<void, std::void_t<decltype(  _counter_t_foo(counter_t<N, c>())  )>>
//		, std::true_type>::value;
//};


template<size_t N, size_t c>
template<class>
constexpr bool _counter_t_helper3_well_checker_t::foo()
{
	return false;
}

template<size_t N, size_t c>
template<>
constexpr bool _counter_t_helper3_well_checker_t::foo<void>()
{
	decltype(_counter_t_foo(_counter_t2<N, c>()));

	return true;
}




//_KSN_DETAIL_END



template<size_t N>
constexpr size_t count()
{
	return counter_t<N, 0>::value;
}





_KSN_END



int main()
{

	constexpr size_t a = ksn::count<0>();
	
}

/*
Constexpr counter
To be remade using concepts
*/