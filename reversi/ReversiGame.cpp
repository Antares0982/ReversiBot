//
// Created by antares on 4/28/22.
//
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"


#include "ReversiGame.h"
#include "botconfig.h"
#include <fstream>
#include <regex>

std::vector<std::string> split(const std::string &s) {
    std::regex we("\\s+");
    return {std::sregex_token_iterator(s.begin(), s.end(), we, -1), std::sregex_token_iterator()};
}

static ReversiOneMove refine(const std::string &s) {
    auto t = split(s);
    if (t.size() < 2) return {-1, -1};
    return {std::stoi(t[t.size() - 2]), std::stoi(t[t.size() - 1])};
}

static const PicCreater ReversiMapPicture = []() {
    PicCreater ans(640, 640);
    int i = 0;
    for (int centerx = 40; centerx < 640; centerx += 80) {
        for (int centery = 40; centery < 640; centery += 80) {
            if (((i / 8) & 1) ^ (i & 1))
                ans.drawRectangle(centerx - 40, centery - 40, centerx + 39, centery + 39, {0xfa, 0xce, 0x87});
            else
                ans.drawRectangle(centerx - 40, centery - 40, centerx + 39, centery + 39, {0xe6, 0xd8, 0xad});
            ++i;
        }
    }

    return ans;
}();

moveInfo ReversiGame::move(const ReversiInput &_input) {
    renewTimeStamp();

    if (_input.who != player) {
        return {false, false, false, {}, {-1, -1}};
    }

    auto movelist = map.find_can_move_points(player);

    bool skipped = false;

    for (auto &&pr: movelist) {
        if (pr.x == _input.movex && pr.y == _input.movey) {
            auto newmap = map.reversiMove(player, {_input.movex, _input.movey});
            auto reversed = map.diffAll(newmap, pr);
            map.copyFrom(newmap);

            player = switchSide(player);
            if (!map.has_can_move_point(player)) {
                player = switchSide(player);
                skipped = true;
            }

            return {true, map.isStaled(), skipped, reversed, {_input.movex, _input.movey}};
        }
    }

    return {false, false, false, {}, {-1, -1}};
}

std::string ReversiGame::drawPic(MiraiCP::QQID id, std::vector<ReversiOneMove> &reversed, ReversiOneMove lastmove) {
    PicCreater pic(ReversiMapPicture);

    int i = 0;
    for (int centerx = 40; centerx < 640; centerx += 80) {
        int j = 0;
        for (int centery = 40; centery < 640; centery += 80) {
            if (map[i][j] == BLACK)
                pic.drawCircle(centerx, centery, 30, {0x00, 0x00, 0x00});
            else if (map[i][j] == WHITE)
                pic.drawCircle(centerx, centery, 30, {0xff, 0xff, 0xff});
            ++j;
        }
        ++i;
    }

    for (auto &&pr: reversed) {
        pic.drawCircle(40 + pr.x * 80, 40 + pr.y * 80, 7, {0x80, 0x00, 0x80});
    }

    pic.drawCircle(40 + lastmove.x * 80, 40 + lastmove.y * 80, 7, {0, 0, 0xff});
    std::string name = "data/" + std::to_string(id) + ".jpg";
    pic.save(name);
    return name;
}

std::string ReversiGame::drawPic(MiraiCP::QQID id) {
    PicCreater pic(ReversiMapPicture);

    int i = 0;
    for (int centerx = 40; centerx < 640; centerx += 80) {
        int j = 0;
        for (int centery = 40; centery < 640; centery += 80) {
            if (map[i][j] == BLACK)
                pic.drawCircle(centerx, centery, 30, {0x00, 0x00, 0x00});
            else if (map[i][j] == WHITE)
                pic.drawCircle(centerx, centery, 30, {0xff, 0xff, 0xff});
            ++j;
        }
        ++i;
    }

    std::string name = "data/" + std::to_string(id) + ".jpg";
    pic.save(name);
    return name;
}

std::string ReversiGame::getAInamesString() {
    std::string ans;
    std::string split;
    for (auto &&[k, v]: QQCONFIG.ai_list) {
        ans += split + k;
        split = "\n";
    }
    return ans;
}

ReversiInput ReversiGame::call_ai() {
    bool isblack;
    if (blackid == 0) {
        isblack = true;
    } else if (whiteid == 0) {
        isblack = false;
    } else {
        throw MiraiCP::IllegalStateException("无效，双方均是人类玩家", MIRAICP_EXCEPTION_WHERE);
    }

    auto arg = map.to_str(!isblack);

    // subprocess
    std::string command(ai.AIpath + " 5 " + arg);

    std::array<char, 1024> buffer = {0};
    std::string result;
    // s << "Opening reading pipe" << std::endl;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        throw ReversiAIException(command, MIRAICP_EXCEPTION_WHERE);
    }

    while (fgets(buffer.data(), 1024, pipe) != nullptr) {
        // std::cout << "Reading..." << std::endl;
        result += buffer.data();
    }
    auto returnCode = pclose(pipe);

    if (returnCode) {
        throw ReversiAIException("返回值异常，返回：" + std::to_string(returnCode) + ", 命令：" + command, MIRAICP_EXCEPTION_WHERE);
    }

    auto _input = refine(result);

    return {_input.x, _input.y, isblack ? BLACK : WHITE};
}

ReversiGame::ReversiAIException::ReversiAIException(
        const std::string &commandAndPath,
        MiraiCP::MiraiCPExceptionBase::string _filename,
        int _lineNum)
    : MiraiCPExceptionCRTP("调用黑白棋AI异常：" + commandAndPath,
                           std::move(_filename),
                           _lineNum) {}

#pragma clang diagnostic pop
