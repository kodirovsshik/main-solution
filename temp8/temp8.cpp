#include <stdio.h>

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>

#pragma warning(disable : 4996)

struct variable
{
	uint16_t sub;
	char c;

	variable() : c(0), sub(-1) {}
	variable(const char* p) : c(0), sub(-1)
	{
		if (!isalpha(*p)) return;
		c = *p++;

		if (!isdigit(*p)) return;

		uint16_t a = -1;

		(void)sscanf(p, "%hud", &a);
		if (a != -1) this->sub = 1;
	}

	variable(char c, uint16_t sub) : c(0), sub(-1)
	{
		if (!isalpha(c)) return;
		this->c = c;
		if (sub == -1) return;
		this->sub = sub;
	}

	bool operator==(const variable& other) const
	{
		return this->sub == other.sub && this->c == other.c;
	}
	bool operator!=(const variable& other) const
	{
		return !(*this == other);
	}
};


//typedef std::vector<std::pair<double, std::vector<std::pair<variable, int>>>> polynomial;

//Multiple variables polynomial
class polynomial
{
	typedef std::pair<variable, int> var_n_pow;
	struct unit
	{
		double coeff = 0;
		std::vector<var_n_pow> vars_n_pows;

		unit& operator*=(const unit& other)
		{
			if ((this->coeff *= other.coeff) == 0)
			{
				this->vars_n_pows.clear();
				return *this;
			}

			std::vector<var_n_pow> new_data;

			for (const auto& [var1, pow1] : this->vars_n_pows)
			{
				for (const auto& [var2, pow2] : other.vars_n_pows)
				{
					if (var1 == var2)
					{
						const int new_pow = pow1 + pow2;
						if (new_pow) new_data.emplace_back(std::make_pair(var2, new_pow));
					}
					else
					{
						new_data.emplace_back(std::make_pair(var1, pow1));
						new_data.emplace_back(std::make_pair(var2, pow2));
					}
				}
			}
		}
	};

	std::vector<unit> data;

	polynomial& operator*=(const polynomial& other)
	{

	}
};

int main()
{
	std::vector<variable> vars;
	while (1)
	{
		std::string input;
		std::getline(std::cin, input);
		variable v;
	}

}
