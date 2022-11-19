//
// Created by antares on 10/20/21.
//
#ifndef MIRAICP_MAIN_CPP
#define MIRAICP_MAIN_CPP

#include "CPPPlugin.h"
#include "Event.h"
#include "PluginConfig.h"
#include "reversi/ReversiServer.h"
#include "reversi/botconfig.h"
#include "utils.h"
#include <functional>

using namespace MiraiCP;
const PluginConfig CPPPlugin::config{
        "0982",
        "AntaresTesting",
        "v1.0.0", "Antares",
        "Test mirai plugin",
        "2021/10/19"};


class Main : public CPPPlugin {
public:
    // 配置插件信息
    Main() : CPPPlugin() {
    }

    void onEnable() override {
        // 监听
        Event::registerEvent<GroupMessageEvent>(ReversiServer::groupHandle);
        Event::registerEvent<PrivateMessageEvent>(ReversiServer::privateHandle);

        Event::registerEvent<BotOnlineEvent>([](const BotOnlineEvent &e) {
            auto f = Friend(QQCONFIG.MASTERID, QQCONFIG.BOTID);
            f.sendMessage("Bot is live!");
        });

        Event::registerEvent<MiraiCPExceptionEvent>([](const MiraiCPExceptionEvent &e) {
            try {
                auto f = Friend(QQCONFIG.MASTERID, QQCONFIG.BOTID);
                auto e_ptr = e.getException();
                f.sendMessage("黑白棋bot出现了未经处理的异常，类型：" + e_ptr->getExceptionType() + "，内容：" + e_ptr->getError() + "\n文件：" + e_ptr->filename + "，行号：" + std::to_string(e_ptr->lineNum));
            } catch (...) { // catch all exception, else infinite loop of MiraiCPExceptionEvent will occur
            }
        });
    }

    void onDisable() override {
        /*插件结束*/
        ReversiServer::cleanUp();
    }
};

// 绑定当前插件实例
void MiraiCP::enrollPlugin() {
    enrollPlugin<Main>();
}

#endif
