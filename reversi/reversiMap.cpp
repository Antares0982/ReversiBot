#include "reversiMap.h"
#include <string>

static const int directions[8][2] = {
        {-1, -1},
        {-1, 0},
        {-1, 1},
        {0, -1},
        {0, 1},
        {1, -1},
        {1, 0},
        {1, 1}};


bool ReversiMap::search_directions_can_move(GridType side, int x, int y) const {
    for (auto &&dir: directions) {
        for (int dum = 1; true; ++dum) {
            int xx = x + dum * dir[0];
            int yy = y + dum * dir[1];
            if (!in_board(xx, yy)) break;
            if (EMPTY == (*this)[xx][yy]) break;
            if (dum == 1 && side == (*this)[xx][yy]) break;
            if (side == (*this)[xx][yy]) return true;
        }
    }
    return false;
}

std::vector<ReversiOneMove> ReversiMap::find_can_move_points(GridType side) const {
    std::vector<ReversiOneMove> ans;
    for (int i = 0; i < REVERSI_GRID; ++i) {
        for (int j = 0; j < REVERSI_GRID; ++j) {
            if ((*this)[i][j] != EMPTY) continue;

            if (search_directions_can_move(side, i, j)) ans.emplace_back(i, j);
        }
    }

    return ans;
}

void ReversiMap::flip_y(int x, int y, int yy) {
    // x == xx, and y < yy
    auto a = (*this)[x][y];
    for (int i = y + 1; i < yy; ++i) {
        (*this)[x][i] = a;
    }
}

void ReversiMap::flip_x(int x, int xx, int y) {
    // y == yy, and x < xx
    auto a = (*this)[x][y];
    for (int i = x + 1; i < xx; ++i) {
        (*this)[i][y] = a;
    }
}

void ReversiMap::flip(int x, int y, int xx, int yy) {
    if (x == xx) {
        if (y > yy) std::swap(y, yy);
        flip_y(x, y, yy);
        return;
    }
    if (y == yy) {
        if (x > xx) std::swap(x, xx);
        flip_x(x, xx, y);
        return;
    }

    if (x > xx) {
        std::swap(x, xx);
        std::swap(y, yy);
    }

    auto a = (*this)[x][y];
    int dx = x + 1;
    if (yy > y) {
        //left up to right bottom
        int dy = y + 1;
        for (; dx < xx; ++dx, ++dy) {
            (*this)[dx][dy] = a;
        }
    } else {
        int dy = y - 1;
        for (; dx < xx; ++dx, --dy) {
            (*this)[dx][dy] = a;
        }
    }
}


ReversiMap ReversiMap::reversiMove(GridType side, ReversiOneMove where) const {
    auto dum = ReversiMap(*this);
    auto [x, y] = where;
    dum[x][y] = side;
    for (auto &&pr: directions) {
        int xx = where.x + pr[0];
        int yy = where.y + pr[1];
        if (!in_board(xx, yy)) continue;
        auto &&foo = dum[xx][yy];
        if (foo == EMPTY || foo == side) continue;
        while (true) {
            xx += pr[0];
            yy += pr[1];
            if (!in_board(xx, yy)) break;
            auto &&foo2 = dum[xx][yy];
            if (foo2 == EMPTY) break;
            if (foo2 == side) {
                dum.flip(x, y, xx, yy);
                break;
            }
        }
    }
    return dum;
}

bool ReversiMap::has_can_move_point(GridType side) const {
    for (int i = 0; i < REVERSI_GRID; ++i) {
        for (int j = 0; j < REVERSI_GRID; ++j) {
            if ((*this)[i][j] != EMPTY) continue;
            if (search_directions_can_move(side, i, j)) return true;
        }
    }
    return false;
}

scoreResult ReversiMap::sum_up() const {
    int a = 0, b = 0;
    for (int i = 0; i < REVERSI_GRID * REVERSI_GRID; ++i) {
        if (map_uniptr[i] == WHITE)
            ++b;
        else if (map_uniptr[i] == BLACK)
            ++a;
    }
    return {a, b};
}

bool ReversiMap::isStaled() const {
    return !has_can_move_point(BLACK) && !has_can_move_point(WHITE);
}

std::string ReversiMap::to_str(bool _reverse, char seperator) const {
    auto p = std::string(REVERSI_GRID * (REVERSI_GRID + 1) - 1, seperator);
    for (int i = 0; i < REVERSI_GRID; ++i) {
        for (int j = 0; j < REVERSI_GRID; ++j) {
            switch ((*this)[i][j]) {
                case EMPTY: {
                    p[i * (REVERSI_GRID + 1) + j] = '0';
                    break;
                }
                case WHITE: {
                    p[i * (REVERSI_GRID + 1) + j] = _reverse ? '1' : '2';
                    break;
                }
                default:
                    p[i * (REVERSI_GRID + 1) + j] = _reverse ? '2' : '1';
            }
        }
    }
    return p;
}

ReversiOneMove ReversiMap::mapDiff(const ReversiMap &otherMap) const {
    for (int i = 0; i < REVERSI_GRID * REVERSI_GRID; ++i) {
        if (map_uniptr[i] == EMPTY && otherMap.map_uniptr[i] != EMPTY) return {i / 8, i % 8};
        if (map_uniptr[i] != EMPTY && otherMap.map_uniptr[i] == EMPTY) return {i / 8, i % 8};
    }
    return {-1, -1};
}

std::vector<ReversiOneMove> ReversiMap::diffAll(const ReversiMap &_copy_map, ReversiOneMove thismove) {
    std::vector<ReversiOneMove> ans;
    for (int i = 0; i < REVERSI_GRID; ++i) {
        for (int j = 0; j < REVERSI_GRID; ++j) {
            if (thismove.x == i && thismove.y == j) continue;
            if ((*this)[i][j] != _copy_map[i][j]) ans.emplace_back(i, j);
        }
    }
    return ans;
}
