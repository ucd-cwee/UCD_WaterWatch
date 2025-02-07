#pragma once

#include "Scopes.h"
#include "Foundation.h"
#include "Any.h"
#include "AST_Node.h"
#include <string>
#include <memory>
#include <string_view>
#include <vector>
#include <sstream>

namespace GoodLang {
    namespace Engine {
        /// Special type for returned values
        struct Return_Value {
            Any retval;
        };

        /// Special type indicating a call to 'break'
        struct Break_Loop {
        };

        /// Special type indicating a call to 'continue'
        struct Continue_Loop {
        };

        struct File_Position {
            int line = 0;
            int column = 0;

            constexpr File_Position(int t_file_line, int t_file_column) noexcept
                : line(t_file_line)
                , column(t_file_column) {
            }

            constexpr File_Position() noexcept = default;
        };
        struct Parse_Location {
            Parse_Location(File_Position const& start_pos = File_Position(), File_Position const& end_pos = File_Position())
                : start(start_pos)
                , end(end_pos) {}

            File_Position start;
            File_Position end;
        };

        /// Creates a new scope then pops it on destruction
        //struct Scope_Push_Pop {
        //    Scope_Push_Pop(Scope_Push_Pop&&) = default;
        //    Scope_Push_Pop& operator=(Scope_Push_Pop&&) = delete;
        //    Scope_Push_Pop(const Scope_Push_Pop&) = delete;
        //    Scope_Push_Pop& operator=(const Scope_Push_Pop&) = delete;

        //    explicit Scope_Push_Pop(std::shared_ptr<Global> t_ds)
        //        : m_ds(t_ds) {






        //        m_ds->new_scope(m_ds.stack_holder());
        //    }

        //    ~Scope_Push_Pop() { m_ds->pop_scope(m_ds.stack_holder()); }

        //private:
        //    std::shared_ptr<Global> m_ds;
        //};

    };








    //class AST_Node;
    //using AST_NodePtr = std::unique_ptr<AST_Node>;

    ///// \brief Struct that doubles as both a parser ast_node and an AST node.
    //struct AST_Node {
    //public:        
    //    AST_Node(AST_Node&&) = default;
    //    AST_Node& operator=(AST_Node&&) = delete;
    //    AST_Node(const AST_Node&) = delete;
    //    AST_Node& operator=(const AST_Node&) = delete;
    //    virtual ~AST_Node() noexcept = default;

    //    const AST_Node_Type identifier; // this node type
    //    const std::string_view text; // this node's text
    //    Parse_Location location; // this node's parse location

    //    const File_Position& start() const noexcept { return location.start; }
    //    const File_Position& end() const noexcept { return location.end; }

    //    std::string pretty_print() const {
    //        std::ostringstream oss;

    //        oss << text;

    //        for (auto& elem : get_children()) {
    //            oss << elem.get().pretty_print() << ' ';
    //        }

    //        return oss.str();
    //    }

    //    virtual std::vector<std::reference_wrapper<AST_Node>> get_children() const = 0;
    //    virtual Any eval() const = 0;

    //    /// Prints the contents of an AST node, including its children, recursively
    //    std::string to_string(const std::string& t_prepend = "") const {
    //        std::ostringstream oss;

    //        oss << t_prepend << "(" << identifier.ToString() << ") " << this->text << " : " << this->location.start.line << ", " << this->location.start.column << '\n';

    //        for (auto& elem : get_children()) {
    //            oss << elem.get().to_string(t_prepend + "  ");
    //        }

    //        return oss.str();
    //    };

    //protected:
    //    AST_Node(std::string_view t_ast_node_text, AST_Node_Type t_id, Parse_Location t_loc)
    //        : identifier(t_id)
    //        , text(std::move(t_ast_node_text))
    //        , location(std::move(t_loc))
    //    {}
    //};






};