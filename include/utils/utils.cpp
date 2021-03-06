#include "utils.h"

void
fix_term_encoding()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
}

inv_id_t
iota()
{
    static inv_id_t _count = 1;
    return _count++;
}

void
clear_terminal()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

std::string
table_header()
{
    std::stringstream buffer;
    buffer << std::left 
           << std::setw(3)  << "Id"       << '|'
           << std::setw(13) << "Тип"      << '|'
           << std::setw(20) << "Название" << '|'
           << std::setw(11) << "Цена"     << '|'
           << std::setw(11) << "Доход"    << '|'
           << std::setw(7)  << "Риск"     << '\n';

    buffer << std::string(3,  '-') << '+'
           << std::string(10, '-') << '+'
           << std::string(12, '-') << '+'
           << std::string(7,  '-') << '+'
           << std::string(6,  '-') << '+'
           << std::string(4,  '-') << '\n';

    return buffer.str();
}

void
set_cursor_pos(int x, int y)
{
    printf("\033[%d;%dH", y + 1, x + 1);
}

std::pair<int, int>
get_term_size()
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    return std::make_pair(columns, rows);
#else
    struct winsize max;
    ioctl(0, TIOCGWINSZ , &max);
    return std::make_pair(max.ws_col, max.ws_row);
#endif
}

std::vector<std::string>
parse_into_words(const std::string& s)
{
    std::stringstream stream(s);
    std::string word;
    std::vector<std::string> words;
    while (!stream.eof())
    {
        stream >> word;
        words.push_back(word);
    }

    return words;
}

void
set_tem_color_default()
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 15);
#else
    std::cout << "\033[0m";
#endif
}

void
set_tem_color_green()
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
#else
    std::cout << "\033[0;32m";
#endif
}

void
set_tem_color_red()
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
#else
    std::cout << "\033[31m";
#endif
}

std::string
get_table_header()
{
    std::stringstream buffer;
    buffer << std::left
           << std::setw(3)  << "id"       << '|'
           << std::setw(13) << "Тип"      << '|'
           << std::setw(20) << "Название" << '|'
           << std::setw(11) << "Цена"     << '|'
           << std::setw(11) << "Доход"    << '|'
           << std::setw(8)  << "Риск";

    return buffer.str();
}

std::string
get_table_divider()
{
    std::stringstream buffer;
    buffer << std::string(3,  '-') << '+'
           << std::string(10, '-') << '+'
           << std::string(12, '-') << '+'
           << std::string(7,  '-') << '+'
           << std::string(6,  '-') << '+'
           << std::string(4,  '-');


    return buffer.str();
}