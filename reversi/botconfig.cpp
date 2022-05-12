//
// Created by antares on 4/28/22.
//

#include "botconfig.h"
#include <fstream>

const QQinfo QQCONFIG = []() -> QQinfo {
    std::ifstream i;
    i.open("config/config.json", std::ios::in);
    nlohmann::json j;
    i >> j;
    i.close();
    std::unordered_map<std::string, std::string> p = j["reversi"];
    return {j["MASTERID"].get<MiraiCP::QQID>(), j["BOTID"].get<MiraiCP::QQID>(), std::move(p)};
}();
