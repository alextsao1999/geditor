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
    Copy,
    Paste,
    Cut,
    Delete,
    CD,
    CF,
    CG
};
typedef uint32_t KeyState;
class KeyMap {
public:
    std::map<KeyState, KeyCommand> m_maps = {
            {'C' | KEY_CTRL, KeyCommand::Copy},
            {'V' | KEY_CTRL, KeyCommand::Paste},
            {'X' | KEY_CTRL, KeyCommand::Cut},
            {'Z' | KEY_CTRL, KeyCommand::Undo},
            {'D' | KEY_CTRL, KeyCommand::CD},
            {'F' | KEY_CTRL, KeyCommand::CF},
            {'G' | KEY_CTRL, KeyCommand::CG},
    };
    bool hasKey(KeyState key) {
        return m_maps.count(GetState(key));
    }
    KeyCommand lookUp(KeyState key) {
        return m_maps[GetState(key)];
    }
    static KeyState GetState(KeyState state = 0) {
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            state |= KEY_CTRL;
        }
        if (GetKeyState(VK_SHIFT) & 0x8000) {
            state |= KEY_SHIFT;
        }
        if (GetKeyState(VK_MENU) & 0x8000) {
            state |= KEY_ALT;
        }
        return state;
    }

};

#endif //GEDITOR_KEYMAP_H
