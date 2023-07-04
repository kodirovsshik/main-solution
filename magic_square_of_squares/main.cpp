
import <assert.h>;
import <stdlib.h>;
import <Windows.h>;
#undef min
#undef max

import std;
using namespace std;
using namespace chrono;

import libksn.multithreading;



using xint = int64_t;
using int64 = uint64_t;

template<class T> 
pair<T, T> divmod(T a, T b)
{
	return { a / b, a % b };
}
template<class T>
T sqr(T x)
{
	return x * x;
}
template<class T>
T xabs(T x)
{
	if (x < 0)
		return -x;
	return x;
}
template<class T>
T isqrt(const T& s)
{
	if (s <= 1)
		return s;

	T x0 = T(1) << (1 + bit_width(s) / 2);
	T x1 = (x0 + s / x0) / 2;

	while (x1 < x0)
	{
		x0 = x1;
		x1 = (x0 + s / x0) / 2;
	}
	//assert(s >= sqr(x0) && s < sqr(x0 + 1));
	return x0;
}


template<class T>
void testeq(const T& a, const T& b)
{
	if (a != b)
		__debugbreak();
}

int64 r(int64 m)
{
	return sqr(m - 2) / 4;
}

int64 pair_to_linear(int64 m, int64 n)
{
	const int64 q = r(m);
	const int64 start = (m - 1) % 2 + 1;
	const int64 index = (n - start) / 2;
	return q + index;
}
pair<int64, int64> linear_to_pair(int64 i)
{
	int64 m = 2 + 2 * isqrt(i);
	int64 idx = i - r(m);
	int64 index_bound = (m - 1) / 2;
	if (idx >= index_bound)
	{
		idx -= index_bound;
		m++;
	}
	const int64 start = (m - 1) % 2 + 1;
	return { m, start + 2 * idx };
}



struct record
{
	int64 iteration;
};

unordered_multimap<xint, record> total_mids;
binary_semaphore total_lock(1);
uint64_t work_offset = 0;
atomic_int n_working_threads = 0;
mutex worker_lock;
condition_variable worker_cv;

class local_storage_t
{
public:
	unordered_multimap<xint, record> mids;

	~local_storage_t()
	{
		total_lock.acquire();
		total_mids.insert(mids.begin(), mids.end());
		total_lock.release();
	}
};

template<class It>
auto to_range(const pair<It, It>& p)
{
	return ranges::subrange(p.first, p.second);
}


template<class T>
void dump(std::ostream& os, const T& x)
{
	os.write((const char*)&x, sizeof(x));
}

void save_data()
{
	auto clock_f = steady_clock::now;

	auto t1 = clock_f();
	ofstream fout("data_tmp.txt", ios::out | ios::binary);
	for (auto it = total_mids.begin(); it != total_mids.end();)
	{
		const auto& key = it->first;
		const size_t count = total_mids.count(key);
		advance(it, count);

		dump(fout, key);
		dump(fout, (int)count);

		for (auto&& [key, record] : to_range(total_mids.equal_range(key)))
			dump(fout, record.iteration);
	}
	fout.flush();
	fout.close();

	filesystem::remove("data.txt");
	try
	{
		filesystem::rename("data_tmp.txt", "data.txt");
	}
	catch (filesystem::filesystem_error excp)
	{
		ofstream("err.txt", ios::out | ios::app) << excp.what();
	}
	auto t2 = clock_f();

	ofstream("io_benchmark.txt", ios::out | ios::app) << duration_cast<milliseconds>(t2 - t1).count() * 1e-3 << "\n";
}

void run_work_set()
{
	const uint64_t work_size = 500'000'000ull * n_working_threads;

	ksn::run_parallel_work<local_storage_t>(work_offset, work_offset + work_size, [&]
	(uint64_t work_unit, local_storage_t& storage)
		{
			const auto [m, n] = linear_to_pair(work_unit);

			const xint alpha = xint(m - n) * (m + n);
			const xint beta = xint(m) * (n * 2);
			const xint c = xint(m + n) * (m + n) - beta;

			const auto [x5, x5rem] = divmod(c * c, 2 * alpha - beta);

			if (x5rem) return;
			if (xabs(x5) < 13386490) return;

			storage.mids.insert({ x5, { work_unit } });
		}, n_working_threads);

	ofstream("offset.txt") << (work_offset += work_size);
}



void work_control_thread()
{
	while (true)
	{
		unique_lock<mutex> l(worker_lock);
		worker_cv.wait(l, []() { return n_working_threads != 0; });

		if (n_working_threads < 0)
			return;
		run_work_set();

		l.unlock();
		save_data();
	}
}

template<class T>
void undump(istream& is, T& x)
{
	is.read((char*)&x, sizeof(x));
}

void read_data()
{
	ifstream("offset.txt") >> work_offset;

	ifstream fin("data.txt", ios::in | ios::binary);
	if (!fin.is_open())
		return;

	while (!fin.eof())
	{
		xint key;
		undump(fin, key);

		int count;
		undump(fin, count);

		while (count --> 0)
		{
			int64 val;
			undump(fin, val);
			total_mids.insert({ key, { val } });
		}
	}
}

int main()
{
	read_data();

	jthread worker(work_control_thread);
	while (true)
	{
		system("cls");
		print("\
status: {}\n\
 -1: exit\n\
 0: pause\n\
 n: set workers count to 'n'\n\
>>> ", n_working_threads.load());

		int x;
		cin >> x;
		cout << "...";

		lock_guard<mutex> l(worker_lock);
		n_working_threads = x;
		worker_cv.notify_all();

		if (x < 0)
			break;
	}

	return 0;
}
