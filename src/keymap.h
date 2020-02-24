//
// Created by Alex on 2020/2/23.
//

#ifndef GEDITOR_KEYMAP_H
#define GEDITOR_KEYMAP_H

#include <map>
#include <windows.h>
#define KEY_CTRL (1 << 24)
#define KEY_SHIFT (1 << 25)
#define KEY_ALT (1 << 26)

enum class KeyCommand {
    None,
    Undo,
};

class KeyMap {
public:
    std::map<unsigned, KeyCommand> m_maps = {{0x1A | KEY_CTRL, KeyCommand::Undo}};
    KeyCommand lookUp(unsigned key) {
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            key |= KEY_CTRL;
        }
        return m_maps[key];
    }

};

#endif //GEDITOR_KEYMAP_H
