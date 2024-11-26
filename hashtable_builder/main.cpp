
import <utility>;
import <string_view>;
import <random>;
import <iostream>;
import <fstream>;
import <tuple>;
import <ranges>;
import <algorithm>;
import <semaphore>;
import <thread>;
import <deque>;
import <chrono>;

#define KSN_FORCEINLINE __forceinline



template<class T, T First, T... Next>
struct index_set_total_work_size
{
	static constexpr T value = First * index_set_total_work_size<T, Next...>::value;
};
template<class T, T First>
struct index_set_total_work_size<T, First>
{
	static constexpr T value = First;
};

template<size_t N, class T, T First, T... Next>
struct index_set_nth_dim
{
	static constexpr T value = index_set_nth_dim<N - 1, T, Next...>::value;
};
template<class T, T First, T... Next>
struct index_set_nth_dim<0, T, First, Next...>
{
	static constexpr T value = First;
};

template<class T, T... dims>
struct index_set
{
public:
	using my_t = index_set<T, dims...>;
	static constexpr size_t size = sizeof...(dims);

private:
	T indices[size]{};

	template<size_t N>
	constexpr static T nth_dim() noexcept
	{
		return index_set_nth_dim<N, T, dims...>::value;
	}

	template<size_t N>
	constexpr void KSN_FORCEINLINE set_indices_recursive(T idx)
	{
		if constexpr (N == size)
			return;
		else
		{
			constexpr T current = nth_dim<N>();
			this->indices[N] = idx % current;
			this->set_indices_recursive<N + 1>(idx / current);
		}
	}
	template<size_t N>
	constexpr void KSN_FORCEINLINE increment_recursive()
	{
		if constexpr (N == size)
			return;
		else
		{
			if (++this->indices[N] < nth_dim<N>())
				return;
			this->indices[N] = 0;
			increment_recursive<N + 1>();
		}
	}

public:

	static consteval T total_work_size() noexcept
	{
		return index_set_total_work_size<T, dims...>::value;
	}

	using value_type = T;
	using reference = T&;
	using const_reference = const T&;

	using iterator = T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_iterator = const T*;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	constexpr iterator begin() noexcept
	{
		return this->indices;
	}
	constexpr iterator end() noexcept
	{
		return this->indices + size;
	}

	static constexpr my_t from_single_index(T idx) noexcept
	{
		my_t result;
		result.set_indices_recursive<0>(idx);
		return result;
	}

	template<class Self>
	constexpr auto&& operator[](this Self&& self, size_t n) noexcept
	{
		return self.indices[n];
	}

	constexpr index_set& operator++() noexcept
	{
		this->increment_recursive<0>();
		return *this;
	}

	constexpr friend bool operator==(const my_t& a, const my_t& b) noexcept
	{
		for (size_t i = 0; i < size; ++i)
		{
			if (a[i] != b[i])
				return false;
		}
		return true;
	}

	constexpr operator bool() noexcept
	{
		for (auto&& x : *this)
			if (x)
				return true;
		return false;
	}
};



template<std::integral T>
T randint(T min, T max)
{
	static constexpr size_t seed = 0;
	static std::mt19937_64 engine(seed);
	std::uniform_int_distribution<T> dist(min, max);
	return dist(engine);
}
template<std::integral T>
T randint(T max)
{
	return randint<T>(0, max);
}



struct string_hasher
{
	size_t a, p;

	size_t operator()(std::string_view v)
	{
		size_t hash = 0;
		for (char c : v)
		{
			hash = hash * a + c;
			hash %= p;
		}
		return hash;
	}
};



size_t primes[10000] = 
{
};

constexpr size_t n_primes = std::size(primes);



const std::string_view raw[] =
{
	{"sin"},
	{"cos"},
	{"tan"},
	{"tg"},
	{"ctg"},
	{"cot"},
	{"asin"},
	{"arcsin"},
	{"acos"},
	{"arccos"},
	{"atg"},
	{"arctg"},
	{"atan"},
	{"arctan"},
	{"ln"},
	{"lg"},
	{"log"},
	{"exp"},
};
constexpr size_t N = std::size(raw);



using tuple = std::tuple<size_t, size_t>;

template<size_t N, class... Types>
size_t getter(const std::tuple<Types...>& t)
{
	return std::get<N>(t);
}
size_t tuple_max(const tuple& x)
{
	auto&& [a1, a2] = x;
	return std::max({ a1, a2 });
}

template<class T>
bool _put_tuple_element(std::ostream& os, T&& elem)
{
	os << ", " << elem;
	return true;
}
template<size_t Zero, size_t... Rest, class... Types>
void _print_tuple1(std::ostream& os, const std::tuple<Types...>& t)
{
	(... && _put_tuple_element(os, getter<Rest>(t)));
}
template<size_t... Seq, class... Types>
void _print_tuple(std::ostream& os, const std::tuple<Types...>& t, std::integer_sequence<size_t, Seq...>)
{
	os << getter<0>(t);
	_print_tuple1<Seq...>(os, t);
}
template<class... Types>
std::ostream& operator<<(std::ostream& os, const std::tuple<Types...>& t) noexcept
{
	_print_tuple(os, t, std::make_index_sequence<sizeof...(Types)>());
	return os;
}



std::deque<tuple> data;
std::binary_semaphore data_semaphore(1);
size_t global_collision_numbers[N]{};
std::binary_semaphore global_collision_numbers_semaphore(1);

using job_index = index_set<size_t, n_primes, n_primes>;



void worker(job_index indices, job_index end_indices)
{
	string_hasher hasher{};
	size_t collision_numbers[N]{};
	tuple current_max;
	size_t current_max_value = std::numeric_limits<size_t>::max();

	while (true)
	{
		hasher.a = primes[indices[0]];
		hasher.p = primes[indices[1]];
		//hasher.s = indices[2];

		size_t collisions = 0;
		size_t table[N]{};
		for (auto&& key : raw)
		{
			if (0 != table[hasher(key) % N]++)
				collisions++;
		}

		collision_numbers[collisions]++;

		tuple current(hasher.a, hasher.p);
		if (collisions == 0 && tuple_max(current) < current_max_value)
		{
			current_max_value = tuple_max(current);
			current_max = current;
		}

		if (++indices == end_indices)
			break;
	}

	if (tuple_max(current_max) != 0)
	{
		data_semaphore.acquire();
		data.push_back(current_max);
		data_semaphore.release();
	}

	global_collision_numbers_semaphore.acquire();
	for (size_t i = 0; i < N; ++i)
		global_collision_numbers[i] += collision_numbers[i];
	global_collision_numbers_semaphore.release();
}

int main()
{
	std::ifstream primes_file("../sieve_of_eratosthenes/up to 100000000.txt");
	for (auto& prime : primes)
		primes_file >> prime;
	if (!primes_file)
		return -1;
	primes_file.close();

	auto now = std::chrono::steady_clock::now;
	auto t1 = now(), t2 = t1;

	const size_t n_threads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads(n_threads);

	constexpr size_t total_work_size = job_index::total_work_size();
	const size_t work_split_size = total_work_size / n_threads;
	const size_t unsplit_work = total_work_size % n_threads;

	for (size_t i = 0; i < n_threads; ++i)
	{
		threads[i] = std::thread(worker,
			job_index::from_single_index(i * work_split_size),
			job_index::from_single_index(i * work_split_size + work_split_size));
	}

	t1 = now();

	if (unsplit_work != 0)
		worker(job_index::from_single_index(n_threads * work_split_size), job_index::from_single_index(total_work_size));

	for (auto& thread : threads)
		if (thread.joinable())
			thread.join();

	t2 = now();

	std::cout << "Collisions: " << std::endl;
	for (size_t i = 0; auto n : global_collision_numbers)
		std::cout << '[' << i++ << "] = " << n << std::endl;

	if (!data.empty())
	{
		auto min = std::ranges::min(data, []
		(const tuple& a, const tuple& b) -> bool
		{
			return tuple_max(a) < tuple_max(b);
		});

		std::cout << "No collisions: " << min << '\n';
	}

	std::cout << "Time: " << (double)(t2 - t1).count() * 1e-9 << " seconds\n";

	return 0;
}
