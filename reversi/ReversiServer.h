//
// Created by antares on 4/28/22.
//

#ifndef MIRAICP_REVERSISERVER_H
#define MIRAICP_REVERSISERVER_H

#include "ReversiGame.h"
#include <mutex>
#include <unordered_map>
#include <unordered_set>

using QQID = MiraiCP::QQID;
using s_QQID = long long;

namespace MiraiCP {
    class Contact;
    class MessageEvent;
    class GroupMessageEvent;
    class PrivateMessageEvent;
    class PlainText;
} // namespace MiraiCP

class ReversiServer {
    enum ReversiStatus {
        NO_STATUS,
        WAITING_SIDE,
        CHOOSING_AI,
        PVP_WAITING_RIVAL,
        PLAYING,
    };

private:
    // group id = -id
    std::unordered_map<s_QQID, ReversiGame> server_memory;
    std::unordered_map<s_QQID, ReversiStatus> status_memory;
    std::unordered_set<s_QQID> disabled;
    std::recursive_mutex mtx;

private:
    static ReversiServer reversiServer;

private:
    ReversiServer() = default;

public:
    static void advanceGame(MiraiCP::Contact *chat, s_QQID id, ReversiInput _input, bool isAI = false);

    static void cleanUp();

    static s_QQID to_sqqid(QQID id) {
        return static_cast<s_QQID>(id);
    }

    static void groupHandle(MiraiCP::GroupMessageEvent e);

    static void privateHandle(MiraiCP::PrivateMessageEvent e);

    static void helpMessage(MiraiCP::MessageEvent *e);

private:
    static void runReversiServerGroup(MiraiCP::GroupMessageEvent &e, const std::string &_type);

    static void runReversiServerPrivate(MiraiCP::PrivateMessageEvent &e);

    // 状态转移
    static void chooseSide(s_QQID hashid, MiraiCP::MessageEvent *evt, std::vector<MiraiCP::PlainText> &texts);

    static void chooseAi(s_QQID hashid, MiraiCP::Contact *chat, std::vector<MiraiCP::PlainText> &texts);

    static void rivalIn(s_QQID hashid, MiraiCP::MessageEvent *evt, std::vector<MiraiCP::PlainText> &texts);

    static void playing(s_QQID hashid, MiraiCP::MessageEvent *evt);

    static std::string rawDraw(s_QQID);
};


#endif //MIRAICP_REVERSISERVER_H
