#include "broker_game.h"

#include <memory>

game_ptr_t
game_init()
{
    clear_terminal();
    std::string _game_length, _initial_budget;
    size_t game_length;
    rubles initial_budget;
    std::cout << "Введите предварительные настройки игры\n\n";

    do {
        set_cursor_pos(0, 2);
        std::cout << std::string(get_term_size().first, ' ') << '\r';
        std::cout << "Продолжительность игры в месяцах: ";
        std::cin >> _game_length;
    } while (!parse<size_t>(_game_length, game_length));

    do {
        set_cursor_pos(0, 3);
        std::cout << std::string(get_term_size().first, ' ') << '\r';
        std::cout << "Изначальный бюджет фонда в рублях: ";
        std::cin >> _initial_budget;
    } while (!parse<rubles>(_initial_budget, initial_budget));

    getchar();

    if (get_term_size().first < 100)
    {
        set_tem_color_red();
        std::cout << "\nПРЕДУПРЕЖДЕНИЕ: эта игра расчитана для работы в терминале с шириной не менее 100 символов: измените размер окна для наилучшего результата";
        set_tem_color_default();
        std::cout << "\n\nНажмите ENTER, для начала игры...\n";
        getchar();
    }

    return std::make_unique<BrokerGame>(game_length, initial_budget);
}

// Constructors:

BrokerGame::BrokerGame(size_t game_end, rubles default_fund_budget) :
_market(new Market()), _fund(new Fund(default_fund_budget)), _game_duration(0),
_game_end(game_end), _status(game_status::PLAYING)
{}

// Destructor:

BrokerGame::~BrokerGame()
{
    delete _market;
    delete _fund;
}

Market *
BrokerGame::get_market()
{
    return _market;
}

Fund *
BrokerGame::get_fund()
{
    return _fund;
}

date_t
BrokerGame::get_date()
const
{
    return _market->get_date();
}

void
BrokerGame::step()
{
    _market->step();
    ++_game_duration;

    if (_game_duration == _game_end)
        _status = game_status::ENDED;
}

void
BrokerGame::_draw_available()
{
    std::cout << "        Инвестиции, доступные на рынке:\n";
    std::cout << table_header();

    for (auto& investment : _market->get_available())
        std::cout << get_table_style_info(investment, 0) << '\n';

    _cursor_pos.second = 3 + _market->get_available().size();
}

void
BrokerGame::_clear_second_column(int second_col)
{
    int cur_cursor_row = 0;
    set_cursor_pos(second_col, cur_cursor_row++);

    size_t n_lines = _market->get_available().size();
    size_t n_col = get_term_size().first - second_col;

    for (size_t i = 0; i < n_lines; ++i)
    {
        std::cout << std::string(n_col, ' ');
        set_cursor_pos(second_col, cur_cursor_row++);
    }

    set_cursor_pos(_cursor_pos.first, _cursor_pos.second);
}

void
BrokerGame::_draw_owned()
{
    int cur_cursor_row = 0;
    int second_col = 49;
    _clear_second_column(second_col);
    set_cursor_pos(second_col, cur_cursor_row++);

    auto fix_cursor_pos = [](pos_t& _cursor_pos, Market * _market) {
        _cursor_pos = std::make_pair(0, 3 + _market->get_available().size());
        set_cursor_pos(_cursor_pos.first, _cursor_pos.second);
    };

    if (_fund->get_owned().empty())
    {
        std::cout << "        Фонд не владеет никакими инвестициями\n";
        fix_cursor_pos(_cursor_pos, _market);
        return;
    }

    std::cout << "                   Инвестиции фонда:\n";
    set_cursor_pos(second_col, cur_cursor_row++);
    std::cout << std::left << std::setw(3)  << "n" << '|' << get_table_header()
              << '\n';
    set_cursor_pos(second_col, cur_cursor_row++);
    std::cout << std::string(3,  '-') << '+' << get_table_divider()
              << '\n';

    set_cursor_pos(second_col, cur_cursor_row++);
    for (auto& inv : _fund->get_owned())
    {
        std::cout << get_table_style_info(inv.first, inv.second) << '\n';
        set_cursor_pos(second_col, cur_cursor_row++);
    }

    fix_cursor_pos(_cursor_pos, _market);
}

void
BrokerGame::_draw_quick_stats()
{
    for (int i = 0; i < 5; ++i)
        std::cout << std::string(get_term_size().first, ' ') << '\n';

    _cursor_pos = std::make_pair(0, 3 + _market->get_available().size());
    set_cursor_pos(_cursor_pos.first, _cursor_pos.second);
    std::cout << "\nКраткая статистика:\n";    
    std::cout << "Внутриигровая дата: " << _market->get_date().get_formated() 
                                        << '\n';
    std::cout << "Баланс фонда: " << _fund->get_budget() << " руб" << '\n';

    std::cout << "Общий доход: "; 
    if (_fund->calc_earnings() > 0)
        set_tem_color_green();
    else if (_fund->calc_earnings() < 0)
        set_tem_color_red();

    std::cout << _fund->calc_earnings() << " руб" << '\n';
    set_tem_color_default();

    _cursor_pos.second += 5;
}

void
BrokerGame::_draw_console()
{
    set_cursor_pos(_cursor_pos.first, _cursor_pos.second);
    for (int i = 0; i < 3; ++i)
        std::cout << std::string(get_term_size().first, ' ') << '\n';
    set_cursor_pos(_cursor_pos.first, _cursor_pos.second);

    std::cout << "\nВведите команду (\"help\" для просмотра всех команд):\n";
    std::cout << "> ";
    _cursor_pos.first = 2;
    _cursor_pos.second += 2;
}

void
BrokerGame::draw_interface()
{
    clear_terminal();
    _draw_available();
    _draw_owned();
    _draw_quick_stats();
    _draw_console();
}

void
BrokerGame::_draw_help()
{
    static list<std::pair<std::string, std::string>> commands_help = {
        { "help", "Открыть меню помощи" },
        { "next", "Закончить ход и перейти к следующему месяцу" },
        { "stats", "Открыть страницу с полной статистикой" },
        { "buy [id] [n]", "Купить n инвестиций определенного id" },
        { "sell [id] [n]", "Продать n инвестиций определенного id" },
        { "add [amount]", "Добавить в фонд amount рублей" },
        { "end", "Закончить игру сейчас" }
    };

    int offset = 20;

    clear_terminal();
    std::cout << "Команда";
    set_cursor_pos(offset, 0);
    std::cout << "Описание";

    int current_row = 2;
    for (auto& command : commands_help)
    {
        set_cursor_pos(0, current_row);
        std::cout << command.first;
        set_cursor_pos(offset, current_row++);
        std::cout << command.second;
    }
    
    set_cursor_pos(0, current_row);
    std::cout << "\nНажмите ENTER, чтобы вернуться к игре...\n";
    getchar();
    draw_interface();
}

void
BrokerGame::_draw_full_stats()
{
    clear_terminal();
    int cursor_x = 0, cursor_y = 0;
    set_cursor_pos(cursor_x, cursor_y);

    rubles total_stocks = 0;
    rubles total_obligations = 0;
    rubles total_metals = 0;
    rubles total_currencies = 0;

    if (!_fund->get_owned().empty())
    {
        std::cout << "             Статистика по инвестициям фонда:\n";

        std::cout << std::left << std::setw(3)  << "n" << '|' << get_table_header()
                  << '|' << std::setw(25) << "Изменение цен\n";
        std::cout << std::string(3,  '-') << '+' << get_table_divider()
                  << '+' << std::string(13, '-') << '\n';

        for (auto& inv : _fund->get_owned())
        {
            std::cout << get_table_style_info(inv.first, inv.second);

            rubles price_change = inv.first->get_price_change();

            std::cout << '|';
            if (price_change > 0)
            {
                set_tem_color_green();
                std::cout << '+';
            }
            else if (price_change < 0)
            {
                set_tem_color_red();
            }
            std::cout << std::setw(12) << price_change << '\n';
            set_tem_color_default();

            std::string inv_type = get_inv_type(inv.first);

            if (inv_type == "Stock")
                total_stocks += price_change;
            else if (inv_type == "Obligation")
                total_obligations += price_change;
            else if (inv_type == "Metal")
                total_metals += price_change;
            else if (inv_type == "Currency")
                total_currencies += price_change;
        }
    }
    else
    {
        std::cout << "\nФонд не владеет никакими инвестициями\n";
    }

    std::cout << "\nСтатистика по категориям:\n";

    auto pick_color = [](rubles price_change) {
        if (price_change > 0)
            set_tem_color_green();
        else if (price_change < 0)
            set_tem_color_red();
    };

    auto print_price_change = [&](rubles price_change) {
        pick_color(price_change);
        if (price_change > 0)
            std::cout << '+';
        std::cout << price_change << '\n';
        set_tem_color_default();
    };

    int offset = 16;
    std::cout << std::left;
    std::cout << std::setw(offset) << "Stocks: ";
    print_price_change(total_stocks);
    std::cout << std::setw(offset) << "Obligations: ";
    print_price_change(total_obligations);
    std::cout << std::setw(offset) << "Metals: ";
    print_price_change(total_metals);
    std::cout << std::setw(offset) << "Currencies: ";
    print_price_change(total_currencies);

    std::cout << "\nНажмите ENTER, чтобы вернуться к игре...\n";
    getchar();
    draw_interface();
}

void
BrokerGame::command()
{
    std::string command;

    while (true)
    {
        std::getline(std::cin, command);
        std::vector<std::string> cmd_split = parse_into_words(command);
        size_t cmd_sz = cmd_split.size();

        if (cmd_sz == 0)
            continue;

        if (cmd_sz == 1 && cmd_split[0] == "help")
        {
            _draw_help();
        }
        else if (cmd_sz == 1 && cmd_split[0] == "next")
        {
            step();
            return;
        }
        else if (cmd_sz == 1 && cmd_split[0] == "end")
        {
            _status = game_status::ENDED;
            clear_terminal();
            return;
        }
        else if (cmd_sz == 1 && cmd_split[0] == "stats")
        {
            _draw_full_stats();
        }
        else if (cmd_sz == 1 && cmd_split[0] == "easter")
        {
            _draw_easter();
        }
        else if (cmd_sz >= 1 && cmd_split[0] == "buy")
        {
            _handle_cmd_buy(cmd_split);
        }
        else if (cmd_sz >= 1 && cmd_split[0] == "sell")
        {
            _handle_cmd_sell(cmd_split);
        }
        else if (cmd_sz >= 1 && cmd_split[0] == "add")
        {
            _handle_cmd_add(cmd_split);
        }
        else
        {
            after_cmd_msg("  Неизвестная команда!", set_tem_color_red);
        }
    }
}

void
BrokerGame::_handle_cmd_buy(const std::vector<std::string>& cmd)
{
    if (cmd.size() != 3)
    {
        after_cmd_msg("  Использование: buy [id] [n]", set_tem_color_red);
        return;
    }

    inv_id_t id_to_buy;
    if (!parse<inv_id_t>(cmd[1], id_to_buy))
    {
        after_cmd_msg("  Ошибка: Неверный id", set_tem_color_red);
        return;
    }

    int n_to_buy;
    if (!parse<int>(cmd[2], n_to_buy))
    {
        after_cmd_msg("  Ошибка: Неверное количество для покупки", 
                      set_tem_color_red);
        return;
    }

    investment_ptr_t inv_to_buy = find_by_id(_market->get_available(),
                                             id_to_buy);

    if (n_to_buy + _fund->get_owned()[inv_to_buy] >= 1000)
    {
        after_cmd_msg("  Ошибка: Максимаольное количество лотов во владении 999", 
                      set_tem_color_red);
        return;
    }

    if (_fund->buy(inv_to_buy, n_to_buy))
        after_cmd_msg("  Успешная покупка!", set_tem_color_green);
    else
        after_cmd_msg("  Ошибка: Невозможно совершить покупку!",
                       set_tem_color_red);

    _draw_owned();
    _draw_quick_stats();
    _draw_console();
}

void
BrokerGame::_handle_cmd_sell(const std::vector<std::string>& cmd)
{
    if (cmd.size() != 3)
    {
        after_cmd_msg("  Использование: sell [id] [n]", set_tem_color_red);
        return;
    }

    inv_id_t id_to_sell;
    if (!parse<inv_id_t>(cmd[1], id_to_sell))
    {
        after_cmd_msg("  Ошибка: Неверный id", set_tem_color_red);
        return;
    }

    int n_to_sell;
    if (!parse<int>(cmd[2], n_to_sell))
    {
        after_cmd_msg("  Ошибка: Неверное количество для продажи", 
                      set_tem_color_red);
        return;
    }

    investment_ptr_t inv_to_sell = find_by_id(_market->get_available(),
                                              id_to_sell);

    if (_fund->sell(inv_to_sell, n_to_sell, _tax_rate))
        after_cmd_msg("  Успешная продажа!", set_tem_color_green);
    else
        after_cmd_msg("  Ошибка: Фонд не владеет таким количеством акций!",       
                      set_tem_color_red);

    _draw_owned();
    _draw_quick_stats();
    _draw_console();
}

void
BrokerGame::_handle_cmd_add(const std::vector<std::string>& cmd)
{
    if (cmd.size() != 2)
    {
        after_cmd_msg("  Использование: add [amount]", set_tem_color_red);
        return;
    }

    rubles amount;
    if (!parse<rubles>(cmd[1], amount))
    {
        after_cmd_msg("  Ошибка: Неверная сумма", set_tem_color_red);
        return;
    }

    if (amount <= 0)
    {
        after_cmd_msg("  Ошибка: счёт можно пополнить только на положительную сумму", set_tem_color_red);
        return;
    }

    _fund->add_to_budget(amount);

    _draw_owned();
    _draw_quick_stats();
    _draw_console();
}

void
BrokerGame::after_cmd_msg(const std::string& msg, void color())
{
    // clear msg line
    set_cursor_pos(0, _cursor_pos.second + 1);
    std::cout << std::string(get_term_size().first, ' ');

    // write msg
    set_cursor_pos(0, _cursor_pos.second + 1);
    color();
    std::cout << msg;

    // clear console
    set_tem_color_default();
    set_cursor_pos(0, _cursor_pos.second);
    std::cout << std::string(get_term_size().first, ' ');
    std::cout << '\r' << "> ";
    _cursor_pos.first = 2;
    set_cursor_pos(_cursor_pos.first, _cursor_pos.second);
}

bool
BrokerGame::is_not_over()
{
    return (_status == game_status::PLAYING);
}

// TODO: Implement end screen
void
BrokerGame::end_screen()
{

}

void
BrokerGame::_draw_easter()
{
    clear_terminal();
    char decimal[] = {
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,44,49,44,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,46,46,46,46,46,46,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,46,105,102,76,49,10,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,46,59,49,116,116,116,116,76,76,102,49,44,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,59,102,102,102,102,102,44,10,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,58,76,67,76,102,116,
116,102,76,76,76,76,76,58,32,32,32,32,32,32,32,32,32,32,32,32,32,32,44,116,102,
102,102,102,102,102,105,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,44,67,67,67,76,102,49,49,102,102,49,116,102,102,59,32,32,32,32,32,
32,32,32,32,32,32,32,44,49,76,102,102,102,102,102,102,102,102,46,10,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,105,76,116,116,116,49,49,105,
102,76,76,76,102,102,116,46,32,32,32,32,32,32,32,32,32,32,32,58,105,105,116,102,
102,102,102,102,102,76,105,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,59,116,49,49,49,49,105,105,49,102,102,102,102,102,49,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,102,102,102,102,102,116,59,105,49,46,10,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,44,49,105,105,105,105,
105,105,49,116,116,49,105,116,44,32,32,32,32,32,32,32,32,32,32,32,32,32,32,105,
102,102,102,102,102,44,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,59,49,105,59,59,59,59,105,49,116,102,102,58,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,46,102,102,102,102,102,49,10,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,44,116,49,105,105,59,58,58,58,59,105,59,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,105,102,102,102,102,102,44,10,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,46,44,59,49,49,105,105,
105,59,58,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,46,102,102,102,102,
102,49,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,46,46,44,44,46,44,44,
46,44,105,102,116,105,116,71,105,46,46,46,32,32,32,32,32,32,32,32,32,32,32,32,
32,105,102,102,102,102,102,44,10,32,32,32,32,32,32,32,32,32,32,32,32,46,44,44,
44,58,58,58,58,44,44,44,44,46,46,59,76,76,76,71,116,58,58,58,58,44,44,44,46,32,
32,32,32,32,32,32,46,102,102,102,102,102,49,10,32,32,32,32,32,32,32,32,32,32,32,
32,59,44,44,44,44,44,44,44,44,44,44,44,44,44,46,44,59,116,102,105,44,58,58,44,
44,58,58,58,58,44,32,32,32,32,32,59,102,102,102,102,102,44,10,32,32,32,32,32,32,
32,32,32,32,32,32,58,58,44,44,44,44,44,44,44,44,46,46,44,58,44,44,46,58,102,116,
44,58,58,44,44,58,44,44,58,58,46,32,32,32,46,102,102,102,102,102,49,10,32,32,32,
32,32,32,32,32,32,32,32,32,44,105,58,44,44,44,44,44,44,44,44,46,46,44,44,44,44,
44,44,59,44,58,58,58,44,58,44,44,44,58,58,32,32,32,59,102,102,102,102,102,44,10,
32,32,32,32,32,32,32,32,32,32,32,32,46,49,58,44,44,44,44,44,44,44,44,46,46,44,
44,44,58,44,44,44,58,58,44,44,44,58,44,44,44,44,58,44,32,46,102,102,102,102,102,
49,10,32,32,32,32,32,32,32,32,32,32,32,32,32,59,59,58,44,44,44,44,44,58,44,32,
46,46,44,44,44,58,58,44,44,44,58,44,44,58,44,44,44,44,58,58,46,44,49,116,102,
102,102,58,10,32,32,32,32,32,32,32,32,32,32,32,32,32,44,49,59,58,44,44,44,44,59,
102,58,46,44,44,44,44,44,44,58,44,46,46,44,44,58,44,46,46,44,44,58,58,44,32,32,
46,46,44,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,59,105,58,58,44,44,44,44,
44,58,44,44,58,44,44,58,44,58,58,58,44,44,44,59,49,58,44,46,44,44,44,58,44,10,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,46,59,59,59,58,44,44,44,44,44,44,44,
44,44,58,58,58,58,58,58,58,58,58,58,76,71,67,59,44,44,44,44,58,46,10,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,46,59,59,58,44,44,46,44,44,44,44,44,44,44,
44,44,44,44,44,44,44,58,58,58,59,59,58,44,44,44,44,44,58,41};

    for (char i : decimal)
        std::cout << (char) i;

    std::cout << "\n\nНажмите ENTER, чтобы вернуться к игре...\n";
    getchar();
    draw_interface();
}
