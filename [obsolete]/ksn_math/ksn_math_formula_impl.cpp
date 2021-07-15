// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/math_formula.hpp>
//#include "formula_instructions.hpp"



_KSN_BEGIN


_KSN_DETAIL_BEGIN

struct _formula_initializator
{
	using _trie_entry = _formula_base::_trie_entry;

	static const _trie_entry *_ctrie;
	static const _trie_entry *_ftrie;
	static const uint8_t *_classify_array;

	_formula_initializator()
	{
		static int counter = 0;
		
		if (counter)
		{
			return;
		}
		else
		{
			counter = 1;
		}

		_KSN_CONSTEXPR const size_t constants_trie_nodes = 12, functions_trie_nodes = 49;

		_KSN_CONSTEXPR const size_t constants_trie_size = constants_trie_nodes * sizeof(_trie_entry);
		_KSN_CONSTEXPR const size_t functions_trie_size = functions_trie_nodes * sizeof(_trie_entry);

		_KSN_CONSTEXPR const size_t allocation_size = constants_trie_size + functions_trie_size + 256;

		void* p_memory = malloc(allocation_size);

		if (p_memory == nullptr)
		{
			abort();
		}

		memset(p_memory, 0, allocation_size);
		
		_trie_entry *constants_trie = (_trie_entry*)p_memory;
		_trie_entry *functions_trie = constants_trie + constants_trie_nodes;
		uint8_t *classify_array = (uint8_t*)(functions_trie + functions_trie_nodes);

		memset(classify_array, 0, 256);
		memset(classify_array + 'a', 1, 26);
		memset(classify_array + 'A', 2, 26);

		_formula_initializator::_classify_array = classify_array;

		_formula_initializator::_ctrie = (const _trie_entry*)constants_trie;
		_formula_initializator::_ftrie = (const _trie_entry*)functions_trie;

#define link_lower_no_case(from, to, letter) { constants_trie[from].links[(letter) - 'a'] = to; constants_trie[from].links[(letter) - 'a' + 26] = to; } ((void)0)
#define link_lower(from, to, letter) { constants_trie[from].links[(letter) - 'a'] = to; } ((void)0)
#define link_upper(from, to, letter) { constants_trie[from].links[(letter) - 'A' + 26] = to; } ((void)0)
#define bind_to_const(trie_index, const_index) { constants_trie[trie_index].index = const_index; } ((void)0)

		link_lower_no_case(0, 1, 'p');
		
			link_lower_no_case(1, 2, 'i');
			bind_to_const(2, 1);

			link_lower_no_case(1, 3, 'h');
				link_lower_no_case(3, 4, 'i');
				bind_to_const(4, 2);

		link_lower_no_case(0, 5, 'e');
		bind_to_const(5, 3);

		link_lower_no_case(0, 6, 'i');
		bind_to_const(6, 4);

		link_lower_no_case(0, 7, 'g');
			link_lower_no_case(7, 8, 'a');
				link_lower_no_case(8, 9, 'm');
					link_lower_no_case(9, 10, 'm');
						link_lower_no_case(10, 11, 'a');
						bind_to_const(11, 5);

		; //It IS supposed be here, nevermind anyway

#undef link_lower_no_case
#undef link_lower
#undef link_upper
#undef bind_to_const

#define link_lower_no_case(from, to, letter) { functions_trie[from].links[(letter) - 'a'] = to; functions_trie[from].links[(letter) - 'a' + 26] = to; } ((void)0)
#define link_lower(from, to, letter) { functions_trie[from].links[(letter) - 'a'] = to; } ((void)0)
#define link_upper(from, to, letter) { functions_trie[from].links[(letter) - 'A' + 26] = to; } ((void)0)
#define bind_to_func(trie_index, func_index) { functions_trie[trie_index].index = func_index; } ((void)0)
		

		link_lower_no_case(0, 1, 's');
			link_lower_no_case(1, 2, 'i');
				link_lower_no_case(2, 3, 'n');
				bind_to_func(3, 1);

		link_lower_no_case(0, 4, 'c');
			link_lower_no_case(4, 5, 'o');
				link_lower_no_case(5, 6, 's');
				bind_to_func(6, 2);

				link_lower_no_case(5, 43, 't');
				bind_to_func(43, 4);

			link_lower_no_case(4, 12, 't');
				link_lower_no_case(12, 13, 'g');
				bind_to_func(13, 4);

		link_lower_no_case(0, 7, 't');
			link_lower_no_case(7, 8, 'g');
			bind_to_func(8, 3);

			link_lower_no_case(8, 10, 'a');
				link_lower_no_case(9, 10, 'n');
				bind_to_func(10, 3);

		link_lower_no_case(0, 14, 'a');
			link_lower_no_case(14, 39, 'r');
				link_lower_no_case(39, 40, 'c');
					link_lower_no_case(40, 11, 's');
					link_lower_no_case(40, 15, 't');
					link_lower_no_case(40, 41, 'c');
						link_lower_no_case(41, 20, 't');

						link_lower_no_case(41, 42, 'o');
							link_lower_no_case(42, 23, 't');
							bind_to_func(23, 6);

							link_lower_no_case(42, 28, 's');
							bind_to_func(28, 8);
			
			link_lower_no_case(14, 15, 't');
				link_lower_no_case(15, 16, 'g');
				bind_to_func(16, 5);

				link_lower_no_case(14, 17, 'a');
					link_lower_no_case(17, 18, 'n');
					bind_to_func(18, 5);

			link_lower_no_case(14, 19, 'c');
				link_lower_no_case(19, 20, 't');
					link_lower_no_case(20, 21, 'g');
					bind_to_func(21, 6);

				link_lower_no_case(19, 22, 'o');
					link_lower_no_case(22, 23, 't');
					bind_to_func(23, 6);

					link_lower_no_case(22, 28, 's');
					bind_to_func(28, 8);

			link_lower_no_case(14, 11, 's');
				link_lower_no_case(11, 24, 'i');
					link_lower_no_case(24, 25, 'n');
					bind_to_func(25, 7);

		link_upper(0, 26, 'P');
			link_lower_no_case(26, 27, 'i');
			bind_to_func(27, 12);

		link_upper(0, 29, 'F');
		bind_to_func(29, 11);

		link_upper(0, 30, 'S');
			link_lower_no_case(30, 31, 'i');
			bind_to_func(31, 13);

		link_lower_no_case(0, 32, 'z');
			link_lower_no_case(32, 33, 'e');
				link_lower_no_case(33, 34, 't');
					link_lower_no_case(34, 35, 'a');
					bind_to_func(35, 9);

		link_lower_no_case(0, 36, 'e');
			link_lower_no_case(36, 37, 't');
				link_lower_no_case(37, 38, 'a');
				bind_to_func(38, 10);

		link_upper(0, 44, 'G');
			link_lower_no_case(44, 45, 'a');
				link_lower_no_case(45, 46, 'm');
					link_lower_no_case(46, 47, 'm');
						link_lower_no_case(47, 48, 'a');
						bind_to_func(48, 14);

#undef link_lower_no_case
#undef link_lower
#undef link_upper
#undef bind_to_func
	}

	int operator()()
	{
		return 0;
	}

	~_formula_initializator()
	{
		free((void*)_ctrie);
	}

} static _initializator;

const _formula_base::_trie_entry *_formula_initializator::_ctrie;
const _formula_base::_trie_entry *_formula_initializator::_ftrie;
const uint8_t *_formula_initializator::_classify_array;

int initialize_result = _initializator();

const _formula_base::_trie_entry* const _formula_base::_trie_constants = _formula_initializator::_ctrie;
const _formula_base::_trie_entry* const _formula_base::_trie_functions = _formula_initializator::_ftrie;
const uint8_t* const _formula_base::_classify = _formula_initializator::_classify_array;

void __cdecl _formula_default_deallocator(void* p, size_t)
{
	free(p);
}

_KSN_DETAIL_END



struct fpl_domain_descriptor_functions
{
private:
	fpl_domain_descriptor_functions() = delete;

public:
	using number_type = fpl_domain_descriptor::number_type;
	using descriptor_t = detail::_formula_base::_function_descriptor_t;

	static descriptor_t table[];

	static number_type sin(
		number_type* parameter,
		number_type* arguments,
		size_t arguments_count
	)
	{
		return ::sinl(*arguments);
	}

	static number_type cos(
		number_type* parameter,
		number_type* arguments,
		size_t arguments_count
	)
	{
		return ::cosl(*arguments);
	}

	static number_type tan(
		number_type* parameter,
		number_type* arguments,
		size_t arguments_count
	)
	{
		return ::tanl(*arguments);
	}

	static number_type ctg(
		number_type* parameter,
		number_type* arguments,
		size_t arguments_count
	)
	{
		return 1.0L / ::tanl(*arguments);
	}

	static number_type asin(
		number_type* parameter,
		number_type* arguments,
		size_t arguments_count
	)
	{
		return ::asinl(*arguments);
	}

	static number_type acos(
		number_type* parameter,
		number_type* arguments,
		size_t arguments_count
	)
	{
		return ::acosl(*arguments);
	}

	static number_type atan(
		number_type* parameter,
		number_type* arguments,
		size_t arguments_count
	)
	{
		return ::atanl(*arguments);
	}

	static number_type actg(
		number_type* parameter,
		number_type* arguments,
		size_t arguments_count
	)
	{
		return ::atanl(1.0L / *arguments);
	}
};

detail::_formula_base::_function_descriptor_t fpl_domain_descriptor_functions::table[] =
{
	{nullptr, 0, 0, 0},
	{&fpl_domain_descriptor_functions::sin, 1, 1, 0},
};



fpl_domain_descriptor::fpl_domain_descriptor() {}

fpl_domain_descriptor::number_type fpl_domain_descriptor::constant_table(size_t index)
{
	
	static const fpl_domain_descriptor::number_type table[] =
	{
		0,
		KSN_PI,
		KSN_PHI,
		KSN_E,
		0,
		KSN_GAMMA
	};

	enum class allowance : uint8_t { ok = 0, invalid = 1, domain_unallowed = 2};

	static const allowance allowance_table[] =
	{
		allowance::invalid,
		allowance::ok,
		allowance::ok,
		allowance::ok,
		allowance::domain_unallowed,
		allowance::ok,
	};

	if (allowance_table[index] == allowance::invalid || index >= sizeof(table) / sizeof(table[0]))
	{
		_KSN_RAISE(std::invalid_argument("Invalid index"));
	}
	else if (allowance_table[index] == allowance::domain_unallowed)
	{
		_KSN_RAISE(detail::_formula_domain_exception());
	}

	return table[index];
}

detail::_formula_base::_function_descriptor_t* fpl_domain_descriptor::function_table(size_t index)
{


	return fpl_domain_descriptor_functions::table + index;
}


_KSN_END
