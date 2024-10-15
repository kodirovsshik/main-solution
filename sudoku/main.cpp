
import std;
import <stdint.h>;
import <assert.h>;
import <stdio.h>;

template<class T>
using make_cref_t = const T&;



constexpr bool verbose_solving = true;

constexpr size_t N = 3;

constexpr size_t board_length_boxes = N;
constexpr size_t board_width_boxes = board_length_boxes;
constexpr size_t board_height_boxes = board_length_boxes;
constexpr size_t board_size_boxes = board_width_boxes * board_height_boxes;

constexpr size_t box_length = N;
constexpr size_t box_width = box_length;
constexpr size_t box_height = box_length;
constexpr size_t box_size = box_width * box_height;

constexpr size_t board_length = board_length_boxes * box_length;
constexpr size_t board_width = board_length;
constexpr size_t board_height = board_length;
constexpr size_t board_size = board_width * board_height;

constexpr size_t rows = board_height;
constexpr size_t columns = board_width;

constexpr size_t digits = N * N;

//int capable of holding 0, all digits, and 1 above it: [0; N^2 + 1]
using digit_t = int8_t;
static_assert(N* N <= std::numeric_limits<digit_t>::max() - 1 && SIZE_MAX / N >= N);



template<class T, class Cont>
size_t index_of(const T& val, const Cont& cont)
{
	return &val - &cont.front();
}

using pos_t = digit_t;
struct board_pos_t
{
	pos_t y{}, x{};

	using my_t = board_pos_t;
	using cref = make_cref_t<my_t>;

	friend my_t operator+(cref a, cref b) noexcept
	{
		return { pos_t(a.y + b.y), pos_t(a.x + b.x) };
	}
	friend my_t operator-(cref a, cref b) noexcept
	{
		return { pos_t(a.y - b.y), pos_t(a.x - b.x) };
	}

	size_t linear() const noexcept
	{
		return y * columns + x;
	}

	bool operator==(cref) const = default;
};

template <>
struct std::formatter<board_pos_t> : public std::formatter<std::string> {
	template <typename FormatContext>
	auto format(const board_pos_t& p, FormatContext& ctx) const {
		return std::formatter<std::string>::format("R" + std::to_string(p.y + 1) + "C" + std::to_string(p.x + 1), ctx);
	}
};

struct board_range_iterator;
struct board_range
{
	using cref = make_cref_t<board_range>;
	board_pos_t first{}, last{};

	using iterator = board_range_iterator;

	iterator begin() const noexcept;
	iterator end() const noexcept;
};

template<class T>
bool between_inclusive(T x, T a, T b)
{
	return x >= a && x <= b;
}
bool belongs(board_pos_t::cref pos, board_range::cref range)
{
	return
		between_inclusive(pos.x, range.first.x, range.last.x) &&
		between_inclusive(pos.y, range.first.y, range.last.y);
}

struct board_range_iterator
{
	using my_t = board_range_iterator;

	board_range range{};
	board_pos_t pos{};

	board_range_iterator& operator++()
	{
		inc();
		return *this;
	}

	auto operator*() const
	{
		return pos;
	}

	friend bool operator==(const my_t& a, const my_t& b) noexcept
	{
		return a.pos == b.pos;
	}

	void inc()
	{
		this->pos.x++;
		if (this->pos.x <= this->range.last.x)
			return;
		this->pos.x = this->range.first.x;
		this->pos.y++;
	}
};

board_range::iterator board_range::begin() const noexcept
{
	return board_range_iterator{ *this, this->first };
}
board_range::iterator board_range::end() const noexcept
{
	return board_range_iterator{ *this, { pos_t(this->last.y + 1), this->first.x } };
}


board_range get_row_range(pos_t i)
{
	board_pos_t start = { (pos_t)i, 0 };
	return { start, start + board_pos_t{ 0, board_width - 1} };
}
board_range get_column_range(pos_t i)
{
	board_pos_t start = { 0, (pos_t)i };
	return { start, start + board_pos_t{ board_height - 1, 0 } };
}
board_pos_t get_box_first_coord(pos_t i)
{
	const size_t box_row = i / board_length_boxes;
	const size_t box_column = i % board_length_boxes;
	return { pos_t(box_row * box_height), pos_t(box_column * box_width) };
}
board_range get_box_range(pos_t i)
{
	auto first = get_box_first_coord(i);
	return { first, first + board_pos_t{ box_height - 1, box_width - 1 } };
}
board_range get_entire_board_range()
{
	return { {0, 0}, { board_height - 1, board_width - 1} };
}

using unit_range_getter_func = board_range(*)(pos_t);


struct board_t
{
	digit_t data[board_size]{};

	auto& operator[](board_pos_t::cref pos)
	{
		return data[pos.y * columns + pos.x];
	}
};

struct cache_t
{
	using count_t = digit_t;
	using cell_options_t = std::bitset<digits>;

	cell_options_t candidates_in_cell[board_size]{};

	using unit_candidate_location_info_t = std::unordered_multimap<digit_t, board_pos_t>;
	using unit_group_t = std::array<unit_candidate_location_info_t, board_length>;

	enum class unit_type { row, column, box };
	static constexpr std::string_view unit_type_name[] = { "row", "column", "box" };
	static constexpr unit_range_getter_func unit_range_getter[] = { get_row_range, get_column_range, get_box_range };

	struct unit_group_descriptor
	{
		unit_type type;
		unit_group_t candidates_locatons{};
	};

	unit_group_descriptor rows{ unit_type::row };
	unit_group_descriptor columns{ unit_type::column };
	unit_group_descriptor boxes{ unit_type::box };
};

digit_t get_cell_value(const cache_t::cell_options_t& cell)
{
	static_assert(digits <= 64);
	return std::bit_width(cell.to_ullong());
}

void remove_candidate(cache_t::unit_candidate_location_info_t& unit, digit_t digit, board_pos_t target_pos)
{
	auto [digit_begin, digit_end] = unit.equal_range(digit);
	for (auto it = digit_begin; it != digit_end; ++it)
	{
		const auto current_pos = it->second;
		if (current_pos != target_pos)
			continue;

		unit.erase(it);
		break;
	}
}

struct game_state
{
	board_t board;
	cache_t cache;
};

pos_t to_box_number(board_pos_t pos)
{
	const pos_t box_x = pos.x / box_length;
	const pos_t box_y = pos.y / box_length;
	return box_y * board_length_boxes + box_x;
}
pos_t to_row_number(board_pos_t pos)
{
	return pos.y;
}
pos_t to_column_number(board_pos_t pos)
{
	return pos.x;
}


template<bool use_ranged_caches = true>
bool remove_candidate(game_state& state, board_pos_t::cref pos, digit_t digit)
{
	auto& cell = state.cache.candidates_in_cell[pos.linear()];
	
	if (!cell.test(digit - 1))
		return false;
	
	cell.reset(digit - 1);

	if constexpr (!use_ranged_caches)
		return true;
	
	remove_candidate(state.cache.boxes.candidates_locatons[to_box_number(pos)], digit, pos);
	remove_candidate(state.cache.rows.candidates_locatons[pos.y], digit, pos);
	remove_candidate(state.cache.columns.candidates_locatons[pos.x], digit, pos);

	return true;
}

template<bool use_ranged_caches = true>
void propagate_solved_cell(game_state& state, board_pos_t::cref pos)
{
	const auto digit = state.board[pos];
	assert(digit != 0);

	if constexpr (use_ranged_caches)
	{
		//TODO: check if helpful for optimization
		state.cache.rows.candidates_locatons[pos.y].erase(digit);
		state.cache.columns.candidates_locatons[pos.x].erase(digit);
		state.cache.boxes.candidates_locatons[to_box_number(pos)].erase(digit);
	}

	if constexpr (use_ranged_caches)
	{
		for (digit_t d = 1; d <= digits; ++d)
			remove_candidate<true>(state, pos, d);
	}
	else
	{
		state.cache.candidates_in_cell[pos.linear()].reset();
	}

	auto worker = [&](board_pos_t::cref pos)
	{
		remove_candidate<use_ranged_caches>(state, pos, digit);
	};

	for (auto pos : get_row_range(pos.y))
		worker(pos);
	for (auto pos : get_column_range(pos.x))
		worker(pos);
	for (auto pos : get_box_range(to_box_number(pos)))
		worker(pos);
}
void solve_cell(game_state& state, board_pos_t::cref pos, digit_t digit)
{
	assert(state.board[pos] == 0);
	state.board[pos] = digit;
	propagate_solved_cell(state, pos);
}

void rebuild_cache(game_state& state)
{
#define SKIP_RANGED_CACHES false

	state.cache = {};
	for (auto& cell : state.cache.candidates_in_cell)
		cell.set();

	for (auto pos : get_entire_board_range())
		if (state.board[pos])
			propagate_solved_cell<SKIP_RANGED_CACHES>(state, pos);

	for (auto pos : get_entire_board_range())
	{
		const auto& cell = state.cache.candidates_in_cell[pos.linear()];
		for (digit_t d = 1; d <= digits; ++d)
		{
			if (cell.test(d - 1) == false)
				continue;

			state.cache.boxes.candidates_locatons[to_box_number(pos)].insert({ d, pos });
			state.cache.rows.candidates_locatons[pos.y].insert({ d, pos });
			state.cache.columns.candidates_locatons[pos.x].insert({ d, pos });
		}
	}

}

bool try_read_board(board_t& board, const std::filesystem::path& p)
{
#define expect(cond, fmt, ...) { if (!(cond)) { std::println("Failed to read sudoku: "); std::println(stderr, fmt __VA_OPT__(,) __VA_ARGS__); return false; }; }
	std::ifstream fin(p);
	std::string input;
	expect(fin.is_open(), "Failed to open {}", p.generic_string());

	fin >> input;
	expect(fin && input == "sudoku", "Keyword \"sudoku\" expected, got \"{}\"", input);

	while (true)
	{
		fin >> input;
		expect(fin, "stream error");

		if (input == "N")
		{
			size_t expected_n;
			fin >> expected_n;
			expect(expected_n == N, "invalid sudoku size: configured for {}, got {}", N, expected_n);
		}
		else if (input == "begin")
			break;
		else
			expect(false, "Unknown command: {}", input);
	}

	for (auto pos : get_entire_board_range())
	{
		fin >> input;

		digit_t num;

		if (input.size() == 1 && input[0] == '.')
			num = 0;
		else
		{
			auto result = std::from_chars(&input.front(), &input.back() + 1, num);
			const bool parsed = result.ec == std::errc{} && result.ptr == &input.back() + 1;
			if (!parsed)
			{
				if (input == "end")
					expect(false, "Early \"end\" encountered - not enough data for {}x{} sudoku", board_height, board_width)
				else
					expect(false, "Failed to parse \"{}\" as number", input);
			}
		}
		
		board[pos] = num;
	}

	fin >> input;
	expect(input == "end", "Missing \"end\" command at expected place after {}x{} sudoku", board_height, board_width);

	return true;
}

class rule_helper
{
	std::string_view rule_name;
	std::ostream& sink;

	bool printed_anything = false;
	bool newline_pending = false;
	bool made_progress = false;

	enum class category { none = 0, solve, narrow, info } last_category{};
	static constexpr std::string_view category_names[] = { "", "SOLVE", "NARROW", "INFO" };

	void log(category new_category, std::string_view message)
	{
		if (!printed_anything)
		{
			printed_anything = true;
			std::println("Rule {}:", rule_name);
		}

		if (new_category != last_category)
		{
			handle_newline();
			std::print(sink, "\t{}: ", category_names[(int)new_category]);
			last_category = new_category;
			newline_pending = true;
		}
		
		std::print(sink, "{}; ", message);
	}

	void handle_newline()
	{
		if (newline_pending)
			sink << '\n';
		newline_pending = false;
	}

public:
	rule_helper(std::string_view rule_name, std::ostream& stream = std::cout) : rule_name(rule_name), sink(stream) {}
	~rule_helper()
	{
		handle_newline();
	}

	void solve(game_state& state, board_pos_t pos, digit_t digit)
	{
		solve_cell(state, pos, digit);
		made_progress = true;
		log(category::solve, std::format("{} = {}", pos, digit));
	}
	void narrow(game_state& state, board_pos_t pos, digit_t digit)
	{
		if (!remove_candidate(state, pos, digit))
			return;
		made_progress = true;
		log(category::narrow, std::format("{} != {}", pos, digit));
	}
	template<class... Args>
	void info(std::string_view fmt, const Args& ...args) const
	{
		log(category::info, std::vformat(fmt, std::make_format_args(args...)));
	}

	bool applied() const
	{
		return made_progress;
	}
};

template<class It, class F>
auto map_to_single_value(It begin, It end, F&& func)
{
	if (begin == end)
		return true;

	const auto val = func(*begin);
	while (++begin != end)
		if (func(*begin) != val)
			return std::nullopt;

	return std::optional{ val };
}

bool rule_last_possible_number(game_state& state)
{
	rule_helper helper("last possible number");

	for (auto pos : get_entire_board_range())
	{
		const auto& cell = state.cache.candidates_in_cell[pos.linear()];
		if (cell.count() == 1)
			helper.solve(state, pos, get_cell_value(cell));
	}
	return helper.applied();
}
bool rule_last_remaining_cell(game_state& state)
{
	rule_helper helper("last remaining cell");

	auto unit_worker = [&](cache_t::unit_group_descriptor& unit_group) 
	{
		for (auto& unit : unit_group.candidates_locatons)
		{
			for (digit_t digit = 1; digit <= digits; ++digit)
			{
				if (unit.count(digit) != 1)
					continue;
				const auto pos = unit.find(digit)->second;
				
				const char unit_mark = std::toupper(cache_t::unit_type_name[(int)unit_group.type][0]);

				if constexpr (verbose_solving)
					helper.info("In {}{}: {} can only appear at {}", unit_mark, 1 + index_of(unit, unit_group.candidates_locatons), digit, pos);
				helper.solve(state, pos, digit);
			}
		}
	};

	unit_worker(state.cache.rows);
	unit_worker(state.cache.columns);
	unit_worker(state.cache.boxes);

	return helper.applied();
}
bool rule_intersection_removal_box_line_reduction(game_state& state)
{
	rule_helper helper("intersection removal: box-line reduction");
	auto line_worker = [&](cache_t::unit_group_descriptor& desc)
	{
		for (const auto& unit : desc.candidates_locatons)
		{
			for (digit_t digit = 1; digit <= digits; ++digit)
			{
				auto [begin, end] = unit.equal_range(digit);
				if (begin == end)
					continue;

				const auto line_box_intersection = map_to_single_value(begin, end, to_box_number);
				if (!line_box_intersection)
					continue;

				for (auto pos : get_box_range(line_box_intersection.value()))
					helper.narrow(state, pos, digit);
			}
		}
	};
}
bool rule_intersection_removal_pointing_lines(game_state& state)
{
	rule_helper helper("intersection removal: pointing lines");
	auto box_worker = [&](cache_t::unit_group_descriptor& desc)
	{
		;

	};
}

int main()
{
	auto pstate = std::make_unique<game_state>();
	auto& state = *pstate;

	if (!try_read_board(state.board, "sudoku1.txt"))
		return 1;

	rebuild_cache(state);
	
	using rule_func_t = bool(*)(game_state&);

	rule_func_t rules[] = {
		rule_last_possible_number,
		rule_last_remaining_cell,
		rule_intersection_removal_pointing_lines,
		rule_intersection_removal_box_line_reduction,
	};

	while (true)
	{
		bool made_progress = false;
		for (auto rule : rules)
			if (rule(state))
			{
				made_progress = true;
				break;
			}

		if (made_progress)
			continue;
		else
			break;
	}

	bool solved = true;
	for (auto pos : get_entire_board_range())
		if (state.board[pos] == 0)
		{
			solved = false;
			break;
		}

	std::println("Solution {}found", solved ? "" : "not ");

	return 0;
}
