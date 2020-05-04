//
// Created by Alex on 2020/5/3.
//

#ifndef GEDITOR_JSONWALKER_H
#define GEDITOR_JSONWALKER_H
#include <uri.h>
#include <string>
template<class ValueType>
class JsonWalker {
    //using ValueType = int;
    using Handler = std::function<ValueType(json &value)>;
    std::map<std::string, Handler> m_handler;
public:
    inline Handler &operator[](const std::string &index) { return m_handler[index]; }
    inline ValueType operator()(json &value) { return walk(value); }
    void add_handler(const char *type, Handler handler) {
        m_handler[type] = handler;
    }
    ValueType walk(json &value) {
        std::string type = value["type"];
        if (auto func = m_handler.find(type)->second) {
            return func(value["value"]);
        }
        return ValueType();
    }
};

#endif //GEDITOR_JSONWALKER_H
