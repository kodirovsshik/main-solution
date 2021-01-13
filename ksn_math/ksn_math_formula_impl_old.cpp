// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <ksn/math.hpp>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <stdexcept>
#include <sstream>
#include <string>

//#pragma warning(pack(push))

#pragma warning(disable : 6011 6387 26439 6031 5040) //trust me, i know what i'm doing disabling so many warnings

_KSN_BEGIN

size_t formula::g_init_capacity = 256;
bool formula::g_perform_bracket_mismatch_check = false;

formula::formula()
{
	this->m_capacity = formula::g_init_capacity;
	this->m_p_equation = malloc(this->m_capacity);
	*(int8_t*)(this->m_p_equation) = 0;
	this->m_length = 0;
}

formula::formula(const formula& other)
{
	this->_copy_construct(other);
}

formula::formula(formula&& other)
{
	this->_copy_construct(other);
	other.m_length = 0;
}

formula::~formula()
{
	free(this->m_p_equation);
}



formula& formula::operator=(const formula& other)
{
	free(this->m_p_equation);
	this->_copy_construct(other);
	return *this;
}

void formula::_copy_construct(const formula& other)
{
	this->m_length = other.m_length;
	this->m_p_equation = malloc(other.m_capacity);
	this->m_capacity = other.m_capacity;
	this->m_p_equation = malloc(this->m_capacity);
	memcpy(this->m_p_equation, other.m_p_equation, this->m_length);
}

formula::operator bool() const noexcept
{
	return (size_t)this->m_p_equation & this->m_length;
}

double formula::operator()(const std::vector<double>& arguments) const
{
	if (arguments.size() != this->m_variables_numbers.size())
	{
		std::stringstream ss;
		ss << "Invalid count of arguments passed: Excepted " << this->m_variables_numbers.size() << ", got " << arguments.size();
		throw std::invalid_argument(ss.str().data());
	}
	//no plz, dont make me implement it :c

	const uint8_t* instruction_pointer = this->m_p8_equation,
		* p_end = instruction_pointer + this->m_length;

	double* storage = (double*)malloc(32767 * sizeof(double));


	auto extract_1_2_bytes = []
	(const uint8_t*& p) -> uint16_t
	{
		union
		{
			uint16_t x;
			uint8_t x_b[2];
		};

		x = 0;
		if (*p & 128)
		{
			x_b[1] = *p++ & 127;
		}

		x_b[0] = *p++;

		return x;
	};

	auto extract_operand = [&]
	(const uint8_t*& p, operand_type type) -> double
	{
		double result = double(INFINITY) * double(0);

		switch (type)
		{
		case operand_type::constant:
			result = *(double*)p;
			p += sizeof(double);
			break;

		case operand_type::variable:
		{
			union
			{
				uint32_t as_dword;
				uint16_t as_word[2];
				uint8_t as_byte[4];
			};
			as_dword = *(uint32_t*)p;
			p += sizeof(uint32_t);

			variable var;
			var.symbol = as_byte[0];
			var.index = as_word[1];

			if (!this->m_variables_numbers.count(var) || this->m_variables_numbers.at(var) >= arguments.size())
			{
				std::stringstream ss;
				ss << "Error: variable \"" << std::string(var) << "\" is unknown or corrupted. Did you modify formula::m_variables_numbers yourself?";
				throw std::logic_error(ss.str().data());
			}

			result = arguments[this->m_variables_numbers.at(var)];
		}
		break;

		case operand_type::intermediate:
		{
			size_t number = extract_1_2_bytes(p);
			result = storage[number];
		}
		break;

		case operand_type::error:
			throw std::invalid_argument("Operand type \"error\" is passed");
			break;
		}

		return result;
	};


	while (instruction_pointer < p_end)
	{
		uint8_t instruction = *instruction_pointer++;

		if (instruction == _KSN_MATH_FORMULA_INVALID_INSTRUCTION)
		{
			throw std::logic_error("Invalid instruction");
		}
		else if (instruction == _KSN_MATH_FORMULA_EXTENDED_INSTRUCTION)
		{
			throw std::logic_error("We don't implement any 2 or more bytes instructions yet. And probably never will");
		}
		else if (instruction <= _KSN_MATH_FORMULA_INSTRUCTION_MOD_II)
		{
			uint8_t temp = (instruction & 7) + 1;

			operand_type type1, type2;
			type1 = operand_type(temp / 3);
			type2 = operand_type(temp % 3);

			uint16_t destenation = extract_1_2_bytes(instruction_pointer);

			double result;
			double operand1 = extract_operand(instruction_pointer, type1);
			double operand2 = extract_operand(instruction_pointer, type2);

			switch (instruction & ~7)
			{
			case _KSN_MATH_FORMULA_INSTRUCTION_ADD_CV:
				result = operand1 + operand2;
				break;

			case _KSN_MATH_FORMULA_INSTRUCTION_SUB_CV:
				result = operand1 - operand2;
				break;

			case _KSN_MATH_FORMULA_INSTRUCTION_MUL_CV:
				result = operand1 * operand2;
				break;

			case _KSN_MATH_FORMULA_INSTRUCTION_DIV_CV:
				result = operand1 / operand2; //division by zero is user's problem, not mine
				break;

			case _KSN_MATH_FORMULA_INSTRUCTION_POW_CV:
				result = ::pow(operand1, operand2);
				break;

			case _KSN_MATH_FORMULA_INSTRUCTION_IDIV_CV:
				double unused;
				result = modf(operand1 / operand2, &unused);
				break;

			case _KSN_MATH_FORMULA_INSTRUCTION_MOD_CV:
				result = fmod(operand1, operand2);
				break;

			default:
				throw;
			}

			storage[destenation] = result;
		}
		else if (instruction >= _KSN_MATH_FORMULA_INSTRUCTION_RET_C)
		{
			double result = 0;

			switch (instruction)
			{
			case _KSN_MATH_FORMULA_INSTRUCTION_RET_I:
				result = storage[extract_1_2_bytes(instruction_pointer)];
				break;

			case _KSN_MATH_FORMULA_INSTRUCTION_RET_V:
			{
				result = extract_operand(instruction_pointer, operand_type::variable);
			}
			break;

			case _KSN_MATH_FORMULA_INSTRUCTION_RET_C:
				result = extract_operand(instruction_pointer, operand_type::constant);
				break;
			}

			free(storage);
			return result;
		}
		else
		{
			free(storage);
			throw std::logic_error("Illegal instruction");
		}
	}

	free(storage);
	throw std::logic_error("No return instruction was found during calculation. Did you modify the object yourself?");
}

void formula::expand() noexcept(false)
{
	void* p_new = realloc(this->m_p_equation, this->m_capacity *= 2);
	if (p_new == nullptr)
	{
		free(this->m_p_equation);

		this->m_p_equation = nullptr;

		throw this->m_capacity;
	}

	this->m_p_equation = p_new;
}

void formula::expand(size_t to) noexcept(false)
{
	while (this->m_capacity - this->m_length < to)
	{
		this->expand();
	}
}

void formula::clear() noexcept
{
	this->m_length = 0;
	this->m_variables_numbers.clear();
}



formula::variable::variable()
{
	this->index = this->symbol = 0;
}

formula::variable::variable(const variable& other)
{
	this->index = other.index;
	this->symbol = other.symbol;
}

bool formula::variable::operator<(const formula::variable& other) const noexcept
{
	if (this->index == uint16_t(-1) && other.index == uint16_t(-1))
	{
		return this->symbol < other.symbol;
	}
	if (this->index == uint16_t(-1))
	{
		return true;
	}
	if (other.index == uint16_t(-1))
	{
		return false;
	}
	if (this->symbol == other.symbol)
	{
		return this->index < other.index;
	}

	return this->symbol < other.symbol;
}

bool formula::variable::operator==(const formula::variable& other) const noexcept
{
	return this->index == other.index && this->symbol == other.symbol;
}

formula::variable::operator std::string() const
{
	if (this->symbol)
	{
		std::string s;
		s.reserve(8);

		s.push_back(this->symbol);

		if (index != uint16_t(-1))
			s += std::to_string(index);

		return s;
	}
	else
		return "";
}

size_t formula::variable::hash::operator()(const formula::variable& x) const noexcept
{
	return std::hash<uint16_t>()(x.index) + std::hash<char>()(x.symbol);
}

_KSN_END

//#pragma warning(pack(pop))
