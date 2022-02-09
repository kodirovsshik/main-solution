
#include <ksn/math_vec.hpp>
#include <ksn/math_matrix.hpp>

#include <numeric>

#pragma warning(disable : 4996)

class nn_t
{
	ksn::matrix<9, 9> w0, w1;
	ksn::vec<9> b0, b1;


	static float rand_float()
	{
		return rand() / (float)RAND_MAX * 2 - 1;
	}
	template<size_t N>
	static void randomize(ksn::vec<N>& v)
	{
		for (auto& x : v.data)
			x = rand_float();
	}
	template<size_t N, size_t M>
	static void randomize(ksn::matrix<N, M>& v)
	{
		for (auto& row : v.data)
			for (auto& x : row)
				x = rand_float();
	}

	static constexpr float mutate_max = 0.1f;

	template<size_t N>
	void mutate(ksn::vec<N>& v)
	{
		v[rand() % N] += rand_float() * mutate_max;
	}
	template<size_t N, size_t M>
	void mutate(ksn::matrix<N, M>& v)
	{
		v[rand() % N][rand() % M] += rand_float() * mutate_max;
	}

	float activation_function(float x)
	{
		return tanh(x / 2);
	}
	template<size_t N>
	ksn::vec<N> activation_function(ksn::vec<N> v)
	{
		for (auto& x : v.data)
			x = activation_function(x);
		return v;
	}

public:
	void randomize()
	{
		randomize(this->w0);
		randomize(this->w1);
		randomize(this->b0);
		randomize(this->b1);
	}

	void mutate()
	{
		mutate(this->w0);
		mutate(this->w1);
		mutate(this->b0);
		mutate(this->b1);
	}

	ksn::vec<9> eval(const ksn::vec<9> input)
	{
		return activation_function(w1 * activation_function(w0 * input + b0) + b1);
	}

	void store(FILE *f) const
	{
		fwrite(w0.data, 1, sizeof(w0.data), f);
		fwrite(w1.data, 1, sizeof(w1.data), f);
		fwrite(b0.data, 1, sizeof(b0.data), f);
		fwrite(b1.data, 1, sizeof(b1.data), f);
	}
	void load(FILE* f)
	{
		fread(w0.data, 1, sizeof(w0.data), f);
		fread(w1.data, 1, sizeof(w1.data), f);
		fread(b0.data, 1, sizeof(b0.data), f);
		fread(b1.data, 1, sizeof(b1.data), f);
	}
};

class field_t
{
	int8_t field[9]{};

public:
	void play(const ksn::vec<9>& data, bool player2)
	{
		size_t indexes[9];
		std::iota(indexes, indexes + 9, 0);
		std::sort(indexes, indexes + 9, [&]
		(size_t a, size_t b)
		{
			return data[a] > data[b];
		});

		int8_t* p = nullptr;
		for (int i = 0; i < 9; ++i)
		{
			if (field[indexes[i]] == 0)
			{
				p = &field[indexes[i]];
				break;
			}
		}

		*p = player2 ? 1 : -1;
	}

	int check_win() const
	{
		int triplets[][3] =
		{
			{0, 1, 2},
			{3,4, 5},
			{6, 7, 8},
			{0, 3, 6},
			{1, 4, 7},
			{2, 5, 8},
			{0, 4, 8},
			{2, 4, 6}
		};

		for (auto [a, b, c] : triplets)
		{
			if (field[a] && field[a] == field[b] && field[a] == field[c])
				return field[a];
		}
		return 0;
	}
	bool check_tie()
	{
		return std::all_of(field, field + 9, [](int x) {return x != 0; });
	}

	auto get_nn_input(bool player2)
	{
		ksn::vec<9> result;
		std::transform(field, field + 9, result.data, [player2] 
		(int8_t f) -> float 
		{
			return player2 ? (float)- f : (float)f;
		});
		return result;
	}
};


int main()
{
	FILE* fd = fopen("nn", "ab+");
	if (!fd) return 1;

	srand(time(0));

	nn_t nns[2];
	for (auto& nn : nns)
		nn.randomize();

	const size_t games = 1000000;
	for (size_t i = 0; i < games; ++i)
	{
		if (i == games - 1)
			__debugbreak();
		field_t field;
		int win = 0;
		bool tie = false;
		
		while (true)
		{
			for (auto& nn : nns)
			{
				bool player2 = &nn == &nns[1];
				field.play(nn.eval(field.get_nn_input(player2)), player2);
				
				win = field.check_win();
				if (win != 0)
					break;
				tie = field.check_tie();
				if (tie)
					break;
			}
			if (win || tie)
				break;
		}

		if (!tie)
			if (win < 0)
				nns[1] = nns[0];
			else
				nns[0] = nns[1];

		for (auto& nn : nns)
			nn.mutate();
	}

	//nns[0].store(fd);

	return 0;
}
