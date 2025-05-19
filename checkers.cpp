#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <cctype>
#include <sstream>
#include <algorithm>

using namespace std;

const int BOARD_SIZE = 8;
const char EMPTY = '.';
const char WHITE = 'w';
const char BLACK = 'b';
const char WHITE_KING = 'W';
const char BLACK_KING = 'B';

mutex mtx;

struct Move {
    int x1, y1, x2, y2;
};

class Checkers {
    vector<vector<char>> board;
    char player, ai;

public:
    Checkers() {
        board.resize(BOARD_SIZE, vector<char>(BOARD_SIZE, EMPTY));
        setupBoard();
    }

    void setupBoard() {
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if ((i + j) % 2 == 1) {
                    if (i < 3) board[i][j] = BLACK;
                    else if (i > 4) board[i][j] = WHITE;
                }
            }
        }
    }

    void printBoard() {
        cout << "  H G F E D C B A\n";
        for (int i = 0; i < BOARD_SIZE; ++i) {
            cout << 8 - i << " ";
            for (int j = BOARD_SIZE - 1; j >= 0; --j) {
                cout << board[i][j] << " ";
            }
            cout << 8 - i << "\n";
        }
        cout << "  H G F E D C B A\n";
    }

    pair<int, int> parseCoord(const string &coord) {
        int y = toupper(coord[0]) - 'A';
        int x = 8 - (coord[1] - '0');
        return {x, y};
    }

    bool isValidMove(int x1, int y1, int x2, int y2, char turn) {
        if (x2 < 0 || x2 >= BOARD_SIZE || y2 < 0 || y2 >= BOARD_SIZE)
            return false;
        char piece = board[x1][y1];
        if (board[x2][y2] != EMPTY) return false;
        int dx = x2 - x1;
        int dy = y2 - y1;

        if (tolower(piece) != turn) return false;

        bool isKing = isupper(piece);

        if (abs(dx) == 1 && abs(dy) == 1) {
            if (!isKing && ((turn == WHITE && dx != -1) || (turn == BLACK && dx != 1))) return false;
            return true;
        }

        if (abs(dx) == 2 && abs(dy) == 2) {
            int mx = x1 + dx / 2;
            int my = y1 + dy / 2;
            char middle = board[mx][my];
            if (tolower(middle) == EMPTY || tolower(middle) == turn) return false;
            return true;
        }

        return false;
    }

    void makeMove(int x1, int y1, int x2, int y2) {
        char piece = board[x1][y1];
        board[x2][y2] = piece;
        board[x1][y1] = EMPTY;

        if (abs(x2 - x1) == 2) {
            int mx = (x1 + x2) / 2;
            int my = (y1 + y2) / 2;
            board[mx][my] = EMPTY;
        }

        if (piece == WHITE && x2 == 0) board[x2][y2] = WHITE_KING;
        if (piece == BLACK && x2 == 7) board[x2][y2] = BLACK_KING;
    }

    void getAllMoves(char turn, vector<Move> &moves) {
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                if (tolower(board[i][j]) == turn) {
                    for (int dx = -2; dx <= 2; ++dx) {
                        for (int dy = -2; dy <= 2; ++dy) {
                            if (dx != 0 && dy != 0 && abs(dx) == abs(dy)) {
                                int ni = i + dx, nj = j + dy;
                                if (isValidMove(i, j, ni, nj, turn)) {
                                    moves.push_back({i, j, ni, nj});
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void aiMove() {
        vector<Move> moves;
        getAllMoves(ai, moves);
        if (moves.empty()) {
            cout << "\n\tHas ganado, la IA no tiene movimientos!\n";
            exit(0);
        }
        // Algoritmo paralelo para evaluar jugadas
        Move bestMove = moves[0];
        thread t([&]() {
            lock_guard<mutex> lock(mtx);
            bestMove = moves[rand() % moves.size()];
        });
        t.join();
        makeMove(bestMove.x1, bestMove.y1, bestMove.x2, bestMove.y2);
        cout << "\nLa IA ha jugado.\n";
    }

    void play() {
        string choice;
        cout << "\nElige color (blancas / negras): ";
        cin >> choice;

        transform(choice.begin(), choice.end(), choice.begin(), ::tolower);
        player = (choice == "blancas") ? WHITE : BLACK;
        ai = (player == WHITE) ? BLACK : WHITE;

        if (player == BLACK) aiMove();

        while (true) {
            printBoard();
            string from, to;
            cout << "\nIngresa tu movimiento (ej. A6 B5): ";
            cin >> from >> to;
            auto [x1, y1] = parseCoord(from);
            auto [x2, y2] = parseCoord(to);
            if (!isValidMove(x1, y1, x2, y2, player)) {
                cout << "\nMovimiento no valido, intenta de nuevo.\n";
                continue;
            }
            makeMove(x1, y1, x2, y2);
            aiMove();
        }
    }
};

int main() {
    Checkers game;
    game.play();
    return 0;
}

