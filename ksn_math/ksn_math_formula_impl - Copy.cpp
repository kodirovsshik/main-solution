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
	static const uint8_t* _classify_array;

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

		_KSN_CONSTEXPR const size_t constants_trie_nodes = 13, functions_trie_nodes = 43;

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
		uint8_t* classify_array = (uint8_t*)(functions_trie + functions_trie_nodes);

		memset(classify_array, 0, 256);
		memset(classify_array + 'a', 1, 26);
		memset(classify_array + 'A', 2, 26);

		_formula_initializator::_classify_array = classify_array;

		_formula_initializator::_ctrie = (const _trie_entry*)constants_trie;
		_formula_initializator::_ftrie = (const _trie_entry*)functions_trie;

#define link_lower_no_case(from, to, letter) { constants_trie[from].links[(letter) - 'a'] = to; constants_trie[from].links[(letter) - 'a' + 26] = to; } ((void)0)
#define link_lower(from, to, letter) { constants_trie[from].links[(letter) - 'a'] = to; } ((void)0)
#define link_upper(from, to, letter) { constants_trie[from].links[(letter) - 'A' + 26] = to; } ((void)0)
#define bind_to_const(trie_index, const_index) {constants_trie[trie_index].index = const_index; } ((void)0)

		link_lower(0, 1, 'p');
		link_lower_no_case(1, 2, 'i');
		bind_to_const(2, 1);

		//constants_trie[0].links['p' - 'a'] = 1;
		//constants_trie[0].links['P' - 'A' + 26] = 1;
		//
		//constants_trie[1].links['i' - 'a'] = 2;
		////constants_trie[1].links['I' - 'A' + 26] = 2; //"PI" is for capital PI function (Ï(õ))

		//constants_trie[2].index = 1;

		//
		//constants_trie[1].links['h' - 'a'] = 3;
		//constants_trie[1].links['H' - 'A' + 26] = 3;

		//constants_trie[3].links['i' - 'a'] = 4;
		//constants_trie[3].links['I' - 'A' + 26] = 4;

		//constants_trie[4].index = 2;


		//constants_trie[0].links['e' - 'a'] = 5;
		//constants_trie[0].links['E' - 'A' + 26] = 5;

		//constants_trie[5].index = 3;


		//constants_trie[0].links['i' - 'a'] = 6;
		//constants_trie[0].links['I' - 'A' + 26] = 6;

		//constants_trie[6].index = 4;

		//constants_trie[0].links['g' - 'a'] = 7;
		//constants_trie[0].links['G' - 'A' + 26] = 7;
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

_KSN_DETAIL_END



fp_domain_descriptor::fp_domain_descriptor() {}

fp_domain_descriptor::number_type fp_domain_descriptor::constant_table(size_t index)
{
	
	static const fp_domain_descriptor::number_type table[] =
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


_KSN_END
