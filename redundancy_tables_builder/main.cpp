
#include <stdint.h>
#include <assert.h>

#include <vector>
#include <bit>
#include <random>
#include <numeric>
#include <ranges>
#include <barrier>
#include <semaphore>
#include <thread>
#include <print>

using in_t = uint8_t;
using out_t = uint16_t;

static constexpr size_t n = 3;
static constexpr size_t m = 6;

static constexpr size_t table_len = 1 << n;
static constexpr size_t aux_table_len = 1 << m;

using table_t = out_t[table_len];
using aux_table_t = out_t[aux_table_len];

std::mt19937_64 global_rng{};



#define bit_distance(a, b) (size_t)std::popcount(uintmax_t((a) ^ (b)))

struct
{
	bool exception_enabled = false;
	in_t exception_index;
	out_t exception_value;

} thread_local decode_context;

in_t decode_with_context(out_t val, const table_t& table)
{
	size_t min_distance = SIZE_MAX;
	in_t min_idx = 0;
	for (in_t i = 0; true;)
	{
		out_t table_val = table[i];
		if (decode_context.exception_enabled && i == decode_context.exception_index) [[unlikely]]
			table_val = decode_context.exception_value;

		const size_t current_distance = bit_distance(val, table_val);

		if (current_distance < min_distance)
		{
			min_distance = current_distance;
			min_idx = i;
		}

		if (++i == (in_t)table_len)
			break;
	}
	return min_idx;
}


//template <typename T>
//class semaphore_lock_guard
//{
//	T& stored_semaphore;
//
//public:
//	semaphore_lock_guard(T& semaphore) : stored_semaphore(semaphore)
//	{
//		stored_semaphore.acquire();
//	}
//	~semaphore_lock_guard()
//	{
//		stored_semaphore.release();
//	}
//};

size_t C(size_t n, size_t k)
{
	size_t result = 1;
	for (size_t i = n; i != n - k; --i)
		result = result * i / (n - i + 1);
	return result;
}

out_t* generate_bitmasks(out_t* p, size_t len, size_t set, out_t val = 0)
{
	if (set == 0)
	{
		*p = val;
		return p + 1;
	}
	if (set == 1)
	{
		for (size_t i = 0; i < len; ++i)
			*p++ = val | (out_t(1) << i);
		return p;
	}
	for (size_t top_bit_pos = set - 1; top_bit_pos < len; ++top_bit_pos)
	{
		p = generate_bitmasks(p, top_bit_pos, set - 1, val | (out_t(1) << top_bit_pos));
	}
	return p;
}
const auto& enumerate_n_bit_masks(size_t set_bits)
{
	static std::vector<out_t> masks[m + 1];

	assert(set_bits <= m);

	//static std::binary_semaphore masks_access_semaphore(1);
	//semaphore_lock_guard raii_guard(masks_access_semaphore);

	auto& v = masks[set_bits];
	if (v.size() != 0)
		return v;

	v.resize(C(m, set_bits));
	generate_bitmasks(v.data(), m, set_bits);
	return v;
}



size_t evaluate_table(const table_t& table)
{
	//evaluation = sum of misdecodings after (upto) *delta* bits are changed
	static constexpr size_t delta = 1;

	size_t result = 0;

	for (in_t i = 0; true;)
	{
		for (size_t j = 1; j <= delta; ++j)
		{
			for (auto mask : enumerate_n_bit_masks(j))
			{
				result += decode_with_context(table[i] ^ mask, table) != i;
			}
		}

		if (++i == (in_t)table_len)
			break;
	}

	return result;
}

void generate_table(table_t& table)
{
	aux_table_t aux_table;
	std::ranges::iota(aux_table, 0);
	std::ranges::shuffle(aux_table, global_rng);
	std::copy(std::begin(aux_table), std::begin(aux_table) + table_len, std::begin(table));
}

struct
{
	table_t table;
	size_t evaluation;
	std::thread::id thread_to_make_change;
} optimization_context;


struct worker_data_t
{
	std::thread thread;
};

struct worker_shared_t
{
	std::binary_semaphore gloval_evaluation_update_semaphore;
	std::barrier<> iteration_end_barrier;
	std::atomic_bool stop = false;

	worker_shared_t(size_t threads) :
		gloval_evaluation_update_semaphore(1),
		iteration_end_barrier(threads)
	{
	}
};
std::unique_ptr<worker_shared_t> syncronization_context_ptr;

thread_local std::mt19937_64 local_rng;
thread_local std::uniform_int_distribution<uint64_t> table_index_distr(0, table_len - 1);
thread_local std::uniform_int_distribution<uint64_t> table_value_distr(0, (out_t(1) << m) - 1);

void thread_worker(worker_data_t& data)
{
	decode_context.exception_enabled = true;

	while (true)
	{
		decode_context.exception_index = (in_t)table_index_distr(local_rng);
		decode_context.exception_value = (out_t)table_value_distr(local_rng);
		
		const size_t new_evaluation = evaluate_table(optimization_context.table);
		
		syncronization_context_ptr->gloval_evaluation_update_semaphore.acquire();
		if (new_evaluation < optimization_context.evaluation)
		{
			optimization_context.evaluation = new_evaluation;
			optimization_context.thread_to_make_change = std::this_thread::get_id();
		}
		syncronization_context_ptr->gloval_evaluation_update_semaphore.release();

		syncronization_context_ptr->iteration_end_barrier.arrive_and_wait();

		if (std::this_thread::get_id() == optimization_context.thread_to_make_change)
		{
			std::println("{}", new_evaluation);
			optimization_context.thread_to_make_change = {};
			optimization_context.table[decode_context.exception_index] = decode_context.exception_value;
		}

		if (syncronization_context_ptr->stop)
			break;
	}
}



void start_computations(size_t threads)
{
	syncronization_context_ptr = std::make_unique<worker_shared_t>(threads);
	std::vector<worker_data_t> workers_data(threads);
	
	for (size_t i = 0; i < threads; ++i)
		workers_data[i].thread = std::thread(thread_worker, std::ref(workers_data[i]));

	for (auto&& data : workers_data)
		data.thread.join();
}

int main()
{
	for (size_t i = 0; i <= m; ++i)
		enumerate_n_bit_masks(i); //this forces generation of bitmasks vectors

	generate_table(optimization_context.table);
	optimization_context.evaluation = evaluate_table(optimization_context.table);
	std::println("{}", optimization_context.evaluation);

	start_computations(std::thread::hardware_concurrency());

	return 0;
}
