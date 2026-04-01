#ifndef GAME_H
#define GAME_H

// Player : X
// CPU : O
// CPU - Player
// max - min: -> par = max, impar = min

#define PLAYER_PIECE 'X'
#define CPU_PIECE 'O'
#define EMPTY ' '

#include <vector>
#include <limits>
#include <thread>
#include <chrono>
#include <map>
#include <random>
#include <iostream>

using map = std::vector<std::vector<char>>;

namespace utils
{
    bool winner_column(int n_row, const map& in_map, const char& game_piece, const int& n)
    {
        for (int i = 0; i < n; i++)
            if (in_map[i][n_row] != game_piece)
                return false;

        return true;
    }

    bool winner_row(int n_col, const map& in_map, const char& game_piece, const int& n)
    {
        for (int i = 0; i < n; i++)
            if (in_map[n_col][i] != game_piece)
                return false;

        return true;
    }

    bool winner_diagonal(const map& in_map, const char& game_piece, const int& n)
    {
        for (int i = 0; i < n; i++)
            if (in_map[i][i] != game_piece)
                return false;

        return true;
    }

    bool winner_diagonal_reverse(const map& in_map, const char& game_piece, const int& n)
    {
        for (int i = 0; i < n; i++)
            if (in_map[i][n - i - 1] != game_piece)
                return false;

        return true;
    }

    bool check_winner(const map& in_map, int& winner, const int& n) // 1 == player, 2 == cpu
    {
        for (int i = 0; i < n; i++)
        {
            // Player
            if (winner_column(i, in_map, PLAYER_PIECE, n) || winner_row(i, in_map, PLAYER_PIECE, n))
            {
                winner = 1;
                return true;
            }
            // CPU
            else if (winner_column(i, in_map, CPU_PIECE, n) || winner_row(i, in_map, CPU_PIECE, n))
            {
                winner = 2;
                return true;
            }
        }

        // Player
        if (winner_diagonal(in_map, PLAYER_PIECE, n) || winner_diagonal_reverse(in_map, PLAYER_PIECE, n))
        {
            winner = 1;
            return true;
        }
        else if (winner_diagonal(in_map, CPU_PIECE, n) || winner_diagonal_reverse(in_map, CPU_PIECE, n))
        {
            winner = 2;
            return true;
        }

        return false;
    }

    bool map_is_full(const map& in_map, const int& n)
    {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (in_map[i][j] == EMPTY)
                    return false;

        return true;
    }
}

class node
{
public:
    int value;
    std::vector<node*> children;
    map table;

    node(int in_value, const map& in_table)://, const map& in_table) :
        value(in_value), children(), table(in_table)
    { }
};

class tree
{
public:

    tree(const int& in_n) :
        n(in_n), root(nullptr) { }
    
    ~tree()
    {
        if (root)
            erase_node(root);
    }

    map get_next_move(const map& in_map, const int& difficulty)
    {
        root = build_tree(in_map, 0, difficulty);
        alpha_beta_prune(root, 0, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

        map table = in_map;
        int mov_number = root->value;
        for (auto& c : root->children)
            if (c->value == mov_number)
            {
                table = c->table;
                break;
            }

        return table;
    }

private:
    int n;
    node* root;

    void erase_node(node*& in_node)
    {
        if (!in_node)
            return;

        for (auto& node : in_node->children)
            erase_node(node);

        delete in_node;
    }

    int count_options_to_win(const char& game_piece, const map& in_map)
    {
        int options = 0;

        map filled_table = fill_map_with_symbol(in_map, game_piece);

        for (size_t i = 0; i < n; i++)
        {
            if (utils::winner_column(i, filled_table, game_piece, n))
                options++;

            if (utils::winner_row(i, filled_table, game_piece, n))
                options++;
        }

        if (utils::winner_diagonal(filled_table, game_piece, n))
            options++;
        if (utils::winner_diagonal_reverse(filled_table, game_piece, n))
            options++;

        return options;
    }

    int alpha_beta_prune(node* in_node, int height, int alpha, int beta)
    {
        // Check all childs
        if (in_node->children.empty())
            return in_node->value;

        bool is_max = height % 2 == 0;
        int best_value = is_max ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

        for (auto it = in_node->children.begin();
            it != in_node->children.end() && (alpha < beta);
            it++)
        {
            int temp_value = alpha_beta_prune(*it, height + 1, alpha, beta);

            if (is_max) // Max level -> Edit alpha
            {
                alpha = std::max(temp_value, alpha);
                best_value = std::max(best_value, temp_value);
            }
            else // Min level -> Edit beta
            {
                beta = std::min(beta, temp_value);
                best_value = std::min(best_value, temp_value);
            }
        }

        in_node->value = best_value;
        return best_value;
    }

    std::vector<map> get_all_variants(map in_table, const char& player)
    {
        std::vector<map> vector_maps;

        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                if (in_table[i][j] == EMPTY)
                {
                    in_table[i][j] = player;
                    vector_maps.push_back(in_table);

                    in_table[i][j] = EMPTY;
                }
            }
        }

        return vector_maps;
    }

    node* build_tree(const map& in_map, int current_height, int max_height)
    {
        // There is a winner or a tie
        int winner = 0;
        //bool check_winner(const map & in_map, int& winner) // 1 == player, 2 == cpu
        if ((current_height >= max_height) ||
            utils::map_is_full(in_map, n) ||
            utils::check_winner(in_map, winner, n))
        {
            int player = count_options_to_win(PLAYER_PIECE, in_map);
            int CPU = count_options_to_win(CPU_PIECE, in_map);

            return new node(CPU - player, in_map);
        }


        char current_piece = EMPTY;

        if (current_height % 2 == 0)
            current_piece = CPU_PIECE;
        else
            current_piece = PLAYER_PIECE;

        auto variants = get_all_variants(in_map, current_piece);

        std::vector<node*> children;

        for (auto v : variants)
        {
            node* new_node = build_tree(v, current_height + 1, max_height);
            children.push_back(new_node);
        }

        node* new_node = new node(0, in_map);
        new_node->children = children;

        return new_node;
    }

    map fill_map_with_symbol(const map in_map, const char& game_piece)
    {
        map temp = in_map;

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (temp[i][j] == EMPTY)
                    temp[i][j] = game_piece;

        return temp;
    }
};

class game
{
public:
    bool is_playing, PLAYER_TURN, CPU_IS_THINKING;

    game() :
        n(0), table(), difficulty(0), is_playing(true), PLAYER_TURN(false), CPU_IS_THINKING(false)
    {}
    
    void create_game()
    {
        std::cout << "Enter game size: ";
        std::cin >> n;

        is_playing = true;
        PLAYER_TURN = false;
        CPU_IS_THINKING = false;
        table = std::vector(n, std::vector<char>(n, EMPTY));

        std::random_device rd;
        std::mt19937  gen(rd());
        std::uniform_int_distribution<> distrib(1, 2);

        int turn = distrib(gen);

        PLAYER_TURN = (turn % 2)? true : false;
        std::cout << ((turn % 2)? "PLAYER STARTS" : "CPU STARTS") << "\n";
    }

    void setup_game()
    {
        int opt = -1;
        std::cout << "Select difficulty:\n";
        std::cout << "1. Easy\n";
        std::cout << "2. Medium\n";
        std::cout << "3. Hard\n";
        std::cout << "Enter option: ";
        std::cin >> opt;

        switch (opt)
        {
        case 1:
            difficulty = 3;
            break;
        case 2:
            difficulty = 6;
            break;
        case 3:
            difficulty = 9;
            break;
        }
    }

    void is_there_any_winner()
    {
        int winner = -1;
        if (utils::check_winner(table, winner, n) || utils::map_is_full(table, n))
        {
            std::cout << "\n\n\n\n";

            if (winner == 1)
            {
                std::cout << "---------------------------\n";
                std::cout << "---------------------------\n";
                std::cout << "-----  PLAYER wins!  ------\n";
                std::cout << "---------------------------\n";
                std::cout << "---------------------------\n";
            }
            else if (winner == 2)
            {
                std::cout << "---------------------------\n";
                std::cout << "---------------------------\n";
                std::cout << "-------  CPU wins!  -------\n";
                std::cout << "---------------------------\n";
                std::cout << "---------------------------\n";
            }
            else
            {
                std::cout << "---------------------------\n";
                std::cout << "---------------------------\n";
                std::cout << "---  Nobody wins, tie!  ---\n";
                std::cout << "---------------------------\n";
                std::cout << "---------------------------\n";
            }

            is_playing = false;
            return;
        }
    }    

    void move_CPU()
    {
        std::cout << "\n\n\nCPU - CHAMBEANDING\n\n";
        choose_CPU_move(difficulty);

        PLAYER_TURN = true;
        CPU_IS_THINKING = false;

        is_there_any_winner();

        if (is_playing)
            std::cout << "PLAYER TURN\n";
    }
    
    void move_PLAYER(int row, int col)
    {
        if (row < 0 || row >= n || col < 0 || col >= n)
            return;

        if (table[row][col] == EMPTY)
        {
            table[row][col] = PLAYER_PIECE;
            PLAYER_TURN = false;

            is_there_any_winner();
        }
    }

    void draw_table()
    {
        for (int i = 0; i <= n; i++)
            std::cout << "-\t";
        std::cout << "\n";
    
        std::cout << "\n";
        std::cout << "\t";
        for (int i = 0; i < n; i++)
            std::cout << i << "\t";
        std::cout << "\n";
    
    
        for (int i = 0; i < n; i++)
        {
            std::cout << i << "\t";
            for (int j = 0; j < n; j++)
                std::cout << table[i][j] << "\t";
            std::cout << "\n";
        }
    
        for (int i = 0; i <= n; i++)
            std::cout << "-\t";
        std::cout << "\n";
    }

    map get_table()
    {
        return table;
    }

    size_t get_n()
    {
        return n;
    }
private:
    map table;
    size_t n, difficulty;

    void choose_CPU_move(int difficulty)
    {
        tree choosing_tree(n);
        table = choosing_tree.get_next_move(table, difficulty);
    }

};

#endif