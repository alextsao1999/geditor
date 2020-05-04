//
// Created by Alex on 2020/5/4.
//

#include <tree_sitter.h>
#include <cstring>
#include <iostream>
extern "C" TSLanguage *tree_sitter_cpp();
int main() {
    using namespace ts;
    Parser parser(tree_sitter_cpp());
    std::string str = "int main(int abc) {auto *str = \"asdf\";return 0;} int test {return 111;}\n";
    Tree tree = parser.parse(str);
    //TSInputEdit edit;
    //tree.edit(edit);
/*
    Node root = tree.root();
    for (auto &iter : root) {
        printf("%d %d %d %s\n", iter.length(), iter.start_point().row, iter.end_point().row, iter.string());
    }
    tree.root();
*/
    auto cursor = tree.root().walk();
    while (cursor.goto_first_by_byte(34)) {
    }

    std::cout << cursor.node().string() << ":" << str.substr(cursor.node().start_byte(), cursor.node().length());


    return 0;
}
