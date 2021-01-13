#include <Windows.h>


#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

struct switch_button
{
    bool isactive;
    switch_button* p_prev, * p_next;

    void (*switch_handle)(bool is_active, void* parameter);
};

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd
)
{


    


	return 0;
}