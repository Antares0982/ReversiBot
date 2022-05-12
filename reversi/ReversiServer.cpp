//
// Created by antares on 4/28/22.
//
#include "ReversiServer.h"
#include "Contact.h"
#include "Event.h"
#include "ReversiTools.h"


ReversiServer ReversiServer::reversiServer;

void ReversiServer::cleanUp() {
    std::lock_guard<std::recursive_mutex> lk(reversiServer.mtx);
    reversiServer.server_memory.clear();
    reversiServer.status_memory.clear();
}

void ReversiServer::advanceGame(MiraiCP::Contact *chat, s_QQID id, ReversiInput _input, bool isAI) {
    if (!chat) return;

    auto &memory = reversiServer.server_memory;
    auto &game = reversiServer.server_memory[id];

    auto [success, end, skipped, reversed, lastmove] = game.move(_input);

    if (!success) {
        chat->sendMessage("无效移动");
        return;
    }

    if (end) {
        auto count = game.result();
        if (count.scoreBlack > count.scoreWhite)
            chat->sendMessage("黑方胜利！");
        else if (count.scoreWhite > count.scoreBlack)
            chat->sendMessage("白方胜利！");
        else
            chat->sendMessage("平局！");
        chat->sendMessage(chat->uploadImg(game.drawPic(id, reversed, lastmove)));
        memory.erase(id);
        reversiServer.status_memory.erase(id);
        return;
    }

    if (skipped) {
        chat->sendMessage("无棋可下，跳过一回合");
        if (game.usingAI() && isAI) {
            chat->sendMessage("等待AI……");
            auto result = game.call_ai();
            advanceGame(chat, id, result, true);
        }
    }

    chat->sendMessage(chat->uploadImg(game.drawPic(id, reversed, lastmove)));

    if (game.usingAI() && !isAI && !skipped) {
        chat->sendMessage("等待AI……");
        auto result = game.call_ai();
        advanceGame(chat, id, result, true);
    }
}

std::string ReversiServer::rawDraw(s_QQID id) {
    auto &mem = reversiServer.server_memory;
    if (mem.find(id) == mem.end()) return "";
    auto &game = mem[id];
    return game.drawPic(id);
}

void ReversiServer::helpMessage(MiraiCP::MessageEvent *e) {
    e->chat()->sendMessage("群聊中使用\"reversi pvp\"或\"reversi pve\"开始游戏，用\"reversi\"加入游戏。私聊时使用\"reversi\"开始游戏。用\"abort\"放弃或终止一场超时的游戏。");
}

void ReversiServer::groupHandle(MiraiCP::GroupMessageEvent e) {
    std::lock_guard<std::recursive_mutex> lk(reversiServer.mtx);

    s_QQID hashid = -to_sqqid(e.group.id());

    if (e.sender.id() == QQCONFIG.MASTERID && !e.message.empty() && e.message[0].type() == MiraiCP::PlainText::type()) {
        auto msg = e.message[0].get<MiraiCP::PlainText>().content;
        if (ReversiTools::isPrefixOf("disable", msg)) {
            reversiServer.disabled.insert(hashid);
            reversiServer.server_memory.erase(hashid);
            reversiServer.status_memory.erase(hashid);
            e.group.sendMessage("已关闭");
            return;
        } else if (ReversiTools::isPrefixOf("enable", msg)) {
            reversiServer.disabled.erase(hashid);
            e.group.sendMessage("已开启");
            return;
        }
    }

    if (reversiServer.disabled.find(hashid) != reversiServer.disabled.end()) return;

    bool activate = (reversiServer.server_memory.find(hashid) != reversiServer.server_memory.end());

    std::string activateType;

    auto textMessages = e.message.filter<MiraiCP::PlainText>();

    if (!textMessages.empty() && ReversiTools::isPrefixOf("help", textMessages[0].content)) {
        helpMessage(&e);
        return;
    }

    if (!activate) { // 判断游戏是否现在开始
        activate = std::any_of(textMessages.begin(), textMessages.end(), [&](const auto &m) {
            if (m.content == "reversi pvp" || m.content == "reversi pve") {
                activateType = m.content;
                return true;
            }
            return false;
        });
    } else { // 判断游戏是否终止
        auto &game = reversiServer.server_memory[hashid];
        auto t = ReversiTools::getNowTimeStamp();
        if (ReversiTools::timeDiff(t, game.getTimeStamp()) > 180 || e.sender.id() == game.getblackid() || e.sender.id() == game.getwhiteid()) {
            for (auto &&m: textMessages) {
                if (m.content == "abort") {
                    reversiServer.server_memory.erase(hashid);
                    reversiServer.status_memory.erase(hashid);
                    if (ReversiTools::timeDiff(t, game.getTimeStamp()) > 180)
                        e.group.sendMessage("超过三分钟未响应，游戏结束");
                    else
                        e.group.sendMessage("游戏结束");
                    return;
                }
            }
        }
    }

    if (!activate) return;

    runReversiServerGroup(e, activateType);
}

void ReversiServer::runReversiServerGroup(MiraiCP::GroupMessageEvent &e, const std::string &_type) {
    s_QQID hashid = -to_sqqid(e.group.id());

    auto &mem = reversiServer.server_memory;

    if (mem.find(hashid) == mem.end()) {
        // start new game
        auto &game = mem[hashid];
        game.start();
        if (_type == "reversi pvp") {
            game.setblackPlayer(e.sender.id());
            reversiServer.status_memory[hashid] = PVP_WAITING_RIVAL;
            e.group.sendMessage("黑棋加入了！");
        } else if (_type == "reversi pve") {
            e.group.sendMessage("回复 黑 或 白 选边，黑为先手");
            reversiServer.status_memory[hashid] = WAITING_SIDE;
        } else {
            throw MiraiCP::IllegalStateException("无效的黑白棋状态：unreachable code", MIRAICP_EXCEPTION_WHERE);
        }
        return;
    }

    auto &game = mem[hashid];
    auto &status = reversiServer.status_memory[hashid];

    auto texts = e.message.filter<MiraiCP::PlainText>();

    switch (status) {
        case NO_STATUS: {
            throw MiraiCP::IllegalStateException("无效的黑白棋状态：no status", MIRAICP_EXCEPTION_WHERE);
        }
        case WAITING_SIDE: {
            chooseSide(hashid, &e, texts);
            return;
        }
        case CHOOSING_AI: {
            chooseAi(hashid, &e.group, texts);
            return;
        }
        case PVP_WAITING_RIVAL: {
            rivalIn(hashid, &e, texts);
            return;
        }
        case PLAYING: {
            playing(hashid, &e);
            return;
        }
    }

    throw MiraiCP::IllegalStateException("unreachable code", MIRAICP_EXCEPTION_WHERE);
}

void ReversiServer::privateHandle(MiraiCP::PrivateMessageEvent e) {
    std::lock_guard<std::recursive_mutex> lk(reversiServer.mtx);

    auto hashid = to_sqqid(e.sender.id());

    auto textMessages = e.message.filter<MiraiCP::PlainText>();
    if (!textMessages.empty() && ReversiTools::isPrefixOf("help", textMessages[0].content)) {
        helpMessage(&e);
        return;
    }

    bool activate = false;

    auto &mem = reversiServer.server_memory;
    if (mem.find(hashid) != mem.end()) activate = true;

    if (!activate) {
        activate = std::any_of(textMessages.begin(), textMessages.end(), [](const auto &m) {
            return ReversiTools::isPrefixOf("reversi", m.content);
        });
    } else {
        if (std::any_of(textMessages.begin(), textMessages.end(), [](const auto &m) {
                return m.content == "abort";
            })) {
            mem.erase(hashid);
            reversiServer.status_memory.erase(hashid);
            e.sender.sendMessage("游戏结束");
            return;
        }
    }

    if (!activate) return;

    runReversiServerPrivate(e);
}

void ReversiServer::runReversiServerPrivate(MiraiCP::PrivateMessageEvent &e) {
    s_QQID hashid = to_sqqid(e.sender.id());

    auto &mem = reversiServer.server_memory;
    if (mem.find(hashid) == mem.end()) {
        // start new game
        auto &game = mem[hashid];
        e.sender.sendMessage("回复 黑 或 白 选边，黑为先手");
        reversiServer.status_memory[hashid] = WAITING_SIDE;
        return;
    }

    auto texts = e.message.filter<MiraiCP::PlainText>();

    auto &status = reversiServer.status_memory[hashid];

    switch (status) {
        case NO_STATUS: {
            throw MiraiCP::IllegalStateException("无效的黑白棋状态：no status", MIRAICP_EXCEPTION_WHERE);
        }
        case WAITING_SIDE: {
            chooseSide(hashid, &e, texts);
            return;
        }
        case CHOOSING_AI: {
            chooseAi(hashid, &e.sender, texts);
            return;
        }
        case PVP_WAITING_RIVAL: {
            throw MiraiCP::IllegalStateException("无效的黑白棋状态：private message event waiting rival", MIRAICP_EXCEPTION_WHERE);
        }
        case PLAYING: {
            playing(hashid, &e);
            return;
        }
    }

    throw MiraiCP::IllegalStateException("unreachable code", MIRAICP_EXCEPTION_WHERE);
}

void ReversiServer::chooseSide(s_QQID hashid, MiraiCP::MessageEvent *evt, std::vector<MiraiCP::PlainText> &texts) {
    auto &game = reversiServer.server_memory[hashid];
    auto &status = reversiServer.status_memory[hashid];

    std::string side;
    if (!std::any_of(texts.begin(), texts.end(), [&](const auto &_e) {
            if (_e.content == "黑" || _e.content == "白") {
                side = _e.content;
                return true;
            }
            return false;
        }))
        return;

    if (side == "黑") {
        game.setblackPlayer(evt->from()->id());
        game.setwhitePlayer(0);
    } else if (side == "白") {
        game.setwhitePlayer(evt->from()->id());
        game.setblackPlayer(0);
    } else {
        throw MiraiCP::IllegalStateException("状态异常：黑白棋选边", MIRAICP_EXCEPTION_WHERE);
    }

    auto names = ReversiGame::getAInamesString();
    evt->chat()->sendMessage("请选择AI，可选的AI有\n" + names);
    status = CHOOSING_AI;
}

void ReversiServer::chooseAi(s_QQID hashid, MiraiCP::Contact *chat, std::vector<MiraiCP::PlainText> &texts) {
    auto &game = reversiServer.server_memory[hashid];
    auto &status = reversiServer.status_memory[hashid];

    std::string ai;
    if (!std::any_of(texts.begin(), texts.end(), [&](const auto &txt) {
            for (auto &&v: ReversiGame::getAInames()) {
                if (v == txt.content) {
                    ai = txt.content;
                    return true;
                }
            }
            return false;
        }))
        return;

    status = PLAYING;
    game.setAIname(std::move(ai));
    game.start();
    if (game.getblackid() == 0) {
        auto input = game.call_ai();
        advanceGame(chat, hashid, input, true);
    } else {
        chat->sendMessage(chat->uploadImg(rawDraw(hashid)));
    }
}

void ReversiServer::rivalIn(s_QQID hashid, MiraiCP::MessageEvent *evt, std::vector<MiraiCP::PlainText> &texts) {
    auto &status = reversiServer.status_memory[hashid];
    auto &game = reversiServer.server_memory[hashid];
    if (evt->from()->id() == game.getblackid()) return;
    if (std::any_of(texts.begin(), texts.end(), [](auto &&m) {
            return ReversiTools::isPrefixOf("reversi", m.content);
        })) {
        game.setwhitePlayer(evt->from()->id());
        evt->chat()->sendMessage("白棋加入了！");
        evt->chat()->sendMessage(evt->chat()->uploadImg(rawDraw(hashid)));
        status = PLAYING;
    }
}

void ReversiServer::playing(s_QQID hashid, MiraiCP::MessageEvent *evt) {
    ReversiInput _input;

    auto &mem = reversiServer.server_memory;
    auto &game = reversiServer.server_memory[hashid];
    {
        // 双方已经加入，开始下棋了
        auto &messages = *evt->getMessageChain();

        if (messages.size() != 1) return;
        auto &message = messages[0];
        if (message.type() != MiraiCP::PlainText::type()) return;

        const std::string &str = message.get<MiraiCP::PlainText>().content;
        if (str.size() < 2) return;

        bool playerblack = evt->from()->id() == game.getblackid();
        if (!playerblack && evt->from()->id() != game.getwhiteid()) return;

        _input.who = playerblack ? BLACK : WHITE;

        _input.movex = str[0] - '1';
        _input.movey = str[(str[1] == ' ') ? 2 : 1] - '1';

        if (!in_board(_input.movex, _input.movey)) return;
    }

    advanceGame(evt->chat(), hashid, _input, false);
}
