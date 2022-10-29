
#include <iostream>
#include <vector>
#include <list>
//using fp_t = double;
//using polynomial_t = std::vector<fp_t>;
//
//polynomial_t polynomial_reduce(const polynomial_t& P, const fp_t& at)
//{
//	const size_t N = P.size();
//	std::vector<fp_t> result;
//	result.reserve(N - 1);
//
//	fp_t c = 0;
//	for (size_t i = 0; i < N - 1; ++i)
//	{
//		c = c * at + P[i];
//		result.push_back(c);
//	}
//	return result;
//}
//
//fp_t polynomial_reduce_in_place(polynomial_t& P, const fp_t& at, bool return_remainder = false)
//{
//}

template<class fp_t = double>
class polynomial_t
{
public:
	std::vector<fp_t> m_data;

	polynomial_t() noexcept;
	polynomial_t(std::initializer_list<fp_t> data);

	void reduce_root(const fp_t& root) noexcept;
	template<class T>
	void reduce_order(const T& threshold) noexcept;

	fp_t operator()(const fp_t& x) const noexcept;

	template<class Self>
	auto& operator[](this Self&&, size_t n) noexcept;

	size_t size() const noexcept;
	size_t order() const noexcept;
	size_t capacity() const noexcept;
	void reserve(size_t) const;

	void integrate(const fp_t& constant = fp_t());
	void differentiate() noexcept;


};

template<class fp_t>
polynomial_t<fp_t>::polynomial_t() noexcept
{
}
template<class fp_t>
polynomial_t<fp_t>::polynomial_t(std::initializer_list<fp_t> data)
	: m_data(data)
{
}

template<class fp_t>
void polynomial_t<fp_t>::reduce_root(const fp_t& root) noexcept
{
	const size_t N = this->size();
	fp_t c = 0;
	for (size_t i = 0; i < N - 1; ++i)
	{
		c = c * root + this->m_data[i];
		this->m_data[i] = c;
	}
	this->m_data.pop_back();
}
template<class fp_t>
template<class T>
void polynomial_t<fp_t>::reduce_order(const T& threshold) noexcept
{
	using std::abs;
	const size_t N = this->size();
	size_t zeros = N;
	for (size_t i = 0; i < N; ++i)
	{
		if (abs(this->m_data[i]) > threshold)
		{
			zeros = i;
			break;
		}
	}

	auto data_begin = this->m_data.begin();
	this->m_data.erase(data_begin, data_begin + zeros);
}

template<class fp_t>
fp_t polynomial_t<fp_t>::operator()(const fp_t& x) const noexcept
{
	const size_t N = this->size();
	fp_t y = N ? this->m_data.front() : 0;

	for (size_t i = 1; i < N; ++i)
		y = y * x + this->m_data[i];

	return y;
}

template<class fp_t>
template<class Self>
auto& polynomial_t<fp_t>::operator[](this Self&& self, size_t i) noexcept
{
	return self.m_data[self.size() - 1 - i];
}

template<class fp_t>
size_t polynomial_t<fp_t>::size() const noexcept
{
	return this->m_data.size();
}
template<class fp_t>
size_t polynomial_t<fp_t>::order() const noexcept
{
	const size_t n = this->m_data.size();
	return n > 0 ? n - 1 : 0;
}
template<class fp_t>
size_t polynomial_t<fp_t>::capacity() const noexcept
{
	return this->m_data.capacity();
}
template<class fp_t>
void polynomial_t<fp_t>::reserve(size_t n) const
{
	return this->m_data.reserve(n);
}

template<class fp_t>
void polynomial_t<fp_t>::integrate(const fp_t& constant);
template<class fp_t>
void polynomial_t<fp_t>::differentiate() noexcept
{
	const size_t N = this->order();
	for (size_t i = 0; i < N; ++i)
		this->m_data[i] *= (N - i);
	this->m_data.pop_back();
}


template<class fp_t>
bool is_negative(const fp_t& x)
{
	return x < 0;
}

template<class fp_t>
fp_t recalculate_dx(const fp_t& dx, const fp_t& at)
{
	using std::abs;
	return abs(dx) <= 1 ? dx : dx * abs(at);
}

template<class fp_t>
void normalize(polynomial_t<fp_t>& p)
{
	fp_t shrink = 0;

	for (auto&& x : p.m_data)
		shrink += fp_t(std::abs(x));

	shrink = 1 / shrink;
	for (auto& x : p.m_data)
		x *= shrink;
}

template<class Container>
void reserve(Container&, size_t) {}

template<class T, class Alloc>
void reserve(std::vector<T, Alloc>& v, size_t n)
{
	v.reserve(n);
}

template<class C1, class C2>
void emplace_back_container(C1&& what, C2& where)
{
	reserve(where, what.size() + where.size());
	for (auto&& elem : what)
		where.emplace_back(std::move(elem));
}

template<class fp_t, class abs_t = fp_t>
struct polynomial_solve_params
{
	fp_t dx = 1 / fp_t(10);
	abs_t y_threshold = 1 / fp_t(1000000);
	size_t newton_method_max_iters = 50;
};

namespace solve1
{

	template<class fp_t>
	std::vector<fp_t> _solve_linear(const polynomial_t<fp_t>& p)
	{
		return { -p[0] / p[1] };
	}
	template<class fp_t>
	std::vector<fp_t> _solve_quadratic(const polynomial_t<fp_t>& p)
	{
		using std::sqrt;

		fp_t a_inv = fp_t(1) / p[2];
		fp_t b = p[1] * a_inv / fp_t(-2);
		fp_t c = p[0] * a_inv;

		fp_t D = b * b - c;
		if (is_negative<fp_t>(D))
			return {};
		D = sqrt(D);
		return { b - D, b + D };
	}
	//template<class fp_t>
	//std::vector<fp_t> _solve_cubic(const polynomial_t<fp_t>& p)
	//{
	//
	//}




	template<class fp_t, class T, class Container>
	void _solve_reduced(polynomial_t<fp_t>& p, Container& container, const polynomial_solve_params<fp_t, T>& params);

	template<class fp_t, class T, class Container>
	bool _solve_for_some_roots(const polynomial_t<fp_t>& p, Container& roots, const polynomial_solve_params<fp_t, T>& params)
	{
		using std::abs;

		roots.clear();

		polynomial_t<fp_t> derivative = p;
		derivative.differentiate();

		std::list<fp_t> points;
		_solve_reduced(derivative, points, params);

		derivative = p;
		derivative.differentiate();

		if (points.size())
		{
			points.sort();
			points.push_back(points.back() + recalculate_dx(params.dx, points.back()));

			auto last = std::next(points.end(), -1);
			for (auto iter = points.begin(); iter != last; ++iter)
				*iter -= recalculate_dx(params.dx, *iter);
		}
		else
			points.emplace_back(0);

		static const T shift_threshold = 0.1;

		size_t iters = params.newton_method_max_iters;
		while (iters-- > 0)
		{
			fp_t shift;
			auto iter = points.begin();
			while (iter != points.end())
			{
				auto& point = *iter;
				shift = -p(point) / derivative(point);

				if (abs(shift) > shift_threshold)
					shift *= shift_threshold / abs(shift);

				auto old_iter = iter++;

				if (abs(shift) <= shift_threshold)
				{
					point += shift;
					if (abs(p(point)) <= params.y_threshold)
					{
						roots.emplace_back(std::move(point));
						points.erase(old_iter);
					}
				}

				//nothing changed if shift is NAN or +-INF
			}
			if (points.empty())
				break;
		}
		return !roots.empty();
	}

	template<class fp_t, class T, class Container>
	void _solve_reduced(polynomial_t<fp_t>& p, Container& roots, const polynomial_solve_params<fp_t, T>& params)
	{
		using std::abs;

		if (p.order() == 0)
			return;
		if (p.order() == 1)
			return emplace_back_container(_solve_linear(p), roots);
		if (p.order() == 2)
			return emplace_back_container(_solve_quadratic(p), roots);

		normalize(p);

		std::vector<fp_t> new_roots;
		while (p.order() > 2)
		{
			new_roots.clear();
			if (!_solve_for_some_roots(p, new_roots, params))
				break;

			for (auto& root : new_roots)
				p.reduce_root(root);

			emplace_back_container(new_roots, roots);
		}

		if (p.order() <= 2)
			_solve_reduced(p, roots, params);
	}

	template<class fp_t, class T = fp_t, class Container>
	void solve(polynomial_t<fp_t> p, Container& container, const polynomial_solve_params<fp_t, T>& params = {})
	{
		p.reduce_order(params.y_threshold);
		return _solve_reduced(p, container, params);
	}
}

namespace solve2
{
	template<class fp_t, class abs_t, class Container>
	void _solve_reduced(polynomial_t<fp_t>& p, Container& roots, const polynomial_solve_params<fp_t>& params = {})
	{
		while (p.order() > 2)
		{

		}
	}

	template<class fp_t, class abs_t, class Container>
	void solve(polynomial_t<fp_t> p, Container& roots, const polynomial_solve_params<fp_t>& params = {})
	{
		p.reduce_order(params.y_threshold);
		_solve_reduced(p, roots, params);
	}
}


int main()
{
	polynomial_solve_params<double> params;
	params.y_threshold = 1e-12;

	polynomial_t<double> p{ 1, -16, 100, -310, 499, -394, 120 };
	std::vector<double> roots;
	//solve(p, roots, params);
	return 0;
}
