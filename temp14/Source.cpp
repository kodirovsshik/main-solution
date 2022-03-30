
#include <vector>
#include <iostream>
#include <sstream>


class transformation_base
{
	virtual void print(std::ostream& os) const = 0;

public:
	virtual int operator()(int x) const = 0;
	virtual ~transformation_base() = 0 {};

	friend std::ostream& operator<<(std::ostream& os, const transformation_base& self)
	{
		self.print(os);
		return os;
	}
};

using transformation_ptr = std::unique_ptr<transformation_base>;
using transformations = std::vector<transformation_ptr>;
using transformation_sequence = std::vector<size_t>;



class transformation_add
	: public transformation_base
{
	const int operand;
public:
	transformation_add(int x) : operand(x) {}
	int operator()(int x) const { return x + this->operand; }
	void print(std::ostream& os) const { os << "x + " << this->operand; }
};

class transformation_sub
	: public transformation_base
{
	const int operand;
public:
	transformation_sub(int x) : operand(x) {}
	int operator()(int x) const { return x - this->operand; }
	void print(std::ostream& os) const { os << "x - " << this->operand; }
};

class transformation_mul
	: public transformation_base
{
	const int operand;
public:
	transformation_mul(int x) : operand(x) {}
	int operator()(int x) const { return x * this->operand; }
	void print(std::ostream& os) const { os << "x * " << this->operand; }
};

class transformation_div
	: public transformation_base
{
	const int operand;
public:
	transformation_div(int x) : operand(x) {}
	int operator()(int x) const { return x / this->operand; }
	void print(std::ostream& os) const { os << "x / " << this->operand; }
};

class transformation_mod
	: public transformation_base
{
	const int operand;
public:
	transformation_mod(int x) : operand(x) {}
	int operator()(int x) const { return x % this->operand; }
	void print(std::ostream& os) const { os << "x % " << this->operand; }
};

class transformation_neg
	: public transformation_base
{
public:
	int operator()(int x) const { return -x; }
	void print(std::ostream& os) const { os << "-x"; }
};


transformation_ptr parse_and_create_transformation()
{
	std::string str;
	std::getline(std::cin, str);
	
	std::istringstream ss(std::move(str));

	char op;
	int data;
	ss >> op >> data;

	bool have_data = (bool)ss;
	if (!have_data)
	{
		if (op == '-')
		{
			if (ss.rdbuf()->in_avail() == 0)
				op = 'n';
			else
				return nullptr;
		}
		else
			return nullptr;
	}

	switch (op)
	{
	case '+': return std::make_unique<transformation_add>(data);
	case '-': return std::make_unique<transformation_sub>(data);
	case '*': return std::make_unique<transformation_mul>(data);
	case '/': return std::make_unique<transformation_div>(data);
	case '%': return std::make_unique<transformation_mod>(data);
	case 'n': return std::make_unique<transformation_neg>();
	}

	return nullptr;
}



int eval(int x, const transformation_sequence& seq, const transformations& trs)
{
	for (auto& idx : seq)
	{
		const auto& tr = *trs[idx];
		x = tr(x);
	}
	return x;
}

int main()
{
	int x1, x2;
	size_t no, ns;

	std::cout << "Starting and ending numbers: ";
	std::cin >> x1 >> x2;

	std::cout << "Steps count: ";
	std::cin >> ns;

	std::cout << "Operations count: ";
	std::cin >> no;

	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


	transformations trs;
	trs.reserve(no);
	for (size_t i = 0; i < no; ++i)
	{
		do
		{
			std::cout << "Enter operation " << i << ": ";

			auto tr = parse_and_create_transformation();
			if (tr)
			{
				trs.emplace_back(std::move(tr));
				break;
			}
			
			std::cout << "Invalid operation\n";
		} while (true);
	}

	transformation_sequence current_seq;
	current_seq.resize(ns, 0);
	bool found = false;

	while (true)
	{
		if (eval(x1, current_seq, trs) == x2)
		{
			found = true;
			break;
		}

		bool overflow = true;
		for (auto& idx : current_seq)
		{
			overflow = false;
			if (++idx < no)
				break;

			overflow = true;
			idx = 0;
		}

		if (overflow)
			break;
	}

	if (found)
	{
		std::cout << "Transformation sequence: \n";
		for (const size_t& idx : current_seq)
		{
			std::cout << *trs[idx];
			if (&idx != &current_seq.back())
				std::cout << ", ";
		}
	}
	else
	{
		std::cout << "No solution found";
	}

	std::cout << std::endl;
}