
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
//#include <numeric>
//#include <set>
//#include <unordered_set>
//#include <map>
//#include <unordered_map>

using namespace std;

static const auto _____ = []()
{
	// fast IO
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	cout.tie(nullptr);
	return 0;
}();

namespace detail
{
	template<typename T, typename F, int... Is>
	void for_each(T&& t, F f, std::integer_sequence<int, Is...>)
	{
		(void)std::initializer_list<uint8_t>{ (f(std::get<Is>(t)), uint8_t())... };
	}
	template<typename T1, typename T2, typename F, int... Is>
	void for_each2(T1&& t1, T2&& t2, F&& f, std::integer_sequence<int, Is...>)
	{
		(void)std::initializer_list<uint8_t>{ (f(std::get<Is>(t1), std::get<Is>(t2)), uint8_t())... };
	}

	struct incrementer
	{
		template<class Iter>
		void operator()(Iter& iter) noexcept
		{
			++iter;
		}
	};
	struct decrementer
	{
		template<class Iter>
		void operator()(Iter& iter) noexcept
		{
			--iter;
		}
	};
	struct adder
	{
		ptrdiff_t n;
		adder(ptrdiff_t _n) : n(_n) {}
		template<class Iter>
		void operator()(Iter& iter)
		{
			iter += n;
		}
	};
	struct swapper
	{
		template<class I1, class I2>
		void operator()(I1 a, I2 b)
		{
			using namespace std;
			iter_swap(a, b);
		}
	};

	template<typename... Ts, typename F>
	void for_each_in_const_tuple(const std::tuple<Ts...>& t, F f)
	{
		detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
	}
	template<typename... Ts, typename F>
	void for_each_in_tuple(std::tuple<Ts...>& t, F f)
	{
		detail::for_each(t, f, std::make_integer_sequence<int, sizeof...(Ts)>());
	}
	template<typename... Ts, typename F>
	void for_each_in_const_tuple2(const std::tuple<Ts...>& t1, const std::tuple<Ts...>& t2, F&& f)
	{
		detail::for_each2(t1, t2, std::forward<F>(f), std::make_integer_sequence<int, sizeof...(Ts)>());
	}

}


template<class tag = void>
struct map_iterator_tag_to_number {};
template<>
struct map_iterator_tag_to_number<std::input_iterator_tag>
{
	static constexpr size_t value = 1;
};
template<>
struct map_iterator_tag_to_number<std::output_iterator_tag>
{
	static constexpr size_t value = 2;
};
template<>
struct map_iterator_tag_to_number<std::forward_iterator_tag>
{
	static constexpr size_t value = 3;
};
template<>
struct map_iterator_tag_to_number<std::bidirectional_iterator_tag>
{
	static constexpr size_t value = 4;
};
template<>
struct map_iterator_tag_to_number<std::random_access_iterator_tag>
{
	static constexpr size_t value = 5;
};

template<size_t N = 0>
struct map_number_to_iterator_tag {};
template<>
struct map_number_to_iterator_tag<1> 
{
	using type = std::input_iterator_tag;
};
template<>
struct map_number_to_iterator_tag<2> 
{
	using type = std::output_iterator_tag;
};
template<>
struct map_number_to_iterator_tag<3> 
{
	using type = std::forward_iterator_tag;
};
template<>
struct map_number_to_iterator_tag<4> 
{
	using type = std::bidirectional_iterator_tag;
};
template<>
struct map_number_to_iterator_tag<5> 
{
	using type = std::random_access_iterator_tag;
};

template<class T, T First, T... Rest>
struct min_value
{
	static constexpr T value = std::min<T>(First, min_value<T, Rest...>::value);
};
template<class T, T First>
struct min_value<T, First>
{
	static constexpr T value = First;
};

template<class T, T... Args>
static constexpr T min_value_v = min_value<T, Args...>::value;



template<class... Iters>
class associated_iterator_reference
{
	using storage_t = std::tuple<Iters...>;
	storage_t m_iters;

	using my_t = associated_iterator_reference<Iters...>;
	
	template<class... Iters>
	friend class associated_iterator;

	associated_iterator_reference(const storage_t& iters)
		: m_iters(iters) {}

public:
	template<size_t n>
	auto&& get() const
	{
		return *std::get<n>(this->m_iters);
	}

	template<class ...XIters>
	friend void swap(const associated_iterator_reference<XIters...>& a, const associated_iterator_reference<XIters...>& b)
	{
		detail::for_each_in_const_tuple2(a.m_iters, b.m_iters, detail::swapper());
	}
};

template<class... Iters>
class associated_iterator
{
	using storage_t = std::tuple<Iters...>;
	storage_t m_iters;

	static constexpr size_t weakest_iterator_category = 
		min_value_v<size_t, 
			map_iterator_tag_to_number<typename std::iterator_traits<Iters>::iterator_category>::value...
		>;

	static constexpr size_t input_iterator_category = map_iterator_tag_to_number<std::input_iterator_tag>::value;
	static constexpr size_t output_iterator_category = map_iterator_tag_to_number<std::output_iterator_tag>::value;
	static constexpr size_t forward_iterator_category = map_iterator_tag_to_number<std::forward_iterator_tag>::value;
	static constexpr size_t bidirectional_iterator_category = map_iterator_tag_to_number<std::bidirectional_iterator_tag>::value;
	static constexpr size_t random_access_iterator_category = map_iterator_tag_to_number<std::random_access_iterator_tag>::value;

	static constexpr bool is_nothrow_copy_constructible = std::conjunction_v<std::bool_constant<std::is_nothrow_copy_constructible_v<Iters>>...>;
	static constexpr bool is_nothrow_move_constructible = std::conjunction_v<std::bool_constant<std::is_nothrow_move_constructible_v<Iters>>...>;

public:

	using difference_type = ptrdiff_t;
	using value_type = associated_iterator_reference<Iters...>;
	using pointer = void;
	using reference = associated_iterator_reference<Iters...>;
	using iterator_category = typename map_number_to_iterator_tag<weakest_iterator_category>::type;

	associated_iterator(const storage_t& iters) noexcept(is_nothrow_copy_constructible)
		: m_iters(iters) {}
	associated_iterator(storage_t&& iters) noexcept(is_nothrow_move_constructible)
		: m_iters(std::forward<storage_t>(iters)) {}
	
	template<class _SFINAE_all_types_must_be_default_constructible = 
		std::enable_if_t<std::conjunction_v<std::bool_constant<std::is_default_constructible_v<Iters>>...>>>
	associated_iterator() noexcept(std::conjunction_v<std::bool_constant<std::is_nothrow_default_constructible_v<Iters>>...>)
	{}

	template<class _XIters, class _SFINAE_can_construct_from_provided_types = 
		std::enable_if_t<std::is_constructible_v<storage_t, const std::tuple<_XIters>&>>>
	associated_iterator(const std::tuple<_XIters>& iters)
		: m_iters(iters) {}

	associated_iterator(const associated_iterator<Iters...>& x) noexcept(is_nothrow_copy_constructible) 
		: m_iters(x.m_iters) {};
	associated_iterator(associated_iterator<Iters...>&& x) noexcept(is_nothrow_move_constructible)
		: m_iters(std::forward<storage_t>(x.m_iters)) {};

	associated_iterator& operator=(const associated_iterator<Iters...>& x)
	{
		this->m_iters = x.m_iters;
		return *this;
	}
	associated_iterator& operator=(associated_iterator<Iters...>&& x)
	{
		this->m_iters = std::forward<storage_t>(x.m_iters);
		return *this;
	}

	reference operator*() const noexcept(std::is_nothrow_copy_constructible_v<storage_t>)
	{
		return associated_iterator_reference<Iters...>(this->m_iters);
	}
	associated_iterator& operator++()
	{
		detail::for_each_in_tuple(this->m_iters, detail::incrementer());
		return *this;
	}
	template<class _SFINAE_is_forward_iterator = std::enable_if_t<(weakest_iterator_category >= forward_iterator_category)>>
	associated_iterator operator++(int)
	{
		auto copy = *this;
		++*this;
		return copy;
	}

	template<class _SFINAE_is_bidirectional_iterator = std::enable_if_t<(weakest_iterator_category >= bidirectional_iterator_category)>>
	associated_iterator& operator--()
	{
		detail::for_each_in_tuple(this->m_iters, detail::decrementer());
		return *this;
	}
	template<class _SFINAE_is_bidirectional_iterator = std::enable_if_t<(weakest_iterator_category >= bidirectional_iterator_category)>>
	associated_iterator operator--(int)
	{
		auto copy = *this;
		--*this;
		return copy;
	}

	template<class _SFINAE_is_bidirectional_iterator = std::enable_if_t<(weakest_iterator_category >= bidirectional_iterator_category)>>
	bool operator==(const associated_iterator<Iters...>& other) const
	{
		return get<0>(this->m_iters) == get<0>(other.m_iters);
	}
	template<class _SFINAE_is_bidirectional_iterator = std::enable_if_t<(weakest_iterator_category >= bidirectional_iterator_category)>>
	bool operator!=(const associated_iterator<Iters...>& other) const
	{
		return get<0>(this->m_iters) != get<0>(other.m_iters);
	}

	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	associated_iterator& operator+=(difference_type n)
	{
		detail::for_each_in_tuple(this->m_iters, detail::adder(n));
		return *this;
	}
	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	associated_iterator& operator-=(difference_type n)
	{
		detail::for_each_in_tuple(this->m_iters, detail::adder(-n));
		return *this;
	}
	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	associated_iterator operator+(difference_type n) const
	{
		auto copy = *this;
		copy += n;
		return copy;
	}
	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	associated_iterator operator-(difference_type n) const
	{
		auto copy = *this;
		copy -= n;
		return copy;
	}
	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	friend associated_iterator operator+(difference_type n, associated_iterator it)
	{
		it += n;
		return it;
	}
	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	friend difference_type operator-(const associated_iterator& i1, const associated_iterator& i2)
	{
		return static_cast<difference_type>(get<0>(i1.m_iters) - get<0>(i2.m_iters));
	}
	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	reference operator[](difference_type n) const
	{
		return *(*this + n);
	}

	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	bool operator<(const associated_iterator<Iters...>& other) const
	{
		return get<0>(this->m_iters) < get<0>(other.m_iters);
	}
	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	bool operator<=(const associated_iterator<Iters...>& other) const
	{
		return get<0>(this->m_iters) <= get<0>(other.m_iters);
	}
	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	bool operator>(const associated_iterator<Iters...>& other) const
	{
		return get<0>(this->m_iters) > get<0>(other.m_iters);
	}
	template<class _SFINAE_is_random_access_iterator = std::enable_if_t<(weakest_iterator_category >= random_access_iterator_category)>>
	bool operator>=(const associated_iterator<Iters...>& other) const
	{
		return get<0>(this->m_iters) >= get<0>(other.m_iters);
	}
};

template<class... Iters, class Pred = std::less<void>>
void associated_sort(const tuple<Iters...>& begin, const tuple<Iters...>& end, Pred pred = {})
{
	using assoc_iter = associated_iterator<Iters...>;
	return std::sort(assoc_iter(begin), assoc_iter(end), pred);
}

class Solution {
public:
	int earliestFullBloom(vector<int>& plantTime, vector<int>& growTime) {
		const int n = plantTime.size();

		associated_sort(
			std::tuple{ plantTime.begin(), growTime.begin() },
			std::tuple{ plantTime.end(), growTime.end() },
			[&](auto&& pl1, auto&& pl2)
		{
			const int p1 = pl1.get<0>();
			const int g1 = pl1.get<1>();
			const int p2 = pl2.get<0>();
			const int g2 = pl2.get<1>();
			const int x1 = p1 + max(g1, p2 + g2);
			const int x2 = p2 + max(g2, p1 + g1);
			return x1 < x2;
		});
		
		int plant_time = 0;
		int max_time = 0;
		for (size_t k = 0; k < n; ++k)
		{
			plant_time += plantTime[k];
			max_time = max(max_time, plant_time + growTime[k]);
		}
		return max_time;
	}
};

int main()
{
	vector<int> v1{ 4,3,1 }, v2{ 1,2,3 };
	Solution().earliestFullBloom(v1, v2);

	return 0;
}
