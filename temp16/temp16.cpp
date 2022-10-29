
#include <type_traits>
#include <stdexcept>
#include <numeric>


static constexpr bool modulo_integer_check_divisibility = 
#ifdef _DEBUG
true
#else
false
#endif
;

//modulus must not exceed (max(T)+1)/2
template<class T, T mod, T phi = T()>
class modulo_integer
{
	using my_t = modulo_integer<T, mod, phi>;
	static_assert(mod > 0, "");

	T m_value;

public:
	constexpr modulo_integer() noexcept = default;
	constexpr modulo_integer(const T&) noexcept(std::is_nothrow_copy_constructible_v<T>);

	constexpr my_t& operator+=(const my_t&) noexcept;
	constexpr my_t& operator-=(const my_t&) noexcept;
	constexpr my_t& operator*=(const my_t&) noexcept;
	constexpr my_t& operator/=(const my_t&);
	constexpr my_t& operator%=(const my_t&) noexcept;

	constexpr my_t operator+(const my_t&) const noexcept;
	constexpr my_t operator-(const my_t&) const noexcept;
	constexpr my_t operator*(const my_t&) const noexcept;
	constexpr my_t operator/(const my_t&) const;
	constexpr my_t operator%(const my_t&) const noexcept;

	constexpr my_t operator+() const noexcept;
	constexpr my_t operator-() const noexcept;

	constexpr operator T() const noexcept;
};


template<class T, T mod, T phi>
modulo_integer<T, mod, phi> pow(const modulo_integer<T, mod, phi>& a, const modulo_integer<T, mod, phi>& b);

template<class T, T mod, T phi>
modulo_integer<T, mod, phi> pow(const modulo_integer<T, mod, phi>& a, const T& b);



template<class T>
constexpr bool _is_prime(const T& x) noexcept
{
	for (T i = 2; i < x; ++i)
	{
		if ((x % i) == 0)
			return false;
	}
	return true;
}
template<class T, T mod, T _phi_v>
constexpr  T _phi() noexcept
{
	if constexpr (_phi_v == T())
	{
		T phi = mod;
		for (T i = 2; i <= mod; ++i)
		{
			if (_is_prime(i) && (mod % i) == 0)
				phi -= phi / i;
		}
		return phi;
	}
	else
		return _phi_v;
}

template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi> _inverse(const modulo_integer<T, mod, phi>& x)
{
	return pow(x, _phi<T, mod, phi>() - 1);
}
template<class T, T mod, T phi>
constexpr T _inverse(const T& x)
{
	if (modulo_integer_check_divisibility && std::gcd<T, T>(x, mod) != 1)
		throw std::domain_error("modular_integer: invalid division");
	return pow(modulo_integer<T, mod, phi>(x), _phi<T, mod, phi>() - 1);
}

template<class T, T mod>
constexpr T _normalize(const T& x) noexcept
{
	if (x >= 0)
		return x % mod;
	else
		return x - (x / mod - 1) * mod;
}

template<class T, T mod>
constexpr T _multiply(const T& a, T b) noexcept
{
	if (a < b)
		return _multiply<T, mod>(b, a);
	T result{};
	T current = a;
	while (b)
	{
		if (b & 1)
			result = (result + current) % mod;
		b >>= 1;
		current = (current << 1) % mod;

	}
	return result;
}

template<class T, T mod, T phi>
constexpr T _divide(const T& a, const T& b)
{
	return _multiply<T, mod>(a, _inverse<T, mod, phi>(b));
}



template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi>::modulo_integer(const T& x) noexcept(std::is_nothrow_copy_constructible_v<T>)
	: m_value(_normalize<T, mod>(x))
{
}

template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi>& modulo_integer<T, mod, phi>::operator+=(const my_t& rhs) noexcept
{
	this->m_value = (this->m_value + rhs.m_value) % mod;
	return *this;
}
template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi>& modulo_integer<T, mod, phi>::operator-=(const my_t& rhs) noexcept
{
	this->m_value = (this->m_value + mod - rhs.m_value) % mod;
	return *this;
}
template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi>& modulo_integer<T, mod, phi>::operator*=(const my_t& rhs) noexcept
{
	this->m_value = _multiply<T, mod>(this->m_value, rhs.m_value);
	return *this;
}
template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi>& modulo_integer<T, mod, phi>::operator/=(const my_t& rhs)
{
	this->m_value = _divide<T, mod>(this->m_value, rhs.m_value);
	return *this;
}
template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi>& modulo_integer<T, mod, phi>::operator%=(const my_t& rhs) noexcept
{
	this->m_value %= rhs.m_value;
	return *this;
}

template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi> modulo_integer<T, mod, phi>::operator+(const my_t& other) const noexcept
{
	my_t result(*this);
	result += other;
	return result;
}
template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi> modulo_integer<T, mod, phi>::operator-(const my_t& other) const noexcept
{
	my_t result(*this);
	result -= other;
	return result;
}
template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi> modulo_integer<T, mod, phi>::operator*(const my_t& other) const noexcept
{
	my_t result(*this);
	result *= other;
	return result;
}
template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi> modulo_integer<T, mod, phi>::operator/(const my_t& other) const
{
	my_t result(*this);
	result /= other;
	return result;
}
template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi> modulo_integer<T, mod, phi>::operator%(const my_t& other) const noexcept
{
	my_t result(*this);
	result %= other;
	return result;
}

template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi> modulo_integer<T, mod, phi>::operator+() const noexcept
{
	//bruh
	return *this;
}
template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi> modulo_integer<T, mod, phi>::operator-() const noexcept
{
	//conditional move pretty please?
	return this->m_value ? my_t(mod - this->m_value) : my_t(0);
}

template<class T, T mod, T phi>
constexpr modulo_integer<T, mod, phi>::operator T() const noexcept
{
	return this->m_value;
}



template<class T, T mod, T phi>
modulo_integer<T, mod, phi> _pow(modulo_integer<T, mod, phi> a, T b)
{
	modulo_integer<T, mod, phi> result(T(1));
	
	while (b > 0)
	{
		if (b & 1)
			result *= a;
		b >>= 1;
		a *= a;
	}
	return result;
}

template<class T, T mod, T phi>
modulo_integer<T, mod, phi> pow(const modulo_integer<T, mod, phi>& a, const modulo_integer<T, mod, phi>& b)
{
	return _pow(a, (T)b);
}

template<class T, T mod, T phi>
modulo_integer<T, mod, phi> pow(const modulo_integer<T, mod, phi>& a, const T& b)
{
	if (b < 0)
		return _pow(_inverse(a), -b);
	return _pow(a, b);
}


template<class int_t, int_t mod>
struct prime_mod_hasher_t
{
private:
	using hash_t = modulo_integer<int_t, mod, mod - 1>;
	//static constexpr int_t _start = int_t(2305843009213693921);
	static constexpr int_t _start = int_t(0);

public:
	hash_t exponent = 0;
	hash_t value;

	constexpr prime_mod_hasher_t(int_t exponent);
	template<class T>
	constexpr void update(const T& with);
};


int main1()
{
	using hasher = prime_mod_hasher_t<uint64_t, ((uint64_t)1 << 61) - 1>;
	const uint64_t p1 = 1152921504606846883;
	const uint64_t p2 = 1894530472435035030;
	hasher h1(1152921504606846883), h2(1894530472435035030);

	return 0;
}

template<class int_t, int_t mod>
constexpr prime_mod_hasher_t<int_t, mod>::prime_mod_hasher_t(int_t _exponent)
	: exponent(_exponent), value(_start)
{
}

template<class int_t, int_t mod>
template<class T>
constexpr void prime_mod_hasher_t<int_t, mod>::update(const T& with)
{
	this->value = this->value * this->exponent + (hash_t)with;
}

int main()
{
	int arr[6];
	for (auto& x : arr)
		x = 1;

	int c = 0;

	for (int i = 0; i < 1000000; ++i)
	{
		prime_mod_hasher_t<int, 97> h1(83);
		for (auto&& x : arr)
			h1.update(x);
		h1.value %= pow(modulo_integer<int, 97, 96>(83), 3);

		prime_mod_hasher_t<int, 97> h2(90);
		for (auto iter = std::rbegin(arr); iter != std::rend(arr); ++iter)
		{
			auto&& x = *iter;
			h2.update(x);
		}

		h2.value %= pow(modulo_integer<int, 97, 96>(90), 3);
		h2.value *= pow(modulo_integer<int, 97, 96>(83), 5);

		if (h1.value != h2.value)
			++c;

		for (int j = 0; j < 6; ++j)
			if (++arr[j] == 11)
				arr[j] = 1;
			else
				break;
	}

	return c;
}