
import <stdint.h>;
import <unordered_map>;
import <semaphore>;

import libksn.multithreading;

template<ptrdiff_t N>
class semaphore_lock
{
	using S = std::counting_semaphore<N>;
	S* p = nullptr;

public:
	semaphore_lock(S& s) noexcept
	{
		this->acquire(s);
	}
	~semaphore_lock() noexcept
	{
		this->release();
	}

	semaphore_lock(const semaphore_lock&) = delete;
	semaphore_lock(semaphore_lock&&) = delete;

	void acquire(S& s) noexcept
	{
		this->release();
		p = &s;
		p->acquire();
	}
	void release() noexcept
	{
		if (p) p->release();
		p = nullptr;
	}
};

using xint = int64_t;

xint sqr(xint x)
{
	return x * x;
}

//n = (a^2 + b^2) / 2(a - b)
//(n - a)^2, n^2, (n + b)^2 are equidistant
struct equidistant_squares_params
{
	xint a, b;

	xint n() const
	{
		return (sqr(a) + sqr(b)) / 2 / (a - b);
	}
};


struct
{
	std::binary_semaphore semaphore{ 1 };
	std::unordered_multimap<xint, equidistant_squares_params> map;
} candidates;

void append_candidate(xint a, xint b)
{
	equidistant_squares_params pair{ a,b };
	auto insertee = std::pair{ pair.n(), pair };

	semaphore_lock lock(candidates.semaphore);
	candidates.map.insert(std::move(insertee));
}

void clear_lesser_candidates(xint n)
{
	semaphore_lock lock(candidates.semaphore);
	for (auto iter = candidates.map.begin(); iter != candidates.map.end(); ++iter)
	{
		if (iter->first < n)
			iter = candidates.map.erase(iter);
	}
}

void worker_process_b_line(xint b)
{

}

int main()
{


	return 0;
}
