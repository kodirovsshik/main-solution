
import <cinttypes>;
import <climits>;
import <chrono>;
import <print>;

template<class T>
T sqr(const T& x)
{
	return x * x;
}

template<class T>
struct value_stream_stats
{
	T sum{};
	T sum2{};
	T min{};
	T max{};
	size_t n = 0;

	void append(const T& x)
	{
		if (n == 0) [[unlikely]]
		{
			min = max = x;
		}
		else
		{
			if (x < min) min = x;
			if (x > max) max = x;
		}
		sum += x;
		sum2 += x * x;
		n++;
	}

	template<class U = T>
	U mean() const
	{
		return U(sum) / U(n);
	}

	template<class U = T>
	U var_p() const
	{
		return U(sum2) / U(n) - sqr(U(sum) / U(n));
	}
	template<class U = T>
	U var() const
	{
		return U(n) / U(n - 1) * this->var_p<U>();
	}

	template<class U = T>
	U sd_p() const
	{
		using std::sqrt;
		return sqrt(this->var_p<U>());
	}
	template<class U = T>
	U sd() const
	{
		using std::sqrt;
		return sqrt(this->var<U>());
	}
};

auto measure()
{
	auto now = std::chrono::steady_clock::now;

	const auto t1 = now();

	volatile uint32_t i = 0;
	while (++i != 0);

	const auto t2 = now();

	return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / 1e3;
}

double binpow(double x, int n)
{
	if (n < 0)
		return 1 / binpow(x, -n);

	double r = 1;
	while (n)
	{
		if (n & 1)
			r *= x;
		x *= x;
		n >>= 1;
	}
	return r;
}

double xpow(double x)
{
	const double eps = 0.0001;

	double xint;
	const double frac = modf(x, &xint);

	double sum = 0;
	size_t n = 0;
	double num = 1;
	double denom = 1;

	while (true)
	{
		const double dsum = num / denom;
		sum += dsum;
		if (dsum <= eps)
			break;
		++n;
		num *= frac;
		denom *= n;
	}
	return sum * binpow(2.718281828, (int)xint);
}

int main()
{
	xpow(0);
	xpow(1);
	xpow(0.5);
	//value_stream_stats<double> stats;

	//for (size_t i = 0; i < 50; ++i)
	//	stats.append(measure());

	//std::println("{} +- {} s", stats.mean(), stats.sd());
	//std::println("{} ~ {} s", stats.min, stats.max);
}

