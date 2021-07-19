#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <thread>
#include <vector>
#include <utility>
#include <unordered_map>
#include <unordered_set>

#include <windows.h>


#pragma warning (disable : 4244 4996)


#define PI 3.14159265f

template<typename T>
const char* get_type_format()
{
	return nullptr;
}
template<>
const char* get_type_format<float>()
{
	return "%lg";
}
template<>
const char* get_type_format<int32_t>()
{
	return "%d";
}
template<>
const char* get_type_format<uint32_t>()
{
	return "%u";
}

template<typename T>
std::string get_format(const char* name, T val)
{
	return std::string(name) + " = " + get_type_format<T>() + "\n";
}

#ifdef _DEBUG
#define dprintf(fmt, ...) printf(fmt, __VA_ARGS__)
#define do_dprintf(x) dprintf(get_format(#x, x).data(), x)
#else
#define dprintf(fmt, ...) ((int)0)
#define do_dprintf ((void)0)
#endif




float g_vertex_radius = 3;



void draw_vertex(int x, int y, sf::RenderTarget& dest, sf::Color color = sf::Color::Red)
{
	sf::CircleShape vertex;
	vertex.setFillColor(color);
	vertex.setRadius(g_vertex_radius);
	vertex.setPosition(x - g_vertex_radius, y - g_vertex_radius);
	dest.draw(vertex);
}

void draw_line(int x1, int x2, int y1, int y2, sf::RenderTarget& dest, sf::Color color = sf::Color::Green)
{
	sf::RectangleShape line;
	line.setPosition(x1, y1);

	int lenx = x2 - x1;
	int leny = y2 - y1;

	float angle = atan2f(leny, lenx);
	angle = angle * 180 / PI;

	line.setSize({ sqrtf(lenx * lenx + leny * leny), 1.f });
	line.setRotation(angle);

	line.setOutlineColor(color);

	dest.draw(line);
}



void wait_for_framerate(int framerate)
{
	static auto prev_time = std::chrono::time_point<std::chrono::high_resolution_clock>(std::chrono::nanoseconds(0));

	auto now = std::chrono::high_resolution_clock::now();

	int64_t passed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - prev_time).count();

	int64_t to_wait = 1000000000 / framerate;
	to_wait -= passed;

	auto t1 = std::chrono::high_resolution_clock::now();
	std::this_thread::sleep_for(std::chrono::nanoseconds(to_wait));
	auto t2 = std::chrono::high_resolution_clock::now();

	FILE* fp = fopen("a.txt", "a");
	fprintf(fp, "%lli %llu\n", to_wait / 1000, std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
	fclose(fp);

	prev_time = now;
}



struct vertex2
{
	float x;
	float y;
};
struct vertex3
{
	float x;
	float y;
	float z;

	vertex3& operator+=(const vertex3& other)
	{
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
		return *this;
	}
	vertex3& operator-=(const vertex3& other)
	{
		this->x -= other.x;
		this->y -= other.y;
		this->z -= other.z;
		return *this;
	}
};
struct vertex4
{
	float x;
	float y;
	float z;
	float w;
};



vertex2 rotate2(vertex2 dot, vertex2 origin, float angle_in_radians)
{
	dot.x -= origin.x;
	dot.y -= origin.y;

	vertex2 result;
	result.x = dot.x * cos(angle_in_radians) - dot.y * sin(angle_in_radians);
	result.y = dot.x * sin(angle_in_radians) + dot.y * cos(angle_in_radians);

	result.x += origin.x;
	result.y += origin.y;

	return result;
}

vertex2 rotate2_d(vertex2 dot, vertex2 origin, float angle)
{
	return rotate2(dot, origin, angle / 180 * PI);
}



vertex3 rotate3_xz(vertex3 vertex, vertex3 origin, float angle_in_radians)
{
	vertex2 projected_vertex{ vertex.x, vertex.z }, projected_origin{ origin.x, origin.y };
	projected_vertex = rotate2(projected_vertex, projected_origin, angle_in_radians);
	return vertex3{ projected_vertex.x, vertex.y, projected_vertex.y };
}
vertex3 rotate3_xy(vertex3 vertex, vertex3 origin, float angle_in_radians)
{
	vertex2 projected_vertex{ vertex.x, vertex.y }, projected_origin{ origin.x, origin.y };
	projected_vertex = rotate2(projected_vertex, projected_origin, angle_in_radians);
	return vertex3{ projected_vertex.x, projected_vertex.y, vertex.z };
}
vertex3 rotate3_yz(vertex3 vertex, vertex3 origin, float angle_in_radians)
{
	vertex2 projected_vertex{ vertex.z, vertex.y }, projected_origin{ origin.z, origin.y };
	projected_vertex = rotate2(projected_vertex, projected_origin, angle_in_radians);
	return vertex3{ vertex.x, projected_vertex.y, projected_vertex.x };
}

vertex3 rotate3_xz_d(vertex3 vertex, vertex3 origin, float angle)
{
	vertex2 projected_vertex{ vertex.x, vertex.z }, projected_origin{ origin.x, origin.y };
	projected_vertex = rotate2(projected_vertex, projected_origin, angle / 180 * PI);
	return vertex3{ projected_vertex.x, vertex.y, projected_vertex.y };
}
vertex3 rotate3_xy_d(vertex3 vertex, vertex3 origin, float angle)
{
	vertex2 projected_vertex{ vertex.x, vertex.y }, projected_origin{ origin.x, origin.y };
	projected_vertex = rotate2(projected_vertex, projected_origin, angle / 180 * PI);
	return vertex3{ projected_vertex.x, projected_vertex.y, vertex.z };
}
vertex3 rotate3_yz_d(vertex3 vertex, vertex3 origin, float angle)
{
	vertex2 projected_vertex{ vertex.z, vertex.y }, projected_origin{ origin.z, origin.y };
	projected_vertex = rotate2(projected_vertex, projected_origin, angle / 180 * PI);
	return vertex3{ vertex.x, projected_vertex.y, projected_vertex.x };
}

vertex4 rotate4_xy(vertex4 vertex, vertex4 origin, float angle)
{
	vertex2 projected_vertex{ vertex.x, vertex.y }, projected_origin{ origin.x, origin.y };

	projected_vertex = rotate2(projected_vertex, projected_origin, angle);

	vertex.x = projected_vertex.x;
	vertex.y = projected_vertex.y;

	return vertex;
}
vertex4 rotate4_xz(vertex4 vertex, vertex4 origin, float angle)
{
	vertex2 projected_vertex{ vertex.x, vertex.z }, projected_origin{ origin.x, origin.z };

	projected_vertex = rotate2(projected_vertex, projected_origin, angle);

	vertex.x = projected_vertex.x;
	vertex.z = projected_vertex.y;

	return vertex;
}
vertex4 rotate4_yz(vertex4 vertex, vertex4 origin, float angle)
{
	vertex2 projected_vertex{ vertex.y, vertex.z }, projected_origin{ origin.y, origin.z };

	projected_vertex = rotate2(projected_vertex, projected_origin, angle);

	vertex.y = projected_vertex.x;
	vertex.z = projected_vertex.y;

	return vertex;
}
vertex4 rotate4_xw(vertex4 vertex, vertex4 origin, float angle)
{
	vertex2 projected_vertex{ vertex.x, vertex.w }, projected_origin{ origin.x, origin.w };

	projected_vertex = rotate2(projected_vertex, projected_origin, angle);

	vertex.x = projected_vertex.x;
	vertex.w = projected_vertex.y;

	return vertex;
}
vertex4 rotate4_yw(vertex4 vertex, vertex4 origin, float angle)
{
	vertex2 projected_vertex{ vertex.y, vertex.w }, projected_origin{ origin.y, origin.w };

	projected_vertex = rotate2(projected_vertex, projected_origin, angle);

	vertex.y = projected_vertex.x;
	vertex.w = projected_vertex.y;

	return vertex;
}
vertex4 rotate4_zw(vertex4 vertex, vertex4 origin, float angle)
{
	vertex2 projected_vertex{ vertex.z, vertex.w }, projected_origin{ origin.z, origin.w };

	projected_vertex = rotate2(projected_vertex, projected_origin, angle);

	vertex.z = projected_vertex.x;
	vertex.w = projected_vertex.y;

	return vertex;
}

vertex4 rotate4_xy_d(vertex4 vertex, vertex4 origin, float angle)
{
	return rotate4_xy(vertex, origin, angle / 180 * PI);
}
vertex4 rotate4_xz_d(vertex4 vertex, vertex4 origin, float angle)
{
	return rotate4_xz(vertex, origin, angle / 180 * PI);
}
vertex4 rotate4_yz_d(vertex4 vertex, vertex4 origin, float angle)
{
	return rotate4_yz(vertex, origin, angle / 180 * PI);
}
vertex4 rotate4_xw_d(vertex4 vertex, vertex4 origin, float angle)
{
	return rotate4_xw(vertex, origin, angle / 180 * PI);
}
vertex4 rotate4_yw_d(vertex4 vertex, vertex4 origin, float angle)
{
	return rotate4_yw(vertex, origin, angle / 180 * PI);
}
vertex4 rotate4_zw_d(vertex4 vertex, vertex4 origin, float angle)
{
	return rotate4_zw(vertex, origin, angle / 180 * PI);
}



//For triangulation:

//std::vector<std::pair<uint32_t, uint32_t>> vertexes = {
//	{150, 150}, //1
//	{300, 170}, //2
//	{450, 150}, //3
//	{525, 250}, //4
//	{450, 300}, //5
//	{375, 325}, //6
//	{300, 375}, //7
//	{225, 325}, //8
//	{150, 300}, //9
//	{75, 250} //10
//};

std::unordered_map<size_t, vertex3> figure3_cube = {
	{1, {100, 500, 100}},
	{2, {100, 500, 500}},
	{3, {500, 500, 500}},
	{4, {500, 500, 100}},
	{5, {100, 100, 100}},
	{6, {100, 100, 500}},
	{7, {500, 100, 500}},
	{8, {500, 100, 100}}
};
std::unordered_multimap<size_t, size_t> edges3_cube = {
		{1, 2},
		{1, 4},
		{1, 5},
		{5, 6},
		{5, 8},
		{2, 3},
		{2, 6},
		{4, 3},
		{4, 8},
		{3, 7},
		{6, 7},
		{8, 7},
		/*{9, 1},
		{9, 2},
		{9, 3},
		{9, 4},
		{9, 5},
		{9, 6},
		{9, 7},
		{9, 8},
		{9, 9},*/
};

std::unordered_map<size_t, vertex4> figure4_tesseract = {
	{1, {100, 100, 100, 100}},
	{2, {100, 100, 700, 100}},
	{3, {700, 100, 700, 100}},
	{4, {700, 100, 100, 100}},
	{5, {100, 700, 100, 100}},
	{6, {100, 700, 700, 100}},
	{7, {700, 700, 700, 100}},
	{8, {700, 700, 100, 100}},
	{9, {100, 100, 100, 700}},
	{10, {100, 100, 700, 700}},
	{11, {700, 100, 700, 700}},
	{12, {700, 100, 100, 700}},
	{13, {100, 700, 100, 700}},
	{14, {100, 700, 700, 700}},
	{15, {700, 700, 700, 700}},
	{16, {700, 700, 100, 700}},
};
std::unordered_multimap<size_t, size_t> edges4_tesseract = {
		{1, 2},
		{1, 4},
		{1, 5},
		{5, 6},
		{5, 8},
		{2, 3},
		{2, 6},
		{4, 3},
		{4, 8},
		{3, 7},
		{6, 7},
		{8, 7},
		{9, 10},
		{9, 12},
		{9, 13},
		{13, 14},
		{13, 16},
		{10, 11},
		{10, 14},
		{12, 11},
		{12, 16},
		{11, 15},
		{14, 15},
		{16, 15},
		{1, 9},
		{2, 10},
		{3, 11},
		{4, 12},
		{5, 13},
		{6, 14},
		{7, 15},
		{8, 16},
};

std::unordered_map<size_t, vertex3> figure3_square = {
	{1, {100, 100, 100}},
	{2, {100, 100, 500}},
	{3, {500, 100, 500}},
	{4, {500, 100, 100}}
};
std::unordered_multimap<size_t, size_t> edges3_square = {
	{1, 2},
	{1, 4},
	{2, 3},
	{4, 3}
};




vertex3 g_camera3;
float g_camera_rotation_right;
float g_camera_rotation_down;
float g_camera_rotation_ccw;
int32_t g_width;
int32_t g_height;
float fov;



//vertex2 project32_1(vertex3 vertex)
//{
//	vertex2 result;
//	result.x = 300 - g_camera3.z * (vertex.x - g_camera3.x) / (vertex.z - g_camera3.z);
//	result.y = 300 - g_camera3.z * (vertex.y - g_camera3.y) / (vertex.z - g_camera3.z);
//	return result;
//}
//
//vertex2 project32_2(vertex3 vertex)
//{
//	vertex2 result;
//	result.x = 300 - (vertex.z + g_camera3.z - distance_to_screen) * (vertex.x - g_camera3.x) / (vertex.z - g_camera3.z);
//	result.y = 300 - (vertex.z + g_camera3.z - distance_to_screen) * (vertex.y - g_camera3.y) / (vertex.z - g_camera3.z);
//	return result;
//}
//
//#define project32(x) project32_2(x)

bool g_camera_rotation_modified = false;
float g_camera_rotate_matrix[3][3];
//
//vertex2 vertex_to_screen1(vertex3 vertex)
//{
//	//Convert world coordinates to camera coordinates
//
//	float x0 = +(vertex.x - g_camera3.x);
//	float y0 = -(vertex.z - g_camera3.z);
//	float z0 = +(vertex.y - g_camera3.y);
//
//
//
//	//rotate around Y axis
//
//	float x1 = x0 * cos(g_camera_rotation_right) - z0 * sin(g_camera_rotation_right);
//	float y1 = y0;
//	float z1 = x0 * sin(g_camera_rotation_right) + z0 * cos(g_camera_rotation_right);
//
//
//
//	//rotate around X axis
//
//	float x2 = x1;
//	float y2 = z1 * sin(g_camera_rotation_down) - y1 * cos(g_camera_rotation_down);
//	float z2 = z1 * cos(g_camera_rotation_down) + y1 * sin(g_camera_rotation_down);
//
//
//
//	//rotate around Z axis
//
//	float x3 = x2 * cos(g_camera_rotation_ccw) - y2 * sin(g_camera_rotation_ccw);
//	float y3 = x2 * sin(g_camera_rotation_ccw) + y2 * cos(g_camera_rotation_ccw);
//	float z3 = z2;
//
//
//
//	//Now project is to the screen
//	//We've got camera at (0; 0; -dist)
//	vertex2 result;
//	//result.x = g_width / 2 + x3 * distance_to_screen / (z3 + distance_to_screen);
//	//result.y = g_height / 2 + y3 * distance_to_screen / (z3 + distance_to_screen);
//
//	float fov_1 = fov / 2;
//	fov_1 *= PI / 180;
//
//	float angle_x = atan2(z3, -x3) - PI / 2;
//	result.x = g_width / 2 + g_width * (angle_x / fov_1);
//
//	float angle_y = atan2(z3, -y3) - PI / 2;
//	result.y = g_height / 2 - g_height * (angle_y / fov_1);
//
//	return result;
//}
//vertex2 vertex_to_screen2(vertex3 vertex)
//{
//	//Convert world coordinates to camera coordinates
//
//	float x0 = +(vertex.x - g_camera3.x);
//	float y0 = -(vertex.z - g_camera3.z);
//	float z0 = +(vertex.y - g_camera3.y);
//
//
//
//	//rotate around X axis
//
//	float x1 = x0;
//	float y1 = z0 * sin(g_camera_rotation_down) - y0 * cos(g_camera_rotation_down);
//	float z1 = z0 * cos(g_camera_rotation_down) + y0 * sin(g_camera_rotation_down);
//
//
//
//	//rotate around Y axis
//
//	float x2 = x1 * cos(g_camera_rotation_right) - z1 * sin(g_camera_rotation_right);
//	float y2 = y1;
//	float z2 = x1 * sin(g_camera_rotation_right) + z1 * cos(g_camera_rotation_right);
//
//
//
//	//rotate around Z axis
//
//	float x3 = x2 * cos(g_camera_rotation_ccw) - y2 * sin(g_camera_rotation_ccw);
//	float y3 = x2 * sin(g_camera_rotation_ccw) + y2 * cos(g_camera_rotation_ccw);
//	float z3 = z2;
//
//
//
//	//Now project is to the screen
//	//We've got camera at (0; 0; -dist)
//	vertex2 result;
//	//result.x = g_width / 2 + x3 * distance_to_screen / (z3 + distance_to_screen);
//	//result.y = g_height / 2 + y3 * distance_to_screen / (z3 + distance_to_screen);
//
//	float fov_1 = fov / 2;
//	fov_1 *= PI / 180;
//
//	float angle_x = atan2(z3, -x3) - PI / 2;
//	result.x = g_width / 2 + g_width * (angle_x / fov_1);
//
//	float angle_y = atan2(z3, -y3) - PI / 2;
//	result.y = g_height / 2 - g_height * (angle_y / fov_1);
//
//	return result;
//}
//
//vertex2 project32(const vertex3& vertex)
//{
//	vertex2 result;
//
//	float fov_ = fov / 2;
//	fov_ *= PI / 180;
//
//	float max_difference = vertex.z * tan(fov_);
//
//	result.x = g_width / 2 * (1 + vertex.x / max_difference);
//	result.y = g_height / 2 * (1 + vertex.y / max_difference);
//
//	return result;
//}
//
//vertex2 vertex_to_screen3(vertex3 vertex)
//{
//	{
//		float x0 = +(vertex.x - g_camera3.x);
//		float y0 = -(vertex.z - g_camera3.z);
//		float z0 = +(vertex.y - g_camera3.y);
//
//		vertex.x = x0;
//
//		vertex.y = y0;
//		vertex.z = z0;
//	}
//
//	vertex3 copy = vertex;
//
//#define matrix g_camera_rotate_matrix
//	vertex.x = copy.x * matrix[0][0] + copy.y * matrix[0][1] + copy.z * matrix[0][2];
//	vertex.y = copy.x * matrix[1][0] + copy.y * matrix[1][1] + copy.z * matrix[1][2];
//	vertex.z = copy.x * matrix[2][0] + copy.y * matrix[2][1] + copy.z * matrix[2][2];
//#undef matrix
//
//	return project32(vertex);
//}

vertex2 vertex_to_screen4(vertex3 vertex)
{
	vertex3 copy = vertex;
	copy -= g_camera3;
#define matrix g_camera_rotate_matrix
	vertex.x = copy.x * matrix[0][0] + copy.y * matrix[0][1] + copy.z * matrix[0][2];
	vertex.y = copy.x * matrix[1][0] + copy.y * matrix[1][1] + copy.z * matrix[1][2];
	vertex.z = copy.x * matrix[2][0] + copy.y * matrix[2][1] + copy.z * matrix[2][2];
#undef matrix
	//vertex += g_camera3;

	float fov_v = fov / 2 * PI / 180;

	float z_near = 0.01f;
	float z_far = INFINITY;

	float q = (z_far == INFINITY) ? 1 : (z_far / (z_far - z_near));

	//Normalize
	vertex.x *= g_height / (g_width * tanf(fov_v) * vertex.z);
	vertex.y /= tanf(fov_v) * vertex.z;
	//vertex.z = q * (1 - z_near / vertex.z);

	vertex.x = g_width / 2 * (1 + vertex.x);
	vertex.y = g_height / 2 * (1 + vertex.y);

	return vertex2{ vertex.x, vertex.y };
}






#define project43(vertex) (abort(), vertex3{})
#define vertex_to_screen vertex_to_screen4



template<class F, class ...Args>
void __debug_measure(const char* fname, F&& run, Args&&... args)
{
	auto t1 = std::chrono::high_resolution_clock::now();
	run(std::forward<Args&&...>(args...));
	auto t2 = std::chrono::high_resolution_clock::now();

	int64_t dur = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

	FILE* fp = fopen(fname, "a");
	fprintf(fp, "%llu\n", dur);
	fclose(fp);
}

#ifdef _DEBUG
#define debug_measure(fn, foo, ...) __debug_measure(fn, foo, __VA_ARGS__)
#else
#define debug_measure(fn, foo, ...) foo(__VA_ARGS__); ((void)0)
#endif

float get_vector_angle_cos(const vertex3& v1, const vertex3& v2)
{
	auto get_len = []
	(const vertex3& v)
	{
		return sqrt(v.x * v.x * v.y * v.y + v.z * v.z);
	};

	float scalar_multilpy = 0;

	scalar_multilpy += v1.x * v2.x;
	scalar_multilpy += v1.y * v2.y;
	scalar_multilpy += v1.z * v2.z;

	return scalar_multilpy / (get_len(v1) * get_len(v2));
}

struct vertex3_colored
{
	vertex3 vertex{};
	sf::Color color;
};

struct vertex2_colored
{
	vertex2 vertex{};
	sf::Color color;
};

int main()
{
#define matrix g_camera_rotate_matrix
	matrix[0][0] = matrix[1][1] = matrix[2][2] = 1;
	matrix[0][1] = matrix[0][2] = matrix[1][0] = matrix[1][2] = matrix[2][0] = matrix[2][1] = 0;
#undef matrix

	//get_2d_angle(0, 0);
	//get_2d_angle(PI, PI);
	//get_2d_angle(PI / 2, PI / 2);
	//get_2d_angle(PI / 3, PI / 3);
	//get_2d_angle(PI / 4, PI / 4);

	fclose(fopen("a.txt", "w"));
	fclose(fopen("b.txt", "w"));

	g_camera3.x = 300;
	g_camera3.y = 300;
	g_camera3.z = -300;

	g_camera_rotation_right = 0;
	g_camera_rotation_down = 0;
	g_camera_rotation_ccw = 0;

	g_width = 600;
	g_height = 600;

	vertex3 rotate3_origin{ 300, 300, 300 };
	fov = 90;

	//vertex3 vertex_debug;

	if (fov < 90)
	{
		fov = 90;
	}
	if (fov > 360)
	{
		fov = 360;
	}

	bool enable_moovable_camera = 1;
	bool enable_43_projection = 0;
	bool enable_32_projection = 1;
	bool enable_32_colored_projection = 0;
	bool enable_vertexes = 1;
	bool enable_colored_vertexes = 0;
	bool enable_edges = 1;
	bool enable_rotation_4 = 0;
	bool enable_rotatable_camera = 1;
	bool enable_mouse_grab = 1;
	bool enable_console_releasing = 0;
	bool enable_rotation_limits = 1;
	bool enable_draw_world_axis = 1;
	bool enable_clear_on_43_projection = 0;
	bool enable_clear_on_32_projection = 1;
	bool enable_clear_on_32_colored_projection = 1;
	bool enable_debug_logging = 1;
	bool enable_move_acceleration_on_ctrl = 1;

	if (enable_console_releasing)
	{
		FreeConsole();
	}

	int framerate = 60;

	float sensetivity = 1;

	float rotate_speed = 60.0 / framerate;

	const char* window_title = "2D";
	sf::RenderWindow win;
	win.create(sf::VideoMode(g_width, g_height), window_title, sf::Style::Close | sf::Style::Titlebar);
	win.setFramerateLimit(framerate);



	bool win_is_fullscreen = false;

	bool key_pressed[sf::Keyboard::KeyCount];
	memset(key_pressed, 0, sizeof(key_pressed));

	std::unordered_map<size_t, vertex2> vertexes2;
	std::unordered_map<size_t, vertex2_colored> vertexes2_colored;

	std::unordered_map<size_t, vertex3> vertexes3 = figure3_cube;
	std::unordered_map<size_t, vertex3_colored> vertexes3_colored;

	std::unordered_map<size_t, vertex4> vertexes4;// = figure4_tesseract;
	std::unordered_multimap<size_t, size_t> edges = edges3_cube;

	if (enable_mouse_grab)
	{
		win.setMouseCursorVisible(false);
		sf::Mouse::setPosition({ g_width / 2, g_height / 2 }, win);
		win.setMouseCursorGrabbed(true);
	}

	if (enable_draw_world_axis)
	{
		std::pair<size_t, vertex3_colored> pair;
		float length = 250;

		pair.first = vertexes3_colored.size() + 1;
		pair.second.vertex = vertex3{ 0, 0, 0 };
		pair.second.color = sf::Color(0, 0, -1);

		vertexes3_colored.emplace(pair);

		pair.first = vertexes3_colored.size() + 1;
		pair.second.vertex = vertex3{ length, 0, 0 };
		pair.second.color = sf::Color(-1, 0, 0);

		vertexes3_colored.emplace(pair);

		pair.first = vertexes3_colored.size() + 1;
		pair.second.vertex = vertex3{ 0, length, 0 };
		pair.second.color = sf::Color(0, -1, -1);

		vertexes3_colored.emplace(pair);

		pair.first = vertexes3_colored.size() + 1;
		pair.second.vertex = vertex3{ 0, 0, length };
		pair.second.color = sf::Color(0, -1, 0);

		vertexes3_colored.emplace(pair);
	}

	while (win.isOpen())
	{
		sf::Event ev;
		while (win.pollEvent(ev))
		{
			switch (ev.type)
			{
			case sf::Event::Closed:
				win.close();
				break;

			case sf::Event::KeyPressed:
				if (ev.key.code >= 0)
				{
					key_pressed[ev.key.code] = true;
				}

				switch (ev.key.code)
				{
				case sf::Keyboard::Enter:
					if (ev.key.alt)
					{
						win_is_fullscreen = !win_is_fullscreen;

						if (win_is_fullscreen)
						{
							win.create(sf::VideoMode(600, 600), window_title, sf::Style::Fullscreen);
						}
						else
						{
							win.create(sf::VideoMode(600, 600), window_title, sf::Style::Close | sf::Style::Titlebar);
						}

						win.setFramerateLimit(framerate);
					}
					break;

				case sf::Keyboard::Escape:
					win.close();
					break;
				}
				break;

			case sf::Event::KeyReleased:
				if (ev.key.code >= 0)
				{
					key_pressed[ev.key.code] = false;
				}
				break;

			case sf::Event::GainedFocus:
				if (enable_rotatable_camera && enable_mouse_grab)
				{
					sf::Mouse::setPosition({ g_width / 2, g_height / 2 }, win);
				}
				break;

			default:
				break;

			}
		}

		if (enable_rotation_4)
		{
			if (key_pressed[sf::Keyboard::Numpad4])
			{
				for (auto& vertex : vertexes4)
				{
					vertex.second = rotate4_xw_d(vertex.second, { 300, 300, 300, 300 }, rotate_speed);
				}
			}
			if (key_pressed[sf::Keyboard::Numpad6])
			{
				for (auto& vertex : vertexes4)
				{
					vertex.second = rotate4_xw_d(vertex.second, { 300, 300, 300, 300 }, -rotate_speed);
				}
			}
		}

		if (enable_rotatable_camera && enable_mouse_grab && win.hasFocus())
		{
			auto pos = sf::Mouse::getPosition(win);
			sf::Vector2i origin{ g_width / 2, g_height / 2 };
			pos -= origin;

			if (pos != sf::Vector2i{ 0, 0 })
			{
				g_camera_rotation_right += pos.x * 1.0 / g_width * PI / 4 * sensetivity;
				g_camera_rotation_down += pos.y * 1.0 / g_height * PI / 4 * sensetivity;

				sf::Mouse::setPosition(origin, win);
				g_camera_rotation_modified = true;
			}
		}

		if (enable_rotatable_camera)
		{
			float rotation_speed = 60 / (float)framerate;
			rotation_speed *= PI / 180;

			if (key_pressed[sf::Keyboard::Numpad8])
			{
				g_camera_rotation_down -= rotation_speed;
				g_camera_rotation_modified = true;
			}
			if (key_pressed[sf::Keyboard::Numpad2])
			{
				g_camera_rotation_down += rotation_speed;
				g_camera_rotation_modified = true;
			}
			if (key_pressed[sf::Keyboard::Numpad4])
			{
				g_camera_rotation_right -= rotation_speed;
				g_camera_rotation_modified = true;
			}
			if (key_pressed[sf::Keyboard::Numpad6])
			{
				g_camera_rotation_right += rotation_speed;
				g_camera_rotation_modified = true;
			}
			if (key_pressed[sf::Keyboard::Numpad1])
			{
				g_camera_rotation_ccw += rotation_speed;
				g_camera_rotation_modified = true;
			}
			if (key_pressed[sf::Keyboard::Numpad3])
			{
				g_camera_rotation_ccw -= rotation_speed;
				g_camera_rotation_modified = true;
			}
			if (key_pressed[sf::Keyboard::Numpad0])
			{
				g_camera_rotation_ccw = g_camera_rotation_down = g_camera_rotation_right = 0;
				g_camera_rotation_modified = true;
			}
		}

		if (enable_rotation_limits)
		{
			if (g_camera_rotation_down > PI / 2)
			{
				g_camera_rotation_down = PI / 2;
			}
			else if (g_camera_rotation_down < -PI / 2)
			{
				g_camera_rotation_down = -PI / 2;
			}
		}
		else
		{
			if (g_camera_rotation_modified)
			{
				g_camera_rotation_down = fmod(g_camera_rotation_down, PI / 2);
			}
		}

		if (g_camera_rotation_modified)
		{
			g_camera_rotation_right = fmod(g_camera_rotation_right, 2 * PI);

			vertex3 i = { 1, 0, 0 }, i2 = i;
			vertex3 j = { 0, 1, 0 }, j2 = j;
			vertex3 k = { 0, 0, 1 }, k2 = k;

			vertex3 zero = { 0, 0, 0 };

			i2 = rotate3_xz(i2, zero, g_camera_rotation_right);
			j2 = rotate3_xz(j2, zero, g_camera_rotation_right);
			k2 = rotate3_xz(k2, zero, g_camera_rotation_right);

			i2 = rotate3_yz(i2, zero, -g_camera_rotation_down);
			j2 = rotate3_yz(j2, zero, -g_camera_rotation_down);
			k2 = rotate3_yz(k2, zero, -g_camera_rotation_down);

			g_camera_rotate_matrix[0][0] = i2.x;
			g_camera_rotate_matrix[0][1] = j2.x;
			g_camera_rotate_matrix[0][2] = k2.x;

			g_camera_rotate_matrix[1][0] = i2.y;
			g_camera_rotate_matrix[1][1] = j2.y;
			g_camera_rotate_matrix[1][2] = k2.y;

			g_camera_rotate_matrix[2][0] = i2.z;
			g_camera_rotate_matrix[2][1] = j2.z;
			g_camera_rotate_matrix[2][2] = k2.z;

			g_camera_rotation_modified = false;
		}

		if (enable_moovable_camera)
		{
			float camera_speed = 500 / framerate;

			if (enable_move_acceleration_on_ctrl)
			{
				if (key_pressed[sf::Keyboard::LControl] || key_pressed[sf::Keyboard::RControl])
				{
					camera_speed *= 3;
				}
			}

			vertex3 move_vector{ 0, 0, 0 };

			if (key_pressed[sf::Keyboard::W])
			{
				move_vector.z += camera_speed;
			}
			if (key_pressed[sf::Keyboard::A])
			{
				move_vector.x -= camera_speed;
			}
			if (key_pressed[sf::Keyboard::S])
			{
				move_vector.z -= camera_speed;
			}
			if (key_pressed[sf::Keyboard::D])
			{
				move_vector.x += camera_speed;
			}
			if (key_pressed[sf::Keyboard::Space])
			{
				move_vector.y -= camera_speed;
			}
			if (key_pressed[sf::Keyboard::LShift] || key_pressed[sf::Keyboard::RShift])
			{
				move_vector.y += camera_speed;
			}

			vertex3 zero_point{ 0, 0, 0 };

			//move_vector = rotate3_xy(move_vector, zero_point, -g_camera_rotation_right);
			//move_vector = rotate3_xz(move_vector, zero_point, g_camera_rotation_ccw);
			//move_vector = rotate3_yz(move_vector, zero_point, g_camera_rotation_down);
			move_vector = rotate3_xz(move_vector, zero_point, -g_camera_rotation_right);

			g_camera3 += move_vector;
		}

		if (enable_43_projection)
		{
			if (enable_clear_on_43_projection)
			{
				vertexes3.clear();
			}
			for (const auto& pair : vertexes4)
			{
				vertexes3.emplace(std::pair<size_t, vertex3>{ pair.first, project43(pair.second) });
			}
		}

		if (enable_32_projection)
		{
			if (enable_clear_on_32_projection)
			{
				vertexes2.clear();
			}
			for (const auto& pair : vertexes3)
			{
				vertexes2.emplace(std::pair<size_t, vertex2>{ pair.first, vertex_to_screen(pair.second) });
			}
		}

		if (enable_32_colored_projection)
		{
			if (enable_clear_on_32_colored_projection)
			{
				vertexes2_colored.clear();
			}
			for (const auto& p : vertexes3_colored)
			{
				if (vertexes2_colored.count(p.first))
				{
					abort();
				}

				std::pair<size_t, vertex2_colored> result_pair;

				result_pair.first = p.first;
				result_pair.second.color = p.second.color;
				result_pair.second.vertex = vertex_to_screen(p.second.vertex);

				vertexes2_colored.emplace(result_pair);
			}
		}

		if (enable_debug_logging && !enable_console_releasing)
		{
			system("cls");
			dprintf("Debug info:\n");

			do_dprintf(g_camera3.x);
			do_dprintf(g_camera3.y);
			do_dprintf(g_camera3.z);
			do_dprintf(g_camera_rotation_right);
			do_dprintf(g_camera_rotation_down);
			do_dprintf(g_camera_rotation_ccw);
		}



		win.clear();

		if (enable_edges)
		{
			for (const auto& points : edges)
			{
				if (vertexes2.count(points.first) + vertexes2.count(points.second) != 2)
				{
					abort();
				}

				vertex2 from = vertexes2[points.first], to = vertexes2[points.second];
				draw_line(from.x, to.x, from.y, to.y, win);
			}
		}

		if (enable_vertexes)
		{
			for (const auto& pair : vertexes2)
			{
				draw_vertex(pair.second.x, pair.second.y, win);
			}
		}

		if (enable_colored_vertexes)
		{
			for (const auto& pair : vertexes2_colored)
			{
				draw_vertex(pair.second.vertex.x, pair.second.vertex.y, win, pair.second.color);
			}
		}

		win.display();
	}

}
