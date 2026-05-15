#include "../mktui.h"

int main() {
    mktui::console_guard console;
    mktui::set_title("mktui Demo");
    int x = 10;
    int y = 5;
    bool running = true;
    while (running) {
        mktui::clear_screen();
        mktui::set_cursor(2, 1);
        std::cout << mktui::attrs::bold << mktui::colors::bit16::cyan << "mktui Demo" << mktui::attrs::reset;
        mktui::set_cursor(2, 3);
        std::cout << "Arrow Keys : Move the box";
        mktui::set_cursor(2, 4);
        std::cout << "Mouse Click: Move box to cursor";
        mktui::set_cursor(2, 5);
        std::cout << "Q           : Quit";
        mktui::set_cursor(2, 7);
        std::cout << "Console Size: " << mktui::get_Console_Width() << "x" << mktui::get_Console_Height();
        mktui::set_cursor(2, 8);
        std::cout << "Mouse: " << mktui::get_MouseX() << ", " << mktui::get_MouseY();
        mktui::set_cursor(x, y);
        std::cout << mktui::colors::bit24::bg(255, 100, 100) << mktui::colors::bit8::white << " @ " << mktui::attrs::reset;
        std::cout.flush();
        mktui::event ev = mktui::get_event();
        switch (ev.input) {
        case mktui::Input_Type::MoveUp:
            y--;
            break;
        case mktui::Input_Type::MoveDown:
            y++;
            break;
        case mktui::Input_Type::MoveLeft:
            x--;
            break;
        case mktui::Input_Type::MoveRight:
            x++;
            break;
        case mktui::Input_Type::MouseLeftDown:
            x = ev.mouse_x;
            y = ev.mouse_y;
            break;
        case mktui::Input_Type::Q:
            running = false;
            break;
        default:
            break;
        }
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x > mktui::get_Console_Width() - 3)
            x = mktui::get_Console_Width() - 3;
        if (y > mktui::get_Console_Height() - 1)
            y = mktui::get_Console_Height() - 1;
        mktui::sleep_ms(16);
    }

    return 0;
}