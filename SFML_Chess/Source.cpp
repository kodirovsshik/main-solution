#define NOMINMAX
#include <Windows.h>

#include <unordered_map>
#include <chrono>
#include <thread>

#include <SFML/Graphics.hpp>

#pragma warning(disable : 26812)

//class scratch
//{
//private:
//    
//    struct _block
//    {
//        uint8_t data[1024][3072];
//
//        void reset() noexcept;
//    };
//
//    class _index_handler
//    {
//    public:
//        std::unordered_map<uint64_t, struct _block*> *p_blocks;
//        int32_t first_index;
//
//        uint8_t* operator[](int32_t index);
//    };
//
//public:
//
//    scratch();
//
//    ~scratch();
//
//    void draw(sf::RenderTarget&, int scroll, int w, int h);
//
//    std::unordered_map<uint64_t, struct _block*> blocks;
//
//    class _index_handler operator[](int32_t index);
//};
//
//
//
//scratch::scratch()
//{
//    (*this)[0][0];
//}
//scratch::~scratch()
//{
//    for (auto p : blocks)
//    {
//        delete p.second;
//    }
//}
//
//
//
//void scratch::draw(sf::RenderTarget& target, int s, int w, int h)
//{
//    int starty = s, endy = starty + h;
//
//    while (starty < endy)
//    {
//        for (size_t i = 0; i < 1024; ++i)
//        {
//            int startx = 0, endx = w;
//            while (startx < endx)
//            {
//                sf::VertexArray arr;
//                arr.resize(1024);
//
//                uint8_t* data = (*this)[starty][startx];
//                for (size_t j = 0; j < 1024; ++j)
//                {
//                    arr[j] = sf::Vertex( sf::Vector2f{(float)startx + j, (float)starty + i}, sf::Color(data[0], data[1], data[2]) );
//                    data += 3;
//                }
//                startx += 1024;
//
//                target.draw(arr);
//            }
//        }
//        starty += 1024;
//    }
//}
//
//
//scratch::_index_handler scratch::operator[](int32_t index)
//{
//    class _index_handler handle;
//    
//    handle.first_index = index;
//    handle.p_blocks = &this->blocks;
//    
//    return handle;
//}
//
//
//uint8_t* scratch::_index_handler::operator[](int32_t index)
//{
//    uint64_t key;
//
//    key = uint32_t(index) >> 10;
//    key <<= 22;
//
//    key |= uint32_t(this->first_index) >> 10;
//
//    struct scratch::_block* p_block;
//
//    if (this->p_blocks->count(key) == 0)
//    {
//        p_block = new struct scratch::_block;
//        p_block->reset();
//        this->p_blocks->emplace(std::make_pair(key, p_block));
//    }
//    else
//    {
//        p_block = this->p_blocks->at(key);
//    }
//
//    return p_block->data[first_index & 1023] + (index & 1023) * 3;
//}
// 
//
//void scratch::_block::reset() noexcept
//{
//    memset(this, 0, sizeof(*this));
//}

template<class X>
class object_hasher
{
public:
	constexpr size_t operator()(const X& x) const noexcept
	{
		return std::_Hash_array_representation((const char*)std::addressof(x), sizeof(x));
	} 
};

int main()

//int WINAPI WinMain(
//    HINSTANCE hInstance,
//    HINSTANCE hPrevInstance,
//    LPSTR     lpCmdLine,
//    int       nShowCmd
//)

{

	int width = 800, heigth = 600;
	int scroll = 0;
	bool updated = true;
	sf::Color color1 = sf::Color::White, color2 = sf::Color::Black;



	std::unordered_map<std::pair<size_t, size_t>, sf::RenderTexture, object_hasher<std::pair<size_t, size_t>> > scratch;
	





	sf::Sprite scratch_sp;
	scratch_sp.setPosition(0, 0);
	


	sf::RenderWindow window;
	window.create(sf::VideoMode(width, heigth, 24), "");
	window.setVerticalSyncEnabled(true);
	


	std::vector<sf::VertexArray> undo_arr;
	size_t undo_index;

	undo_arr.reserve(512);



	sf::Event ev;
	while (window.isOpen())
	{
		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
		{
			auto pos = sf::Mouse::getPosition();

			sf::VertexArray arr;

		}

		window.clear(sf::Color::Black);
		if (updated)
		{
			updated = false;

		}
		window.draw(scratch_sp);
		window.display();

		while (window.pollEvent(ev))
		{
			printf("%i\n", ev.type);
			if (ev.type == ev.Closed)
			{
				window.close();
				break;
			}

			if (ev.type == ev.Resized)
			{
				sf::View view({0, (float)scroll, (float)ev.size.width, (float)ev.size.height + scroll});
				window.setView(view);
			}
		}


	}

	return 0;
}
