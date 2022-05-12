#pragma once

#ifndef REVERSI_REVERSIMAP_H
#define REVERSI_REVERSIMAP_H

#include <cstring>
#include <memory>
#include <vector>
#define REVERSI_GRID 8

enum GridType {
    WHITE = '/',
    EMPTY = '0',
    BLACK = '1',
};

struct ReversiOneMove {
    int x;
    int y;
    ReversiOneMove(int _x, int _y) : x(_x), y(_y) {}
};

struct scoreResult {
    int scoreBlack;
    int scoreWhite;
};

constexpr bool in_board(int xx, int yy) {
    return 0 <= xx && xx < REVERSI_GRID && 0 <= yy && yy < REVERSI_GRID;
}

constexpr GridType switchSide(GridType a) {
    return a == WHITE ? BLACK : WHITE;
}

class ReversiMap {
private:
    std::unique_ptr<char[]> map_uniptr;

public:
    ReversiMap() noexcept : map_uniptr(new char[REVERSI_GRID * REVERSI_GRID + 1]) {
        std::memset(&map_uniptr[0], EMPTY, sizeof(char) * (REVERSI_GRID * REVERSI_GRID));
        map_uniptr[REVERSI_GRID * REVERSI_GRID] = 0;
    }

    ReversiMap(const ReversiMap &m) noexcept : ReversiMap() {
        copyFrom(m);
    }

    ReversiMap(ReversiMap &&m) noexcept : map_uniptr(std::move(m.map_uniptr)) {}

    virtual ~ReversiMap() = default;

public:
    void copyFrom(const ReversiMap &m) noexcept {
        memcpy(&map_uniptr[0], &m.map_uniptr[0], REVERSI_GRID * REVERSI_GRID + 1);
    }

    bool search_directions_can_move(GridType side, int x, int y) const;

    std::vector<ReversiOneMove> find_can_move_points(GridType side) const;

    bool has_can_move_point(GridType side) const;

    ReversiMap reversiMove(GridType side, ReversiOneMove where) const;

    scoreResult sum_up() const;

    bool isStaled() const;

    ReversiOneMove mapDiff(const ReversiMap &otherMap) const;

    std::vector<ReversiOneMove> diffAll(const ReversiMap &, ReversiOneMove);

public:
    char *operator[](int x) {
        return &map_uniptr[x * REVERSI_GRID];
    }

    const char *operator[](int x) const {
        return &map_uniptr[x * REVERSI_GRID];
    }

    std::string to_str(bool _reverse = false, char seperator = ' ') const;

    bool isStarting() const {
        for (int i = 0; i < REVERSI_GRID * REVERSI_GRID; ++i) {
            if (map_uniptr[i] != EMPTY) {
                if (i == 27 || i == 28 || i == 35 || i == 36) continue;
                return false;
            }
        }

        return map_uniptr[27] == WHITE && map_uniptr[28] == BLACK && map_uniptr[35] == BLACK && map_uniptr[36] == WHITE;
    }

    void start() {
        memset(&map_uniptr[0], EMPTY, (REVERSI_GRID * REVERSI_GRID) * sizeof(char));
        map_uniptr[27] = WHITE;
        map_uniptr[28] = BLACK;
        map_uniptr[35] = BLACK;
        map_uniptr[36] = WHITE;
    }

    std::pair<long long, long long> to_sql_index() const {
        long long x = 0;
        long long y = 0;
        for (int i = 0; i < REVERSI_GRID * REVERSI_GRID; ++i) {
            long long foo = ((long long) 1) << (i - 1);
            switch (map_uniptr[i]) {
                case BLACK:
                    x |= foo;
                    break;
                case WHITE:
                    y |= foo;
                    break;
            }
        }
        return {x, y};
    }

private:
    void flip_y(int x, int y, int yy);

    void flip_x(int x, int y, int yy);

    void flip(int, int, int, int);
};


#endif // REVERSI_REVERSIMAP_H
