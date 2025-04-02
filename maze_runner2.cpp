#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

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

// Variáveis globais
Maze maze;
int num_rows;
int num_cols;
mutex maze_mutex;  // Protege o acesso ao labirinto

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
    lock_guard<mutex> lock(maze_mutex);
    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            cout << maze[i][j] << " ";
        }
        cout << endl;
    }
    cout << "\n";
    this_thread::sleep_for(chrono::milliseconds(100));  // Para visualização melhor
}

// Função para verificar se uma posição é válida
bool is_valid_position(int row, int col) {
    return row >= 0 && row < num_rows && col >= 0 && col < num_cols &&
           (maze[row][col] == 'x' || maze[row][col] == 's');
}

// Função para explorar o labirinto recursivamente
void walk(Position pos) {
    if (maze[pos.row][pos.col] == 's') {
        cout << "Saída encontrada em (" << pos.row << ", " << pos.col << ")!\n";
        return;
    }

    // Protege a atualização do labirinto
    {
        lock_guard<mutex> lock(maze_mutex);
        maze[pos.row][pos.col] = 'o';
    }

    print_maze();  // Mostra o estado atual do labirinto

    vector<Position> directions = {
        {pos.row - 1, pos.col},
        {pos.row + 1, pos.col},
        {pos.row, pos.col - 1},
        {pos.row, pos.col + 1}};

    vector<Position> valid_moves;
    for (const auto& next_pos : directions) {
        if (is_valid_position(next_pos.row, next_pos.col)) {
            valid_moves.push_back(next_pos);
        }
    }

    if (valid_moves.empty()) {
        return;  
    }

    // Explora o primeiro caminho na mesma thread
    Position first_move = valid_moves[0];

    // Cria threads para os caminhos adicionais
    vector<thread> threads;
    for (size_t i = 1; i < valid_moves.size(); ++i) {
        cout << "Criando thread para explorar (" << valid_moves[i].row << ", " << valid_moves[i].col << ")\n";
        threads.emplace_back(walk, valid_moves[i]);
    }

    // Explora o primeiro caminho antes de continuar com os outros
    walk(first_move);

    // Espera todas as threads terminarem
    for (auto& t : threads) {
        t.join();
    }

    // Marca a posição como explorada após a visita
    {
        lock_guard<mutex> lock(maze_mutex);
        maze[pos.row][pos.col] = '.';
    }
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

    walk(initial_pos);

    cout << "Exploração concluída!\n";
    return 0;
}