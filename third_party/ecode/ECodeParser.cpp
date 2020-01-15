//
// Created by 曹顺 on 2019/5/3.
//

#include "ECodeParser.h"

ostream &operator<<(ostream &os, Key &key) {
    os << "{" << key.index << "-" << (int) key.type << "-" << (int) key.code << "}";
    return os;
}

ostream &operator<<(ostream &os, FixedData &data) {
    string str(data.data, static_cast<unsigned int>(data.length));
    os << str;
    return os;
}


bool ECodeParser::Check(const char *check, size_t length) {
    if (Cmp(check, length)) {
        _buffer.pos += length;
        return true;
    } else {
        return false;
    }
}

bool ECodeParser::Cmp(const char *check, size_t length) {
    if (_buffer.pos + length > _buffer.length) {
        return false;
    }
    return memcmp(&_buffer.code[_buffer.pos], check, length) == 0;
}

void ECodeParser::SetElibPath(char *path) {
    int length = strlen(path);
    delete[]_eLibPath;
    _eLibPath = new char[length + 1];
    strcpy(_eLibPath, path);
}

void ECodeParser::Parse() {
    assert(Check("CNWTEPRG", 8), "not a e code file!");
    while (_buffer.Good()) {
        // 查找段
        if (Check(seg_start, 4)) {
            _check++; // 用来校检
            int arg = _buffer.ReadInt();
            int flag = _buffer.ReadInt();
            int type = (flag >> 24) & 0xff;
            while (!Cmp(sec_start, 2))
                _buffer.pos++;
            SkipSegment();
            switch (type) {
                case 1:
                    ParseInfoSegement(arg);
                    break;
                case 2:
                    _buffer.Skip(60);
                    continue;
                case 3:
                    ParseCodeSegement();
                    break;
                case 4:
                    ParseResourceSegement();
                    break;
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    ParseAST();
                    return;
                default:
                    break;
            }

        } else {
            _buffer.pos++;
        }
    }
}

bool ECodeParser::CheckSegment(int num) {
    _buffer.Skip(4 * num + 1);
    int temp_check = _buffer.ReadInt();
    if (temp_check != _check) {
        std::cout << "open e file error!" << std::endl;
        return false;
    }
    _buffer.Skip(52);
    return true;
}

void ECodeParser::SkipSegment() {
    while (Cmp(sec_start, 2))
        _buffer.Skip(4);
    _buffer.Skip(1);
    int check = _buffer.ReadInt();
    if (check != _check) {
        std::cout << "文件可能有错误!" << std::endl;
    }
    _buffer.Skip(52);
}

Key ECodeParser::ParseKey() {
    Key key{};
    key.index = _buffer.ReadShort();
    key.code = _buffer.ReadByte();
    key.type = (KeyType) _buffer.ReadByte();
    return key;
}

void ECodeParser::ParseCodeSegement() {
    ParseLibrary();
    ParseModule();
    ParseSub();
    ParseVariable(code.globals);
    ParseDataStruct();
    ParseDll();
}

void ECodeParser::ParseLibrary() {
    Key key = ParseKey();
    _buffer.Skip(4);
    code.libraries.resize(_buffer.ReadInt() >> 2);
    _buffer.Skip(10 + code.libraries.size() * 8);
    int length = strlen(_eLibPath);
    char buf[512] = {'\0'};
    strcpy(buf, _eLibPath);
    for (auto & librarie : code.libraries) {
        librarie.path = _buffer.ReadFixedData();
        char *data = librarie.path.data;
        int j = 0;
        do {
            buf[length + j++] = *data;
        } while (*++data != '\r');
        buf[length + j] = '\0';
        strcat(buf, ".fne");
        librarie.hModule = LoadLibraryA(buf);
        auto fn = (PFN_GET_LIB_INFO) GetProcAddress(librarie.hModule, "GetNewInf");
        if (fn) {
            librarie.info = fn();
        }
    }

}

void ECodeParser::ParseModule() {
    _buffer.Skip(8);
    _buffer.Skip(_buffer.ReadInt());
    _buffer.Skip(_buffer.ReadInt());
    code.modules.resize(_buffer.ReadInt() >> 3);
    for (auto & module : code.modules) {
        module.key = ParseKey();
    }
    _buffer.Skip(code.modules.size() * 4);
    for (auto & module : code.modules) {
        module.property = _buffer.ReadInt();
        module.base = ParseKey();
        module.name = _buffer.ReadFixedData();
        module.comment = _buffer.ReadFixedData();
        module.include.resize(_buffer.ReadInt() >> 2);
        for (auto & include : module.include) {
            include = ParseKey();
        }
        ParseVariable(module.vars);
    }
}

void ECodeParser::ParseInfoSegement(int arg) {
    code.info.type = arg & 0xff;
    code.info.name = _buffer.ReadFixedData();
    code.info.description = _buffer.ReadFixedData();
    code.info.author = _buffer.ReadFixedData();
    code.info.code = _buffer.ReadFixedData();
    code.info.address = _buffer.ReadFixedData();
    code.info.telephone = _buffer.ReadFixedData();
    code.info.fox = _buffer.ReadFixedData();
    code.info.email = _buffer.ReadFixedData();
    code.info.host = _buffer.ReadFixedData();
    code.info.copyright = _buffer.ReadFixedData();
    code.info.version[0] = _buffer.ReadInt();
    code.info.version[1] = _buffer.ReadInt();
    code.info.create[0] = _buffer.ReadInt();
    code.info.create[1] = _buffer.ReadInt();
}

void ECodeParser::ParseWindow() {
    code.windows.resize(_buffer.ReadInt() >> 3);
    for (auto & window : code.windows) {
        window.key = ParseKey();
    }
    _buffer.Skip(code.windows.size() << 2);
    for (auto & window : code.windows) {
        _buffer.Skip(4);
        window.belong = ParseKey();
        window.name = _buffer.ReadFixedData();
        window.comment = _buffer.ReadFixedData();
        window.number = _buffer.ReadInt();

        // 下一个窗口的偏移
        int offset = _buffer.ReadInt();
        assert(offset > 0, "read windows error");
        offset += _buffer.pos;

        _buffer.Skip(34 + (window.number << 3));

        window.left = _buffer.ReadInt();
        window.top = _buffer.ReadInt();
        window.height = _buffer.ReadInt();
        window.width = _buffer.ReadInt();

        _buffer.Skip(12);
        window.cursor = _buffer.ReadFixedData();
        window.mark = _buffer.ReadFixedData();

        _buffer.Skip(4);
        int n = _buffer.ReadInt();
        window.visible = n & 1;
        window.bidden = n & 2;

        _buffer.Skip(4);

        n = _buffer.ReadInt();
        _buffer.Skip(n << 3);
        if (_buffer.pos > offset - 24) {
            _buffer.pos = static_cast<size_t>(offset);
            continue;
        }

        _buffer.Skip(12);
        int property = _buffer.ReadInt();
        if (property > 0 && property <= 6) {
            window.border = _buffer.ReadInt();
            window.bgSize = _buffer.ReadInt();
            window.bgColor = _buffer.ReadInt();
            window.maxBtn = _buffer.ReadInt();
            window.minBtn = _buffer.ReadInt();
            window.ctrlBtn = _buffer.ReadInt();
            window.position = _buffer.ReadInt();
            window.movable = _buffer.ReadInt();
            window.musicTimes = _buffer.ReadInt();
            window.enterFocus = _buffer.ReadInt();
            window.escClose = _buffer.ReadInt();
            window.f1Help = _buffer.ReadInt();
            window.helpMark = _buffer.ReadInt();
            window.helpMark = _buffer.ReadInt();
            window.helpMark = _buffer.ReadInt();
            window.helpMark = _buffer.ReadInt();
            window.helpMark = _buffer.ReadInt();
            window.showInTaskbar = _buffer.ReadInt();
            window.mov = _buffer.ReadInt();
            window.shape= _buffer.ReadInt();
            window.alwaysTop= _buffer.ReadInt();
            window.alwaysActive= _buffer.ReadInt();
        }
        if (property == 6) {
            window.className = _buffer.ReadFixedData();
        }
        if (property >= 2) {
            window.title = _buffer.ReadFixedData();
        }
        window.helpFileName = _buffer.ReadFixedData();
        _buffer.pos = static_cast<size_t>(offset);
    }

}

void ECodeParser::ParseResourceSegement() {
    ParseWindow();
    ParseConstant();
}

void ECodeParser::ParseConstant() {
    code.constants.resize(_buffer.ReadInt());
    _buffer.Skip(4);
    for (auto & constant : code.constants) {
        constant.key = ParseKey();
    }
    _buffer.Skip(code.constants.size() * 4);
    for (auto & constant : code.constants) {
        _buffer.Skip(4);
        constant.property = _buffer.ReadShort();
        constant.name = _buffer.ReadString();
        constant.comment = _buffer.ReadString();
        if (constant.key.type == KeyType_ImageRes || constant.key.type == KeyType_SoundRes) {
            constant.data = _buffer.ReadFixedData();
        } else {
            uint8_t  type = _buffer.ReadByte();
            constant.value = ParseValue(_buffer, type);
        }
    }

}

void ECodeParser::ParseVariable(std::vector<EVar> &vars) {
    vars.resize(_buffer.ReadInt());
    size_t offset = _buffer.ReadInt() + _buffer.pos;
    for (auto & var : vars) {
        var.key = ParseKey();
    }
    _buffer.Skip(vars.size() * 4);
    for (auto & var : vars) {
        size_t next_offset = _buffer.ReadInt() + _buffer.pos;
        var.type = _buffer.ReadInt();
        var.property = _buffer.ReadShort();
        var.dimension.resize(_buffer.ReadByte());
        for (int & d : var.dimension) {
            d = _buffer.ReadInt();
        }
        var.name = _buffer.ReadString();
        var.comment = _buffer.ReadString();
        _buffer.pos = next_offset;
    }
    _buffer.pos = offset;
}

void ECodeParser::ParseSub() {
    code.subs.resize(_buffer.ReadInt() >> 3);
    for (auto & sub : code.subs) {
        sub.key = ParseKey();
        for (auto & module : code.modules) {
            if (module.has(sub.key)) {
                sub.module = &module;
                break;
            }
        }
    }
    _buffer.Skip(code.subs.size() * 4);
    for (auto & sub : code.subs) {
        _buffer.Skip(4);
        sub.property = _buffer.ReadInt();
        sub.type = _buffer.ReadInt();
        sub.name = _buffer.ReadFixedData();
        sub.comment = _buffer.ReadFixedData();
        ParseVariable(sub.locals);
        ParseVariable(sub.params);
        for (auto &i : sub.code) {
            i = _buffer.ReadFixedData();
        }
    }
}

void ECodeParser::ParseDataStruct() {
    code.structs.resize(_buffer.ReadInt() >> 3);
    for (auto & i : code.structs) {
        i.key = ParseKey();
    }
    _buffer.Skip(code.structs.size() * 4);
    for (auto & j : code.structs) {
        j.property = _buffer.ReadInt();
        j.name = _buffer.ReadFixedData();
        j.comment = _buffer.ReadFixedData();
        ParseVariable(j.members);
    }

}

void ECodeParser::ParseDll() {
    code.dlls.resize(_buffer.ReadInt() >> 3);
    for (auto &dll : code.dlls) {
        dll.key = ParseKey();
    }
    _buffer.Skip(code.dlls.size() * 4);
    for (auto & dll : code.dlls) {
        dll.property = _buffer.ReadInt();
        dll.type = _buffer.ReadInt();
        dll.name = _buffer.ReadFixedData();
        dll.comment = _buffer.ReadFixedData();
        dll.lib = _buffer.ReadFixedData();
        dll.func = _buffer.ReadFixedData();
        ParseVariable(dll.params);
    }
}

void ECodeParser::ParseAST() {
    for (auto & sub : code.subs) {
        FileBuffer code_buf(sub.code[5].data, (size_t) sub.code[5].length);
        sub.ast = ParseSubCode(code_buf);
    }
}

Value ECodeParser::ParseValue(FileBuffer &buf, uint8_t type) {
    switch (type) {
        case 22:
            // 空值
            return Value();
        case 23:
        {
            // double
            FixedData data = buf.Read(8);
            return Value(*((double *) data.data));
        }
        case 59:
            return Value(buf.ReadInt());
            // int
        case 24:
            // 逻辑
            return Value(buf.ReadShort() != 0);
        case 25:
        {
            // 日期时间
            FixedData data = buf.Read(8);
            return Value(*((long long *) data.data));
        }
        case 26:
            // 文本
            return Value(buf.ReadFixedData());
        default:
            return Value();
    }
}

ASTProgramPtr ECodeParser::ParseSubCode(FileBuffer &buf) {
    ASTProgramPtr ast = make_ptr(ASTProgram);
    while (buf.Good()) {
        ast->AddStmt(ParseLineNode(buf, buf.ReadByte()));
    }
    return ast;
}

ASTNodePtr ECodeParser::ParseLineNode(FileBuffer &buf, uint8_t type) {
    switch (type) {
        case 1:
            return ParseLineNode(buf, buf.ReadByte());
        case 106:
            // 行
            return ParseFunCall(buf);
        case 107:
            // 如果
            return ParseIf(buf);
        case 108:
            // 如果真
            return ParseIfTrue(buf);
        case 109:
            // 判断
            return ParseJudge(buf);
        case 112:
            // 循环
            return ParseLoop(buf);
        default:
            break;
    }
    return nullptr;
}

ASTIfStmtPtr ECodeParser::ParseIf(FileBuffer &buf) {
    ASTIfStmtPtr ifstmt = make_ptr(ASTIfStmt);
    ASTFunCallPtr ptr = ParseFunCall(buf);
    if (!ptr->args->args.empty()) {
        ifstmt->condition = ptr->args->args[0];
    }
    ifstmt->then_block = make_ptr(ASTBlock);
    uint8_t next;
    do {
        next = buf.ReadByte();
        if (next == 80 || next == 81)
            break;
        ifstmt->then_block->AddStmt(ParseLineNode(buf, next));
    } while (buf.Good());
    if (next == 80) {
        ifstmt->else_block = make_ptr(ASTBlock);
        do {
            next = buf.ReadByte();
            if (next == 81)
                break;
            ifstmt->else_block->AddStmt(ParseLineNode(buf, next));
        } while (buf.Good());

    }
    buf.Match(114);
    return ifstmt;
}

ASTIfStmtPtr ECodeParser::ParseIfTrue(FileBuffer &buf) {
    ASTIfStmtPtr ifstmt = make_ptr(ASTIfStmt);
    ASTFunCallPtr ptr = ParseFunCall(buf);
    if (!ptr->args->args.empty()) {
        ifstmt->condition = ptr->args->args[0];
    }
    ifstmt->then_block = make_ptr(ASTBlock);
    uint8_t next;
    do {
        next = buf.ReadByte();
        if (next == 82)
            break;
        ifstmt->then_block->AddStmt(ParseLineNode(buf, next));
    } while (buf.Good());
    buf.Match(115);
    return ifstmt;
}

ASTJudgePtr ECodeParser::ParseJudge(FileBuffer &buf) {
    ASTJudgePtr ast = make_ptr(ASTJudge);
    uint8_t next;
    ASTBlockPtr block;
    do {
        next = buf.ReadByte();
        if (next == 110) {
            ASTFunCallPtr ptr = ParseFunCall(buf);
            if (!ptr->args->args.empty()) {
                ast->conditions.push_back(ptr->args->args[0]);
            }
            block = make_ptr(ASTBlock);
        } else if (next == 111) {
            ast->blocks.push_back(block);
            // 进入默认分支
            ast->default_block = make_ptr(ASTBlock);
            block = ast->default_block;
        } else if (next == 83) {
            // 分支结束
            ast->blocks.push_back(block);
        } else if (next == 84) {
            // 判断结束
            break;
        } else {
            if (block == nullptr)
                return ast;
            block->AddStmt(ParseLineNode(buf, next));
        }
    } while (buf.Good());

    buf.Match(116);
    return ast;
}

ASTLoopPtr ECodeParser::ParseLoop(FileBuffer &buf) {
    ASTLoopPtr ast = make_ptr(ASTLoop);
    ast->head = ParseFunCall(buf);
    ast->block = make_ptr(ASTBlock);
    uint8_t next;
    do {
        next = buf.ReadByte();
        if (next == 85)
            break;
        ast->block->AddStmt(ParseLineNode(buf, next));
    } while (buf.Good());
    if (buf.Match(113)) {
        ast->tail = ParseFunCall(buf);
    }
    return ast;
}

ASTFunCallPtr ECodeParser::ParseFunCall(FileBuffer &buf) {
    ASTFunCallPtr ptr = make_ptr(ASTFunCall);
    ptr->key.value = buf.ReadInt();
    ptr->lib = buf.ReadShort();
    ptr->unknown = buf.ReadShort();
    ptr->object = buf.ReadFixedData();
    ptr->comment = buf.ReadFixedData();
    ptr->args = ParseArgs(buf);
    return ptr;
}

ASTNodePtr ECodeParser::ParseNode(FileBuffer &buf, uint8_t type) {
    switch (type) {
        case 27:
            // 自定义常量
            return make_ptr(ASTConstant, buf.ReadInt());
        case 28:
            // 支持库常量
            return make_ptr(ASTLibConstant, buf.ReadInt());
        case 29:
            // 变量
            return ParseNode(buf, buf.ReadByte());
        case 30:
            // 子程序指针
            return make_ptr(ASTAddress, buf.ReadInt());
        case 31:
            // 左大括号

            return make_ptr(ASTBrace, ParseArgs(buf));

            NOT_REACHED();
            break;
        case 32:
            // 右大括号
            NOT_REACHED();
            break;
        case 33:
            // 函数
            return ParseFunCall(buf);
        case 35:
            // 枚举常量
            return make_ptr(ASTEnumConstant, buf.ReadInt(), buf.ReadInt());
        case 54:
            // 左小括号
            return ParseNode(buf, buf.ReadByte());
        case 56:
            // 对象开始
        {
            int mark = buf.ReadInt();
            if (mark == 0x500FFFE)
                buf.ReadByte();

            ASTNodePtr ast = make_ptr(ASTVariable, mark);
            uint8_t next;
            while ((next = buf.ReadByte()) != 55) {
                ast = make_ptr(ASTDot, ast, ParseNode(buf, next));
            }
            return ast;
        }
        case 57:
            // 数据成员
            return make_ptr(ASTStructMember, buf.ReadInt(), buf.ReadInt());
        case 58:
            // 数组下标
            return make_ptr(ASTSubscript, ParseNode(buf, buf.ReadByte()));
        default:
            if ((type >= 22 && type <= 26) || type == 59) {
                return make_ptr(ASTLiteral, ParseValue(buf, type));
            }
            break;
    }
    NOT_REACHED();
    return nullptr;
}

ASTArgsPtr ECodeParser::ParseArgs(FileBuffer &buf) {
    ASTArgsPtr ast = make_ptr(ASTArgs);
    buf.Match(54);

    uint8_t type;
    do {
        type = buf.ReadByte();
        if (type == 1 || type == 0 || type == 32) {
            break;
        }
        ast->AddArg(ParseNode(buf, type));
    } while (buf.Good());
    return ast;
}


