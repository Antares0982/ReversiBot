//
// Created by antares on 4/28/22.
//

#ifndef MIRAICP_REVERSIGAME_H
#define MIRAICP_REVERSIGAME_H

#include "Exception.h"
#include "PicCreater.h"
#include "ReversiTools.h"
#include "botconfig.h"
#include "reversiMap.h"
#include <chrono>
#include <string>
#include <unordered_map>

struct ReversiInput {
    int movex;
    int movey;
    GridType who;
};

struct moveInfo {
    bool valid;
    bool end;
    bool skipped;
    std::vector<ReversiOneMove> reversed;
    ReversiOneMove moved;
};

struct AIinfo {
    std::string AIname;
    std::string AIpath;

    AIinfo() : AIname(), AIpath() {}

    AIinfo(std::string _ai_name, std::string _path) : AIname(std::move(_ai_name)), AIpath(std::move(_path)) {}
};


class ReversiGame {
public:
    // CRTP异常类，黑白棋AI调用异常
    class ReversiAIException : MiraiCP::MiraiCPExceptionCRTP<ReversiAIException> {
    public:
        explicit ReversiAIException(
                const std::string &commandAndPath,
                string _filename,
                int _lineNum);

        static std::string exceptionType() { return "ReversiAIException"; }
    };

private:
    ReversiMap map;
    GridType player = BLACK;
    AIinfo ai;
    decltype(std::chrono::system_clock::now()) timeStamp;
    MiraiCP::QQID blackid = 0;
    MiraiCP::QQID whiteid = 0;

public:
    ReversiGame() : map(), ai(), timeStamp(std::chrono::system_clock::now()) { map.start(); }

    ReversiGame(std::string _ai_name, std::string _ai_path, GridType AiSide = WHITE) : map(), player(switchSide(AiSide)), ai(std::move(_ai_name), std::move(_ai_path)), timeStamp(std::chrono::system_clock::now()) {}

public:
    void start() {
        map.start();
    }

    void setblackPlayer(MiraiCP::QQID _id) {
        blackid = _id;
    }

    void setwhitePlayer(MiraiCP::QQID _id) {
        whiteid = _id;
    }

    void setAIname(std::string name) {
        auto it = QQCONFIG.ai_list.find(name);
        if (it == QQCONFIG.ai_list.end()) throw MiraiCP::IllegalArgumentException("无效的AI名称：" + name, MIRAICP_EXCEPTION_WHERE);
        ai.AIname = std::move(name);
        ai.AIpath = it->second;
    }

    [[nodiscard]] MiraiCP::QQID getblackid() const {
        return blackid;
    }

    [[nodiscard]] MiraiCP::QQID getwhiteid() const {
        return whiteid;
    }

    [[nodiscard]] auto getTimeStamp() const {
        return timeStamp;
    }

    [[nodiscard]] scoreResult result() const {
        return map.sum_up();
    }

    [[nodiscard]] bool usingAI() const {
        return (whiteid & blackid) == 0 && !ai.AIname.empty();
    }

    moveInfo move(const ReversiInput &_input);

    std::string drawPic(MiraiCP::QQID id);

    std::string drawPic(MiraiCP::QQID id, std::vector<ReversiOneMove> &reversed, ReversiOneMove lastmove);

    ReversiInput call_ai();

private:
    void renewTimeStamp() {
        timeStamp = std::chrono::system_clock::now();
    }

public:
    static std::string getAInamesString();

    static std::vector<std::string> getAInames() {
        std::vector<std::string> ans;
        for (auto &&[k, v]: QQCONFIG.ai_list) {
            ans.emplace_back(k);
        }
        return ans;
    }
};


#endif //MIRAICP_REVERSIGAME_H
