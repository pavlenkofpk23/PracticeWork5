#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <cmath>
#include <algorithm>
#include <cwctype>
#include <clocale>
#include <limits>
#include <io.h>
#include <fcntl.h>

#ifdef _WIN32
#include <windows.h>
#endif

void clearInputBuffer() {
    std::wcin.clear();
    std::wcin.ignore((std::numeric_limits<std::streamsize>::max)(), L'\n');
}

std::wstring toUpper(std::wstring str) {
    for (auto& ch : str) {
        ch = std::towupper(ch);
    }
    return str;
}

struct Cell {
    std::wstring letter = L".";
    bool is_new = false;
};

class BaldaBoard {
private:
    static const int SIZE = 5;
    std::vector<std::vector<Cell>> grid;

public:
    BaldaBoard() : grid(SIZE, std::vector<Cell>(SIZE)) {}

    void startNewGame(const std::wstring& start_word) {
        std::wstring upper_word = toUpper(start_word);
        if (upper_word.length() != SIZE) {
            std::wcout << L"Помилка: Початкове слово має складатися з 5 літер !!!\n";
            return;
        }
        for (int j = 0; j < SIZE; ++j) {
            grid[2][j].letter = std::wstring(1, upper_word[j]);
        }
    }

    void display() const {
        std::wcout << L"\n    0 1 2 3 4\n  -----------\n";
        for (int i = 0; i < SIZE; ++i) {
            std::wcout << i << L" | ";
            for (int j = 0; j < SIZE; ++j) {
                std::wcout << grid[i][j].letter << L" ";
            }
            std::wcout << L"|\n";
        }
        std::wcout << L"  -----------\n";
    }

    bool placeLetter(int r, int c, const std::wstring& ch) {
        if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) {
            std::wcout << L"Помилка: Координати виходять за межі поля !!!\n";
            return false;
        }
        if (grid[r][c].letter != L".") {
            std::wcout << L"Помилка: Клітинка вже зайнята !!!\n";
            return false;
        }

        bool has_neighbor = false;
        int dr[] = { -1, 1, 0, 0 };
        int dc[] = { 0, 0, -1, 1 };

        for (int i = 0; i < 4; ++i) {
            int nr = r + dr[i];
            int nc = c + dc[i];
            if (nr >= 0 && nr < SIZE && nc >= 0 && nc < SIZE) {
                if (grid[nr][nc].letter != L".") {
                    has_neighbor = true;
                    break;
                }
            }
        }

        if (!has_neighbor) {
            std::wcout << L"Помилка: Нова літера повинна межувати з існуючими !!!\n";
            return false;
        }

        grid[r][c].letter = toUpper(ch);
        grid[r][c].is_new = true;
        return true;
    }

    bool validateWordPath(const std::wstring& word, const std::vector<std::pair<int, int>>& path, int new_r, int new_c) const {
        std::wstring upper_word = toUpper(word);
        if (upper_word.length() != path.size()) return false;

        bool used_new_cell = false;
        std::vector<std::vector<bool>> visited(SIZE, std::vector<bool>(SIZE, false));

        for (size_t i = 0; i < path.size(); ++i) {
            int r = path[i].first;
            int c = path[i].second;

            if (r < 0 || r >= SIZE || c < 0 || c >= SIZE) return false;
            if (grid[r][c].letter != std::wstring(1, upper_word[i])) return false;

            if (visited[r][c]) return false;
            visited[r][c] = true;

            if (i > 0) {
                int pr = path[i - 1].first;
                int pc = path[i - 1].second;
                if (std::abs(r - pr) + std::abs(c - pc) != 1) return false;
            }

            if (r == new_r && c == new_c) used_new_cell = true;
        }

        return used_new_cell;
    }

    void clearCell(int r, int c) {
        grid[r][c].letter = L".";
        grid[r][c].is_new = false;
    }

    void resetNewFlag(int r, int c) {
        grid[r][c].is_new = false;
    }
};

class BaldaGameManager {
private:
    BaldaBoard board;
    std::set<std::wstring> used_words;
    int player1_score = 0;
    int player2_score = 0;
    int current_player = 1;

public:
    void initGame(const std::wstring& start_word) {
        std::wstring upper_start = toUpper(start_word);
        board.startNewGame(upper_start);
        used_words.insert(upper_start);
        std::wcout << L"Гра розпочата! Початкове слово: " << upper_start << L" !!!\n";
    }

    void playTurn() {
        board.display();
        std::wcout << L"\n--- Хід Гравця " << current_player << L" ---\n";

        int r, c;
        std::wstring ch;
        std::wcout << L"Введіть рядок (0-4), стовпець (0-4) та нову літеру: ";
        if (!(std::wcin >> r >> c >> ch)) {
            std::wcout << L"Помилка некоректного введення даних !!!\n";
            clearInputBuffer();
            return;
        }

        if (ch.length() != 1) {
            std::wcout << L"Помилка: Потрібно ввести рівно один символ !!!\n";
            clearInputBuffer();
            return;
        }

        ch = toUpper(ch);

        if (!board.placeLetter(r, c, ch)) {
            std::wcout << L"Спробуйте ще раз !!!\n";
            return;
        }

        board.display();

        std::wstring word;
        std::wcout << L"Введіть утворене слово: ";
        if (!(std::wcin >> word)) {
            std::wcout << L"Помилка введення слова !!!\n";
            clearInputBuffer();
            board.clearCell(r, c);
            return;
        }

        word = toUpper(word);

        if (used_words.count(word)) {
            std::wcout << L"Помилка: Це слово вже використовувалося !!!\n";
            board.clearCell(r, c);
            return;
        }

        int path_len;
        std::wcout << L"Введіть довжину шляху слова (" << word.length() << L"): ";
        if (!(std::wcin >> path_len) || path_len != static_cast<int>(word.length())) {
            std::wcout << L"Помилка: Довжина шляху має відповідати довжині слова !!!\n";
            clearInputBuffer();
            board.clearCell(r, c);
            return;
        }

        std::vector<std::pair<int, int>> path(path_len);
        std::wcout << L"Введіть послідовність координат (ряд стовпець) для кожного символу:\n";
        for (int i = 0; i < path_len; ++i) {
            if (!(std::wcin >> path[i].first >> path[i].second)) {
                std::wcout << L"Помилка введення координат шляху !!!\n";
                clearInputBuffer();
                board.clearCell(r, c);
                return;
            }
        }

        if (board.validateWordPath(word, path, r, c)) {
            used_words.insert(word);
            int points = static_cast<int>(word.length());

            if (current_player == 1) player1_score += points;
            else player2_score += points;

            std::wcout << L"Успіх! Слово \"" << word << L"\" зараховано (+" << points << L" pts) !!!\n";
            board.resetNewFlag(r, c);
            current_player = (current_player == 1) ? 2 : 1;
        }
        else {
            std::wcout << L"Помилка: Некоректний шлях або слово не використовує нову літеру !!!\n";
            board.clearCell(r, c);
        }

        std::wcout << L"Поточний рахунок -> Гравець 1: " << player1_score << L" | Гравець 2: " << player2_score << L" !!!\n";
    }
};

int main() {
    std::setlocale(LC_ALL, "");

#ifdef _WIN32
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
#endif

    BaldaGameManager game;
    game.initGame(L"БАЛДА");

    wchar_t choice = L'y';
    while (choice == L'y' || choice == L'Y') {
        game.playTurn();
        std::wcout << L"\nПродовжити гру? (y/n): ";
        if (!(std::wcin >> choice)) {
            break;
        }
    }
    return 0;
}