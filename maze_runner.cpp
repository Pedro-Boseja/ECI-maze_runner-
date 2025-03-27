#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <thread>
#include <chrono>
#include <string>

using namespace std;

//conteúdo do labirinto:

// 'x': caminho válido
// '#': parede
// 'e': entrada do labirinto
// 's': saída do labirinto

//  Para cada nova posição explorada, o programa deve imprimir o labirinto atualizado:

// '.': posições já exploradas
// 'o': posição corrente

// Representação do labirinto
using Maze = vector<vector<char>>;

struct Position {
    int row;
    int col;
};

Maze maze;
int num_rows;
int num_cols;
stack<Position> valid_positions;

// Função para carregar o labirinto de um arquivo
Position load_maze(const string& file_name) {
    ifstream labirinto(file_name);

    if (!labirinto) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        return {-1, -1};
    }

    if (!(labirinto >> num_rows >> num_cols)) {
        cerr << "Erro: formato inválido no cabeçalho do arquivo!" << endl;
        return {-1, -1};
    }

    maze.resize(num_rows, vector<char>(num_cols));
    Position start = {-1, -1};

    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            labirinto >> maze[i][j];
            if (maze[i][j] == 'e') {
                start = {i, j};
            }
        }
    }
    labirinto.close();
    return start;
}

// Função para imprimir o labirinto
void print_maze() {
    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            cout << maze[i][j];
        }
        cout << "\n" << endl;
    }
    cout << "\n" << endl;
}

// Função para verificar se uma posição é válida
bool is_valid_position(int row, int col) {
    if (row >= 0 && row < num_rows && col >= 0 && col < num_cols) {
        if (maze[row][col] == 'x' || maze[row][col] == 's') {
            return true;
        }
    } else {
        cout << "Posição fora dos limites do labirinto" << endl;
    }
    return false;
}

//A função walk é usada para percorrer o labirinto a partir de uma posição inicial até encontrar a saída
bool walk(Position pos) {
    if (maze[pos.row][pos.col] == 's') {
        return true;
    }

    char original = maze[pos.row][pos.col];
    maze[pos.row][pos.col] = 'o';
    print_maze();
    this_thread::sleep_for(chrono::milliseconds(50));
    
    if (original == 'x' || original == 'e') {
        maze[pos.row][pos.col] = '.';
    }
    
    vector<Position> directions = {
        {pos.row - 1, pos.col},
        {pos.row + 1, pos.col},
        {pos.row, pos.col - 1},
        {pos.row, pos.col + 1}
    };

    for (const auto& next_pos : directions) {
        if (is_valid_position(next_pos.row, next_pos.col)) {
            valid_positions.push(next_pos);
        }
    }

    while (!valid_positions.empty()) {
        Position next = valid_positions.top();
        valid_positions.pop();

        if (walk(next)) {
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <arquivo_labirinto>" << endl;
        return 1;
    }

    Position initial_pos = load_maze(argv[1]);
    
    if (initial_pos.row == -1 || initial_pos.col == -1) {
        cerr << "Posição inicial não encontrada no labirinto." << endl;
        return 1;
    }

    bool exit_found = walk(initial_pos);

    if (exit_found) {
        cout << "Saída encontrada!" << endl;
    } else {
        cout << "Não foi possível encontrar a saída." << endl;
    }

    return 0;
}