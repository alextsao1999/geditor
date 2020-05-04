//
// Created by Alex on 2020/5/4.
//

#ifndef GEDITOR_TREE_SITTER_H
#define GEDITOR_TREE_SITTER_H
#include <tree_sitter/api.h>
#include <string>
namespace ts {
    class Logger {
    public:
        virtual void report(TSLogType type, const char *) = 0;
        static void Callback(void *payload, TSLogType type, const char *str) {
            auto *logger = (Logger *) payload;
            logger->report(type, str);
        }
    };
    class Tree;
    class Parser {
        TSParser *m_parser = nullptr;
    public:
        Parser() = default;
        Parser(TSLanguage *language) : m_parser(ts_parser_new()) {
            ts_parser_set_language(m_parser, language);
        }
        Parser(const Parser &rhs) : m_parser(ts_parser_new()) {
            ts_parser_set_language(m_parser, ts_parser_language(rhs.m_parser));
        }
        Parser(Parser &&rhs) : m_parser(rhs.m_parser) {
            rhs.m_parser = nullptr;
        }
        ~Parser() {
            ts_parser_delete(m_parser);
        }
        Tree parse(const std::string &str);
        void parse(const std::string &str, Tree &tree);
        void set_logger(Logger *logger) {
            ts_parser_set_logger(m_parser, {logger, Logger::Callback});
        }
        Logger *get_logger() {
            return (Logger *) ts_parser_logger(m_parser).payload;
        }
    };
    class Node {
        TSNode m_node;
        friend class Cursor;
    public:
        Node(const TSNode &mNode) : m_node(mNode) {}
        bool empty() { return ts_node_is_null(m_node); }
        TSSymbol symbol() { return ts_node_symbol(m_node); }
        bool has_changes() { return ts_node_has_changes(m_node); }
        bool is_extra() { return ts_node_is_extra(m_node); }
        bool is_missing() { return ts_node_is_missing(m_node); }
        bool is_named() { return ts_node_is_named(m_node); }
        uint32_t count() { return ts_node_child_count(m_node); }
        char *string() { return ts_node_string(m_node); }
        const char *type() { return ts_node_type(m_node); }
        uint32_t start_byte() { return ts_node_start_byte(m_node); }
        uint32_t end_byte() { return ts_node_end_byte(m_node); }
        uint32_t length() { return end_byte() - start_byte(); }
        TSPoint start_point() { return ts_node_start_point(m_node); }
        TSPoint end_point() { return ts_node_end_point(m_node); }
        Node parent() { return ts_node_parent(m_node); }
        Node prev() { return ts_node_prev_sibling(m_node); }
        Node next() { return ts_node_next_sibling(m_node); }
        Node prev_named() { return ts_node_prev_named_sibling(m_node); }
        Node next_named() { return ts_node_next_named_sibling(m_node); }
        bool operator==(const Node &rhs) { return ts_node_eq(m_node, rhs.m_node); }
        bool operator!=(const Node &rhs) { return !ts_node_eq(m_node, rhs.m_node); }
        operator TSNode& () { return m_node; }
        Node by_field_id (TSFieldId id) { return ts_node_child_by_field_id(m_node, id); }
        Node by_index (const uint32_t &index) { return ts_node_child(m_node, index); }
        Node operator[](const uint32_t &index) { return ts_node_child(m_node, index); }
        Node operator[](const std::string &index) {
            return ts_node_child_by_field_name(m_node, index.c_str(), index.length());
        }
        Node &operator++() { m_node = ts_node_next_sibling(m_node); return *this; }
        Node operator++(int) { return next(); }
        inline Node begin() { return ts_node_child(m_node, 0); }
        inline Node end() { return ts_node_child(m_node, count() - 1); }
        inline Node &operator*() { return *this; }
        Cursor walk();
    };
    class Tree {
        friend class Parser;
        TSTree *m_tree = nullptr;
    public:
        Tree() = default;
        Tree(TSTree *tree) : m_tree(tree) {}
        Tree(const Tree &rhs) {
            m_tree = ts_tree_copy(rhs.m_tree);
        }
        Tree(Tree &&rhs) {
            m_tree = rhs.m_tree;
            rhs.m_tree = nullptr;
        }
        ~Tree() {
            ts_tree_delete(m_tree);
        }
        Node root() { return ts_tree_root_node(m_tree); }
        void print_dot_graph(FILE *io) {
            ts_tree_print_dot_graph(m_tree, io);
        }
        void edit(const TSInputEdit &input) {
            ts_tree_edit(m_tree, &input);
        }
    };
    class Cursor {
        TSTreeCursor m_cursor;
    public:
        Cursor(const Node &node) : m_cursor(ts_tree_cursor_new(node.m_node)) {}
        Cursor(const Cursor &rhs) : m_cursor(ts_tree_cursor_copy(&rhs.m_cursor)) {}
        ~Cursor() {
            ts_tree_cursor_delete(&m_cursor);
        }
        Cursor &operator=(const Cursor &rhs) = delete;
        inline void reset(const Node &node) {
            ts_tree_cursor_reset(&m_cursor, node.m_node);
        }
        inline Node node() { return Node(ts_tree_cursor_current_node(&m_cursor)); }
        inline TSFieldId field_id() {
            return ts_tree_cursor_current_field_id(&m_cursor);
        }
        inline const char *field_name() {
            return ts_tree_cursor_current_field_name(&m_cursor);
        }
        inline bool goto_parent() {
            return ts_tree_cursor_goto_parent(&m_cursor);
        }
        inline bool goto_next() {
            return ts_tree_cursor_goto_next_sibling(&m_cursor);
        }
        inline bool goto_first() {
            return ts_tree_cursor_goto_first_child(&m_cursor);
        }
        inline bool goto_first_by_point(const TSPoint &pt) {
            if (node().start_point().row >= pt.row) {

            }
            return false;
        }
        inline bool goto_first_by_byte(uint32_t byte) {
            return ts_tree_cursor_goto_first_child_for_byte(&m_cursor, byte) != -1;
        }

    };
    Cursor Node::walk() {
        return Cursor(*this);
    }
    Tree Parser::parse(const std::string &str) {
        return Tree(ts_parser_parse_string(m_parser, nullptr, str.c_str(), str.length()));
    }
    void Parser::parse(const std::string &str, Tree &tree) {
        tree.m_tree = ts_parser_parse_string(m_parser, tree.m_tree, str.c_str(), str.length());
    }

}

#endif //GEDITOR_TREE_SITTER_H
