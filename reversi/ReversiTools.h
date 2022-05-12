//
// Created by antares on 5/10/22.
//

#ifndef ANTARESRBQQ_REVERSITOOLS_H
#define ANTARESRBQQ_REVERSITOOLS_H

#include <algorithm>
#include <chrono>
#include <string>

namespace ReversiTools {
    inline bool isPrefixOf(const std::string &pre, const std::string &str) {
        if (pre.size() > str.size()) return false;
        auto it2 = str.begin();
        return std::all_of(pre.begin(), pre.end(), [&](auto &&c) { return c == *(it2++); });
    }

    inline auto getNowTimeStamp() {
        return std::chrono::system_clock::now();
    }

    template<class T>
    inline auto timeDiff(const T &t1, const T &t2) {
        return std::chrono::duration_cast<std::chrono::seconds>(t1 - t2).count();
    }
} // namespace ReversiTools
#endif //ANTARESRBQQ_REVERSITOOLS_H
