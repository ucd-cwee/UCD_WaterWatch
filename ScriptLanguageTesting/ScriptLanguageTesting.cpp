#pragma region "Includes"
#pragma once

#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <string_view>
#include <regex>
#include <list>
#include <thread>
#include <concurrent_unordered_map.h>
#include <stdlib.h>

#include "util.h"
#include "atomic_allocator.h"
#include "atomic_vector.h"
#include "atomic_stack.h"
#include "atomic_queue.h"
#include "atomic_numbers.h"
#include "atomic_maps.h"
#include "stopwatch.h"
#include "strings.h"
#include "atomic_shared_ptr.h"
#include "types.h"
#include "Parallel.h"
#include "shared_ptr.h"
#include "units.h"
#include "datetime.h"
#include "functions.h"
#include "scripting.h"
#include "atomic_tree.h"

#pragma endregion

#pragma region "Definitions"
#define SINGLE_ARG(...) __VA_ARGS__
#define EXPECT_EQ_PRINTF(A,B) [a = (A), b = (B)]()->bool{ \
    if (a == b) { return true; } else { \
        std::string tempA{std::to_string(a)}, tempB{std::to_string(b)}; \
        std::cout << GoodLang::printf("FAILURE AT LINE %i: (%s != %s)\n", (int)__LINE__, tempA.c_str(), tempB.c_str()); \
    return false; } \
}()

#define print(a) std::cout << a << std::endl
struct catcher {
public:
    static bool& allow_print(){ 
        static bool out{ true };
        return out;
    };
    static __declspec(noinline) void CatchMe(long L) {
        if (allow_print()) {
            std::cout << GL::printf("FAILURE AT LINE %i\n", (int)L);
        }
    }
};
#define EXPECT_EQ(a, b) if ((a) != (b)){ catcher::CatchMe(__LINE__); }
#define EXPECT_NE(a, b) if ((a) == (b)){ catcher::CatchMe(__LINE__); }
#define ASSERT(a) if ((a) != true){ catcher::CatchMe(__LINE__); }
#pragma endregion

#include "../GpuProgramming/matrix.h" // Working implimentation of GPU-accelrated matrix
// #include "../ExcelInterop/Wrapper.h"

__forceinline void console_clear() {
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
    );
    SetConsoleCursorPosition(console, topLeft);
}

#include "../WaterWatchCpp/enum.h"

namespace GL {

	namespace utility {
		struct Static_String {
			template<size_t N>
			constexpr Static_String(const char(&str)[N]) noexcept
				: m_size(N - 1)
				, data(&str[0]) {
			}

			constexpr size_t size() const noexcept { return m_size; }

			constexpr const char* c_str() const noexcept { return data; }

			constexpr auto begin() const noexcept { return data; }

			constexpr auto end() const noexcept { return data + m_size; }

			constexpr bool operator==(std::string_view other) const noexcept {
				// return std::string_view(data, m_size) == other;
				auto b1 = begin();
				const auto e1 = end();
				auto b2 = other.begin();
				const auto e2 = other.end();

				if (e1 - b1 != e2 - b2) {
					return false;
				}

				while (b1 != e1) {
					if (*b1 != *b2) {
						return false;
					}
					++b1;
					++b2;
				}
				return true;
			}

			bool operator==(const std::string& t_str) const noexcept { return std::equal(begin(), end(), std::cbegin(t_str), std::cend(t_str)); }

			operator const char* () const {
				return c_str();
			};

			const size_t m_size;
			const char* data = nullptr;
		};
		template<typename Itr> static constexpr std::uint32_t hash(Itr begin, Itr end) noexcept {
			std::uint32_t h = 0x811c9dc5;
			while (begin != end) {
				h = (h ^ (*begin)) * 0x01000193;
				++begin;
			}
			return h;
		};
		template<size_t N> static constexpr std::uint32_t hash(const char(&str)[N]) noexcept { return hash(std::begin(str), std::end(str) - 1); };
		static constexpr std::uint32_t hash(std::string_view sv) noexcept { return hash(sv.begin(), sv.end()); };
	};

	namespace Engine {
		BETTER_ENUM(AST_Node_Type, uint32_t,
			File, Noop, Comment,
			Id, Reference, Var_Decl, Assign_Decl, Constant,
			Fun_Call, Unused_Return_Fun_Call, Type_Cast,
			Arg_List, Arg,
			Equation,
			Array_Call, Dot_Access,
			Lambda,
			FunctionBlock, DeclarationBlock, Block, Scopeless_Block,
			Def, If,
			Parallel, Parallel_For, Parallel_Ranged_For, For, Ranged_For, While,
			Inline_Array, Inline_Map,
			Return,
			Prefix, Postfix,
			Break, Continue,
			Map_Pair, Value_Range, Inline_Range,
			Do, Try, Catch, Finally,
			Throw,
			Attr_Decl,
			Logical_And, Logical_Or,
			Switch, PreprocessedSwitch, Case, Default,
			Class, Namespace,
			FunctionDecl,
			BinaryFoldRight, BinaryFoldLeft, Binary,
			Global_Decl,
			Compiled,
			ControlBlock,
			Assign_Retroactively,
			TypeId,
			JustInTimeCompilation,
			PreprocessorMacro
		);
		BETTER_ENUM(PreprocessorDirectives, uint32_t,
			Completed, None,
			Include, Define, Undefine,
			If, ElseIf, Else, EndIf, IfDefined, IfNotDefined, IfChain,
			Error, Warning, Pragma
		);
		enum Alphabet {
			symbol_alphabet = 0,
			keyword_alphabet,
			int_alphabet,
			float_alphabet,
			x_alphabet,
			hex_alphabet,
			b_alphabet,
			bin_alphabet,
			id_alphabet,
			white_alphabet,
			int_suffix_alphabet,
			float_suffix_alphabet,
			max_alphabet,
			lengthof_alphabet = 256
		};
		enum class Operator_Precedence {
			Ternary_Cond,
			Logical_Or,
			Logical_And,
			Bitwise_Or,
			Bitwise_Xor,
			Bitwise_And,
			Equality,
			Comparison,
			Shift,
			Addition,
			Multiplication,
			Prefix
		};

		struct File_Position {
			int line = 0;
			int column = 0;
			int position = 0;

			constexpr File_Position(int t_file_line, int t_file_column, int pos) noexcept
				: line(t_file_line)
				, column(t_file_column)
				, position(pos)
			{}

			constexpr File_Position() noexcept = default;
		};
		struct Position {
			operator File_Position() const {
				return File_Position(line, col, pos);
			};
			Position() 
				: line(-1)
				, col(-1)
				, pos(-1)
				, m_pos(0)
				, m_str_sets(nullptr)
				, m_last_col(-1) {
			};
			Position(size_t t_pos, GL::string const& srce) noexcept
				: line(1)
				, col(1)
				, pos(0)
				, m_pos(t_pos)
				, m_last_col(1) 
			{
				m_str_sets = std::make_shared<std::vector< std::pair<GL::string, bool> >>();
				m_str_sets->push_back({ srce.to_string(), true });
				current_position = srce.substr(m_pos);
			};
			GL::string str() const noexcept {
				GL::string out;
				for (auto& x : *m_str_sets) {
					out = out + x.first;
				}
				return out;
			};
			size_t size() const noexcept {
				size_t out = 0;
				for (auto& x : *m_str_sets) {
					out += x.first.length();
				}
				return out;
			};
			const char& operator[](size_t pos) const {
				for (auto& x : *m_str_sets) {
					if (pos >= x.first.length()) {
						pos -= x.first.length();
					}
					else {
						return x.first[pos];
					}
				}
				static const char out = '\0';
				return out;
			};
			char& operator[](size_t pos) {
				for (auto& x : *m_str_sets) {
					if (pos >= x.first.length()) {
						pos -= x.first.length();
					}
					else {
						return const_cast<char&>(x.first[pos]);
					}
				}
				static char out = '\0';
				return out;
			};
			void insert_at(size_t pos, GL::string insert) {
				int index = 0;
				for (auto& x : *m_str_sets) {
					if (pos >= x.first.length()) {
						pos -= x.first.length();
						++index;
					}
					else {
						auto LHS = m_str_sets->operator[](index).first.left(pos);
						auto RHS = m_str_sets->operator[](index).first.right(m_str_sets->operator[](index).first.length() - LHS.length());

						m_str_sets->insert(m_str_sets->begin() + index + 1, { RHS, m_str_sets->operator[](index).second });
						(m_str_sets->begin() + index)->first = LHS;
						m_str_sets->insert(m_str_sets->begin() + index + 1, { insert, false });
						return;
					}
				}
			};


			static GL::string str(const Position& t_begin, const Position& t_end) noexcept {
				return t_begin.str().substr(t_begin.m_pos, t_end.m_pos - t_begin.m_pos);
			};
			Position& operator++() noexcept {
				if (m_pos < size()) {
					if (operator[](m_pos) == '\n') {
						++line;
						m_last_col = col;
						col = 1;
					}
					else {
						++col;
					}

					++pos;
					++m_pos;
					current_position = str().substr(m_pos);
				}
				return *this;
			};
			Position& operator--() noexcept {
				--pos;
				--m_pos;
				if (operator[](m_pos) == '\n') {
					--line;
					col = m_last_col;
				}
				else {
					--col;
				}
				current_position = str().substr(m_pos);
				return *this;
			};
			Position& operator+=(size_t t_distance) noexcept {
				*this = (*this) + t_distance;
				return *this;
			}
			Position operator+(size_t t_distance) const noexcept {
				Position ret(*this);
				for (size_t i = 0; i < t_distance; ++i) {
					++ret;
				}
				return ret;
			}
			Position& operator-=(size_t t_distance) noexcept {
				*this = (*this) - t_distance;
				return *this;
			}
			Position operator-(size_t t_distance) const noexcept {
				Position ret(*this);
				for (size_t i = 0; i < t_distance; ++i) {
					--ret;
				}
				return ret;
			}
			bool operator==(const Position& t_rhs) const noexcept { return m_pos == t_rhs.m_pos; }
			bool operator!=(const Position& t_rhs) const noexcept { return m_pos != t_rhs.m_pos; }
			bool has_more() const noexcept { return m_pos != size(); }
			size_t remaining() const noexcept { return static_cast<size_t>(size() - m_pos); }
			const char& operator*() const noexcept {
				if (m_pos == size()) {
					return ""[0];
				}
				else {
					return operator[](m_pos);
				}
			};

			int line = -1;
			int col = -1;
			int pos = -1;

			GL::string to_string() const {
				return GL::printf("L%iC%i(#%i)", line, col, pos);
			};
			std::vector< std::pair<GL::string, bool> >& Source() {
				return *m_str_sets;
			};

		private:
			size_t m_pos = 0;
			std::shared_ptr<std::vector< std::pair<GL::string, bool> >> m_str_sets;
			GL::string current_position;
			int m_last_col = -1;
		};
		struct Parse_Location {
			Parse_Location()
				: start()
				, end()
			{}
			Parse_Location(Position _start, Position _end)
				: start(_start)
				, end(_end)
			{}
			Position start;
			Position end;

			GL::string to_string() const {
				return start.to_string() + " - " + end.to_string();
			};
		};

		template<typename string_type>
		struct Char_Parser_Helper {
			// common for all implementations
			static std::string u8str_from_ll(long long val) {
				using char_type = std::string::value_type;

				char_type c[2];
				c[1] = char_type(val);
				c[0] = char_type(val >> 8);

				if (c[0] == 0) {
					return std::string(1, c[1]); // size, character
				}

				return std::string(c, 2); // char buffer, size
			}

			static string_type str_from_ll(long long val) {
				using target_char_type = typename string_type::value_type;
				return string_type(1, target_char_type(val)); // size, character
			}
		};
		template<> struct Char_Parser_Helper<std::string> {
			static std::string str_from_ll(long long val) {
				// little SFINAE trick to avoid base class
				return Char_Parser_Helper<std::true_type>::u8str_from_ll(val);
			}
		};
		template<typename Itr> static constexpr std::uint32_t hash(Itr begin, Itr end) noexcept {
			std::uint32_t h = 0x811c9dc5;
			while (begin != end) {
				h = (h ^ (*begin)) * 0x01000193;
				++begin;
			}
			return h;
		};
		template<size_t N> static constexpr std::uint32_t hash(const char(&str)[N]) noexcept { return hash(std::begin(str), std::end(str) - 1); };
		static constexpr std::uint32_t hash(std::string_view sv) noexcept { return hash(sv.begin(), sv.end()); };

		struct Operators {
			enum class Opers {
				equals,
				less_than,
				greater_than,
				less_than_equal,
				greater_than_equal,
				not_equal,
				assign_if_null,
				assign,
				pre_increment,
				pre_decrement,
				assign_product,
				assign_sum,
				assign_quotient,
				assign_difference,
				assign_bitwise_and,
				assign_bitwise_or,
				assign_shift_left,
				assign_shift_right,
				assign_remainder,
				assign_bitwise_xor,
				shift_left,
				shift_right,
				remainder,
				bitwise_and,
				bitwise_or,
				bitwise_xor,
				bitwise_complement,
				sum,
				quotient,
				product,
				difference,
				unary_plus,
				unary_minus,
				invalid,
				logical_list
			};

			constexpr static const char* to_string(Opers t_oper) noexcept {
				constexpr const char* opers[]
					= { "", "==", "<", ">", "<=", ">=", "!=", "?=", "", "=", "++", "--", "*=", "+=", "/=", "-=", "", "&=", "|=", "<<=", ">>=", "%=", "^=", "", "<<", ">>", "%", "&", "|", "^", "~", "", "+", "/", "*", "-", "+", "-", "", ".." };
				return opers[static_cast<int>(t_oper)];
			}

			constexpr static Opers to_operator(std::string_view t_str, bool t_is_unary = false) noexcept {
				const auto op_hash = hash(t_str);
				switch (op_hash) {
				case hash("?="): {
					return Opers::assign_if_null;
				}
				case hash("=="): {
					return Opers::equals;
				}
				case hash("<"): {
					return Opers::less_than;
				}
				case hash(">"): {
					return Opers::greater_than;
				}
				case hash("<="): {
					return Opers::less_than_equal;
				}
				case hash(">="): {
					return Opers::greater_than_equal;
				}
				case hash("!="): {
					return Opers::not_equal;
				}
				case hash("="): {
					return Opers::assign;
				}
				case hash("++"): {
					return Opers::pre_increment;
				}
				case hash("--"): {
					return Opers::pre_decrement;
				}
				case hash("*="): {
					return Opers::assign_product;
				}
				case hash("+="): {
					return Opers::assign_sum;
				}
				case hash("-="): {
					return Opers::assign_difference;
				}
				case hash("&="): {
					return Opers::assign_bitwise_and;
				}
				case hash("|="): {
					return Opers::assign_bitwise_or;
				}
				case hash("<<="): {
					return Opers::assign_shift_left;
				}
				case hash(">>="): {
					return Opers::assign_shift_right;
				}
				case hash("%="): {
					return Opers::assign_remainder;
				}
				case hash("^="): {
					return Opers::assign_bitwise_xor;
				}
				case hash("<<"): {
					return Opers::shift_left;
				}
				case hash(">>"): {
					return Opers::shift_right;
				}
				case hash("%"): {
					return Opers::remainder;
				}
				case hash("&"): {
					return Opers::bitwise_and;
				}
				case hash("|"): {
					return Opers::bitwise_or;
				}
				case hash("^"): {
					return Opers::bitwise_xor;
				}
				case hash("~"): {
					return Opers::bitwise_complement;
				}
				case hash("+"): {
					return t_is_unary ? Opers::unary_plus : Opers::sum;
				}
				case hash("-"): {
					return t_is_unary ? Opers::unary_minus : Opers::difference;
				}
				case hash("/"): {
					return Opers::quotient;
				}
				case hash("*"): {
					return Opers::product;
				}
				case hash(".."): {
					return Opers::logical_list;
				}
				default: {
					return Opers::invalid;
				}
				}
			};
		};
	};
	namespace Engine2 {
		struct AST_Node {
		public:
			Engine::AST_Node_Type 
				identifier; // node type
			GL::string 
				text; // processed text, relevant to the node type
			Engine::Parse_Location 
				location; // start & end Position within provided script

			const Engine::Position& start() const noexcept { return location.start; }
			const Engine::Position& end() const noexcept { return location.end; }

			GL::string pretty_print() const {
				auto children = get_children();
				if (children.size() > 0) {
					GL::string out;
					for (auto& elem : children) {
						out = out.add_to_delim(elem.get().pretty_print(), ", ");
					}
					return text + "{" + out + "}";
				}
				else {
					return text;
				}
			};

			virtual std::vector<std::reference_wrapper<AST_Node>> get_children() const = 0;
			virtual void compile(GL::scope::impl::BasicScope* currentScope) const = 0;
			virtual GL::any::fast_any eval(GL::scope::impl::BasicScope* currentScope) const = 0;
			virtual GL::type return_type() const = 0;

			/// Prints the contents of an AST node, including its children, recursively
			GL::string to_string(const GL::string& t_prepend = "") const {
				GL::string Text{ text };
				GL::string str = std::string(identifier.ToString());
				GL::string returnType = return_type().name();
				GL::string TimeSpentEvaling = std::to_string(TimeSpent_Total());
				GL::string locationStr = location.to_string();
				auto out = t_prepend + "(" + TimeSpentEvaling.add_to_delim(str, " ") + ") \"" + Text + "\": " + locationStr + " -> " + returnType;
				for (auto& elem : get_children()) { out = out.add_to_delim(elem.get().to_string(t_prepend + "\t"), "\n"); }
				return out;
			};

			virtual ~AST_Node() noexcept = default;
			AST_Node(AST_Node&&) = default;
			AST_Node& operator=(AST_Node&&) = delete;
			AST_Node(const AST_Node&) = delete;
			AST_Node& operator=(const AST_Node&) = delete;
			virtual GL::second TimeSpent_Total() const { return 0; };
			virtual GL::second TimeSpent_Self() const { return 0; };

		protected:
			AST_Node(GL::string t_ast_node_text, Engine::AST_Node_Type t_id, Engine::Parse_Location t_loc)
				: identifier(t_id)
				, text(std::move(t_ast_node_text))
				, location(std::move(t_loc))
			{}
		};
		class exception {
		public:
			/// Errors generated during parsing or evaluation
			struct eval_error : std::runtime_error {
				GL::string reason;
				Engine::Position start_position;
				GL::string filename;
				GL::string detail;

				eval_error(const GL::string& t_why, const Engine::Position& t_where, const GL::string& t_fname = "__EVAL__") noexcept
					: std::runtime_error(format(t_why, t_where, t_fname).to_string())
					, reason(t_why)
					, start_position(t_where)
					, filename(t_fname) {
				}

				explicit eval_error(const GL::string& t_why) noexcept
					: std::runtime_error(t_why.to_string())
					, reason(t_why) {
				}

				eval_error(const eval_error&) = default;

				GL::string pretty_print() const {
					std::ostringstream ss;
					return ss.str();
				};

				~eval_error() noexcept override = default;

			private:
				template<typename T> static Engine::AST_Node_Type id(const T& t) noexcept {
					return t.identifier;
				};
				template<typename T> static GL::string pretty(const T& t) {
					return t.pretty_print();
				};
				template<typename T> static const GL::string& fname(const T& t) noexcept {
					return t.filename();
				};
				template<typename T> static GL::string startpos(const T& t) {
					std::ostringstream oss;
					oss << t.start().line << ", " << t.start().column;
					return oss.str();
				};
				static GL::string format_why(const GL::string& t_why) { return "Error: \"" + t_why + "\""; };
				template<typename T> static GL::string format_location(const T& t) {
					std::ostringstream oss;
					oss << "(" << t.filename() << " " << t.start().line << ", " << t.start().column << ")";
					return oss.str();
				};
				static GL::string format_filename(const GL::string& t_fname) {
					std::stringstream ss;

					if (t_fname != "__EVAL__") {
						ss << "in '" << t_fname << "' ";
					}
					else {
						ss << "during evaluation ";
					}

					return ss.str();
				};
				static GL::string format_location(const Engine::Position& t_where) {
					std::stringstream ss;
					ss << "at (" << t_where.line << ", " << t_where.col << ")";
					return ss.str();
				};
				static GL::string format(const GL::string& t_why, const Engine::Position& t_where, const GL::string& t_fname) {
					std::stringstream ss;

					ss << format_why(t_why);
					ss << " ";

					ss << format_filename(t_fname);
					ss << " ";

					ss << format_location(t_where);

					return ss.str();
				};
			};
		};
		class Compiler {
		public:
			// The preprocessor should take in source code and perform substitutions based on preprocessor directives
			class Preprocessor {
			public:
				class PreprocessorState {
				public:
					struct word_t {
						GL::string word;
						size_t pos_start;
					};
				public:
					std::vector<std::pair<GL::string, Engine::Parse_Location>>
						Final_Script;
					std::map<
						GL::string, // e.g. macro name (to be found)
						GL::string> // content (to be replaced with)
						macro_definitions;
					std::map<GL::string, // e.g. function name (e.g. add_together)
						std::pair<
						GL::string, // function content (e.g. x + y)
						std::vector<GL::string> // function variables to look for and replace (e.g. x, y)
						> > macro_functions;
					std::vector<std::pair<GL::string, Engine::Parse_Location>>
						preprocessor_warnings;

				public:
					PreprocessorState() { // Build-In Preprocessor Macros
						Define("__VERSION__",
							"1.0"
						);
						Define("__DATE__",
							std::to_string(GL::datetime::Now().tm_mon() + 1) + "/"
							+ std::to_string(GL::datetime::Now().tm_mday()) + "/"
							+ std::to_string(GL::datetime::Now().tm_year() + 1900)
						);
						Define("__TIME__",
							std::to_string(GL::datetime::Now().tm_hour()) + ":"
							+ std::to_string(GL::datetime::Now().tm_min()) + ":"
							+ std::to_string(GL::datetime::Now().tm_sec())
						);
						Define("__TIMESTAMP__",
							GL::datetime::Now().c_str()
						);
					};

				public:
					GL::string
						GetFinalScript() const {
						GL::string y;
						for (auto& x : Final_Script) {
							y = y + x.first;
						}
						return y;
					};
					void
						PrintFinalScript() const {
						for (auto& x : Final_Script) {
							print(x.first + "\t\t\t" + GL::printf("L%iC%i-L%iC%i", x.second.start.operator GL::Engine::File_Position().line, x.second.start.operator GL::Engine::File_Position().column, x.second.end.operator GL::Engine::File_Position().line, x.second.end.operator GL::Engine::File_Position().column));
						}
					};
					bool Define(GL::string const& Name, GL::string const& Content) {
						macro_definitions[RemoveLeadingAndTrailingWhiteSpace(Name)] = RemoveLeadingAndTrailingWhiteSpace(Content);
						return true;
					};
					bool Define(GL::string const& Name, std::vector<GL::string> variables, GL::string const& Content) {
						macro_functions[RemoveLeadingAndTrailingWhiteSpace(Name)] = { RemoveLeadingAndTrailingWhiteSpace(Content), variables };
						return true;
					};
					bool Undefine(GL::string const& name) {
						auto Name = RemoveLeadingAndTrailingWhiteSpace(name);
						if (macro_definitions.find(Name) != macro_definitions.end()) {
							macro_definitions.erase(Name);
							return true;
						}
						if (macro_functions.find(Name) != macro_functions.end()) {
							macro_functions.erase(Name);
							return true;
						}
						return false;
					};
					// To-Do
					bool Include(GL::string const& IncludePath) {
						auto include_path = RemoveLeadingAndTrailingWhiteSpaceAndQuotes(IncludePath);
						auto key = include_path + "_" + macro_definitions["__DATE__"] + "_" + macro_definitions["__VERSION__"];
						if (this->macro_definitions.find(key) != this->macro_definitions.end()) {
							return false;
						}
						else {
							Define(key, IncludePath);
							return true;
						}
					};

					static std::vector<word_t> GetAllWords(GL::string const& text) {
						std::vector<word_t> out;
						word_t this_word;
						size_t current_pos = 0;
						this_word.pos_start = current_pos;
						bool got_anything = false;
						for (const char& c : text) {
							++current_pos;
							// A-z0-9_#
							if ((c >= 'A') && (c <= 'z')) {
								got_anything = true;
							}
							else if ((c >= '0') && (c <= '9')) {
								got_anything = true;
							}
							else if ((c == '#') || (c == '_')) {
								got_anything = true;
							}							
							else {
								if (got_anything) {
									this_word.word = text.substr(this_word.pos_start, ((current_pos-1) - this_word.pos_start));
									out.push_back(this_word);
								}
								// skip
								this_word.pos_start = current_pos;
								got_anything = false;
							}
						}
						if (got_anything && (this_word.pos_start < text.length())) {
							this_word.word = text.substr(this_word.pos_start, text.length() - this_word.pos_start);
							out.push_back(this_word);
						}
						return out;







						//std::vector<word_t> out; {
						//	static std::regex pattern{ R"("[^"]*"|\/\/[^\n]*\n|\/\*[^\*\/]*\*\/|[A-z0-9_#]+)" };
						//	size_t position, length;
						//	for (auto i = std::regex_iterator(text.begin(), text.end(), pattern);
						//		i != decltype(i)();
						//		++i)
						//	{
						//		const auto& m = *i;
						//		position = ((text.length() - m.suffix().length()) - m.length());
						//		length = m.length();
						//		out.push_back(word_t{ text.substr(position, length), position });
						//	}
						//}
						//return out;
					};
					static bool TryGetFunctionParams(GL::string const& text, std::vector<GL::string>& splits) {
						GL::string to_split = text.remove_leading_and_trailing(' ');
						bool retval = false;
						std::regex pattern(R"(\(\s*([^)]+?)\s*\))");
						for (auto i = std::regex_iterator(to_split.begin(), to_split.end(), pattern);
							i != decltype(i)();
							++i) {
							auto const& m = *i;
							retval = true;
							to_split = m.str();
							if (m.prefix().str().find(" ") != GL::string::npos) {
								return false;
							}
							else {
								break;
							}
						}
						if (retval && (to_split.size() >= 2)) {
							splits = to_split.remove_leading('(').remove_trailing(')').split(",");
							for (auto& split : splits) {
								split = RemoveLeadingAndTrailingWhiteSpace(split);
							}
						}
						return retval;
					};
					bool TryReplace(GL::string& Text, word_t const& Where, GL::string const& Find, GL::string const& ReplaceWith) {
						// we don't do replacements on strings -- their content should be left as-is
						if (Where.word.length() >= 2) {
							if ((Where.word[0] == '\"') && (Where.word[Where.word.size() - 1] == '\"')) {
								return false;
							}
						}

						// we don't do replacements on comments -- their content should be left as-is
						if (Where.word.length() >= 2) {
							if (Where.word.find("//") == 0) {
								return false;
							}
							if (Where.word.find("/*") == 0) {
								return false;
							}
						}

						// we can't find nothing	
						if (Find.empty()) return false;

						// exact word match
						if (Where.word == Find) {
							if (Where.pos_start == 0) {
								Text = ExpandCode(ReplaceWith) + ExpandCode(Text.substr(Where.word.length()));
							}
							else if ((Where.pos_start + Where.word.length()) == Text.length()) {
								Text = ExpandCode(Text.substr(0, Where.pos_start)) + ExpandCode(ReplaceWith);
							}
							else {
								auto t0 = ExpandCode(Text.substr(0, Where.pos_start));
								auto t1 = ExpandCode(ReplaceWith);
								auto t2 = ExpandCode(Text.substr(Where.pos_start + Where.word.length()));
								Text = t0 + t1 + t2;
							}
							return true;
						}

						// in-word replacement when explicitely requested
						if (auto f_p = Where.word.find("##" + Find); f_p != GL::string::npos) {
							auto expanded_lhs = ExpandCode(Where.word.left(f_p));
							auto NewReplaceWith = expanded_lhs + Replace(Where.word.right(Where.word.length() - f_p), "##" + Find, ExpandCode(ReplaceWith));

							if (Where.pos_start == 0) {
								Text = NewReplaceWith + ExpandCode(Text.substr(Where.word.length()));
							}
							else if ((Where.pos_start + (Where.word.length())) == Text.length()) {
								Text = ExpandCode(Text.substr(0, Where.pos_start)) + NewReplaceWith;
							}
							else {
								Text = ExpandCode(Text.substr(0, Where.pos_start)) + NewReplaceWith + ExpandCode(Text.substr(Where.pos_start + (Where.word.length())));
							}
							return true;
						}

						// in-word string replacement when explicitely requested
						if (auto f_p = Where.word.find("#" + Find); f_p != GL::string::npos) {
							auto expanded_lhs = ExpandCode(Where.word.left(f_p));
							auto NewReplaceWith = expanded_lhs + Replace(Where.word.right(Where.word.length() - f_p), "#" + Find, "\"" + ExpandCode(ReplaceWith) + "\"");

							if (Where.pos_start == 0) {
								Text = NewReplaceWith + ExpandCode(Text.substr(Where.word.length()));
							}
							else if ((Where.pos_start + (Where.word.length())) == Text.length()) {
								Text = ExpandCode(Text.substr(0, Where.pos_start)) + NewReplaceWith;
							}
							else {
								Text = ExpandCode(Text.substr(0, Where.pos_start)) + NewReplaceWith + ExpandCode(Text.substr(Where.pos_start + (Where.word.length())));
							}
							return true;
						}

						if (Where.word.begins_with("#") && !Where.word.begins_with("##")) {
							Text = ExpandCode(Text.substr(0, Where.pos_start)) + "\"" + ExpandCode(Where.word.right(Where.word.length() - 1)) + "\"" + ExpandCode(Text.substr(Where.pos_start + (Where.word.length())));
							return true;
						}

						return false;
					};
					static GL::string Replace(GL::string const& text, GL::string const& find, GL::string const& replaceWith) {
						return text.replace(find, replaceWith);
					};
					static GL::string RemoveLeadingAndTrailingWhiteSpace(GL::string const& text) {
						return text.remove_leading_and_trailing(' ').remove_leading_and_trailing('\n').remove_leading_and_trailing('\t');
					};
					static GL::string RemoveLeadingAndTrailingWhiteSpaceAndQuotes(GL::string const& text) {
						return RemoveLeadingAndTrailingWhiteSpace(text).remove_leading_and_trailing('\"');
					};

					GL::string ExpandCode(GL::string Code) {
						bool MadeAnyChanges = true;
						int maxIterations = 1000000;
						while (MadeAnyChanges && (--maxIterations >= 0)) {
							// note, do not perform this work if we do not have to.
							bool anyReason = false;
							for (auto& macro_definition : macro_definitions) {
								if (Code.find(macro_definition.first) != GL::string::npos) {
									anyReason = true;
									break;
								}
							}
							for (auto& macro_definition : macro_functions) {
								if (Code.find(macro_definition.first) != GL::string::npos) {
									anyReason = true;
									break;
								}
							}
							if (!anyReason) break;

							MadeAnyChanges = false;
							auto words = GetAllWords(Code);

							if (MadeAnyChanges) continue;
							for (auto& macro_definition : macro_definitions) {
								if (MadeAnyChanges) break;
								for (auto& word : words) {
									if (MadeAnyChanges) break;
									MadeAnyChanges = TryReplace(Code, word, macro_definition.first, macro_definition.second);
								}
							}

							if (MadeAnyChanges) continue;
							for (auto& macro_function : macro_functions) {
								if (MadeAnyChanges) break;
								for (int word_index = 0; word_index < words.size(); word_index++) {
									if (MadeAnyChanges) break;

									if (words[word_index].word == macro_function.first) { // print
										// see if the next non-empty character is a '(' character
										size_t start_pos = words[word_index].pos_start + words[word_index].word.length();
										size_t end_pos;
										while (Code[start_pos] == ' ' || Code[start_pos] == '\t') { start_pos++; }
										if (Code[start_pos] == '(') {
											size_t commaCount = macro_function.second.second.size() - 1;
											std::vector<GL::string> functionParams;

											// find the end to this, just in case we need it later
											end_pos = start_pos;
											int parenCount = 1;
											size_t currentVarStart = start_pos + 1;
											while ((parenCount > 0) && (++end_pos < Code.size())) {
												if (Code[end_pos] == '(') parenCount++;
												else if (Code[end_pos] == '[') parenCount++;
												else if (Code[end_pos] == '{') parenCount++;
												else if (Code[end_pos] == ')') parenCount--;
												else if (Code[end_pos] == ']') parenCount--;
												else if (Code[end_pos] == '}') parenCount--;

												if ((parenCount == 1) && (Code[end_pos] == ',')) {
													functionParams.push_back(Code.substr(currentVarStart, (end_pos)-currentVarStart));
													currentVarStart = end_pos + 1ull;
												}
											}
											if ((currentVarStart < end_pos) && (functionParams.size() < macro_function.second.second.size())) {
												functionParams.push_back(Code.substr(currentVarStart, (end_pos)-currentVarStart));
											}

											if (parenCount > 0) continue; // we didn't get an actual function decl

											// we have a candidate -- ensure there are enough "vars" for the variables
											if (functionParams.size() >= macro_function.second.second.size()) {
												auto& function_name = macro_function.first;
												auto& function_content = macro_function.second.first;
												auto& function_var_names = macro_function.second.second;
												auto& function_vars = functionParams;

												PreprocessorState newState;
												for (int var_index = 0; var_index < function_var_names.size(); var_index++) {
													newState.Define(function_var_names[var_index], function_vars[var_index]);
												}
												auto implimented_function = newState.ExpandCode(function_content);

												auto Fm = Code.substr(words[word_index].pos_start, (end_pos - words[word_index].pos_start) + 1);
												MadeAnyChanges = TryReplace(Code, word_t{
													 Fm,
													 words[word_index].pos_start
											    }, Fm, implimented_function);
											}
										}
									}
								}
							}
						}
						return Code;
					};
				};
				class PreprocessorToken {
				private:
					GL::string
						Text;
				public:
					const Engine::PreprocessorDirectives
						identifier;
					const GL::string
						text;
					Engine::Parse_Location
						location;
					std::vector<std::shared_ptr<PreprocessorToken>>
						children;

					const Engine::Position& start() const noexcept { return location.start; }
					const Engine::Position& end() const noexcept { return location.end; }

					GL::string pretty_print() const {
						GL::string out = text;
						for (auto& elem : children) {
							out = out.add_to_delim(elem->pretty_print(), " ");
						}
						return out;
					};

					virtual void GenerateExpandedCode(PreprocessorState& state) const = 0;

					static bool replace(GL::string& str, const GL::string& from, const GL::string& to) {
						if (str.find(from) != GL::string::npos) {
							str = str.replace(from, to);
							return true;
						}
						return false;
					};
					static bool replaceAll(GL::string& str, const GL::string& from, const GL::string& to) {
						bool out = false;
						while (str.find(from) != GL::string::npos) {
							str = str.replace(from, to);
							out = true;
						}
						return out;
					};

					/// Prints the contents of an AST node, including its children, recursively
					GL::string to_string(const GL::string& t_prepend = "") const {
						GL::string str = std::string_view(this->identifier.ToString());
						GL::string data = this->text;

						replaceAll(data, "\n", "\\n");
						replaceAll(data, "\r", "\\r");
						replaceAll(data, "\t", "\\t");

						auto out = t_prepend + "(" + str  +") " + data  +" : " + GL::printf("L%iC%i - L%iC%i\n", this->location.start.line, this->location.start.col, this->location.end.line, this->location.end.col);

						for (auto& elem : children) {
							out = out.add_to_delim(elem->to_string(), " ");
						}

						return out;
					}

					virtual ~PreprocessorToken() noexcept = default;
					PreprocessorToken(PreprocessorToken&&) = default;
					PreprocessorToken& operator=(PreprocessorToken&&) = delete;
					PreprocessorToken(const PreprocessorToken&) = delete;
					PreprocessorToken& operator=(const PreprocessorToken&) = delete;

				protected:
					PreprocessorToken(GL::string t_ast_node_text, Engine::PreprocessorDirectives t_id, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: identifier(t_id)
						, text()
						, location(std::move(t_loc))
						, children(std::move(t_children))
						, Text()
					{
						if ((this->identifier == Engine::PreprocessorDirectives::None) || (this->identifier == Engine::PreprocessorDirectives::Completed)) {
							// accept the script as-is, no change. 
							const_cast<GL::string&>(text) = t_ast_node_text;
						}
						else {
							// remove the leading and tailing white space
							t_ast_node_text = PreprocessorState::RemoveLeadingAndTrailingWhiteSpace(t_ast_node_text);
							Text = t_ast_node_text;
							while (replaceAll(Text, "\\\n\t", "\\\n")
								|| replaceAll(Text, "\\\n ", "\\\n")
								|| replaceAll(Text, "\\\r\t", "\\\r")
								|| replaceAll(Text, "\\\r ", "\\\r")
								) {
							}
							replaceAll(Text, "\\\n", "\n");
							replaceAll(Text, "\\\r", "\r");
							const_cast<GL::string&>(text) = Text;
						}
					}
				};
				using PreprocessorTokenPtr = typename std::shared_ptr<PreprocessorToken>;
				class CompletedPreprocessor final : public PreprocessorToken {
				public:
					CompletedPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::Completed, std::move(t_loc), std::move(t_children)) {}
					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class NonePreprocessor final : public PreprocessorToken {
				public:
					NonePreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::None, std::move(t_loc), std::move(t_children)) {}
					void GenerateExpandedCode(PreprocessorState& state) const override {
						state.Final_Script.push_back({ state.ExpandCode(this->text), this->location });

						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class DefinePreprocessor final : public PreprocessorToken {
				public:
					DefinePreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::Define, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						// this->text will include something like:
						// __VERSION__ 
						//       |--> this should be defined as a definition named "__VERSION__" with content ""
						// 
						// __VERSION__ 100
						//       |--> this should be defined as a definition named "__VERSION__" with content "100"
						// 
						// __VERSION__(x) x + 1
						//       |--> this should be defined as a function "__VERSION__" with param "x" and content "x + 1"

						auto thisText = state.RemoveLeadingAndTrailingWhiteSpace(this->text);
						auto words = state.GetAllWords(thisText);
						if (words.size() > 0) {
							auto defName = words[0].word;
							std::vector<GL::string> var_names;
							if (state.TryGetFunctionParams(thisText, var_names)) {
								// is function
								state.Define(defName, var_names, thisText.substr(thisText.find(")") + 1));
							}
							else {
								// is basic definition
								auto sub = thisText.substr(defName.length());
								auto potentialDef = state.RemoveLeadingAndTrailingWhiteSpace(sub);
								while ((potentialDef.size() > 0) && (potentialDef[0] == '=')) {
									potentialDef.remove_prefix(1);
								}
								potentialDef = state.RemoveLeadingAndTrailingWhiteSpace(potentialDef);
								if (potentialDef.size() > 0) {
									state.Define(defName, potentialDef);
								}
								else {
									state.Define(defName, "");
								}
							}
						}
					};
				};
				class IncludePreprocessor final : public PreprocessorToken {
				public:
					IncludePreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::Include, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						if (state.Include(this->text)) {
							GL::string downloadedScript;
							downloadedScript = /*state.ExpandCode(*/R"(		
								#define __print_todays_date__ print(__DATE__)
								namespace string {
									string ReplaceOnce(string findWhat, string With){
										__print_todays_date__;
									};
									string ReplaceAll(string findWhat, string With){
										__print_todays_date__
									};
								};
							)"/*)*/;
							const_cast<IncludePreprocessor*>(this)->children.push_back(Preprocessor().Parse(downloadedScript));
						}
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class UndefinePreprocessor final : public PreprocessorToken {
				public:
					UndefinePreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::Undefine, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						state.Undefine(this->text);
					};
				};
				class IfChainPreprocessor final : public PreprocessorToken {
				public:
					IfChainPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::IfChain, std::move(t_loc), {}) {

						// the first child will either be an If or an IfDefined
						// the final child will always be an EndIf

						std::shared_ptr<PreprocessorToken> currentIfStatement = t_children[0];
						int currentIfStatementIndex = 0;

						for (int i = 1; i < (t_children.size() - 1); ++i) {
							if (t_children[i]->identifier == Engine::PreprocessorDirectives::ElseIf) {
								// end the current PreprocessorToken and add it to our children
								for (int j = currentIfStatementIndex + 1; j < i; ++j) {
									currentIfStatement->children.push_back(t_children[j]);
								}
								this->children.push_back(currentIfStatement);
								currentIfStatement = t_children[i];
								currentIfStatementIndex = i;
							}
							else if (t_children[i]->identifier == Engine::PreprocessorDirectives::Else) {
								// end the current PreprocessorToken and add it to our children
								for (int j = currentIfStatementIndex + 1; j < i; ++j) {
									currentIfStatement->children.push_back(t_children[j]);
								}
								this->children.push_back(currentIfStatement);
								currentIfStatement = t_children[i];
								currentIfStatementIndex = i;
							}
							else {
								continue;
							}
						}
						// end the current PreprocessorToken and add it to our children
						for (int j = currentIfStatementIndex + 1; j < (t_children.size() - 1); ++j) {
							currentIfStatement->children.push_back(t_children[j]);
						}
						this->children.push_back(currentIfStatement);
						this->children.push_back(t_children[t_children.size() - 1]);
					}

					static bool Evaluate(PreprocessorState& state, Engine::PreprocessorDirectives const& NodeType, GL::string Text) {
						GL::string ToEvaluate;
						switch (NodeType) {
						case Engine::PreprocessorDirectives::If:
						case Engine::PreprocessorDirectives::ElseIf: {
							// throw std::runtime_error("To-Do: Need to support calling the parser from the compiler to evaluate compile-time equations.");

							ToEvaluate = state.ExpandCode(Text);

							GL::scope::impl::RootScope temp_master_scope;
							temp_master_scope.perform_builtins();

							std::cout << "TO-DO: UPDATE ME ONCE POSSIBLE\n";
							return true; 

							//auto parsed_result = Compiler::Interpreter::Parser().Parse(ToEvaluate, &temp_master_scope);
							//auto returned = parsed_result.first->eval(&temp_master_scope);
							//return temp_master_scope.cast<bool>(returned);
						}
						case Engine::PreprocessorDirectives::IfDefined:
							if (state.macro_definitions.find(Text) != state.macro_definitions.end()) {
								return true;
							}
							if (state.macro_functions.find(Text) != state.macro_functions.end()) {
								return true;
							}
							return false;
						case Engine::PreprocessorDirectives::IfNotDefined:
							return !Evaluate(state, Engine::PreprocessorDirectives::IfDefined, Text);
						default:
							return false;
						}
					}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							if (
								(child->identifier == Engine::PreprocessorDirectives::If)
								|| (child->identifier == Engine::PreprocessorDirectives::IfDefined)
								|| (child->identifier == Engine::PreprocessorDirectives::IfNotDefined)
								|| (child->identifier == Engine::PreprocessorDirectives::ElseIf)
								) {
								if (Evaluate(state, child->identifier, child->text)) {
									// end the search
									child->GenerateExpandedCode(state);
									return;
								}
							}
							else if (
								(child->identifier == Engine::PreprocessorDirectives::Else)
								) {
								if (true) {
									// end the search
									child->GenerateExpandedCode(state);
									return;
								}
							}
							else {
								// do nothing?
							}
						}
					};
				};
				class IfPreprocessor final : public PreprocessorToken {
				public:
					IfPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::If, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class ElseIfPreprocessor final : public PreprocessorToken {
				public:
					ElseIfPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::ElseIf, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class ElsePreprocessor final : public PreprocessorToken {
				public:
					ElsePreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::Else, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class EndIfPreprocessor final : public PreprocessorToken {
				public:
					EndIfPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::EndIf, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class IfDefinedPreprocessor final : public PreprocessorToken {
				public:
					IfDefinedPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::IfDefined, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class IfNotDefinedPreprocessor final : public PreprocessorToken {
				public:
					IfNotDefinedPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::IfNotDefined, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						for (auto& child : this->children) {
							child->GenerateExpandedCode(state);
						}
					};
				};
				class ErrorPreprocessor final : public PreprocessorToken {
				public:
					ErrorPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::Error, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						auto orig_error = this->text;
						throw exception::eval_error("Compilation error was thrown: " + state.RemoveLeadingAndTrailingWhiteSpace(orig_error), this->location.start);
					};
				};
				class WarningPreprocessor final : public PreprocessorToken {
				public:
					WarningPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::Warning, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						state.preprocessor_warnings.push_back({ this->text, this->location });
					};
				};
				// To-Do: Add pragma support, to enable changes to the compiler.
				class PragmaPreprocessor final : public PreprocessorToken {
				public:
					PragmaPreprocessor(GL::string const& t_ast_node_text, Engine::Parse_Location t_loc, std::vector<std::shared_ptr<PreprocessorToken>> t_children)
						: PreprocessorToken(t_ast_node_text, Engine::PreprocessorDirectives::Pragma, std::move(t_loc), std::move(t_children)) {}

					void GenerateExpandedCode(PreprocessorState& state) const override {
						// To-Do
					};
				};

			public:
				Preprocessor() = default;
				~Preprocessor() = default;

				PreprocessorTokenPtr Parse(GL::string t_input) { return parse(t_input); };

			private:
				Engine::Position m_position{};
				std::vector<PreprocessorTokenPtr> m_match_stack;

			private:
				template<typename string_type>
				struct Char_Parser {
					string_type& match;
					using char_type = typename string_type::value_type;
					bool is_escaped = false;
					bool is_interpolated = false;
					bool saw_interpolation_marker = false;
					bool is_octal = false;
					bool is_hex = false;
					std::size_t unicode_size = 0;
					const bool interpolation_allowed;

					string_type octal_matches;
					string_type hex_matches;

					Char_Parser(string_type& t_match, const bool t_interpolation_allowed)
						: match(t_match)
						, interpolation_allowed(t_interpolation_allowed) {
					}

					Char_Parser& operator=(const Char_Parser&) = delete;

					~Char_Parser() {
						try {
							if (is_octal) {
								process_octal();
							}

							if (is_hex) {
								process_hex();
							}

							if (unicode_size > 0) {
								process_unicode();
							}
						}
						catch (const std::invalid_argument&) {
						}
						catch (const exception::eval_error&) {
							// Something happened with parsing, we'll catch it later?
						}
					}

					void process_hex() {
						if (!hex_matches.empty()) {
							auto val = stoll(hex_matches, nullptr, 16);
							match.push_back(char_type(val));
						}
						hex_matches.clear();
						is_escaped = false;
						is_hex = false;
					}

					void process_octal() {
						if (!octal_matches.empty()) {
							auto val = stoll(octal_matches, nullptr, 8);
							match.push_back(char_type(val));
						}
						octal_matches.clear();
						is_escaped = false;
						is_octal = false;
					}

					void process_unicode() {
						const auto ch = static_cast<uint32_t>(std::stoi(hex_matches, nullptr, 16));
						const auto match_size = hex_matches.size();
						hex_matches.clear();
						is_escaped = false;
						const auto u_size = unicode_size;
						unicode_size = 0;

						char buf[4];
						if (u_size != match_size) {
							throw exception::eval_error("Incomplete unicode escape sequence");
						}
						if (u_size == 4 && ch >= 0xD800 && ch <= 0xDFFF) {
							throw exception::eval_error("Invalid 16 bit universal character");
						}

						if (ch < 0x80) {
							match += static_cast<char>(ch);
						}
						else if (ch < 0x800) {
							buf[0] = static_cast<char>(0xC0 | (ch >> 6));
							buf[1] = static_cast<char>(0x80 | (ch & 0x3F));
							match.append(buf, 2);
						}
						else if (ch < 0x10000) {
							buf[0] = static_cast<char>(0xE0 | (ch >> 12));
							buf[1] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
							buf[2] = static_cast<char>(0x80 | (ch & 0x3F));
							match.append(buf, 3);
						}
						else if (ch < 0x200000) {
							buf[0] = static_cast<char>(0xF0 | (ch >> 18));
							buf[1] = static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
							buf[2] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
							buf[3] = static_cast<char>(0x80 | (ch & 0x3F));
							match.append(buf, 4);
						}
						else {
							// this must be an invalid escape sequence?
							throw exception::eval_error("Invalid 32 bit universal character");
						}
					}

					void parse(const char_type t_char, Engine::Position pos) {
						const bool is_octal_char = t_char >= '0' && t_char <= '7';

						const bool is_hex_char = (t_char >= '0' && t_char <= '9') || (t_char >= 'a' && t_char <= 'f') || (t_char >= 'A' && t_char <= 'F');

						if (is_octal) {
							if (is_octal_char) {
								octal_matches.push_back(t_char);

								if (octal_matches.size() == 3) {
									process_octal();
								}
								return;
							}
							else {
								process_octal();
							}
						}
						else if (is_hex) {
							if (is_hex_char) {
								hex_matches.push_back(t_char);

								if (hex_matches.size() == 2 * sizeof(char_type)) {
									// This rule differs from the C/C++ standard, but ChaiScript
									// does not offer the same workaround options, and having
									// hexadecimal sequences longer than can fit into the char
									// type is undefined behavior anyway.
									process_hex();
								}
								return;
							}
							else {
								process_hex();
							}
						}
						else if (unicode_size > 0) {
							if (is_hex_char) {
								hex_matches.push_back(t_char);

								if (hex_matches.size() == unicode_size) {
									// Format is specified to be 'slash'uABCD
									// on collecting from A to D do parsing
									process_unicode();
								}
								return;
							}
							else {
								// Not a unicode anymore, try parsing any way
								// May be someone used 'slash'uAA only
								process_unicode();
							}
						}

						if (t_char == '\\') {
							if (is_escaped) {
								match.push_back('\\');
								is_escaped = false;
							}
							else {
								is_escaped = true;
							}
						}
						else {
							if (is_escaped) {
								if (is_octal_char) {
									is_octal = true;
									octal_matches.push_back(t_char);
								}
								else if (t_char == 'x') {
									is_hex = true;
								}
								else if (t_char == 'u') {
									unicode_size = 4;
								}
								else if (t_char == 'U') {
									unicode_size = 8;
								}
								else {
									switch (t_char) {
									case ('\''):
										match.push_back('\'');
										break;
									case ('\"'):
										match.push_back('\"');
										break;
									case ('?'):
										match.push_back('?');
										break;
									case ('a'):
										match.push_back('\a');
										break;
									case ('b'):
										match.push_back('\b');
										break;
									case ('f'):
										match.push_back('\f');
										break;
									case ('n'):
										match.push_back('\n');
										break;
									case ('r'):
										match.push_back('\r');
										break;
									case ('t'):
										match.push_back('\t');
										break;
									case ('v'):
										match.push_back('\v');
										break;
									case ('$'):
										match.push_back('$');
										break;
									default:
										throw exception::eval_error("Unknown escaped sequence in string", pos);
									}
									is_escaped = false;
								}
							}
							else if (interpolation_allowed && t_char == '$') {
								saw_interpolation_marker = true;
							}
							else {
								match.push_back(t_char);
							}
						}
					}
				};
				template<typename Array2D, typename First, typename Second>
				constexpr static void set_alphabet(Array2D& array, const First first, const Second second) noexcept {
					auto* first_ptr = &std::get<0>(array) + static_cast<std::size_t>(first);
					auto* second_ptr = &std::get<0>(*first_ptr) + static_cast<std::size_t>(second);
					*second_ptr = true;
				};
				static constexpr std::array<std::array<bool, Engine::lengthof_alphabet>, Engine::max_alphabet> build_alphabet() noexcept {
					std::array<std::array<bool, Engine::lengthof_alphabet>, Engine::max_alphabet> alphabet{};

					set_alphabet(alphabet, Engine::symbol_alphabet, '?');

					set_alphabet(alphabet, Engine::symbol_alphabet, '?');
					set_alphabet(alphabet, Engine::symbol_alphabet, '+');
					set_alphabet(alphabet, Engine::symbol_alphabet, '-');
					set_alphabet(alphabet, Engine::symbol_alphabet, '*');
					set_alphabet(alphabet, Engine::symbol_alphabet, '/');
					set_alphabet(alphabet, Engine::symbol_alphabet, '|');
					set_alphabet(alphabet, Engine::symbol_alphabet, '&');
					set_alphabet(alphabet, Engine::symbol_alphabet, '^');
					set_alphabet(alphabet, Engine::symbol_alphabet, '=');
					set_alphabet(alphabet, Engine::symbol_alphabet, '.');
					set_alphabet(alphabet, Engine::symbol_alphabet, '<');
					set_alphabet(alphabet, Engine::symbol_alphabet, '>');

					for (size_t c = 'a'; c <= 'z'; ++c) {
						set_alphabet(alphabet, Engine::keyword_alphabet, c);
					}
					for (size_t c = 'A'; c <= 'Z'; ++c) {
						set_alphabet(alphabet, Engine::keyword_alphabet, c);
					}
					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, Engine::keyword_alphabet, c);
					}
					set_alphabet(alphabet, Engine::keyword_alphabet, '_');
					// set_alphabet(alphabet, keyword_alphabet, ':');

					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, Engine::int_alphabet, c);
					}
					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, Engine::float_alphabet, c);
					}
					set_alphabet(alphabet, Engine::float_alphabet, '.');

					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, Engine::hex_alphabet, c);
					}
					for (size_t c = 'a'; c <= 'f'; ++c) {
						set_alphabet(alphabet, Engine::hex_alphabet, c);
					}
					for (size_t c = 'A'; c <= 'F'; ++c) {
						set_alphabet(alphabet, Engine::hex_alphabet, c);
					}

					set_alphabet(alphabet, Engine::x_alphabet, 'x');
					set_alphabet(alphabet, Engine::x_alphabet, 'X');

					for (size_t c = '0'; c <= '1'; ++c) {
						set_alphabet(alphabet, Engine::bin_alphabet, c);
					}
					set_alphabet(alphabet, Engine::b_alphabet, 'b');
					set_alphabet(alphabet, Engine::b_alphabet, 'B');

					for (size_t c = 'a'; c <= 'z'; ++c) {
						set_alphabet(alphabet, Engine::id_alphabet, c);
					}
					for (size_t c = 'A'; c <= 'Z'; ++c) {
						set_alphabet(alphabet, Engine::id_alphabet, c);
					}
					set_alphabet(alphabet, Engine::id_alphabet, '_');
					set_alphabet(alphabet, Engine::id_alphabet, ':'); // RG
					for (size_t c = '0'; c <= '9'; ++c) { set_alphabet(alphabet, Engine::id_alphabet, c); } // RG
					//set_alphabet(alphabet, Engine::id_alphabet, '<'); // RG
					//set_alphabet(alphabet, Engine::id_alphabet, '>'); // RG

					set_alphabet(alphabet, Engine::white_alphabet, ' ');
					set_alphabet(alphabet, Engine::white_alphabet, '\t');

					set_alphabet(alphabet, Engine::int_suffix_alphabet, 'l');
					set_alphabet(alphabet, Engine::int_suffix_alphabet, 'L');
					set_alphabet(alphabet, Engine::int_suffix_alphabet, 'u');
					set_alphabet(alphabet, Engine::int_suffix_alphabet, 'U');

					set_alphabet(alphabet, Engine::float_suffix_alphabet, 'l');
					set_alphabet(alphabet, Engine::float_suffix_alphabet, 'L');
					set_alphabet(alphabet, Engine::float_suffix_alphabet, 'f');
					set_alphabet(alphabet, Engine::float_suffix_alphabet, 'F');

					return alphabet;
				}
				bool char_in_alphabet(char c, Engine::Alphabet a) const noexcept { return m_alphabet[a][static_cast<uint8_t>(c)]; } // test a char in an m_alphabet

			private:
				std::array<std::array<bool, Engine::lengthof_alphabet>, Engine::max_alphabet> m_alphabet{ build_alphabet() };
				constexpr static utility::Static_String m_multiline_comment_end{ "*/" };
				constexpr static utility::Static_String m_multiline_comment_begin{ "/*" };
				constexpr static utility::Static_String m_singleline_comment{ "//" };
				constexpr static utility::Static_String m_annotation{ "#" };
				constexpr static utility::Static_String m_cr_lf{ "\r\n" };

			private:
				/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
				bool Symbol_(const utility::Static_String& sym) noexcept {
					const auto len = sym.size();
					if (m_position.remaining() >= len) {
						const char* file_pos = &(*m_position);
						for (size_t pos = 0; pos < len; ++pos) {
							if (sym.c_str()[pos] != file_pos[pos]) {
								return false;
							}
						}
						m_position += len;
						return true;
					}
					return false;
				};
				/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
				bool Symbol_(const GL::string& sym) noexcept {
					const auto len = sym.size();
					if (m_position.remaining() >= len) {
						const char* file_pos = &(*m_position);
						for (size_t pos = 0; pos < len; ++pos) {
							if (sym[pos] != file_pos[pos]) {
								return false;
							}
						}
						m_position += len;
						return true;
					}
					return false;
				};
				/// Reads a char from input if it matches the parameter, without skipping initial whitespace
				bool Char_(const char c) {
					if (m_position.has_more() && (*m_position == c)) {
						++m_position;
						return true;
					}
					else {
						return false;
					}
				};
				/// Reads an end-of-line group from input, without skipping initial whitespace
				bool Eol_(const bool t_eos = false) {
					bool retval = false;

					if (m_position.has_more() && (Symbol_(m_cr_lf) || Char_('\n'))) {
						retval = true;
						//++m_position.line;
						m_position.col = 1;
					}
					//else if (m_position.has_more() && !t_eos && Char_(';')) {
					//	retval = true;
					//}

					return retval;
				};
				/// Reads a string from input if it matches the parameter, without skipping initial whitespace
				bool Keyword_(const GL::string& t_s) {
					const auto len = t_s.size();
					if (m_position.remaining() >= len) {
						auto tmp = m_position;
						for (size_t i = 0; tmp.has_more() && i < len; ++i) {
							if (*tmp != t_s[i]) {
								return false;
							}
							++tmp;
						}
						m_position = tmp;
						return true;
					}

					return false;
				};

				/// Reads an identifier from input which conforms to C's identifier naming conventions, without skipping initial whitespace
				bool Id_() {
					if (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) {
						while (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) { //keyword_alphabet)) {
							++m_position;
						}

						return true;
					}
					else if (m_position.has_more() && (*m_position == '`')) {
						++m_position;
						const auto start = m_position;

						while (m_position.has_more() && (*m_position != '`')) {
							if (Eol()) {
								throw exception::eval_error("Carriage return in identifier literal", m_position);
							}
							else {
								++m_position;
							}
						}

						if (start == m_position) {
							throw exception::eval_error("Missing contents of identifier literal", m_position);
						}
						else if (!m_position.has_more()) {
							throw exception::eval_error("Incomplete identifier literal", m_position);
						}

						++m_position;

						return true;
					}
					return false;
				};

			private: // TO-DO, reimpliment the optimization sequence inside of "build_match"
				/// Helper function that collects ast_nodes from a starting position to the top of the stack into a new AST node
				template<typename NodeType>
				void build_match(size_t t_match_start, GL::string t_text) {
					bool is_deep = false;

					Engine::Parse_Location filepos = [&]() -> Engine::Parse_Location {
						// so we want to take everything to the right of this and make them children
						if (t_match_start != m_match_stack.size()) {
							is_deep = true;
							return Engine::Parse_Location(
								m_match_stack[t_match_start]->location.start,
								m_position);
						}
						else {
							return Engine::Parse_Location(m_position, m_position);
						}
					}();

					std::vector<PreprocessorTokenPtr> new_children;
					if (is_deep) {
						new_children.assign(std::make_move_iterator(m_match_stack.begin() + static_cast<int>(t_match_start)),
							std::make_move_iterator(m_match_stack.end()));
						m_match_stack.erase(m_match_stack.begin() + static_cast<int>(t_match_start), m_match_stack.end());
					}

					m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<NodeType>(t_text, std::move(filepos), std::move(new_children))));
				};

				/// create a node
				template<typename T, typename... Param>
				PreprocessorTokenPtr make_node(GL::string t_match, Engine::Position t_prev, Param &&...param) {
					auto out = std::make_shared<T>(
						t_match,
						Engine::Parse_Location(t_prev, m_position),
						std::forward<Param>(param)...
						);
					return std::dynamic_pointer_cast<PreprocessorToken>(out);
				};

			private:
				/// Skips whitespace, which means space and tab, but not cr/lf
				/// jespada: Modified SkipWS to skip optionally CR ('\n') and/or LF+CR ("\r\n")
				/// AlekMosingiewicz: Added exception when illegal character detected
				bool SkipWS(bool skip_cr = false) {
					bool retval = false;

					while (m_position.has_more()) {
						if (static_cast<unsigned char>(*m_position) > 0x7e) {
							throw exception::eval_error("Illegal character", m_position);
						}
						auto end_line = (*m_position != 0) && ((*m_position == '\n') || (*m_position == '\r' && *(m_position + 1) == '\n'));

						if (char_in_alphabet(*m_position, Engine::white_alphabet) || (skip_cr && end_line)) {
							if (end_line) {
								if (*m_position == '\r') {
									// discards lf
									++m_position;
								}
							}

							++m_position;

							retval = true;
						}
						else {
							break;
						}
					}
					return retval;
				};
				/// Reads until the end of the current statement
				bool Eos() {
					SkipWS();
					return Eol_(true);
				};
				/// Reads (and potentially captures) an end-of-line group from input
				bool Eol() {
					SkipWS();
					return Eol_();
				};
				/// Reads (and potentially captures) a char from input if it matches the parameter
				bool Char(const char t_c) {
					SkipWS();
					return Char_(t_c);
				};
				/// Reads (and potentially captures) a string from input if it matches the parameter
				bool Keyword(const GL::string& t_s) {
					SkipWS();
					const auto start = m_position;
					bool retval = Keyword_(t_s);
					// ignore substring matches
					if (retval && m_position.has_more() && char_in_alphabet(*m_position, Engine::keyword_alphabet)) {
						m_position = start;
						retval = false;
					}
					return retval;
				};
				/// Reads (and potentially captures) a symbol group from input if it matches the parameter
				bool Symbol(GL::string t_s, const bool t_disallow_prevention = false) {
					SkipWS();
					const auto start = m_position;
					bool retval = Symbol_(t_s);

					// ignore substring matches
					if (retval && m_position.has_more() && (t_disallow_prevention == false) && char_in_alphabet(*m_position, Engine::symbol_alphabet)) {
						m_position = start;
						retval = false;
					}
					return retval;
				}

				PreprocessorTokenPtr parse(GL::string& t_input) {
					m_position = Engine::Position(0, t_input);

					// top level stack        
					if (Statements()) {
						if (m_position.has_more()) {
							throw exception::eval_error("Unparsed input", m_position);
						}
						else {
							build_match<CompletedPreprocessor>(0, t_input);
						}
					}
					else {
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<NonePreprocessor>("", Engine::Parse_Location(m_position, m_position), std::vector<PreprocessorTokenPtr>{})));
					}

					PreprocessorTokenPtr retval = m_match_stack.front();
					m_match_stack.clear();

					return retval;
				};

				bool SkipToEndOfLine(bool InPreprocessorMacro = true) {
					SkipWS();
					bool retval = false;
					while (m_position.has_more()) {
						SkipWS();
						if (InPreprocessorMacro && (Keyword_("\\\n") || Keyword_("\\\r"))) {

						}
						else {
							if (Eol()) {
								retval = true;
								break;
							}
							else {
								++m_position;
							}
						}
					}
					return retval;
				}
				bool SearchFor(std::vector<GL::string> const& options, GL::string& foundOption) {
					SkipWS();
					while (m_position.has_more()) {
						for (auto const& option : options) {
							if (Keyword(option)) {
								foundOption = option;
								return true;
							}
						}
						++m_position;
					}
					return false;
				}

				/// Top level parser, starts parsing of all known parses
				bool Statements() {
					bool retval = false;
					bool has_more = true;
					bool saw_eol = true;

					while (has_more) {
						SkipWS(true);
						if (Warning() || Error() || Pragma() || Include() || If() || Define() || Undefine() || None()) {
							has_more = true;
							retval = true;
						}
						else {
							has_more = false;
						}
					}
					return retval;
				};

				bool Define() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Engine::Position prev_position = this->m_position;
					SkipWS(true);
					if (Keyword("#define")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<DefinePreprocessor>(
							this->m_position.str(prev_position, m_position),
							Engine::Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool Undefine() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Engine::Position prev_position = this->m_position;
					SkipWS(true);
					if (Keyword("#undef")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<UndefinePreprocessor>(
							this->m_position.str(prev_position, m_position),
							Engine::Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool If() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					Engine::Position prev_position = this->m_position;

					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						m_position = prev_position;
						return false;
					};

					bool foundIfOrElseIf = Keyword("#if");
					if (foundIfOrElseIf) {
						auto this_prev_position = this->m_position;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<IfPreprocessor>(
							this->m_position.str(this_prev_position, m_position),
							Engine::Parse_Location{ this_prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					if (!foundIfOrElseIf) {
						foundIfOrElseIf = Keyword("#ifdef");
						if (foundIfOrElseIf) {
							auto this_prev_position = this->m_position;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<IfDefinedPreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Engine::Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
						}
					}
					if (!foundIfOrElseIf) {
						foundIfOrElseIf = Keyword("#ifndef");
						if (foundIfOrElseIf) {
							Engine::Position this_prev_position = this->m_position;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<IfNotDefinedPreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Engine::Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
						}
					}

					while (foundIfOrElseIf) {
						foundIfOrElseIf = false;
						retval = true;

						Statements();

						SkipWS(true);
						if (Keyword("#elif")) {
							auto this_prev_position = this->m_position;
							foundIfOrElseIf = true;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<ElseIfPreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Engine::Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
							continue;
						}
						else if (Keyword("#else")) {
							auto this_prev_position = this->m_position;
							foundIfOrElseIf = true;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<ElsePreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Engine::Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
							continue;
						}
						else if (Keyword("#endif")) {
							auto this_prev_position = this->m_position;
							SkipToEndOfLine();
							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<EndIfPreprocessor>(
								this->m_position.str(this_prev_position, m_position),
								Engine::Parse_Location{ this_prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));
						}
						else {
							return failure();
						}
						build_match<IfChainPreprocessor>(prev_stack_top, "");
					}

					return retval;
				};
				bool Error() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					auto prev_position = this->m_position;
					SkipWS(true);
					if (Keyword("#error")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<ErrorPreprocessor>(
							this->m_position.str(prev_position, m_position),
							Engine::Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool Warning() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					auto prev_position = this->m_position;

					SkipWS(true);
					if (Keyword("#warning")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<WarningPreprocessor>(
							this->m_position.str(prev_position, m_position),
							Engine::Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool Include() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					auto prev_position = this->m_position;

					SkipWS(true);
					if (Keyword("#include")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<IncludePreprocessor>(
							this->m_position.str(prev_position, m_position),
							Engine::Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool Pragma() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					auto prev_position = this->m_position;

					SkipWS(true);
					if (Keyword("#pragma")) {
						prev_position = this->m_position;
						retval = true;
						SkipToEndOfLine();
						m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<PragmaPreprocessor>(
							this->m_position.str(prev_position, m_position),
							Engine::Parse_Location{ prev_position, m_position },
							std::vector<PreprocessorTokenPtr>{}
						)));
					}
					return retval;
				};
				bool None() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					auto prev_position = this->m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						m_position = prev_position;
						return false;
					};

					SkipWS(true);
					if (
						Keyword("#define") ||
						Keyword("#undef") ||
						Keyword("#if") ||
						Keyword("#elif") ||
						Keyword("#else") ||
						Keyword("#endif") ||
						Keyword("#ifdef") ||
						Keyword("#ifndef") ||
						Keyword("#error") ||
						Keyword("#warning") ||
						Keyword("#pragma") ||
						Keyword("#include")
						) {
						return failure();
					}

					while (SkipToEndOfLine(false)) {
						retval = true;
						SkipWS(true);

						const auto this_prev_stack_top = m_match_stack.size();
						auto this_prev_position = this->m_position;
						if (!m_position.has_more() ||
							Keyword("#define") ||
							Keyword("#undef") ||
							Keyword("#if") ||
							Keyword("#elif") ||
							Keyword("#else") ||
							Keyword("#endif") ||
							Keyword("#ifdef") ||
							Keyword("#ifndef") ||
							Keyword("#error") ||
							Keyword("#warning") ||
							Keyword("#pragma") ||
							Keyword("#include")
							) {
							while (m_match_stack.size() != this_prev_stack_top) m_match_stack.pop_back();
							m_position = this_prev_position;

							m_match_stack.push_back(std::dynamic_pointer_cast<PreprocessorToken>(std::make_shared<NonePreprocessor>(
								this->m_position.str(prev_position, m_position),
								Engine::Parse_Location{ prev_position, m_position },
								std::vector<PreprocessorTokenPtr>{}
							)));

							return retval;
						}
					}

					return retval;
				};
			};
			
			// The interpreter should take in source code and develop an AST node diagram.
			class Interpreter {
			public:
				struct AST_Node_Impl;
				using AST_Node_Impl_Ptr = typename std::shared_ptr<AST_Node_Impl>;
				using AST_NodePtr = typename std::shared_ptr<AST_Node>;

				class detail {
				public:
					/// Special type for returned values
					struct Return_Value {
						GL::any::fast_any retval;
					};

					/// Special type indicating a call to 'break'
					struct Break_Loop {};

					/// Special type indicating a call to 'continue'
					struct Continue_Loop {};

					template<typename T>
					static bool GetTextImpl(T const& r, GL::string& out) {
						if (!r->text.empty()) {
							out = r->text;
							return true;
						}
						else {
							for (auto& child : r->children) {
								if (GetTextImpl(child, out)) {
									return true;
								}
							}
						}
						return false;
					};
					template<typename T>
					static bool GetClassTypeImpl(T const& r, GL::type& out) {
						if (r->identifier == Engine::AST_Node_Type::Id) {
							if (auto ptr = std::dynamic_pointer_cast<AST_Nodes::Id_AST_Node>(r)) {
								if (ptr->type == AST_Nodes::IdType::Class) {
									if (auto ptr2 = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(ptr)) {
										out = ptr2->TypeInfo;
										return true;
									}
								}
							}
						}

						for (auto& child : r->children) {
							if (GetClassTypeImpl(child, out)) {
								return true;
							}
						}

						return false;
					};

				};

				static GL::type GetClassType(AST_Node_Impl_Ptr const& r, GL::scope::impl::BasicScope* currentScope) {
					GL::type out = GL::type_of<void>();
					if (r) {
						if (!detail::GetClassTypeImpl(r, out)) {
							auto sv = GetText(r);
							return currentScope->DetermineType(sv);
						}
					}
					return out;
				}

				template<typename T> static GL::string GetText(T const& r) {
					GL::string out;
					(void)detail::GetTextImpl(r, out);
					return out;
				};
				static GL::any::fast_any const_var(GL::any const& rhs) {
					return rhs.fast() | GL::type::Const;
				};

				struct AST_Node_Impl : public AST_Node {
					AST_Node_Impl(GL::string t_ast_node_text,
						Engine::AST_Node_Type t_id,
						Engine::Parse_Location t_loc,
						std::vector<AST_Node_Impl_Ptr> t_children = std::vector<AST_Node_Impl_Ptr>())
						: AST_Node(std::move(t_ast_node_text), t_id, std::move(t_loc))
						, children(std::move(t_children))
						, time_spent_during_eval{ 0 }
						, num_evals{ 0 }
					{};

					std::vector<std::reference_wrapper<AST_Node>> get_children() const override final {
						std::vector<std::reference_wrapper<AST_Node>> retval;
						retval.reserve(children.size());
						for (const AST_Node_Impl_Ptr& child : children) {
							retval.emplace_back(*child);
						}
						return retval;
					};

					GL::any::fast_any eval(GL::scope::impl::BasicScope* currentScope) const override final {
						thread_local GL::stopwatch sw;

						InterlockedIncrementAcquire64(&num_evals);
						defer(
							const_cast<GL::second&>(time_spent_during_eval) += GL::second((float)sw.stop());
						);

						try {
							return eval_internal(currentScope);
						}
						catch (exception::eval_error& ee) {							
							throw ee;
						}
						catch (std::runtime_error& ee) {
							auto e = exception::eval_error(std::string(ee.what()), this->location.start, "Compiled C++ Function");
							throw e;
						}
						catch (std::exception& ee) {
							auto e = exception::eval_error(std::string(ee.what()), this->location.start, "Compiled C++ Function");
							throw e;
						}
					}
					void compile(GL::scope::impl::BasicScope* currentScope) const override final {
						try {
							(void)compile_internal(currentScope);
						}
						catch (exception::eval_error& ee) {
							// ee.call_stack.push_back(*this);
							throw ee;
						}
						catch (std::runtime_error& ee) {
							auto e = exception::eval_error(std::string(ee.what()), this->location.start, "Compiled C++ Function");
							throw e;
						}
						catch (std::exception& ee) {
							auto e = exception::eval_error(std::string(ee.what()), this->location.start, "Compiled C++ Function");
							throw e;
						}
					};
					GL::type return_type() const override final {
						try {
							return return_type_internal();
						}
						catch (exception::eval_error& ee) {
							throw ee;
						}
						catch (std::runtime_error& ee) {
							auto e = exception::eval_error(std::string(ee.what()), this->location.start, "Compiled C++ Function");
							throw e;
						}
						catch (std::exception& ee) {
							auto e = exception::eval_error(std::string(ee.what()), this->location.start, "Compiled C++ Function");
							throw e;
						}
					};

					GL::second TimeSpent_Total() const override {
						GL::second out = time_spent_during_eval;
						for (auto& child : children) {
							out += child->TimeSpent_Total();
						}
						return out;
					};
					GL::second TimeSpent_Self() const override {
						return time_spent_during_eval / (float)std::max<long long>(1ll, num_evals);
					};

					GL::second time_spent_during_eval;
					mutable __int64 num_evals;
					std::vector<AST_Node_Impl_Ptr> children;

				protected:
					virtual GL::any::fast_any eval_internal(GL::scope::impl::BasicScope*) const {
						return {};
						// throw std::runtime_error("Undispatched ast_node (internal error)");
					};
					virtual void compile_internal(GL::scope::impl::BasicScope* currentScope) const {
						for (auto& child : children) child->compile(currentScope);
					};
					virtual GL::type return_type_internal() const {
						return GL::type_of<void>();
					};
				};
				class AST_Nodes {
				public:
					// wrapper for an entire script
					struct File_AST_Node final : AST_Node_Impl {
						File_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::File, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							try {
								const auto num_children = this->children.size();
								try {
									if (num_children > 0) {
										for (size_t i = 0; i < num_children - 1; ++i) {
											this->children[i]->eval(currentScope);
										}
										return this->children.back()->eval(currentScope);
									}
									else {
										return {};
									}
								}
								catch (detail::Return_Value& rv) {
									return rv.retval;
								}
							}
							catch (const detail::Continue_Loop&) {
								throw exception::eval_error("Unexpected `continue` statement outside of a loop", this->location.start);
							}
							catch (const detail::Break_Loop&) {
								throw exception::eval_error("Unexpected `break` statement outside of a loop", this->location.start);
							}
						}
					};
					// empty lines, comments, etc.
					struct Noop_AST_Node final : AST_Node_Impl {
						Noop_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc)
							: AST_Node_Impl(t_ast_node_text, Engine::AST_Node_Type::Noop, t_loc)
						{};

						Noop_AST_Node()
							: AST_Node_Impl("", Engine::AST_Node_Type::Noop, Engine::Parse_Location{ Engine::Position{}, Engine::Position{} })
						{};
						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							// It's a no-op, that evaluates to "void"
							return {};
						}
					};
					// return ARG
					struct Return_AST_Node final : AST_Node_Impl {
						Return_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Return, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							if (this->children.size() == 0) {
								throw detail::Return_Value{ GL::any::fast_any() };
							}
							else if (this->children.size() == 1) {
								GL::any::fast_any out{ this->children[0]->eval(currentScope) };
								if (out.empty()) throw exception::eval_error("Cannot return void from a return statement.", this->location.start);
								else throw detail::Return_Value{ out };
							}
							else {
								auto vec = currentScope->call("::vector<::var>");
								for (const auto& child : this->children) currentScope->call("push_back", { vec, child->eval(currentScope) });								
								throw detail::Return_Value{ vec };
							}
						}
					};
					// built-in constants that could be understood by the compiler, such as integers, floating-point values, strings, vectors, etc.
					struct Constant_AST_Node final : public AST_Node_Impl {
						Constant_AST_Node(GL::string t_ast_node_text, Engine::Parse_Location t_loc, GL::any const& t_value)
							: AST_Node_Impl(t_ast_node_text, Engine::AST_Node_Type::Constant, std::move(t_loc))
							, m_value(t_value.fast() | GL::type::Const)
						{}

						explicit Constant_AST_Node(GL::any::fast_any t_value)
							: AST_Node_Impl("", Engine::AST_Node_Type::Constant, Engine::Parse_Location{ Engine::Position{}, Engine::Position{} })
							, m_value(std::move(t_value) | GL::type::Const)
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope*) const override {
							return m_value;
						}

						GL::any::fast_any m_value;
					};

					// intended for basic operations like "+"
					struct Binary_Operator_AST_Node : AST_Node_Impl {
						Binary_Operator_AST_Node(GL::scope::impl::BasicScope* currentScope, const GL::string& t_oper, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(t_oper, Engine::AST_Node_Type::Binary, std::move(t_loc), std::move(t_children))
							, m_oper(Engine::Operators::to_operator(t_oper.c_str()))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return currentScope->call(this->text, { this->children[0]->eval(currentScope), this->children[1]->eval(currentScope) });
						};

					private:
						Engine::Operators::Opers m_oper;

					};

					enum class IdType {
						Id,
						Function,
						Variable,
						Class
					};
					// keyname node, could be a function name, could be a variable name, etc.
					struct Id_AST_Node : AST_Node_Impl {
						Id_AST_Node(GL::scope::impl::BasicScope* currentScope, const GL::string& t_ast_node_text, Engine::Parse_Location t_loc)
							: AST_Node_Impl(t_ast_node_text, Engine::AST_Node_Type::Id, std::move(t_loc))
						{}

					public:
						IdType type = IdType::Id;
					};
					// 
					struct FunctionName_AST_Node final : Id_AST_Node {
						FunctionName_AST_Node(GL::scope::impl::BasicScope* currentScope, const GL::string& t_ast_node_text, Engine::Parse_Location t_loc)
							: Id_AST_Node(currentScope, t_ast_node_text, std::move(t_loc))
						{
							type = IdType::Function;
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							if (auto obj = currentScope->find_object(this->text)) {
								return obj;
							}
							throw exception::eval_error("Can not find object: " + this->text, this->location.start);
						}
					};
					// 
					struct VariableName_AST_Node final : Id_AST_Node {
						VariableName_AST_Node(GL::scope::impl::BasicScope* currentScope, const GL::string& t_ast_node_text, Engine::Parse_Location t_loc)
							: Id_AST_Node(currentScope, t_ast_node_text, std::move(t_loc))
						{
							type = IdType::Variable;
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							if (auto obj = currentScope->find_object(this->text)) {
								return obj;
							}
							throw exception::eval_error("Can not find object: " + this->text, this->location.start);
						}
					};
					// 
					struct ClassName_AST_Node final : Id_AST_Node {
						ClassName_AST_Node(GL::scope::impl::BasicScope* currentScope, const GL::string& t_ast_node_text, Engine::Parse_Location t_loc)
							: Id_AST_Node(currentScope, t_ast_node_text, std::move(t_loc))
						{
							type = IdType::Class;
							TypeInfo = currentScope->DetermineType(t_ast_node_text);
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							if (auto obj = currentScope->find_object(this->text)) {
								return obj;
							}
							throw exception::eval_error("Can not find object: " + this->text, this->location.start);
						}

					public:
						GL::type TypeInfo;
					};
					//
					struct Arg_AST_Node final : AST_Node_Impl {
						Arg_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Arg, std::move(t_loc), std::move(t_children))
						{};

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return this->children.back()->eval(currentScope);
						}
					};
					//
					struct Arg_List_AST_Node final : AST_Node_Impl {
						Arg_List_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Arg_List, std::move(t_loc), std::move(t_children))
						{};

						static GL::string get_arg_name(const AST_Node_Impl& t_node) {
							if (t_node.children.empty()) {
								return t_node.text;
							}
							else if (t_node.children.size() == 1) {
								return t_node.children[0]->text;
							}
							else {
								return t_node.children[1]->text;
							}
						}
						static std::vector<GL::string> get_arg_names(const AST_Node_Impl& t_node) {
							std::vector<GL::string> retval;

							for (const auto& node : t_node.children) {
								retval.push_back(get_arg_name(*node));
							}

							return retval;
						}
						static GL::type get_arg_type(const AST_Node_Impl& t_node) {
							if (t_node.children.empty()) {
								return GL::type_of<GL::any::fast_any>();
							}
							else if (t_node.children.size() == 1) {
								return GL::type_of<GL::any::fast_any>();
							}
							else if (t_node.children[0]->identifier == Engine::AST_Node_Type::Id) {
								if (auto ptr = std::dynamic_pointer_cast<AST_Nodes::Id_AST_Node>(t_node.children[0])) {
									if (ptr->type == AST_Nodes::IdType::Class) {
										if (auto ptr2 = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(ptr)) {
											return ptr2->TypeInfo;
										}
									}
								}
								if (GetText(t_node.children[0]) == "void") {
									return GL::type_of<void>();
								}
								return GL::type_of<GL::any::fast_any>();
							}
							else {
								return GL::type_of<GL::any::fast_any>();
							}
						}
						static std::vector<GL::type> get_arg_types(const AST_Node_Impl& t_node) {
							std::vector<GL::type> retval;
							for (const auto& node : t_node.children) {
								retval.push_back(get_arg_type(*node));
							}
							return retval;
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							// evaluate all of the children, return the result of the last child
							size_t numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(currentScope);
								}
								return this->children.back()->eval(currentScope);
							}
							else {
								return {};
							}
						}
					};
					// ID(ARG_LIST)
					struct Fun_Call_AST_Node : AST_Node_Impl {
						Fun_Call_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Fun_Call, std::move(t_loc), std::move(t_children))
						{
							function_name = GetText(this->children[0]);
						};

						GL::any::fast_any do_eval_internal(GL::scope::impl::BasicScope* currentScope) const {
							std::vector<GL::any::fast_any> params;
							params.reserve(this->children[1]->children.size());
							for (const AST_Node_Impl_Ptr& child : this->children[1]->children) {
								params.push_back(child->eval(currentScope));
							}
							return currentScope->call(function_name, params);
						};
						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return do_eval_internal(currentScope);
						};
						GL::string function_name;
					};
					// ID(ARG_LIST), but we know for a fact that the return value will go unused. 
					struct Unused_Return_Fun_Call_AST_Node final : Fun_Call_AST_Node {
						Unused_Return_Fun_Call_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: Fun_Call_AST_Node(currentScope, std::move(t_ast_node_text), std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return this->do_eval_internal(currentScope);
						};
					};
					// 
					struct Equation_AST_Node final : AST_Node_Impl {
						Equation_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Equation, std::move(t_loc), std::move(t_children))
							, m_oper(Engine::Operators::to_operator(text.c_str()))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							if (m_oper == Engine::Operators::Opers::assign_if_null || this->text == "?=") {
								auto lhs = this->children[0]->eval(currentScope);
								if (lhs.m_casted_type.is_void()) {
									return currentScope->call(":=", { lhs, this->children[1]->eval(currentScope) });
								}
								else {
									return lhs;
								}
							}
							else {
								auto lhs = this->children[0]->eval(currentScope);
								auto rhs = this->children[1]->eval(currentScope);

								if (m_oper == Engine::Operators::Opers::assign) {
									return currentScope->call("=", { lhs, rhs });
								}
								else if (this->text == ":=") {
									return currentScope->call(":=", { lhs, rhs });
								}
								else {
									return currentScope->call(this->text, { lhs, rhs });
								}
							}
						}

					private:
						Engine::Operators::Opers m_oper;

					};
					// &&
					struct Logical_And_AST_Node final : AST_Node_Impl {
						Logical_And_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Logical_And, std::move(t_loc), std::move(t_children)) {
							EXPECT_EQ(this->children.size(), 2);
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return GL::any::fast_any::instance(currentScope->cast<bool>(this->children[0]->eval(currentScope)) && currentScope->cast<bool>(this->children[1]->eval(currentScope)));
						}
					};
					// ||
					struct Logical_Or_AST_Node final : AST_Node_Impl {
						Logical_Or_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Logical_Or, std::move(t_loc), std::move(t_children)) {
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return GL::any::fast_any::instance(currentScope->cast<bool>(this->children[0]->eval(currentScope)) || currentScope->cast<bool>(this->children[1]->eval(currentScope)));
						}
					};

					/* var x; */
					struct Var_Decl_AST_Node final : AST_Node_Impl {
						Var_Decl_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Var_Decl, std::move(t_loc), std::move(t_children))
						{}

						/*! Empty variable assignment:
						  var j;
						*/
						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							const auto& idname = this->children[0]->text;
							currentScope->insert_object_here(idname, GL::var());
							if (auto* p = currentScope->find_object_here(idname)) return p->fast();
							return {};
						}
					};
					/* double x; */
					struct Assign_Retroactively_AST_Node final : AST_Node_Impl {
						Assign_Retroactively_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Assign_Retroactively, std::move(t_loc), std::move(t_children))
						{
							ASSERT(this->children.size() >= 1);
							idname = GL::string(GetText(this->children[1]));// ->text; // e.g. x, y, z
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							GL::any::fast_any defaultVal;
							if (this->children.size() == 3) {
								defaultVal = this->children[2]->eval(currentScope);
							}

							if (currentScope->is_class()) {
								if (auto* ClassPtr = dynamic_cast<scope::impl::ClassScope*>(currentScope)) {
									ClassPtr->add_member_object(idname, currentScope->DetermineType(this->children[0]->text), defaultVal);
									return ClassPtr->find_object_here(idname)->fast();
								}
							}
							if (currentScope->is_namespace()) {
								if (auto* ClassPtr = dynamic_cast<scope::impl::NamespaceScope*>(currentScope)) {
									if (auto* BC = currentScope->GetRoot()->try_find_class(currentScope->DetermineType(this->children[0]->text)); BC && BC->this_m.is_class()) {
										auto& Class = *dynamic_cast<GL::scope::impl::ClassScope*>(BC->this_m.scope);
										if (!defaultVal.empty()) {
											ClassPtr->insert_object_here(idname, Class.cast(defaultVal, Class.this_type));
										}
										else {
											ClassPtr->insert_object_here(idname, Class.call(Class.this_type.name()));
										}
									}
									else {
										if (!defaultVal.empty()) {
											ClassPtr->insert_object_here(idname, currentScope->cast(defaultVal, currentScope->DetermineType(this->children[0]->text)));
										}
										else {
											ClassPtr->insert_object_here(idname, currentScope->call(currentScope->DetermineType(this->children[0]->text).name()));
										}
									}
									return ClassPtr->find_object_here(idname)->fast();
								}
							}
							return currentScope->find_object(idname);
						};

						// GL::string type_name;
						GL::string idname;
					};
					/* var x = double(); */
					struct Assign_Decl_AST_Node final : AST_Node_Impl {
						Assign_Decl_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Assign_Decl, std::move(t_loc), std::move(t_children))
						{};

						/*! Non-Empty variable assignment:
						  var j = 100;
						*/
						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							const auto& idname = this->children[0]->text;
							GL::any::fast_any value = this->children[1]->eval(currentScope);
							currentScope->insert_object_here(idname, value);
							return value;
						}
					};
					// ++x
					struct Prefix_AST_Node final : AST_Node_Impl {
						Prefix_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Prefix, std::move(t_loc), std::move(t_children))
							, m_oper(Engine::Operators::to_operator(this->text.c_str(), true))
						{}

						// ++x;
						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return currentScope->call(this->text, { this->children[0]->eval(currentScope) }); // we currently do not attempt to validate -- just process the request and see what lands. 
						};

					private:
						Engine::Operators::Opers m_oper = Engine::Operators::Opers::invalid;
					};
					// x++
					struct Postfix_AST_Node final : AST_Node_Impl {
						Postfix_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Postfix, std::move(t_loc), std::move(t_children))
							, m_oper(Engine::Operators::to_operator(this->text.c_str(), true)) {
						}

						// x++; 
						// depending on the context, is either specifying the type (e.g. _ft, ull) or is modifying the underlying value (++, --)
						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							auto var(this->children[0]->eval(currentScope)); // an int, float, etc.                 

							// the type is known. Short-circuit and get out fast. 
							if (m_oper == Engine::Operators::Opers::pre_increment) {
								auto copied = currentScope->cast(var, var.m_casted_type - GL::type::Reference - GL::type::Const);
								(void)currentScope->call("++", { var });
								return copied;
							}
							else if (m_oper == Engine::Operators::Opers::pre_decrement) {
								auto copied = currentScope->cast(var, var.m_casted_type - GL::type::Reference - GL::type::Const);
								(void)currentScope->call("--", { var });
								return copied;
							}
							else if (m_oper == Engine::Operators::Opers::invalid) {
								if (this->text != "" && this->text.length() >= 1) {
									for (auto& si_unit_type : GL::value::all_known_unit_types()) {
										for (auto& unit_type : si_unit_type.second.implimented_units) {
											auto& abbreviation = unit_type.second.abbreviation;
											if (this->text == abbreviation) {
												auto out = GL::any::fast_any::instance(GL::value(unit_type.second));
												(void)currentScope->call("=", { out, var });
												return out;
											}
										}
									}
								}
								return var;
							}
							else {
								throw exception::eval_error("Only increment (i++) or decrement (i--) operators are supported in a postfix context, as well as custom postfixes.", this->location.start);
							}
						};

						Engine::Operators::Opers m_oper = Engine::Operators::Opers::invalid;
					};
					// if (Scopeless_Block_AST_Node) Block_AST_Node else Block_AST_Node	
					struct If_AST_Node final : AST_Node_Impl {
						If_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::If, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							// create a temporary scope for temporary variables made in the CONDITION
							if (1) {
								auto newScope = currentScope->make_scope();
								// evaluate the CONDITION statement within this new scope -- note that the new scope only applies if true! 
								if (newScope.cast<bool>(this->children[0]->eval(&newScope))) {
									return this->children[1]->eval(&newScope);
								}
							}

							// if an else-statement is available...
							if (this->children.size() >= 3) {
								auto newScope = currentScope->make_scope();
								return this->children[2]->eval(&newScope);
							}
							else return {}; // returns void
						}
					};
					// while (Scopeless_Block_AST_Node) Block_AST_Node
					struct While_AST_Node final : AST_Node_Impl {
						While_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::While, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							while (currentScope->cast<bool>(this->children[0]->eval(currentScope))) {
								auto newScope = currentScope->make_scope();

								try {
									(void)this->children[1]->eval(&newScope);
								}
								catch (detail::Break_Loop&) {
									break;
								}
								catch (detail::Continue_Loop&) {}
							}
							return {};
						}
					};
					// for (INIT_BLOCK; CONDITION_BLOCK; PROGRESS_BLOCK) WORK_BLOCK
					struct For_AST_Node final : AST_Node_Impl {
						For_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::For, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							auto forScope = currentScope->make_scope();
							{
								// INIT_BLOCK
								if (this->children[0]->identifier != Engine::AST_Node_Type::Noop) {
									this->children[0]->eval(&forScope); // may include declaring a variable, or nothing at all
								}

								// CONDITION_BLOCK
								while (this->children[1]->identifier == Engine::AST_Node_Type::Noop
									||
									forScope.cast<bool>(this->children[1]->eval(&forScope))
									) {
									auto newScope = forScope.make_scope();

									try {
										(void)this->children[3]->eval(&newScope);
									}
									catch (detail::Continue_Loop&) {}
									catch (detail::Break_Loop&) { break; }

									// PROGRESS_BLOCK
									this->children[2]->eval(&forScope);
								}
							}
							return {};
						}
					};
					// for (range_declaration : range_expression) loop_statement
					struct Ranged_For_AST_Node final : AST_Node_Impl {
						Ranged_For_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Ranged_For, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							GL::any::fast_any out;
							auto forScope = currentScope->make_scope();

							GL::any::fast_any item_decl = this->children[0]->eval(&forScope); // var x;
							GL::any::fast_any range = this->children[1]->eval(&forScope); // 0..100 or [0,1,2,3] or vectorObjName etc;
							try {
								// user-defined functions for begin() and end() were found -- this is the ideal.
								for (
									auto begin = forScope.call("begin", { range }),
									end = forScope.call("end", { range });
									forScope.cast<bool>(forScope.call("!=", { begin, end }));
									forScope.call("++", { begin })
									) {
									forScope.call(":=", { item_decl, forScope.call("get", { begin }) });
									try {
										auto innerScope = forScope.make_scope();
										out = this->children[2]->eval(&innerScope);
									}
									catch (detail::Continue_Loop&) {}
								}								
							}
							catch (detail::Break_Loop&) {}
							return out;
						};
					};
					// x[1]
					struct Array_Call_AST_Node final : AST_Node_Impl {
						Array_Call_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Array_Call, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return currentScope->call("[]", {
								this->children[0]->eval(currentScope),
								this->children[1]->eval(currentScope)
							});
						}

					private:
						mutable std::atomic_uint_fast32_t m_loc = { 0 };
					};
					// x.first
					struct Dot_Access_AST_Node final : AST_Node_Impl {
						Dot_Access_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Dot_Access, std::move(t_loc), std::move(t_children))
							, m_fun_name(((this->children[1]->identifier == Engine::AST_Node_Type::Fun_Call) || (this->children[1]->identifier == Engine::AST_Node_Type::Array_Call))
								? this->children[1]->children[0]->text
								: this->children[1]->text)
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							std::vector<GL::any::fast_any> params{ this->children[0]->eval(currentScope) };
							GL::string functionName;

							// what happens next depends on the RHS
							switch (this->children[1]->identifier) {
							case Engine::AST_Node_Type::Fun_Call:
								functionName = this->children[1]->children[0]->text;
								for (auto& child : this->children[1]->children[1]->children)
									params.push_back(child->eval(currentScope));
								break;
							default: // case AST_Node_Type::Id:
								functionName = this->children[1]->text;
								break;
							}
							return currentScope->call(functionName, std::move(params));
						}
						const GL::string m_fun_name;
					};
					// [x](FF) async -> int { return x+FF; };
					struct Lambda_AST_Node final : AST_Node_Impl {
						Lambda_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(t_ast_node_text,
								Engine::AST_Node_Type::Lambda,
								std::move(t_loc),
								std::vector<AST_Node_Impl_Ptr>(t_children))
							, m_param_names(Arg_List_AST_Node::get_arg_names(*this->children[1]))
							, m_param_types(Arg_List_AST_Node::get_arg_types(*this->children[1]))
							//, m_this_capture(has_this_capture(this->children[0]->children))
							, m_lambda_node(t_children.back())
						{
							const_cast<std::shared_ptr<AST_Node_Impl>&>(m_lambda_node) = this->children.back() = optimizer::optimize(this->children.back(), currentScope);

							// need to immediately optimize the lambda node if at all possible, and reduce the likelihood of throwing (which significantly impacts performance). 
							while (m_lambda_node->identifier == Engine::AST_Node_Type::Return) {
								if (m_lambda_node->children.size() == 0) {
									const_cast<std::shared_ptr<AST_Node_Impl>&>(m_lambda_node) = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<Noop_AST_Node>());
								}
								else if (m_lambda_node->children.size() == 1) {
									const_cast<std::shared_ptr<AST_Node_Impl>&>(m_lambda_node) = std::move(m_lambda_node->children[0]);
								}
								else {
									break;
								}
							}
							this->children.back() = m_lambda_node;
						};

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							// children[0] -> list of Id's or variables to be captured. 
							// children[1] -> list of (possibly type'd) variables that define the inputs to the function.
							// children[2] -> either Noop or Id whose name is the desired return type
							// children[3] -> m_lambda_node -> function to call

							std::map<GL::string, std::shared_ptr<Any>>
								captures;

							for (auto& var_name : Arg_List_AST_Node::get_arg_names(*this->children[0])) {
								if (auto obj = currentScope->FindObject(var_name)) {
									captures[var_name] = obj;
								}
								else {
									throw exception::eval_error("Cannot find captured variable", this->location.start);
								}
							}


							GL::type returnType = GetClassType(this->children[2], currentScope);
							//if (returnType.expired()) {
							//	returnType = user_type_shared<void>();
							//}
							bool returnVoid = false; // (GetHash(returnType) == GetHash(user_type_shared<void>()));

							if (is_async) {
								if (returnVoid) {
									return GoodLang::make_callable([
										lambda_node = m_lambda_node,
											param_names = this->m_param_names,
											captures,
											paramTypes = this->m_param_types
									](
										std::shared_ptr<Scopes::BasicScope> currentScope,
										std::shared_ptr<std::vector<Any>> params
										)->Any {
											auto function_scope = currentScope->make_scope();

											// insert the captures
											for (auto& capture : captures) {
												function_scope->EmplaceObject(capture.first, capture.second, false);
											}

											// insert the function params
											for (int i = 0; (i < param_names.size()) && (i < params->size()); ++i) {
												function_scope->EmplaceObject(param_names[i], std::make_shared<Any>(currentScope->Cast(params->operator[](i), paramTypes[i], true)), false);
												// function_scope->EmplaceObject(param_names[i], std::make_shared<Any>(params->operator[](i)), false);
											}

											return parallel::async([
												function_scope,
													lambda_node
											]() -> Any {
													try {
														lambda_node->eval(function_scope);
													}
													catch (detail::Return_Value& rv) {
														// if the retval is anything but void, we should throw an error
														if (!rv.retval.IsEmpty()) {
															throw exception::eval_error("Cannot return with a value inside of a lambda that expects to return void.");
														}
													}
													return Any();
												}).as_promise();
										}, ParamTypes({ user_type_shared<Scopes::BasicScope>(), user_type_shared<std::vector<Any>>() }), GoodLang::user_type_shared<GoodLang::parallel::promise>()
											);
								}
								else {
									return GoodLang::make_callable([
										lambda_node = m_lambda_node,
											param_names = this->m_param_names,
											captures,
											thisReturnType = returnType,
											paramTypes = this->m_param_types
									](
										std::shared_ptr<Scopes::BasicScope> currentScope,
										std::shared_ptr<std::vector<Any>> params
										)->Any {
											auto function_scope = currentScope->make_scope();

											// insert the captures
											for (auto& capture : captures) {
												function_scope->EmplaceObject(capture.first, capture.second, false);
											}

											// insert the function params
											for (int i = 0; (i < param_names.size()) && (i < params->size()); ++i) {
												function_scope->EmplaceObject(param_names[i], std::make_shared<Any>(currentScope->Cast(params->operator[](i), paramTypes[i], true)), false);
												// function_scope->EmplaceObject(param_names[i], std::make_shared<Any>(params->operator[](i)), false);
											}

											return parallel::async([
												currentScope_t = currentScope,
													function_scope_t = function_scope,
													lambda_node_t = lambda_node,
													thisReturnType_t = thisReturnType
											]() -> Any {
													Any lambda_result;
													try {
														lambda_result = lambda_node_t->eval(function_scope_t);
													}
													catch (detail::Return_Value& rv) {
														lambda_result = rv.retval;
													}

													if (thisReturnType_t.expired()) {
														return lambda_result;
													}
													else {
														return function_scope_t->Cast(lambda_result, thisReturnType_t);
													}
												}).as_promise();
										}, ParamTypes({ user_type_shared<Scopes::BasicScope>(), user_type_shared<std::vector<Any>>() }), GoodLang::user_type_shared<GoodLang::parallel::promise>()
											);
								}
							}
							else {
								if (returnVoid) {
									return GoodLang::make_callable([
										lambda_node = m_lambda_node,
											param_names = this->m_param_names,
											captures,
											paramTypes = this->m_param_types
									](
										std::shared_ptr<Scopes::BasicScope> currentScope,
										std::shared_ptr<std::vector<Any>> params
										)->Any {
											auto function_scope = currentScope->make_scope();

											// insert the captures
											for (auto& capture : captures) {
												function_scope->EmplaceObject(capture.first, capture.second, false);
											}

											// insert the function params
											for (int i = 0; (i < param_names.size()) && (i < params->size()); ++i) {
												function_scope->EmplaceObject(param_names[i], std::make_shared<Any>(currentScope->Cast(params->operator[](i), paramTypes[i], true)), false);
												// function_scope->EmplaceObject(param_names[i], std::make_shared<Any>(params->operator[](i)), false);
											}

											try {
												lambda_node->eval(function_scope);
											}
											catch (detail::Return_Value& rv) {
												// if the retval is anything but void, we should throw an error
												if (!rv.retval.IsEmpty()) {
													throw exception::eval_error("Cannot return with a value inside of a lambda that expects to return void.");
												}
											}
											return Any();
										}, ParamTypes({ user_type_shared<Scopes::BasicScope>(), user_type_shared<std::vector<Any>>() })
											);
								}
								else {
									return GoodLang::make_callable([
										lambda_node = m_lambda_node,
											param_names = this->m_param_names,
											captures,
											thisReturnType = returnType,
											paramTypes = this->m_param_types
									](
										std::shared_ptr<Scopes::BasicScope> currentScope,
										std::shared_ptr<std::vector<Any>> params
										)->Any {
											auto function_scope = currentScope->make_scope();

											// insert the captures
											for (auto& capture : captures) {
												function_scope->EmplaceObject(capture.first, capture.second, false);
											}

											// insert the function params
											for (int i = 0; (i < param_names.size()) && (i < params->size()); ++i) {
												function_scope->EmplaceObject(param_names[i], std::make_shared<Any>(currentScope->Cast(params->operator[](i), paramTypes[i], true)), false);
												// function_scope->EmplaceObject(param_names[i], std::make_shared<Any>(params->operator[](i)), false);
											}

											Any lambda_result;
											try {
												lambda_result = lambda_node->eval(function_scope);
											}
											catch (detail::Return_Value& rv) {
												lambda_result = rv.retval;
											}

											if (thisReturnType.expired()) {
												return lambda_result;
											}
											else {
												return function_scope->Cast(lambda_result, thisReturnType);
											}
										}, ParamTypes({ user_type_shared<Scopes::BasicScope>(), user_type_shared<std::vector<Any>>()/*, user_type_shared<AST_Node_Impl>()*/ }), returnType.expired() ? user_type_shared<Any>() : returnType
											);
								}
							}

#else
                            return {};
#endif
						}

					public:
						bool is_async = false;

					private:
						const std::vector<GL::type> m_param_types;
						const std::vector<GL::string> m_param_names;
						const std::shared_ptr<AST_Node_Impl> m_lambda_node;
					};
					// [0, 1, 2, 3]
					struct Inline_Array_AST_Node final : AST_Node_Impl {
						Inline_Array_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Inline_Array, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							// assumes the first child is an ArgList or Arg
							Vector<Var> vec;
							if (!this->children.empty()) {
								vec.reserve(this->children[0]->children.size());
								for (auto& child : this->children[0]->children) {
									vec.push_back(Var(child->eval(currentScope)));
								}
							}
							return vec;
#else
							return {};
#endif
						}

					private:
						mutable std::atomic_uint_fast32_t m_loc = { 0 };
					};
					struct Map_Pair_AST_Node final : AST_Node_Impl {
						Map_Pair_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Map_Pair, std::move(t_loc), std::move(t_children))
						{}
					};
					// ["":10, 10:10, Vector():10, 20:Vector()]
					struct Inline_Map_AST_Node final : AST_Node_Impl {
						Inline_Map_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Inline_Map, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							// assumes the first child is an ArgList or Arg
							Any out{ Map<size_t, std::pair<Var, Var>>() };
							if (!this->children.empty()) {
								for (const auto& child : this->children[0]->children) {
									currentScope->Call("emplace", { out, child->children[0]->eval(currentScope), child->children[1]->eval(currentScope) });
								}
							}
							return out;
#else
							return {};
#endif
						};

					};

					// parallel_for (var x = START_VALUE ; END_VALUE) WORK_BLOCK; // this approach means every iteration will see it's own local "x"
					// parallel_for (START_VALUE ; END_VALUE) WORK_BLOCK // this approach means every iteration will NOT see any "x" at all
					struct Parallel_For_AST_Node final : AST_Node_Impl {
						Parallel_For_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Parallel_For, std::move(t_loc), std::move(t_children))
						{
							ASSERT(this->children.size() == 3);
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							auto forScope = currentScope->make_scope();

							if (1) { // parallel_for (var x = START_VALUE; END_VALUE) WORK_BLOCK
								int startPos{ 0 }, endPos{ 0 };
								if (1) {
									auto temp_memory = forScope->make_scope();

									startPos = temp_memory->Cast<int>(this->children[0]->eval(temp_memory));
									endPos = temp_memory->Cast<int>(this->children[1]->eval(temp_memory));
								}
								if (startPos > endPos) {
									int temp = endPos;
									endPos = startPos;
									startPos = temp;
								}
								if (endPos > startPos) {
									impl::context ctx;
									using DateStorageType = GoodLang::Union<Any, std::shared_ptr<Scopes::BasicScope>, std::weak_ptr<details::Proxy_Function_Base>>;
									impl::Dispatch(ctx,
										endPos - startPos /* count of jobs */,
										[&](impl::JobArgs const& _args)-> void {
											DateStorageType& shared_memory
												= *((DateStorageType*)_args.sharedmemory);
											if (_args.groupIndex == 0) {
												// start of a group
												shared_memory.get<1>()->Call(":=", { shared_memory.get<0>(), _args.jobIndex });
											}
											else {
												//if (auto func = shared_memory.get<2>().lock()) 
												//	shared_memory.get<1>()->Call(func, shared_memory.get<0>());
												//else 
												shared_memory.get<1>()->Call("++", { shared_memory.get<0>() });
											}

											// do the work
											try {
												auto newScope = shared_memory.get<1>()->make_scope();
												this->children[2]->eval(newScope);
											}
											catch (detail::Continue_Loop&) {}
										},
										sizeof(DateStorageType) /* size of shared memory */,
											[&](void* p) -> void {
											new (p) DateStorageType{
												Any{},
												forScope->make_scope(),
												Proxy_Function{}
											}; // initialize the shared memory
											DateStorageType& iter = *static_cast<DateStorageType*>(p);
											iter.get<0>() = this->children[0]->eval(iter.get<1>()); // e.g. int x = 0 or var& x = 0
										},
											[&](void* p) -> void {
											((DateStorageType*)p)->~DateStorageType(); // destroy the shared memory
										}
										);
									try {
										impl::Wait(ctx);
									}
									catch (detail::Break_Loop&) {};
								}
							}

							return Any();
#else
							return {};
#endif
						}
					};

					// parallel_for (range_declaration : range_expression) loop_statement;
					struct Parallel_Ranged_For_AST_Node final : AST_Node_Impl {
						Parallel_Ranged_For_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Parallel_Ranged_For, std::move(t_loc), std::move(t_children))
						{
							ASSERT(this->children.size() == 3);
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							auto forScope = currentScope->make_scope();

							Any range = this->children[1]->eval(forScope); // 0..100 or [0,1,2,3] or vectorObjName etc;
							try {
								auto begin_func = forScope->FindFunction("begin", ParamTypes({ range }));
								auto end_func = forScope->FindFunction("end", ParamTypes({ range }));
								if (begin_func && end_func) {
									Any begin = forScope->Call(begin_func, { range });
									Any end = forScope->Call(end_func, { range });
									auto& copyConstructorFunctor = begin.Type().lock()->GetCopyConstructor();

									// if we can get the distance quickly, then great
									size_t count = 0;
									if (auto distanceFunction = forScope->FindFunction("-", ParamTypes({ end, begin }))) {
										count = forScope->Cast<size_t>(forScope->Call(distanceFunction, { end, begin }));
									}
									else {
										while (forScope->Cast<bool>(forScope->Call("!=", { begin, end }))) {
											count++;
											forScope->Call("++", { begin });
										}
										begin = forScope->Call(begin_func, { range });
									}

									using shared_type = std::pair< std::pair<Any, Any>, std::shared_ptr<Scopes::BasicScope>>;
									impl::context ctx;
									impl::Dispatch(ctx,
										count,
										[&](impl::JobArgs const& _args)-> void {
											shared_type& iter = *static_cast<shared_type*>(_args.sharedmemory);
											if (_args.groupIndex == 0) {
												// start of a group
												if (auto jumpFunction = iter.second->FindFunction("+=", ParamTypes({ iter.first.first, _args.jobIndex }))) {
													iter.second->Call(jumpFunction, { iter.first.first, _args.jobIndex });
												}
												else {
													for (int i = 0; i < _args.jobIndex; i++) iter.second->Call("++", { iter.first.first });
												}
											}
											else {
												// within a group, we know the jobs are done in sequence, so we can safely increment by 1.
												iter.second->Call("++", { iter.first.first });
											}
											iter.second->Call(":=", { iter.first.second, iter.second->Call("get", {iter.first.first}) });

											// do the work
											try {
												auto innerScope = iter.second->make_scope();
												this->children[2]->eval(innerScope);
											}
											catch (detail::Continue_Loop&) {}
										},
										sizeof(shared_type),
											[&](void* p) -> void {
											new (p) shared_type{ std::pair<Any,Any>{}, forScope->make_scope() };
											shared_type& iter = *static_cast<shared_type*>(p);
											iter.first.first = copyConstructorFunctor(begin); // iterator
											iter.first.second = this->children[0]->eval(iter.second); // var x;
										},
											[](void* p) -> void {
											((shared_type*)p)->~shared_type();
										}
										);
									impl::Wait(ctx);
								}
								else {
									throw exception::eval_error("begin() and/or end() functions were not found for the provided type", this->location.start);
								}
							}
							catch (detail::Break_Loop&) {}

							return Any();
#else
							return {};
#endif
						}
					};

					struct Break_AST_Node final : AST_Node_Impl {
						Break_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Break, std::move(t_loc), std::move(t_children)) {
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override { throw detail::Break_Loop(); }
					};

					struct Continue_AST_Node final : AST_Node_Impl {
						Continue_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Continue, std::move(t_loc), std::move(t_children)) {
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override { throw detail::Continue_Loop(); }
					};

					struct Case_AST_Node final : AST_Node_Impl {
						Case_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Case, std::move(t_loc), std::move(t_children))
						{
#if 0
							ASSERT(this->children.size() == 2);
							// if this is a constant, its hash should also be a constant
							if (this->children[0]->identifier == Engine::AST_Node_Type::Constant) {
								try {
									constexprHash = currentScope->Cast<size_t>(currentScope->Call("to_hash", { std::dynamic_pointer_cast<Constant_AST_Node>(this->children[0])->m_value }));
									// don't need the first child anymore
									const_cast<GL::string&>(this->text) = this->children.front()->text;
									this->children.front() = this->children.back();
									this->children.pop_back();
								}
								catch (...) {}
							}
#endif
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							auto thisScope = currentScope->make_scope();
							return this->children.back()->eval(&thisScope);
						}

						// std::optional<size_t> constexprHash;
					};

					struct Switch_AST_Node final : AST_Node_Impl {
						Switch_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Switch, std::move(t_loc), std::move(t_children)) {
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							auto thisScope = currentScope->make_scope();

							bool breaking = false;
							size_t currentCase = 1;
							bool hasMatched = false;

							size_t match_hash = currentScope->Cast<size_t>(currentScope->Call("to_hash", { this->children[0]->eval(thisScope) }));

							Any out;
							while (!breaking && (currentCase < this->children.size())) {
								try {
									if (this->children[currentCase]->identifier == Engine::AST_Node_Type::Case) {
										if (hasMatched) {
											out = this->children[currentCase]->eval(thisScope);
										}
										else {
											std::optional<size_t>& constexprHash = std::dynamic_pointer_cast<Case_AST_Node>(this->children[currentCase])->constexprHash;
											size_t this_hash;
											if (constexprHash.has_value()) {
												// best-case scenario
												this_hash = constexprHash.value();
											}
											else {
												this_hash = currentScope->Cast<size_t>(currentScope->Call("to_hash", { this->children[currentCase]->children[0]->eval(thisScope) }));
											}

											// This is a little odd, but because want to see both the switch and the case simultaneously, I do a downcast here.
											if (hasMatched || (this_hash == match_hash)) {
												out = this->children[currentCase]->eval(thisScope);
												hasMatched = true;
											}
										}
									}
									else if (this->children[currentCase]->identifier == Engine::AST_Node_Type::Default) {
										out = this->children[currentCase]->eval(thisScope);
										// hasMatched = true;
									}
								}
								catch (detail::Break_Loop&) {
									breaking = true;
								}
								++currentCase;
							}
							return out;
#else
							return {};
#endif
						}
					};

					struct Default_AST_Node final : AST_Node_Impl {
						Default_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Default, std::move(t_loc), std::move(t_children)) {
							ASSERT(this->children.size() == 1);
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							auto thisScope = currentScope->make_scope();
							return this->children[0]->eval(&thisScope);
						}
					};

					struct Do_AST_Node final : AST_Node_Impl {
						Do_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Do, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							Any retval;

							auto thisScope = currentScope->make_scope();

							std::exception_ptr err{ nullptr };

							try { retval = this->children[0]->eval(thisScope); }
							catch (...) { err = std::current_exception(); }

							if (this->children.back()->identifier == Engine::AST_Node_Type::Finally)
								this->children.back()->children[0]->eval(thisScope);

							if (err)
								std::rethrow_exception(err);

							return retval;
#else
							return {};
#endif
						}
					};

					struct Try_AST_Node final : AST_Node_Impl {
						Try_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Try, std::move(t_loc), std::move(t_children))
						{}

						GL::any::fast_any handle_exception(GL::scope::impl::BasicScope* currentScope, const GL::any::fast_any& t_except, bool& rethrow) const {
							GL::any::fast_any retval;
							bool wasCaught = false;
							for (int i = 1; i < this->children.size(); i++) {
								auto& catch_block = *this->children[i];
								if (catch_block.identifier == Engine::AST_Node_Type::Finally) {
									continue;
								}
								//else if (catch_block.identifier == AST_Node_Type::Noop) {
								//	// catches anything, but doesn't provide a type or var name. Therefore this will always catch, regardless of t_except's type
								//	retval = catch_block.children[1]->eval(currentScope);
								//}
								else if (catch_block.identifier == Engine::AST_Node_Type::Catch) {
									if (catch_block.children.size() == 1) { // catch{ ... }
										// No variable capture
										retval = catch_block.children[0]->eval(currentScope);
										wasCaught = true;
										break;
									}
									else {
										// variable capture
										if (catch_block.children[0]->identifier == Engine::AST_Node_Type::Arg) {
											if (catch_block.children[0]->children.size() == 1) {
												// catch(e){...}
												auto& varName = catch_block.children[0]->children[0]->text; // e.g. x

												currentScope->insert_object_here(varName, t_except);
												retval = catch_block.children[1]->eval(currentScope);
												wasCaught = true;
												break;
											}
											else {
												// catch(exception& e){...}
												auto& varTypeName = catch_block.children[0]->children[0]->text; // e.g. exception&
												auto& varName = catch_block.children[0]->children[1]->text; // e.g. x

												// ensure the var type matches

												// TO-DO
												currentScope->insert_object_here(varName, t_except);
												retval = catch_block.children[1]->eval(currentScope);
												wasCaught = true;
												break;
											}
										}
										else {
											throw exception::eval_error("Internal error: catch block variable unrecognized", this->location.start);
										}
									}
								}
								else {
									throw exception::eval_error("Internal error: catch block type unrecognized", this->location.start);
								}
							}

							if (!wasCaught) {
								rethrow = true;
							}

							return retval;
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							Any retval;
							Any err;
							std::exception_ptr Error;
							bool rethrow = false;
							auto thisScope = currentScope->make_scope();

							try {
								retval = this->children[0]->eval(thisScope);
							}
							catch (const std::exception& e) {
								Error = std::current_exception();
								// this must be handled within this scope before the exception goes out-of-scope.
								auto exception = Any(std::shared_ptr<std::exception>(const_cast<std::exception*>(&e), [](std::exception* p) { /* do nothing */ }));
								retval = handle_exception(thisScope, exception, rethrow);
							}
							catch (Any& e) {
								Error = std::current_exception();
								retval = handle_exception(thisScope, e, rethrow);
							}
							catch (...) {
								Error = std::current_exception();
								// unhandled exception type
								if (this->children.back()->identifier == Engine::AST_Node_Type::Finally) {
									this->children.back()->children[0]->eval(thisScope);
								}
								rethrow = true;
							}

							if (rethrow) {
								std::rethrow_exception(Error);
							}

							if (this->children.back()->identifier == Engine::AST_Node_Type::Finally) {
								retval = this->children.back()->children[0]->eval(thisScope);
							}

							return retval;
#else
							return {};
#endif
						}
					};

					struct Catch_AST_Node final : AST_Node_Impl {
						Catch_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Catch, std::move(t_loc), std::move(t_children))
						{}
					};

					struct Finally_AST_Node final : AST_Node_Impl {
						Finally_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Finally, std::move(t_loc), std::move(t_children))
						{}
					};

					/*! Currently, the JIT compilation does not support preprocessor macros or other preprocessor activities. */
					struct JustInTimeCompilation_AST_Node final : AST_Node_Impl {
						JustInTimeCompilation_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::JustInTimeCompilation, std::move(t_loc), std::move(t_children))
						{
#if 0
							if (this->children[0]->identifier == Engine::AST_Node_Type::Constant) {
								auto& scriptVar = std::dynamic_pointer_cast<Constant_AST_Node>(this->children[0])->m_value;
								if (scriptVar.IsTypeOf<GL::string>()) {
									Compile(scriptVar.cast<GL::string&>(), currentScope, true);
								}
								else {
									auto SCRIPT = currentScope->Cast<GL::string>(currentScope->Call("to_string", { scriptVar }));
									Compile(SCRIPT, currentScope, true);
								}
							}
#endif
						}

						// eval("x + 1");
						// Each thread will compile its own code. If a thread sees the same code again, it will not re-compile it.
						// Pre-processor macros are not supported at this time. To do so would require text splicing, running the preprocessor, and then resuming the code here.
						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							if (this->children[0]->identifier == Engine::AST_Node_Type::Constant) {
								// consider this already done
							}
							else {
								auto SCRIPT = currentScope->Cast<GL::string>(currentScope->Call("to_string", { this->children[0]->eval(currentScope) }));
								Compile(SCRIPT, currentScope);
							}
							return TLS->second->eval(currentScope);
#else
							return {};
#endif
						};

					private:
						GL::thread_object_no_default< 
							std::pair < GL::string, AST_Node_Impl_Ptr >
						> TLS;
						void Compile(GL::string const& SCRIPT, GL::scope::impl::BasicScope* currentScope, bool UpdateAll = false) const {
							if (UpdateAll) {
								auto PARSER = Engine2::Compiler::Interpreter::Parser();
								auto PARSED_RESULT = PARSER.Parse(SCRIPT, currentScope);
								const_cast<decltype(TLS)&>(TLS)->first = SCRIPT;
								const_cast<decltype(TLS)&>(TLS)->second = PARSED_RESULT.first;
							}
							else {
								if ((!TLS->second) || (TLS->first != SCRIPT)) {
									auto PARSER = Engine2::Compiler::Interpreter::Parser();
									auto PARSED_RESULT = PARSER.Parse(SCRIPT, currentScope);

									const_cast<GL::string&>(TLS->first) = SCRIPT;
									const_cast<AST_Node_Impl_Ptr&>(TLS->second) = PARSED_RESULT.first;
								}
							}
						};
					};

					struct Scopeless_Block_AST_Node final : AST_Node_Impl {
						Scopeless_Block_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Scopeless_Block, std::move(t_loc), std::move(t_children))
						{};

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							// evaluate all of the children, return the result of the last child
							size_t numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(currentScope);
								}
								return this->children.back()->eval(currentScope);
							}
							else {
								return {};
							}
						}
					};
					struct Block_AST_Node final : AST_Node_Impl {
						Block_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Block, std::move(t_loc), std::move(t_children))
						{};

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							auto newScope = currentScope->make_scope();

							const auto numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(&newScope);
								}
								return this->children.back()->eval(&newScope);
							}
							else {
								return {};
							}
						};
					};

					struct Function_Block_AST_Node final : AST_Node_Impl {
						Function_Block_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::FunctionBlock, std::move(t_loc), std::move(t_children))
						{};

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							auto newScope = currentScope->make_scope();

							const auto numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(&newScope);
								}
								return this->children.back()->eval(&newScope);
							}
							else {
								return {};
							}
						};
					};

					struct Fold_Right_Binary_Operator_AST_Node : AST_Node_Impl {
						Fold_Right_Binary_Operator_AST_Node(GL::scope::impl::BasicScope* currentScope, const GL::string& t_oper, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children, GL::any::fast_any t_rhs)
							: AST_Node_Impl(t_oper, Engine::AST_Node_Type::BinaryFoldRight, std::move(t_loc), std::move(t_children))
							, m_oper(Engine::Operators::to_operator(t_oper.c_str()))
							, m_rhs(t_rhs)
						{}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return currentScope->call(this->text, { this->children[0]->eval(currentScope), m_rhs });
						};

					private:
						Engine::Operators::Opers m_oper;
						GL::any::fast_any m_rhs;
					};

					struct Namespace_AST_Node final : AST_Node_Impl {
						Namespace_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Namespace, std::move(t_loc), std::move(t_children))
						{
							(void)this->children.back()->eval(currentScope);
						}
						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return {};
						};
					};

					struct Class_AST_Node final : AST_Node_Impl {
						Class_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::Namespace, std::move(t_loc), std::move(t_children))
						{
							(void)this->children.back()->eval(currentScope);
						}

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							return {};
						}
					};

					struct Declaration_Block_AST_Node final : AST_Node_Impl {
						Declaration_Block_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::DeclarationBlock, std::move(t_loc), std::move(t_children))
						{};

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
							const auto numChildren = this->children.size();
							if (numChildren > 0) {
								for (int i = 0; i < numChildren - 1; i++) {
									this->children[i]->eval(currentScope);
								}
								return this->children.back()->eval(currentScope);
							}
							else {
								return {};
							}
						};
					};

					struct FunctionDecl_AST_Node final : AST_Node_Impl {
						FunctionDecl_AST_Node(GL::scope::impl::BasicScope* currentScope, GL::string t_ast_node_text, Engine::Parse_Location t_loc, std::vector<AST_Node_Impl_Ptr> t_children)
							: AST_Node_Impl(std::move(t_ast_node_text), Engine::AST_Node_Type::FunctionDecl, std::move(t_loc), std::move(t_children))
							//, inputArgNames(Arg_List_AST_Node::get_arg_names(*this->children[2]))
							//, inputArgTypes(Arg_List_AST_Node::get_arg_types(*this->children[2]))
						{
							ASSERT(this->children.size() == 4); // Id -> return_type, Id -> function_name, Arg_List, Block

							//function_name = GetText(this->children[1]);
							//numArgs = this->children[2]->children.size();
							//FunctionBlock = this->children[3] = optimizer::optimize(this->children[3], currentScope);
							//return_type_name = GetText(this->children[0]);
							//returnArgType = GetClassType(this->children[0], currentScope);
						};
#if 0
						static void AddObjects(int startposition, std::shared_ptr< Scopes::BasicScope> const& thisScope, std::vector<GL::string> const& argNames, std::vector<GL::type> const& argTypes) {};
						template<typename T, typename... R> static void AddObjects(int startposition, std::shared_ptr< Scopes::BasicScope> const& thisScope, std::vector<GL::string> const& argNames, std::vector<GL::type> const& argTypes, T const& argument, R const&... arguments) {
							thisScope->EmplaceObject(argNames[startposition], std::make_shared<Any>(thisScope->Cast(argument, argTypes[startposition], true)), false);
							AddObjects(startposition + 1, thisScope, argNames, argTypes, arguments...);
						};
#endif

						GL::any::fast_any eval_internal(GL::scope::impl::BasicScope* currentScope) const override {
#if 0
							if (currentScope->is_class()) {
								if (auto ptr = std::dynamic_pointer_cast<Scopes::ClassScope>(currentScope)) {
									auto p_locked = initialized.Unique();
									if (!*p_locked) {
										auto& types = const_cast<std::vector<GL::type>&>(inputArgTypes);
										auto& names = const_cast<std::vector<GL::string>&>(inputArgNames);

										types.insert(types.begin(), ptr->ClassType->MakeConstRef());
										names.insert(names.begin(), "this");
										const_cast<int&>(numArgs)++;
										const_cast<bool&>(*p_locked) = true;
									}
								}
							}

							// the current scope should be a namespace... HOPEFULLY! Need to confirm... Perhaps also need to change the impl based on whether we are in a Class or in a Namespace or in a Global?
							Proxy_Function func;
							if ((!this->returnArgType.expired()) && (GetHash(this->returnArgType) == GetHash(user_type<void>()))) {
								switch (numArgs) {
								case 0:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									]() {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 1:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& in1) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, in1);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 2:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& in1, Any const& in2) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, in1, in2);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 3:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 4:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 5:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 6:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 7:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 8:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 9:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 10:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 11:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 12:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l) {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 13:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 14:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m, Any const& n) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m, n);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;

								case 15:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m, Any const& n, Any const& o) {
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);

											try { lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) {}
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes));
									break;
								}
							}
							else {
								switch (numArgs) {
								case 0:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									]()->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }

											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 1:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& in1)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, in1);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 2:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& in1, Any const& in2)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, in1, in2);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 3:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 4:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 5:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 6:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 7:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 8:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 9:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 10:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 11:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 12:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 13:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 14:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m, Any const& n)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m, n);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;

								case 15:
									func = GoodLang::make_callable([InputArgNames = this->inputArgNames, InputArgTypes = this->inputArgTypes, lambda = FunctionBlock, declaringNamespace = std::weak_ptr<Scopes::BasicScope>(currentScope), returnType = this->returnArgType
									](Any const& a, Any const& b, Any const& c, Any const& d, Any const& e, Any const& f, Any const& g,
										Any const& h, Any const& i, Any const& j, Any const& k, Any const& l, Any const& m, Any const& n, Any const& o)->Any {
										Any result;
										if (auto parentScope = declaringNamespace.lock()) {
											auto thisScope = parentScope->make_scope();

											AddObjects(0, thisScope, InputArgNames, InputArgTypes, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o);

											try { result = lambda->eval(thisScope); }
											catch (detail::Return_Value& rv) { result = rv.retval; }
											if (returnType.expired()) { return result; }
											else { return thisScope->Cast(result, returnType); }
										}
										else { throw exception::eval_error("Namespace with declared function is no longer available"); }
									}, ParamTypes(inputArgTypes), this->returnArgType);
									break;





								}
							};

							if (func) {
								currentScope->EmplaceFunction(function_name, func);
							}
							else {
								throw exception::eval_error("Function has too many parameters!");
							}
							return {};
#else
                        return {};
#endif
						};

						//GoodLang::SharedLockable<bool> initialized{ false };
						//GL::string return_type_name;
						//GL::string function_name;
						//int numArgs;
						//AST_Node_Impl_Ptr FunctionBlock;
						//std::vector<GL::type> inputArgTypes;
						//std::vector<GL::string> inputArgNames;
						//GL::type returnArgType;
					};
				};

				class optimizer {
				private:
					template<typename... T> struct Optimizer : T... {
						Optimizer() = default;
						explicit Optimizer(T... t) : T(std::move(t))... { };
						AST_Node_Impl_Ptr optimize(AST_Node_Impl_Ptr p, GL::scope::impl::BasicScope* currentScope) {
							long long maxDepth = 100;
							while (--maxDepth >= 0) {
								bool successful = false;
								((successful = (successful || static_cast<T&>(*this).optimize(p, currentScope))), ...); // this line performs all optimizations in-line
								if (!successful) break;
							}
							return p;
						};
					};

					static AST_Node_Impl& child_at(AST_Node_Impl& node, const size_t offset) noexcept {
						return *node.children[offset];
					};
					static const AST_Node_Impl& child_at(const AST_Node_Impl& node, const size_t offset) noexcept {
						return *node.children[offset];
					};
					static size_t child_count(const AST_Node_Impl& node) noexcept {
						return node.children.size();
					};
					static bool contains_var_decl_in_scope(const AST_Node_Impl& node) noexcept {
						if (
							node.identifier == Engine::AST_Node_Type::Var_Decl
							|| node.identifier == Engine::AST_Node_Type::Assign_Decl
							|| node.identifier == Engine::AST_Node_Type::Reference
							|| node.identifier == Engine::AST_Node_Type::Assign_Retroactively
							|| node.identifier == Engine::AST_Node_Type::Def
							|| node.identifier == Engine::AST_Node_Type::Class
							) {
							return true;
						}

						const auto num = child_count(node);

						for (size_t i = 0; i < num; ++i) {
							const auto& child = child_at(node, i);
							if (child.identifier != Engine::AST_Node_Type::Block
								&& child.identifier != Engine::AST_Node_Type::For
								&& child.identifier != Engine::AST_Node_Type::Ranged_For
								&& child.identifier != Engine::AST_Node_Type::Parallel_For
								&& child.identifier != Engine::AST_Node_Type::Parallel_Ranged_For
								&& contains_var_decl_in_scope(child)
								) {
								return true;
							}
						}

						return false;
					};

					// re-arrange the return statement, to avoid throwing whenever possible
					struct Example {
						bool optimize(AST_Node_Impl_Ptr& p, GL::scope::impl::BasicScope* currentScope) {
							return false; // does nothing
						}
					};

					// String embedding results in a structure that may resemble:
					//		ArgList -> {  File -> {   Constant   }  }
					// This should be simplified to: 
					//		ArgList -> {  Constant  }
					struct ArgListFileConstant {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if (node->identifier == Engine::AST_Node_Type::Arg_List
								&& node->children.size() == 1
								&& node->children[0]->identifier == Engine::AST_Node_Type::File
								&& node->children[0]->children.size() == 1
								&& node->children[0]->children[0]->identifier == Engine::AST_Node_Type::Constant
								) {
								node->children[0] = std::move(node->children[0]->children[0]);
								return true;
							}
							return false;
						}
					};

					// converts from:
					//		var x = int(1)
					// to:
					//		int x{ 1 };
					struct VarDeclEquation_To_RetroactiveAssignment {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if (node->identifier == Engine::AST_Node_Type::Equation
								&& node->children.size() == 2
								&& ((node->children[0]->identifier == Engine::AST_Node_Type::Reference) || (node->children[0]->identifier == Engine::AST_Node_Type::Var_Decl))
								&& node->children[0]->children.size() == 1
								// && node->children[0]->children[0]->identifier == AST_Node_Type::Id
								// && node->children[1]->identifier == AST_Node_Type::Fun_Call
								&& ((node->text == "=") || (node->text == ":="))
								) {
								node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Assign_Retroactively_AST_Node>(
									currentScope, node->text, node->location, std::vector<AST_Node_Impl_Ptr>{
										std::move(node->children[1]),
										std::move(node->children[0])
									}
								));
								return true;
							}

							if (node->identifier == Engine::AST_Node_Type::Equation
								&& node->children.size() == 2
								&& node->children[0]->identifier == Engine::AST_Node_Type::Assign_Retroactively
								&& ((node->text == "=") || (node->text == ":="))
								) {
								node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Assign_Retroactively_AST_Node>(
									currentScope, node->text, node->location, std::vector<AST_Node_Impl_Ptr>{
										std::move(node->children[0]->children[0]),
										std::move(node->children[0]->children[1]),
										std::move(node->children[1])
									}
								));
								return true;
							}




							return false;
						}
					};

					// String embedding results in a structure that may resemble:
					//		Fun_Call -> {  Id{ to_string }, Arg_list{ Constant{} } }
					// This should be simplified and completed:
					//      Constant(to_string(Constant()))
					struct ToStringFunctionCallWithConstant {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if (node->identifier == Engine::AST_Node_Type::Fun_Call
								&& node->children.size() == 2
								&& node->children[0]->identifier == Engine::AST_Node_Type::Id
								&& node->children[1]->identifier == Engine::AST_Node_Type::Arg_List
								&& node->children[1]->children.size() == 1
								&& node->children[1]->children[0]->identifier == Engine::AST_Node_Type::Constant
								&& node->children[0]->text == "to_string"
								) {
								const GL::any::fast_any& rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[1]->children[0].get())->m_value;
								try {
									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, currentScope->call("to_string", { rhs })
								    ));
									return true;
								}
								catch (...) {
									// failure to fold -- that's OK
									return false;
								}
							}
							return false;
						}
					};

					// removes items from Blocks that are unecessary (e.g. floating code) or will never be hit (e.g. following return statements)
					struct Dead_Code {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if ((node->identifier == Engine::AST_Node_Type::Block) || (node->identifier == Engine::AST_Node_Type::Scopeless_Block)) {
								std::vector<size_t> keepers;
								const auto num_children = node->children.size();
								keepers.reserve(num_children);
								bool foundReturnStatement = false;
								for (size_t i = 0; i < (num_children - 1); ++i) {
									const auto& child = *node->children[i];
									switch (child.identifier) {
									case Engine::AST_Node_Type::Constant: // 50.0f;
									case Engine::AST_Node_Type::Noop: // comments
									case Engine::AST_Node_Type::Id: // y, x, etc.
										break;
									case Engine::AST_Node_Type::Return: // return; return x; return 50; etc.
										keepers.push_back(i);
										i = num_children; // stop considering the remaining items -- they'll never be found anyways. 
										foundReturnStatement = true;
										break;
									default:
										keepers.push_back(i);
										break;
									}
								}
								if ((!foundReturnStatement) && (num_children > 0)) { keepers.push_back(num_children - 1); };

								if (keepers.size() == num_children) {
									return false;
								}
								else {
									const auto new_children = [&]() {
										std::vector<AST_Node_Impl_Ptr> retval;
										for (const auto x : keepers) {
											retval.push_back(std::move(node->children[x]));
										}
										return retval;
									};

									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Block_AST_Node>(currentScope, node->text, node->location, new_children()));

									return true;
								}
							}
							else {
								return false;
							}
						}
					};

					// re-arrange the return statement, to avoid throwing whenever possible
					struct Return {
						bool optimize(AST_Node_Impl_Ptr& p, GL::scope::impl::BasicScope* currentScope) {
							if ((p->identifier == Engine::AST_Node_Type::Lambda) && !p->children.empty()) {
								auto& last_child = p->children.back();
								if (last_child->identifier == Engine::AST_Node_Type::Block || last_child->identifier == Engine::AST_Node_Type::Scopeless_Block) {
									auto& block_last_child = last_child->children.back();
									if (block_last_child->identifier == Engine::AST_Node_Type::Return) {
										if (block_last_child->children.size() == 1) {
											block_last_child = std::move(block_last_child->children[0]);
											return true;
										}
										else if (block_last_child->children.size() == 0) {
											block_last_child = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Noop_AST_Node>());
											return true;
										}
									}
								}
								//if (last_child->identifier == AST_Node_Type::Return) {
								//	if (last_child->children.size() == 1) {
								//		last_child = std::move(last_child->children[0]);
								//		return true;
								//	}
								//	else if (last_child->children.size() == 0) {
								//		last_child = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<Noop_AST_Node>());
								//		return true;
								//	}
								//}
							}
							if ((p->identifier == Engine::AST_Node_Type::Def) && !p->children.empty()) {
								auto& last_child = p->children.back();
								if (last_child->identifier == Engine::AST_Node_Type::Block || last_child->identifier == Engine::AST_Node_Type::Scopeless_Block) {
									auto& block_last_child = last_child->children.back();
									if (block_last_child->identifier == Engine::AST_Node_Type::Return) {
										if (block_last_child->children.size() == 1) {
											last_child->children.back() = std::move(block_last_child->children[0]);
											return true;
										}
									}
								}
							}
							if (p->identifier == Engine::AST_Node_Type::File && !p->children.empty()) {
								auto& last_child = p->children.back();
								if (last_child->identifier == Engine::AST_Node_Type::Block || last_child->identifier == Engine::AST_Node_Type::Scopeless_Block) {
									auto& block_last_child = last_child->children.back();
									if (block_last_child->identifier == Engine::AST_Node_Type::Return) {
										if (block_last_child->children.size() == 0) {
											last_child->children.back() = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Noop_AST_Node>());
											return true;
										}
										else if (block_last_child->children.size() == 1) {
											last_child->children.back() = std::move(block_last_child->children[0]);
											return true;
										}
									}
								}
								else if (last_child->identifier == Engine::AST_Node_Type::Return) {
									if (last_child->children.size() == 0) {
										last_child = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Noop_AST_Node>());
										return true;
									}
									else if (last_child->children.size() == 1) {
										last_child = std::move(last_child->children[0]);
										return true;
									}
								}
							}
							return false;
						}
					};

					// removes the scope from blocks if they do not have declarations at all
					struct Block {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if (node->identifier == Engine::AST_Node_Type::Block) {
								if (!contains_var_decl_in_scope(*node)) {
									if (node->children.size() == 1) {
										node = std::move(node->children[0]);
										return true;
									}
									else {
										node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Scopeless_Block_AST_Node>(
											currentScope,
											node->text,
											node->location,
											std::move(node->children)
											));
										return true;
									}
								}
							}
							else if (node->identifier == Engine::AST_Node_Type::Scopeless_Block) {
								if (!contains_var_decl_in_scope(*node)) {
									if (node->children.size() == 1) {
										node = std::move(node->children[0]);
										return true;
									}
								}
							}
							return false;
						}
					};

					// If a function call's return value was going to be unused, there may be no point to holding onto it. 
					struct Unused_Fun_Return {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							bool result = false;
							if ((node->identifier == Engine::AST_Node_Type::Block || node->identifier == Engine::AST_Node_Type::Scopeless_Block) && !node->children.empty()) {
								for (size_t i = 0; i < node->children.size() - 1; ++i) {
									auto child = node->children[i].get();
									if (child->identifier == Engine::AST_Node_Type::Fun_Call) {
										node->children[i] = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Unused_Return_Fun_Call_AST_Node>(
											currentScope,
											child->text,
											child->location,
											std::move(child->children)
											));
										result = true;
									}
								}
							}
							else if ((node->identifier == Engine::AST_Node_Type::For || node->identifier == Engine::AST_Node_Type::While) && child_count(*node) > 0) {
								auto& child = child_at(*node, child_count(*node) - 1);
								if (child.identifier == Engine::AST_Node_Type::Block || child.identifier == Engine::AST_Node_Type::Scopeless_Block) {
									auto num_sub_children = child_count(child);
									for (size_t i = 0; i < num_sub_children; ++i) {
										auto& sub_child = child_at(child, i);
										if (sub_child.identifier == Engine::AST_Node_Type::Fun_Call) {
											child.children[i] = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Unused_Return_Fun_Call_AST_Node>(
												currentScope,
												sub_child.text,
												sub_child.location,
												std::move(sub_child.children)
											));
											result = true;
										}
									}
								}
							}
							return result;
						}
					};

					// If the condition of an If statement is constant and known, then simply skip the check and hard-code the correct path. 
					struct If {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if ((node->identifier == Engine::AST_Node_Type::If) && (node->children.size() >= 2) && (node->children[0]->identifier == Engine::AST_Node_Type::Constant)) {
								try {
									if (currentScope->cast<bool>(dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[0].get())->m_value)) {
										// "TRUE" statement is the exclusive path
										node = std::move(node->children[1]);
										return true;
									}
									else if (node->children.size() == 3) {
										// "FALSE" statement is the exclusive path (and a false path is even present)
										node = std::move(node->children[2]);
										return true;
									}
									else {
										// do nothing?
										node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Noop_AST_Node>());
										return true;
									}
								}
								catch (...) {
									return false;
								}
							}
							return false;
						}
					};

					// Try to fold a basic prefix operation with a constant value
					struct PrefixFold {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if (node->identifier == Engine::AST_Node_Type::Prefix
								&& node->children.size() == 1
								&& node->children[0]->identifier == Engine::AST_Node_Type::Constant
								) {
								const GL::any::fast_any& rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[0].get())->m_value;
								try {
									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, currentScope->call(node->text, { rhs })
								    ));
									return true;
								}
								catch (...) {
									// failure to fold -- that's OK. 
									return false;
								}
							}
							return false;
						}
					};

					// postfix's (++/--) on constant values should simply return the same constant value before the change anyways. 
					struct PostfixFold {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if (node->identifier == Engine::AST_Node_Type::Postfix
								&& node->children.size() == 1
								&& node->children[0]->identifier == Engine::AST_Node_Type::Constant
								&& ((node->text == "++") || (node->text == "--"))
						    ) {
								node = std::move(node->children[0]);
								return true;
							}
							return false;
						}
					};

					// Try to fold a basic binary operation between two constant values (e.g. GL::string + GL::string, or Units::foot == Units::meter)
					struct BinaryFold {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if (node->identifier == Engine::AST_Node_Type::Binary
								&& node->children.size() == 2
								&& node->children[0]->identifier == Engine::AST_Node_Type::Constant
								&& node->children[1]->identifier == Engine::AST_Node_Type::Constant
								) {
								const GL::any::fast_any& lhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[0].get())->m_value;
								const GL::any::fast_any& rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[1].get())->m_value;

								try {
									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, currentScope->call(node->text, { lhs, rhs })
									));
									return true;
								}
								catch (...) {
									// failure to fold -- that's OK
									return false;
								}
							}
							return false;
						}
					};

					// Try to fold a basic binary operation (e.g. +/-/*) with one constant value, to speed-up evaluation in the future
					struct PartialBinaryFold {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							// Fold right side
							if (node->identifier == Engine::AST_Node_Type::Binary
								&& node->children.size() == 2
								&& node->children[0]->identifier != Engine::AST_Node_Type::Constant
								&& node->children[1]->identifier == Engine::AST_Node_Type::Constant
								) {
								try {
									const auto& oper = node->text;
									const auto parsed = Engine::Operators::to_operator(oper.c_str());
									if (parsed != Engine::Operators::Opers::invalid) {
										const auto rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(node->children[1].get())->m_value;
										node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Fold_Right_Binary_Operator_AST_Node>(
											currentScope, node->text, node->location, std::move(node->children), rhs
										));
										return true;
									}
								}
								catch (const std::exception&) {
									// failure to fold, that's OK
									return false;
								}
							}

							return false;
						}
					};

					// If an Inline_Array is made-up of const elements, then evaluate and store it as constexpr too.
					struct ConstArray {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
#if 0
							// Fold right side
							if (node->identifier == Engine::AST_Node_Type::Inline_Array
								&& node->children.size() == 1
								&& node->children[0]->identifier == Engine::AST_Node_Type::Arg_List
								) {
								auto& argList = *node->children.back();

								bool allItemsAreConst = true;
								for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
									if (argList.children[childIndex]->identifier != Engine::AST_Node_Type::Constant) {
										allItemsAreConst = false;
										break;
									}
								}

								if (allItemsAreConst) {
									Vector<Var> constArray;
									for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
										Any rhs = dynamic_cast<AST_Nodes::Constant_AST_Node*>(argList.children[childIndex].get())->m_value;
										rhs.SetFlag(AnyData::Flag::constant, true);
										constArray.push_back(Var(std::move(rhs)));
									}
									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, std::move(constArray)
										));
									return true;
								}
							}
							return false;
#else
							return false;
#endif
						}
					};

					// If an Inline_Map is made-up of const elements, then evaluate and store it as constexpr too.
					struct ConstMap {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							// Fold right side
#if 0
							if (node->identifier == Engine::AST_Node_Type::Inline_Map
								&& node->children.size() == 1
								&& node->children[0]->identifier == Engine::AST_Node_Type::Arg_List
								) {
								auto& argList = *node->children.back();

								bool allItemsAreConst = true;
								for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
									if (argList.children[childIndex]->identifier == Engine::AST_Node_Type::Map_Pair) {
										auto& map_pair = *dynamic_cast<AST_Nodes::Map_Pair_AST_Node*>(argList.children[childIndex].get());
										if (
											(map_pair.children[0]->identifier == Engine::AST_Node_Type::Constant) &&
											(map_pair.children[1]->identifier == Engine::AST_Node_Type::Constant)
											) {
											continue;
										}
										else {
											allItemsAreConst = false;
											break;
										}
									}
									else {
										allItemsAreConst = false;
										break;
									}
								}

								if (allItemsAreConst) {
									Any constArray{ Map<size_t, std::pair<Var, Var>>() };
									if (!node->children.empty()) {
										for (const auto& child : node->children[0]->children) {
											auto rhs_key = child->children[0]->eval(currentScope);
											auto rhs_val = child->children[1]->eval(currentScope);

											rhs_key.SetFlag(AnyData::Flag::constant, true);
											rhs_val.SetFlag(AnyData::Flag::constant, true);

											currentScope->Call("emplace", {
												constArray,
												rhs_key,
												rhs_val
												});
										}
									}
									constArray.SetFlag(AnyData::Flag::constant, true);

									node = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Constant_AST_Node>(
										node->text, node->location, std::move(constArray)
										));
									return true;
								}
							}

							return false;
#else
							return false;
#endif
						}
					};

					// Improve the performance of a well-defined for loop by re-structuring it.
					struct ForLoopSignature {
						bool optimize(AST_Node_Impl_Ptr& node, GL::scope::impl::BasicScope* currentScope) {
							if (node->identifier == Engine::AST_Node_Type::For
								&& node->children.size() >= 4
								) {
								// x++ into ++x;
								if (node->children[2]->identifier == Engine::AST_Node_Type::Postfix) { // x++
									switch (Engine::hash(GetText(node->children[2]).c_str())) {
									case Engine::hash("++"):
										node->children[2] = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Prefix_AST_Node>(
											currentScope, "++", node->children[2]->location, std::move(node->children[2]->children)
											));
										return true;
									case Engine::hash("--"):
										node->children[2] = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Prefix_AST_Node>(
											currentScope, "--", node->children[2]->location, std::move(node->children[2]->children)
											));
										return true;
									default:
										return false;
									};
								}
							}
							return false;
						};
					};

					using Optimizer_Default = Optimizer<
						optimizer::PostfixFold,
						optimizer::PrefixFold,
						optimizer::BinaryFold,
						optimizer::PartialBinaryFold,
						optimizer::Unused_Fun_Return,
						optimizer::ArgListFileConstant,
						optimizer::VarDeclEquation_To_RetroactiveAssignment,
						optimizer::ToStringFunctionCallWithConstant,
						optimizer::ConstArray,
						optimizer::ConstMap,
						optimizer::If,
						optimizer::Return,
						optimizer::Dead_Code,
						optimizer::ForLoopSignature,
						optimizer::Block
					>;

				public:
					static AST_Node_Impl_Ptr optimize(AST_Node_Impl_Ptr p, GL::scope::impl::BasicScope* currentScope) {
						return Optimizer_Default().optimize(p, currentScope);
					};
				}; // namespace optimizer

				class Parser {
				private:
					constexpr static utility::Static_String m_multiline_comment_end{ "*/" };
					constexpr static utility::Static_String m_multiline_comment_begin{ "/*" };
					constexpr static utility::Static_String m_singleline_comment{ "//" };
					constexpr static utility::Static_String m_annotation{ "#" };
					constexpr static utility::Static_String m_cr_lf{ "\r\n" };

					template<typename string_type> struct Char_Parser {
						string_type& match;
						using char_type = typename string_type::value_type;
						bool is_escaped = false;
						bool is_interpolated = false;
						bool saw_interpolation_marker = false;
						bool is_octal = false;
						bool is_hex = false;
						std::size_t unicode_size = 0;
						const bool interpolation_allowed;

						string_type octal_matches;
						string_type hex_matches;

						Char_Parser(string_type& t_match, const bool t_interpolation_allowed)
							: match(t_match)
							, interpolation_allowed(t_interpolation_allowed) {
						}

						Char_Parser& operator=(const Char_Parser&) = delete;

						~Char_Parser() {
							try {
								if (is_octal) {
									process_octal();
								}

								if (is_hex) {
									process_hex();
								}

								if (unicode_size > 0) {
									process_unicode();
								}
							}
							catch (const std::invalid_argument&) {
							}
							catch (const exception::eval_error&) {
								// Something happened with parsing, we'll catch it later?
							}
						}

						void process_hex() {
							if (!hex_matches.empty()) {
								auto val = stoll(hex_matches.to_string(), nullptr, 16);
								match = match + std::string(1, char_type(val));
							}
							hex_matches = "";
							is_escaped = false;
							is_hex = false;
						}

						void process_octal() {
							if (!octal_matches.empty()) {
								auto val = stoll(octal_matches.to_string(), nullptr, 8);
								match = match + std::string(1, char_type(val));
							}
							octal_matches = "";
							is_escaped = false;
							is_octal = false;
						}

						void process_unicode() {
							const auto ch = static_cast<uint32_t>(std::stoi(hex_matches.to_string(), nullptr, 16));
							const auto match_size = hex_matches.size();
							hex_matches = "";
							is_escaped = false;
							const auto u_size = unicode_size;
							unicode_size = 0;

							char buf[4];
							if (u_size != match_size) {
								throw exception::eval_error("Incomplete unicode escape sequence");
							}
							if (u_size == 4 && ch >= 0xD800 && ch <= 0xDFFF) {
								throw exception::eval_error("Invalid 16 bit universal character");
							}

							if (ch < 0x80) {
								match = match + std::string(1, static_cast<char>(ch));
							}
							else if (ch < 0x800) {
								buf[0] = static_cast<char>(0xC0 | (ch >> 6));
								buf[1] = static_cast<char>(0x80 | (ch & 0x3F));
								match = match + std::string(buf, 2);
							}
							else if (ch < 0x10000) {
								buf[0] = static_cast<char>(0xE0 | (ch >> 12));
								buf[1] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
								buf[2] = static_cast<char>(0x80 | (ch & 0x3F));
								match = match + std::string(buf, 3);
							}
							else if (ch < 0x200000) {
								buf[0] = static_cast<char>(0xF0 | (ch >> 18));
								buf[1] = static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
								buf[2] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
								buf[3] = static_cast<char>(0x80 | (ch & 0x3F));
								match = match + std::string(buf, 4);
							}
							else {
								// this must be an invalid escape sequence?
								throw exception::eval_error("Invalid 32 bit universal character");
							}
						}

						void parse(const char_type t_char, Engine::Position pos) {
							const bool is_octal_char = t_char >= '0' && t_char <= '7';

							const bool is_hex_char = (t_char >= '0' && t_char <= '9') || (t_char >= 'a' && t_char <= 'f') || (t_char >= 'A' && t_char <= 'F');

							if (is_octal) {
								if (is_octal_char) {
									octal_matches = octal_matches + std::string(1, t_char);

									if (octal_matches.size() == 3) {
										process_octal();
									}
									return;
								}
								else {
									process_octal();
								}
							}
							else if (is_hex) {
								if (is_hex_char) {
									hex_matches = hex_matches + std::string(1, t_char);

									if (hex_matches.size() == 2 * sizeof(char_type)) {
										// This rule differs from the C/C++ standard, but ChaiScript
										// does not offer the same workaround options, and having
										// hexadecimal sequences longer than can fit into the char
										// type is undefined behavior anyway.
										process_hex();
									}
									return;
								}
								else {
									process_hex();
								}
							}
							else if (unicode_size > 0) {
								if (is_hex_char) {
									hex_matches = hex_matches + std::string(1, t_char);

									if (hex_matches.size() == unicode_size) {
										// Format is specified to be 'slash'uABCD
										// on collecting from A to D do parsing
										process_unicode();
									}
									return;
								}
								else {
									// Not a unicode anymore, try parsing any way
									// May be someone used 'slash'uAA only
									process_unicode();
								}
							}

							if (t_char == '\\') {
								if (is_escaped) {
									match = match + "\\";
									is_escaped = false;
								}
								else {
									is_escaped = true;
								}
							}
							else {
								if (is_escaped) {
									if (is_octal_char) {
										is_octal = true;
										octal_matches = octal_matches + std::string(1, t_char);
									}
									else if (t_char == 'x') {
										is_hex = true;
									}
									else if (t_char == 'u') {
										unicode_size = 4;
									}
									else if (t_char == 'U') {
										unicode_size = 8;
									}
									else {
										switch (t_char) {
										case ('\''):
											match = match + "\'";
											break;
										case ('\"'):
											match = match + "\"";
											break;
										case ('?'):
											match = match + "?";
											break;
										case ('a'):
											match = match + "\a";
											break;
										case ('b'):
											match = match + "\b";
											break;
										case ('f'):
											match = match + "\f";
											break;
										case ('n'):
											match = match + "\n";
											break;
										case ('r'):
											match = match + "\r";
											break;
										case ('t'):
											match = match + "\t";
											break;
										case ('v'):
											match = match + "\v";
											break;
										case ('$'):
											match = match + "$";
											break;
										default:
											throw exception::eval_error("Unknown escaped sequence in string", pos);
										}
										is_escaped = false;
									}
								}
								else if (interpolation_allowed && t_char == '$') {
									saw_interpolation_marker = true;
								}
								else {
									match = match + std::string(1, t_char);
								}
							}
						}
					};
					static std::map<GL::string, GL::any::fast_any> build_constants() {
						std::map<GL::string, GL::any::fast_any> out;
						out["true"] = const_var(true);
						out["false"] = const_var(false);
						out["Infinity"] = const_var(std::numeric_limits<double>::infinity());
						out["NaN"] = const_var(std::numeric_limits<double>::quiet_NaN());
						out["nullptr"] = const_var(GL::any());
						out["null"] = const_var(GL::any());
						return out;
					};
					static std::map<GL::string, GL::any::fast_any> const& constants() {
						static auto out{ build_constants() };
						return out;
					};


					template<typename Array2D, typename First, typename Second>
					static void set_alphabet(Array2D& array, const First first, const Second second) noexcept {
						auto* first_ptr = &std::get<0>(array) + static_cast<std::size_t>(first);
						auto* second_ptr = &std::get<0>(*first_ptr) + static_cast<std::size_t>(second);
						*second_ptr = true;
					};
					static std::array<std::array<bool, Engine::lengthof_alphabet>, Engine::max_alphabet> build_alphabet() noexcept {
						std::array<std::array<bool, Engine::lengthof_alphabet>, Engine::max_alphabet> alphabet{};

						set_alphabet(alphabet, Engine::symbol_alphabet, '?');

						set_alphabet(alphabet, Engine::symbol_alphabet, '?');
						set_alphabet(alphabet, Engine::symbol_alphabet, '+');
						set_alphabet(alphabet, Engine::symbol_alphabet, '-');
						set_alphabet(alphabet, Engine::symbol_alphabet, '*');
						set_alphabet(alphabet, Engine::symbol_alphabet, '/');
						set_alphabet(alphabet, Engine::symbol_alphabet, '|');
						set_alphabet(alphabet, Engine::symbol_alphabet, '&');
						set_alphabet(alphabet, Engine::symbol_alphabet, '^');
						set_alphabet(alphabet, Engine::symbol_alphabet, '=');
						set_alphabet(alphabet, Engine::symbol_alphabet, '.');
						set_alphabet(alphabet, Engine::symbol_alphabet, '<');
						set_alphabet(alphabet, Engine::symbol_alphabet, '>');

						for (size_t c = 'a'; c <= 'z'; ++c) {
							set_alphabet(alphabet, Engine::keyword_alphabet, c);
						}
						for (size_t c = 'A'; c <= 'Z'; ++c) {
							set_alphabet(alphabet, Engine::keyword_alphabet, c);
						}
						for (size_t c = '0'; c <= '9'; ++c) {
							set_alphabet(alphabet, Engine::keyword_alphabet, c);
						}
						set_alphabet(alphabet, Engine::keyword_alphabet, '_');
						// set_alphabet(alphabet, keyword_alphabet, ':');

						for (size_t c = '0'; c <= '9'; ++c) {
							set_alphabet(alphabet, Engine::int_alphabet, c);
						}
						for (size_t c = '0'; c <= '9'; ++c) {
							set_alphabet(alphabet, Engine::float_alphabet, c);
						}
						set_alphabet(alphabet, Engine::float_alphabet, '.');

						for (size_t c = '0'; c <= '9'; ++c) {
							set_alphabet(alphabet, Engine::hex_alphabet, c);
						}
						for (size_t c = 'a'; c <= 'f'; ++c) {
							set_alphabet(alphabet, Engine::hex_alphabet, c);
						}
						for (size_t c = 'A'; c <= 'F'; ++c) {
							set_alphabet(alphabet, Engine::hex_alphabet, c);
						}

						set_alphabet(alphabet, Engine::x_alphabet, 'x');
						set_alphabet(alphabet, Engine::x_alphabet, 'X');

						for (size_t c = '0'; c <= '1'; ++c) {
							set_alphabet(alphabet, Engine::bin_alphabet, c);
						}
						set_alphabet(alphabet, Engine::b_alphabet, 'b');
						set_alphabet(alphabet, Engine::b_alphabet, 'B');

						for (size_t c = 'a'; c <= 'z'; ++c) {
							set_alphabet(alphabet, Engine::id_alphabet, c);
						}
						for (size_t c = 'A'; c <= 'Z'; ++c) {
							set_alphabet(alphabet, Engine::id_alphabet, c);
						}
						set_alphabet(alphabet, Engine::id_alphabet, '_');
						set_alphabet(alphabet, Engine::id_alphabet, ':'); // RG
						for (size_t c = '0'; c <= '9'; ++c) { set_alphabet(alphabet, Engine::id_alphabet, c); } // RG

						set_alphabet(alphabet, Engine::white_alphabet, ' ');
						set_alphabet(alphabet, Engine::white_alphabet, '\t');

						set_alphabet(alphabet, Engine::int_suffix_alphabet, 'l');
						set_alphabet(alphabet, Engine::int_suffix_alphabet, 'L');
						set_alphabet(alphabet, Engine::int_suffix_alphabet, 'u');
						set_alphabet(alphabet, Engine::int_suffix_alphabet, 'U');

						set_alphabet(alphabet, Engine::float_suffix_alphabet, 'l');
						set_alphabet(alphabet, Engine::float_suffix_alphabet, 'L');
						set_alphabet(alphabet, Engine::float_suffix_alphabet, 'f');
						set_alphabet(alphabet, Engine::float_suffix_alphabet, 'F');

						return alphabet;
					}
					static std::array<std::array<bool, Engine::lengthof_alphabet>, Engine::max_alphabet> const& alphabet() {
						static auto out{ build_alphabet() };
						return out;
					};

					struct Operator_Matches {
						using SS = utility::Static_String;

						struct Operator_Matches_Impl {
							using SS = utility::Static_String;
							// should match the order and categories from create_operators()
							const std::array<utility::Static_String, 2> m_0{ {SS("?"), SS("?=")} };
							const std::array<utility::Static_String, 1> m_1{ {SS("||")} };
							const std::array<utility::Static_String, 1> m_2{ {SS("&&")} };
							const std::array<utility::Static_String, 1> m_3{ {SS("|")} };
							const std::array<utility::Static_String, 1> m_4{ {SS("&")} };
							const std::array<utility::Static_String, 3> m_5{ {SS("=="), SS("!="), SS("..")} };
							const std::array<utility::Static_String, 4> m_6{ {SS("<"), SS("<="), SS(">"), SS(">=")} };
							const std::array<utility::Static_String, 2> m_7{ {SS("<<"), SS(">>")} };
							const std::array<utility::Static_String, 2> m_8{ {SS("+"), SS("-")} };
							const std::array<utility::Static_String, 3> m_9{ {SS("*"), SS("/"), SS("%")} };
							const std::array<utility::Static_String, 1> m_10{ {SS("^")} };
							const std::array<utility::Static_String, 6> m_11{ SS("++"), SS("--"), SS("-"), SS("+"), SS("!"), SS("~") };
						};
						static auto const& Data() {
							static Operator_Matches_Impl out;
							return out;
						};
						static bool is_match(GL::string t_str) noexcept {
							constexpr std::array<std::size_t, 12> groups{ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } };
							return std::any_of(groups.begin(), groups.end(), [&t_str](const std::size_t group) { return is_match(group, t_str); });
						};
						template<typename Predicate> static bool any_of(const std::size_t t_group, Predicate&& predicate) {
							auto match = [&predicate](const auto& array) { return std::any_of(array.begin(), array.end(), predicate); };

							switch (t_group) {
							case 0:
								return match(Data().m_0);
							case 1:
								return match(Data().m_1);
							case 2:
								return match(Data().m_2);
							case 3:
								return match(Data().m_3);
							case 4:
								return match(Data().m_4);
							case 5:
								return match(Data().m_5);
							case 6:
								return match(Data().m_6);
							case 7:
								return match(Data().m_7);
							case 8:
								return match(Data().m_8);
							case 9:
								return match(Data().m_9);
							case 10:
								return match(Data().m_10);
							case 11:
								return match(Data().m_11);
							default:
								return false;
							}
						}
						static bool is_match(const std::size_t t_group, GL::string t_str)  noexcept {
							auto match = [&t_str](const auto& array) {
								return std::any_of(array.begin(), array.end(), [&t_str](const auto& v) { return std::string_view(v) == t_str; });
							};

							switch (t_group) {
							case 0:
								return match(Data().m_0);
							case 1:
								return match(Data().m_1);
							case 2:
								return match(Data().m_2);
							case 3:
								return match(Data().m_3);
							case 4:
								return match(Data().m_4);
							case 5:
								return match(Data().m_5);
							case 6:
								return match(Data().m_6);
							case 7:
								return match(Data().m_7);
							case 8:
								return match(Data().m_8);
							case 9:
								return match(Data().m_9);
							case 10:
								return match(Data().m_10);
							case 11:
								return match(Data().m_11);
							default:
								return false;
							}
						}
					};
					static std::array<Engine::Operator_Precedence, 12> build_operators() noexcept {
						return std::array<Engine::Operator_Precedence, 12>{
							{
								Engine::Operator_Precedence::Ternary_Cond,
								Engine::Operator_Precedence::Logical_Or,
								Engine::Operator_Precedence::Logical_And,
								Engine::Operator_Precedence::Bitwise_Or,
								Engine::Operator_Precedence::Bitwise_And,
								Engine::Operator_Precedence::Equality,
								Engine::Operator_Precedence::Comparison,
								Engine::Operator_Precedence::Shift,
								Engine::Operator_Precedence::Addition,
								Engine::Operator_Precedence::Multiplication,
								Engine::Operator_Precedence::Bitwise_Xor,
								Engine::Operator_Precedence::Prefix
							}
						};
					};
					static std::array<Engine::Operator_Precedence, 12> const& operators() noexcept {
						static auto out{ build_operators() };
						return out;
					};

					constexpr bool char_in_alphabet(char c, Engine::Alphabet a) const noexcept { return alphabet()[a][static_cast<uint8_t>(c)]; } // test a char in an m_alphabet

				public:
					using ParseNode = std::pair< std::shared_ptr<AST_Node_Impl>, GL::scope::impl::BasicScope* >;
				private:
					Engine::Position m_position{};
					std::vector<ParseNode> m_match_stack;
					std::vector<ParseNode> m_comment_stack;

				private:
					// check if the string is a valid operator
					static bool is_operator(GL::string t_s) noexcept { return Operator_Matches::is_match(t_s); }
					static void validate_object_name(GL::string const& name, Engine::Position const& m_position) {
						switch (Engine::hash(name.c_str())) {
						case Engine::hash(""):
							throw exception::eval_error("Id names cannot be empty", m_position);
						case Engine::hash("#define"):
						case Engine::hash("#undef"):
						case Engine::hash("#ifdef"):
						case Engine::hash("#ifndef"):
						case Engine::hash("#elif"):
						case Engine::hash("#else"):
						case Engine::hash("#endif"):
						case Engine::hash("#error"):
						case Engine::hash("#warning"):
						case Engine::hash("#include"):
						case Engine::hash("#pragma"):
						case Engine::hash("auto"):
						// case Engine::hash("var"):
						case Engine::hash("global"):
						case Engine::hash("while"):
						case Engine::hash("for"):
						case Engine::hash("parallel_for"):
						case Engine::hash("break"):
						case Engine::hash("continue"):
						case Engine::hash("case"):
						case Engine::hash("default"):
						case Engine::hash("switch"):
						case Engine::hash("try"):
						case Engine::hash("catch"):
						case Engine::hash("finally"):
						case Engine::hash("do"):
						case Engine::hash("evaluate"):
						case Engine::hash("namespace"):
						case Engine::hash("class"):
						case Engine::hash("return"):
						case Engine::hash("if"):
						case Engine::hash("else"):
						{
							GL::string temp = GL::string(name);
							throw exception::eval_error("Id name '"+ temp +"' was reserved for the langauge", m_position);
						}
						default:
							return;
						}
					};

				private:
					/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
					bool Symbol_(const utility::Static_String& sym) noexcept {
						const auto len = sym.size();
						if (m_position.remaining() >= len) {
							const char* file_pos = &(*m_position);
							for (size_t pos = 0; pos < len; ++pos) {
								if (sym.c_str()[pos] != file_pos[pos]) {
									return false;
								}
							}
							m_position += len;
							return true;
						}
						return false;
					};
					/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
					bool Symbol_(const GL::string& sym) noexcept {
						const auto len = sym.size();
						if (m_position.remaining() >= len) {
							const char* file_pos = &(*m_position);
							for (size_t pos = 0; pos < len; ++pos) {
								if (sym[pos] != file_pos[pos]) {
									return false;
								}
							}
							m_position += len;
							return true;
						}
						return false;
					};
					/// Reads a char from input if it matches the parameter, without skipping initial whitespace
					bool Char_(const char c) {
						if (m_position.has_more() && (*m_position == c)) {
							++m_position;
							return true;
						}
						else {
							return false;
						}
					};
					/// Reads an end-of-line group from input, without skipping initial whitespace
					bool Eol_(const bool t_eos = false) {
						bool retval = false;

						if (m_position.has_more() && (Symbol_(m_cr_lf) || Char_('\n'))) {
							retval = true;
							//++m_position.line;
							m_position.col = 1;
						}
						else if (m_position.has_more() && !t_eos && Char_(';')) {
							retval = true;
						}

						return retval;
					};
					/// Reads a string from input if it matches the parameter, without skipping initial whitespace
					bool Keyword_(const utility::Static_String& t_s) {
						const auto len = t_s.size();
						if (m_position.remaining() >= len) {
							auto tmp = m_position;
							for (size_t i = 0; tmp.has_more() && i < len; ++i) {
								if (*tmp != t_s.c_str()[i]) {
									return false;
								}
								++tmp;
							}
							m_position = tmp;
							return true;
						}

						return false;
					};
					/// Reads the optional exponent (scientific notation) and suffix for a Float, without skipping initial whitespace
					/// Support a form of scientific notation: 1e-5, 35.5E+8, 0.01e19
					bool read_exponent_and_suffix_() noexcept {
						// Support a form of scientific notation: 1e-5, 35.5E+8, 0.01e19
						if (m_position.has_more() && (std::tolower(*m_position) == 'e')) {
							++m_position;
							if (m_position.has_more() && ((*m_position == '-') || (*m_position == '+'))) {
								++m_position;
							}
							auto exponent_pos = m_position;
							while (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_alphabet)) {
								++m_position;
							}
							if (m_position == exponent_pos) {
								// Require at least one digit after the exponent
								return false;
							}
						}

						// Parse optional float suffix
						while (m_position.has_more() && char_in_alphabet(*m_position, Engine::float_suffix_alphabet)) {
							++m_position;
						}

						return true;
					};
					/// Reads a floating point value from input, without skipping initial whitespace
					bool Float_() noexcept {
						if (m_position.has_more() && char_in_alphabet(*m_position, Engine::float_alphabet)) {
							while (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_alphabet)) {
								++m_position;
							}

							if (m_position.has_more() && (std::tolower(*m_position) == 'e')) {
								// The exponent is valid even without any decimal in the Float (1e8, 3e-15)
								return read_exponent_and_suffix_();
							}
							else if (m_position.has_more() && (*m_position == '.')) {
								++m_position;
								if (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_alphabet)) {
									while (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_alphabet)) {
										++m_position;
									}
									// After any decimal digits, support an optional exponent (3.7e3)
									return read_exponent_and_suffix_();
								}
								else {
									--m_position;
								}
							}
						}
						return false;
					};
					/// Reads a hex value from input, without skipping initial whitespace
					bool Hex_() noexcept {
						if (m_position.has_more() && (*m_position == '0')) {
							++m_position;

							if (m_position.has_more() && char_in_alphabet(*m_position, Engine::x_alphabet)) {
								++m_position;
								if (m_position.has_more() && char_in_alphabet(*m_position, Engine::hex_alphabet)) {
									while (m_position.has_more() && char_in_alphabet(*m_position, Engine::hex_alphabet)) {
										++m_position;
									}
									while (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_suffix_alphabet)) {
										++m_position;
									}

									return true;
								}
								else {
									--m_position;
								}
							}
							else {
								--m_position;
							}
						}

						return false;
					}
					/// Reads an integer suffix, without skipping initial whitespace
					void IntSuffix_() {
						while (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_suffix_alphabet)) {
							++m_position;
						}
					}
					/// Reads a binary value from input, without skipping initial whitespace
					bool Binary_() {
						if (m_position.has_more() && (*m_position == '0')) {
							++m_position;

							if (m_position.has_more() && char_in_alphabet(*m_position, Engine::b_alphabet)) {
								++m_position;
								if (m_position.has_more() && char_in_alphabet(*m_position, Engine::bin_alphabet)) {
									while (m_position.has_more() && char_in_alphabet(*m_position, Engine::bin_alphabet)) {
										++m_position;
									}
									return true;
								}
								else {
									--m_position;
								}
							}
							else {
								--m_position;
							}
						}

						return false;
					};
					template<typename T> constexpr static auto parse_num_(const GL::string t_str) noexcept -> typename std::enable_if<std::is_integral<T>::value, T>::type {
						T t = 0;
						for (const auto c : t_str) {
							if (c < '0' || c > '9') {
								return t;
							}
							t *= 10;
							t += c - '0';
						}
						return t;
					};
					template<typename T> static auto parse_num_(const GL::string t_str) -> typename std::enable_if<!std::is_integral<T>::value, T>::type {
						T t = 0;
						T base{};
						T decimal_place = 0;
						int exponent = 0;

						for (const auto c : t_str) {
							switch (c) {
							case '.':
								decimal_place = 10;
								break;
							case 'e':
							case 'E':
								exponent = 1;
								decimal_place = 0;
								base = t;
								t = 0;
								break;
							case '-':
								exponent = -1;
								break;
							case '+':
								break;
							case '0':
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7':
							case '8':
							case '9':
								if (decimal_place < 10) {
									t *= 10;
									t += static_cast<T>(c - '0');
								}
								else {
									t += static_cast<T>(c - '0') / decimal_place;
									decimal_place *= 10;
								}
								break;
							default:
								break;
							}
						}
						return exponent ? base * std::pow(T(10), t * static_cast<T>(exponent)) : t;
					};
					/// Parses a floating point value
					static GL::value buildFloat(GL::string t_val) {
						bool float_ = false;
						bool long_ = false;

						auto i = t_val.size();

						for (; i > 0; --i) {
							char val = t_val[i - 1];

							if (val == 'f' || val == 'F') {
								float_ = true;
							}
							else if (val == 'l' || val == 'L') {
								long_ = true;
							}
							else {
								break;
							}
						}

						if (float_) {
							return GL::value((float)parse_num_<float>(t_val.substr(0, i)));
						}
						else if (long_) {
							return GL::value((float)parse_num_<long double>(t_val.substr(0, i)));
						}
						else {
							return GL::value((float)parse_num_<double>(t_val.substr(0, i)));
						}
					}
					/// Parses a integer value and returns a wrapped representation of it
					static GL::value buildInt(const int base, GL::string t_val, const bool prefixed) {
						bool unsigned_ = false;
						bool long_ = false;
						bool longlong_ = false;

						auto i = t_val.size();

						for (; i > 0; --i) {
							const char val = t_val[i - 1];

							if (val == 'u' || val == 'U') {
								unsigned_ = true;
							}
							else if (val == 'l' || val == 'L') {
								if (long_) {
									longlong_ = true;
								}

								long_ = true;
							}
							else {
								break;
							}
						}

						if (prefixed) {
							t_val.remove_prefix(2);
						}

						try {
							/// TODO fix this to use from_chars
							auto u = std::stoll(t_val.to_string(), nullptr, base);

							if (!unsigned_ && !long_ && u >= std::numeric_limits<int>::min() && u <= std::numeric_limits<int>::max()) {
								return (float)static_cast<int>(u);
							}
							else if ((unsigned_ || base != 10) && !long_ && u >= std::numeric_limits<unsigned int>::min()
								&& u <= std::numeric_limits<unsigned int>::max()) {
								return (float)static_cast<unsigned int>(u);
							}
							else if (!unsigned_ && !longlong_ && u >= std::numeric_limits<long>::min() && u <= std::numeric_limits<long>::max()) {
								return (float)static_cast<long>(u);
							}
							else if ((unsigned_ || base != 10) && !longlong_ && u >= std::numeric_limits<unsigned long>::min()
								&& u <= std::numeric_limits<unsigned long>::max()) {
								return (float)static_cast<unsigned long>(u);
							}
							else if (!unsigned_ && u >= std::numeric_limits<long long>::min() && u <= std::numeric_limits<long long>::max()) {
								return (float)static_cast<long long>(u);
							}
							else {
								return (float)static_cast<unsigned long long>(u);
							}
						}
						catch (const std::out_of_range&) {
							// too big to be signed
							try {
								/// TODO fix this to use from_chars
								auto u = std::stoull(t_val.to_string(), nullptr, base);

								if (!longlong_ && u >= std::numeric_limits<unsigned long>::min() && u <= std::numeric_limits<unsigned long>::max()) {
									return (float)static_cast<unsigned long>(u);
								}
								else {
									return (float)static_cast<unsigned long long>(u);
								}
							}
							catch (const std::out_of_range&) {
								// it's just simply too big
								return (float)std::numeric_limits<long long>::max();
							}
						}
					}
					/// Reads an identifier from input which conforms to C's identifier naming conventions, without skipping initial whitespace
					bool Id_(GL::string* out = nullptr) {
						const auto start = m_position;
						if (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) {
							while (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) { //keyword_alphabet)) {
								++m_position;
							}
							if (out) *out = Engine::Position::str(start, m_position);
							return true;
						}
						else if (m_position.has_more() && (*m_position == '`')) {
							++m_position;
							const auto start = m_position;

							while (m_position.has_more() && (*m_position != '`')) {
								if (Eol()) {
									throw exception::eval_error("Carriage return in identifier literal", m_position);
								}
								else {
									++m_position;
								}
							}

							if (start == m_position) {
								throw exception::eval_error("Missing contents of identifier literal", m_position);
							}
							else if (!m_position.has_more()) {
								throw exception::eval_error("Incomplete identifier literal", m_position);
							}

							++m_position;
							if (out) *out = Engine::Position::str(start, m_position);
							return true;
						}
						return false;
					};
					/// Reads a quoted string from input, without skipping initial whitespace
					bool Quoted_String_() {
						if (m_position.has_more() && (*m_position == '\"')) {
							char prev_char = *m_position;
							++m_position;

							int in_interpolation = 0;
							bool in_quote = false;

							while (m_position.has_more() && ((*m_position != '\"') || (in_interpolation > 0) || (prev_char == '\\'))) {
								if (!Eol_()) {
									if (prev_char == '$' && *m_position == '{') {
										++in_interpolation;
									}
									else if (prev_char != '\\' && *m_position == '"') {
										in_quote = !in_quote;
									}
									else if (*m_position == '}' && !in_quote) {
										--in_interpolation;
									}

									if (prev_char == '\\') {
										prev_char = 0;
									}
									else {
										prev_char = *m_position;
									}
									++m_position;
								}
							}

							if (m_position.has_more()) {
								++m_position;
							}
							else {
								throw exception::eval_error("Unclosed quoted string", m_position);
							}

							return true;
						}
						return false;
					};
					/// Reads (and potentially captures) a number from the input, detecting if it's an integer or floating point, without skipping initial whitespace
					bool Num_() {
						const auto start = m_position;
						if (m_position.has_more() && char_in_alphabet(*m_position, Engine::float_alphabet)) {
							try {
								if (Hex_()) {
									auto match = Engine::Position::str(start, m_position);
									auto bv = buildInt(16, match, true);
									m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									return true;
								}
								else if (Binary_()) {
									auto match = Engine::Position::str(start, m_position);
									auto bv = buildInt(2, match, true);
									m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									return true;
								}
								else if (Float_()) {
									auto match = Engine::Position::str(start, m_position);
									auto bv = buildFloat(match);
									m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									return true;
								}
								else {
									IntSuffix_();
									auto match = Engine::Position::str(start, m_position);
									if (!match.empty() && (match[0] == '0')) {
										auto bv = buildInt(8, match, false);
										m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									}
									else if (!match.empty()) {
										auto bv = buildInt(10, match, false);
										m_match_stack.push_back(ParseNode{ make_const(match, start, bv), nullptr });
									}
									else {
										return false;
									}
									return true;
								}
							}
							catch (const std::invalid_argument&) {
								// error parsing number passed in to buildFloat/buildInt
								return false;
							}
						}
						else {
							return false;
						}
					};

				private:
					/// Helper function that collects ast_nodes from a starting position to the top of the stack into a new AST node
					template<typename NodeType> std::shared_ptr<NodeType> build_match(GL::scope::impl::BasicScope* currentScope, size_t t_match_start, GL::string t_text = "") {
						bool is_deep = false;

						Engine::Parse_Location filepos = [&]() -> Engine::Parse_Location {
							// so we want to take everything to the right of this and make them children
							if (t_match_start != m_match_stack.size()) {
								is_deep = true;
								return Engine::Parse_Location(
									m_match_stack[t_match_start].first->location.start,
									m_position);
							}
							else {
								return Engine::Parse_Location(m_position, m_position);
							}
						}();

						std::vector<ParseNode> new_children_ParseNodes;
						std::vector<AST_Node_Impl_Ptr> new_children;
						if (is_deep) {
							new_children_ParseNodes.assign(std::make_move_iterator(m_match_stack.begin() + static_cast<int>(t_match_start)),
								std::make_move_iterator(m_match_stack.end()));
							m_match_stack.erase(m_match_stack.begin() + static_cast<int>(t_match_start), m_match_stack.end());
						}
						for (auto& x : new_children_ParseNodes) {
							new_children.push_back(std::dynamic_pointer_cast<AST_Node_Impl>(x.first));
						}

						auto ptr = optimizer::optimize(std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<NodeType>(
							currentScope
							, std::move(t_text)
							, std::move(filepos)
							, std::move(new_children)
							)), currentScope);
						m_match_stack.push_back(ParseNode{ ptr, currentScope });
						return std::dynamic_pointer_cast<NodeType>(ptr);
					};
					/// create a node
					template<typename T, typename... Param> std::shared_ptr<AST_Node_Impl> make_node(GL::scope::impl::BasicScope* currentScope, GL::string t_match, Engine::Position t_prev, Param &&...param) {
						auto out = std::make_shared<T>(
							currentScope,
							GL::string(t_match),
							Engine::Parse_Location(t_prev, m_position),
							std::forward<Param>(param)...
							);
						return std::dynamic_pointer_cast<AST_Node_Impl>(out);
					};
					/// create a node
					template<typename... Param> std::shared_ptr<AST_Nodes::Constant_AST_Node> make_const(GL::string t_match, Engine::Position t_prev, Param &&...param) {
						return std::make_shared<AST_Nodes::Constant_AST_Node>(
							GL::string(t_match),
							Engine::Parse_Location(t_prev, m_position),
							std::forward<Param>(param)...
							);
					};

				private:
					/// Skips (and potentially captures w/ nullptr scope) any multi-line or single-line comment
					bool SkipComment() {
						const auto start = m_position;
						if (Symbol_(m_multiline_comment_begin)) {
							while (m_position.has_more()) {
								if (Symbol_(m_multiline_comment_end)) {
									break;
								}
								else if (!Eol_()) {
									++m_position;
								}
							}
							GL::string comment = Engine::Position::str(start, m_position);
							auto parseLoc = Engine::Parse_Location(start, m_position);
							m_comment_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(nullptr, GL::string(comment), parseLoc), nullptr });

							return true;
						}
						else if (Symbol_(m_singleline_comment)) {
							while (m_position.has_more()) {
								if (Symbol_(m_cr_lf)) {
									m_position -= 2;
									break;
								}
								else if (Char_('\n')) {
									--m_position;
									break;
								}
								else {
									++m_position;
								}
							}

							GL::string comment = Engine::Position::str(start, m_position);
							auto parseLoc = Engine::Parse_Location(start, m_position);
							m_comment_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(nullptr, GL::string(comment), parseLoc), nullptr });

							return true;

						}
						else if (Symbol_(m_annotation)) {
							while (m_position.has_more()) {
								if (Symbol_(m_cr_lf)) {
									m_position -= 2;
									break;
								}
								else if (Char_('\n')) {
									--m_position;
									break;
								}
								else {
									++m_position;
								}
							}
							GL::string comment = Engine::Position::str(start, m_position);
							auto parseLoc = Engine::Parse_Location(start, m_position);
							m_comment_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(nullptr, GL::string(comment), parseLoc), nullptr });

							return true;
						}
						return false;
					}
					/// Skips whitespace, which means space and tab, but not cr/lf
					/// jespada: Modified SkipWS to skip optionally CR ('\n') and/or LF+CR ("\r\n")
					/// AlekMosingiewicz: Added exception when illegal character detected
					bool SkipWS(bool skip_cr = false) {
						bool retval = false;

						while (m_position.has_more()) {
							if (static_cast<unsigned char>(*m_position) > 0x7e) {
								throw exception::eval_error("Illegal character", m_position);
							}
							auto end_line = (*m_position != 0) && ((*m_position == '\n') || (*m_position == '\r' && *(m_position + 1) == '\n'));

							if (char_in_alphabet(*m_position, Engine::white_alphabet) || (skip_cr && end_line)) {
								if (end_line) {
									if (*m_position == '\r') {
										// discards lf
										++m_position;
									}
								}

								++m_position;

								retval = true;
							}
							else if (SkipComment()) {
								retval = true;
							}
							else {
								break;
							}
						}
						return retval;
					};
					/// Reads until the end of the current statement
					bool Eos() {
						SkipWS();
						return Eol_(true);
					};
					/// Reads (and potentially captures) an end-of-line group from input
					bool Eol() {
						SkipWS();
						return Eol_();
					};
					/// Reads (and potentially captures) a char from input if it matches the parameter
					bool Char(const char t_c) {
						SkipWS();
						return Char_(t_c);
					};
					/// Reads (and potentially captures) a string from input if it matches the parameter
					bool Keyword(const utility::Static_String& t_s) {
						SkipWS();
						const auto start = m_position;
						bool retval = Keyword_(t_s);
						// ignore substring matches
						if (retval && m_position.has_more() && char_in_alphabet(*m_position, Engine::keyword_alphabet)) {
							m_position = start;
							retval = false;
						}
						return retval;
					};
					/// Reads (and potentially captures) a symbol group from input if it matches the parameter
					bool Symbol(const GL::string& t_s, const bool t_disallow_prevention = false) {
						SkipWS();
						const auto start = m_position;
						bool retval = Symbol_(t_s);

						// ignore substring matches
						if (retval && m_position.has_more() && (t_disallow_prevention == false) && char_in_alphabet(*m_position, Engine::symbol_alphabet)) {
							if (*m_position != '=' && is_operator(Engine::Position::str(start, m_position)) && !is_operator(Engine::Position::str(start, m_position + 1))) {
								// don't throw this away, it's a good match and the next is not
							}
							else {
								m_position = start;
								retval = false;
							}
						}
						return retval;
					}
					/// Reads (and potentially captures) a number from the input, detecting if it's an integer or floating point
					bool Num() {
						SkipWS();
						return Num_();
					};
					/// 
					bool Operator_Helper(const size_t t_precedence, GL::string& oper) {
						return Operator_Matches::any_of(t_precedence, [&oper, this](const auto& elem) {
							if (Symbol(std::string_view(elem))) {
								oper = std::string_view(elem);
								return true;
							}
							else {
								return false;
							}
							});
					};
					/// Reads (and potentially captures) a quoted string from input.  Translates escaped sequences.
					bool Quoted_String(GL::scope::impl::BasicScope* currentScope) {
						SkipWS();

						const auto start = m_position;

						if (Quoted_String_()) {
							GL::string match;
							const auto prev_stack_top = m_match_stack.size();

							bool is_interpolated = [&]() -> bool {
								Char_Parser<GL::string> cparser(match, true);

								auto s = start + 1, end = m_position - 1;

								while (s != end) {
									if (cparser.saw_interpolation_marker) {
										if (*s == '{') {
											// We've found an interpolation point

											m_match_stack.push_back({ make_const(match, start, match), nullptr });

											if (cparser.is_interpolated) {
												// If we've seen previous interpolation, add on instead of making a new one
												build_match<AST_Nodes::Binary_Operator_AST_Node>(currentScope, prev_stack_top, "+");
											}

											// We've finished with the part of the string up to this point, so clear it
											match = "";

											GL::string eval_match;

											++s;
											while ((s != end) && (*s != '}')) {
												eval_match = eval_match + std::string(1, *s);
												++s;
											}

											if (*s == '}') {
												cparser.is_interpolated = true;
												++s;

												const auto tostr_stack_top = m_match_stack.size();

												m_match_stack.push_back({ make_node<AST_Nodes::FunctionName_AST_Node>(currentScope, "to_string", start), currentScope }); //  Id_AST_Node

												const auto ev_stack_top = m_match_stack.size();

												try {
													m_match_stack.push_back(parse_instr_eval(eval_match, currentScope));
												}
												catch (const exception::eval_error& e) {
													throw exception::eval_error(std::string(e.what()), start);
												}

												build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, ev_stack_top);
												build_match<AST_Nodes::Fun_Call_AST_Node>(currentScope, tostr_stack_top);
												build_match<AST_Nodes::Binary_Operator_AST_Node>(currentScope, prev_stack_top, "+");
											}
											else {
												throw exception::eval_error("Unclosed in-string eval", start);
											}
										}
										else {
											match = match + "$";
										}
										cparser.saw_interpolation_marker = false;
									}
									else {
										cparser.parse(*s, start);

										++s;
									}
								}

								if (cparser.saw_interpolation_marker) {
									match = match + "$";
								}

								return cparser.is_interpolated;
							}();

							m_match_stack.push_back({ make_const(match, start, match), nullptr });

							if (is_interpolated) {
								build_match<AST_Nodes::Binary_Operator_AST_Node>(currentScope, prev_stack_top, "+");
							}

							return true;
						}
						else {
							return false;
						}
					};
					/// Reads (and potentially captures) an identifier from input
					bool Id(const bool validate, GL::scope::impl::BasicScope* currentScope, AST_Nodes::IdType T) {
						SkipWS();
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						auto failure = [&]() {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
							return false;
						};

						if (T == AST_Nodes::IdType::Class) {
							GL::string className;

							auto Success = [&](bool Const, bool Ref) -> bool {
								auto classT = currentScope->DetermineType(className);
								if (auto* BC = currentScope->GetRoot()->try_find_class(classT)) {
									auto& Class = *dynamic_cast<GL::scope::impl::ClassScope*>(BC->this_m.scope);

									auto ptr = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(make_node<AST_Nodes::ClassName_AST_Node>(currentScope, className, prev_pos));
									if (Const && Ref) {
										ptr->TypeInfo = classT | GL::type::Const | GL::type::Reference;
									}
									else if (Const && !Ref) {
										ptr->TypeInfo = classT | GL::type::Const;
									}
									else if (!Const && Ref) {
										ptr->TypeInfo = classT | GL::type::Reference;
									}
									else {
										ptr->TypeInfo = classT;
									}
									m_match_stack.push_back({ std::dynamic_pointer_cast<AST_Node_Impl>(ptr), currentScope }); // e.g. "x", "Units::meter", etc.
									return true;
								}
								return failure();
							};

							// valid arrangements: 
							//     typename
							//     const typename
							//     typename&
							//     const typename&
							//     typename const
							//     typename const&
							//     const& typename
							if (Keyword("const")) {
								if (Char('&')) {
									SkipWS();
									if (Id_(&className)) {
										// const& typename
										return Success(true, true);
									}
								}
								else if (Id_(&className)) {
									const auto prev_pos_temp = m_position;
									if (Char('&')) {
										// const typename&
										return Success(true, true);
									}
									else {
										m_position = prev_pos_temp;
										// const typename
										return Success(true, false);
									}
								}
							}
							else if (Id_(&className)) {
								auto prev_pos_temp = m_position;
								//     typename
								//     typename&
								//     typename const
								//     typename const&
								if (Keyword("const")) {
									prev_pos_temp = m_position;
									if (Char('&')) {
										//     typename const&
										return Success(true, true);
									}
									else {
										m_position = prev_pos_temp;
										//     typename const
										return Success(true, false);
									}
								}
								else if (Char('&')) {
									//     typename&
									return Success(false, true);
								}
								else {
									m_position = prev_pos_temp;
									//     typename
									return Success(false, false);
								}
							}
							return failure();

						}
						else {
							if (Id_()) {
								GL::string text = Engine::Position::str(prev_pos, m_position);
								if (validate) { validate_object_name(text, m_position); }

								auto foundConstant = constants().find(text);
								if (foundConstant != constants().end()) {
									if (AST_Nodes::IdType::Class == T) throw exception::eval_error("Cannot use constant value \""+text+"\" as a class or type name", m_position);
									m_match_stack.push_back({ make_const(text, prev_pos, foundConstant->second), nullptr });
								}
								else {
									switch (Engine::hash(text.c_str())) {
									case Engine::hash("__LINE__"): {
										if (AST_Nodes::IdType::Class == T) throw exception::eval_error("Cannot use constant value \""+ text +"\" as a class or type name", m_position);
										m_match_stack.push_back({ make_const(text, prev_pos, const_var(prev_pos.line)), nullptr });
									} break;
										//case hash("__FILE__"): {
										//	m_match_stack.push_back(make_node<eval::Constant_AST_Node>(currentScope, text, prev_pos.line, prev_pos.col, const_var(m_filename)));
										//} break;
									default: {
										auto val = text;
										if (*prev_pos == '`') { // 'escaped' literal, like an operator name ( e.g. `[]`(...) )
											val = Engine::Position::str(prev_pos + 1, m_position - 1);
										}
										if (1) {
											switch (T) {
											default:
												m_match_stack.push_back({ make_node<AST_Nodes::Id_AST_Node>(currentScope, val, prev_pos), currentScope }); // e.g. "x", "Units::meter", etc.
												break;
											case AST_Nodes::IdType::Function:
												m_match_stack.push_back({ make_node<AST_Nodes::FunctionName_AST_Node>(currentScope, val, prev_pos), currentScope }); // e.g. "x", "Units::meter", etc.
												break;
											case AST_Nodes::IdType::Variable:
												m_match_stack.push_back({ make_node<AST_Nodes::VariableName_AST_Node>(currentScope, val, prev_pos), currentScope }); // e.g. "x", "Units::meter", etc.
												break;
											case AST_Nodes::IdType::Class:
												m_match_stack.push_back({ make_node<AST_Nodes::ClassName_AST_Node>(currentScope, val, prev_pos), currentScope }); // e.g. "x", "Units::meter", etc.
												break;
											}
										}
									} break;
									}
								}
								return true;
							}
							else {
								return false;
							}
						}
					};
					/// Reads (and potentially captures) an type or class identifier from input
					bool TypeName(GL::scope::impl::BasicScope* currentScope, bool allowAuto = false) {
						if (Id(false, currentScope, AST_Nodes::IdType::Class)) {
							return true;
						}
						else if (allowAuto) {
							const auto prev_pos = m_position;
							if (Keyword("auto")) {
								auto ptr = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(make_node<AST_Nodes::ClassName_AST_Node>(currentScope, Engine::Position::str(prev_pos, m_position), prev_pos));
								ptr->TypeInfo = {};
								m_match_stack.push_back({ std::dynamic_pointer_cast<AST_Node_Impl>(ptr), currentScope }); // e.g. "x", "Units::meter", etc.
							}
							//else if (Keyword("var")) {
							//	auto ptr = std::dynamic_pointer_cast<AST_Nodes::ClassName_AST_Node>(make_node<AST_Nodes::ClassName_AST_Node>(currentScope, Engine::Position::str(prev_pos, m_position), prev_pos));
							//	ptr->TypeInfo = {};
							//	m_match_stack.push_back({ std::dynamic_pointer_cast<AST_Node_Impl>(ptr), currentScope }); // e.g. "x", "Units::meter", etc.
							//}
							else {
								return false;
							}
						}
						return false;
					};

					/// Reads an argument from input
					bool Arg(GL::scope::impl::BasicScope* currentScope, const bool t_type_allowed = true) {
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						auto failure = [&]() {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
							return false;
						};

						SkipWS();

						bool foundType = false;
						if (t_type_allowed) {
							foundType = TypeName(currentScope);
						}

						if (!Id(true, currentScope, AST_Nodes::IdType::Variable)) {
							return failure();
						}

						build_match<AST_Nodes::Arg_AST_Node>(currentScope, prev_stack_top);

						return true;
					};

					/// Reads a comma-separated list of values from input. Id's only, no types allowed
					bool Id_Arg_List(GL::scope::impl::BasicScope* currentScope) {
						SkipWS(true);

						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Arg(currentScope, false)) {
							retval = true;
							SkipWS(true);
							while (Char(',')) {
								SkipWS(true);
								if (!Arg(currentScope, false)) {
									throw exception::eval_error("Unexpected value in parameter list", m_position);
								}
							}
						}
						build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);

						SkipWS(true);

						return retval;
					};

					/// Reads a comma-separated list of values from input, for function declarations
					bool Decl_Arg_List(GL::scope::impl::BasicScope* currentScope) {
						SkipWS(true);

						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Arg(currentScope, true)) {
							retval = true;
							SkipWS(true);
							while (Char(',')) {
								SkipWS(true);
								if (!Arg(currentScope, true)) {
									throw exception::eval_error("Unexpected value in parameter list", m_position);
								}
							}
						}
						build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);

						SkipWS(true);

						return retval;
					};

					/// Reads a comma-separated list of values from input
					bool Arg_List(GL::scope::impl::BasicScope* currentScope, int maxNumArgs = std::numeric_limits<int>::max()) {
						SkipWS(true);
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Equation(currentScope)) {
							retval = true;
							SkipWS(true);
							while (((--maxNumArgs) > 0)) {
								SkipWS(true);
								if (!Char(',')) break;
								SkipWS(true);
								if (!Equation(currentScope)) {
									throw exception::eval_error("Unexpected value in parameter list", m_position);
								}
							}
						}

						build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);

						SkipWS(true);

						return retval;
					};

					/// Reads a C-style type-cast from input (e.g. (int)0.0f )
					bool TypeCastOperation(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						SkipWS(true);

						// (string)100
						if (Char('(') && TypeName(currentScope) && Char(')')) {
							if (Operator(currentScope)) {
								retval = true;
								build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top + 1);
								build_match<AST_Nodes::Fun_Call_AST_Node>(currentScope, prev_stack_top); // Id(fun name), Arg_List()
							}
						}

						if (!retval) {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
						}
						return retval;
					};

					/// Parses a string of binary equation operators
					bool Equation(GL::scope::impl::BasicScope* currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						using SS = utility::Static_String;

						if (TypeCastOperation(currentScope)) {
							return true;
						}

						if (Operator(currentScope)) {
							for (const auto& sym :
								{ SS{"="}, SS{":="}, SS{"?="}, SS{".."}, SS{"+="}, SS{"-="}, SS{"*="}, SS{"/="}, SS{"%="}, SS{"<<="}, SS{">>="}, SS{"&="}, SS{"^="}, SS{"|="} }) {
								if (Symbol(std::string_view(sym.c_str()), true)) {
									SkipWS(true);
									if (!Equation(currentScope)) {
										throw exception::eval_error("Incomplete equation", m_position);
									}

									build_match<AST_Nodes::Equation_AST_Node>(currentScope, prev_stack_top, std::string_view(sym.c_str()));
									return true;
								}
							}
							return true;
						}

						return false;
					};
					// int x;
					// int const& x;
					bool SpecifiedType_Var_Decl(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						if (TypeName(currentScope)) { // typename was specified and found
							build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top + 1); // {no_params}
							build_match<AST_Nodes::Fun_Call_AST_Node>(currentScope, prev_stack_top); // collapse all into a function call (i.e. int({no_params}))

							if (Id(true, currentScope, AST_Nodes::IdType::Variable)) {
								retval = true;
								build_match<AST_Nodes::Var_Decl_AST_Node>(currentScope, prev_stack_top + 1);  // var i;                           
							}

							if (retval) {
								// Fun_Call ("Typename()") , Id or Ref ("Variable name");
								build_match < AST_Nodes::Assign_Retroactively_AST_Node>(currentScope, prev_stack_top);
							}
						}
						if (!retval) {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
						}

						return retval;
					}

					/// Reads a variable declaration from input
					bool Var_Decl(GL::scope::impl::BasicScope* currentScope) {
						const bool t_namespace_context = currentScope->is_namespace();

						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();
						if (t_namespace_context) { // Classes and namespaces must explicitely define their variable types. They must not be left implicit e.g. auto or var
							return SpecifiedType_Var_Decl(currentScope);
						}
						else { // Normal scopes may utilize implicit or late-definition style variables if they so choose, for ease.
							if (Keyword("auto") /*|| Keyword("var")*/) {
								(void)Char('&');
								if (Id(true, currentScope, AST_Nodes::IdType::Variable)) {
									build_match<AST_Nodes::Var_Decl_AST_Node>(currentScope, prev_stack_top);
									retval = true;
								}
								else {
									throw exception::eval_error("Incomplete variable declaration ", m_position);
								}
							}
							else {
								return SpecifiedType_Var_Decl(currentScope);
							}
						}
						return retval;
					};

					/// Reads a unary prefixed expression from input
					bool Prefix(GL::scope::impl::BasicScope* currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						using SS = utility::Static_String;
						constexpr std::array<utility::Static_String, 6> prefix_opers{ SS{"++"}, SS{"--"}, SS{"-"}, SS{"+"}, SS{"!"}, SS{"~"} };
						for (const auto& oper : prefix_opers) {
							const bool is_char = oper.size() == 1;
							if ((is_char && Char(oper.c_str()[0])) || (!is_char && Symbol(std::string_view(oper.c_str())))) {
								if (!Operator(currentScope, operators().size() - 1)) {
									throw exception::eval_error("Incomplete prefix '" + GL::string(std::string_view(oper.c_str())) + "' expression", m_position);
								}
								build_match<AST_Nodes::Prefix_AST_Node>(currentScope, prev_stack_top, std::string_view(oper.c_str()));
								return true;
							}
						}
						return false;
					};

					static auto make_postfix_operators() {
						std::map<int, std::vector<std::pair<GL::type, std::pair<GL::value, GL::any::fast_any>>>, std::greater_equal<int>> out;
						//for (auto& unit_type : Units::value::GetValueTypes()) {
						//	auto abbreviation = GL::string("_") + GL::string(unit_type.second.first.UnitAbbreviation());
						//	out[abbreviation.length()].push_back(unit_type);
						//}
						return out;
					}

					bool Postfix(GL::scope::impl::BasicScope* currentScope, bool gotValueAlready) {
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						// add support for custom post-fixes
						// Examples: 
						// 12_in = inch(12)
						// 1_gal = gallon(1)

						if (gotValueAlready) {
							if (Symbol("++")) {
								build_match<AST_Nodes::Postfix_AST_Node>(currentScope, prev_stack_top - 1, "++");
								return true;
							}
							else if (Symbol("--")) {
								build_match<AST_Nodes::Postfix_AST_Node>(currentScope, prev_stack_top - 1, "--");
								return true;
							}
							else {
								static std::map<int, std::vector<std::pair<GL::type, std::pair<GL::value, GL::any::fast_any>>>, std::greater_equal<int>>
									customOperators{ make_postfix_operators() };

								// evaluate the custom operators...
								if (prev_stack_top > 0 && m_match_stack[prev_stack_top - 1].first->text != "" && m_match_stack[prev_stack_top - 1].first->identifier == Engine::AST_Node_Type::Constant) {
									// this path means the incoming value is constant and known			
#if 0
									for (auto& abbreviation_length_to_category : customOperators) {
										for (auto& unit_type : abbreviation_length_to_category.second) {
											auto abbreviation = GL::string("_") + GL::string(unit_type.second.first.UnitAbbreviation());
											if (Symbol(abbreviation)) {
												auto& rhs = std::dynamic_pointer_cast<AST_Nodes::Constant_AST_Node>(m_match_stack[prev_stack_top - 1].first)->m_value;
												Any lhs(unit_type.second.first);
												if (auto Class = currentScope->FindClass(unit_type.first.lock())) {
													if (auto ClassPtr = Class->this_m->scope_ptr) {
														lhs = ClassPtr->Call(Class->this_m->scope_name, { rhs });
													}
													else {
														currentScope->Call("=", { lhs, rhs });
													}
												}
												else {
													currentScope->Call("=", { lhs, rhs });
												}
												GL::string temp = GoodLang::ToString(lhs);

												Engine::Parse_Location loc = m_match_stack[prev_stack_top - 1].first->location;
												loc.end += abbreviation.length();

												m_match_stack[prev_stack_top - 1].first =
													std::dynamic_pointer_cast<AST_Node_Impl>(
														std::make_shared<AST_Nodes::Constant_AST_Node>(temp, loc, lhs)
														);

												return true;
											}
										}
									}
#endif
								}
								else if (prev_stack_top > 0 && m_match_stack[prev_stack_top - 1].first->text != "") {
									// this path means the incoming value is NOT constant and is not known. 
#if 0
									for (auto& abbreviation_length_to_category : customOperators) {
										for (auto& unit_type : abbreviation_length_to_category.second) {
											auto abbreviation = GL::string("_") + GL::string(unit_type.second.first.UnitAbbreviation());
											if (Symbol(abbreviation)) {
												Any lhs(unit_type.second.first);
												if (auto Class = currentScope->FindClass(unit_type.first.lock())) {
													if (auto ClassPtr = Class->this_m->scope_ptr) {
														lhs = ClassPtr->Call(Class->this_m->scope_name, {});
													}
												}

												Engine::Parse_Location loc = m_match_stack[prev_stack_top - 1].first->location;
												loc.end += abbreviation.length();

												m_match_stack[prev_stack_top - 1].first =
													std::dynamic_pointer_cast<AST_Node_Impl>(
														std::make_shared<AST_Nodes::Equation_AST_Node>(currentScope, "=", loc, std::vector<AST_Node_Impl_Ptr>{
													std::dynamic_pointer_cast<AST_Node_Impl>(
														std::make_shared<AST_Nodes::Constant_AST_Node>(GoodLang::ToString(lhs), loc, lhs)
														),
														std::move(m_match_stack[prev_stack_top - 1].first)
												})
														);

												return true;


												//// To-Do, finish this analysis!
												//// Insert a node that evaluates the function `=`(lhs, rhs) and returns lhs.







												//throw std::runtime_error("FIX ME!");
												//while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
												//m_position = prev_pos;


											}
										}
									}
#endif
								}
							}
						}
						else {
							if (Id(true, currentScope, AST_Nodes::IdType::Variable)) {
								if (Symbol("++")) {
									build_match<AST_Nodes::Postfix_AST_Node>(currentScope, prev_stack_top, "++");
									return true;
								}
								else if (Symbol("--")) {
									build_match<AST_Nodes::Postfix_AST_Node>(currentScope, prev_stack_top, "--");
									return true;
								}
								while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
								m_position = prev_pos;
							}
						}
						return false;
					}

					/// Reads a pair of values used to create a map initialization from input
					bool Map_Pair(GL::scope::impl::BasicScope* currentScope) {
						SkipWS(true);
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						if (Operator(currentScope)) {
							if (Symbol(":")) {
								retval = true;
								if (!Operator(currentScope)) { throw exception::eval_error("Incomplete map pair", m_position); }

								build_match<AST_Nodes::Map_Pair_AST_Node>(currentScope, prev_stack_top);
							}
							else {
								m_position = prev_pos;
								while (prev_stack_top != m_match_stack.size()) {
									m_match_stack.pop_back();
								}
							}
						}

						return retval;
					}

					/// Reads possible special container values, including ranges and map_pairs
					bool Container_Arg_List(GL::scope::impl::BasicScope* currentScope) {
						SkipWS(true);
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Map_Pair(currentScope)) {
							retval = true;
							SkipWS(true);
							while (Char(',')) {
								SkipWS(true);
								if (!Map_Pair(currentScope)) {
									throw exception::eval_error("Unexpected value in container", m_position);
								}
							}
							build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);
						}
						else if (Operator(currentScope)) {
							retval = true;
							SkipWS(true);
							while (Char(',')) {
								SkipWS(true);
								if (!Operator(currentScope)) {
									throw exception::eval_error("Unexpected value in container", m_position);
								}
								SkipWS(true);
							}
							build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);
						}

						SkipWS(true);

						return retval;
					}

					/// Reads, and identifies, a short-form container initialization from input
					bool Inline_Container(GL::scope::impl::BasicScope* currentScope) {
						const auto prev_stack_top = m_match_stack.size();

						if (Char('[')) {
							SkipWS(true);
							Container_Arg_List(currentScope);
							SkipWS(true);
							if (!Char(']')) {
								throw exception::eval_error("Missing closing square bracket ']' in container initializer", m_position);
							}
							if ((prev_stack_top != m_match_stack.size()) && (!m_match_stack.back().first->children.empty())) {
								if (m_match_stack.back().first->children[0]->identifier == Engine::AST_Node_Type::Map_Pair) {
									build_match<AST_Nodes::Inline_Map_AST_Node>(currentScope, prev_stack_top);
								}
								else {
									build_match<AST_Nodes::Inline_Array_AST_Node>(currentScope, prev_stack_top);
								}
							}
							else {
								build_match<AST_Nodes::Inline_Array_AST_Node>(currentScope, prev_stack_top);
							}

							return true;
						}
						else {
							return false;
						}
					}

					/// Reads a lambda (anonymous function) from input
					bool Lambda(GL::scope::impl::BasicScope* currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						auto failure = [&]() {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
							return false;
						};

						/* All of the following should be examples of valid forms of lambda functions */
						// [...](...) async -> typename {...} 
						// [...](...) -> typename {...} 
						// [...](...) async {...} 
						// (...) async {...} 
						// (...) {...} 

						// Arg_List
						if (Char('[')) {
							SkipWS(true);
							Id_Arg_List(currentScope);
							SkipWS(true);
							if (!Char(']')) {
								return failure();
							}
						}
						else {
							// make sure we always have the same number of nodes
							build_match<AST_Nodes::Arg_List_AST_Node>(currentScope, prev_stack_top);
						}

						// Arg_List
						if (Char('(')) {
							SkipWS(true);
							Decl_Arg_List(currentScope);
							SkipWS(true);
							if (!Char(')')) {
								return failure();
							}
						}
						else {
							return failure();
						}

						// KeyWords / modifiers
						bool is_async = false;
						bool foundKeyWord = true;
						while (foundKeyWord) {
							SkipWS(true);
							foundKeyWord = false;

							if (Keyword("async")) {
								foundKeyWord = true;
								is_async = true;
							}
						}

						SkipWS(true);

						// Noop or Id
						if (Symbol("->")) {
							SkipWS(true);
							const auto start = m_position;
							if (!TypeName(currentScope)) {
								return failure();
							}
						}
						else {
							// make sure we always have the same number of nodes
							m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
						}

						// Block
						SkipWS(true);
						if (!Block(currentScope)) {
							return failure();
						}

						auto lambda_node = build_match<AST_Nodes::Lambda_AST_Node>(currentScope, prev_stack_top);
						lambda_node->is_async = is_async;

						return true;
					};

					bool Dot_Fun_Array(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						if (Lambda(currentScope) || Num() || Quoted_String(currentScope) || Paren_Expression(currentScope) || Inline_Container(currentScope) || Id(false, currentScope, AST_Nodes::IdType::Variable)) {
							retval = true;
							bool has_more = true;

							while (has_more) {
								has_more = false;
								if (Char('(')) {
									has_more = true;
									SkipWS(true);
									Arg_List(currentScope);
									SkipWS(true);
									if (!Char(')')) {
										throw exception::eval_error("Incomplete function call", m_position);
									}

									build_match<AST_Nodes::Fun_Call_AST_Node>(currentScope, prev_stack_top, "()");
									/// \todo Work around for method calls until we have a better solution
									if (!m_match_stack.back().first->children.empty()) {
										if (m_match_stack.back().first->children[0]->identifier == Engine::AST_Node_Type::Dot_Access) {
											if (m_match_stack.empty()) {
												throw exception::eval_error("Incomplete dot access fun call", m_position);
											}
											if (m_match_stack.back().first->children.empty()) {
												throw exception::eval_error("Incomplete dot access fun call", m_position);
											}
											auto dot_access = std::move(m_match_stack.back().first->children[0]);
											auto func_call = std::move(m_match_stack.back());
											m_match_stack.pop_back();
											func_call.first->children.erase(func_call.first->children.begin());
											if (dot_access->children.empty()) {
												throw exception::eval_error("Incomplete dot access fun call", m_position);
											}
											func_call.first->children.insert(func_call.first->children.begin(), std::move(dot_access->children.back()));
											dot_access->children.pop_back();
											dot_access->children.push_back(std::move(func_call.first));
											if (dot_access->children.size() != 2) {
												throw exception::eval_error("Incomplete dot access fun call", m_position);
											}
											m_match_stack.push_back({ dot_access, func_call.second });
										}
									}
								}
								else if (Char('[')) {
									has_more = true;
									if (!(Operator(currentScope) && Char(']'))) {
										// TO-DO, Extend to allow matrix accessors, i.e. matrix_obj[0,0] = 10.0;
										throw exception::eval_error("Incomplete array access", m_position);
									}

									build_match<AST_Nodes::Array_Call_AST_Node>(currentScope, prev_stack_top, "[]");
								}
								else if (Symbol(".")) {
									has_more = true;
									if (!(Id(true, currentScope, AST_Nodes::IdType::Function))) {
										throw exception::eval_error("Incomplete dot access fun call", m_position);
									}

									if (std::distance(m_match_stack.begin() + static_cast<int>(prev_stack_top), m_match_stack.end()) != 2) {
										throw exception::eval_error("Incomplete dot access fun call", m_position);
									}

									build_match<AST_Nodes::Dot_Access_AST_Node>(currentScope, prev_stack_top, ".");
								}
								else if (Eol()) {
									auto start = (--m_position);
									SkipWS(true);
									if (Symbol(".")) {
										has_more = true;
										--m_position;
									}
									else {
										m_position = start;
									}
								}
							}
						}

						return retval;
					};

					/// Parses any of a group of 'value' style ast_node groups from input
					bool Value(GL::scope::impl::BasicScope* currentScope) {
						if (Var_Decl(currentScope) || Dot_Fun_Array(currentScope) || Prefix(currentScope)) {
							Postfix(currentScope, true);
							return true;
						}
						else {
							return Postfix(currentScope, false);
						}
					};

					bool Operator(GL::scope::impl::BasicScope* currentScope, const size_t t_precedence = 0) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						if (operators()[t_precedence] != Engine::Operator_Precedence::Prefix) {
							if (Operator(currentScope, t_precedence + 1)) {
								GL::string oper;
								retval = true;
								while (Operator_Helper(t_precedence, oper)) {
									while (Eol()) {}

									if (!Operator(currentScope, t_precedence + 1)) {
										throw exception::eval_error("Incomplete '" + oper + "' expression", m_position);
									}

									switch (operators()[t_precedence]) {
									case (Engine::Operator_Precedence::Ternary_Cond):
										if (oper == "?=") {
											build_match<AST_Nodes::Equation_AST_Node>(currentScope, prev_stack_top, oper);
										}
										else {
											if (Symbol(":")) {
												if (!Operator(currentScope, t_precedence + 1)) {
													throw exception::eval_error("Incomplete '" + oper + "' expression", m_position);
												}
												build_match<AST_Nodes::If_AST_Node>(currentScope, prev_stack_top);
											}
											else {
												throw exception::eval_error("Incomplete '" + oper + "' expression", m_position);
											}
										}
										break;

									case (Engine::Operator_Precedence::Addition):
									case (Engine::Operator_Precedence::Multiplication):
									case (Engine::Operator_Precedence::Shift):
									case (Engine::Operator_Precedence::Equality):
									case (Engine::Operator_Precedence::Bitwise_And):
									case (Engine::Operator_Precedence::Bitwise_Xor):
									case (Engine::Operator_Precedence::Bitwise_Or):
									case (Engine::Operator_Precedence::Comparison):
										build_match<AST_Nodes::Binary_Operator_AST_Node>(currentScope, prev_stack_top, oper);
										break;

									case (Engine::Operator_Precedence::Logical_And):
										build_match<AST_Nodes::Logical_And_AST_Node>(currentScope, prev_stack_top, oper);
										break;
									case (Engine::Operator_Precedence::Logical_Or):
										build_match<AST_Nodes::Logical_Or_AST_Node>(currentScope, prev_stack_top, oper);
										break;
									case (Engine::Operator_Precedence::Prefix):
										ASSERT(false); // cannot reach here because of if() statement at the top
										break;
									}
								}
							}
						}
						else {
							return Value(currentScope);
						}

						return retval;
					}

					/// Reads an expression surrounded by parentheses from input
					bool Paren_Expression(GL::scope::impl::BasicScope* currentScope) {
						if (Char('(')) {
							SkipWS(true);
							if (!Operator(currentScope)) {
								throw exception::eval_error("Incomplete expression", m_position);
							}
							SkipWS(true);
							if (!Char(')')) {
								throw exception::eval_error("Missing closing parenthesis ')'", m_position);
							}
							return true;
						}
						else {
							return false;
						}
					};

					/// Reads a while block from input
					bool While(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Keyword("while")) {
							retval = true;

							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'while' expression", m_position);
							}

							if (!(Operator(currentScope) && Char(')'))) {
								throw exception::eval_error("Incomplete 'while' expression", m_position);
							}

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'while' block", m_position);
							}

							build_match<AST_Nodes::While_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads the C-style `for` conditions from input
					bool For_Guards(GL::scope::impl::BasicScope* currentScope) {
						if (!(Equation(currentScope) && Eol())) {
							if (!Eol()) {
								return false;
							}
							else {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}
						}

						if (!(Equation(currentScope) && Eol())) {
							if (!Eol()) {
								return false;
							}
							else {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Constant_AST_Node>(GL::any::fast_any::instance(true)), nullptr });
							}
						}

						if (!Equation(currentScope)) {
							m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
						}

						return true;
					}

					/// Reads the C-style `for` conditions from input
					bool Parallel_For_Guards(GL::scope::impl::BasicScope* currentScope) {
						if (!(Equation(currentScope) && Eol())) {
							if (!Eol()) {
								return false;
							}
							else {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}
						}

						if (!(Equation(currentScope))) {
							m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Constant_AST_Node>(GL::any::fast_any::instance(true)), nullptr });
						}

						return true;
					}

					/// Reads the ranged `for` conditions from input
					bool Range_Expression(GL::scope::impl::BasicScope* currentScope) {
						// the first element will have already been captured by the For_Guards() call that preceeds it
						return Char(':') && Equation(currentScope);
					}

					/// Reads a for block from input
					bool For(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						if (Keyword("for")) {
							retval = true;

							SkipWS(true);

							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'for' expression", m_position);
							}

							SkipWS(true);

							bool classic_for = For_Guards(currentScope);
							SkipWS(true);
							if (classic_for) classic_for = classic_for && Char(')');
							if (!classic_for) {
								classic_for = Range_Expression(currentScope);
								SkipWS(true);
								if (classic_for) classic_for = classic_for && Char(')');

								if (!classic_for) {
									throw exception::eval_error("Incomplete 'for' expression", m_position);
								}

								classic_for = false;
							}

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'for' block", m_position);
							}

							const auto num_children = m_match_stack.size() - prev_stack_top;

							if (classic_for) {
								if (num_children != 4) {
									throw exception::eval_error("Incomplete 'for' expression", m_position);
								}
								build_match<AST_Nodes::For_AST_Node>(currentScope, prev_stack_top);
							}
							else {
								if (num_children != 3) {
									throw exception::eval_error("Incomplete ranged-for expression", m_position);
								}
								build_match<AST_Nodes::Ranged_For_AST_Node>(currentScope, prev_stack_top);
							}
						}
						else if (Keyword("parallel_for")) {
							// parallel_for (var x = START_VALUE ; END_VALUE) WORK_BLOCK; // this approach means every iteration will see it's own local "x"
							// parallel_for (START_VALUE ; END_VALUE) WORK_BLOCK // this approach means every iteration will NOT see any "x" at all
							// parallel_for (range_declaration : range_expression) loop_statement;
							retval = true;

							SkipWS(true);

							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'parallel_for' expression", m_position);
							}

							SkipWS(true);

							bool classic_for = Parallel_For_Guards(currentScope);
							SkipWS(true);
							if (classic_for) classic_for = classic_for && Char(')');
							if (!classic_for) {
								classic_for = Range_Expression(currentScope);
								SkipWS(true);
								if (classic_for) classic_for = classic_for && Char(')');

								if (!classic_for) {
									throw exception::eval_error("Incomplete 'parallel_for' expression", m_position);
								}

								classic_for = false;
							}

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'parallel_for' block", m_position);
							}

							const auto num_children = m_match_stack.size() - prev_stack_top;

							if (classic_for) {
								if (num_children != 3) {
									throw exception::eval_error("Incomplete 'parallel_for' expression", m_position);
								}
								build_match<AST_Nodes::Parallel_For_AST_Node>(currentScope, prev_stack_top);
							}
							else {
								if (num_children != 3) {
									throw exception::eval_error("Incomplete ranged-parallel_for expression", m_position);
								}
								build_match<AST_Nodes::Parallel_Ranged_For_AST_Node>(currentScope, prev_stack_top);
							}
						}

						return retval;
					}

					/// Reads a break statement from input
					bool Break(GL::scope::impl::BasicScope* currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("break")) {
							build_match<AST_Nodes::Break_AST_Node>(currentScope, prev_stack_top);
							return true;
						}
						else {
							return false;
						}
					}

					/// Reads a continue statement from input
					bool Continue(GL::scope::impl::BasicScope* currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("continue")) {
							build_match<AST_Nodes::Continue_AST_Node>(currentScope, prev_stack_top);
							return true;
						}
						else {
							return false;
						}
					}

					/// Reads a case block from input
					bool Case(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						// case "option": { ... }
						// case "option" { ... }
						if (Keyword("case")) {
							retval = true;

							SkipWS(true);

							if (!Operator(currentScope)) {
								throw exception::eval_error("Incomplete 'case' expression", m_position);
							}

							SkipWS(true);

							(void)Char(':'); // optional

							SkipWS(true);

							if (!Block(currentScope)) {


								throw exception::eval_error("Incomplete 'case' block", m_position);
							}

							build_match<AST_Nodes::Case_AST_Node>(currentScope, prev_stack_top);
						}
						// default: { ... }
						// default { ... }
						else if (Keyword("default")) {
							retval = true;

							SkipWS(true);

							(void)Char(':'); // optional

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'default' block", m_position);
							}

							build_match<AST_Nodes::Default_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads a switch statement from input
					bool Switch(GL::scope::impl::BasicScope* currentScope) {
						const auto prev_stack_top = m_match_stack.size();

						if (Keyword("switch")) {
							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'switch' expression", m_position);
							}

							if (!(Operator(currentScope) && Char(')'))) {
								throw exception::eval_error("Incomplete 'switch' expression", m_position);
							}

							SkipWS(true);

							if (Char('{')) {
								SkipWS(true);

								while (Case(currentScope)) {
									SkipWS(true);
								}

								SkipWS(true);

								if (!Char('}')) {
									throw exception::eval_error("Incomplete block", m_position);
								}
							}
							else {
								throw exception::eval_error("Incomplete block", m_position);
							}

							build_match<AST_Nodes::Switch_AST_Node>(currentScope, prev_stack_top);
							return true;

						}
						else {
							return false;
						}
					}

					/// Reads a try-catch from input
					bool Try(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("try")) {
							retval = true;

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'try' block", m_position);
							}

							bool has_matches = true;
							while (has_matches) {
								SkipWS(true);
								has_matches = false;
								if (Keyword("catch")) {
									const auto catch_stack_top = m_match_stack.size();
									if (Char('(')) {
										bool success = false;
										if (Symbol("...")) {
											// captures anything...
											if (!Char(')')) {
												throw exception::eval_error("Incomplete 'catch(...)' expression", m_position);
											}
											success = true;
										}

										if (Arg(currentScope, true)) {
											if (!Char(')')) {
												throw exception::eval_error("Incomplete 'catch' expression", m_position);
											}
											success = true;
										}

										if (!success) {
											throw exception::eval_error("Incomplete 'catch' expression", m_position);
										}
									}

									SkipWS(true);

									if (!Block(currentScope)) {
										throw exception::eval_error("Incomplete 'catch' block", m_position);
									}
									build_match<AST_Nodes::Catch_AST_Node>(currentScope, catch_stack_top);
									has_matches = true;
								}
							}
							SkipWS(true);
							if (Keyword("finally")) {
								const auto finally_stack_top = m_match_stack.size();

								SkipWS(true);

								if (!Block(currentScope)) {
									throw exception::eval_error("Incomplete 'finally' block", m_position);
								}
								build_match<AST_Nodes::Finally_AST_Node>(currentScope, finally_stack_top);
							}

							build_match<AST_Nodes::Try_AST_Node>(currentScope, prev_stack_top);
						}
						else if (Keyword("do")) {
							retval = true;

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'do' block", m_position);
							}
							SkipWS(true);
							if (Keyword("finally")) {
								const auto finally_stack_top = m_match_stack.size();

								SkipWS(true);

								if (!Block(currentScope)) {
									throw exception::eval_error("Incomplete 'finally' block", m_position);
								}
								build_match<AST_Nodes::Finally_AST_Node>(currentScope, finally_stack_top);
							}

							build_match<AST_Nodes::Do_AST_Node>(currentScope, prev_stack_top);
						}
						return retval;
					}

					/// Reads a just-in-time compilation request from input
					bool Eval(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("evaluate")) {
							retval = true;
							SkipWS(true);
#if 1
							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'evaluate' expression", m_position);
							}
							SkipWS(true);
							if (!Equation(currentScope)) {
								throw exception::eval_error("Incomplete 'evaluate' expression", m_position);
							}
							SkipWS(true);
							if (!Char(')')) {
								throw exception::eval_error("Incomplete 'evaluate' expression", m_position);
							}
#else
							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'evaluate' block", m_position);
							}
#endif
							build_match<AST_Nodes::JustInTimeCompilation_AST_Node>(currentScope, prev_stack_top);
						}
						return retval;
					}

					/// Reads a namespace block from input
					/// namespace Thing{ ... };
					bool DeclClass(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						// class TypeName { ... }
						// class TypeName : ParentTypeName { ... }

						if (Keyword("class")) {
							retval = true;
							SkipWS(true);

							if (Id(true, currentScope, AST_Nodes::IdType::Variable)) { // variable becase this namespace may not exist yet! 
								/* Great! Got the desired name of the new namespace */
							}
							else {
								throw exception::eval_error("Incomplete 'class' block: class must have a name", m_position);
							}

							auto this_class_name = GL::string(GetText(m_match_stack.back().first));
							auto& Class = currentScope->GetNamespace()->make_class(this_class_name);

							// instead of collecting statements, we want to collect declarations...
							if (!DeclarationsBlock(&Class)) {
								throw exception::eval_error("Incomplete 'class' block: class declarations must be wrapped in a curly-bracket block", m_position);
							}

							build_match<AST_Nodes::Class_AST_Node>(&Class, prev_stack_top);

							Class.initialize_basic_member_functions();
						}
						return retval;
					};

					/// Reads a namespace block from input
					/// namespace Thing{ ... };
					bool DeclNamespace(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();

						if (Keyword("namespace")) {
							retval = true;
							SkipWS(true);

							if (Id(true, currentScope, AST_Nodes::IdType::Variable)) { // variable becase this namespace may not exist yet! 
								/* Great! Got the desired name of the new namespace */
							}
							else {
								throw exception::eval_error("Incomplete 'namespace' block: namespace must have a name", m_position);
							}

							auto this_class_name = GL::string(GetText(m_match_stack.back().first));
							auto& newNamespace = currentScope->GetNamespace()->make_namespace(this_class_name);

							// instead of collecting statements, we want to collect declarations...
							if (!DeclarationsBlock(&newNamespace)) {
								throw exception::eval_error("Incomplete 'namespace' block: namespace declarations must be wrapped in a curly-bracket block", m_position);
							}
							build_match<AST_Nodes::Namespace_AST_Node>(&newNamespace, prev_stack_top);
						}
						return retval;
					};

					/// Reads a declared function from input
					/// Type Foo(...){ ... };
					bool DeclFunction(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;
						const auto prev_stack_top = m_match_stack.size();
						const auto prev_pos = m_position;

						auto failure = [&]() {
							while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
							m_position = prev_pos;
							return false;
						};

						// return type (Id)
						if (!TypeName(currentScope, true)) { // Id
							return failure();
						}

						// function name (Id)
						if (!Id(true, currentScope, AST_Nodes::IdType::Function)) {
							return failure();
						}

						// Arg_List 
						if (Char('(')) {
							SkipWS(true);
							Decl_Arg_List(currentScope);
							SkipWS(true);
							if (!Char(')')) {
								return failure();
							}
						}
						else {
							return failure();
						}

						// Block
						auto this_scope = currentScope->make_scope();
						if (!Block(&this_scope)) {
							return failure();
						}

						build_match<AST_Nodes::FunctionDecl_AST_Node>(currentScope, prev_stack_top);

						return true;
					};

					/// Top level parser, starts parsing of all known parses
					bool Declarations(GL::scope::impl::BasicScope* currentScope) {
						SkipWS();

						bool retval = false;
						bool has_more = true;
						bool saw_eol = true;

						while (has_more) {
							const auto start = m_position;

							// TO-DO, complete impl of these evaluations:

							if (DeclNamespace(currentScope) || DeclFunction(currentScope) || DeclClass(currentScope)) {
								if (!saw_eol) {
									throw exception::eval_error("Two function definitions missing line separator", start);
								}
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else if (Equation(currentScope)) {
								if (!saw_eol) {
									throw exception::eval_error("Two expressions missing line separator", start);
								}
								has_more = true;
								retval = true;
								saw_eol = false;
							}
							else if (DeclarationsBlock(currentScope) || Eol()) {
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else {
								has_more = false;
							}
						}
						return retval;
					};

					/// Top level parser, starts parsing of all known parses
					bool Statements(GL::scope::impl::BasicScope* currentScope) {
						SkipWS();

						bool retval = false;
						bool has_more = true;
						bool saw_eol = true;

						while (has_more) {
							const auto start = m_position;
							if (DeclNamespace(currentScope) || DeclClass(currentScope) || DeclFunction(currentScope) || /*Def(currentScope) || */ Try(currentScope) || If(currentScope) || While(currentScope) || /* Class(currentScope) || */ For(currentScope) || Switch(currentScope) || Eval(currentScope)) {
								if (!saw_eol) {
									throw exception::eval_error("Two function definitions missing line separator", start);
								}
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else if (Return(currentScope) || Break(currentScope) || Continue(currentScope) || Equation(currentScope)) {
								if (!saw_eol) {
									throw exception::eval_error("Two expressions missing line separator", start);
								}
								has_more = true;
								retval = true;
								saw_eol = false;
							}
							else if (Block(currentScope) || Eol()) {
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else {
								has_more = false;
							}
						}
						return retval;
					};

					/// Reads a curly-brace C-style block from input
					bool Block(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Char('{')) {
							retval = true;

							Statements(currentScope);

							if (!Char('}')) {
								throw exception::eval_error("Incomplete block", m_position);
							}

							if (m_match_stack.size() == prev_stack_top) {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}

							build_match<AST_Nodes::Block_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads a curly-brace C-style block from input which only allows for declarations
					bool DeclarationsBlock(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Char('{')) {
							retval = true;

							Declarations(currentScope);

							if (!Char('}')) {
								throw exception::eval_error("Incomplete declaration block", m_position);
							}

							if (m_match_stack.size() == prev_stack_top) {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}

							build_match<AST_Nodes::Declaration_Block_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads a curly-brace C-style block from input -- note that this scope is special and cannot find objects from parent scopes. 
					bool FunctionBlock(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();

						if (Char('{')) {
							retval = true;

							Statements(currentScope);

							if (!Char('}')) {
								throw exception::eval_error("Incomplete function block", m_position);
							}

							if (m_match_stack.size() == prev_stack_top) {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), nullptr });
							}

							build_match<AST_Nodes::Function_Block_AST_Node>(currentScope, prev_stack_top);
						}

						return retval;
					};

					/// Reads a return statement from input
					bool Return(GL::scope::impl::BasicScope* currentScope) {
						const auto prev_stack_top = m_match_stack.size();
						if (Keyword("return")) {
							Operator(currentScope);
							build_match<AST_Nodes::Return_AST_Node>(currentScope, prev_stack_top);
							return true;
						}
						else {
							return false;
						}
					};

					/// Reads an if/else if/else block from input
					bool If(GL::scope::impl::BasicScope* currentScope) {
						bool retval = false;

						const auto prev_stack_top = m_match_stack.size();
						// SkipWS(true);
						if (Keyword("if")) {
							retval = true;
							SkipWS(true);
							if (!Char('(')) {
								throw exception::eval_error("Incomplete 'if' expression: cannot find '('", m_position);
							}
							SkipWS(true);
							if (!Equation(currentScope)) {
								throw exception::eval_error("Incomplete 'if' expression: cannot find equation block", m_position);
							}
							SkipWS(true);
							const bool is_if_init = Eol() && Equation(currentScope);
							SkipWS(true);
							if (!Char(')')) {
								throw exception::eval_error("Incomplete 'if' expression: cannot find ')'", m_position);
							}

							SkipWS(true);

							if (!Block(currentScope)) {
								throw exception::eval_error("Incomplete 'if' block", m_position);
							}

							bool has_matches = true;
							while (has_matches) {
								SkipWS(true);
								has_matches = false;
								if (Keyword("else")) {
									SkipWS(true);
									if (If(currentScope)) {
										has_matches = true;
									}
									else {
										SkipWS(true);
										if (!Block(currentScope)) {
											throw exception::eval_error("Incomplete 'else' block", m_position);
										}
										has_matches = true;
									}
								}
							}

							const auto num_children = m_match_stack.size() - prev_stack_top;

							if ((is_if_init && num_children == 3) || (!is_if_init && num_children == 2)) {
								m_match_stack.push_back(ParseNode{ std::make_shared<AST_Nodes::Noop_AST_Node>(), currentScope });
							}

							if (!is_if_init) {
								build_match<AST_Nodes::If_AST_Node>(currentScope, prev_stack_top);
							}
							else {
								build_match<AST_Nodes::If_AST_Node>(currentScope, prev_stack_top + 1);
								build_match<AST_Nodes::Block_AST_Node>(currentScope, prev_stack_top);
							}
						}

						return retval;
					};

				public:
					Parser() = default;
					~Parser() = default;

					// highest-level parse request, which starts a new scope from scratch and completes it. 
					ParseNode Parse(const GL::string& t_input, GL::scope::impl::BasicScope* parentScope = nullptr) {
						if (!parentScope) {
							GL::scope::impl::RootScope root;
							root.perform_builtins();
							return parse(t_input, &root);
						}
						else {
							return parse(t_input, parentScope);
						}
					};

				private:
					ParseNode parse(const GL::string& t_input, GL::scope::impl::BasicScope* currentScope) {
						return parse_internal(t_input, currentScope);
					};
					ParseNode parse_internal(GL::string t_input, GL::scope::impl::BasicScope* currentScope) {
						auto this_scope = currentScope->make_scope();

						m_position = Engine::Position(0, t_input);

						// top level stack        
						if (Statements(&this_scope)) {
							if (m_position.has_more()) {
								throw exception::eval_error("Unparsed input", m_position);
							}
							else {
								// add the comment nodes to the front of the stack, to not interupt the automatic return behavior
								auto i = m_comment_stack.rbegin();
								while (i != m_comment_stack.rend()) {
									m_match_stack.insert(m_match_stack.begin(), std::move(*i));
									i = decltype(i)(m_comment_stack.erase(std::next(i).base()));
								}
								build_match<AST_Nodes::File_AST_Node>(&this_scope, 0);
							}
						}
						else {
							m_match_stack.push_back({ std::make_shared<AST_Node_Impl, AST_Nodes::Noop_AST_Node>(AST_Nodes::Noop_AST_Node()), nullptr });
						}

						ParseNode retval = m_match_stack.front();
						m_match_stack.clear();
						// retval.second = currentScope;
						return retval;
					};
					ParseNode parse_instr_eval(const GL::string& t_input, GL::scope::impl::BasicScope* currentScope) {
						auto last_position = m_position;
						auto last_match_stack = std::exchange(m_match_stack, decltype(m_match_stack){});

						auto retval = parse_internal(t_input, currentScope);

						m_position = std::move(last_position);

						m_match_stack = std::move(last_match_stack);

						return retval;
					};

				};

			};

		};
	};
	
	namespace Engine {
		enum class throwing {
			Nothing,
			Return,
			Continue,
			Break,
			Error
		};
		class eval_state {
		public:
			GL::any::fast_any to_return;
			throwing throwing;
		};
		class AbstractSyntaxTreeNode {
		public:
			AbstractSyntaxTreeNode(Engine::AST_Node_Type id) : identifier{ id } {};
			AbstractSyntaxTreeNode(Engine::AST_Node_Type id, GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children) : text(t_ast_node_text.to_string()), location(t_loc), identifier{ id }, children(t_children) {};
			AbstractSyntaxTreeNode() : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Noop) {};
			AbstractSyntaxTreeNode(AbstractSyntaxTreeNode const& rhs) : identifier(rhs.identifier), text(rhs.text), location(rhs.location), constant(rhs.constant), children(), output(rhs.output), tag(rhs.tag) {
				if (rhs.children.size() > 0) children = rhs.children;
				// children.insert(children.end(), rhs.children.begin(), rhs.children.end());
			};
			AbstractSyntaxTreeNode& operator=(AbstractSyntaxTreeNode const& rhs) {
				identifier = rhs.identifier;
				text = rhs.text;
				location = rhs.location;
				constant = rhs.constant;
				output = rhs.output;
				tag = rhs.tag;

				if (rhs.children.size() == 0) {
					children.clear();
				}
				else {
					auto temp = rhs.children;
					children = temp;
				}


				// size_t N = children.size();
				// children.insert(children.end(), rhs.children.begin(), rhs.children.end());
				// if (N > 0) children.erase(children.begin(), children.begin() + N);

				return *this;
			};
			~AbstractSyntaxTreeNode() noexcept = default;

		public:
			Engine::AST_Node_Type
				identifier; // node type
			GL::string
				text; // processed text, relevant to the node type
			Engine::Parse_Location
				location; // start & end Position within provided script
			GL::any::fast_any
				constant;
			std::vector< AbstractSyntaxTreeNode >
				children;
			GL::type
				output = GL::type_of<GL::undefined>();
			GL::any::fast_any
				tag; // custom data, unique to the node type

			/// Prints the contents of an AST node, including its children, recursively
			GL::string to_string(GL::string t_prepend, GL::scope::impl::RootScope& local_root) const {
				GL::string str = std::string_view(identifier.ToString());
				GL::string returnType = output.name();
				GL::string locationStr = location.to_string();
				auto out = t_prepend + "(" + str + ") \"" + text + "\": " + locationStr + " -> " + returnType;
				if (constant) {
					try {
						GL::string Const = local_root.call<GL::string>("to_string", { constant | GL::type::Const | GL::type::Reference });
						out = out.add_to_delim(t_prepend + "\t... includes embedded constant { " + Const + " }", "\n");
					}
					catch (...) {
						out = out.add_to_delim(t_prepend + "\t... includes embedded constant { ??? }", "\n");
					}
				}
				for (auto& elem : children) { out = out.add_to_delim(elem.to_string(t_prepend + "\t", local_root), "\n"); }
				return out;
			};
			// [](AbstractSyntaxTreeNode& this_child) -> bool {}
			// Will continue the search until all nodes are searched or until the function returns true. 
			template <typename F> bool for_each_child(F const& Func, bool depth_first = true, int depth = 0) {
				if (depth > 2000) {
					// throw std::runtime_error("Maximum node depth has been reached");
					return false;
				}

				// for (int i = 0; i < (int)this->children.size(); ++i) {
				for (int i = ((int)this->children.size()) - 1; i >= 0; --i) {
					auto& x = this->children[i];
					if (depth_first) {
						if (x.for_each_child(Func, true, depth+1)) {
							return true;
						}
						if (Func(x)) {
							return true;
						}
					}
					else {
						if (Func(x)) {
							return true;
						}
						if (x.for_each_child(Func, false, depth + 1)) {
							return true;
						}						
					}
									
				}			
			    return false;
			};
			// [](AbstractSyntaxTreeNode& this_child) -> bool {}
			// [](AbstractSyntaxTreeNode& this_child) -> void {}
			// [](AbstractSyntaxTreeNode& this_child) -> void {}
			// Will continue the search until all nodes are searched. If the function returns false, this indicates that we should not go deeper into that node.
			template <typename F, typename G, typename H> void for_each_child(F const& Func, G const& Push, H const& Pop, int depth = 0) {
				if (depth > 2000) {
					// return;
					throw std::runtime_error("Maximum node depth has been reached");
				}

				Push(*this);
				for (int i = 0; i < ((int)this->children.size()); ++i) {
					auto& x = this->children[i];
					if (Func(x)) {
						x.for_each_child(Func, Push, Pop, depth + 1);
					}					
				}
				Pop(*this);
			};

			// returns true if a return call is guarranteed or was found.
			bool guarranteed_return() const {
				if (this->identifier == Engine::AST_Node_Type::Return) return true;
				if (
					(this->identifier == Engine::AST_Node_Type::Block
					|| this->identifier == Engine::AST_Node_Type::Scopeless_Block
					|| this->identifier == Engine::AST_Node_Type::File)
					&& this->children.size() > 0
				) {
					return this->children.back().guarranteed_return();
				}
				if (this->identifier == Engine::AST_Node_Type::If) {
					if (this->children.size() == 3) {
						// must have both the if and else statements be qualified.
						return this->children[1].guarranteed_return() && this->children[2].guarranteed_return();
					}
				}
				return false;
			};

		private:
			//bool try_get_text(const GL::string*& out) const {
			//	if (!text.empty()) {
			//		out = &text; 
			//		return true;
			//	}
			//	for (int i = 0; i < children.size(); ++i) {
			//		if (children[i].try_get_text(out)) {
			//			return true;
			//		};
			//	}
			//	return false;
			//};

		public:
			GL::string get_text() const {
				GL::string out;

				if (!this->text.empty()) {
					return this->text;					
				}
				if (const_cast<AbstractSyntaxTreeNode*>(this)->for_each_child([&out](AbstractSyntaxTreeNode& this_child) -> bool {
					if (!this_child.text.empty()) {
						out = this_child.text;
						return true;
					}
					return false;
				})) {
					return out;
				};
				return GL::string::empty_string();
			};
		};
		
		namespace except {
			/// Errors generated during parsing or evaluation
			struct eval_error : std::runtime_error {
				GL::string reason;
				Engine::Parse_Location position;
				GL::string filename;
				GL::string detail;

				eval_error(const GL::string& t_why, const Engine::Parse_Location& t_where, const GL::string& t_fname = "__EVAL__") noexcept
					: std::runtime_error(format(t_why, t_where, t_fname).to_string())
					, reason(t_why)
					, position(t_where)
					, filename(t_fname) {
				}

				explicit eval_error(const GL::string& t_why) noexcept
					: std::runtime_error(t_why.to_string())
					, reason(t_why) {
				}

				eval_error(const eval_error&) = default;

				GL::string pretty_print() const {
					std::ostringstream ss;
					return ss.str();
				};

				~eval_error() noexcept override = default;

			private:
				template<typename T> static Engine::AST_Node_Type id(const T& t) noexcept {
					return t.identifier;
				};
				template<typename T> static GL::string pretty(const T& t) {
					return t.pretty_print();
				};
				template<typename T> static const GL::string& fname(const T& t) noexcept {
					return t.filename();
				};
				template<typename T> static GL::string startpos(const T& t) {
					std::ostringstream oss;
					oss << t.start().line << ", " << t.start().column;
					return oss.str();
				};
				static GL::string format_why(const GL::string& t_why) { return "Error: \"" + t_why + "\""; };
				template<typename T> static GL::string format_location(const T& t) {
					std::ostringstream oss;
					oss << "(" << t.filename() << " " << t.start().line << ", " << t.start().column << ")";
					return oss.str();
				};
				static GL::string format_filename(const GL::string& t_fname) {
					std::stringstream ss;

					if (t_fname != "__EVAL__") {
						ss << "in '" << t_fname << "'";
					}
					else {
						ss << "during evaluation";
					}

					return ss.str();
				};
				static GL::string format_location(const Engine::Parse_Location& t_where) {
					return "at (" + t_where.to_string() + ")";
				};
				static GL::string format(const GL::string& t_why, const Engine::Parse_Location& t_where, const GL::string& t_fname) {
					return format_why(t_why) + " " + format_filename(t_fname) + " " + format_location(t_where);
				};
			};
		};

		class PreprocessorMacro_Node final : public AbstractSyntaxTreeNode {
		public:
			PreprocessorMacro_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::PreprocessorMacro, t_ast_node_text, t_loc, t_children) {};
		};
		class Return_Node final : public AbstractSyntaxTreeNode {
		public:
			Return_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Return, t_ast_node_text, t_loc, t_children) {};
		};
		class File_Node final : public AbstractSyntaxTreeNode {
		public:
			File_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::File, t_ast_node_text, t_loc, t_children) {
				children.push_back(Return_Node("", { t_loc.end, t_loc.end }, {}));
			};
		};
		class Noop_Node final : public AbstractSyntaxTreeNode {
		public:
			Noop_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Noop, t_ast_node_text, t_loc, t_children) {};
		};
		class Comment_Node final : public AbstractSyntaxTreeNode {
		public:
			Comment_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Comment, t_ast_node_text, t_loc, t_children) {};
		};				
		class Constant_Node final : public AbstractSyntaxTreeNode {
		public:
			Constant_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children, GL::any const& t_value) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Constant, t_ast_node_text, t_loc, t_children) {
				this->constant = t_value.fast() | GL::type::Const;
				this->output = t_value.m_casted_type;
			};
		};
		class Binary_Operator_Node final : public AbstractSyntaxTreeNode {
		public:
			Binary_Operator_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Binary, t_ast_node_text, t_loc, t_children) {				
				this->tag = GL::any::fast_any::instance((GL::Engine::Operators::Opers)Engine::Operators::to_operator(t_ast_node_text.c_str()));
			};
		};
		class Id_Node final : public AbstractSyntaxTreeNode {
		public:
			Id_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Id, t_ast_node_text, t_loc, t_children) {};
		};
		class Arg_Node final : public AbstractSyntaxTreeNode {
		public:
			Arg_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Arg, t_ast_node_text, t_loc, t_children) {};
		};
		class Arg_List_Node final : public AbstractSyntaxTreeNode {
		public:
			Arg_List_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Arg_List, t_ast_node_text, t_loc, t_children) {};
		}; 		
		class Type_Cast_Node final : public AbstractSyntaxTreeNode {
		public:
			Type_Cast_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Type_Cast, t_ast_node_text, t_loc, t_children) {
				
			};
		};
		struct FunctionCallInformation {
			GL::string function_name;
			bool use_return;
		};
		class Fun_Call_Node final : public AbstractSyntaxTreeNode {
		public:
			Fun_Call_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Fun_Call, t_ast_node_text, t_loc, t_children) {
				FunctionCallInformation info;
				info.function_name = this->children[0].get_text();
				info.use_return = true;
				this->tag = GL::any::fast_any::instance(info);
			};
		};
		class Unused_Return_Fun_Call_Node final : public AbstractSyntaxTreeNode {
		public:
			Unused_Return_Fun_Call_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Fun_Call, t_ast_node_text, t_loc, t_children) {
				FunctionCallInformation info;
				info.function_name = this->children[0].get_text();
				info.use_return = false;
				this->tag = GL::any::fast_any::instance(info);
			};
		};
		class Equation_Node final : public AbstractSyntaxTreeNode {
		public:
			Equation_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Equation, t_ast_node_text, t_loc, t_children) {
				this->tag = GL::any::fast_any::instance((GL::Engine::Operators::Opers)Engine::Operators::to_operator(t_ast_node_text.c_str()));
			};
		};
		/* x && y */ class Logical_And_Node final : public AbstractSyntaxTreeNode {
		public:
			Logical_And_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Logical_And, t_ast_node_text, t_loc, t_children) {
				ASSERT(this->children.size() == 2);
			};
		};
		/* x || y */ class Logical_Or_Node final : public AbstractSyntaxTreeNode {
		public:
			Logical_Or_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Logical_Or, t_ast_node_text, t_loc, t_children) {
				ASSERT(this->children.size() == 2);
			};
		};
		struct ObjectDeclarationInformation {
			bool is_constexpr;
		};
		/* [[constexpr]] auto x; */ class Var_Decl_Node final : public AbstractSyntaxTreeNode {
		public:
			Var_Decl_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Var_Decl, t_ast_node_text, t_loc, t_children) {
				ObjectDeclarationInformation info;
				info.is_constexpr = false;
				this->tag = GL::any::fast_any::instance(std::move(info));
			};
		};
		/* [[constexpr]] double x; */ class Assign_Retroactively_Node final : public AbstractSyntaxTreeNode {
		public:
			Assign_Retroactively_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Assign_Retroactively, t_ast_node_text, t_loc, t_children) {
				ASSERT(this->children.size() >= 1);
				auto idname = this->children[1].get_text(); // e.g. x, y, z

				ObjectDeclarationInformation info;
				info.is_constexpr = false;
				this->tag = GL::any::fast_any::instance(std::move(info));
			};
		};
		/* ++x */ class Prefix_Node final : public AbstractSyntaxTreeNode {
		public:
			Prefix_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Prefix, t_ast_node_text, t_loc, t_children) {
				this->tag = GL::any::fast_any::instance((GL::Engine::Operators::Opers)Engine::Operators::to_operator(t_ast_node_text.c_str()));
			};
		};
		struct PostfixInformation {
			bool is_unit;
			GL::string unit_name;
			GL::Engine::Operators::Opers oper;
		};
		/* x++ */ class Postfix_Node final : public AbstractSyntaxTreeNode {
		public:
			Postfix_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Postfix, t_ast_node_text, t_loc, t_children) {
				PostfixInformation info;
				info.is_unit = false;
				info.unit_name = GL::string::empty_string();
				info.oper = (GL::Engine::Operators::Opers)Engine::Operators::to_operator(t_ast_node_text.c_str());
				this->tag = GL::any::fast_any::instance(std::move(info));
			};
		};		
		/* if (Scopeless_Block_AST_Node) Block_AST_Node else Block_AST_Node */ class If_Node final : public AbstractSyntaxTreeNode {
		public:
			If_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::If, t_ast_node_text, t_loc, t_children) {};
		};
		/* while (Scopeless_Block_AST_Node) Block_AST_Node */ class While_Node final : public AbstractSyntaxTreeNode {
		public:
			While_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::While, t_ast_node_text, t_loc, t_children) {};
		};
		struct ForLoopInformation {
			bool parallel_hint;
		};
		/* for (INIT_BLOCK; CONDITION_BLOCK; PROGRESS_BLOCK) WORK_BLOCK */ class For_Node final : public AbstractSyntaxTreeNode {
		public:
			For_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}, bool parallel_hint = false) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::For, t_ast_node_text, t_loc, t_children) {
				ForLoopInformation info;
				info.parallel_hint = parallel_hint;
				this->tag = GL::any::fast_any::instance(std::move(info));
			};
		};
		/* for (range_declaration : range_expression) loop_statement */ class Ranged_For_Node final : public AbstractSyntaxTreeNode {
		public:
			Ranged_For_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}, bool parallel_hint = false) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Ranged_For, t_ast_node_text, t_loc, t_children) {
				ForLoopInformation info;
				info.parallel_hint = parallel_hint;
				this->tag = GL::any::fast_any::instance(std::move(info));
			};
		};
		/* x[1] */ class Array_Call_Node final : public AbstractSyntaxTreeNode {
		public:
			Array_Call_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Array_Call, t_ast_node_text, t_loc, t_children) {};
		};
		/* x.first */ class Dot_Access_Node final : public AbstractSyntaxTreeNode {
		public:
			Dot_Access_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Dot_Access, t_ast_node_text, t_loc, t_children) {
				GL::string m_fun_name = ((children[1].identifier == Engine::AST_Node_Type::Fun_Call) || (children[1].identifier == Engine::AST_Node_Type::Array_Call))
					? children[1].children[0].text
					: children[1].text;
			};
		};
		/* [x](FF) async -> int { return x+FF; } */ class Lambda_Node final : public AbstractSyntaxTreeNode {
		public:
			Lambda_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Lambda, t_ast_node_text, t_loc, t_children) {
				// Heavy To-Do
			};
		};
		/* [0, 1, 2, 3] */ class Inline_Array_Node final : public AbstractSyntaxTreeNode {
		public:
			Inline_Array_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Inline_Array, t_ast_node_text, t_loc, t_children) {};
		};
		class Map_Pair_Node final : public AbstractSyntaxTreeNode {
		public:
			Map_Pair_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Map_Pair, t_ast_node_text, t_loc, t_children) {};
		};
		/* ["":10, 10:10, Vector():10, 20:Vector()] */ class Inline_Map_Node final : public AbstractSyntaxTreeNode {
		public:
			Inline_Map_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Inline_Map, t_ast_node_text, t_loc, t_children) {};
		};		
		class Break_Node final : public AbstractSyntaxTreeNode {
		public:
			Break_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Break, t_ast_node_text, t_loc, t_children) {};
		};
		class Continue_Node final : public AbstractSyntaxTreeNode {
		public:
			Continue_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Continue, t_ast_node_text, t_loc, t_children) {};
		};
		class Default_Node final : public AbstractSyntaxTreeNode {
		public:
			Default_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Default, t_ast_node_text, t_loc, t_children) {};
		};
		class Case_Node final : public AbstractSyntaxTreeNode {
		public:
			Case_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Case, t_ast_node_text, t_loc, t_children) {};
		};
		struct SwitchInformation {
			bool allow_precompilation;
			bool is_precompiled;
			std::map<size_t, size_t> hash_to_child_index; // hash to child index;
			size_t default_child_index; // child index for the default param. Values larger than the number of children indicate that no default was provided. 
		};
		class Switch_Node final : public AbstractSyntaxTreeNode {
		public:
			Switch_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Switch, t_ast_node_text, t_loc, t_children) {
				SwitchInformation info;
				info.is_precompiled = false;
				info.allow_precompilation = true;
				info.default_child_index = std::numeric_limits<size_t>::max();
				this->tag = GL::any::fast_any::instance(std::move(info));
			};
		};
		// Preprocessed switches are switch statements that have been already resolved. We still need to handle Break throws, but otherwise they can be processed just like Blocks.
		class PreprocessedSwitch_Node final : public AbstractSyntaxTreeNode {
		public:
			PreprocessedSwitch_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::PreprocessedSwitch, t_ast_node_text, t_loc, t_children) {};
		};
		class Try_Node final : public AbstractSyntaxTreeNode {
		public:
			Try_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Try, t_ast_node_text, t_loc, t_children) {};
		};
		class Catch_Node final : public AbstractSyntaxTreeNode {
		public:
			Catch_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Catch, t_ast_node_text, t_loc, t_children) {};
		};
		class Finally_Node final : public AbstractSyntaxTreeNode {
		public:
			Finally_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Finally, t_ast_node_text, t_loc, t_children) {};
		};
		/*! Currently, the JIT compilation does not support preprocessor macros or other preprocessor activities. */
		class JustInTimeCompilation_Node final : public AbstractSyntaxTreeNode {
		public:
			JustInTimeCompilation_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::JustInTimeCompilation, t_ast_node_text, t_loc, t_children) {};
		};
		class Scopeless_Block_Node final : public AbstractSyntaxTreeNode {
		public:
			Scopeless_Block_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Scopeless_Block, t_ast_node_text, t_loc, t_children) {};
		};
		class Block_Node final : public AbstractSyntaxTreeNode {
		public:
			Block_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Block, t_ast_node_text, t_loc, t_children) {};
		};
		class FunctionBlock_Node final : public AbstractSyntaxTreeNode {
		public:
			FunctionBlock_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::FunctionBlock, t_ast_node_text, t_loc, t_children) {};
		};
		class Fold_Right_Binary_Operator_Node final : public AbstractSyntaxTreeNode {
		public:
			Fold_Right_Binary_Operator_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children, GL::any const& t_value) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::BinaryFoldRight, t_ast_node_text, t_loc, t_children) {
				this->constant = t_value.fast();
				this->tag = GL::any::fast_any::instance((GL::Engine::Operators::Opers)Engine::Operators::to_operator(t_ast_node_text.c_str()));
			};
		};
		class Fold_Left_Binary_Operator_Node final : public AbstractSyntaxTreeNode {
		public:
			Fold_Left_Binary_Operator_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children, GL::any const& t_value) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::BinaryFoldLeft, t_ast_node_text, t_loc, t_children) {
				this->constant = t_value.fast();
				this->tag = GL::any::fast_any::instance((GL::Engine::Operators::Opers)Engine::Operators::to_operator(t_ast_node_text.c_str()));
			};
		};
		struct NamespaceClassInformation {
			std::vector<GL::string> template_types;
			bool original_placement;
		};
		class Namespace_Node final : public AbstractSyntaxTreeNode {
		public:
			Namespace_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Namespace, t_ast_node_text, t_loc, t_children) {
				if (this->children.size() > 0) {
					if (this->children[0].identifier == Engine::AST_Node_Type::Id) {
						this->text = this->children[0].text;
					}
				}
				NamespaceClassInformation info;
				info.template_types = {};
				info.original_placement = true;
				this->tag = GL::any::fast_any::instance(std::move(info));
			};
		};
		class Class_Node final : public AbstractSyntaxTreeNode {
		public:
			Class_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Class, t_ast_node_text, t_loc, t_children) {
				if (this->children.size() > 0) {
					if (this->children[0].identifier == Engine::AST_Node_Type::Id) {
						this->text = this->children[0].text;
					}
				}
				NamespaceClassInformation info;
				info.template_types = {};
				info.original_placement = true;	
				if (this->text.find("<") != GL::string::npos) {
					if (this->text.find(">") != GL::string::npos) {						
						info.template_types = this->text.right_of("<").left_of(">").split_nested(",", "<", ">");
						this->text = this->text.left_of("<");
					}
				}
				this->tag = GL::any::fast_any::instance(std::move(info));
			};
		};
		class Declaration_Block_Node final : public AbstractSyntaxTreeNode {
		public:
			Declaration_Block_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::DeclarationBlock, t_ast_node_text, t_loc, t_children) {};
		};

		struct FunctionDeclInformation {
			bool is_constexpr;
		};
		class FunctionDecl_Node final : public AbstractSyntaxTreeNode {
		public:
			FunctionDecl_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::FunctionDecl, t_ast_node_text, t_loc, t_children) {
				FunctionDeclInformation info;
				info.is_constexpr = false;
				this->tag = GL::any::fast_any::instance(std::move(info));
			};
		};
		class Throw_Node final : public AbstractSyntaxTreeNode {
		public:
			Throw_Node(GL::string const& t_ast_node_text, Engine::Parse_Location const& t_loc, std::vector<AbstractSyntaxTreeNode> const& t_children = {}) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Throw, t_ast_node_text, t_loc, t_children) {};
		};


		class ScriptParser {
		public:
			class Parser;
			class optimizer {
			public:
				static int& OptimizationDepth() {
					thread_local static int out{ 0 };
					return out;
				};
			private:
				class DepthCounter {
				public:
					const int depth;
					DepthCounter() : depth{ ++OptimizationDepth() } {};
					DepthCounter(DepthCounter const&) = delete;
					DepthCounter(DepthCounter &&) = delete;
					DepthCounter& operator=(DepthCounter const&) = delete;
					DepthCounter& operator=(DepthCounter&&) = delete;
					~DepthCounter() {
						--OptimizationDepth();
					};


				};

				static std::deque<Engine::ScriptParser::Parser*>& GetParser() {
					thread_local std::deque<Engine::ScriptParser::Parser*> p;
					return p;
				};
				static Engine::ScriptParser::Parser& CurrentParser() {
					return *GetParser().back();
				};
				static std::deque<GL::scope::impl::RootScope*>& GetEngine() {
					thread_local std::deque<GL::scope::impl::RootScope*> p;
					return p;
				};
				static GL::scope::impl::RootScope& CurrentEngine() {
					return *GetEngine().back();
				};
				static bool AttemptCalculation(GL::string const& operation, std::vector<GL::any::fast_any> const& inputs, GL::any::fast_any& out) {
					try {
						GL::scope::impl::RootScope& temp = CurrentEngine();
						out = temp.call(operation, inputs);					
						return true;
					}
					catch (...) {
						return false;
					}
				};
				template<typename T>
				static bool AttemptCast(GL::any::fast_any const& input, T& out) {
					try {
						GL::scope::impl::RootScope& temp = CurrentEngine();
						out = temp.cast<T>(input);
						return true;
					}
					catch (...) {
						return false;
					}
				};

			private:
				template<typename... T> struct Optimizer : T... {
					Optimizer() = default;
					explicit Optimizer(T... t) : T(std::move(t))... { };

					bool optimize_impl(AbstractSyntaxTreeNode& p) {
						bool successful = false;
						((successful = (successful || static_cast<T&>(*this).optimize(p))), ...); // this line performs all optimizations in-line
						return successful;
					};

					bool optimize(AbstractSyntaxTreeNode& p, GL::scope::impl::RootScope& analysisEngine, int maxDepth = 1000) {
						GetEngine().push_back(&analysisEngine);
						bool any_success = false;
						while (--maxDepth >= 0) {
							if (p.for_each_child([this](AbstractSyntaxTreeNode& this_child) -> bool {
								return this->optimize_impl(this_child);
							}) || optimize_impl(p)) {
								any_success = true;
							}
							else {
								break;
							}
						}
						GetEngine().pop_back();
						return any_success;
					};

				};

				static AbstractSyntaxTreeNode& child_at(AbstractSyntaxTreeNode& node, const size_t offset) noexcept {
					return node.children[offset];
				};
				static const AbstractSyntaxTreeNode& child_at(const AbstractSyntaxTreeNode& node, const size_t offset) noexcept {
					return node.children[offset];
				};
				static size_t child_count(const AbstractSyntaxTreeNode& node) noexcept {
					return node.children.size();
				};
				static bool contains_var_decl_in_scope(const AbstractSyntaxTreeNode& node) noexcept {
					if (
						node.identifier == Engine::AST_Node_Type::Var_Decl
						|| node.identifier == Engine::AST_Node_Type::Assign_Retroactively
						|| node.identifier == Engine::AST_Node_Type::Def
						|| node.identifier == Engine::AST_Node_Type::Class
					) {
						return true;
					}

					const auto num = child_count(node);
					for (size_t i = 0; i < num; ++i) {
						const auto& child = child_at(node, i);
						if (child.identifier != Engine::AST_Node_Type::Block
							&& child.identifier != Engine::AST_Node_Type::For
							&& child.identifier != Engine::AST_Node_Type::Ranged_For
							&& child.identifier != Engine::AST_Node_Type::Parallel_For
							&& child.identifier != Engine::AST_Node_Type::Parallel_Ranged_For
							&& contains_var_decl_in_scope(child)
						) {
							return true;
						}
					}

					return false;
				};

				// 
				struct Example {
					bool optimize(AbstractSyntaxTreeNode& p) {
						return false; // does nothing
					}
				};

				// String embedding results in a structure that may resemble:
				//		ArgList -> {  File -> {   Constant   }  }
				// This should be simplified to: 
				//		ArgList -> {  Constant  }
				struct ArgListFileConstant {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Arg_List
							&& node.children.size() == 1
							&& node.children[0].identifier == Engine::AST_Node_Type::File
							&& node.children[0].children.size() == 1
							&& node.children[0].children[0].identifier == Engine::AST_Node_Type::Constant
						) {
							node.children[0] = std::move(node.children[0].children[0]);
							return true;
						}
						return false;
					}
				};

				// converts from:
				//		auto x = int(1)
				// to:
				//		int x{ 1 };
				struct VarDeclEquation_To_RetroactiveAssignment {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Equation
							&& node.children.size() == 2
							&& node.children[0].identifier == Engine::AST_Node_Type::Var_Decl
							&& node.children[0].children.size() == 1
							&& ((node.text == "=") || (node.text == ":=") || (node.text == "?="))
						) {
							std::vector<AbstractSyntaxTreeNode> new_children = {
								node.children[1], // Constant, Fun_Call, Dot_Access, etc.
								node.children[0] // Var_Decl node
							};
							node = Assign_Retroactively_Node(node.text, node.location, new_children);
							node.tag = new_children[1].tag;
							node.children[1].tag = GL::any::fast_any::instance(ObjectDeclarationInformation{ 
								false 
							});
							return true;
						}

						if (node.identifier == Engine::AST_Node_Type::Equation
							&& node.children.size() == 2
							&& node.children[0].identifier == Engine::AST_Node_Type::Assign_Retroactively
							&& node.children[0].children.size() == 2
							&& ((node.text == "=") || (node.text == ":=") || (node.text == "?="))
						) {
							std::vector<AbstractSyntaxTreeNode> new_children = {
								node.children[0].children[0], // Default Value or Initializer (e.g. Fun_Call, Int(0), etc.). May be converted to constexpr already. 
								node.children[0].children[1], // VarDecl
								node.children[1] // New value or assignment. 
							};						
							node = node.children[0];
							node.identifier = Engine::AST_Node_Type::Assign_Retroactively;
							node.children = new_children;
							
							if (!node.tag.cast<ObjectDeclarationInformation>().is_constexpr) {
								node.constant = nullptr;
								node.children[1].tag.cast<ObjectDeclarationInformation>().is_constexpr = false;
							}
							else {
								node.children[1].tag.cast<ObjectDeclarationInformation>().is_constexpr = true;
								if (node.constant) {
									if (node.children[2].identifier == Engine::AST_Node_Type::Constant
										|| (node.children[2].identifier == Engine::AST_Node_Type::Id
											&& node.children[2].constant)
									) {
										// we were already given a contant value!
										if (GL::any::fast_any result; AttemptCalculation("=", { node.constant, node.children[2].constant | GL::type::Const | GL::type::Reference }, result)) {
											// successfully "assigned" the RHS to the LHS type.
											node.children[0] = Constant_Node(node.children[0].text, node.children[0].location, {}, node.constant);
											node.children.pop_back();
											return true;
										}
									}
									else {
										node.constant = nullptr;
									}
								}
								//else {									
									//if (node.children[2].constant) {
									//	node.constant = node.children[2].constant;
									//}
								//}
							}

							return true;
						}
						return false;
					}
				};

				// String embedding results in a structure that may resemble:
				//		Fun_Call -> {  Id{ to_string }, Arg_list{ Constant{} } }
				// This should be simplified and completed:
				//      Constant(to_string(Constant()))
				struct ToStringFunctionCallWithConstant {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Fun_Call
							&& node.children.size() == 2
							&& node.children[0].identifier == Engine::AST_Node_Type::Id
							&& node.children[0].children.size() == 0
							&& node.children[1].identifier == Engine::AST_Node_Type::Arg_List
							&& node.children[1].children.size() == 1
							&& node.children[1].children[0].identifier == Engine::AST_Node_Type::Constant
							&& node.children[0].text == "to_string"
						) {
							const GL::any::fast_any& rhs = node.children[1].children[0].constant;
							if (GL::any::fast_any result; AttemptCalculation("to_string", { rhs | GL::type::Const | GL::type::Reference }, result)) {
								node = Constant_Node(node.text, (Engine::Parse_Location)node.location, {}, result);
								return true;
							}
							return false;
						}
						return false;
					}
				};

				// removes items from Blocks that are unecessary (e.g. floating code) or will never be hit (e.g. following return statements)
				struct Dead_Code {
					__declspec(noinline) bool optimize(AbstractSyntaxTreeNode& node) {
						if (const auto num_children = node.children.size(); 
							((node.identifier == Engine::AST_Node_Type::Block) || (node.identifier == Engine::AST_Node_Type::File))
							&& (num_children > 0)
						) {							
							for (size_t i = 0; i < (num_children - 1); ++i) {
								if (node.children[i].identifier == Engine::AST_Node_Type::Assign_Retroactively
									&& node.children[i].children.size() >= 2
									&& node.children[i].children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& node.children[i].children[1].children.size() >= 1
									&& node.children[i].children[1].children[0].identifier == Engine::AST_Node_Type::Id
									&& node.children[i].children[1].children[0].children.size() == 0
								) {
									// if (node.children[i].tag.cast<ObjectDeclarationInformation>().is_constexpr && node.children[i].constant) {
										bool is_used = false;
										for (size_t j = i + 1; j < num_children; ++j) {
											if (node.children[j].identifier == Engine::AST_Node_Type::Id
												&& node.children[j].children.size() == 0
												) {
												if (node.children[j].text == node.children[i].children[1].children[0].text) {
													is_used = true;
													break;
												}
											}
											if (node.children[j].for_each_child([&](AbstractSyntaxTreeNode& this_child) -> bool {
												if (this_child.identifier == Engine::AST_Node_Type::Id
													&& this_child.children.size() == 0
													) {
													if (this_child.text == node.children[i].children[1].children[0].text) {
														return true;
													}
												}
												return false;
											})) {
												is_used = true;
												break;
											};
										}
										if (!is_used) {
											node.children[i] = Noop_Node("", {}, {});
											return true;
										}
									// }
								}
								else if (node.children[i].identifier == Engine::AST_Node_Type::Var_Decl 
									&& node.children[i].children.size() >= 1 
									&& node.children[i].children[0].identifier == Engine::AST_Node_Type::Id
									&& node.children[i].children[0].children.size() == 0
								) {
									// if (node.children[i].tag.cast<ObjectDeclarationInformation>().is_constexpr && node.children[i].constant) {
										bool is_used = false;
										for (size_t j = i + 1; j < num_children; ++j) {
											if (node.children[j].identifier == Engine::AST_Node_Type::Id
												&& node.children[j].children.size() == 0
											) {
												if (node.children[j].text == node.children[i].children[0].text) {
													is_used = true;
													break;
												}
											}
											if (node.children[j].for_each_child([&](AbstractSyntaxTreeNode& this_child) -> bool {
												if (this_child.identifier == Engine::AST_Node_Type::Id
													&& this_child.children.size() == 0
												) {
													if (this_child.text == node.children[i].children[0].text) {
														return true;
													}
												}
												return false;
											})) {
												is_used = true;
												break;
											};
										}
										if (!is_used) {
											node.children[i] = Noop_Node("", {}, {});
											return true;
										}
									// }
								}
							}
						}
						
						if ((node.identifier == Engine::AST_Node_Type::File) || (node.identifier == Engine::AST_Node_Type::Block) || (node.identifier == Engine::AST_Node_Type::Scopeless_Block)) {
							std::vector<size_t> keepers;
							const auto num_children = node.children.size();
							keepers.reserve(num_children);
							bool foundReturnStatement = false;
							for (size_t i = 0; i < num_children; ++i) {
								const auto& child = node.children[i];
								switch (child.identifier) {
								//case Engine::AST_Node_Type::Constant: // 50.0f;
								case Engine::AST_Node_Type::Noop: // comments
								// case Engine::AST_Node_Type::Comment: // comments
								//case Engine::AST_Node_Type::Id: // y, x, etc.
									break;
								case Engine::AST_Node_Type::Return: // return; return x; return 50; etc.
									keepers.push_back(i);
									i = num_children; // stop considering the remaining items -- they'll never be found anyways. 
									foundReturnStatement = true;
									break;
								default:
									keepers.push_back(i);
									break;
								}
							}
							// if ((!foundReturnStatement) && (num_children > 0)) { keepers.push_back(num_children - 1); };

							if (keepers.size() == num_children) {
								if (num_children == 0) {
									node = Noop_Node(node.text, node.location, {});
									return true;
								}
								else {
									return false;
								}								
							}
							else {
								const auto new_children = [&]() {
									std::vector<AbstractSyntaxTreeNode> retval;
									for (const auto x : keepers) {
										retval.push_back(node.children[x]);
									}
									return retval;
								};

								node.children = new_children();		

								return true;
							}
						}

						// Var_Decl nodes whose ID's have been converted into constexpr values...
						if (node.identifier == Engine::AST_Node_Type::Var_Decl
							&& node.children.size() == 1
							&& node.children[0].identifier == Engine::AST_Node_Type::Constant
						) {
							node = Noop_Node(node.text, node.location, {});
							return true;
						};

						// Assign_Retroactively nodes whose ID's have been converted into constexpr values...
						if (node.identifier == Engine::AST_Node_Type::Assign_Retroactively
							&& node.children.size() >= 2
							&& node.children[1].identifier == Engine::AST_Node_Type::Noop
						) {
							node = Noop_Node(node.text, node.location, {});
							return true;
						};

						return false;
					}
				};

				// re-arrange the return statement, to avoid throwing whenever possible
				struct Return {
					bool optimize(AbstractSyntaxTreeNode& p) {
	#if 0
						if ((p.identifier == Engine::AST_Node_Type::Lambda) && !p.children.empty()) {
							auto& last_child = p->children.back();
							if (last_child->identifier == Engine::AST_Node_Type::Block || last_child->identifier == Engine::AST_Node_Type::Scopeless_Block) {
								auto& block_last_child = last_child->children.back();
								if (block_last_child->identifier == Engine::AST_Node_Type::Return) {
									if (block_last_child->children.size() == 1) {
										block_last_child = std::move(block_last_child->children[0]);
										return true;
									}
									else if (block_last_child->children.size() == 0) {
										block_last_child = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<AST_Nodes::Noop_AST_Node>());
										return true;
									}
								}
							}
							//if (last_child->identifier == AST_Node_Type::Return) {
							//	if (last_child->children.size() == 1) {
							//		last_child = std::move(last_child->children[0]);
							//		return true;
							//	}
							//	else if (last_child->children.size() == 0) {
							//		last_child = std::dynamic_pointer_cast<AST_Node_Impl>(std::make_shared<Noop_AST_Node>());
							//		return true;
							//	}
							//}
						}

						if ((p->identifier == Engine::AST_Node_Type::Def) && !p->children.empty()) {
							auto& last_child = p->children.back();
							if (last_child->identifier == Engine::AST_Node_Type::Block || last_child->identifier == Engine::AST_Node_Type::Scopeless_Block) {
								auto& block_last_child = last_child->children.back();
								if (block_last_child->identifier == Engine::AST_Node_Type::Return) {
									if (block_last_child->children.size() == 1) {
										last_child->children.back() = std::move(block_last_child->children[0]);
										return true;
									}
								}
							}
						}
	#endif
						if ((p.identifier == Engine::AST_Node_Type::File) && (!p.children.empty())) {
							auto& last_child = p.children.back();
							if ((last_child.identifier == Engine::AST_Node_Type::Block) || (last_child.identifier == Engine::AST_Node_Type::Scopeless_Block)) {
								auto& block_last_child = last_child.children.back();
								if (block_last_child.identifier == Engine::AST_Node_Type::Return) {
									if (block_last_child.children.size() == 0) {
										last_child.children.back() = Noop_Node(GL::string::empty_string(), (Parse_Location)last_child.location, {});
										return true;
									}
									else if (block_last_child.children.size() == 1) {
										last_child.children.back() = block_last_child.children[0];
										return true;
									}
								}
							}
							else if (last_child.identifier == Engine::AST_Node_Type::Return) {
								if (last_child.children.size() == 0) {
									last_child = Noop_Node(GL::string::empty_string(), (Parse_Location)last_child.location, {});
									return true;
								}
								else if (last_child.children.size() == 1) {
									last_child = last_child.children[0];
									return true;
								}
							}
							else if (last_child.identifier == Engine::AST_Node_Type::If) {
								if ((last_child.children.size() == 3) && last_child.guarranteed_return()) {
									// both paths are guarranteed to return. We can remove the call to "return" explicitely for each path. 
									bool out = false;
									if (1) {
										auto& this_child = last_child.children[1];
										if ((this_child.identifier == Engine::AST_Node_Type::Block) || (this_child.identifier == Engine::AST_Node_Type::Scopeless_Block)) {
											auto& block_last_child = this_child.children.back();
											if (block_last_child.identifier == Engine::AST_Node_Type::Return) {
												if (block_last_child.children.size() == 0) {
													this_child.children.back() = Noop_Node(GL::string::empty_string(), (Parse_Location)last_child.location, {});												
													out = true;
												}
												else if (block_last_child.children.size() == 1) {
													this_child.children.back() = block_last_child.children[0];
													out = true;
												}
											}
										}
										else if (this_child.identifier == Engine::AST_Node_Type::Return) {
											if (this_child.children.size() == 0) {
												this_child = Noop_Node(GL::string::empty_string(), (Parse_Location)last_child.location, {});
												out = true;
											}
											else if (this_child.children.size() == 1) {
												this_child = this_child.children[0];
												out = true;
											}
										}
									}
									if (1) {
										auto& this_child = last_child.children[2];
										if ((this_child.identifier == Engine::AST_Node_Type::Block) || (this_child.identifier == Engine::AST_Node_Type::Scopeless_Block)) {
											auto& block_last_child = this_child.children.back();
											if (block_last_child.identifier == Engine::AST_Node_Type::Return) {
												if (block_last_child.children.size() == 0) {
													this_child.children.back() = Noop_Node(GL::string::empty_string(), (Parse_Location)last_child.location, {});
													out = true;
												}
												else if (block_last_child.children.size() == 1) {
													this_child.children.back() = block_last_child.children[0];
													out = true;
												}
											}
										}
										else if (this_child.identifier == Engine::AST_Node_Type::Return) {
											if (this_child.children.size() == 0) {
												this_child = Noop_Node(GL::string::empty_string(), (Parse_Location)last_child.location, {});
												out = true;
											}
											else if (this_child.children.size() == 1) {
												this_child = this_child.children[0];
												out = true;
											}
										}
									}
									if (out) return out;
								}
							}
						}

						if (const auto num_children = p.children.size();
							((p.identifier == Engine::AST_Node_Type::Block)/* || (p.identifier == Engine::AST_Node_Type::File)*/)
							&& (num_children > 0)
						) {
							auto& this_child = p.children[num_children - 1];
							if (this_child.identifier == Engine::AST_Node_Type::Assign_Retroactively) {
								if (this_child.tag.cast<ObjectDeclarationInformation&>().is_constexpr && this_child.constant) {
									// the final object in the block or file is a Constant value. 
									this_child = Constant_Node(this_child.text, this_child.location, {}, this_child.constant);
									return true;
								}
								else {
									// the final object in the block or file is a Assign_Retroactively, e.g. (var x = 10) or (int x = 200.0)
									if (this_child.children.size() == 2) {
										this_child = this_child.children[0];
										return true;
									}
									else if (this_child.children.size() == 3) {
										this_child = Equation_Node("=", this_child.location, { this_child.children[0], this_child.children[2] });
										return true;
									}
								}								
							}
						}

						return false;
					}
				};

				// re-order blocks, scopeless blocks, and file declarations.
				struct Block {
					bool optimize(AbstractSyntaxTreeNode& node) {
						// un-scope blocks that do not require the use of an explicit scope.
						if (node.identifier == Engine::AST_Node_Type::Block) {
							if (!contains_var_decl_in_scope(node)) {
								if (node.children.size() == 1) {
									node = node.children[0];
									return true;
								}
								else {
									node = Scopeless_Block_Node(
										node.text,
										node.location,
										node.children
									);
									return true;
								}
							}
						}

						// if a scopeless block node only has one child, accept that child as the content of this node
						if ((node.identifier == Engine::AST_Node_Type::Scopeless_Block) || (node.identifier == Engine::AST_Node_Type::File)) {							
							if (node.children.size() == 1) {
								if (!contains_var_decl_in_scope(node)) {
									node = node.children[0];
									return true;
								}
							}
						}

						// if a block, scopeless block, or file has a child that is a scopeless block, then incorporate their children directly, eliminating the middle-man.
						if ((node.identifier == Engine::AST_Node_Type::Block)
							|| (node.identifier == Engine::AST_Node_Type::Scopeless_Block)
							|| (node.identifier == Engine::AST_Node_Type::File)
						) {
							bool success = false;
							for (int i = ((int)node.children.size()) - 1; i >= 0; --i) {
								if (node.children[i].identifier == Engine::AST_Node_Type::Scopeless_Block) {
									node.children.insert(node.children.begin() + i + 1, node.children[i].children.begin(), node.children[i].children.end());
									node.children.erase(node.children.begin() + i);								
									success = true;
								}
							}
							if (success) return success;
						}

						// if a file has an inner child that is a file, change that into a block.
						if (node.identifier == Engine::AST_Node_Type::File) {
							if (node.for_each_child([](AbstractSyntaxTreeNode& child) -> bool {
								if (child.identifier == Engine::AST_Node_Type::File) {
									child = Block_Node(child.text, child.location, child.children);
									return true;
								}
								return false;
							})) return true;
						}

						return false;
					}
				};

				// If a function call's return value was going to be unused, there may be no point to holding onto it. 
				struct Unused_Fun_Return {
					bool optimize(AbstractSyntaxTreeNode& node) {
						bool result = false;
						if ((node.identifier == Engine::AST_Node_Type::Block || node.identifier == Engine::AST_Node_Type::Scopeless_Block) && !node.children.empty()) {
							for (size_t i = 0; i < node.children.size() - 1; ++i) {
								auto& child = node.children[i];
								if ((child.identifier == Engine::AST_Node_Type::Fun_Call)
									&& child.tag.can_cast(GL::type_of<FunctionCallInformation>())
									&& (child.tag.cast< FunctionCallInformation>().use_return == true)
								) {
									node.children[i] = Unused_Return_Fun_Call_Node(
										child.text,
										child.location,
										child.children
									);
									result = true;
								}
							}
						}
						else if ((node.identifier == Engine::AST_Node_Type::For || node.identifier == Engine::AST_Node_Type::While) && child_count(node) > 0) {
							auto& child = child_at(node, child_count(node) - 1);
							if (child.identifier == Engine::AST_Node_Type::Block || child.identifier == Engine::AST_Node_Type::Scopeless_Block) {
								auto num_sub_children = child_count(child);
								for (size_t i = 0; i < num_sub_children; ++i) {
									auto& sub_child = child.children[i];
									if (sub_child.identifier == Engine::AST_Node_Type::Fun_Call
										&& sub_child.tag.can_cast(GL::type_of<FunctionCallInformation>())
										&& (sub_child.tag.cast< FunctionCallInformation>().use_return == true)
									) {
										child.children[i] = Unused_Return_Fun_Call_Node(
											sub_child.text,
											sub_child.location,
											sub_child.children
										);
										result = true;
									}
								}
							}
						}
						return result;
					}
				};

				// Re-order or collapse if statements.
				struct If {
					bool optimize(AbstractSyntaxTreeNode& node) {
						// If the condition of an If statement is constant and known, then simply skip the check and hard-code the correct path. 
						if ((node.identifier == Engine::AST_Node_Type::If) && (node.children.size() >= 2) && (node.children[0].identifier == Engine::AST_Node_Type::Constant)) {
							bool result;
							if (node.children[0].constant.can_cast(GL::type_of<bool>())) {
								result = node.children[0].constant.cast<bool>();
							}
							else {
								if (!AttemptCast<bool>(node.children[0].constant, result)) {
									return false;
								}
							}

							if (result) {
								// "TRUE" statement is the exclusive path
								node = node.children[1];
								return true;
							}
							else if (node.children.size() == 3) {
								// "FALSE" statement is the exclusive path (and a false path is even present)
								node = node.children[2];
								return true;
							}
							else {
								// do nothing?
								node = Noop_Node(GL::string::empty_string(), (Parse_Location)node.location, {});
								return true;
							}
						}

						// Combine statements made after an If statement into the If statement, when a return call has been demonstrated to be guarranteed.
						if (((node.identifier == Engine::AST_Node_Type::Block)
							|| (node.identifier == Engine::AST_Node_Type::Scopeless_Block)
							|| (node.identifier == Engine::AST_Node_Type::File))
						) {
							for (int i = ((int)node.children.size()) - 1; i >= 0; --i) {
								if (
									(node.children[i].identifier == Engine::AST_Node_Type::If)
									&& (i < (((int)node.children.size()) - 1))
								) {
									auto& this_if = node.children[i];
									if (this_if.children.size() == 2) {
										if (this_if.children[1].guarranteed_return()) {
											// move all children after this If statement into the Else statement
											this_if.children.push_back(Block_Node("", this_if.location, {
												node.children.begin() + i + 1, node.children.end()
											}));
											node.children.erase(node.children.begin() + i + 1, node.children.end());
											return true;
										}
									}
									else if (this_if.children.size() == 3) {
										if (this_if.guarranteed_return()) {
											// all children after this If statement can be eliminated
											node.children.erase(node.children.begin() + i + 1, node.children.end());
											return true;
										}
										if (this_if.children[1].guarranteed_return()) {
											// move all children after this If statement into the Else statement
											auto new_else_statement = Block_Node("", this_if.location, { this_if.children[2] });
											new_else_statement.children.insert(new_else_statement.children.end(), node.children.begin() + i + 1, node.children.end());
											this_if.children[2] = new_else_statement;
											node.children.erase(node.children.begin() + i + 1, node.children.end());
											return true;
										}
										if (this_if.children[2].guarranteed_return()) {
											// move all children after this If statement into the Then statement
											auto new_then_statement = Block_Node("", this_if.location, { this_if.children[1] });
											new_then_statement.children.insert(new_then_statement.children.end(), node.children.begin() + i + 1, node.children.end());
											this_if.children[1] = new_then_statement;
											node.children.erase(node.children.begin() + i + 1, node.children.end());
											return true;
										}
									}
								}
							}
						}

						return false;
					}
				};

				// Re-order or collapse if statements.
				struct Switch {
					bool optimize(AbstractSyntaxTreeNode& node) {
						// Pre-compile the hash table for a Switch statement if possible. 
						if ((node.identifier == Engine::AST_Node_Type::Switch)
							&& (node.children.size() > 1)
							&& (node.tag.cast< SwitchInformation>().is_precompiled == false)
							&& (node.tag.cast< SwitchInformation>().allow_precompilation == true)
						) {
							// early-exit if there are any non-const case switches
							for (int i = 1; i < node.children.size(); ++i) {
								auto& child = node.children[i];
								if (child.identifier == Engine::AST_Node_Type::Case
									&& child.children.size() == 2
									&& child.children[0].identifier == Engine::AST_Node_Type::Constant
								) {
								
								}
								else if (child.identifier == Engine::AST_Node_Type::Default) {

								}
								else {
									return false;
								}
							}
							// we know all switches are either const, or are the default. 
							try {
								for (int i = 1; i < node.children.size(); ++i) {
									auto& child = node.children[i];
									if (child.identifier == Engine::AST_Node_Type::Case
										&& child.children.size() == 2
										&& child.children[0].identifier == Engine::AST_Node_Type::Constant
									) {
										size_t this_hash = CurrentEngine().call<size_t>("to_hash", { child.children[0].constant | GL::type::Const | GL::type::Reference });
										node.tag.cast< SwitchInformation>().hash_to_child_index[this_hash] = i;
									}
									else if (child.identifier == Engine::AST_Node_Type::Default) {
										node.tag.cast< SwitchInformation>().default_child_index = i;
									}
								}
								node.tag.cast< SwitchInformation>().is_precompiled = true;
								return true;
							}
							catch (...) { 
								node.tag.cast< SwitchInformation>().allow_precompilation = false;
								return false; 
							}
						}

						// Resolve a Switch statement into a PreprocessedSwitch statement, if possible.
						if ((node.identifier == Engine::AST_Node_Type::Switch)
							&& (node.children.size() > 1)
							&& (node.tag.cast< SwitchInformation>().is_precompiled == true)
							&& (node.tag.cast< SwitchInformation>().allow_precompilation == true)
							&& (node.children[0].identifier == Engine::AST_Node_Type::Constant)
						) {
							size_t starting_child_index = 1;
							try {
								size_t this_hash = CurrentEngine().call<size_t>("to_hash", { node.children[0].constant | GL::type::Const | GL::type::Reference });
								if (auto f = node.tag.cast< SwitchInformation>().hash_to_child_index.find(this_hash), e = node.tag.cast< SwitchInformation>().hash_to_child_index.end(); f != e) {
									// from the found index on, compile into a block
									starting_child_index = f->second;
								}							
								else {
									starting_child_index = 0;
									if (node.tag.cast< SwitchInformation>().default_child_index < node.children.size()) {
										starting_child_index = node.tag.cast< SwitchInformation>().default_child_index;
									}
									// the hash did not match. See if we can evaluate the "==" operation. 
									for (int i = 1; i < node.children.size(); ++i) {
										auto& child = node.children[i];
										if (child.identifier == Engine::AST_Node_Type::Case
											&& child.children.size() == 2
											&& child.children[0].identifier == Engine::AST_Node_Type::Constant
										) {
											if (GL::any::fast_any out; AttemptCalculation("==", { node.children[0].constant | GL::type::Const | GL::type::Reference, child.children[0].constant | GL::type::Const | GL::type::Reference }, out)) {
												if (out.cast<bool>()) {
													starting_child_index = i;
													break;
												}
											}
											else {
												// precompilation failed
												node.tag.cast< SwitchInformation>().allow_precompilation = false;
												return false;
											}
										}
									}
									if (starting_child_index == 0) {
										// no matching hash, AND, no default. Therefore, this should compile down to nothing. 
										node = Noop_Node("", {}, {});
										return true;
									}
								}
							}
							catch (...) {
								node.tag.cast< SwitchInformation>().allow_precompilation = false;
								return false;
							}

							std::vector<AbstractSyntaxTreeNode> out;
							for (size_t i = starting_child_index; i < node.children.size(); ++i) {
								if (node.children[i].identifier == Engine::AST_Node_Type::Case) {
									if (node.children[i].children.size() == 2) {
										out.push_back(node.children[i].children[1]);
										continue;
									}
								}
								else if (node.children[i].identifier == Engine::AST_Node_Type::Default) {
									if (node.children[i].children.size() == 1) {
										out.push_back(node.children[i].children[0]);
										continue;
									}
								}
								node.tag.cast< SwitchInformation>().allow_precompilation = false;
								return false;
							}

							node = PreprocessedSwitch_Node("", node.location, { Block_Node("", node.location, out) });
							return true;
						}

						// Reduce a PreprocessedSwitch by eliminating any nodes after an in-line break.
						if (node.identifier == Engine::AST_Node_Type::PreprocessedSwitch) {
							std::deque< AbstractSyntaxTreeNode*> parents;
							bool found = false;
							node.for_each_child([&](AbstractSyntaxTreeNode& this_child) -> bool {
								if (found) return false;
								switch (this_child.identifier) {
								case Engine::AST_Node_Type::DeclarationBlock:
								case Engine::AST_Node_Type::Do:
								case Engine::AST_Node_Type::Try:
								case Engine::AST_Node_Type::Parallel_Ranged_For:
								case Engine::AST_Node_Type::Parallel_For:
								case Engine::AST_Node_Type::FunctionBlock:
								case Engine::AST_Node_Type::If:
								case Engine::AST_Node_Type::Switch:
								case Engine::AST_Node_Type::For:
								case Engine::AST_Node_Type::Ranged_For:
								case Engine::AST_Node_Type::While:
									return false;
								case Engine::AST_Node_Type::Break: {
									this_child = Noop_Node("break", this_child.location, {});

									AbstractSyntaxTreeNode* current_parent;
									AbstractSyntaxTreeNode* current_node = &this_child;
									while (parents.size() > 0) {
										current_parent = parents.back();
										if (current_parent->identifier == Engine::AST_Node_Type::Block
											|| current_parent->identifier == Engine::AST_Node_Type::Scopeless_Block
											|| current_parent->identifier == Engine::AST_Node_Type::File
										) {
											// find the current node
											int i;
											for (i = 0; i < current_parent->children.size(); ++i) {
												if (&current_parent->children[i] == current_node) { // found
													break;
												}
											}
											if (i >= current_parent->children.size()) throw except::eval_error("Optimization failed for unknown reason", this_child.location);

											for (++i; i < current_parent->children.size(); ++i) {
												current_parent->children[i] = Noop_Node("break", current_parent->children[i].location, {});
											}
										}

										current_node = current_parent;
										parents.pop_back();
									}
									found = true;
									return false;
								};
								default:
									return true;
								}
							}, [&](AbstractSyntaxTreeNode& this_child) {
								if (!found) {
									parents.push_back(&this_child);
								}
							}, [&](AbstractSyntaxTreeNode&) {
								if (!found) {
									parents.pop_back();
								}
							});
							if (found) {
								return true;
							}
						}

						// Reduce a PreprocessedSwitch that does not call "break"
						if (node.identifier == Engine::AST_Node_Type::PreprocessedSwitch
							&& node.children.size() == 1
						) {
							bool found = false;
							node.for_each_child([&](AbstractSyntaxTreeNode& this_child) -> bool {
								if (found) return false;
								switch (this_child.identifier) {
								case Engine::AST_Node_Type::DeclarationBlock:
								case Engine::AST_Node_Type::Do:
								case Engine::AST_Node_Type::Try:
								case Engine::AST_Node_Type::Parallel_Ranged_For:
								case Engine::AST_Node_Type::Parallel_For:
								case Engine::AST_Node_Type::FunctionBlock:								
								case Engine::AST_Node_Type::Switch:
								case Engine::AST_Node_Type::For:
								case Engine::AST_Node_Type::Ranged_For:
								case Engine::AST_Node_Type::While:
									return false;
								case Engine::AST_Node_Type::Break: 
									found = true;
									return false;
								default:
									return true;
								}
							}, [&](AbstractSyntaxTreeNode& this_child) {

							}, [&](AbstractSyntaxTreeNode& this_child) {

							});
							if (!found) {
								node = node.children[0];
								return true;
							}
						}

						return false;
					}
				};

				// Try to fold a basic prefix operation with a constant value
				struct PrefixFold {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Prefix
							&& node.children.size() == 1
							&& node.children[0].identifier == Engine::AST_Node_Type::Constant
						) {
							const GL::any::fast_any& rhs = node.children[0].constant;
							if (node.text == "++") {
								if (GL::any::fast_any out; AttemptCalculation("+", { rhs, GL::any::fast_any::instance(1) }, out)) {
									node = Constant_Node(node.text, (Parse_Location)node.location, {}, out);
									return true;
								}
							}
							else if (node.text == "--") {
								if (GL::any::fast_any out; AttemptCalculation("-", { rhs, GL::any::fast_any::instance(1) }, out)) {
									node = Constant_Node(node.text, (Parse_Location)node.location, {}, out);
									return true;
								}
							}
							else if (node.text == "-") {
								if (GL::any::fast_any out; AttemptCalculation("*", { rhs, GL::any::fast_any::instance(-1) }, out)) {
									node = Constant_Node(node.text, (Parse_Location)node.location, {}, out);
									return true;
								}
							}
							else if (node.text == "!") {
								if (GL::any::fast_any out; AttemptCalculation("bool", { rhs }, out)) {
									try {
										node = Constant_Node(node.text, (Parse_Location)node.location, {}, !out.cast<bool>());
										return true;
									}
									catch (...) {
										return false;
									}
								}
							}
						}
						return false;
					}
				};

				// postfix's (++/--) on constant values should simply return the same constant value before the change anyways. 
				struct PostfixFold {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Postfix
							&& node.children.size() == 1
							&& node.children[0].identifier == Engine::AST_Node_Type::Constant
						) {
							if ((node.text == "++") || (node.text == "--")) {
								// ++/-- as a post-fix on constant values should simply return the same constant value before the change anyways. 
								node = node.children[0];
								return true;
							}
							else {
								// handle constexpr compilations of unit types
								if (auto iter = GL::value::abbreviations_to_type().find(node.text); iter != GL::value::abbreviations_to_type().end()) {
									if (auto* BC = CurrentEngine().try_find_class(iter->second.m_casted_type); BC && BC->this_m.is_class()) {
										try {
											auto result = BC->this_m.scope->call(BC->this_m.scope_name, { node.children[0].constant | GL::type::Const | GL::type::Reference });
											node = Constant_Node(node.text, node.location, {}, result);
											return true;
										}
										catch (...) {
											return false;
										}
									}
								}
							}
						}
	#if 0
						if (node.identifier == Engine::AST_Node_Type::Postfix
							&& node.children.size() == 1
							&& !((node.text == "++") || (node.text == "--"))
						) {
							// re-order the postfix into a Fun_Call. This speeds-up the process by compiling-out the search for the unit type based on the abbreviation.
							if (auto iter = GL::value::abbreviations_to_type().find(node.text); iter != GL::value::abbreviations_to_type().end()) {
								if (auto* BC = CurrentEngine().try_find_class(iter->second.m_casted_type); BC && BC->this_m.is_class()) {
									node = Fun_Call_Node("", {}, {
										Id_Node(BC->this_m.scope_name, node.location, {}),
										Arg_List_Node("", node.location, node.children)
									});
									return true;
								}
							}
						}
	#endif
						if ((node.identifier == Engine::AST_Node_Type::Fun_Call)
							&& (node.children.size() == 2)
							&& (node.children[0].identifier == Engine::AST_Node_Type::Id)							
							&& (node.children[0].children.size() == 0)
							&& (node.children[1].identifier == Engine::AST_Node_Type::Arg_List)
							&& (node.children[1].children.size() == 1)
							&& (node.children[1].children[0].identifier == Engine::AST_Node_Type::Id)
							&& (node.children[1].children[0].children.size() == 0)
							&& (node.children[1].children[0].text[0] == '_')
						) {
							GL::string const& variable_name = node.children[0].text;
							GL::string const& abbreviation = node.children[1].children[0].text;

							if (auto iter = GL::value::abbreviations_to_type().find(abbreviation.right_of("_")); iter != GL::value::abbreviations_to_type().end()) {
								if (auto* BC = CurrentEngine().try_find_class(iter->second.m_casted_type); BC && BC->this_m.is_class()) {
									node = Postfix_Node(abbreviation, node.location, { node.children[0] });
									node.tag.cast<PostfixInformation>().is_unit = true;
									node.tag.cast<PostfixInformation>().unit_name = BC->this_m.scope_name;
									return true;
								}
							}

						}

						return false;
					}
				};

				// Try to fold a basic binary operation between two constant values (e.g. GL::string + GL::string, or Units::foot == Units::meter)
				struct BinaryFold {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Binary
							|| node.identifier == Engine::AST_Node_Type::BinaryFoldRight
							|| node.identifier == Engine::AST_Node_Type::BinaryFoldLeft
						) {
							if (node.identifier == Engine::AST_Node_Type::Binary
								&& node.children.size() == 2
								&& node.children[0].identifier == Engine::AST_Node_Type::Constant
								&& node.children[1].identifier == Engine::AST_Node_Type::Constant
								) {
								auto& lhs = node.children[0].constant;
								auto& rhs = node.children[1].constant;
								if (GL::any::fast_any out; AttemptCalculation(node.text, { lhs | GL::type::Const | GL::type::Reference, rhs | GL::type::Const | GL::type::Reference }, out)) {
									node = Constant_Node(node.text, (Parse_Location)node.location, {}, out);
									return true;
								}
								return false;
							}

							if (node.identifier == Engine::AST_Node_Type::BinaryFoldRight
								&& node.children.size() == 1
								&& node.children[0].identifier == Engine::AST_Node_Type::Constant
								) {
								auto& lhs = node.children[0].constant;
								auto& rhs = node.constant;
								if (GL::any::fast_any out; AttemptCalculation(node.text, { lhs | GL::type::Const | GL::type::Reference, rhs | GL::type::Const | GL::type::Reference }, out)) {
									node = Constant_Node(node.text, (Parse_Location)node.location, {}, out);
									return true;
								}
								return false;
							}

							if (node.identifier == Engine::AST_Node_Type::BinaryFoldLeft
								&& node.children.size() == 1
								&& node.children[0].identifier == Engine::AST_Node_Type::Constant
								) {
								auto& lhs = node.constant;
								auto& rhs = node.children[0].constant;
								if (GL::any::fast_any out; AttemptCalculation(node.text, { lhs | GL::type::Const | GL::type::Reference, rhs | GL::type::Const | GL::type::Reference }, out)) {
									node = Constant_Node(node.text, (Parse_Location)node.location, {}, out);
									return true;
								}
								return false;
							}
						}
						return false;
					}
				};

				struct LogicalBinaryFold {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Logical_Or
							&& node.children.size() == 2
							&& node.children[0].identifier == Engine::AST_Node_Type::Constant
							&& node.children[1].identifier == Engine::AST_Node_Type::Constant
						) {
							auto& lhs = node.children[0].constant;
							auto& rhs = node.children[1].constant;
							if (GL::any::fast_any out; AttemptCalculation("||", { lhs | GL::type::Const | GL::type::Reference, rhs | GL::type::Const | GL::type::Reference }, out)) {
								node = Constant_Node(node.text, (Parse_Location)node.location, {}, out);
								return true;
							}
							return false;
						}
						if (node.identifier == Engine::AST_Node_Type::Logical_And
							&& node.children.size() == 2
							&& node.children[0].identifier == Engine::AST_Node_Type::Constant
							&& node.children[1].identifier == Engine::AST_Node_Type::Constant
						) {
							auto& lhs = node.children[0].constant;
							auto& rhs = node.children[1].constant;
							if (GL::any::fast_any out; AttemptCalculation("&&", { lhs | GL::type::Const | GL::type::Reference, rhs | GL::type::Const | GL::type::Reference }, out)) {
								node = Constant_Node(node.text, (Parse_Location)node.location, {}, out);
								return true;
							}
							return false;
						}
						return false;
					};
				};

				// Try to fold a basic binary operation (e.g. +/-/*) with one constant value, to speed-up evaluation in the future
				struct PartialBinaryFold {
					static bool is_numeric(AbstractSyntaxTreeNode const& rhs) {
						if (rhs.identifier == GL::Engine::AST_Node_Type::Constant
							|| ((rhs.identifier == GL::Engine::AST_Node_Type::Id) && rhs.constant)
						) {
							return rhs.constant.can_cast(GL::type_of<GL::value>())
								|| rhs.constant.can_cast(GL::type_of<float>())
								|| rhs.constant.can_cast(GL::type_of<double>())
								|| rhs.constant.can_cast(GL::type_of<long>())
								|| rhs.constant.can_cast(GL::type_of<long long>())
								|| rhs.constant.can_cast(GL::type_of<long double>())
								|| rhs.constant.can_cast(GL::type_of<int>())
								|| rhs.constant.can_cast(GL::type_of<unsigned int>())
								|| rhs.constant.can_cast(GL::type_of<unsigned long>())
								|| rhs.constant.can_cast(GL::type_of<unsigned long long>())
								|| rhs.constant.can_cast(GL::type_of<char>())
								|| rhs.constant.can_cast(GL::type_of<unsigned char>());
						}
						return false;
					}
					static bool equals(AbstractSyntaxTreeNode const& rhs, GL::value V) {
						if (rhs.identifier == GL::Engine::AST_Node_Type::Constant
							|| ((rhs.identifier == GL::Engine::AST_Node_Type::Id) && rhs.constant)
						) {
							if (rhs.constant.can_cast(GL::type_of<GL::value>())) {
								return rhs.constant.cast<GL::value>() == V;
							}
							else if (rhs.constant.can_cast(GL::type_of<float>())) {
								return rhs.constant.cast<float>() == (float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<double>())) {
								return rhs.constant.cast<double>() == (double)(float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<long>())) {
								return rhs.constant.cast<long>() == (long)(float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<long long>())) {
								return rhs.constant.cast<long long>() == (long long)(float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<long double>())) {
								return rhs.constant.cast<long double>() == (long double)(float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<int>())) {
								return rhs.constant.cast<int>() == (int)(float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<unsigned int>())) {
								return rhs.constant.cast<unsigned int>() == (unsigned int)(float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<unsigned long>())) {
								return rhs.constant.cast<unsigned long>() == (unsigned long)(float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<unsigned long long>())) {
								return rhs.constant.cast<unsigned long long>() == (unsigned long long)(float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<char>())) {
								return rhs.constant.cast<char>() == (char)(float)(V);
							}
							else if (rhs.constant.can_cast(GL::type_of<unsigned char>())) {
								return rhs.constant.cast<unsigned char>() == (unsigned char)(float)(V);
							}
						}
						return false;
					}

					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Binary
							|| node.identifier == Engine::AST_Node_Type::BinaryFoldRight
							|| node.identifier == Engine::AST_Node_Type::BinaryFoldLeft
						) {
							// Fold right side
							if (node.identifier == Engine::AST_Node_Type::Binary
								&& node.children.size() == 2
								&& node.children[0].identifier != Engine::AST_Node_Type::Constant
								&& node.children[1].identifier == Engine::AST_Node_Type::Constant
							) {
								const auto& oper = node.text;
								const auto parsed = Engine::Operators::to_operator(oper.c_str());
								if (parsed != Engine::Operators::Opers::invalid) {
									auto& rhs = node.children[1].constant;
									node = Fold_Right_Binary_Operator_Node(node.text, std::move(node.location), { node.children[0] }, rhs);
									return true;
								}
							}

							// Fold left side
							if (node.identifier == Engine::AST_Node_Type::Binary
								&& node.children.size() == 2
								&& node.children[0].identifier == Engine::AST_Node_Type::Constant
								&& node.children[1].identifier != Engine::AST_Node_Type::Constant
							) {
								const auto& oper = node.text;
								const auto parsed = Engine::Operators::to_operator(oper.c_str());
								if (parsed != Engine::Operators::Opers::invalid) {
									auto& rhs = node.children[0].constant;
									node = Fold_Left_Binary_Operator_Node(node.text, std::move(node.location), { node.children[1] }, rhs);
									return true;
								}
							}

							// Reduction of unecessary binary operations
							if (((node.identifier == Engine::AST_Node_Type::BinaryFoldRight) || (node.identifier == Engine::AST_Node_Type::BinaryFoldLeft))
								&& node.children.size() == 1
								&& node.text == "+"
								&& is_numeric(node)
							) {
								if (equals(node, 0)) {
									node = node.children[0];
									return true;
								}
							}

							if (node.identifier == Engine::AST_Node_Type::BinaryFoldRight
								&& node.children.size() == 1
								&& node.text == "-"
								&& is_numeric(node)
							) {
								if (equals(node, 0)) {
									node = node.children[0];
									return true;
								}
							}

							// removal of "appending" empty strings
							if (((node.identifier == Engine::AST_Node_Type::BinaryFoldLeft) || (node.identifier == Engine::AST_Node_Type::BinaryFoldRight))
								&& node.children.size() == 1
								&& node.text == "+"
								&& node.constant.can_cast(GL::type_of<GL::string>())
							) {
								if (node.constant.cast<GL::string>().empty()) {
									node = node.children[0];
									return true;
								}						
							}

							// Left-Fold Within Right-Fold
							if (node.identifier == Engine::AST_Node_Type::BinaryFoldRight
								&& node.children.size() == 1
								&& node.children[0].identifier == Engine::AST_Node_Type::BinaryFoldLeft
								&& node.children[0].children.size() == 1
								&& is_numeric(node)
								&& is_numeric(node.children[0])
							) {
								GL::Engine::Operators::Opers outter_oper = node.tag.cast<GL::Engine::Operators::Opers>();
								GL::Engine::Operators::Opers inner_oper = node.children[0].tag.cast<GL::Engine::Operators::Opers>();
						
								GL::string const_operation;
								GL::string runtime_operation;
								GL::any::fast_any lhs = node.children[0].constant;
								GL::any::fast_any rhs = node.constant;
								const_operation = node.text; // outter operation
								runtime_operation = node.children[0].text; // inner operation

								switch (outter_oper) {
								case GL::Engine::Operators::to_operator("+"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// no issues
										break;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// no issues
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("-"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// no issues
										break;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// no issues
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("*"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("*"): {
										// no issues
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// no issues
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("/"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("*"): {
										// no issues
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// no issues
										break;
									}
									default: return false;
									}
									break;
								}
								default: return false;
								}

								if (GL::any::fast_any out; AttemptCalculation(const_operation, { lhs | GL::type::Const | GL::type::Reference, rhs | GL::type::Const | GL::type::Reference }, out)) {
									node = Fold_Left_Binary_Operator_Node(runtime_operation, (Parse_Location)node.location, { node.children[0].children[0] }, out);
									return true;
								}
								return false;
							}

							// Right-Fold Within Right-Fold
							if (node.identifier == Engine::AST_Node_Type::BinaryFoldRight
								&& node.children.size() == 1
								&& node.children[0].identifier == Engine::AST_Node_Type::BinaryFoldRight
								&& node.children[0].children.size() == 1
								&& is_numeric(node)
								&& is_numeric(node.children[0])
							) {
								GL::Engine::Operators::Opers outter_oper = node.tag.cast<GL::Engine::Operators::Opers>();
								GL::Engine::Operators::Opers inner_oper = node.children[0].tag.cast<GL::Engine::Operators::Opers>();

								GL::string const_operation;
								GL::string runtime_operation;
								GL::any::fast_any lhs = node.children[0].constant;
								GL::any::fast_any rhs = node.constant;
								const_operation = node.text; // outter operation
								runtime_operation = node.children[0].text; // inner operation

								switch (outter_oper) {
								case GL::Engine::Operators::to_operator("+"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (x+C)+C
										break;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (x-C)+C
										rhs = node.children[0].constant;
										lhs = node.constant;
										const_operation = "-";								
										runtime_operation = "+";
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("-"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (x+C)-C
										break;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (x-C)-C
										const_operation = "+";
										runtime_operation = "-";
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("*"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("*"): {
										// (x*C)*C
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (x/C)*C
										rhs = node.children[0].constant;
										lhs = node.constant;
										const_operation = "/";
										runtime_operation = "*";
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("/"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("*"): {
										// (x*C)/C
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (x/C)/C
										const_operation = "*";
										runtime_operation = "/";
										break;
									}
									default: return false;
									}
									break;
								}
								default: return false;
								}

								if (GL::any::fast_any out; AttemptCalculation(const_operation, { lhs | GL::type::Const | GL::type::Reference, rhs | GL::type::Const | GL::type::Reference }, out)) {
									node = Fold_Right_Binary_Operator_Node(runtime_operation, (Parse_Location)node.location, { node.children[0].children[0] }, out);
									return true;
								}
								return false;
							}

							// Left-Fold Within Left-Fold
							if (node.identifier == Engine::AST_Node_Type::BinaryFoldLeft
								&& node.children.size() == 1
								&& node.children[0].identifier == Engine::AST_Node_Type::BinaryFoldLeft
								&& node.children[0].children.size() == 1
								&& is_numeric(node)
								&& is_numeric(node.children[0])
							) {
								GL::Engine::Operators::Opers outter_oper = node.tag.cast<GL::Engine::Operators::Opers>();
								GL::Engine::Operators::Opers inner_oper = node.children[0].tag.cast<GL::Engine::Operators::Opers>();

								GL::string const_operation;
								GL::string runtime_operation;
								GL::any::fast_any lhs = node.children[0].constant;
								GL::any::fast_any rhs = node.constant;
								const_operation = node.text; // outter operation
								runtime_operation = node.children[0].text; // inner operation

								switch (outter_oper) {
								case GL::Engine::Operators::to_operator("+"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// 10+(10+y)
										break;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// 10+(10-y)
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("-"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// 10-(10+y)
										const_operation = "-";
										runtime_operation = "-";
										break;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// 10-(10-y)
										const_operation = "-";
										runtime_operation = "+";
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("*"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("*"): {
										// 10*(10*y)
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// 10*(10/y)
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("/"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("*"): {
										// 10/(10*y)
										const_operation = "/";
										runtime_operation = "/";
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// 10/(10/y)
										const_operation = "/";
										runtime_operation = "*";
										break;
									}
									default: return false;
									}
									break;
								}
								default: return false;
								}

								if (GL::any::fast_any out; AttemptCalculation(const_operation, { lhs | GL::type::Const | GL::type::Reference, rhs | GL::type::Const | GL::type::Reference }, out)) {
									node = Fold_Left_Binary_Operator_Node(runtime_operation, (Parse_Location)node.location, { node.children[0].children[0] }, out);
									return true;
								}
								return false;
							}

							// Right-Fold Within Left-Fold
							if (node.identifier == Engine::AST_Node_Type::BinaryFoldLeft
								&& node.children.size() == 1
								&& node.children[0].identifier == Engine::AST_Node_Type::BinaryFoldRight
								&& node.children[0].children.size() == 1
								&& is_numeric(node)
								&& is_numeric(node.children[0])
							) {
								GL::Engine::Operators::Opers outter_oper = node.tag.cast<GL::Engine::Operators::Opers>();
								GL::Engine::Operators::Opers inner_oper = node.children[0].tag.cast<GL::Engine::Operators::Opers>();

								GL::string const_operation;
								GL::string runtime_operation;
								GL::any::fast_any lhs = node.children[0].constant;
								GL::any::fast_any rhs = node.constant;
								const_operation = node.text; // outter operation
								runtime_operation = node.children[0].text; // inner operation
								bool fold_left = false;

								switch (outter_oper) {
								case GL::Engine::Operators::to_operator("+"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// 10+(y+10)
										break;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// 10+(y-10)
										const_operation = "-";
										runtime_operation = "+";
										// y+(10-10)
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("-"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// 10-(y+10)
										const_operation = "-";
										runtime_operation = "-";
										fold_left = true;
										// (10-10)-y
										break;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// 10-(y-10)
										const_operation = "+";
										runtime_operation = "-";
										fold_left = true;
										// (10+10)-y
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("*"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("*"): {
										// 10*(y*10)
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// 10*(y/10)
										const_operation = "/";
										runtime_operation = "*";
										// y*(10/10)
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("/"): {
									switch (inner_oper) {
									case GL::Engine::Operators::to_operator("*"): {
										// 10/(y*10)
										const_operation = "/";
										runtime_operation = "/";
										fold_left = true;
										// (10/10)/y
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// 10/(y/10)
										const_operation = "*";
										runtime_operation = "/";
										fold_left = true;
										// (10*10)/y
										break;
									}
									default: return false;
									}
									break;
								}
								default: return false;
								}

								if (GL::any::fast_any out; AttemptCalculation(const_operation, { lhs | GL::type::Const | GL::type::Reference, rhs | GL::type::Const | GL::type::Reference }, out)) {
									if (fold_left) {
										node = Fold_Left_Binary_Operator_Node(runtime_operation, (Parse_Location)node.location, { node.children[0].children[0] }, out);
									}
									else {
										node = Fold_Right_Binary_Operator_Node(runtime_operation, (Parse_Location)node.location, { node.children[0].children[0] }, out);
									}
									return true;
								}
								return false;
							}

							if ((node.identifier == Engine::AST_Node_Type::Binary)
								&& (node.children.size() == 2)
								&& ((node.children[0].identifier == Engine::AST_Node_Type::BinaryFoldRight) || (node.children[0].identifier == Engine::AST_Node_Type::BinaryFoldLeft))
								&& (node.children[0].children.size() == 1)
								&& (node.children[0].children[0].identifier == Engine::AST_Node_Type::Id)
								&& (node.children[0].children[0].children.size() == 0)
								&& ((node.children[1].identifier == Engine::AST_Node_Type::BinaryFoldRight) || (node.children[1].identifier == Engine::AST_Node_Type::BinaryFoldLeft))
								&& (node.children[1].children.size() == 1)
								&& (node.children[1].children[0].identifier == Engine::AST_Node_Type::Id)
								&& (node.children[1].children[0].children.size() == 0)
								&& (node.children[0].children[0].get_text() == node.children[1].children[0].get_text())
								&& is_numeric(node.children[0])
								&& is_numeric(node.children[1])
							) {
								GL::Engine::Operators::Opers& outter_oper = node.tag.cast<GL::Engine::Operators::Opers>();
								GL::Engine::Operators::Opers& LHS_oper = node.children[0].tag.cast<GL::Engine::Operators::Opers>();
								GL::Engine::Operators::Opers& RHS_oper = node.children[1].tag.cast<GL::Engine::Operators::Opers>();
								AbstractSyntaxTreeNode& IdNode = node.children[0].children[0];
								GL::any::fast_any& LHS = node.children[0].constant;
								GL::any::fast_any& RHS = node.children[1].constant;
								GL::string const_operation;
								GL::string runtime_operation;
								bool fold_left = true;

								if (outter_oper == GL::Engine::Operators::to_operator("+")
									&& LHS_oper == GL::Engine::Operators::to_operator("+")
									&& RHS_oper == GL::Engine::Operators::to_operator("+")
								) {
									// (10+y)+(10+y)
									const_operation = "+";
									runtime_operation = "+";
									fold_left = true;
									// (20)+y
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("+")
									&& LHS_oper == GL::Engine::Operators::to_operator("*")
									&& RHS_oper == GL::Engine::Operators::to_operator("*")
								) {
									// (10*y)+(10*y)
									const_operation = "+";
									runtime_operation = "*";
									fold_left = true;
									// (20)*y
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("-")
									&& LHS_oper == GL::Engine::Operators::to_operator("*")
									&& RHS_oper == GL::Engine::Operators::to_operator("*")
								) {
									// (10*y)-(5*y)
									const_operation = "-";
									runtime_operation = "*";
									fold_left = true;
									// (5)*y
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("+")
									&& LHS_oper == GL::Engine::Operators::to_operator("*")
									&& RHS_oper == GL::Engine::Operators::to_operator("+")
								) {
									// (10*y)+(5+y)
									// reduces to:
									// (10+1)*y + 5

									if (GL::any::fast_any new_LHS; AttemptCalculation("+", { LHS | GL::type::Const | GL::type::Reference, GL::any::fast_any::instance(1) | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Fold_Right_Binary_Operator_Node("+", (Parse_Location)node.location, {
											Fold_Left_Binary_Operator_Node("*", (Parse_Location)node.location, { IdNode }, new_LHS)
										}, RHS);
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("-")
									&& LHS_oper == GL::Engine::Operators::to_operator("*")
									&& RHS_oper == GL::Engine::Operators::to_operator("+")
								) {
									// (10*y)-(5+y)
									// reduces to:
									// (10-1)*y - 5

									if (GL::any::fast_any new_LHS; AttemptCalculation("-", { LHS | GL::type::Const | GL::type::Reference, GL::any::fast_any::instance(1) | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Fold_Right_Binary_Operator_Node("-", (Parse_Location)node.location, {
											Fold_Left_Binary_Operator_Node("*", (Parse_Location)node.location, { IdNode }, new_LHS)
										}, RHS);
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("+")
									&& LHS_oper == GL::Engine::Operators::to_operator("*")
									&& RHS_oper == GL::Engine::Operators::to_operator("-")
								) {
									// (10*y)+(5-y)
									// reduces to:
									// (10-1)*y + 5

									if (GL::any::fast_any new_LHS; AttemptCalculation("-", { LHS | GL::type::Const | GL::type::Reference, GL::any::fast_any::instance(1) | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Fold_Right_Binary_Operator_Node("+", (Parse_Location)node.location, {
											Fold_Left_Binary_Operator_Node("*", (Parse_Location)node.location, { IdNode }, new_LHS)
										}, RHS);
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("-")
									&& LHS_oper == GL::Engine::Operators::to_operator("*")
									&& RHS_oper == GL::Engine::Operators::to_operator("-")
								) {
									// (10*y)-(5-y)
									// reduces to:
									// (10+1)*y - 5

									if (GL::any::fast_any new_LHS; AttemptCalculation("+", { LHS | GL::type::Const | GL::type::Reference, GL::any::fast_any::instance(1) | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Fold_Right_Binary_Operator_Node("-", (Parse_Location)node.location, {
											Fold_Left_Binary_Operator_Node("*", (Parse_Location)node.location, { IdNode }, new_LHS)
										}, RHS);
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("-")
									&& LHS_oper == GL::Engine::Operators::to_operator("-")
									&& RHS_oper == GL::Engine::Operators::to_operator("-")
								) {
									// (10-y)-(5-y)
									// reduces to:
									// (10-5)-y+y
									// reduces to:
									// (10-5)

									if (GL::any::fast_any new_LHS; AttemptCalculation("-", { LHS | GL::type::Const | GL::type::Reference, RHS | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Constant_Node("", (Parse_Location)node.location, {}, new_LHS);
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("+")
									&& LHS_oper == GL::Engine::Operators::to_operator("-")
									&& RHS_oper == GL::Engine::Operators::to_operator("-")
								) {
									// (10-y)+(5-y)
									// reduces to:
									// ((10+5)-y)-y

									if (GL::any::fast_any new_LHS; AttemptCalculation("+", { LHS | GL::type::Const | GL::type::Reference, RHS | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Binary_Operator_Node("-", (Parse_Location)node.location, {
											Fold_Left_Binary_Operator_Node("-", (Parse_Location)node.location, { IdNode }, new_LHS),
											IdNode
										});
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("-")
									&& LHS_oper == GL::Engine::Operators::to_operator("+")
									&& RHS_oper == GL::Engine::Operators::to_operator("+")
								) {
									// (10+y)-(5+y)
									// reduces to:
									// (10-5)

									if (GL::any::fast_any new_LHS; AttemptCalculation("-", { LHS | GL::type::Const | GL::type::Reference, RHS | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Constant_Node("", (Parse_Location)node.location, {}, new_LHS);
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("-")
									&& LHS_oper == GL::Engine::Operators::to_operator("+")
									&& RHS_oper == GL::Engine::Operators::to_operator("-")
								) {
									// (10+y)-(5-y)
									// reduces to:
									// (10-5)+y+y

									if (GL::any::fast_any new_LHS; AttemptCalculation("-", { LHS | GL::type::Const | GL::type::Reference, RHS | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Binary_Operator_Node("+", (Parse_Location)node.location, {
											Fold_Left_Binary_Operator_Node("+", (Parse_Location)node.location, { IdNode }, new_LHS),
											IdNode
										});
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("-")
									&& LHS_oper == GL::Engine::Operators::to_operator("-")
									&& RHS_oper == GL::Engine::Operators::to_operator("+")
								) {
									// (10-y)-(5+y)
									// reduces to:
									// (10-5)-y-y

									if (GL::any::fast_any new_LHS; AttemptCalculation("-", { LHS | GL::type::Const | GL::type::Reference, RHS | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Binary_Operator_Node("-", (Parse_Location)node.location, {
											Fold_Left_Binary_Operator_Node("-", (Parse_Location)node.location, { IdNode }, new_LHS),
											IdNode
										});
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("+")
									&& LHS_oper == GL::Engine::Operators::to_operator("-")
									&& RHS_oper == GL::Engine::Operators::to_operator("+")
								) {
									// (10-y)+(5+y)
									// reduces to:
									// 10+5

									if (GL::any::fast_any new_LHS; AttemptCalculation("+", { LHS | GL::type::Const | GL::type::Reference, RHS | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Constant_Node("", (Parse_Location)node.location, {}, new_LHS);
										return true;
									}
									return false;
								}
								else if (outter_oper == GL::Engine::Operators::to_operator("+")
									&& LHS_oper == GL::Engine::Operators::to_operator("+")
									&& RHS_oper == GL::Engine::Operators::to_operator("-")
								) {
									// (10+y)+(5-y)
									// reduces to:
									// 10+5

									if (GL::any::fast_any new_LHS; AttemptCalculation("+", { LHS | GL::type::Const | GL::type::Reference, RHS | GL::type::Const | GL::type::Reference }, new_LHS)) {
										node = Constant_Node("", (Parse_Location)node.location, {}, new_LHS);
										return true;
									}
									return false;
								}

								if (const_operation.empty() || runtime_operation.empty()) return false;

								if (GL::any::fast_any out; AttemptCalculation(const_operation, { LHS | GL::type::Const | GL::type::Reference, RHS | GL::type::Const | GL::type::Reference }, out)) {
									if (fold_left) {
										node = Fold_Left_Binary_Operator_Node(runtime_operation, (Parse_Location)node.location, { IdNode }, out);
									}
									else {
										node = Fold_Right_Binary_Operator_Node(runtime_operation, (Parse_Location)node.location, { IdNode }, out);
									}
									return true;
								}
								return false;
							}
					
							if (node.identifier == Engine::AST_Node_Type::Binary
								&& node.children.size() == 2
								&& node.children[0].identifier == Engine::AST_Node_Type::BinaryFoldRight
								&& node.children[1].identifier == Engine::AST_Node_Type::Id
								&& node.children[1].children.size() == 0
								&& node.children[0].children.size() == 1
								&& node.children[0].children[0].get_text() == node.children[1].get_text()
								&& is_numeric(node.children[0])
							) {
								// (y+10)+y
								// reduces to:
								// (y+y)+10
								GL::Engine::Operators::Opers& RHS_oper = node.tag.cast<GL::Engine::Operators::Opers>();
								GL::Engine::Operators::Opers& LHS_oper = node.children[0].tag.cast<GL::Engine::Operators::Opers>();
								AbstractSyntaxTreeNode& IdNode = node.children[1];
								GL::any::fast_any& Constant = node.children[0].constant;

								switch (RHS_oper) {
								case GL::Engine::Operators::to_operator("+"): {
									switch (LHS_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (y+10)+y
										// reduces to:
										// (y+y)+10
										node = Fold_Right_Binary_Operator_Node("+", node.location, {
											Binary_Operator_Node("+", node.location, {
												IdNode,
												IdNode
											})
										}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (y-10)+y
										// reduces to:
										// (y+y)-10
										node = Fold_Right_Binary_Operator_Node("-", node.location, {
											Binary_Operator_Node("+", node.location, {
												IdNode,
												IdNode
											})
										}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("*"): {
										// (y*10)+y
										// reduces to:
										// y*(10+1)
										if (GL::any::fast_any out; AttemptCalculation("+", { Constant | GL::type::Const | GL::type::Reference, GL::any::fast_any::instance(1) | GL::type::Const | GL::type::Reference }, out)) {
											node = Fold_Right_Binary_Operator_Node("*", node.location, {
												IdNode
											}, out);
											return true;
										}
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (y/10)+y
										return false;
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("-"): {
									switch (LHS_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (y+10)-y
										// reduces to:
										// (y-y)+10
										// reduces to:
										// 10
										node = Constant_Node("", node.location, {}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (y-10)-y
										// reduces to:
										// (y-y)-10
										// reduces to:
										// 0-10
										if (GL::any::fast_any out; AttemptCalculation("-", { GL::any::fast_any::instance(0) | GL::type::Const | GL::type::Reference, Constant | GL::type::Const | GL::type::Reference }, out)) {
											node = Constant_Node("", node.location, {}, out);
											return true;
										}
										break;
									}
									case GL::Engine::Operators::to_operator("*"): {
										// (y*10)-y
										// reduces to:
										// y*(10-1)
										if (GL::any::fast_any out; AttemptCalculation("-", { Constant | GL::type::Const | GL::type::Reference, GL::any::fast_any::instance(1) | GL::type::Const | GL::type::Reference }, out)) {
											node = Fold_Right_Binary_Operator_Node("*", node.location, {
												IdNode
											}, out);
											return true;
										}
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (y/10)-y
										return false;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("*"): {
									switch (LHS_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (y+10)*y
										return false;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (y-10)*y
										return false;
									}
									case GL::Engine::Operators::to_operator("*"): {
										// (y*10)*y
										// rearranges to:
										// (y*y)*10
										node = Fold_Right_Binary_Operator_Node("*", node.location, {
											Binary_Operator_Node("*", node.location, {
												IdNode,
												IdNode
											})
										}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (y/10)*y
										// rearranges to:
										// (y*y)/10
										node = Fold_Right_Binary_Operator_Node("/", node.location, {
											Binary_Operator_Node("*", node.location, {
												IdNode,
												IdNode
											})
										}, Constant);
										return true;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("/"): {
									switch (LHS_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (y+10)/y
										// reduces to:
										// 1 + 10/y
										node = Fold_Left_Binary_Operator_Node("+", node.location, {
											Fold_Left_Binary_Operator_Node("/", node.location, {
												IdNode
											}, Constant)
										}, GL::any::fast_any::instance(1));
										return true;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (y-10)/y
										// reduces to:
										// 1 - 10/y
										node = Fold_Left_Binary_Operator_Node("-", node.location, {
											Fold_Left_Binary_Operator_Node("/", node.location, {
												IdNode
											}, Constant)
										}, GL::any::fast_any::instance(1));
										return true;
									}
									case GL::Engine::Operators::to_operator("*"): {
										// (y*10)/y
										// rearranges to:
										// 10
										node = Constant_Node("", node.location, {}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (y/10)/y
										// rearranges to:
										// 1/10
										if (GL::any::fast_any out; AttemptCalculation("/", { GL::any::fast_any::instance(1) | GL::type::Const | GL::type::Reference, Constant | GL::type::Const | GL::type::Reference }, out)) {
											node = Constant_Node("", node.location, {}, out);
											return true;
										}
										break;
									}
									default: return false;
									}
									break;
								}
								default: return false;
								}
							}

							if (node.identifier == Engine::AST_Node_Type::Binary
								&& node.children.size() == 2
								&& node.children[0].identifier == Engine::AST_Node_Type::BinaryFoldLeft
								&& node.children[1].identifier == Engine::AST_Node_Type::Id
								&& node.children[1].children.size() == 0
								&& node.children[0].children.size() == 1
								&& node.children[0].children[0].get_text() == node.children[1].get_text()
								&& is_numeric(node.children[0])
							) {
								// (10+y)+y
								// reduces to:
								// 10+(y+y)
								GL::Engine::Operators::Opers& RHS_oper = node.tag.cast<GL::Engine::Operators::Opers>();
								GL::Engine::Operators::Opers& LHS_oper = node.children[0].tag.cast<GL::Engine::Operators::Opers>();
								AbstractSyntaxTreeNode& IdNode = node.children[1];
								GL::any::fast_any& Constant = node.children[0].constant;

								switch (RHS_oper) {
								case GL::Engine::Operators::to_operator("+"): {
									switch (LHS_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (10+y)+y
										// reduces to:
										// (y+y)+10
										node = Fold_Right_Binary_Operator_Node("+", node.location, {
											Binary_Operator_Node("+", node.location, {
												IdNode,
												IdNode
											})
										}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (10-y)+y
										// reduces to:
										// 10
										node = Constant_Node("", node.location, {}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("*"): {
										// (10*y)+y
										// reduces to:
										// y*(10+1)
										if (GL::any::fast_any out; AttemptCalculation("+", { Constant | GL::type::Const | GL::type::Reference, GL::any::fast_any::instance(1) | GL::type::Const | GL::type::Reference }, out)) {
											node = Fold_Right_Binary_Operator_Node("*", node.location, {
												IdNode
											}, out);
											return true;
										}
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (10/y)+y
										return false;
										break;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("-"): {
									switch (LHS_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (10+y)-y
										// reduces to:
										// 10
										node = Constant_Node("", node.location, {}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (10-y)-y
										// reduces to:
										// 10-(y-y)
										node = Fold_Left_Binary_Operator_Node("-", node.location, {
											Binary_Operator_Node("-", node.location, {
												IdNode,
												IdNode
											})
										}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("*"): {
										// (10*y)-y
										// reduces to:
										// y*(10-1)
										if (GL::any::fast_any out; AttemptCalculation("-", { Constant | GL::type::Const | GL::type::Reference, GL::any::fast_any::instance(1) | GL::type::Const | GL::type::Reference }, out)) {
											node = Fold_Right_Binary_Operator_Node("*", node.location, {
												IdNode
												}, out);
											return true;
										}
										break;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (10/y)-y
										return false;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("*"): {
									switch (LHS_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (10+y)*y
										return false;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (10-y)*y
										return false;
									}
									case GL::Engine::Operators::to_operator("*"): {
										// (10*y)*y
										// rearranges to:
										// (y*y)*10
										node = Fold_Right_Binary_Operator_Node("*", node.location, {
											Binary_Operator_Node("*", node.location, {
												IdNode,
												IdNode
											})
										}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (10/y)*y
										// rearranges to:
										// 10
										node = Constant_Node("", node.location, {}, Constant);
										return true;
									}
									default: return false;
									}
									break;
								}
								case GL::Engine::Operators::to_operator("/"): {
									switch (LHS_oper) {
									case GL::Engine::Operators::to_operator("+"): {
										// (10+y)/y
										// reduces to:
										// 1 + 10/y
										node = Fold_Left_Binary_Operator_Node("+", node.location, {
											Fold_Left_Binary_Operator_Node("/", node.location, {
												IdNode
											}, Constant)
											}, GL::any::fast_any::instance(1));
										return true;
									}
									case GL::Engine::Operators::to_operator("-"): {
										// (10-y)/y
										// reduces to:
										// 10/y - 1 
										node = Fold_Right_Binary_Operator_Node("-", node.location, {
											Fold_Left_Binary_Operator_Node("/", node.location, {
												IdNode
											}, Constant)
										}, GL::any::fast_any::instance(1));
										return true;
									}
									case GL::Engine::Operators::to_operator("*"): {
										// (10*y)/y
										// rearranges to:
										// 10
										node = Constant_Node("", node.location, {}, Constant);
										return true;
									}
									case GL::Engine::Operators::to_operator("/"): {
										// (10/y)/y
										// rearranges to:
										// 10/(y*y)
										node = Fold_Left_Binary_Operator_Node("/", node.location, {
											Binary_Operator_Node("*", node.location, {
												IdNode,
												IdNode
											})
										}, Constant);
										return true;
									}
									default: return false;
									}
									break;
								}
								default: return false;
								}
							}
						}
						return false;
					}
				};

				// If an Inline_Array is made-up of const elements, then evaluate and store it as constexpr too.
				struct ConstArray {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Inline_Array
							&& node.children.size() == 0
						) {
							auto& engine = CurrentEngine();
							try {
								auto new_vector = engine.call("vector<var>", {});
								node = Constant_Node(node.text, node.location, {}, new_vector);
								return true;
							}
							catch (...) {
								return false;
							}
						}
					
						if (node.identifier == Engine::AST_Node_Type::Inline_Array
							&& node.children.size() == 1
							&& node.children[0].identifier == Engine::AST_Node_Type::Arg_List
						) {
							auto& argList = node.children.back();

							std::set<GL::type> types;
							bool allItemsAreConst = true;
							for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
								if (argList.children[childIndex].identifier != Engine::AST_Node_Type::Constant) {
									allItemsAreConst = false;
									break;
								}
								else {
									types.insert(argList.children[childIndex].constant.m_casted_type);
								}
							}

							if (allItemsAreConst) {
								if (types.size() == 1) {
									// vector<type>
									try {
										auto& engine = CurrentEngine();
										if (auto* BC = engine.try_find_class(*types.begin()); BC && BC->this_m.is_class()) {
											auto new_vector = engine.call("vector<"+ BC->this_m.scope_name +">", {});
											for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
												engine.call("push_back", { new_vector, argList.children[childIndex].constant | GL::type::Const | GL::type::Reference });
											}										
											node = Constant_Node(node.text, node.location, {}, new_vector);
											return true;
										}
										else {
											auto& engine = CurrentEngine();
											auto new_vector = engine.call("vector<var>", {});
											for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
												engine.call("push_back", { new_vector, argList.children[childIndex].constant | GL::type::Const | GL::type::Reference });
											}
											node = Constant_Node(node.text, node.location, {}, new_vector);
											return true;
										}
									}
									catch (...) {
										return false;
									}
								}
								else {
									// vector<var>
									try {
										auto& engine = CurrentEngine();
										auto new_vector = engine.call("vector<var>", {});
										for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
											engine.call("push_back", { new_vector, argList.children[childIndex].constant | GL::type::Const | GL::type::Reference });
										}
										node = Constant_Node(node.text, node.location, {}, new_vector);
										return true;									
									}
									catch (...) {
										return false;
									}
								}
							}
						}

						return false;
					}
				};

				// If an Inline_Map is made-up of const elements, then evaluate and store it as constexpr too.
				struct ConstMap {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Inline_Map
							&& node.children.size() == 0
						) {
							auto& engine = CurrentEngine();
							try {
								auto new_map = engine.call("map<var,var>", {});
								node = Constant_Node(node.text, node.location, {}, new_map);
								return true;
							}
							catch (...) {
								return false;
							}
						}					

						if (node.identifier == Engine::AST_Node_Type::Inline_Map
							&& node.children.size() == 1
							&& node.children[0].identifier == Engine::AST_Node_Type::Arg_List
						) {
							auto& argList = node.children.back();

							std::set<GL::type> key_types;
							std::set<GL::type> value_types;

							bool allItemsAreConst = true;
							for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
								if (argList.children[childIndex].identifier == Engine::AST_Node_Type::Map_Pair
									&& argList.children[childIndex].children.size() == 2
									&& argList.children[childIndex].children[0].identifier == Engine::AST_Node_Type::Constant
									&& argList.children[childIndex].children[1].identifier == Engine::AST_Node_Type::Constant
								) {
									key_types.insert(argList.children[childIndex].children[0].constant.m_casted_type);
									value_types.insert(argList.children[childIndex].children[1].constant.m_casted_type);
								}
								else {
									allItemsAreConst = false;
									break;
								}
							}

							if (allItemsAreConst) {
								auto& engine = CurrentEngine();
								try {
									GL::any::fast_any new_map;
									if (key_types.size() == 1) {
										if (auto* BC = engine.try_find_class(*key_types.begin()); BC && BC->this_m.is_class()) {
											if (value_types.size() == 1) {
												if (auto* BC2 = engine.try_find_class(*value_types.begin()); BC2 && BC2->this_m.is_class()) {
													new_map = engine.call("map<" + BC->this_m.scope_name + "," + BC2->this_m.scope_name + ">", {});
												}
												else return false;
											}
											else {
												new_map = engine.call("map<" + BC->this_m.scope_name + ",var>", {});
											}
										}
										else return false;
									}
									else {
										if (value_types.size() == 1) {
											if (auto* BC2 = engine.try_find_class(*value_types.begin()); BC2 && BC2->this_m.is_class()) {
												new_map = engine.call("map<var," + BC2->this_m.scope_name + ">", {});
											}
											else return false;
										}
										else {
											new_map = engine.call("map<var,var>", {});
										}
									}

									for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
										engine.call("insert", { new_map, argList.children[childIndex].children[0].constant | GL::type::Const | GL::type::Reference, argList.children[childIndex].children[1].constant | GL::type::Const | GL::type::Reference });
									}

									node = Constant_Node(node.text, node.location, {}, new_map);
									return true;
								}
								catch (...) {
									return false;
								}
							}
						}

						return false;
					}
				};

				// Improve the performance of a well-defined for loop by re-structuring it.
				struct ForLoopSignature {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::For
							&& node.children.size() >= 4
							) {
							// x++ into ++x;
							if (node.children[2].identifier == Engine::AST_Node_Type::Postfix) { // x++
								switch (Engine::hash(node.children[2].get_text().c_str())) {
								case Engine::hash("++"):
									node.children[2] = Prefix_Node(
										"++", std::move(node.children[2].location), std::move(node.children[2].children)
									);
									return true;
								case Engine::hash("--"):
									node.children[2] = Prefix_Node(
										"--", std::move(node.children[2].location), std::move(node.children[2].children)
									);
									return true;
								default:
									return false;
								};
							}
						}
						return false;
					};
				};

				// Re-order the catch statements to make the most sense, prefering type-matching to capture-at-all to catch-but-no-knowledge
				struct TryCatch {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if ((node.identifier == Engine::AST_Node_Type::Try)
							&& (node.children.size() > 2)
						) {
							std::vector< size_t > catches_with_types;
							std::vector< size_t > catches_without_types;
							std::vector< size_t > catches_without_anything;
							std::vector< size_t > finallys;
							for (size_t i = 1; i < node.children.size(); ++i) {
								auto& this_node = node.children[i];
								if (this_node.identifier == Engine::AST_Node_Type::Catch) {
									if (this_node.children.size() == 1) {
										catches_without_anything.push_back(i);
									}
									else if (this_node.children.size() == 2) {
										if (this_node.children[0].identifier == Engine::AST_Node_Type::Arg) {
											if (this_node.children[0].children.size() == 1) {
												catches_without_types.push_back(i);
											}
											else if (this_node.children[0].children.size() == 2) {
												catches_with_types.push_back(i);
											}
										}
									}
								}
								else if (this_node.identifier == Engine::AST_Node_Type::Finally) {
									finallys.push_back(i);
								}
							}
						
							if (catches_without_types.size() > 1) catches_without_types.erase(catches_without_types.begin() + 1, catches_without_types.end());
							if (catches_without_types.size() > 0) catches_without_anything.clear();						
							if (finallys.size() > 1) finallys.erase(finallys.begin() + 1, finallys.end());

							bool do_work = (node.children.size() - 1) != (catches_with_types.size() + catches_without_types.size() + catches_without_anything.size() + finallys.size());
							int expected_progress = 0;
							if (!do_work) for (auto& x : catches_with_types) if (x != ++expected_progress) {
								do_work = true;
								break;
							}						
							if (!do_work) for (auto& x : catches_without_types) if (x != ++expected_progress) {
								do_work = true;
								break;
							}						
							if (!do_work) for (auto& x : catches_without_anything) if (x != ++expected_progress) {
								do_work = true;
								break;
							}						
							if (!do_work) for (auto& x : finallys) if (x != ++expected_progress) {
								do_work = true;
								break;
							}
						
							if (do_work) {
								node.children = [&]() -> std::vector< AbstractSyntaxTreeNode > {
									std::vector< AbstractSyntaxTreeNode > out;
									out.push_back(node.children[0]);
									for (auto& x : catches_with_types) out.push_back(node.children[x]);
									for (auto& x : catches_without_types) out.push_back(node.children[x]);
									for (auto& x : catches_without_anything) out.push_back(node.children[x]);
									for (auto& x : finallys) out.push_back(node.children[x]);
									return out;
								}();
								return true;
							}
						}
						return false; // does nothing
					}
				};

				// move up the class / namespace declarations outside of inner scopes 
				struct PullOutNamespaceDeclarations {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.children.size() > 0) {
							for (auto& child : node.children) {
								if (child.identifier != Engine::AST_Node_Type::DeclarationBlock
									&& (child.children.size() > 0)
									) {
									for (int i = 0; i < child.children.size(); ++i) {
										if (
											child.children[i].identifier == Engine::AST_Node_Type::Namespace
											|| child.children[i].identifier == Engine::AST_Node_Type::Class
											) {
											AbstractSyntaxTreeNode copy = child.children[i];
											if (copy.tag.cast<NamespaceClassInformation&>().original_placement) {
												copy.tag.cast<NamespaceClassInformation&>().original_placement = false;
												child.children[i] = Noop_Node("", {}, {});

												// find the highest non-namespace or class in this and insert it there. 
												for (i = 0; i < node.children.size(); ++i) {
													if (node.children[i].identifier == Engine::AST_Node_Type::Namespace
														|| node.children[i].identifier == Engine::AST_Node_Type::Class
														) {
													}
													else {
														node.children.insert(node.children.begin() + i, copy);
														return true;
													}
												}
												node.children.insert(node.children.end(), copy);
												return true;
											}
											else {
												child.children.erase(child.children.begin() + i);

												// find the highest non-namespace or class in this and insert it there. 
												for (i = 0; i < node.children.size(); ++i) {
													if (node.children[i].identifier == Engine::AST_Node_Type::Namespace
														|| node.children[i].identifier == Engine::AST_Node_Type::Class
														) {
													}
													else {
														node.children.insert(node.children.begin() + i, copy);
														return true;
													}
												}
												node.children.insert(node.children.end(), copy);

												// node.children.insert(node.children.begin(), copy);
												return true;
											}
										}
									}
								}
							}
						}
						return false;
					};
				};

				// move up the class / namespace declarations outside of inner scopes 
				struct ConstexprObject {
					static bool try_find_constexpr(std::deque<std::map<GL::string, GL::any::fast_any>>& constexpr_results, GL::string const& to_find, GL::any::fast_any& out) {
						for (auto iter = constexpr_results.rbegin(), e = constexpr_results.rend(); iter != e; ++iter) {
							auto f = iter->find(to_find);
							auto ee = iter->end();
							if (f != ee) {
								if (f->second) {
									out = f->second;
									return true;
								}
								else {
									return false;
								}
							}
						}
						return false;
					};
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier == Engine::AST_Node_Type::Var_Decl) {
							// auto x;	
							if (node.tag.cast<ObjectDeclarationInformation>().is_constexpr) {
								// this basic invocation cannot be constexpr - it makes no sense.  
								return false;
								// throw except::eval_error("A non-typed 'auto' declaration cannot be constexpr without a clearly associated type or constant value assigned to it.", node.location.start);
							}
						}
						if (node.identifier == Engine::AST_Node_Type::Assign_Retroactively) {
							if (node.tag.cast<ObjectDeclarationInformation>().is_constexpr) {
								// if ((node.children.size() == 2) && (node.constant)) return false; // already performed -- nothing to be done.
								if (node.constant) return false; // already performed -- nothing to be done.

								if (node.children.size() == 2
									&& node.children[0].identifier == Engine::AST_Node_Type::Constant
									&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& !node.constant
								) {
									// this is a perfect example of a constexpr object.
									node.constant = node.children[0].constant;
									return true;
								}
								if (node.children.size() == 2
									&& node.children[0].identifier == Engine::AST_Node_Type::Fun_Call
									&& node.children[0].children.size() == 2
									&& node.children[0].children[0].identifier == Engine::AST_Node_Type::Id
									&& node.children[0].children[0].children.size() == 0
									&& node.children[0].children[0].text != "var"
									&& node.children[0].children[1].identifier == Engine::AST_Node_Type::Arg_List
									&& node.children[0].children[1].children.size() == 0
									&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& !node.constant
								) {
									try {
										auto initial_value = CurrentEngine().call(node.children[0].children[0].text, {});
										node.constant = initial_value;
										// node.children[0] = Constant_Node(node.children[0].children[0].text, node.children[0].children[0].location, {}, node.constant);
										return true;
									} 
									catch (...) {
										// failure to do constexpr folding.
										node.tag.cast<ObjectDeclarationInformation>().is_constexpr = false;
										return true;
									}
								}
								if (node.children.size() == 3
									&& node.children[0].identifier == Engine::AST_Node_Type::Fun_Call
									&& node.children[0].children.size() == 2
									&& node.children[0].children[0].identifier == Engine::AST_Node_Type::Id
									&& node.children[0].children[0].children.size() == 0
									&& node.children[0].children[0].text != "var"
									&& node.children[0].children[1].identifier == Engine::AST_Node_Type::Arg_List
									&& node.children[0].children[1].children.size() == 0
									&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& node.children[2].identifier == Engine::AST_Node_Type::Constant
									&& !node.constant
								) {
									try {
										auto initial_value = CurrentEngine().call(node.children[0].children[0].text, {});
										CurrentEngine().call("=", { initial_value, node.children[2].constant | GL::type::Const | GL::type::Reference });
										node.constant = initial_value;
										//node.children[0] = Constant_Node(node.children[0].children[0].text, node.children[0].children[0].location, {}, node.constant);
										//node.children.pop_back();
										return true;
									}
									catch (...) {
										// failure to do constexpr folding.
										node.tag.cast<ObjectDeclarationInformation>().is_constexpr = false;
										return true;
									}
								}
								if (node.children.size() == 3
									&& node.children[0].identifier == Engine::AST_Node_Type::Fun_Call
									&& node.children[0].children.size() == 2
									&& node.children[0].children[0].identifier == Engine::AST_Node_Type::Id
									&& node.children[0].children[0].children.size() == 0
									&& node.children[0].children[0].text != "var"
									&& node.children[0].children[1].identifier == Engine::AST_Node_Type::Arg_List
									&& node.children[0].children[1].children.size() == 0
									&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& node.children[2].identifier == Engine::AST_Node_Type::Fun_Call
									&& node.children[2].children.size() == 2
									&& node.children[2].children[0].identifier == Engine::AST_Node_Type::Id
									&& node.children[2].children[0].children.size() == 0
									&& node.children[2].children[1].identifier == Engine::AST_Node_Type::Arg_List
									&& node.children[2].children[1].children.size() == 0
									&& !node.constant
								) {
									try {
										auto initial_value = CurrentEngine().call(node.children[0].children[0].text, {});
										auto new_value = CurrentEngine().call(node.children[2].children[0].text, {});
										CurrentEngine().call("=", { initial_value, new_value | GL::type::Const | GL::type::Reference });
										node.constant = initial_value;
										//node.children[0] = Constant_Node(node.children[0].children[0].text, node.children[0].children[0].location, {}, node.constant);
										//node.children.erase(node.children.begin() + 2, node.children.end());
										return true;
									}
									catch (...) {
										// failure to do constexpr folding.
										node.tag.cast<ObjectDeclarationInformation>().is_constexpr = false;
										return true;
									}
								}
								if (node.children.size() == 3
									&& node.children[0].identifier == Engine::AST_Node_Type::Constant
									&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& node.children[2].identifier == Engine::AST_Node_Type::Fun_Call
									&& node.children[2].children.size() == 2
									&& node.children[2].children[0].identifier == Engine::AST_Node_Type::Id
									&& node.children[2].children[0].children.size() == 0
									&& node.children[2].children[1].identifier == Engine::AST_Node_Type::Arg_List
									&& node.children[2].children[1].children.size() == 0
									&& !node.constant
								) {
									try {
										auto initial_value = node.children[0].constant;
										auto new_value = CurrentEngine().call(node.children[2].children[0].text, {});
										CurrentEngine().call("=", { initial_value, new_value | GL::type::Const | GL::type::Reference });
										node.constant = initial_value;
										//node.children[0] = Constant_Node("", node.children[0].location, {}, node.constant);
										//node.children.erase(node.children.begin() + 2, node.children.end());
										return true;
									}
									catch (...) {
										// failure to do constexpr folding.
										node.tag.cast<ObjectDeclarationInformation>().is_constexpr = false;
										return true;
									}
								}
								if (node.children.size() == 3
									&& node.children[0].identifier == Engine::AST_Node_Type::Constant
									&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& node.children[2].identifier == Engine::AST_Node_Type::Constant
									&& !node.constant
								) {
									try {
										auto initial_value = node.children[0].constant;
										auto new_value = node.children[2].constant;
										CurrentEngine().call("=", { initial_value, new_value | GL::type::Const | GL::type::Reference });
										node.constant = initial_value;
										//node.children[0] = Constant_Node("", node.children[0].location, {}, node.constant);
										//node.children.erase(node.children.begin() + 2, node.children.end());
										return true;
									}
									catch (...) {
										// failure to do constexpr folding.
										node.tag.cast<ObjectDeclarationInformation>().is_constexpr = false;
										return true;
									}
								}

								// failure to do any constexpr folding.
								// node.tag.cast<ObjectDeclarationInformation>().is_constexpr = false;
								return false;
							}
						}

#if 1
						if (((node.identifier == Engine::AST_Node_Type::Block) || (node.identifier == Engine::AST_Node_Type::Arg_List) || (node.identifier == Engine::AST_Node_Type::Scopeless_Block) || (node.identifier == Engine::AST_Node_Type::File))
							&& node.children.size() > 0
						) {
							for (auto& this_child : node.children) {
								if (this_child.identifier == Engine::AST_Node_Type::Id
									&& this_child.constant
								) {
									this_child = Constant_Node(this_child.text, this_child.location, {}, this_child.constant);
									return true;
								}
							}

							//if (node.for_each_child([](AbstractSyntaxTreeNode& this_child) -> bool {
							//	if (this_child.identifier == Engine::AST_Node_Type::Id 
							//		&& this_child.constant
							//	) {
							//		this_child = Constant_Node(this_child.text, this_child.location, {}, this_child.constant);
							//		return true;
							//	}
							//	return false;
							//})) {
							//	return true;
							//}
						}
#endif

						// perform replacements as appropriate with constexpr values. 					
						if (node.identifier != Engine::AST_Node_Type::Assign_Retroactively) {
							std::deque<std::map<GL::string, GL::any::fast_any>> constexpr_results;
							constexpr_results.push_back({});
							bool made_update = false;
							node.for_each_child(
								// called for each child in each layer, depth-first.
								[&](AbstractSyntaxTreeNode& this_child) -> bool {
									// Early exit when we do not want to have constexpr values to bleed deeper into the tree
									if (this_child.identifier == Engine::AST_Node_Type::DeclarationBlock
										|| this_child.identifier == Engine::AST_Node_Type::FunctionDecl 
										|| this_child.identifier == Engine::AST_Node_Type::JustInTimeCompilation
										|| this_child.identifier == Engine::AST_Node_Type::Var_Decl
										|| this_child.identifier == Engine::AST_Node_Type::Constant
									) {
										// do not explore any deeper into these nodes
										return false;
									}
								
									// capture the constexpr value for the current block
									if (this_child.identifier == Engine::AST_Node_Type::Assign_Retroactively) {
										if (this_child.constant
											&& this_child.children.size() >= 2
											&& this_child.children[1].identifier == Engine::AST_Node_Type::Var_Decl
											&& this_child.children[1].children.size() >= 1
											&& this_child.children[1].children[0].identifier == Engine::AST_Node_Type::Id
											&& this_child.children[1].children[0].children.size() == 0
										) {
											constexpr_results.back()[this_child.children[1].children[0].text] = this_child.constant;										
											return true;
										}
										if (!this_child.constant
											&& this_child.children.size() >= 2
											&& this_child.children[1].identifier == Engine::AST_Node_Type::Var_Decl
											&& this_child.children[1].children.size() >= 1
											&& this_child.children[1].children[0].identifier == Engine::AST_Node_Type::Id
											&& this_child.children[1].children[0].children.size() == 0
										) {
											constexpr_results.back()[this_child.children[1].children[0].text] = {}; // nullptr
											return true;
										}
									}

									// Evaluate the places that constexpr should be allowed:
									if (this_child.identifier == Engine::AST_Node_Type::Binary
										&& this_child.children.size() == 2
									) {
										for (auto& child : this_child.children) {
											if (child.identifier == Engine::AST_Node_Type::Id
												&& child.children.size() == 0
											) {
												if (GL::any::fast_any temp; try_find_constexpr(constexpr_results, child.text, temp)) {
													child = Constant_Node(child.text, child.location, {}, temp);
													made_update = true;
													return true;
												}
											}									
										}
									}
									if (((this_child.identifier == Engine::AST_Node_Type::BinaryFoldLeft) || (this_child.identifier == Engine::AST_Node_Type::BinaryFoldRight))
										&& this_child.children.size() == 1
									) {
										for (auto& child : this_child.children) {
											if (child.identifier == Engine::AST_Node_Type::Id
												&& child.children.size() == 0
											) {
												if (GL::any::fast_any temp; try_find_constexpr(constexpr_results, child.text, temp)) {
													child = Constant_Node(child.text, child.location, {}, temp);
													made_update = true;
													return true;
												}
											}
										}
									}

									if (this_child.identifier == Engine::AST_Node_Type::Inline_Array
										&& this_child.children.size() == 1
										&& this_child.children[0].identifier == Engine::AST_Node_Type::Arg_List
									) {
										for (auto& child : this_child.children[0].children) {
											if (child.identifier == Engine::AST_Node_Type::Id
												&& child.children.size() == 0
											) {
												if (GL::any::fast_any temp; try_find_constexpr(constexpr_results, child.text, temp)) {
													child = Constant_Node(child.text, child.location, {}, temp);
													made_update = true;
													return true;
												}
											}
										}
									}

									// constexprMap[constexprIndex]
									if (this_child.identifier == Engine::AST_Node_Type::Array_Call
										&& this_child.children.size() == 2
										&& this_child.children[0].identifier == Engine::AST_Node_Type::Id
										&& this_child.children[0].children.size() == 0
									) {
										if (GL::any::fast_any constexprMapOrArray; try_find_constexpr(constexpr_results, this_child.children[0].text, constexprMapOrArray)) {
											if (this_child.children[1].identifier == Engine::AST_Node_Type::Id
												&& this_child.children[1].children.size() == 0
											) {
												if (GL::any::fast_any Index; try_find_constexpr(constexpr_results, this_child.children[1].text, Index)) {
													if (GL::any::fast_any temp; AttemptCalculation(this_child.text, { constexprMapOrArray | GL::type::Const | GL::type::Reference, Index | GL::type::Const | GL::type::Reference }, temp)) {
														this_child = Constant_Node(this_child.text, this_child.location, {}, temp);
														made_update = true;
														return true;
													}
												}
											}
											else if (this_child.children[1].identifier == Engine::AST_Node_Type::Constant) {
												if (GL::any::fast_any temp; AttemptCalculation(this_child.text, { constexprMapOrArray | GL::type::Const | GL::type::Reference, this_child.children[1].constant | GL::type::Const | GL::type::Reference }, temp)) {
													this_child = Constant_Node(this_child.text, this_child.location, {}, temp);
													made_update = true;
													return true;
												}
											}
										}
									}

									if (this_child.identifier == Engine::AST_Node_Type::If
										|| this_child.identifier == Engine::AST_Node_Type::For
										|| this_child.identifier == Engine::AST_Node_Type::While
										|| this_child.identifier == Engine::AST_Node_Type::Ranged_For
										|| this_child.identifier == Engine::AST_Node_Type::Case
										|| this_child.identifier == Engine::AST_Node_Type::Switch
										|| this_child.identifier == Engine::AST_Node_Type::Map_Pair
									) {
										for (auto& child : this_child.children) {
											if (child.identifier == Engine::AST_Node_Type::Id
												&& child.children.size() == 0
											) {
												if (GL::any::fast_any temp; try_find_constexpr(constexpr_results, child.text, temp)) {
													child = Constant_Node(child.text, child.location, {}, temp);
													made_update = true;
													return true;
												}
											}
										}
									}

									if (this_child.identifier == Engine::AST_Node_Type::Fun_Call
										&& this_child.children.size() == 2
										&& this_child.children[1].identifier == Engine::AST_Node_Type::Arg_List
									) {
										for (auto& child : this_child.children[1].children) {
											if (child.identifier == Engine::AST_Node_Type::Id
												&& child.children.size() == 0
											) {
												if (GL::any::fast_any temp; try_find_constexpr(constexpr_results, child.text, temp)) {
													child = Constant_Node(child.text, child.location, {}, temp);
													made_update = true;
													return true;
												}
											}
										}
									}

									if (this_child.identifier == Engine::AST_Node_Type::Postfix
										&& this_child.children.size() == 1
										&& this_child.children[0].identifier == Engine::AST_Node_Type::Id
										&& this_child.children[0].children.size() == 0
									) {
										if (this_child.tag.cast<PostfixInformation>().is_unit) {
											if (GL::any::fast_any constexprObj; try_find_constexpr(constexpr_results, this_child.children[0].text, constexprObj)) {
												if (GL::any::fast_any temp; AttemptCalculation(this_child.tag.cast<PostfixInformation>().unit_name, { constexprObj | GL::type::Const | GL::type::Reference }, temp)) {
													this_child = Constant_Node(this_child.text, this_child.location, {}, temp);
													made_update = true;
													return true;
												}
											}
										}
										else {
											if ((this_child.text == "++") || (this_child.text == "--")) {
												if (GL::any::fast_any temp; try_find_constexpr(constexpr_results, this_child.children[0].text, temp)) {
													throw except::eval_error("Calling postfix increment or decrement operations on constexpr objects is not supported.", this_child.location);
												}
											}
										}
									}

									if (this_child.identifier == Engine::AST_Node_Type::Type_Cast
										&& this_child.children.size() == 2
										&& this_child.children[0].identifier == Engine::AST_Node_Type::Id
										&& this_child.children[0].children.size() == 0
									) {
										if (this_child.children[1].identifier == Engine::AST_Node_Type::Id
											&& this_child.children[1].children.size() == 0
											) {
											if (GL::any::fast_any constexprObj; try_find_constexpr(constexpr_results, this_child.children[1].text, constexprObj)) {
												if (GL::any::fast_any temp; AttemptCalculation(this_child.children[0].text, { constexprObj | GL::type::Const | GL::type::Reference }, temp)) {
													this_child = Constant_Node(this_child.text, this_child.location, {}, temp);
													made_update = true;
													return true;
												}
											}
										}
										else if (this_child.children[1].identifier == Engine::AST_Node_Type::Constant) {
											if (GL::any::fast_any temp; AttemptCalculation(this_child.children[0].text, { this_child.children[1].constant | GL::type::Const | GL::type::Reference }, temp)) {
												this_child = Constant_Node(this_child.text, this_child.location, {}, temp);
												made_update = true;
												return true;
											}										
										}
									}

									if (this_child.identifier == Engine::AST_Node_Type::Equation
										&& this_child.children.size() == 2
										&& this_child.children[1].identifier == Engine::AST_Node_Type::Id
										&& this_child.children[1].children.size() == 0
									) {
										if (GL::any::fast_any temp; try_find_constexpr(constexpr_results, this_child.children[1].text, temp)) {
											this_child.children[1] = Constant_Node(this_child.text, this_child.location, {}, temp);
											made_update = true;
											return true;
										}
									}
									if (this_child.identifier == Engine::AST_Node_Type::Equation
										&& this_child.children.size() == 2
										&& this_child.children[0].identifier == Engine::AST_Node_Type::Id
										&& this_child.children[0].children.size() == 0
									) {
										if (GL::any::fast_any temp; try_find_constexpr(constexpr_results, this_child.children[0].text, temp)) {
											this_child.children[0] = Constant_Node(this_child.text, this_child.location, {}, temp);
											made_update = true;
											return true;
										}
									}
									if (this_child.identifier == Engine::AST_Node_Type::Equation
										&& this_child.children.size() == 2
										&& this_child.children[0].identifier == Engine::AST_Node_Type::Constant
										&& this_child.text != ".."
									) {
										throw except::eval_error("Calling a modifying equation on constexpr objects is not supported.", this_child.location);
									}

									if (this_child.identifier == Engine::AST_Node_Type::Id
										&& this_child.children.size() == 0
										&& !this_child.constant
									) {
										if (GL::any::fast_any temp; try_find_constexpr(constexpr_results, this_child.text, temp)) {
											this_child.constant = temp;
											made_update = true;
											return true;
										}
									}






									return true;
								}, 
								// Pushed on the start of every new "layer"
								[&](AbstractSyntaxTreeNode& this_child) -> void {								
									if (this_child.identifier == Engine::AST_Node_Type::File
										|| this_child.identifier == Engine::AST_Node_Type::Block
									) {
										constexpr_results.push_back({});
									}								
								}, 
								// Popped on the end of every "layer"
								[&](AbstractSyntaxTreeNode& this_child) -> void {								
									if (this_child.identifier == Engine::AST_Node_Type::File
										|| this_child.identifier == Engine::AST_Node_Type::Block
									) {
										constexpr_results.pop_back();
									}
								}
							);
							return made_update;
						}

						return false;
					};
				};

				// #define foo(x) #x;
				struct PreprocessMacroFunctions {
					bool optimize(AbstractSyntaxTreeNode& node) {
						return node.for_each_child([](AbstractSyntaxTreeNode& this_child) -> bool {
							if ((this_child.identifier == Engine::AST_Node_Type::Fun_Call)
								&& (this_child.children.size() == 2)
								&& (this_child.children[0].identifier == Engine::AST_Node_Type::Id)
								&& (this_child.children[0].children.size() == 0)
								&& (this_child.children[1].identifier == Engine::AST_Node_Type::Arg_List)
							) {
								for (int preprocessorPos = (int)CurrentParser().m_preprocessor_stack.size() - 1; preprocessorPos >= 0; --preprocessorPos) {
									auto* iter = &CurrentParser().m_preprocessor_stack[preprocessorPos];
									if (this_child.location.start.pos >= iter->location.end.pos) {
										if (iter->identifier == Engine::AST_Node_Type::PreprocessorMacro) {
											if (iter->text == "#define") {
												auto& info = iter->tag.cast<Engine::ScriptParser::Parser::PreprocessorDefineInformation>();
												if (info.is_function) {
													if (info.VarName == this_child.children[0].text) {
														// this define has been... defined.
														if (this_child.children[1].children.size() == info.Inputs.size()) {
															// version 1, converted to a block with constexpr object definitions.
															std::vector< AbstractSyntaxTreeNode > new_children;
															for (size_t var_n = 0; var_n < this_child.children[1].children.size(); ++var_n) {
																auto& input_node = this_child.children[1].children[var_n];
																auto& new_name = info.Inputs[var_n];
																new_children.push_back(Assign_Retroactively_Node("", input_node.location, {
																	input_node,
																	Var_Decl_Node("", input_node.location, {
																		Id_Node(new_name, input_node.location, {})
																	})
																}));
																new_children.back().tag.cast< ObjectDeclarationInformation>().is_constexpr = true; // hints to try and compile this out.

																DepthCounter counter;
																if (counter.depth > 100) {
																	GL::string err_txt = "Macro function replacement reached maximum recursive depth";
																	CurrentParser().m_preprocessor_stack.push_back(PreprocessorMacro_Node("#error", this_child.location, {
																		Constant_Node(err_txt, this_child.location, {}, err_txt)
																	}));
																	throw Engine::except::eval_error(err_txt, this_child.location);
																}

																new_children.back() = optimizer::optimize_all(new_children.back(), &CurrentParser(), 100);
															}

															DepthCounter counter;
															if (counter.depth > 100) {
																GL::string err_txt = "Macro function replacement reached maximum recursive depth";
																CurrentParser().m_preprocessor_stack.push_back(PreprocessorMacro_Node("#error", this_child.location, {
																	Constant_Node(err_txt, this_child.location, {}, err_txt)
																}));
																throw Engine::except::eval_error(err_txt, this_child.location);
															}
															
															std::vector< AbstractSyntaxTreeNode > constexpr_children;
															for (auto& x : new_children) if (x.constant) constexpr_children.push_back(x);

															auto new_child = CurrentParser().Parse(info.Remainder, 1000, constexpr_children);
															new_child.location = this_child.location;
															new_child.for_each_child([&](AbstractSyntaxTreeNode& this_child_2) -> bool {
																this_child_2.location = this_child.location;
																return false;
															});	
								
															while (true) {
																bool made_update = false;

																// Id swapping
																if (!made_update)
																	new_child.for_each_child([&info, &new_children, &made_update](AbstractSyntaxTreeNode& new_child) -> bool {
																		if (new_child.identifier == Engine::AST_Node_Type::Id
																			&& new_child.children.size() == 0
																		) {
																			for (size_t input_index = 0; input_index < info.Inputs.size(); ++input_index) {
																				if (info.Inputs[input_index] == new_child.text) {
																					new_child = new_children[input_index].children[0];
																					made_update = true;
																					// return true;
																				}
																			}																			
																		}
																		return false;
																	});
																if (!made_update)
																	if (new_child.identifier == Engine::AST_Node_Type::Id
																		&& new_child.children.size() == 0
																	) {
																		for (size_t input_index = 0; input_index < info.Inputs.size(); ++input_index) {
																			if (info.Inputs[input_index] == new_child.text) {
																				new_child = new_children[input_index].children[0];
																				made_update = true;
																			}
																		}																		
																	}

																// Id fixing
																if (!made_update)
																	new_child.for_each_child([&info, &new_children, &made_update](AbstractSyntaxTreeNode& new_child) -> bool {
																		if (new_child.identifier == Engine::AST_Node_Type::Id
																			&& (new_child.children.size() == 2)
																			&& (new_child.text == "##")
																		) {
																			new_child = CurrentParser().Parse(new_child.children[0].get_text() + new_child.children[1].get_text());
																			made_update = true;
																			// return true;
																		}
																		return false;
																	});
																if (!made_update)
																	if (new_child.identifier == Engine::AST_Node_Type::Id
																		&& (new_child.children.size() == 2)
																		&& (new_child.text == "##")
																	) {
																		new_child = CurrentParser().Parse(new_child.children[0].get_text() + new_child.children[1].get_text());
																		made_update = true;																		
																	}

																// #a
																if (!made_update)
																	new_child.for_each_child([&info, &new_children, &made_update](AbstractSyntaxTreeNode& new_child) -> bool {
																		if (new_child.identifier == Engine::AST_Node_Type::Prefix
																			&& new_child.children.size() == 1
																			&& new_child.text == "#"
																		) {
																			auto& child = new_child.children[0];
																			bool success = false;
																			if (child.identifier == Engine::AST_Node_Type::Id
																				&& child.children.size() == 0
																			) {
																				for (size_t input_index = 0; input_index < info.Inputs.size(); ++input_index) {
																					if (info.Inputs[input_index] == child.text) {
																						GL::string this_text = new_children[input_index].children[0].get_text();
																						new_child = Constant_Node(this_text, child.location, {}, this_text);
																						success = true;
																						made_update = true;
																						break;
																					}
																				}
																			}
																			if (!success) {
																				GL::string this_text = child.get_text();
																				new_child = Constant_Node(this_text, child.location, {}, this_text);
																				made_update = true;
																			}
																		}
																		return false;
																	});
																if (!made_update)
																	if (new_child.identifier == Engine::AST_Node_Type::Prefix
																		&& new_child.children.size() == 1
																		&& new_child.text == "#"
																	) {
																		auto& child = new_child.children[0];
																		bool success = false;
																		if (child.identifier == Engine::AST_Node_Type::Id
																			&& child.children.size() == 0
																			) {
																			for (size_t input_index = 0; input_index < info.Inputs.size(); ++input_index) {
																				if (info.Inputs[input_index] == child.text) {
																					GL::string this_text = new_children[input_index].children[0].get_text();
																					new_child = Constant_Node(this_text, child.location, {}, this_text);
																					success = true;
																					made_update = true;
																					break;
																				}
																			}
																		}
																		if (!success) {
																			GL::string this_text = child.get_text();
																			new_child = Constant_Node(this_text, child.location, {}, this_text);
																			made_update = true;
																		}
																	}

																if (!made_update) break;
															}

															new_children.push_back(new_child);

															this_child = optimizer::optimize_all(Block_Node("", this_child.location, new_children), &CurrentParser(), 100);

															return true;
														}
													}
												}
											}
											if (iter->text == "#undef") {
												if (iter->tag.cast<Engine::ScriptParser::Parser::PreprocessorUndefineInformation>().VarName == this_child.text) {
													// this define has been... undefined. 
													break;
												}
											}
										}
									}
								}
							}
							return false;
						}, false);
					};
				};

				// #define one_hundred 100;
				struct PreprocessMacroObjects {
					bool optimize(AbstractSyntaxTreeNode& node) {
						auto& current_parser = CurrentParser();
						for (int preprocessorPos = (int)current_parser.m_preprocessor_stack.size() - 1; preprocessorPos >= 0; --preprocessorPos) {
							auto* iter = &current_parser.m_preprocessor_stack[preprocessorPos];
							if (node.location.start.pos >= iter->location.end.pos) {
								if ((iter->text == "#define") || (iter->text == "#undef")) {
									if (node.for_each_child([&iter, &current_parser](AbstractSyntaxTreeNode& this_child) -> bool {
										if (this_child.identifier == Engine::AST_Node_Type::Id
											&& this_child.children.size() == 0
										) {
											if (this_child.location.start.pos >= iter->location.end.pos) {
												if (iter->identifier == Engine::AST_Node_Type::PreprocessorMacro) {
													if (iter->text == "#define") {
														auto& info = iter->tag.cast<Engine::ScriptParser::Parser::PreprocessorDefineInformation>();
														if (!info.is_function) {
															if (info.use_preprocessed_Remainder) {
																auto new_child = info.Remainder_Preprocessed;
																new_child.location = this_child.location;
																new_child.for_each_child([&](AbstractSyntaxTreeNode& this_child_2) -> bool {
																	this_child_2.location = this_child.location;
																	return false;
																});
																this_child = new_child;
																return true;
															}
															else {
																if (info.VarName == this_child.text) {
																	// this define has been... defined.
																	auto new_child = current_parser.Parse(info.Remainder);

																	if ((new_child.identifier == Engine::AST_Node_Type::Id) && (new_child.text == info.VarName) && (new_child.children.size() == 0)) {
																		// returns itself?
																		return false;
																	}
																	if (new_child.for_each_child([&](AbstractSyntaxTreeNode& this_child_2) -> bool {
																		if (this_child_2.identifier == Engine::AST_Node_Type::Id
																			&& this_child_2.identifier._size() == 0
																			&& this_child_2.text == info.VarName
																			) {
																			return true;
																		}
																		return false;
																	})) {
																		GL::string err_txt = "Macro ID replacement includes infinite recursion";
																		CurrentParser().m_preprocessor_stack.push_back(PreprocessorMacro_Node("#error", this_child.location, {
																			Constant_Node(err_txt, this_child.location, {}, err_txt)
																		}));
																		throw except::eval_error(err_txt, this_child.location);
																	}

																	new_child.location = this_child.location;
																	new_child.for_each_child([&](AbstractSyntaxTreeNode& this_child_2) -> bool {
																		this_child_2.location = this_child.location;
																		return false;
																	});

																	DepthCounter counter;
																	if (counter.depth > 100) {
																		GL::string err_txt = "Macro ID replacement reached maximum recursive depth";
																		CurrentParser().m_preprocessor_stack.push_back(PreprocessorMacro_Node("#error", this_child.location, {
																			Constant_Node(err_txt, this_child.location, {}, err_txt)
																		}));
																		throw Engine::except::eval_error(err_txt, this_child.location);
																	}

																	this_child = optimizer::optimize_all(new_child, &current_parser, 100);

																	return true;
																}
															}
														}
													}
													if (iter->text == "#undef") {
														if (iter->tag.cast<Engine::ScriptParser::Parser::PreprocessorUndefineInformation>().VarName == this_child.text) {
															// this define has been... undefined. 
															return false;
														}
													}
												}
											}
										}
										return false;
									})) {
										return true;
									}
								}
							}
						}
						return false;
					};
				};

				// Compiles a JustInTime node into the actual script, rather than waiting for the runtime analysis to do so. 
				struct JustInTime_Constexpr {
					bool optimize(AbstractSyntaxTreeNode& node) {
						//if (node.identifier == Engine::AST_Node_Type::File) {
						//	std::deque< AbstractSyntaxTreeNode* > children;							
						//	node.for_each_child(
						//		[](AbstractSyntaxTreeNode& this_child) -> bool { return true; },
						//		[](AbstractSyntaxTreeNode& this_child) -> void { /* push */ },
						//		[](AbstractSyntaxTreeNode& this_child) -> void { /* pop */ }
						//	);
						//}

						if (node.identifier == Engine::AST_Node_Type::JustInTimeCompilation
							&& node.children.size() >= 1
							&& node.children[0].identifier == Engine::AST_Node_Type::Constant
							&& node.children[0].constant
							&& node.children[0].constant.can_cast(GL::type_of<GL::string>())
						) {
							node = CurrentParser().Parse(node.children[0].constant.cast<GL::string>());
							return true;
						}

						return false;
					};
				};

				struct ConstexprFunctionCalls {
					bool optimize(AbstractSyntaxTreeNode& node) {
						if (node.identifier != Engine::AST_Node_Type::Fun_Call) {
							if (node.for_each_child([](AbstractSyntaxTreeNode& node) -> bool {
								if (node.identifier == Engine::AST_Node_Type::Dot_Access
									&& node.children.size() == 2
									&& node.children[0].constant
									&& node.children[1].identifier == Engine::AST_Node_Type::Fun_Call
									&& node.children[1].children.size() == 2
									&& node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
									&& node.children[1].children[1].identifier == Engine::AST_Node_Type::Arg_List
								) {
									std::vector<GL::any::fast_any> inputs;
									inputs.push_back(node.children[0].constant | GL::type::Const | GL::type::Reference);
									for (auto& child : node.children[1].children[1].children) {
										if (!child.constant) {
											break;
										}
										else {
											inputs.push_back(child.constant | GL::type::Const | GL::type::Reference);
										}
									}

									if (inputs.size() == (node.children[1].children[1].children.size() + 1)) {
										if (auto callable = CurrentEngine().try_find_callable(node.children[1].children[0].text, inputs.begin(), inputs.end()); callable) {
											if (   ((callable->m_signature.state_m & GL::function_signature::Static) > 0)
												|| ((callable->m_signature.state_m & GL::function_signature::Constructor) > 0)
												|| ((callable->m_signature.state_m & GL::function_signature::Constant) > 0)
											) {
												if ((callable->m_signature.state_m & GL::function_signature::Volatile) == 0) {
													try {
														auto result = CurrentEngine().call(callable.get(), inputs);
														node = Constant_Node(node.text, node.location, {}, result);
														return true;
													}
													catch (...) {}
												}
											}
										}
									}
								}

								if (node.identifier == Engine::AST_Node_Type::Dot_Access
									&& node.children.size() == 2
									&& node.children[0].constant
									&& node.children[1].identifier == Engine::AST_Node_Type::Id
								) {
									std::vector<GL::any::fast_any> inputs;
									inputs.push_back(node.children[0].constant | GL::type::Const | GL::type::Reference);
									if (auto callable = CurrentEngine().try_find_callable(node.children[1].text, inputs.begin(), inputs.end()); callable) {
										try {
											if (   ((callable->m_signature.state_m & GL::function_signature::Static) > 0)
												|| ((callable->m_signature.state_m & GL::function_signature::Constructor) > 0)
												|| ((callable->m_signature.state_m & GL::function_signature::Constant) > 0)
											) {
												if ((callable->m_signature.state_m & GL::function_signature::Volatile) == 0) {
													try {
														auto result = CurrentEngine().call(callable.get(), inputs);
														node = Constant_Node(node.text, node.location, {}, result);
														return true;
													}
													catch (...) {}
												}
											}
										}
										catch (...) {}
									}									
								}

								if (node.identifier != Engine::AST_Node_Type::Dot_Access) {
									for (auto& child : node.children) {
										if (child.identifier == Engine::AST_Node_Type::Fun_Call
											&& child.children.size() == 2
											&& child.children[0].identifier == Engine::AST_Node_Type::Id
											&& child.children[1].identifier == Engine::AST_Node_Type::Arg_List
											) {
											std::vector<GL::any::fast_any> inputs;
											for (auto& child : child.children[1].children) {
												if (!child.constant) {
													break;
												}
												else {
													inputs.push_back(child.constant | GL::type::Const | GL::type::Reference);
												}
											}
											if (inputs.size() == child.children[1].children.size()) {
												if (auto callable = CurrentEngine().try_find_callable(child.children[0].text, inputs.begin(), inputs.end()); callable) {
													if (((callable->m_signature.state_m & GL::function_signature::Static) > 0)
														|| ((callable->m_signature.state_m & GL::function_signature::Constructor) > 0)
														|| ((callable->m_signature.state_m & GL::function_signature::Constant) > 0)
													) {
														if ((callable->m_signature.state_m & GL::function_signature::Volatile) == 0) {
															try {
																auto result = CurrentEngine().call(callable.get(), inputs);
																child = Constant_Node(child.children[0].text, child.location, {}, result);
																return true;
															}
															catch (...) {}
														}
													}
												}
											}
										}
									}
								}



								return false;
							}, false)) {
								return true;
							};
						}
						return false;
					};
				};

				struct ErrorOrWarningDetection {
					bool optimize(AbstractSyntaxTreeNode& node) {
						// Equations that may modify lhs const values
						if (node.identifier == GL::Engine::AST_Node_Type::Equation
							&& node.children.size() == 2
							&& node.text != ".."
						) {
							if ((node.children[0].identifier == GL::Engine::AST_Node_Type::Constant) || (node.children[0].identifier == GL::Engine::AST_Node_Type::Id && node.children[0].constant)) {
								auto err_txt = "Modifying equations (including `" + node.text + "`) are not allowed on constexpr values";
								CurrentParser().m_preprocessor_stack.push_back(PreprocessorMacro_Node("#error", node.location, {
									Constant_Node(err_txt, node.location, {}, err_txt)
								}));
							}
						}

						return false;
					};
				};

				// re-rorders the tree before other optimizations take place.
				using Optimizer_Constexpr = Optimizer<
					optimizer::PreprocessMacroObjects,
					optimizer::VarDeclEquation_To_RetroactiveAssignment,
					optimizer::PostfixFold,
					optimizer::PrefixFold,
					optimizer::BinaryFold,
					optimizer::LogicalBinaryFold,
					optimizer::PartialBinaryFold,
					optimizer::ToStringFunctionCallWithConstant,
					optimizer::ConstArray,
					optimizer::ConstMap,
					optimizer::ConstexprObject,
					optimizer::JustInTime_Constexpr,
					optimizer::If,
					optimizer::Return,
					optimizer::Dead_Code
				>;
				using Optimizer_Preprocess = Optimizer<
					optimizer::PreprocessMacroFunctions				
				>;
				using Optimizer_Reorg = Optimizer<
					optimizer::PullOutNamespaceDeclarations,
					optimizer::TryCatch					
				>;
				// reduce the tree and remove dead code
				using Optimizer_Normal = Optimizer<	
					optimizer::Unused_Fun_Return,
					optimizer::ArgListFileConstant,
					optimizer::ForLoopSignature,
					optimizer::Block,
					optimizer::Switch, 
					optimizer::ConstexprFunctionCalls
				>;

				using Optimizer_Errors = Optimizer<
					optimizer::ErrorOrWarningDetection
				>;

			public:
				static AbstractSyntaxTreeNode optimize_all(AbstractSyntaxTreeNode p, Engine::ScriptParser::Parser* parser, int maxDepth = 1000) {
					GetParser().push_back(parser);

					while ((--maxDepth >= 0) 
						&& (Optimizer_Constexpr().optimize(p, parser->analysis_engine, maxDepth)
						 || Optimizer_Preprocess().optimize(p, parser->analysis_engine, maxDepth)
						 || Optimizer_Reorg().optimize(p, parser->analysis_engine, maxDepth)
						 || Optimizer_Normal().optimize(p, parser->analysis_engine, maxDepth)
					)) {};

					GetParser().pop_back();
					return p;
				};
				static AbstractSyntaxTreeNode incorporate_warnings_and_errors(AbstractSyntaxTreeNode p, Engine::ScriptParser::Parser* parser, int maxDepth = 1000) {
					GetParser().push_back(parser);

					Optimizer_Errors().optimize(p, parser->analysis_engine, maxDepth);

					GetParser().pop_back();
					return p;
				};
			}; // namespace optimizer
			class Parser {
			private:
				constexpr static utility::Static_String m_multiline_comment_end{ "*/" };
				constexpr static utility::Static_String m_multiline_comment_begin{ "/*" };
				constexpr static utility::Static_String m_singleline_comment{ "//" };
				constexpr static utility::Static_String m_cr_lf{ "\r\n" };

				template<typename string_type> struct Char_Parser {
					string_type& match;
					using char_type = typename string_type::value_type;
					bool is_escaped = false;
					bool is_interpolated = false;
					bool saw_interpolation_marker = false;
					bool is_octal = false;
					bool is_hex = false;
					std::size_t unicode_size = 0;
					const bool interpolation_allowed;

					string_type octal_matches;
					string_type hex_matches;

					Char_Parser(string_type& t_match, const bool t_interpolation_allowed)
						: match(t_match)
						, interpolation_allowed(t_interpolation_allowed) {
					}

					Char_Parser& operator=(const Char_Parser&) = delete;

					~Char_Parser() {
						try {
							if (is_octal) {
								process_octal();
							}

							if (is_hex) {
								process_hex();
							}

							if (unicode_size > 0) {
								process_unicode();
							}
						}
						catch (const std::invalid_argument&) {
						}
						catch (const except::eval_error&) {
							// Something happened with parsing, we'll catch it later?
						}
					}

					void process_hex() {
						if (!hex_matches.empty()) {
							auto val = stoll(hex_matches.to_string(), nullptr, 16);
							match = match + std::string(1, char_type(val));
						}
						hex_matches = "";
						is_escaped = false;
						is_hex = false;
					}

					void process_octal() {
						if (!octal_matches.empty()) {
							auto val = stoll(octal_matches.to_string(), nullptr, 8);
							match = match + std::string(1, char_type(val));
						}
						octal_matches = "";
						is_escaped = false;
						is_octal = false;
					}

					void process_unicode() {
						const auto ch = static_cast<uint32_t>(std::stoi(hex_matches.to_string(), nullptr, 16));
						const auto match_size = hex_matches.size();
						hex_matches = "";
						is_escaped = false;
						const auto u_size = unicode_size;
						unicode_size = 0;

						char buf[4];
						if (u_size != match_size) {
							throw except::eval_error("Incomplete unicode escape sequence");
						}
						if (u_size == 4 && ch >= 0xD800 && ch <= 0xDFFF) {
							throw except::eval_error("Invalid 16 bit universal character");
						}

						if (ch < 0x80) {
							match = match + std::string(1, static_cast<char>(ch));
						}
						else if (ch < 0x800) {
							buf[0] = static_cast<char>(0xC0 | (ch >> 6));
							buf[1] = static_cast<char>(0x80 | (ch & 0x3F));
							match = match + std::string(buf, 2);
						}
						else if (ch < 0x10000) {
							buf[0] = static_cast<char>(0xE0 | (ch >> 12));
							buf[1] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
							buf[2] = static_cast<char>(0x80 | (ch & 0x3F));
							match = match + std::string(buf, 3);
						}
						else if (ch < 0x200000) {
							buf[0] = static_cast<char>(0xF0 | (ch >> 18));
							buf[1] = static_cast<char>(0x80 | ((ch >> 12) & 0x3F));
							buf[2] = static_cast<char>(0x80 | ((ch >> 6) & 0x3F));
							buf[3] = static_cast<char>(0x80 | (ch & 0x3F));
							match = match + std::string(buf, 4);
						}
						else {
							// this must be an invalid escape sequence?
							throw except::eval_error("Invalid 32 bit universal character");
						}
					}

					void parse(const char_type t_char, Engine::Position pos) {
						const bool is_octal_char = t_char >= '0' && t_char <= '7';

						const bool is_hex_char = (t_char >= '0' && t_char <= '9') || (t_char >= 'a' && t_char <= 'f') || (t_char >= 'A' && t_char <= 'F');

						if (is_octal) {
							if (is_octal_char) {
								octal_matches = octal_matches + std::string(1, t_char);

								if (octal_matches.size() == 3) {
									process_octal();
								}
								return;
							}
							else {
								process_octal();
							}
						}
						else if (is_hex) {
							if (is_hex_char) {
								hex_matches = hex_matches + std::string(1, t_char);

								if (hex_matches.size() == 2 * sizeof(char_type)) {
									// This rule differs from the C/C++ standard, but ChaiScript
									// does not offer the same workaround options, and having
									// hexadecimal sequences longer than can fit into the char
									// type is undefined behavior anyway.
									process_hex();
								}
								return;
							}
							else {
								process_hex();
							}
						}
						else if (unicode_size > 0) {
							if (is_hex_char) {
								hex_matches = hex_matches + std::string(1, t_char);

								if (hex_matches.size() == unicode_size) {
									// Format is specified to be 'slash'uABCD
									// on collecting from A to D do parsing
									process_unicode();
								}
								return;
							}
							else {
								// Not a unicode anymore, try parsing any way
								// May be someone used 'slash'uAA only
								process_unicode();
							}
						}

						if (t_char == '\\') {
							if (is_escaped) {
								match = match + "\\";
								is_escaped = false;
							}
							else {
								is_escaped = true;
							}
						}
						else {
							if (is_escaped) {
								if (is_octal_char) {
									is_octal = true;
									octal_matches = octal_matches + std::string(1, t_char);
								}
								else if (t_char == 'x') {
									is_hex = true;
								}
								else if (t_char == 'u') {
									unicode_size = 4;
								}
								else if (t_char == 'U') {
									unicode_size = 8;
								}
								else {
									switch (t_char) {
									case ('\''):
										match = match + "\'";
										break;
									case ('\"'):
										match = match + "\"";
										break;
									case ('?'):
										match = match + "?";
										break;
									case ('a'):
										match = match + "\a";
										break;
									case ('b'):
										match = match + "\b";
										break;
									case ('f'):
										match = match + "\f";
										break;
									case ('n'):
										match = match + "\n";
										break;
									case ('r'):
										match = match + "\r";
										break;
									case ('t'):
										match = match + "\t";
										break;
									case ('v'):
										match = match + "\v";
										break;
									case ('$'):
										match = match + "$";
										break;
									default:
										throw except::eval_error("Unknown escaped sequence in string", Parse_Location(pos, pos));
									}
									is_escaped = false;
								}
							}
							else if (interpolation_allowed && t_char == '$') {
								saw_interpolation_marker = true;
							}
							else {
								match = match + std::string(1, t_char);
							}
						}
					}
				};

				static GL::any::fast_any const_var(GL::any const& rhs) {
					return rhs.fast() | GL::type::Const;
				};
				static std::map<GL::string, GL::any::fast_any> build_constants() {
					std::map<GL::string, GL::any::fast_any> out;
					out["true"] = const_var(true);
					out["false"] = const_var(false);
					out["Infinity"] = const_var(std::numeric_limits<double>::infinity());
					out["NaN"] = const_var(std::numeric_limits<double>::quiet_NaN());
					out["nullptr"] = const_var(GL::any());
					out["null"] = const_var(GL::any());
					return out;
				};
				static std::map<GL::string, GL::any::fast_any> const& constants() {
					static auto out{ build_constants() };
					return out;
				};

				template<typename Array2D, typename First, typename Second>
				static void set_alphabet(Array2D& array, const First first, const Second second) noexcept {
					auto* first_ptr = &std::get<0>(array) + static_cast<std::size_t>(first);
					auto* second_ptr = &std::get<0>(*first_ptr) + static_cast<std::size_t>(second);
					*second_ptr = true;
				};
				static std::array<std::array<bool, Engine::lengthof_alphabet>, Engine::max_alphabet> build_alphabet() noexcept {
					std::array<std::array<bool, Engine::lengthof_alphabet>, Engine::max_alphabet> alphabet{};

					set_alphabet(alphabet, Engine::symbol_alphabet, '?');

					set_alphabet(alphabet, Engine::symbol_alphabet, '?');
					set_alphabet(alphabet, Engine::symbol_alphabet, '+');
					set_alphabet(alphabet, Engine::symbol_alphabet, '-');
					set_alphabet(alphabet, Engine::symbol_alphabet, '*');
					set_alphabet(alphabet, Engine::symbol_alphabet, '/');
					set_alphabet(alphabet, Engine::symbol_alphabet, '|');
					set_alphabet(alphabet, Engine::symbol_alphabet, '&');
					set_alphabet(alphabet, Engine::symbol_alphabet, '^');
					set_alphabet(alphabet, Engine::symbol_alphabet, '=');
					set_alphabet(alphabet, Engine::symbol_alphabet, '.');
					set_alphabet(alphabet, Engine::symbol_alphabet, '<');
					set_alphabet(alphabet, Engine::symbol_alphabet, '>');

					for (size_t c = 'a'; c <= 'z'; ++c) {
						set_alphabet(alphabet, Engine::keyword_alphabet, c);
					}
					for (size_t c = 'A'; c <= 'Z'; ++c) {
						set_alphabet(alphabet, Engine::keyword_alphabet, c);
					}
					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, Engine::keyword_alphabet, c);
					}
					set_alphabet(alphabet, Engine::keyword_alphabet, '_');
					// set_alphabet(alphabet, keyword_alphabet, ':');

					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, Engine::int_alphabet, c);
					}
					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, Engine::float_alphabet, c);
					}
					set_alphabet(alphabet, Engine::float_alphabet, '.');

					for (size_t c = '0'; c <= '9'; ++c) {
						set_alphabet(alphabet, Engine::hex_alphabet, c);
					}
					for (size_t c = 'a'; c <= 'f'; ++c) {
						set_alphabet(alphabet, Engine::hex_alphabet, c);
					}
					for (size_t c = 'A'; c <= 'F'; ++c) {
						set_alphabet(alphabet, Engine::hex_alphabet, c);
					}

					set_alphabet(alphabet, Engine::x_alphabet, 'x');
					set_alphabet(alphabet, Engine::x_alphabet, 'X');

					for (size_t c = '0'; c <= '1'; ++c) {
						set_alphabet(alphabet, Engine::bin_alphabet, c);
					}
					set_alphabet(alphabet, Engine::b_alphabet, 'b');
					set_alphabet(alphabet, Engine::b_alphabet, 'B');

					for (size_t c = 'a'; c <= 'z'; ++c) {
						set_alphabet(alphabet, Engine::id_alphabet, c);
					}
					for (size_t c = 'A'; c <= 'Z'; ++c) {
						set_alphabet(alphabet, Engine::id_alphabet, c);
					}
					set_alphabet(alphabet, Engine::id_alphabet, '_');
					set_alphabet(alphabet, Engine::id_alphabet, ':'); // RG
					for (size_t c = '0'; c <= '9'; ++c) { set_alphabet(alphabet, Engine::id_alphabet, c); } // RG

					set_alphabet(alphabet, Engine::white_alphabet, ' ');
					set_alphabet(alphabet, Engine::white_alphabet, '\t');

					set_alphabet(alphabet, Engine::int_suffix_alphabet, 'l');
					set_alphabet(alphabet, Engine::int_suffix_alphabet, 'L');
					set_alphabet(alphabet, Engine::int_suffix_alphabet, 'u');
					set_alphabet(alphabet, Engine::int_suffix_alphabet, 'U');

					set_alphabet(alphabet, Engine::float_suffix_alphabet, 'l');
					set_alphabet(alphabet, Engine::float_suffix_alphabet, 'L');
					set_alphabet(alphabet, Engine::float_suffix_alphabet, 'f');
					set_alphabet(alphabet, Engine::float_suffix_alphabet, 'F');

					return alphabet;
				}
				static std::array<std::array<bool, Engine::lengthof_alphabet>, Engine::max_alphabet> const& alphabet() {
					static auto out{ build_alphabet() };
					return out;
				};

				struct Operator_Matches {
					using SS = utility::Static_String;

					struct Operator_Matches_Impl {
						using SS = utility::Static_String;
						// should match the order and categories from create_operators()
						const std::array<utility::Static_String, 2> m_0{ { SS("?"), SS("?=") } };
						const std::array<utility::Static_String, 1> m_1{ { SS("||")} };
						const std::array<utility::Static_String, 1> m_2{ { SS("&&")} };
						const std::array<utility::Static_String, 1> m_3{ { SS("|")} };
						const std::array<utility::Static_String, 1> m_4{ { SS("&")} };
						const std::array<utility::Static_String, 3> m_5{ { SS("=="), SS("!="), SS("..") } };
						const std::array<utility::Static_String, 4> m_6{ { SS("<"), SS("<="), SS(">"), SS(">=")} };
						const std::array<utility::Static_String, 2> m_7{ { SS("<<"), SS(">>") } };
						const std::array<utility::Static_String, 2> m_8{ { SS("+"), SS("-")} };
						const std::array<utility::Static_String, 3> m_9{ { SS("*"), SS("/"), SS("%")} };
						const std::array<utility::Static_String, 1> m_10{{ SS("^")/*, SS("##")*/ }};
						const std::array<utility::Static_String, 7> m_11{{ SS("++"), SS("--"), SS("-"), SS("+"), SS("!"), SS("~"), SS("#") }};
					};
					static auto const& Data() {
						static Operator_Matches_Impl out;
						return out;
					};
					static bool is_match(GL::string t_str) noexcept {
						constexpr std::array<std::size_t, 12> groups{ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 } };
						return std::any_of(groups.begin(), groups.end(), [&t_str](const std::size_t group) { return is_match(group, t_str); });
					};
					template<typename Predicate> static bool any_of(const std::size_t t_group, Predicate&& predicate) {
						auto match = [&predicate](const auto& array) { return std::any_of(array.begin(), array.end(), predicate); };

						switch (t_group) {
						case 0:
							return match(Data().m_0);
						case 1:
							return match(Data().m_1);
						case 2:
							return match(Data().m_2);
						case 3:
							return match(Data().m_3);
						case 4:
							return match(Data().m_4);
						case 5:
							return match(Data().m_5);
						case 6:
							return match(Data().m_6);
						case 7:
							return match(Data().m_7);
						case 8:
							return match(Data().m_8);
						case 9:
							return match(Data().m_9);
						case 10:
							return match(Data().m_10);
						case 11:
							return match(Data().m_11);
						default:
							return false;
						}
					}
					static bool is_match(const std::size_t t_group, GL::string t_str)  noexcept {
						auto match = [&t_str](const auto& array) {
							return std::any_of(array.begin(), array.end(), [&t_str](const auto& v) { return std::string_view(v) == t_str; });
						};

						switch (t_group) {
						case 0:
							return match(Data().m_0);
						case 1:
							return match(Data().m_1);
						case 2:
							return match(Data().m_2);
						case 3:
							return match(Data().m_3);
						case 4:
							return match(Data().m_4);
						case 5:
							return match(Data().m_5);
						case 6:
							return match(Data().m_6);
						case 7:
							return match(Data().m_7);
						case 8:
							return match(Data().m_8);
						case 9:
							return match(Data().m_9);
						case 10:
							return match(Data().m_10);
						case 11:
							return match(Data().m_11);
						default:
							return false;
						}
					}
				};
				static std::array<Engine::Operator_Precedence, 12> build_operators() noexcept {
					return std::array<Engine::Operator_Precedence, 12>{
						{
							Engine::Operator_Precedence::Ternary_Cond,
							Engine::Operator_Precedence::Logical_Or,
							Engine::Operator_Precedence::Logical_And,
							Engine::Operator_Precedence::Bitwise_Or,
							Engine::Operator_Precedence::Bitwise_And,
							Engine::Operator_Precedence::Equality,
							Engine::Operator_Precedence::Comparison,
							Engine::Operator_Precedence::Shift,
							Engine::Operator_Precedence::Addition,
							Engine::Operator_Precedence::Multiplication,
							Engine::Operator_Precedence::Bitwise_Xor,
							Engine::Operator_Precedence::Prefix
						}
					};
				};
				static std::array<Engine::Operator_Precedence, 12> const& operators() noexcept {
					static auto out{ build_operators() };
					return out;
				};

				static constexpr bool char_in_alphabet(char c, Engine::Alphabet a) noexcept { return alphabet()[a][static_cast<uint8_t>(c)]; } // test a char in an m_alphabet

			private:
				Engine::Position m_position{};
				std::vector<AbstractSyntaxTreeNode> m_match_stack;
				std::vector<AbstractSyntaxTreeNode> m_comment_stack;
			public:
				std::vector<AbstractSyntaxTreeNode> m_preprocessor_stack;
				enum class preprocessor_state {
					TRUE_UNTIL_ELSE,
					FALSE_UNTIL_ELSE,
					FALSE_UNTIL_ENDIF
				};
				std::vector<preprocessor_state> m_preprocessor_if_stack;

			private:
				// check if the string is a valid operator
				static bool is_operator(GL::string t_s) noexcept { return Operator_Matches::is_match(t_s); }
				static bool validate_object_name(GL::string const& name, Engine::Position const& m_position) {
					switch (Engine::hash(name.c_str())) {
					case Engine::hash(""):
						throw except::eval_error("Id names cannot be empty", Parse_Location(m_position, m_position));
					case Engine::hash("#define"):
					case Engine::hash("#undef"):
					case Engine::hash("#ifdef"):
					case Engine::hash("#ifndef"):
					case Engine::hash("#elif"):
					case Engine::hash("#else"):
					case Engine::hash("#endif"):
					case Engine::hash("#error"):
					case Engine::hash("#warning"):
					case Engine::hash("#include"):
					case Engine::hash("#pragma"):
					case Engine::hash("auto"):
					case Engine::hash("constexpr"):
						// case Engine::hash("var"):
					case Engine::hash("global"):
					case Engine::hash("while"):
					case Engine::hash("for"):
					case Engine::hash("parallel_for"):
					case Engine::hash("break"):
					case Engine::hash("continue"):
					case Engine::hash("case"):
					case Engine::hash("default"):
					case Engine::hash("switch"):
					case Engine::hash("try"):
					case Engine::hash("catch"):
					case Engine::hash("finally"):
					case Engine::hash("do"):
					case Engine::hash("evaluate"):
					case Engine::hash("namespace"):
					case Engine::hash("class"):
					case Engine::hash("return"):
					case Engine::hash("if"):
					case Engine::hash("else"):
					{
						return false;
						// GL::string temp = GL::string(name);
						// throw except::eval_error("Id name '" + temp + "' was reserved for the langauge", m_position);
					}
					default:
						return true;
					}
				};

			private:
				/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
				bool Symbol_(const utility::Static_String& sym) noexcept {
					const auto len = sym.size();
					if (m_position.remaining() >= len) {
						const char* file_pos = &(*m_position);
						for (size_t pos = 0; pos < len; ++pos) {
							if (sym.c_str()[pos] != file_pos[pos]) {
								return false;
							}
						}
						m_position += len;
						return true;
					}
					return false;
				};
				/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
				bool Symbol_(const GL::string& sym) noexcept {
					const auto len = sym.size();
					if (m_position.remaining() >= len) {
						const char* file_pos = &(*m_position);
						for (size_t pos = 0; pos < len; ++pos) {
							if (sym[pos] != file_pos[pos]) {
								return false;
							}
						}
						m_position += len;
						return true;
					}
					return false;
				};
				/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
				static bool Symbol_FreeStanding(const utility::Static_String& sym, GL::Engine::Position& m_position) noexcept {
					const auto len = sym.size();
					if (m_position.remaining() >= len) {
						const char* file_pos = &(*m_position);
						for (size_t pos = 0; pos < len; ++pos) {
							if (sym.c_str()[pos] != file_pos[pos]) {
								return false;
							}
						}
						m_position += len;
						return true;
					}
					return false;
				};
				/// Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
				static bool Symbol_FreeStanding(const GL::string& sym, GL::Engine::Position& m_position) noexcept {
					const auto len = sym.size();
					if (m_position.remaining() >= len) {
						const char* file_pos = &(*m_position);
						for (size_t pos = 0; pos < len; ++pos) {
							if (sym[pos] != file_pos[pos]) {
								return false;
							}
						}
						m_position += len;
						return true;
					}
					return false;
				};
				/// Reads a char from input if it matches the parameter, without skipping initial whitespace
				bool Char_(const char c) {
					if (m_position.has_more() && (*m_position == c)) {
						++m_position;
						return true;
					}
					else {
						return false;
					}
				};
				/// Reads a char from input if it matches the parameter, without skipping initial whitespace
				static bool Char_FreeStanding(const char c, GL::Engine::Position& m_position) {
					if (m_position.has_more() && (*m_position == c)) {
						++m_position;
						return true;
					}
					else {
						return false;
					}
				};
				/// Reads an end-of-line group from input, without skipping initial whitespace
				bool Eol_(const bool t_eos = false) {
					bool retval = false;

					if (m_position.has_more() && (Symbol_(m_cr_lf) || Char_('\n'))) {
						retval = true;
						//++m_position.line;
						m_position.col = 1;
					}
					else if (m_position.has_more() && !t_eos && Char_(';')) {
						retval = true;
					}

					return retval;
				};
				/// Reads an end-of-line group from input, without skipping initial whitespace
				static bool Eol_FreeStanding(const bool t_eos, GL::Engine::Position& m_position) {
					bool retval = false;

					if (m_position.has_more() && (Symbol_FreeStanding(m_cr_lf, m_position) || Char_FreeStanding('\n', m_position))) {
						retval = true;
						//++m_position.line;
						m_position.col = 1;
					}
					else if (m_position.has_more() && !t_eos && Char_FreeStanding(';', m_position)) {
						retval = true;
					}

					return retval;
				};
				/// Reads a string from input if it matches the parameter, without skipping initial whitespace
				bool Keyword_(const utility::Static_String& t_s) {
					const auto len = t_s.size();
					if (m_position.remaining() >= len) {
						auto tmp = m_position;
						for (size_t i = 0; tmp.has_more() && i < len; ++i) {
							if (*tmp != t_s.c_str()[i]) {
								return false;
							}
							++tmp;
						}
						m_position = tmp;
						return true;
					}

					return false;
				};
				/// Reads the optional exponent (scientific notation) and suffix for a Float, without skipping initial whitespace
				/// Support a form of scientific notation: 1e-5, 35.5E+8, 0.01e19
				bool read_exponent_and_suffix_() noexcept {
					// Support a form of scientific notation: 1e-5, 35.5E+8, 0.01e19
					if (m_position.has_more() && (std::tolower(*m_position) == 'e')) {
						++m_position;
						if (m_position.has_more() && ((*m_position == '-') || (*m_position == '+'))) {
							++m_position;
						}
						auto exponent_pos = m_position;
						while (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_alphabet)) {
							++m_position;
						}
						if (m_position == exponent_pos) {
							// Require at least one digit after the exponent
							return false;
						}
					}

					// Parse optional float suffix
					while (m_position.has_more() && char_in_alphabet(*m_position, Engine::float_suffix_alphabet)) {
						++m_position;
					}

					return true;
				};
				/// Reads a floating point value from input, without skipping initial whitespace
				bool Float_() noexcept {
					auto start = m_position;
					if (m_position.has_more() && char_in_alphabet(*m_position, Engine::float_alphabet)) {
						if (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_alphabet)) {
							++m_position;
						}
						bool found_mark = false;
						while (m_position.has_more() && (char_in_alphabet(*m_position, Engine::int_alphabet) || (*m_position == '\''))) {
							if (*m_position == '\'') {
								found_mark = true;
							}
							else {
								found_mark = false;
							}
							++m_position;
						}
						if (found_mark) {
							m_position = start;
							return false;
						}

						if (m_position.has_more() && (std::tolower(*m_position) == 'e')) {
							// The exponent is valid even without any decimal in the Float (1e8, 3e-15)
							return read_exponent_and_suffix_();
						}
						else if (m_position.has_more() && (*m_position == '.')) {
							++m_position;
							if (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_alphabet)) {
								while (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_alphabet)) {
									++m_position;
								}
								// After any decimal digits, support an optional exponent (3.7e3)
								(void)read_exponent_and_suffix_();
								return true;
							}
							else {
								if (m_position.has_more() && (std::tolower(*m_position) == 'e')) {
									// The exponent is valid even without any decimal in the Float (1e8, 3e-15)
									if (read_exponent_and_suffix_()) {
										return true;
									}
									else {
										--m_position;
										return true;
									}
								}								
							}
						}
					}
					m_position = start;
					return false;
				};
				/// Reads a integer value from input, without skipping initial whitespace
				bool Int_() noexcept {
					auto start = m_position;
					if (m_position.has_more() && char_in_alphabet(*m_position, Engine::float_alphabet)) {
						if (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_alphabet)) {
							++m_position;
						}
						bool found_mark = false;
						while (m_position.has_more() && (char_in_alphabet(*m_position, Engine::int_alphabet) || (*m_position == '\''))) {
							if (*m_position == '\'') {
								found_mark = true;
							}
							else {
								found_mark = false;
							}
							++m_position;
						}
						if (found_mark) {
							m_position = start;
							return false;
						}
						return true;
					}
					m_position = start;
					return false;
				};
				/// Reads a hex value from input, without skipping initial whitespace
				bool Hex_() noexcept {
					if (m_position.has_more() && (*m_position == '0')) {
						++m_position;

						if (m_position.has_more() && char_in_alphabet(*m_position, Engine::x_alphabet)) {
							++m_position;
							if (m_position.has_more() && char_in_alphabet(*m_position, Engine::hex_alphabet)) {
								while (m_position.has_more() && char_in_alphabet(*m_position, Engine::hex_alphabet)) {
									++m_position;
								}
								while (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_suffix_alphabet)) {
									++m_position;
								}

								return true;
							}
							else {
								--m_position;
							}
						}
						else {
							--m_position;
						}
					}

					return false;
				}
				/// Reads an integer suffix, without skipping initial whitespace
				void IntSuffix_() {
					while (m_position.has_more() && char_in_alphabet(*m_position, Engine::int_suffix_alphabet)) {
						++m_position;
					}
				}
				/// Reads a binary value from input, without skipping initial whitespace
				bool Binary_() {
					if (m_position.has_more() && (*m_position == '0')) {
						++m_position;

						if (m_position.has_more() && char_in_alphabet(*m_position, Engine::b_alphabet)) {
							++m_position;
							if (m_position.has_more() && char_in_alphabet(*m_position, Engine::bin_alphabet)) {
								while (m_position.has_more() && char_in_alphabet(*m_position, Engine::bin_alphabet)) {
									++m_position;
								}
								return true;
							}
							else {
								--m_position;
							}
						}
						else {
							--m_position;
						}
					}

					return false;
				};
				template<typename T> constexpr static auto parse_num_(const GL::string t_str) noexcept -> typename std::enable_if<std::is_integral<T>::value, T>::type {
					T t = 0;
					for (const auto c : t_str) {
						if (c < '0' || c > '9') {
							return t;
						}
						t *= 10;
						t += c - '0';
					}
					return t;
				};
				template<typename T> static auto parse_num_(const GL::string t_str) -> typename std::enable_if<!std::is_integral<T>::value, T>::type {
					T t = 0;
					T base{};
					T decimal_place = 0;
					int exponent = 0;

					for (const auto c : t_str) {
						switch (c) {
						case '.':
							decimal_place = 10;
							break;
						case 'e':
						case 'E':
							exponent = 1;
							decimal_place = 0;
							base = t;
							t = 0;
							break;
						case '-':
							exponent = -1;
							break;
						case '+':
							break;
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
						case '7':
						case '8':
						case '9':
							if (decimal_place < 10) {
								t *= 10;
								t += static_cast<T>(c - '0');
							}
							else {
								t += static_cast<T>(c - '0') / decimal_place;
								decimal_place *= 10;
							}
							break;
						default:
							break;
						}
					}
					return exponent ? base * std::pow(T(10), t * static_cast<T>(exponent)) : t;
				};
				/// Parses a floating point value
				static GL::any::fast_any buildFloat(GL::string t_val) {
					bool float_ = false;
					bool long_ = false;

					auto i = t_val.size();

					for (; i > 0; --i) {
						char val = t_val[i - 1];

						if (val == 'f' || val == 'F') {
							float_ = true;
						}
						else if (val == 'l' || val == 'L') {
							long_ = true;
						}
						else {
							break;
						}
					}

					if (float_) {
						return GL::any::fast_any::instance(parse_num_<float>(t_val.substr(0, i).replace("\'", "")));
					}
					else if (long_) {
						return GL::any::fast_any::instance(parse_num_<long double>(t_val.substr(0, i).replace("\'", "")));
					}
					else {
						return GL::any::fast_any::instance(parse_num_<double>(t_val.substr(0, i).replace("\'", "")));
					}
				}
				/// Parses a integer value and returns a wrapped representation of it
				static GL::any::fast_any buildInt(const int base, GL::string t_val, const bool prefixed) {
					bool unsigned_ = false;
					bool long_ = false;
					bool longlong_ = false;

					auto i = t_val.size();

					for (; i > 0; --i) {
						const char val = t_val[i - 1];

						if (val == 'u' || val == 'U') {
							unsigned_ = true;
						}
						else if (val == 'l' || val == 'L') {
							if (long_) {
								longlong_ = true;
							}

							long_ = true;
						}
						else {
							break;
						}
					}

					if (prefixed) {
						t_val.remove_prefix(2);
					}

					try {
						/// TODO fix this to use from_chars
						auto u = std::stoll(t_val.replace("\'", "").to_string(), nullptr, base);

						if (!unsigned_ && !long_ && u >= std::numeric_limits<int>::min() && u <= std::numeric_limits<int>::max()) {
							return GL::any::fast_any::instance(static_cast<int>(u));
						}
						else if ((unsigned_ || base != 10) && !long_ && u >= std::numeric_limits<unsigned int>::min()
							&& u <= std::numeric_limits<unsigned int>::max()) {
							return GL::any::fast_any::instance(static_cast<unsigned int>(u));
						}
						else if (!unsigned_ && !longlong_ && u >= std::numeric_limits<long>::min() && u <= std::numeric_limits<long>::max()) {
							return GL::any::fast_any::instance(static_cast<long>(u));
						}
						else if ((unsigned_ || base != 10) && !longlong_ && u >= std::numeric_limits<unsigned long>::min()
							&& u <= std::numeric_limits<unsigned long>::max()) {
							return GL::any::fast_any::instance(static_cast<unsigned long>(u));
						}
						else if (!unsigned_ && u >= std::numeric_limits<long long>::min() && u <= std::numeric_limits<long long>::max()) {
							return GL::any::fast_any::instance(static_cast<long long>(u));
						}
						else {
							return GL::any::fast_any::instance(static_cast<unsigned long long>(u));
						}
					}
					catch (const std::out_of_range&) {
						// too big to be signed
						try {
							/// TODO fix this to use from_chars
							auto u = std::stoull(t_val.replace("\'", "").to_string(), nullptr, base);

							if (!longlong_ && u >= std::numeric_limits<unsigned long>::min() && u <= std::numeric_limits<unsigned long>::max()) {
								return GL::any::fast_any::instance(static_cast<unsigned long>(u));
							}
							else {
								return GL::any::fast_any::instance(static_cast<unsigned long long>(u));
							}
						}
						catch (const std::out_of_range&) {
							// it's just simply too big
							return GL::any::fast_any::instance(std::numeric_limits<long long>::max());
						}
					}
				}
				/// Reads an identifier from input which conforms to C's identifier naming conventions, without skipping initial whitespace
				bool Id_(GL::string* out = nullptr) {
					const auto prev_pos = m_position;

					auto failure = [&]() {
						m_position = prev_pos;
						return false;
					};

					if (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) { // e.g. found `v`
						if (*m_position == ':') {
							// we require colons to come in pairs
							++m_position;
							if (m_position.has_more()) {
								if (*m_position == ':') {
									++m_position;
									//if (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) {
									//	// works for us. 
									//}
									//else {
									//	return failure();
									//}
								}
								else {
									return failure();
								}
							}
							else {
								return failure();
							}
						}
						if ((*m_position >= '0') && (*m_position <= '9')) return failure();

						auto potential_end = m_position;
						auto local_revert = m_position;
						bool inside_brackets = false;
						while (m_position.has_more()) {
							// capture "normal" id chars
							if (char_in_alphabet(*m_position, Engine::id_alphabet)) {
								if (*m_position == ':') {
									// we require colons to come in pairs
									++m_position;
									if (m_position.has_more()) {
										if (*m_position == ':') {
											++m_position;
											//if (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) {
												potential_end = m_position;
												continue;
											//}
										}
									}
									break;								
								}
								else {
									++m_position;
									potential_end = m_position;
									continue;
								}
							}
							// skip whitespace chars
							while (m_position.has_more() && char_in_alphabet(*m_position, Engine::white_alphabet)) ++m_position;
							local_revert = m_position;
							// handle <...>
							if (*m_position == '<') {
								inside_brackets = true;
								++m_position;
								while (m_position.has_more() && char_in_alphabet(*m_position, Engine::white_alphabet)) ++m_position;
								if (Id_(out)) {								
									// successful search, keep going.
									continue;
								}
								else {
									m_position = local_revert;
								}
							}
							if (inside_brackets && (*m_position == ',')) {
								++m_position;
								while (m_position.has_more() && char_in_alphabet(*m_position, Engine::white_alphabet)) ++m_position;
								if (Id_(out)) {
									// successful search, keep going.
									continue;
								}
								else {
									m_position = local_revert;
								}
							}
							if (inside_brackets && (*m_position == '>')) {
								inside_brackets = false;
								++m_position;
								potential_end = m_position;
								continue;
							}
							break;
						}
						m_position = potential_end;
						if (out) *out = Engine::Position::str(prev_pos, m_position);
					
						return true;
					}
					return failure();
				};
				static bool Id_FreeStanding(GL::string* out, GL::Engine::Position& m_position) {
					const auto prev_pos = m_position;

					auto failure = [&]() {
						m_position = prev_pos;
						return false;
					};

					if (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) { // e.g. found `v`
						if (*m_position == ':') {
							// we require colons to come in pairs
							++m_position;
							if (m_position.has_more()) {
								if (*m_position == ':') {
									++m_position;
									//if (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) {
										// works for us. 
									//}
									//else {
									//	return failure();
									//}
								}
								else {
									return failure();
								}
							}
							else {
								return failure();
							}
						}
						if ((*m_position >= '0') && (*m_position <= '9')) return failure();

						auto potential_end = m_position;
						auto local_revert = m_position;
						bool inside_brackets = false;
						while (m_position.has_more()) {
							// capture "normal" id chars
							if (char_in_alphabet(*m_position, Engine::id_alphabet)) {
								if (*m_position == ':') {
									// we require colons to come in pairs
									++m_position;
									if (m_position.has_more()) {
										if (*m_position == ':') {
											++m_position;
											//if (m_position.has_more() && char_in_alphabet(*m_position, Engine::id_alphabet)) {
												potential_end = m_position;
												continue;
											//}
										}
									}
									break;
								}
								else {
									++m_position;
									potential_end = m_position;
									continue;
								}
							}
							// skip whitespace chars
							while (m_position.has_more() && char_in_alphabet(*m_position, Engine::white_alphabet)) ++m_position;
							local_revert = m_position;
							// handle <...>
							if (*m_position == '<') {
								inside_brackets = true;
								++m_position;
								while (m_position.has_more() && char_in_alphabet(*m_position, Engine::white_alphabet)) ++m_position;
								if (Id_FreeStanding(out, m_position)) {
									// successful search, keep going.
									continue;
								}
								else {
									m_position = local_revert;
								}
							}
							if (inside_brackets && (*m_position == ',')) {
								++m_position;
								while (m_position.has_more() && char_in_alphabet(*m_position, Engine::white_alphabet)) ++m_position;
								if (Id_FreeStanding(out, m_position)) {
									// successful search, keep going.
									continue;
								}
								else {
									m_position = local_revert;
								}
							}
							if (inside_brackets && (*m_position == '>')) {
								inside_brackets = false;
								++m_position;
								potential_end = m_position;
								continue;
							}
							break;
						}
						m_position = potential_end;
						if (out) *out = Engine::Position::str(prev_pos, m_position);
						return true;
					}
					return failure();
				};
				/// Reads a quoted string from input, without skipping initial whitespace
				bool Quoted_String_() {
					if (m_position.has_more() && (*m_position == '\"')) {
						char prev_char = *m_position;
						++m_position;

						int in_interpolation = 0;
						bool in_quote = false;

						while (m_position.has_more() && ((*m_position != '\"') || (in_interpolation > 0) || (prev_char == '\\'))) {
							if (!Eol_()) {
								if (prev_char == '$' && *m_position == '{') {
									++in_interpolation;
								}
								else if (prev_char != '\\' && *m_position == '"') {
									in_quote = !in_quote;
								}
								else if (*m_position == '}' && !in_quote) {
									--in_interpolation;
								}

								if (prev_char == '\\') {
									prev_char = 0;
								}
								else {
									prev_char = *m_position;
								}
								++m_position;
							}
						}

						if (m_position.has_more()) {
							++m_position;
						}
						else {
							throw except::eval_error("Unclosed quoted string", Parse_Location(m_position, m_position));
						}

						return true;
					}
					return false;
				};
				/// Reads (and potentially captures) a number from the input, detecting if it's an integer or floating point, without skipping initial whitespace
				bool Num_() {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					const auto start = m_position;
					if (m_position.has_more() && char_in_alphabet(*m_position, Engine::float_alphabet)) {
						try {
							if (Hex_()) {
								auto match = Engine::Position::str(start, m_position);
								auto bv = buildInt(16, match, true);
								m_match_stack.push_back(make_const(match, start, bv));
								return true;
							}
							else if (Binary_()) {
								auto match = Engine::Position::str(start, m_position);
								auto bv = buildInt(2, match, true);
								m_match_stack.push_back(make_const(match, start, bv));
								return true;
							}
							else if (Float_()) {
								auto match = Engine::Position::str(start, m_position);
								auto bv = buildFloat(match);
								m_match_stack.push_back(make_const(match, start, bv));
								return true;
							}
							else if (Int_()) {
								IntSuffix_();
								auto match = Engine::Position::str(start, m_position);
								auto bv = buildInt(10, match, false);
								m_match_stack.push_back(make_const(match, start, bv));
								return true;
							}
							return failure();

							//else {
							//	IntSuffix_();
							//	auto match = Engine::Position::str(start, m_position);
							//	if (!match.empty() && (match[0] == '0')) {
							//		auto bv = buildInt(8, match, false);
							//		m_match_stack.push_back(make_const(match, start, bv));
							//	}
							//	else if (!match.empty()) {
							//		auto bv = buildInt(10, match, false);
							//		m_match_stack.push_back(make_const(match, start, bv));
							//	}
							//	else {
							//		return failure();
							//	}
							//	return true;
							//}
						}
						catch (const std::invalid_argument&) {
							// error parsing number passed in to buildFloat/buildInt
							return failure();
						}
					}
					else {
						return failure();
					}
				};

			private:
				/// Helper function that collects ast_nodes from a starting position to the top of the stack into a new AST node
				template<typename NodeType, typename... Args> AbstractSyntaxTreeNode& build_match(size_t t_match_start, GL::string t_text = "", Args const&... arguments) {
					bool is_deep = false;

					Engine::Parse_Location filepos = [&]() -> Engine::Parse_Location {
						// so we want to take everything to the right of this and make them children
						if (t_match_start != m_match_stack.size()) {
							is_deep = true;
							return Engine::Parse_Location(
								m_match_stack[t_match_start].location.start,
								m_position);
						}
						else {
							return Engine::Parse_Location(m_position, m_position);
						}
					}();

					std::vector<AbstractSyntaxTreeNode> new_children;
					if (is_deep) {
						new_children.assign(std::make_move_iterator(m_match_stack.begin() + static_cast<int>(t_match_start)),
							std::make_move_iterator(m_match_stack.end()));
						m_match_stack.erase(m_match_stack.begin() + static_cast<int>(t_match_start), m_match_stack.end());
					}

#if 1
					m_match_stack.push_back(optimizer::optimize_all(NodeType(
						std::move(t_text)
						, std::move(filepos)
						, std::move(new_children)
						, arguments...
					), this, 100));
#else
					m_match_stack.push_back(NodeType(
						std::move(t_text)
						, std::move(filepos)
						, std::move(new_children)
						, arguments...
					));
#endif
					return m_match_stack.back();
				};
				/// create a node
				template<typename T, typename... Param> AbstractSyntaxTreeNode make_node(GL::string t_match, Engine::Position t_prev, Param &&...param) {
					return T(
						GL::string(t_match),
						Engine::Parse_Location(t_prev, m_position),
						std::forward<Param>(param)...
					);
				};
				/// create a node
				template<typename... Param> AbstractSyntaxTreeNode make_const(GL::string t_match, Engine::Position t_prev, Param &&...param) {
					return Constant_Node(
						GL::string(t_match),
						Engine::Parse_Location(t_prev, m_position),
						{},
						std::forward<Param>(param)...
					);
				};

			private:
				/// Skips (and potentially captures w/ nullptr scope) any multi-line or single-line comment
				bool SkipComment() {
					const auto start = m_position;
					if (Symbol_(m_multiline_comment_begin)) {
						while (m_position.has_more()) {
							if (Symbol_(m_multiline_comment_end)) {
								break;
							}
							else if (!Eol_()) {
								++m_position;
							}
						}
						GL::string comment = Engine::Position::str(start, m_position);
						m_comment_stack.push_back(Comment_Node(GL::string(comment), Engine::Parse_Location(start, m_position), {}));

						return true;
					}
					else if (Symbol_(m_singleline_comment)) {
						while (m_position.has_more()) {
							if (Symbol_(m_cr_lf)) {
								m_position -= 2;
								break;
							}
							else if (Char_('\n')) {
								--m_position;
								break;
							}
							else {
								++m_position;
							}
						}

						GL::string comment = Engine::Position::str(start, m_position);
						m_comment_stack.push_back(Comment_Node(GL::string(comment), Engine::Parse_Location(start, m_position), {}));

						return true;

					}
					return false;
				}
				/// Skips any multi-line or single-line comment. Does NOT capture any comments that were discovered. 
				static bool SkipComment_FreeStanding(GL::Engine::Position& m_position) {
					const auto start = m_position;
					if (Symbol_FreeStanding(m_multiline_comment_begin, m_position)) {
						while (m_position.has_more()) {
							if (Symbol_FreeStanding(m_multiline_comment_end, m_position)) {
								break;
							}
							else if (!Eol_FreeStanding(false, m_position)) {
								++m_position;
							}
						}
						//GL::string comment = Engine::Position::str(start, m_position);
						//  m_comment_stack.push_back(Comment_Node(GL::string(comment), Engine::Parse_Location(start, m_position), {}));

						return true;
					}
					else if (Symbol_FreeStanding(m_singleline_comment, m_position)) {
						while (m_position.has_more()) {
							if (Symbol_FreeStanding(m_cr_lf, m_position)) {
								m_position -= 2;
								break;
							}
							else if (Char_FreeStanding('\n', m_position)) {
								--m_position;
								break;
							}
							else {
								++m_position;
							}
						}

						//GL::string comment = Engine::Position::str(start, m_position);
						//m_comment_stack.push_back(Comment_Node(GL::string(comment), Engine::Parse_Location(start, m_position), {}));

						return true;

					}
					return false;
				}
				/// Skips whitespace, which means space and tab, but not cr/lf
				/// jespada: Modified SkipWS to skip optionally CR ('\n') and/or LF+CR ("\r\n")
				/// AlekMosingiewicz: Added exception when illegal character detected
				static bool SkipWS_FreeStanding(bool skip_cr, GL::Engine::Position& m_position) {
					bool retval = false;

					while (m_position.has_more()) {
						if (static_cast<unsigned char>(*m_position) > 0x7e) {
							throw except::eval_error("Illegal character", Parse_Location(m_position, m_position));
						}
						auto end_line = (*m_position != 0) && ((*m_position == '\n') || (*m_position == '\r' && *(m_position + 1) == '\n'));

						if (char_in_alphabet(*m_position, Engine::white_alphabet) || (skip_cr && end_line)) {
							if (end_line) {
								if (*m_position == '\r') {
									// discards lf
									++m_position;
								}
							}

							++m_position;

							retval = true;
						}
						else if (SkipComment_FreeStanding(m_position)) {
							retval = true;
						}
						else {
							break;
						}
					}
					return retval;
				};
				/// Skips whitespace, which means space and tab, but not cr/lf
				/// jespada: Modified SkipWS to skip optionally CR ('\n') and/or LF+CR ("\r\n")
				/// AlekMosingiewicz: Added exception when illegal character detected
				bool SkipWS(bool skip_cr = false) {
					bool retval = false;

					while (m_position.has_more()) {
						if (static_cast<unsigned char>(*m_position) > 0x7e) {
							throw except::eval_error("Illegal character", Parse_Location(m_position, m_position));
						}
						auto end_line = (*m_position != 0) && ((*m_position == '\n') || (*m_position == '\r' && *(m_position + 1) == '\n'));

						if (char_in_alphabet(*m_position, Engine::white_alphabet) || (skip_cr && end_line)) {
							if (end_line) {
								if (*m_position == '\r') {
									// discards lf
									++m_position;
								}
							}

							++m_position;

							retval = true;
						}
						else if (SkipComment()) {
							retval = true;
						}
						else {
							break;
						}
					}
					return retval;
				};
			
				/// Reads until the end of the current statement
				bool Eos() {
					SkipWS();
					return Eol_(true);
				};
				/// Reads (and potentially captures) an end-of-line group from input
				bool Eol() {
					SkipWS();
					return Eol_();
				};
				/// Reads (and potentially captures) a char from input if it matches the parameter
				bool Char(const char t_c) {
					SkipWS();
					return Char_(t_c);
				};
				/// Reads (and potentially captures) a string from input if it matches the parameter
				bool Keyword(const utility::Static_String& t_s) {
					SkipWS();
					const auto start = m_position;
					bool retval = Keyword_(t_s);
					// ignore substring matches
					if (retval && m_position.has_more() && char_in_alphabet(*m_position, Engine::keyword_alphabet)) {
						m_position = start;
						retval = false;
					}
					return retval;
				};
				/// Reads (and potentially captures) a symbol group from input if it matches the parameter
				bool Symbol(const GL::string& t_s, const bool t_disallow_prevention = false) {
					SkipWS();
					const auto start = m_position;
					bool retval = Symbol_(t_s);

					// ignore substring matches
					if (retval && m_position.has_more() && (t_disallow_prevention == false) && char_in_alphabet(*m_position, Engine::symbol_alphabet)) {
						if (*m_position != '=' && is_operator(Engine::Position::str(start, m_position)) && !is_operator(Engine::Position::str(start, m_position + 1))) {
							// don't throw this away, it's a good match and the next is not
						}
						else {
							m_position = start;
							retval = false;
						}
					}
					return retval;
				}
				/// Reads (and potentially captures) a number from the input, detecting if it's an integer or floating point
				bool Num() {
					SkipWS();
					return Num_();
				};
				/// 
				bool Operator_Helper(const size_t t_precedence, GL::string& oper) {
					return Operator_Matches::any_of(t_precedence, [&oper, this](const auto& elem) {
						if (Symbol(std::string_view(elem))) {
							oper = std::string_view(elem);
							return true;
						}
						else {
							return false;
						}
					});
				};
				/// Reads (and potentially captures) a quoted string from input.  Translates escaped sequences.
				bool Quoted_String() {
					SkipWS();

					const auto start = m_position;

					if (Quoted_String_()) {
						GL::string match;
						const auto prev_stack_top = m_match_stack.size();

						bool is_interpolated = [&]() -> bool {
							Char_Parser<GL::string> cparser(match, true);

							auto s = start + 1, end = m_position - 1;

							while (s != end) {
								if (cparser.saw_interpolation_marker) {
									if (*s == '{') {
										// We've found an interpolation point

										m_match_stack.push_back(make_const(match, start, match));
										if (cparser.is_interpolated) {
											// If we've seen previous interpolation, add on instead of making a new one
											build_match<Binary_Operator_Node>(prev_stack_top, "+");
										}

										// We've finished with the part of the string up to this point, so clear it
										match = "";

										GL::string eval_match;

										++s;
										while ((s != end) && (*s != '}')) {
											eval_match = eval_match + std::string(1, *s);
											++s;
										}

										if (*s == '}') {
											cparser.is_interpolated = true;
											++s;

											const auto tostr_stack_top = m_match_stack.size();

											m_match_stack.push_back(make_node<Id_Node>("to_string", start)); //  Id_AST_Node

											const auto ev_stack_top = m_match_stack.size();

											try {
												m_match_stack.push_back(parse_instr_eval(eval_match));
											}
											catch (const except::eval_error& e) {
												throw except::eval_error(std::string(e.what()), Parse_Location(start, start));
											}									

											//if (
											//	m_match_stack.back().identifier == Engine::AST_Node_Type::File
											//	&& m_match_stack.back().children.size() == 1
											//) {
											//	m_match_stack.back() = m_match_stack.back().children[0];
											//}

											build_match<Arg_List_Node>(ev_stack_top);
											build_match<Fun_Call_Node>(tostr_stack_top);
											build_match<Binary_Operator_Node>(prev_stack_top, "+");
										}
										else {
											throw except::eval_error("Unclosed in-string eval", Parse_Location(start, start));
										}
									}
									else {
										match = match + "$";
									}
									cparser.saw_interpolation_marker = false;
								}
								else {
									cparser.parse(*s, start);

									++s;
								}
							}

							if (cparser.saw_interpolation_marker) {
								match = match + "$";
							}

							return cparser.is_interpolated;
						}();

						m_match_stack.push_back(make_const(match, start, match));
						if (is_interpolated) {
							build_match<Binary_Operator_Node>(prev_stack_top, "+");						
						}

						return true;
					}
					else {
						return false;
					}
				};
				/// Reads (and potentially captures) an identifier from input
				bool Id_Impl(const bool validate) {
					SkipWS();

					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (Id_()) {
						GL::string text = Engine::Position::str(prev_pos, m_position);
						if (validate) { 
							if (!validate_object_name(text, m_position)) {
								return failure();
							}
						}

						auto foundConstant = constants().find(text);
						if (foundConstant != constants().end()) {
							m_match_stack.push_back(make_const(text, prev_pos, foundConstant->second));
						}
						else {
							switch (Engine::hash(text.c_str())) {
							case Engine::hash("__LINE__"): {
								m_match_stack.push_back(make_const(text, prev_pos, const_var(
									prev_pos.line
								)));
							} break;
							case Engine::hash("__VERSION__"): {
								m_match_stack.push_back(make_const(text, prev_pos, const_var(
									GL::string("1.0")
								)));
							} break;
							case Engine::hash("__DATE__"): {
								m_match_stack.push_back(make_const(text, prev_pos, const_var(
									std::to_string(GL::datetime::Now().tm_mon() + 1) + "/"
									+ std::to_string(GL::datetime::Now().tm_mday()) + "/"
									+ std::to_string(GL::datetime::Now().tm_year() + 1900
								))));
							} break;
							case Engine::hash("__TIME__"): {
								m_match_stack.push_back(make_const(text, prev_pos, const_var(
									std::to_string(GL::datetime::Now().tm_hour()) + ":"
									+ std::to_string(GL::datetime::Now().tm_min()) + ":"
									+ std::to_string(GL::datetime::Now().tm_sec())
								)));
							} break;
							case Engine::hash("__TIMESTAMP__"): {
								m_match_stack.push_back(make_const(text, prev_pos, const_var(
									GL::datetime::Now().c_str()
								)));
							} break;								
							//case hash("__FILE__"): {
							//	m_match_stack.push_back(make_node<eval::Constant_AST_Node>(text, prev_pos.line, prev_pos.col, const_var(m_filename)));
							//} break;
							default: {
								auto val = text;
								if (*prev_pos == '`') { // 'escaped' literal, like an operator name ( e.g. `[]`(...) )
									val = Engine::Position::str(prev_pos + 1, m_position - 1);
								}
								m_match_stack.push_back(make_node<Id_Node>(val, prev_pos)); // e.g. "x", "Units::meter", etc.							
							} break;
							}
						}
						return true;
					}
					else {
						return false;
					}				
				};

				/// Reads (and potentially captures) an identifier from input
				bool Id(const bool validate) {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (Id_Impl(validate)) {
						if (optimizer::OptimizationDepth() > 0) {
							const auto prev_preprocessorif_top2 = m_preprocessor_if_stack.size();
							const auto prev_preprocessor_top2 = m_preprocessor_stack.size();
							const auto prev_comment_top2 = m_comment_stack.size();
							const auto prev_stack_top2 = m_match_stack.size();
							const auto prev_pos2 = m_position;
							auto failure2 = [&]() {
								while (m_match_stack.size() != prev_stack_top2) m_match_stack.pop_back();
								while (m_comment_stack.size() != prev_comment_top2) m_comment_stack.pop_back();
								while (m_preprocessor_stack.size() != prev_preprocessor_top2) m_preprocessor_stack.pop_back();
								while (m_preprocessor_if_stack.size() != prev_preprocessorif_top2) m_preprocessor_if_stack.pop_back();
								m_position = prev_pos2;
								return false;
							};

							if (Symbol("##") && Id(validate)) {
								Id_Node temp("##", GL::Engine::Parse_Location(m_match_stack[m_match_stack.size() - 2].location.start, m_match_stack[m_match_stack.size() - 1].location.end), { m_match_stack[m_match_stack.size() - 2], m_match_stack[m_match_stack.size() - 1] });
								m_match_stack.pop_back();
								m_match_stack.pop_back();
								m_match_stack.push_back(temp);
								// build_match<Id_Node>(prev_stack_top - 1, "##");
								return true;
							}
							else {
								failure2();
							}
						}
						return true;						
					}
					else {
						return failure();
					}
				};

				/// Reads (and potentially captures) an type or class identifier from input
				bool TypeName(bool allowAuto = false) {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (Keyword("const")) {
						SkipWS(false);
						if (Id(false)) {
							SkipWS(false);
							if (Keyword("&")) {
								if (Keyword("&")) {
									m_match_stack.back().text = m_match_stack.back().text + "&&";
								}
								else {
									m_match_stack.back().text = "const " + m_match_stack.back().text + "&";
								}
								return true;
							}
							else {
								m_match_stack.back().text = "const " + m_match_stack.back().text;
								return true;
							}
						}
						return failure();
					}
					else {
						if (Id(false)) {
							SkipWS(false);
							if (Keyword("&")) {
								if (Keyword("&")) {
									m_match_stack.back().text = m_match_stack.back().text + "&&";
								}
								else {
									m_match_stack.back().text = m_match_stack.back().text + "&";
								}
								return true;
							}
							else if (Keyword("const")) {
								SkipWS(false);
								if (Keyword("&")) {
									if (Keyword("&")) {
										m_match_stack.back().text = m_match_stack.back().text + "&&";
									}else{
										m_match_stack.back().text = "const " + m_match_stack.back().text + "&";
									}
									return true;
								}
								else {
									m_match_stack.back().text = "const " + m_match_stack.back().text;
									return true;
								}
							}
							else {
								return true;
							}
						}
					}
					return failure();
				};

				/// Reads an argument from input
				bool Arg(const bool t_type_allowed = true) {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					SkipWS();

					bool foundType = false;
					if (t_type_allowed) {
						foundType = TypeName();
					}

					if (!Id(true)) {
						if (!foundType) {
							return failure();
						}
					}

					build_match<Arg_Node>(prev_stack_top);

					return true;
				};

				/// Reads a comma-separated list of values from input. Id's only, no types allowed
				bool Id_Arg_List() {
					SkipWS(true);

					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (Arg(false)) {
						SkipWS(true);
						while (Char(',')) {
							SkipWS(true);
							if (!Arg(false)) {
								return failure(); // throw except::eval_error("Unexpected value in parameter list", m_position);
							}
						}
					}
					build_match<Arg_List_Node>(prev_stack_top);

					SkipWS(true);

					return true;
				};

				/// Reads a comma-separated list of values from input, for function declarations
				bool Decl_Arg_List() {
					SkipWS(true);

					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (Arg(true)) {
						SkipWS(true);
						while (Char(',')) {
							SkipWS(true);
							if (!Arg(true)) {
								return failure(); //  throw except::eval_error("Unexpected value in parameter list", m_position);
							}
						}
					}
					build_match<Arg_List_Node>(prev_stack_top);

					SkipWS(true);

					return true;
				};

				/// Reads a comma-separated list of values from input
				bool Arg_List(int maxNumArgs = std::numeric_limits<int>::max()) {
					SkipWS(true);

					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (Equation()) {
						SkipWS(true);
						while (((--maxNumArgs) > 0)) {
							SkipWS(true);
							if (!Char(',')) break;
							SkipWS(true);
							if (!Equation()) {
								return failure(); // throw except::eval_error("Unexpected value in parameter list", m_position);
							}
						}
					}

					build_match<Arg_List_Node>(prev_stack_top);

					SkipWS(true);

					return true;
				};

				/// Reads a C-style type-cast from input (e.g. (int)0.0f )
				bool TypeCastOperation() {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					SkipWS(true);

					// (string)100
					if (Char('(') && TypeName() && Char(')')) {
						if (Postfix(true)) {
							return true;
						}
						else {
							while (TypeCastOperation() || /*Postfix(false) || */Dot_Fun_Array() || Prefix()) {}
							Postfix(true);
							if ((m_match_stack.size() - prev_stack_top) >= 2) {
								build_match<Type_Cast_Node>(prev_stack_top);
								return true;
							}
							else {
								return failure();
							}

							//if (Dot_Fun_Array() || TypeCastOperation() || Prefix() || Paren_Expression() || Quoted_String() || Num() || Value()) {
							//	Postfix(true);
							//	build_match<Type_Cast_Node>(prev_stack_top);
							//	return true;
							//}


							//if (Quoted_String() || Paren_Expression() || Num() || Value() || Prefix() || Id(false) || TypeCastOperation() || Dot_Fun_Array()) {
							//	Postfix(true);
							//	build_match<Type_Cast_Node>(prev_stack_top);
							//	return true;
							//}
						}

						









						//if (Postfix(true)) {
						//	return true;
						//}
						//else if (Operator()) { // if (TypeCastOperation() || Value()) {
						//	build_match<Type_Cast_Node>(prev_stack_top);
						//	return true;						
						//}					
					}

					return failure();
				};

				/// Parses a string of binary equation operators
				bool Equation() {
					const auto prev_stack_top = m_match_stack.size();
					using SS = utility::Static_String;

					if (Operator()) {
						for (const auto& sym :
							{ SS{"="}, SS{":="}, SS{"?="}, SS{".."}, SS{"+="}, SS{"-="}, SS{"*="}, SS{"/="}, SS{"%="}, SS{"<<="}, SS{">>="}, SS{"&="}, SS{"^="}, SS{"|="} }) {
							if (Symbol(std::string_view(sym.c_str()), true)) {
								SkipWS(true);
								if (Equation() || Value()) {
									build_match<Equation_Node>(prev_stack_top, std::string_view(sym.c_str()));
									return true;									
								}
								throw except::eval_error("Incomplete equation", Parse_Location(m_position, m_position));
							}
						}
						return true;
					}

					return false;
				};

				/// Reads a variable declaration from input
				bool Var_Decl() {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					bool is_constexpr = false;

					if (Keyword("constexpr")) {
						is_constexpr = true;
					}
					// auto x;
					// auto const& x;
					// auto& x;
					if (Keyword("const auto&") || Keyword("auto const&") || Keyword("auto&") || Keyword("const auto") || Keyword("auto const") || Keyword("auto")) {
						if (Id(true)) {
							build_match<Var_Decl_Node>(prev_stack_top);
							if (is_constexpr) {
								m_match_stack.back().tag.cast<ObjectDeclarationInformation&>().is_constexpr = true;
							}
							return true;
						}
					}
					else {
						// int x;
						// int& x;
						// const int& x;
						if (TypeName()) { // captures Id{ TypeName }
							build_match<Arg_List_Node>(prev_stack_top + 1); // {no_params}
							build_match<Fun_Call_Node>(prev_stack_top); // Fun_Call{ Id{TypeName}, ArgList{} }
							if (Id(true)) {
								build_match<Var_Decl_Node>(prev_stack_top + 1);  // var i;
								if (is_constexpr) {
									m_match_stack.back().tag.cast<ObjectDeclarationInformation&>().is_constexpr = true;
								}
								auto var_decl_node = m_match_stack[m_match_stack.size() - 1];
								if (is_constexpr) {
									var_decl_node.tag.cast<ObjectDeclarationInformation&>().is_constexpr = true;
								}
								auto fun_call_node = m_match_stack[m_match_stack.size() - 2];
								m_match_stack.pop_back();
								m_match_stack.pop_back();
								m_match_stack.push_back(Assign_Retroactively_Node("", GL::Engine::Parse_Location(fun_call_node.location.start, var_decl_node.location.end), { fun_call_node, var_decl_node }));
								if (is_constexpr) {
									m_match_stack.back().tag.cast<ObjectDeclarationInformation&>().is_constexpr = true;
								}
								return true;

								//auto LHS = m_match_stack[m_match_stack.size() - 1];
								//auto RHS = m_match_stack[m_match_stack.size() - 2];
								//m_match_stack[m_match_stack.size() - 1] = RHS;
								//m_match_stack[m_match_stack.size() - 2] = LHS;
								//build_match<Equation_Node>(prev_stack_top, "=");
								//return true;
							}
						}





						//if (TypeName()) {
						//	build_match<Arg_List_Node>(prev_stack_top + 1); // {no_params}
						//	build_match<Fun_Call_Node>(prev_stack_top); // Fun_Call{ Id{TypeName}, ArgList{} }
						//	if (Id(true)) {
						//		build_match<Var_Decl_Node>(prev_stack_top + 1);  // var i;               
						//		build_match<Assign_Retroactively_Node>(prev_stack_top); // Assign_Retroactively_Node{ Fun_Call{ Id{TypeName}, ArgList{} }, Id{ VariableName } }
						//		if (is_constexpr) {
						//			m_match_stack.back().tag.cast<ObjectDeclarationInformation&>().is_constexpr = true;
						//		}
						//		return true;
						//	}
						//}
					}
					return failure();
				};

				/// Reads a unary prefixed expression from input
				bool Prefix() {
					const auto prev_stack_top = m_match_stack.size();					

					using SS = utility::Static_String;
					auto& prefix_opers = Operator_Matches::Data().m_11;
					for (const auto& oper : prefix_opers) {
						if (std::string_view(oper.c_str()) == "#") {
							if (optimizer::OptimizationDepth() == 0)
								continue;
						}

						const bool is_char = oper.size() == 1;
						if ((is_char && Char(oper.c_str()[0])) || (!is_char && Symbol(std::string_view(oper.c_str())))) {							
							if (!Operator(operators().size() - 1)) {
								throw except::eval_error("Incomplete prefix '" + GL::string(std::string_view(oper.c_str())) + "' expression", Parse_Location(m_position, m_position));
							}

							build_match<Prefix_Node>(prev_stack_top, std::string_view(oper.c_str()));

							return true;
						}
					}
					return false; //  failure();
				};

				static auto make_postfix_operators() {
					std::multimap<size_t, std::pair<GL::string, std::pair<GL::value, GL::any::fast_any>>, std::greater_equal<size_t>> out;
					for (auto& unit_type : GL::value::abbreviations_to_type()) {
						auto abbreviation = GL::string("_") + unit_type.first;
						out.insert({ abbreviation.length(), { abbreviation, { unit_type.second.cast<GL::value>(), unit_type.second } } });
					}
					return out;
				}

				bool Postfix(bool gotValueAlready) {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					// add support for custom post-fixes
					// Examples: 
					// 12_in = inch(12)
					// 1_gal = gallon(1)
					if (gotValueAlready || (Num() || Id(true) || Paren_Expression())) {
						if (Symbol("++")) {
							build_match<Postfix_Node>(prev_stack_top - gotValueAlready, "++");
							m_match_stack.back().tag.cast<PostfixInformation>().is_unit = false;
							return true;
						}
						else if (Symbol("--")) {
							build_match<Postfix_Node>(prev_stack_top - gotValueAlready, "--");
							m_match_stack.back().tag.cast<PostfixInformation>().is_unit = false;
							return true;
						}
						else {
							static auto
								customOperators{ make_postfix_operators() };

							// evaluate the custom operators...
							if (m_match_stack.back().identifier == Engine::AST_Node_Type::Constant) {
								// this path means the incoming value is constant. Compile to constant. 
								for (auto& unit_type : customOperators) {
									auto& abbreviation = unit_type.second.first;
									if (Symbol(abbreviation, true)) {
										auto& rhs = m_match_stack.back().constant;
										if (auto* BC = this->analysis_engine.try_find_class(unit_type.second.second.second.m_casted_type); BC && BC->this_m.is_class()) {
											auto result = BC->this_m.scope->call(BC->this_m.scope_name, { rhs | GL::type::Const | GL::type::Reference });
											// GL::string temp = BC->this_m.scope->call<GL::string>("to_string", { result | GL::type::Const | GL::type::Reference });
											m_match_stack.back() = Constant_Node(m_match_stack.back().text, m_match_stack.back().location, {}, result);
											return true;
										}
									}								
								}
							}
							else {
								// this path means the incoming value is NOT constant. Compile to a function call. 
								for (auto& unit_type : customOperators) {
									auto& abbreviation = unit_type.second.first;
									if (Symbol(abbreviation, true)) {
										build_match<Postfix_Node>(prev_stack_top - gotValueAlready, unit_type.second.second.first.abbreviation());
										m_match_stack.back().tag.cast<PostfixInformation>().is_unit = true;
										m_match_stack.back().tag.cast<PostfixInformation>().unit_name = unit_type.second.second.second.m_casted_type.name();
										return true;
									}								
								}
							}
						}
					}					
					return failure();
				}

				/// Reads a pair of values used to create a map initialization from input
				bool Map_Pair() {
					SkipWS(true);

					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (Operator()) {
						if (Symbol(":")) {
							if (!Operator()) { throw except::eval_error("Incomplete map pair", Parse_Location(m_position, m_position)); }
							build_match<Map_Pair_Node>(prev_stack_top);
							return true;
						}
					}

					return failure();
				}

				/// Reads possible special container values, including ranges and map_pairs
				bool Container_Arg_List() {
					SkipWS(true);
					bool retval = false;

					const auto prev_stack_top = m_match_stack.size();

					if (Map_Pair()) {
						retval = true;
						SkipWS(true);
						while (Char(',')) {
							SkipWS(true);
							if (!Map_Pair()) {
								throw except::eval_error("Unexpected value in container", Parse_Location(m_position, m_position));
							}
						}
						build_match<Arg_List_Node>(prev_stack_top);
					}
					else if (Operator()) {
						retval = true;
						SkipWS(true);
						while (Char(',')) {
							SkipWS(true);
							if (!Operator()) {
								throw except::eval_error("Unexpected value in container", Parse_Location(m_position, m_position));
							}
							SkipWS(true);
						}
						build_match<Arg_List_Node>(prev_stack_top);
					}

					SkipWS(true);

					return retval;
				}

				/// Reads, and identifies, a short-form container initialization from input
				bool Inline_Container() {
					const auto prev_stack_top = m_match_stack.size();

					if (Char('[')) {
						SkipWS(true);
						Container_Arg_List();
						SkipWS(true);
						if (!Char(']')) {
							throw except::eval_error("Missing closing square bracket ']' in container initializer", Parse_Location(m_position, m_position));
						}
						if ((prev_stack_top != m_match_stack.size()) && (!m_match_stack.back().children.empty())) {
							if (m_match_stack.back().children[0].identifier == Engine::AST_Node_Type::Map_Pair) {
								build_match<Inline_Map_Node>(prev_stack_top);
							}
							else {
								build_match<Inline_Array_Node>(prev_stack_top);
							}
						}
						else {
							build_match<Inline_Array_Node>(prev_stack_top);
						}

						return true;
					}
					else {
						return false;
					}
				}

				/// Reads a lambda (anonymous function) from input
				bool Lambda() {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					/* All of the following should be examples of valid forms of lambda functions */
					// [...](...) async -> typename {...} 
					// [...](...) -> typename {...} 
					// [...](...) async {...} 
					// (...) async {...} 
					// (...) {...} 

					// Arg_List
					if (Char('[')) {
						SkipWS(true);
						Id_Arg_List();
						SkipWS(true);
						if (!Char(']')) {
							return failure();
						}
					}
					else {
						// make sure we always have the same number of nodes
						build_match<Arg_List_Node>(prev_stack_top);
					}

					// Arg_List
					if (Char('(')) {
						SkipWS(true);
						Decl_Arg_List();
						SkipWS(true);
						if (!Char(')')) {
							return failure();
						}
					}
					else {
						return failure();
					}

					// KeyWords / modifiers
					bool is_async = false;
					bool foundKeyWord = true;
					while (foundKeyWord) {
						SkipWS(true);
						foundKeyWord = false;

						if (Keyword("async")) {
							foundKeyWord = true;
							is_async = true;
						}
					}

					SkipWS(true);

					// Noop or Id
					if (Symbol("->")) {
						SkipWS(true);
						const auto start = m_position;
						if (!TypeName()) {
							return failure();
						}
					}
					else {
						// make sure we always have the same number of nodes
						m_match_stack.push_back(Noop_Node("", {}, {}));
					}

					// Block
					SkipWS(true);
					if (!Block()) {
						return failure();
					}

					auto lambda_node = build_match<Lambda_Node>(prev_stack_top);
					// lambda_node.is_async = is_async;

					return true;
				};

				/// Parses a chain of function or member calls, such as: 
				/// `x.y.z` or `x.function().z` or `function(x,y,z).w.function(a).b`
				bool Dot_Fun_Array() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();

					if (Eval() || Lambda() || Postfix(false) || Num() || Quoted_String() || Paren_Expression() || Inline_Container() || Id(false)) {
						retval = true;
						bool has_more = true;

						while (has_more) {
							has_more = false;
							if (Char('(')) {
								has_more = true;
								SkipWS(true);
								Arg_List();
								SkipWS(true);
								if (!Char(')')) {
									throw except::eval_error("Incomplete function call", Parse_Location(m_position, m_position));
								}

								build_match<Fun_Call_Node>(prev_stack_top, "()");
								/// \todo Work around for method calls until we have a better solution
								if (!m_match_stack.back().children.empty()) {
									if (m_match_stack.back().children[0].identifier == Engine::AST_Node_Type::Dot_Access) {
										if (m_match_stack.empty()) {
											throw except::eval_error("Incomplete dot access fun call", Parse_Location(m_position, m_position));
										}
										if (m_match_stack.back().children.empty()) {
											throw except::eval_error("Incomplete dot access fun call", Parse_Location(m_position, m_position));
										}
										auto dot_access = std::move(m_match_stack.back().children[0]);
										auto func_call = std::move(m_match_stack.back());
										m_match_stack.pop_back();
										func_call.children.erase(func_call.children.begin());
										if (dot_access.children.empty()) {
											throw except::eval_error("Incomplete dot access fun call", Parse_Location(m_position, m_position));
										}
										func_call.children.insert(func_call.children.begin(), std::move(dot_access.children.back()));
										dot_access.children.pop_back();
										dot_access.children.push_back(std::move(func_call));
										if (dot_access.children.size() != 2) {
											throw except::eval_error("Incomplete dot access fun call", Parse_Location(m_position, m_position));
										}
										m_match_stack.push_back(dot_access);
									}
								}
							}
							else if (Char('[')) {
								has_more = true;
								if (!(Operator() && Char(']'))) {
									// TO-DO, Extend to allow matrix accessors, i.e. matrix_obj[0,0] = 10.0;
									throw except::eval_error("Incomplete array access", Parse_Location(m_position, m_position));
								}

								build_match<Array_Call_Node>(prev_stack_top, "[]");
							}
							else if (Symbol(".")) {
								has_more = true;
								if (!(Id(true))) {
									throw except::eval_error("Incomplete dot access fun call", Parse_Location(m_position, m_position));
								}

								if (std::distance(m_match_stack.begin() + static_cast<int>(prev_stack_top), m_match_stack.end()) != 2) {
									throw except::eval_error("Incomplete dot access fun call", Parse_Location(m_position, m_position));
								}

								build_match<Dot_Access_Node>(prev_stack_top, ".");
							}
							else if (Eol()) {
								auto start = (--m_position);
								SkipWS(true);
								if (Symbol(".")) {
									has_more = true;
									--m_position;
								}
								else {
									m_position = start;
								}
							}
						}
					}

					return retval;
				};

				/// Parses any of a group of 'value' style ast_node groups from input
				bool Value() {
					if (Var_Decl() || Dot_Fun_Array() || Prefix()) {
						Postfix(true);
						return true;
					}
					else {
						return Postfix(false);
					}
				};

				/// Parses equation components that are meant to be processed together.
				/// For example, `+`, `-`, and `%` would be operators. 
				/// Whereas `+=`, `=`, and `%=` would be equations.
				bool Operator(const size_t t_precedence = 0) {
					bool retval = false;
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (operators()[t_precedence] != Engine::Operator_Precedence::Prefix) {
						if (Operator(t_precedence + 1)) {
							GL::string oper;
							retval = true;
							while (Operator_Helper(t_precedence, oper)) {
								while (Eol()) {}

								if (!Operator(t_precedence + 1)) {
									throw except::eval_error("Incomplete '" + oper + "' expression", Parse_Location(m_position, m_position));
								}

								switch (operators()[t_precedence]) {
								case (Engine::Operator_Precedence::Ternary_Cond):
									if (oper == "?=") {
										build_match<Equation_Node>(prev_stack_top, oper);
									}
									else {
										if (Symbol(":")) {
											if (!Operator(t_precedence + 1)) {
												throw except::eval_error("Incomplete '" + oper + "' expression", Parse_Location(m_position, m_position));
											}
											build_match<If_Node>(prev_stack_top);
										}
										else {
											throw except::eval_error("Incomplete '" + oper + "' expression", Parse_Location(m_position, m_position));
										}
									}
									break;

								case (Engine::Operator_Precedence::Addition):
								case (Engine::Operator_Precedence::Multiplication):
								case (Engine::Operator_Precedence::Shift):
								case (Engine::Operator_Precedence::Equality):
								case (Engine::Operator_Precedence::Bitwise_And):
								case (Engine::Operator_Precedence::Bitwise_Xor):
								case (Engine::Operator_Precedence::Bitwise_Or):
								case (Engine::Operator_Precedence::Comparison):
									//if (oper == "##") {
									//	if (optimizer::OptimizationDepth() > 0) {
									//		build_match<Binary_Operator_Node>(prev_stack_top, oper);
									//	}
									//	else {
									//		return failure();
									//	}
									//}
									//else {
										build_match<Binary_Operator_Node>(prev_stack_top, oper);
									//}
									break;

								case (Engine::Operator_Precedence::Logical_And):
									build_match<Logical_And_Node>(prev_stack_top, oper);
									break;
								case (Engine::Operator_Precedence::Logical_Or):
									build_match<Logical_Or_Node>(prev_stack_top, oper);
									break;
								case (Engine::Operator_Precedence::Prefix):
									ASSERT(false); // cannot reach here because of if() statement at the top
									break;
								}
							}
						}
					}
					else {
						if (TypeCastOperation()) { return true; }
						else return Value();
					}

					return retval;
				}

				/// Reads an expression surrounded by parentheses from input
				bool Paren_Expression() {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (Char('(')) {
						SkipWS(true);
						if (!Operator()) {
							return failure();
						}
						SkipWS(true);
						if (!Char(')')) {
							// return failure();					
							throw except::eval_error("Missing closing parenthesis ')'", Parse_Location(m_position, m_position));
						}
						return true;
					}
					else {
						return false;
					}
				};

				/// Reads a while block from input
				bool While() {
					bool retval = false;

					const auto prev_stack_top = m_match_stack.size();

					if (Keyword("while")) {
						retval = true;

						if (!Char('(')) {
							throw except::eval_error("Incomplete 'while' expression", Parse_Location(m_position, m_position));
						}

						if (!(Operator() && Char(')'))) {
							throw except::eval_error("Incomplete 'while' expression", Parse_Location(m_position, m_position));
						}

						SkipWS(true);

						if (!Block()) {
							if (!SingleStatement()) {
								throw except::eval_error("Incomplete 'while' block", Parse_Location(m_position, m_position));
							}
						}

						build_match<While_Node>(prev_stack_top);
					}

					return retval;
				};

				/// Reads the C-style `for` conditions from input
				bool For_Guards() {
					if (!(Equation() && Eol())) {
						if (!Eol()) {
							return false;
						}
						else {
							m_match_stack.push_back(Noop_Node("", {}, {}));
						}
					}

					if (!(Equation() && Eol())) {
						if (!Eol()) {
							return false;
						}
						else {
							m_match_stack.push_back(make_const("", m_position, true));
						}
					}

					if (!Equation()) {
						m_match_stack.push_back(Noop_Node("", {}, {}));
					}

					return true;
				}

				/// Reads the ranged `for` conditions from input
				bool Range_Expression() {
					// the first element will have already been captured by the For_Guards() call that preceeds it
					return Char(':') && Equation();
				}

				/// Reads a for block from input
				bool For() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();

					bool is_parallel = false;
					bool do_work = false;
					if (Keyword("parallel_for")) {
						is_parallel = true;
						do_work = true;
					}
					else if (Keyword("for")) {
						do_work = true;
					}
			
					if (do_work) {
						retval = true;

						SkipWS(true);

						if (!Char('(')) {
							throw except::eval_error("Incomplete 'for' expression", Parse_Location(m_position, m_position));
						}

						SkipWS(true);

						bool classic_for = For_Guards();
						SkipWS(true);
						if (classic_for) classic_for = classic_for && Char(')');
						if (!classic_for) {
							classic_for = Range_Expression();
							SkipWS(true);
							if (classic_for) classic_for = classic_for && Char(')');

							if (!classic_for) {
								throw except::eval_error("Incomplete 'for' expression", Parse_Location(m_position, m_position));
							}

							classic_for = false;
						}

						SkipWS(true);

						if (!Block()) {
							if (!SingleStatement()) {
								throw except::eval_error("Incomplete 'for' block", Parse_Location(m_position, m_position));
							}
						}

						const auto num_children = m_match_stack.size() - prev_stack_top;

						if (classic_for) {
							if (num_children != 4) {
								throw except::eval_error("Incomplete 'for' expression", Parse_Location(m_position, m_position));
							}
							build_match<For_Node>(prev_stack_top, "", is_parallel);
						}
						else {
							if (num_children != 3) {
								throw except::eval_error("Incomplete ranged-for expression", Parse_Location(m_position, m_position));
							}
							build_match<Ranged_For_Node>(prev_stack_top, "", is_parallel);
						}
					}				

					return retval;
				}

				/// Reads a break statement from input
				bool Break() {
					const auto prev_stack_top = m_match_stack.size();
					if (Keyword("break")) {
						build_match<Break_Node>(prev_stack_top);
						return true;
					}
					else {
						return false;
					}
				}

				/// Reads a continue statement from input
				bool Continue() {
					const auto prev_stack_top = m_match_stack.size();
					if (Keyword("continue")) {
						build_match<Continue_Node>(prev_stack_top);
						return true;
					}
					else {
						return false;
					}
				}

				/// Reads a case block from input
				bool Case() {
					bool retval = false;

					const auto prev_stack_top = m_match_stack.size();

					// case "option": { ... }
					// case "option" { ... }
					if (Keyword("case")) {
						retval = true;

						SkipWS(true);

						if (!Operator()) {
							throw except::eval_error("Incomplete 'case' expression", Parse_Location(m_position, m_position));
						}

						SkipWS(true);

						(void)Char(':'); // optional

						SkipWS(true);

						if (!Block()) {
							if (!SingleStatement()) {
								throw except::eval_error("Incomplete 'case' block", Parse_Location(m_position, m_position));
							}						
						}

						build_match<Case_Node>(prev_stack_top);
					}
					// default: { ... }
					// default { ... }
					else if (Keyword("default")) {
						retval = true;

						SkipWS(true);

						(void)Char(':'); // optional

						SkipWS(true);

						if (!Block()) {
							if (!SingleStatement()) {
								throw except::eval_error("Incomplete 'default' block", Parse_Location(m_position, m_position));
							}
						}

						build_match<Default_Node>(prev_stack_top);
					}

					return retval;
				};

				/// Reads a switch statement from input
				bool Switch() {
					const auto prev_stack_top = m_match_stack.size();

					if (Keyword("switch")) {
						if (!Char('(')) {
							throw except::eval_error("Incomplete 'switch' expression", Parse_Location(m_position, m_position));
						}

						if (!(Operator() && Char(')'))) {
							throw except::eval_error("Incomplete 'switch' expression", Parse_Location(m_position, m_position));
						}

						SkipWS(true);

						if (Char('{')) {
							SkipWS(true);

							while (Case()) {
								SkipWS(true);
							}

							SkipWS(true);

							if (!Char('}')) {
								throw except::eval_error("Incomplete block", Parse_Location(m_position, m_position));
							}
						}
						else {
							throw except::eval_error("Incomplete block", Parse_Location(m_position, m_position));
						}

						build_match<Switch_Node>(prev_stack_top);
						return true;

					}
					else {
						return false;
					}
				}

				/// Reads a try-catch from input
				bool Try() {
					bool retval = false;
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					if (Keyword("throw")) {
						SkipWS(true);
						if (!SingleStatement()) {
							throw except::eval_error("Incomplete 'throw' statement", Parse_Location(m_position, m_position));
						}
						build_match<Throw_Node>(prev_stack_top);
						return true;
					}

					if (Keyword("try")) {
						retval = true;

						SkipWS(true);

						if (!Block()) {
							throw except::eval_error("Incomplete 'try' block", Parse_Location(m_position, m_position));
						}

						bool has_matches = true;
						while (has_matches) {
							SkipWS(true);
							has_matches = false;
							if (Keyword("catch")) {
								const auto catch_stack_top = m_match_stack.size();
								if (Char('(')) {
									bool success = false;
									if (Symbol("...")) {
										// captures anything...
										if (!Char(')')) {
											throw except::eval_error("Incomplete 'catch(...)' expression", Parse_Location(m_position, m_position));
										}
										success = true;
									}

									if (Arg(true)) {
										if (!Char(')')) {
											throw except::eval_error("Incomplete 'catch' expression", Parse_Location(m_position, m_position));
										}
										success = true;
									}

									if (!success) {
										throw except::eval_error("Incomplete 'catch' expression", Parse_Location(m_position, m_position));
									}
								}

								SkipWS(true);

								if (!Block()) {
									throw except::eval_error("Incomplete 'catch' block", Parse_Location(m_position, m_position));
								}
								build_match<Catch_Node>(catch_stack_top);
								has_matches = true;
							}
						}
						SkipWS(true);
						if (Keyword("finally")) {
							const auto finally_stack_top = m_match_stack.size();

							SkipWS(true);

							if (!Block()) {
								throw except::eval_error("Incomplete 'finally' block", Parse_Location(m_position, m_position));
							}
							build_match<Finally_Node>(finally_stack_top);
						}

						build_match<Try_Node>(prev_stack_top);
					}
					//else if (Keyword("do")) {
					//	retval = true;
					//	SkipWS(true);
					//	if (!Block()) {
					//		throw except::eval_error("Incomplete 'do' block", m_position);
					//	}
					//	SkipWS(true);
					//	if (Keyword("finally")) {
					//		const auto finally_stack_top = m_match_stack.size();
					//		SkipWS(true);
					//		if (!Block()) {
					//			throw except::eval_error("Incomplete 'finally' block", m_position);
					//		}
					//		build_match<Finally_Node>(finally_stack_top);
					//	}
					//	build_match<Do_Node>(prev_stack_top);
					//}
					return retval;
				}

				/// Reads a just-in-time compilation request from input
				bool Eval() {
					// SkipWS(true);

					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();
					if (Keyword("evaluate")) {
						retval = true;
						SkipWS(true);
	#if 1
						if (!Char('(')) {
							throw except::eval_error("Incomplete 'evaluate' expression", Parse_Location(m_position, m_position));
						}
						SkipWS(true);
						if (!Equation()) {
							throw except::eval_error("Incomplete 'evaluate' expression", Parse_Location(m_position, m_position));
						}
						SkipWS(true);
						if (!Char(')')) {
							throw except::eval_error("Incomplete 'evaluate' expression", Parse_Location(m_position, m_position));
						}
	#else
						if (!Block()) {
							throw except::eval_error("Incomplete 'evaluate' block", m_position);
						}
	#endif
						build_match<JustInTimeCompilation_Node>(prev_stack_top);
					}
					return retval;
				}

				/// Reads a namespace block from input
				/// namespace Thing{ ... };
				bool DeclClass() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();

					// class TypeName { ... }
					// class TypeName : ParentTypeName { ... }

					if (Keyword("class")) {
						retval = true;
						SkipWS(true);

						if (TypeName(false)) { // variable becase this namespace may not exist yet! 
							/* Great! Got the desired name of the new namespace */
						}
						else {
							throw except::eval_error("Incomplete 'class' block: class must have a name", Parse_Location(m_position, m_position));
						}
						auto this_class_name = GL::string(m_match_stack.back().get_text());

						// instead of collecting statements, we want to collect declarations...
						if (!DeclarationsBlock()) {
							throw except::eval_error("Incomplete 'class' block: class declarations must be wrapped in a curly-bracket block", Parse_Location(m_position, m_position));
						}

						build_match<Class_Node>(prev_stack_top);
					}
					return retval;
				};

				/// Reads a namespace block from input
				/// namespace Thing{ ... };
				bool DeclNamespace() {
					bool retval = false;
					const auto prev_stack_top = m_match_stack.size();

					if (Keyword("namespace")) {
						retval = true;
						SkipWS(true);

						if (Id(true)) { // variable becase this namespace may not exist yet! 
							/* Great! Got the desired name of the new namespace */
						}
						else {
							throw except::eval_error("Incomplete 'namespace' block: namespace must have a name", Parse_Location(m_position, m_position));
						}

						auto this_class_name = m_match_stack.back().get_text();

						// instead of collecting statements, we want to collect declarations...
						if (!DeclarationsBlock()) {
							throw except::eval_error("Incomplete 'namespace' block: namespace declarations must be wrapped in a curly-bracket block", Parse_Location(m_position, m_position));
						}
						build_match<Namespace_Node>(prev_stack_top);
					}
					return retval;
				};

				/// Reads a declared function from input
				/// Type Foo(...){ ... };
				bool DeclFunction() {
					bool retval = false;
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};
					bool is_constexpr = false;

					// optional constexpr
					if (Symbol("constexpr") ) {
						is_constexpr = true;
					}

					// return type (Id)
					if (!TypeName(true)) { // Id
						return failure();
					}

					// function name (Id)
					if (!Id(true)) {
						return failure();
					}

					// Arg_List 
					if (Char('(')) {
						SkipWS(true);
						Decl_Arg_List();
						SkipWS(true);
						if (!Char(')')) {
							return failure();
						}
					}
					else {
						return failure();
					}
					SkipWS(true);
					// Function Characteristics
					// to-do

					// Block
					if (!Block()) {
						return failure();
					}

					build_match<FunctionDecl_Node>(prev_stack_top);
					m_match_stack.back().tag.cast<FunctionDeclInformation>().is_constexpr = true;
					return true;
				};

				/// Top level parser, starts parsing of all known parses
				bool Declarations() {
					SkipWS();

					bool retval = false;
					bool has_more = true;
					bool saw_eol = true;

					while (has_more) {
						const auto start = m_position;

						if ((this->m_preprocessor_if_stack.size() == 0) || (this->m_preprocessor_if_stack.back() == preprocessor_state::TRUE_UNTIL_ELSE)) {
							// TO-DO, complete impl of these evaluations:
							if (PreprocessorDirectives(true) || DeclNamespace() || DeclFunction() || DeclClass()) {
								if (!saw_eol) {
									throw except::eval_error("Two function definitions missing line separator", Parse_Location(start, start));
								}
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else if (Equation()) {
								if (!saw_eol) {
									throw except::eval_error("Two expressions missing line separator", Parse_Location(start, start));
								}
								has_more = true;
								retval = true;
								saw_eol = false;
							}
							else if (DeclarationsBlock() || Eol()) {
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else {
								has_more = false;
							}
						}
						else {							
							SkipWS();
							if (Quoted_String_()) {
								continue;
							}
							if (!PreprocessorDirectives(false)) {
								++m_position;
							}
						}

					}
					return retval;
				};

				/// Reads a single-line statement from input, such as those following an if()/while()/case: statement without the C-style {} braces. 
				bool SingleStatement() {
					const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
					const auto prev_preprocessor_top = m_preprocessor_stack.size();
					const auto prev_comment_top = m_comment_stack.size();
					const auto prev_stack_top = m_match_stack.size();
					const auto prev_pos = m_position;
					auto failure = [&]() {
						while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
						while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
						while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
						while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
						m_position = prev_pos;
						return false;
					};

					SkipWS();
					if (Try() || If() || While() || /* Class() || */ For() || Switch()) {
						if (Eol()) return true;
						else return failure();
					}
					else if (Return() || Break() || Continue() || Equation()) {
						if (Eol()) return true;
						else return failure();
					}
					else if (Block()) {
						return true;
					}				
					return failure();
				}

				/// Top level parser, starts parsing of all known parses
				bool Statements() {
					SkipWS();

					bool retval = false;
					bool has_more = true;
					bool saw_eol = true;

					auto start = m_position;
					while (has_more && m_position.has_more()) {
						start = m_position;

						if ((this->m_preprocessor_if_stack.size() == 0) || (this->m_preprocessor_if_stack.back() == preprocessor_state::TRUE_UNTIL_ELSE)) {
							if (PreprocessorDirectives(true) || DeclNamespace() || DeclClass() || DeclFunction() || /*Def() || */ Try() || If() || While() || /* Class() || */ For() || Switch()) {
								if (!saw_eol) {
									throw except::eval_error("Two statements missing line separator", Parse_Location(start, start));
								}
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else if (Return() || Break() || Continue() || Equation()) {
								if (!saw_eol) {
									throw except::eval_error("Two expressions missing line separator", Parse_Location(start, start));
								}
								has_more = true;
								retval = true;
								saw_eol = false;
							}
							else if (Block() || Eol()) {
								has_more = true;
								retval = true;
								saw_eol = true;
							}
							else {
								has_more = false;
							}
						}
						else {
							SkipWS();
							if (Quoted_String_()) {
								continue;
							}
							if (!PreprocessorDirectives(false)) {
								++m_position;
							}
						}
					}
					return retval;
				};

				/// Reads a curly-brace C-style block from input
				bool Block() {
					bool retval = false;

					const auto prev_stack_top = m_match_stack.size();

					if (Char('{')) {
						retval = true;

						Statements();

						if (!Char('}')) {
							throw except::eval_error("Incomplete block", Parse_Location(m_position, m_position));
						}

						if (m_match_stack.size() == prev_stack_top) {
							m_match_stack.push_back(Noop_Node("", {}, {}));
						}

						build_match<Block_Node>(prev_stack_top);
					}

					return retval;
				};

				/// Reads a curly-brace C-style block from input which only allows for declarations
				bool DeclarationsBlock() {
					bool retval = false;

					const auto prev_stack_top = m_match_stack.size();

					if (Char('{')) {
						retval = true;

						Declarations();

						if (!Char('}')) {
							throw except::eval_error("Incomplete declaration block", Parse_Location(m_position, m_position));
						}

						if (m_match_stack.size() == prev_stack_top) {
							m_match_stack.push_back(Noop_Node("", {}, {}));
						}

						build_match<Declaration_Block_Node>(prev_stack_top);
					}

					return retval;
				};

				/// Reads a curly-brace C-style block from input -- note that this scope is special and cannot find objects from parent scopes. 
				bool FunctionBlock() {
					bool retval = false;

					const auto prev_stack_top = m_match_stack.size();

					if (Char('{')) {
						retval = true;

						Statements();

						if (!Char('}')) {
							throw except::eval_error("Incomplete function block", Parse_Location(m_position, m_position));
						}

						if (m_match_stack.size() == prev_stack_top) {
							m_match_stack.push_back(Noop_Node("", {}, {}));
						}

						build_match<FunctionBlock_Node>(prev_stack_top);
					}

					return retval;
				};

				/// Reads a return statement from input
				bool Return() {
					const auto prev_stack_top = m_match_stack.size();
					if (Keyword("return")) {
						Operator();
						build_match<Return_Node>(prev_stack_top);
						return true;
					}
					else {
						return false;
					}
				};

				/// Reads an if/else if/else block from input
				bool If() {
					bool retval = false;

					const auto prev_stack_top = m_match_stack.size();
					// SkipWS(true);
					if (Keyword("if")) {
						retval = true;
						SkipWS(true);
						if (!Char('(')) {
							throw except::eval_error("Incomplete 'if' expression: cannot find '('", Parse_Location(m_position, m_position));
						}
						SkipWS(true);
						if (!Equation()) {
							throw except::eval_error("Incomplete 'if' expression: cannot find equation block", Parse_Location(m_position, m_position));
						}
						SkipWS(true);
						const bool is_if_init = Eol() && Equation();
						SkipWS(true);
						if (!Char(')')) {
							throw except::eval_error("Incomplete 'if' expression: cannot find ')'", Parse_Location(m_position, m_position));
						}

						SkipWS(true);

						if (!Block()) {
							if (!SingleStatement()) {
								throw except::eval_error("Incomplete 'if' block", Parse_Location(m_position, m_position));
							}
						}

						bool has_matches = true;
						while (has_matches) {
							SkipWS(true);
							has_matches = false;
							if (Keyword("else")) {
								SkipWS(true);
								if (If()) {
									has_matches = true;
								}
								else {
									SkipWS(true);
									if (!Block()) {
										if (!SingleStatement()) {
											throw except::eval_error("Incomplete 'else' block", Parse_Location(m_position, m_position));
										}
									}
									has_matches = true;
								}
							}
						}

						const auto num_children = m_match_stack.size() - prev_stack_top;

						if ((is_if_init && num_children == 3) || (!is_if_init && num_children == 2)) {
							m_match_stack.push_back(Noop_Node("", {}, {}));
						}

						if (!is_if_init) {
							build_match<If_Node>(prev_stack_top);
						}
						else {
							build_match<If_Node>(prev_stack_top + 1);
							build_match<Block_Node>(prev_stack_top);
						}
					}

					return retval;
				};

				/// Reads and processes preprocessor commands, such as #include, #define, #if, etc. 
				bool PreprocessorText(GL::string& out) {
					// SkipWS();
					auto start = m_position;
					out = GL::string::empty_string();
					while (m_position.has_more()) {
						SkipWS();
						if (Quoted_String_()) {
							auto s = GL::Engine::Position::str(start, m_position);
							start = m_position;
							out = out + GL::string(s);
							continue;
						}
						if (Keyword_("\\\n") || Keyword_("\\\r")) {	
							auto s = GL::Engine::Position::str(start, m_position - 2);
							start = m_position;
							out = out + GL::string(s) + "\n";
							continue;
						}
						if (Char_('\n')) {
							auto s = GL::Engine::Position::str(start, m_position - 1);
							start = m_position;
							out = out + GL::string(s);

							out = out.
								remove_leading_and_trailing(' ').
								remove_leading_and_trailing('\t').
								remove_leading_and_trailing('\n').
								remove_leading_and_trailing('\r');

							return true;
						}
						++m_position;					
					}
					return false;
				};
			
            public:
				struct PreprocessorDefineInformation {
					GL::string VarName;
					std::vector<GL::string> Inputs;
					GL::string Remainder;
					bool is_function = false;	
					bool use_preprocessed_Remainder = false;
					AbstractSyntaxTreeNode Remainder_Preprocessed;
				};
				struct PreprocessorUndefineInformation {
					GL::string VarName;
				};

				bool ProcessMacroNode(AbstractSyntaxTreeNode& current_preprocessor_node, bool at_end_of_match_stack = true) {
					bool invert_decision = false;
					// now that we know we "found" a preprocessor token, how should we handle it?
					switch (hash(current_preprocessor_node.text.c_str())) {
					case hash("#warning"): if (current_preprocessor_node.children.size() >= 1
							&& current_preprocessor_node.children[0].identifier == GL::Engine::AST_Node_Type::Constant
						) {
							// ALSO push this warning onto the preprocessor stack, for now.
							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
						}
						break;
					case hash("#error"): if (current_preprocessor_node.children.size() >= 1
							&& current_preprocessor_node.children[0].identifier == GL::Engine::AST_Node_Type::Constant
						) {
							// ALSO push this warning onto the preprocessor stack, for now.
							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
						}
						break;
					case hash("#pragma"):
						if (current_preprocessor_node.children.size() >= 1
							&& current_preprocessor_node.children[0].identifier == GL::Engine::AST_Node_Type::Constant
						) {
							// #pragma redefine_unit(1_gal / 1_min) "gallon_per_minute" "gpm"
							//GL::string node_text = this->analysis_engine.cast<GL::string>(current_preprocessor_node.children[0].constant);
							//
							//GL::string VarName;
							//GL::string remainder;
							//GL::string inputs;
							//auto local_pos = GL::Engine::Position(0, node_text);
							//if (Symbol_FreeStanding(GL::string("redefine_unit"), local_pos)) {
							//	SkipWS_FreeStanding(false, local_pos);
							//	if (Symbol_FreeStanding(GL::string("("), local_pos)) {
							//		auto Inputs_Start = local_pos;
							//		auto Inputs_End = local_pos;
							//		while (!Char_FreeStanding(')', local_pos)) {
							//			++local_pos;
							//			Inputs_End = local_pos;
							//		}
							//		inputs = local_pos.str(Inputs_Start, Inputs_End);
							//		SkipWS_FreeStanding(false, local_pos);
							//	}
							//}


							//if (Id_FreeStanding(&VarName, local_pos)) {
							//	SkipWS_FreeStanding(false, local_pos);
							//	if (Symbol_FreeStanding(GL::string("("), local_pos)) {

							//	}
							//}




							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
						}
						break;
					case hash("#include"):  // to-do, update the "downloaded_script" variable to actually download or find the associated script.
						if (current_preprocessor_node.children.size() >= 1
							&& current_preprocessor_node.children[0].identifier == GL::Engine::AST_Node_Type::Constant
						) {
							GL::string node_text = this->analysis_engine.cast<GL::string>(current_preprocessor_node.children[0].constant);
							GL::string downloaded_script = node_text; // "void Foo(int x){ return x; };";

							for (size_t pos = current_preprocessor_node.location.start.pos; pos < current_preprocessor_node.location.end.pos; ++pos) {
								m_position[pos] = ' ';
							}

							m_position.insert_at(current_preprocessor_node.location.start.pos, downloaded_script);

							m_position = current_preprocessor_node.location.start;

							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
						}
						break;
					case hash("#ifndef"):
						invert_decision = true;
						[[fallthrough]];
					case hash("#ifdef"):
						if (current_preprocessor_node.children.size() >= 1
							&& current_preprocessor_node.children[0].identifier == GL::Engine::AST_Node_Type::Constant
						) {
							auto current_position = m_position;
							GL::string node_text = this->analysis_engine.cast<GL::string>(current_preprocessor_node.children[0].constant);
							GL::string VarName;
							auto local_pos = GL::Engine::Position(0, node_text);

							// #ifdef VarName
							if (Id_FreeStanding(&VarName, local_pos)) {			
								bool success = false;
								for (int preprocessorPos = (int)m_preprocessor_stack.size() - 1; preprocessorPos >= 0; --preprocessorPos) {
									auto* iter = &m_preprocessor_stack[preprocessorPos];
									if (current_preprocessor_node.location.start.pos >= iter->location.end.pos) {
										if ((iter->text == "#define") || (iter->text == "#undef")) {
											if (iter->text == "#define") {
												if (iter->tag.cast<Engine::ScriptParser::Parser::PreprocessorDefineInformation>().VarName == VarName) {
													m_preprocessor_if_stack.push_back(invert_decision ? preprocessor_state::FALSE_UNTIL_ELSE : preprocessor_state::TRUE_UNTIL_ELSE);
													success = true;
													break;
												}
											}
											else {
												if (iter->tag.cast<Engine::ScriptParser::Parser::PreprocessorUndefineInformation>().VarName == VarName) {
													m_preprocessor_if_stack.push_back(invert_decision ? preprocessor_state::TRUE_UNTIL_ELSE : preprocessor_state::FALSE_UNTIL_ELSE);
													success = true;
													break;
												}
											}
										}
									}
								}		
								if (!success) {
									m_preprocessor_if_stack.push_back(invert_decision ? preprocessor_state::TRUE_UNTIL_ELSE : preprocessor_state::FALSE_UNTIL_ELSE);
								}
							}
							else {
								throw except::eval_error("#ifdef preprocessor must include an ID of some type.", Parse_Location(current_position, current_position));
							}							
							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
						}
						break;
					case hash("#if"): if (current_preprocessor_node.children.size() >= 1
						&& current_preprocessor_node.children[0].identifier == GL::Engine::AST_Node_Type::Constant) {
							GL::string node_text = this->analysis_engine.cast<GL::string>(current_preprocessor_node.children[0].constant);
							auto this_text = GL::string(std::string((size_t)current_preprocessor_node.location.start.pos, ' ')) + node_text;
							AbstractSyntaxTreeNode this_node = this->parse_instr_eval(this_text);
							AbstractSyntaxTreeNode temp_combo_node = Block_Node("", {}, {});
							temp_combo_node.children = this->m_match_stack;
							temp_combo_node.children.push_back(this_node);
							temp_combo_node.location.start = temp_combo_node.children.front().location.start;
							temp_combo_node.location.end = temp_combo_node.children.back().location.end;

							temp_combo_node = optimizer::optimize_all(temp_combo_node, this);
							if (temp_combo_node.children.size() > 0) {
								this_node = temp_combo_node.children.back();
								if (this_node.constant) {
									try {
										m_preprocessor_if_stack.push_back(this->analysis_engine.cast<bool>(this_node.constant) ? preprocessor_state::TRUE_UNTIL_ELSE : preprocessor_state::FALSE_UNTIL_ELSE);
									}
									catch (std::exception& e) {
										throw except::eval_error(GL::string("#if preprocessor did not compile to a constexpr boolean value: ") + std::string(e.what()), Parse_Location(m_position, m_position));
									}
								}
								else {
									throw except::eval_error("#if preprocessor did not compile to a constexpr value: " + this_node.to_string("", this->analysis_engine), Parse_Location(m_position, m_position));
								}
							}
							else {
								throw except::eval_error("#if preprocessor compiled down to nothing -- no value was returned.", Parse_Location(m_position, m_position));
							}

							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
						}
						break;
					case hash("#elif"): if (current_preprocessor_node.children.size() >= 1
						&& current_preprocessor_node.children[0].identifier == GL::Engine::AST_Node_Type::Constant 
						&& m_preprocessor_if_stack.size() > 0) {
						    if (m_preprocessor_if_stack.back() == preprocessor_state::TRUE_UNTIL_ELSE) { 
								// already true -- the else statement has failed.
								m_preprocessor_if_stack.pop_back();
								m_preprocessor_if_stack.push_back(preprocessor_state::FALSE_UNTIL_ENDIF);
						    }
							else if (m_preprocessor_if_stack.back() == preprocessor_state::FALSE_UNTIL_ENDIF) {
								// was true at some point in the past and is now processing the `else` statements. remains false.								
							}
							else if (m_preprocessor_if_stack.back() == preprocessor_state::FALSE_UNTIL_ELSE) {
								// was never true (yet)
								GL::string node_text = this->analysis_engine.cast<GL::string>(current_preprocessor_node.children[0].constant);
								auto this_text = GL::string(std::string((size_t)current_preprocessor_node.location.start.pos, ' ')) + node_text;
								AbstractSyntaxTreeNode this_node = this->parse_instr_eval(this_text);
								AbstractSyntaxTreeNode temp_combo_node = Block_Node("", {}, {});
								temp_combo_node.children = this->m_match_stack;
								temp_combo_node.children.push_back(this_node);
								temp_combo_node.location.start = temp_combo_node.children.front().location.start;
								temp_combo_node.location.end = temp_combo_node.children.back().location.end;

								temp_combo_node = optimizer::optimize_all(temp_combo_node, this);
								if (temp_combo_node.children.size() > 0) {
									this_node = temp_combo_node.children.back();
									if (this_node.constant) {
										try {
											m_preprocessor_if_stack.pop_back();
											m_preprocessor_if_stack.push_back(this->analysis_engine.cast<bool>(this_node.constant) ? preprocessor_state::TRUE_UNTIL_ELSE : preprocessor_state::FALSE_UNTIL_ELSE);
										}
										catch (std::exception& e) {
											throw except::eval_error(GL::string("#if preprocessor did not compile to a constexpr boolean value: ") + std::string(e.what()), Parse_Location(m_position, m_position));
										}
									}
									else {
										throw except::eval_error("#if preprocessor did not compile to a constexpr value: " + this_node.to_string("", this->analysis_engine), Parse_Location(m_position, m_position));
									}
								}
								else {
									throw except::eval_error("#if preprocessor compiled down to nothing -- no value was returned.", Parse_Location(m_position, m_position));
								}
							}

							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
					    }
						else {
						    throw except::eval_error("#elif preprocessor must follow either an #if, #ifdef, #ifndef, or #elif statement.", Parse_Location(m_position, m_position));
					    }
						break;
					case hash("#else"): if (m_preprocessor_if_stack.size() > 0) {		
							if (m_preprocessor_if_stack.back() == preprocessor_state::TRUE_UNTIL_ELSE) {
								// already true -- the else statement has failed.
								m_preprocessor_if_stack.pop_back();
								m_preprocessor_if_stack.push_back(preprocessor_state::FALSE_UNTIL_ENDIF);
							}
							else if (m_preprocessor_if_stack.back() == preprocessor_state::FALSE_UNTIL_ENDIF) {
								// was true at some point in the past and is now processing the `else` statements. remains false.								
							}
							else {
								m_preprocessor_if_stack.back() = preprocessor_state::TRUE_UNTIL_ELSE;
							}
							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
					    }
						else {
						    throw except::eval_error("#else preprocessor must follow either an #if, #ifdef, #ifndef, or #elif statement.", Parse_Location(m_position, m_position));
					    }
						break;
					case hash("#endif"): if (m_preprocessor_if_stack.size() > 0) {
							m_preprocessor_if_stack.pop_back();
							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
						}
						else {
						    throw except::eval_error("#endif preprocessor must follow either an #if, #ifdef, #ifndef, #elif, or #else statement.", Parse_Location(m_position, m_position));
					    }
						break;
					case hash("#define"):
						if (current_preprocessor_node.children.size() >= 1
							&& current_preprocessor_node.children[0].identifier == GL::Engine::AST_Node_Type::Constant
							) {
							auto current_position = m_position;
							GL::string node_text = this->analysis_engine.cast<GL::string>(current_preprocessor_node.children[0].constant);

							// #define VarName
							// #define VarName = ...
							// #define VarName ...
							// #define VarName(...) ...

							GL::string VarName;
							GL::string remainder;
							GL::string inputs;
							bool is_function = false;

							auto local_pos = GL::Engine::Position(0, node_text);
							if (Id_FreeStanding(&VarName, local_pos)) {
								SkipWS_FreeStanding(false, local_pos);
								if (Symbol_FreeStanding(GL::string("="), local_pos)) {
									SkipWS_FreeStanding(false, local_pos);
									remainder = node_text.right(node_text.length() - local_pos.pos);
								}
								else if (Char_FreeStanding('(', local_pos)) {
									SkipWS_FreeStanding(false, local_pos);
									is_function = true;
									auto Inputs_Start = local_pos;
									auto Inputs_End = local_pos;
									while (!Char_FreeStanding(')', local_pos)) {
										++local_pos;
										Inputs_End = local_pos;
									}
									inputs = local_pos.str(Inputs_Start, Inputs_End);
									SkipWS_FreeStanding(false, local_pos);
									remainder = node_text.right(node_text.length() - local_pos.pos);
								}
								else {
									SkipWS_FreeStanding(false, local_pos);
									remainder = node_text.right(node_text.length() - local_pos.pos);
								}
							}
							else {
								throw except::eval_error("#define preprocessor must include an ID of some type.", Parse_Location(current_position, current_position));
							}

							PreprocessorDefineInformation info;
							info.Inputs = inputs.remove_leading_and_trailing(' ').split(",");
							for (auto& x : info.Inputs) x = x.remove_leading_and_trailing(' ');
							info.is_function = is_function;
							info.Remainder = remainder;
							info.VarName = VarName;
							current_preprocessor_node.tag = GL::any::fast_any::instance(std::move(info));

							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
						}
						break;
					case hash("#undef"):
						if (current_preprocessor_node.children.size() >= 1
							&& current_preprocessor_node.children[0].identifier == GL::Engine::AST_Node_Type::Constant
							) {
							auto current_position = m_position;
							GL::string node_text = this->analysis_engine.cast<GL::string>(current_preprocessor_node.children[0].constant);
							GL::string VarName;
							auto local_pos = GL::Engine::Position(0, node_text);
							if (!Id_FreeStanding(&VarName, local_pos)) {
								throw except::eval_error("#undef preprocessor must include an ID of some type.", Parse_Location(current_position, current_position));
							}
							SkipWS_FreeStanding(false, local_pos);
							if (local_pos.has_more()) {
								throw except::eval_error("#undef preprocessor included unparsed inputs after the ID", Parse_Location(current_position, current_position));
							}

							PreprocessorUndefineInformation info;
							info.VarName = VarName;
							current_preprocessor_node.tag = GL::any::fast_any::instance(std::move(info));

							m_preprocessor_stack.push_back(current_preprocessor_node);
							if (at_end_of_match_stack) m_match_stack.pop_back();
							else current_preprocessor_node = Noop_Node("", {}, {});
						}
						break;
					default:
						throw except::eval_error("Unknown preprocessor directive: " + current_preprocessor_node.text, Parse_Location(m_position, m_position));
					}
					return true;
				}

			private:
				// Attempt to read a pre-processor directive from input
				bool PreprocessorDirectives(bool allow_any_preprocessor) {
					static auto options1 = std::vector<GL::utility::Static_String>({
						"#warning",
						"#error",
						"#pragma",
						"#include",
						"#ifndef", "#ifdef", "#if", "#elif", "#else", "#endif",
						"#define", "#undef"
					});
					static auto options2 = std::vector<GL::utility::Static_String>({
						"#elif", "#else", "#endif"
					});
					std::vector<GL::utility::Static_String>* options;
					if (allow_any_preprocessor) {
						options = &options1;
					}
					else {
						options = &options2;
					}
					const auto prev_prev_pos = m_position;
					for (auto& preprocessor_call : *options) {
						if ([&]() -> bool {
							const auto prev_preprocessorif_top = m_preprocessor_if_stack.size();
								const auto prev_preprocessor_top = m_preprocessor_stack.size();
								const auto prev_comment_top = m_comment_stack.size();
								const auto prev_stack_top = m_match_stack.size();
								const auto prev_pos = m_position;
								auto failure = [&]() {
								while (m_match_stack.size() != prev_stack_top) m_match_stack.pop_back();
									while (m_comment_stack.size() != prev_comment_top) m_comment_stack.pop_back();
									while (m_preprocessor_stack.size() != prev_preprocessor_top) m_preprocessor_stack.pop_back();
									while (m_preprocessor_if_stack.size() != prev_preprocessorif_top) m_preprocessor_if_stack.pop_back();
									m_position = prev_pos;
								return false;
							};

							if (Keyword(preprocessor_call)) {
								const auto content_start = m_position;
								GL::string s;
								if (PreprocessorText(s)) {
									m_match_stack.push_back(make_const((GL::string)s.to_string(), prev_pos, (GL::string)s.to_string()));
									build_match<PreprocessorMacro_Node>(prev_stack_top);
									m_match_stack.back().text = std::string_view(preprocessor_call.data);
									return true;
								}
							}
							return failure();
						}()) {
							return ProcessMacroNode(m_match_stack.back());
						}
					}
					return false;
				};

			public:
				GL::scope::impl::RootScope& analysis_engine;

			public:
				Parser(GL::scope::impl::RootScope& root) : analysis_engine(root) {};
				~Parser() = default;

				// highest-level parse request, which starts a new scope from scratch and completes it. 
				AbstractSyntaxTreeNode Parse(GL::string t_input, int optimization_depth = 1000, std::vector<AbstractSyntaxTreeNode> const& preloaded_matches = {}) {
					return parse(t_input, optimization_depth, preloaded_matches);
				};

			private:
				AbstractSyntaxTreeNode parse(GL::string& t_input, int optimization_depth = 1000, std::vector<AbstractSyntaxTreeNode> const& preloaded_matches = {}) {
					return parse_instr_eval(t_input, optimization_depth, preloaded_matches);
				};
				AbstractSyntaxTreeNode parse_internal(GL::string& t_input, int optimization_depth = 1000) {
					m_position = Engine::Position(0, t_input);

					// top level stack
					try {
						if (Statements()) {
							if (m_position.has_more()) {
								throw except::eval_error("Unparsed input", Parse_Location(m_position, m_position));
							}
							else {
								for (auto& x : m_match_stack) {
									x.for_each_child([&](AbstractSyntaxTreeNode& this_child) -> bool {
										if (this_child.identifier == Engine::AST_Node_Type::Comment) {
											m_comment_stack.push_back(this_child);
											this_child = Noop_Node("", {}, {});
										}
										return false;
									});
									if (x.identifier == Engine::AST_Node_Type::Comment) {
										m_comment_stack.push_back(x);
										x = Noop_Node("", {}, {});
									}
								}

								// add the comment nodes to the front of the stack, to not interupt the automatic return behavior
								if (1) {
									auto i = m_comment_stack.rbegin();
									while (i != m_comment_stack.rend()) {
										m_match_stack.insert(m_match_stack.begin(), std::move(*i));
										i = decltype(i)(m_comment_stack.erase(std::next(i).base()));
									}
								}

								build_match<File_Node>(0);
								m_match_stack[0] = optimizer::incorporate_warnings_and_errors(m_match_stack[0], this, optimization_depth);

								// add the error or warning nodes to the front of the stack, to not interupt the automatic return behavior.
								if (1) {
									bool added_warnings = false;
									for (auto iter = m_preprocessor_stack.rbegin(); iter != m_preprocessor_stack.rend(); ++iter) {
										if (iter->text == "#error" 
										 || iter->text == "#warning"
										) {
											m_match_stack.insert(m_match_stack.begin(), *iter);
											added_warnings = true;
										}
									}
									if (added_warnings) {
										build_match<File_Node>(0);
										m_match_stack[0] = optimizer::optimize_all(m_match_stack[0], this, optimization_depth);
									}
								}
							}
						}
						else {
							m_match_stack.push_back(Noop_Node("", Parse_Location(m_position, m_position), {}));
						}
						if (m_preprocessor_if_stack.size() > 0) {
							throw except::eval_error("A preprocessor macro (e.g. #if, #ifdef, or #ifndef) failed to have a closing condition (e.g. #endif)", Parse_Location(m_position, m_position));
						}
					}
					catch (except::eval_error& e) {
						m_match_stack.push_back(PreprocessorMacro_Node("#error", e.position, {
							Constant_Node(e.reason, e.position, {}, e.reason)
						}));
						build_match<File_Node>(0);
					}
					catch (std::exception& e) {
						GL::string err_txt = "Error performing AST parsing: \"" + std::string(e.what()) + "\"";
						m_match_stack.push_back(PreprocessorMacro_Node("#error", {}, {
							Constant_Node(err_txt, {}, {}, err_txt)
						}));
						build_match<File_Node>(0);
					}
					catch (...) {
						GL::string err_txt = "Error performing AST parsing: Error type is unknown.";
						m_match_stack.push_back(PreprocessorMacro_Node("#error", {}, {
							Constant_Node(err_txt, {}, {}, err_txt)
						}));
						build_match<File_Node>(0);
					}

					return m_match_stack.front();
				};
				AbstractSyntaxTreeNode parse_instr_eval(GL::string& t_input, int optimization_depth = 1000, std::vector<AbstractSyntaxTreeNode> const& preloaded_matches = {}) {
					auto last_position = m_position;
					auto last_match_stack = std::exchange(m_match_stack, decltype(m_match_stack){});
					auto last_comment_stack = std::exchange(m_comment_stack, decltype(m_comment_stack){});
					auto last_preprocessor_stack = std::exchange(m_preprocessor_stack, decltype(m_preprocessor_stack){});
					auto last_preprocessor_if_stack = std::exchange(m_preprocessor_if_stack, decltype(m_preprocessor_if_stack){});

					m_match_stack = preloaded_matches;
					auto retval = parse_internal(t_input, optimization_depth);

					m_position = std::move(last_position);
					m_match_stack = std::move(last_match_stack);
					m_comment_stack = std::move(last_comment_stack);
					m_preprocessor_stack = std::move(last_preprocessor_stack);
					m_preprocessor_if_stack = std::move(last_preprocessor_if_stack);

					return retval;
				};

			public:		
				GL::Proxy_Function make_callable_from_node(AbstractSyntaxTreeNode& node, GL::scope::impl::BasicScope& current_scope) {
#if 0
					if (node.identifier == GL::Engine::AST_Node_Type::FunctionDecl
						&& node.children.size() == 4
						&& node.children[0].identifier == GL::Engine::AST_Node_Type::Id // Return TypeName
						&& node.children[1].identifier == GL::Engine::AST_Node_Type::Id // FunctionName
						&& node.children[2].identifier == GL::Engine::AST_Node_Type::Arg_List // Arguments
					) {
						auto& function_name = node.children[1].text;
						auto num_arguments = node.children[2].children.size();

						auto eval_node_function = [this, this_node = node.children[3]](std::vector<std::pair<GL::string, GL::any::fast_any>> const& inputs) -> GL::any::fast_any {
							auto Node = this_node;
							eval_state state;
							auto this_scope = GL::scope::GetCurrentCaller()->make_scope();

							GL::any::fast_any result = evaluate(Node, state, this_scope);
							if (state.throwing != throwing::Nothing) {
								if (state.throwing == throwing::Return) {
									return state.to_return;
								}
								throw except::eval_error("Error inside of script function", Node.location);
							}
							return result;
						};





						if (node.children[0].text == "void") {

							return GL::make_callable(function_name, []() -> void {

							}, 0, {}, {}, GL::type_of<void>());
						}
						else {
							auto return_type = current_scope.DetermineType(node.children[0].text);





						}
					}




					GL::make_callable(node.children[1].text, [this, return_type, this_node = node.children[3]]()->GL::any::fast_any {
						auto Node = this_node;
						eval_state state;
						GL::any::fast_any result = evaluate(Node, state, *GL::scope::GetCurrentCaller());
						if (state.throwing != throwing::Nothing) {
							if (state.throwing == throwing::Return) {
								return GL::scope::GetCurrentCaller()->cast(state.to_return, return_type);
							}
							throw except::eval_error("Error inside of script function", Node.location);
						}
						return GL::scope::GetCurrentCaller()->cast(result, return_type);
					}, 0, {}, {}, return_type);






#endif
				};
				// pre-evaluate the classes & namespaces. This will happen only one time. 
				bool preEval_declarations(AbstractSyntaxTreeNode& node, GL::scope::impl::BasicScope& current_scope) {
					if (node.identifier == GL::Engine::AST_Node_Type::Namespace) {
						// node.tag.cast< NamespaceClassInformation>().
						auto& this_namespace = current_scope.GetNamespace()->make_namespace(node.children[0].text);
						for (auto& child_node : node.children) {
							switch (child_node.identifier) {
							case GL::Engine::AST_Node_Type::Assign_Retroactively: {

								// constexpr auto x = 10;
								if (child_node.constant) { // Constexpr value.
									current_scope.emplace_object_here(child_node.children[1].children[0].text, child_node.constant | GL::type::Const | GL::type::Reference);
									break;
								}

								// auto x = 10;
								if (child_node.children.size() == 2
									&& child_node.children[0].identifier == Engine::AST_Node_Type::Constant
									&& child_node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& child_node.children[1].children.size() == 1
									&& child_node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
								) {
									// Constant value provided. Need to copy it. 
									auto& to_copy = child_node.children[0].constant;
									if (auto* BC = current_scope.GetRoot()->try_find_class(to_copy.m_casted_type)) {
										current_scope.emplace_object_here(child_node.children[1].children[0].text, BC->this_m.scope->call(BC->this_m.scope_name, { to_copy | GL::type::Const | GL::type::Reference }));
									}
									else {
										throw except::eval_error("Unable to copy variable to new object", child_node.location);
									}
									break;
								}

								// auto x = int();
								if (child_node.children.size() == 2
									&& child_node.children[0].identifier != Engine::AST_Node_Type::Constant
									&& child_node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& child_node.children[1].children.size() == 1
									&& child_node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
								) {
									eval_state state;
									auto to_add = evaluate(child_node.children[0], state, current_scope);
									if (state.throwing != throwing::Nothing) {
										break;
									}
									current_scope.emplace_object_here(child_node.children[1].children[0].text, to_add);
									break;
								}

								// int x = 10.0f;
								if (child_node.children.size() == 3
									&& child_node.children[0].identifier == Engine::AST_Node_Type::Constant
									&& child_node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& child_node.children[1].children.size() == 1
									&& child_node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
									) {
									// Constant value provided. Need to copy it. 
									auto& to_copy = child_node.children[0].constant;
									eval_state state;
									auto to_assign = evaluate(child_node.children[2], state, current_scope);
									if (state.throwing != throwing::Nothing) {
										break;
									}
									if (auto* BC = current_scope.GetRoot()->try_find_class(to_copy.m_casted_type)) {
										auto copied = BC->this_m.scope->call(BC->this_m.scope_name, { to_copy | GL::type::Const | GL::type::Reference });
										BC->this_m.scope->call("=", { copied, to_assign });
										current_scope.emplace_object_here(child_node.children[1].children[0].text, copied);
									}
									else {
										throw except::eval_error("Unable to copy variable to new object", child_node.location);
									}
									break;
								}

								// SOMETHING x = 10.0f;
								if (child_node.children.size() == 3
									&& child_node.children[0].identifier != Engine::AST_Node_Type::Constant
									&& child_node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& child_node.children[1].children.size() == 1
									&& child_node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
								) {
									eval_state state;
									auto copied = evaluate(child_node.children[0], state, current_scope);
									if (state.throwing != throwing::Nothing) {
										break;
									}
									auto to_assign = evaluate(child_node.children[2], state, current_scope);
									if (state.throwing != throwing::Nothing) {
										break;
									}
									if (auto* BC = current_scope.GetRoot()->try_find_class(copied.m_casted_type)) {
										BC->this_m.scope->call("=", { copied, to_assign });
										current_scope.emplace_object_here(child_node.children[1].children[0].text, copied);
									}
									else {
										current_scope.call("=", { copied, to_assign });
										current_scope.emplace_object_here(child_node.children[1].children[0].text, copied);
									}
								}

								break;
							}
							default: break;
							}
						}
						node.for_each_child([this, &this_namespace](AbstractSyntaxTreeNode& this_node) -> bool {
							if (preEval_declarations(this_node, this_namespace)) {
								// returned true, meaning continue deeper into the child during THIS search
								return true;
							}
							else {
								// returned false, meaning DO NOT continue deeper into the child during THIS search
								return false;
							}
						}, [](AbstractSyntaxTreeNode&) {}, [](AbstractSyntaxTreeNode&) {});
						return false;
					}
					else if (node.identifier == GL::Engine::AST_Node_Type::Class) {						
						auto& this_class = current_scope.GetNamespace()->make_class(node.children[0].text);
						for (int i = 0; i < node.tag.cast< NamespaceClassInformation>().template_types.size(); ++i) {
							this_class.template_types.push_back({ node.tag.cast< NamespaceClassInformation>().template_types[i], GL::is_template::type(i,node.tag.cast< NamespaceClassInformation>().template_types[i]) });
						}
						node.for_each_child([this, &this_class](AbstractSyntaxTreeNode& this_node) -> bool {
							if (preEval_declarations(this_node, this_class)) {
								// returned true, meaning continue deeper into the child during THIS search
								return true;
							}
							else {
								// returned false, meaning DO NOT continue deeper into the child during THIS search
								return false;
							}
						}, [](AbstractSyntaxTreeNode&) {}, [](AbstractSyntaxTreeNode&) {});
						return false;
					}
					else if (node.identifier == GL::Engine::AST_Node_Type::FunctionDecl) {
						if (node.children.size() == 4
							&& node.children[0].identifier == GL::Engine::AST_Node_Type::Id // Return TypeName
							&& node.children[1].identifier == GL::Engine::AST_Node_Type::Id // FunctionName
							&& node.children[2].identifier == GL::Engine::AST_Node_Type::Arg_List // Arguments
						) {
							if (node.children[0].text == "void") {
								current_scope.GetNamespace()->add_function(GL::make_callable(node.children[1].text, []() -> void {
									
								}, 0, {}, {}, GL::type_of<void>()));
							}
							else {
								auto return_type = current_scope.DetermineType(node.children[0].text);
								switch (node.children[2].children.size()) {
								case 0: {
									current_scope.GetNamespace()->add_function(GL::make_callable(node.children[1].text, [this, return_type, this_node = node.children[3]]() -> GL::any::fast_any {
										auto Node = this_node;
										eval_state state; 
										GL::any::fast_any result = evaluate(Node, state, *GL::scope::GetCurrentCaller());
										if (state.throwing != throwing::Nothing) {
											if (state.throwing == throwing::Return) {
												return GL::scope::GetCurrentCaller()->cast(state.to_return, return_type);
											}											
											throw except::eval_error("Error inside of script function", Node.location);											
										}
										return GL::scope::GetCurrentCaller()->cast(result, return_type);
									}, 0, {}, {}, return_type));
									break;
								}
								case 1: {
									current_scope.GetNamespace()->add_function(GL::make_callable(node.children[1].text, [return_type, node](GL::any::fast_any input_1) -> GL::any::fast_any {
										GL::any::fast_any result;

										return GL::scope::GetCurrentCaller()->cast(result, return_type);
									}, 0, {}, {}, return_type));
									break;
								}
								default: break;
								}






							}
						}
						return false;
					}
					else {
						node.for_each_child([this,&current_scope](AbstractSyntaxTreeNode& this_node) -> bool {
							if (preEval_declarations(this_node, current_scope)) {
								// returned true, meaning continue deeper into the child during THIS search
								return true;
							}
							else {
								// returned false, meaning DO NOT continue deeper into the child during THIS search
								return false;
							}
						}, [](AbstractSyntaxTreeNode&) {}, [](AbstractSyntaxTreeNode&) {});
						return true;
					}
				};
				// populate the return type information (as much as possible) for every node. This will happen on repeat until all nodes return false.
				bool preEval_returnTypes(AbstractSyntaxTreeNode& node, GL::scope::impl::BasicScope& current_scope) {
					return false;
				};
				// select the functions that should be used, and their return types (as much as possible) for every node. This will happen on repeat until all nodes return false.
				bool preEval_functionSelections(AbstractSyntaxTreeNode& node, GL::scope::impl::BasicScope& current_scope) {
					return false;
				};

				GL::any::fast_any evaluate(AbstractSyntaxTreeNode& node, eval_state& state, GL::scope::impl::BasicScope& current_scope) {					
					switch (node.identifier) {
					case Engine::AST_Node_Type::PreprocessorMacro: {
						if (node.text == "#error"
							&& node.children.size() >= 1
							&& node.children[0].identifier == Engine::AST_Node_Type::Constant
						) {
							// something went wrong with the analysis -- throw an error. 
							throw except::eval_error(current_scope.cast<GL::string>(node.children[0].constant), node.location);
						}
						return nullptr;
					}
					case Engine::AST_Node_Type::Comment: [[fallthrough]];
					case Engine::AST_Node_Type::Noop: {
						return nullptr;
					}
					case Engine::AST_Node_Type::Block: [[fallthrough]];
					case Engine::AST_Node_Type::File: {
						auto new_scope = current_scope.make_scope();
						for (int i = 0; i < (node.children.size() - 1); ++i) {
							evaluate(node.children[i], state, new_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
						}
						return state.to_return = evaluate(node.children.back(), state, new_scope);
					}
					case Engine::AST_Node_Type::Scopeless_Block: {
						for (int i = 0; i < (node.children.size() - 1); ++i) {
							evaluate(node.children[i], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
						}
						return state.to_return = evaluate(node.children.back(), state, current_scope);
					}
					case Engine::AST_Node_Type::Constant: {
						return state.to_return = node.constant | GL::type::Const | GL::type::Reference;
					}
					case Engine::AST_Node_Type::Id: { // x
						if (node.constant) {
							return state.to_return = node.constant | GL::type::Const | GL::type::Reference;
						}
						else {
							return state.to_return = current_scope.find_object(node.text);
						}
					}
					case Engine::AST_Node_Type::Var_Decl: {
						if (node.children.size() == 1
							&& node.children[0].identifier == Engine::AST_Node_Type::Id
							) {
							current_scope.emplace_object_here(node.children[1].children[0].text, GL::var());
							return state.to_return = nullptr;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Assign_Retroactively: {
						// constexpr auto x = 10;
						if (node.constant) {
							// Constexpr value.
							current_scope.emplace_object_here(node.children[1].children[0].text, node.constant | GL::type::Const | GL::type::Reference);
							return state.to_return = nullptr;
						}

						// auto x = 10;
						if (node.children.size() == 2
							&& node.children[0].identifier == Engine::AST_Node_Type::Constant
							&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
							&& node.children[1].children.size() == 1
							&& node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
							) {
							// Constant value provided. Need to copy it. 
							auto& to_copy = node.children[0].constant;
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							if (auto* BC = current_scope.GetRoot()->try_find_class(to_copy.m_casted_type)) {
								current_scope.emplace_object_here(node.children[1].children[0].text, BC->this_m.scope->call(BC->this_m.scope_name, { to_copy | GL::type::Const | GL::type::Reference }));
							}
							else {
								throw except::eval_error("Unable to copy variable to new object", node.location);
							}
							return state.to_return = nullptr;
						}

						// auto x = int();
						if (node.children.size() == 2
							&& node.children[0].identifier != Engine::AST_Node_Type::Constant
							&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
							&& node.children[1].children.size() == 1
							&& node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
						) {
							auto to_add = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							current_scope.emplace_object_here(node.children[1].children[0].text, to_add);
							return state.to_return = nullptr;
						}

						// int x = 10.0f;
						if (node.children.size() == 3
							&& node.children[0].identifier == Engine::AST_Node_Type::Constant
							&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
							&& node.children[1].children.size() == 1
							&& node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
						) {
							// Constant value provided. Need to copy it. 
							auto& to_copy = node.children[0].constant;
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							auto to_assign = evaluate(node.children[2], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							if (auto* BC = current_scope.GetRoot()->try_find_class(to_copy.m_casted_type)) {
								auto copied = BC->this_m.scope->call(BC->this_m.scope_name, { to_copy | GL::type::Const | GL::type::Reference });
								BC->this_m.scope->call("=", { copied, to_assign });
								current_scope.emplace_object_here(node.children[1].children[0].text, copied);
							}
							else {
								throw except::eval_error("Unable to copy variable to new object", node.location);
							}
							return state.to_return = nullptr;
						}

						// SOMETHING x = 10.0f;
						if (node.children.size() == 3
							&& node.children[0].identifier != Engine::AST_Node_Type::Constant
							&& node.children[1].identifier == Engine::AST_Node_Type::Var_Decl
							&& node.children[1].children.size() == 1
							&& node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
						) {
							auto copied = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							auto to_assign = evaluate(node.children[2], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							if (auto* BC = current_scope.GetRoot()->try_find_class(copied.m_casted_type)) {
								BC->this_m.scope->call("=", { copied, to_assign });
								current_scope.emplace_object_here(node.children[1].children[0].text, copied);
							}
							else {
								current_scope.call("=", { copied, to_assign });
								current_scope.emplace_object_here(node.children[1].children[0].text, copied);
							}
							return state.to_return = nullptr;
						}

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Return: {
						if (node.children.size() == 0) {
							state.throwing = throwing::Return;
							return state.to_return = nullptr;
						}
						else if (node.children.size() == 1) {
							auto to_return = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							state.throwing = throwing::Return;
							return state.to_return = to_return;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Break: {
						state.throwing = throwing::Break;
						return state.to_return = nullptr;
					}
					case Engine::AST_Node_Type::Continue: {
						state.throwing = throwing::Continue;
						return state.to_return = nullptr;
					}
					case Engine::AST_Node_Type::Binary:
					case Engine::AST_Node_Type::Equation: {
						if (node.children.size() == 2) {
							auto assignee = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							auto assigner = evaluate(node.children[1], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							return state.to_return = current_scope.call(node.text, { assignee, assigner });
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::BinaryFoldRight: {
						if (node.children.size() == 1
							&& node.constant
							) {
							auto assignee = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							return state.to_return = current_scope.call(node.text, { assignee, node.constant | GL::type::Const | GL::type::Reference });
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::BinaryFoldLeft: {
						if (node.children.size() == 1
							&& node.constant
							) {
							auto assignee = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							return state.to_return = current_scope.call(node.text, { node.constant | GL::type::Const | GL::type::Reference, assignee });
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Logical_And: {
						if (node.children.size() == 2) {
							auto assignee = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							auto assigner = evaluate(node.children[1], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							return state.to_return = current_scope.call("&&", { assignee, assigner });
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Logical_Or: {
						if (node.children.size() == 2) {
							auto assignee = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;

							auto assigner = evaluate(node.children[1], state, current_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;
							
							return state.to_return = current_scope.call("||", { assignee, assigner });
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Unused_Return_Fun_Call: [[fallthrough]];
					case Engine::AST_Node_Type::Fun_Call: {
						if (node.children.size() == 2
							&& node.children[0].identifier == Engine::AST_Node_Type::Id
							&& node.children[1].identifier == Engine::AST_Node_Type::Arg_List
						) {
							auto function_name = node.children[0].text;
							std::array<GL::any::fast_any, 16> inputs;
							if (node.children[1].children.size() >= 17) {
								throw except::eval_error("Too many parameters have been provided. Limit the number of parameters to 16 for a function call", node.location);
							}
							for (int i = 0; i < node.children[1].children.size(); ++i) {
								inputs[i] = evaluate(node.children[1].children[i], state, current_scope);
								if (state.throwing != throwing::Nothing) return state.to_return;
							}
							state.to_return = current_scope.call_impl(function_name, &inputs[0], &inputs[0] + node.children[1].children.size());
							return state.to_return;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Dot_Access: {
						if (node.children.size() == 2
							&& node.children[1].identifier == Engine::AST_Node_Type::Fun_Call
							&& node.children[1].children[0].identifier == Engine::AST_Node_Type::Id
							&& node.children[1].children[1].identifier == Engine::AST_Node_Type::Arg_List
						) {
							auto function_name = node.children[1].children[0].text;
							std::array<GL::any::fast_any, 16> inputs;
							if (1) {
								inputs[0] = evaluate(node.children[0], state, current_scope);
								if (state.throwing != throwing::Nothing) return state.to_return;								
							}
							if (node.children[1].children[1].children.size() >= 16) {
								throw except::eval_error("Too many parameters have been provided. Limit the number of parameters to 15 for a dot access", node.location);
							}
							for (int i = 0; i < node.children[1].children[1].children.size(); ++i) {
								inputs[i + 1] = evaluate(node.children[1].children[1].children[i], state, current_scope);
								if (state.throwing != throwing::Nothing) return state.to_return;
							}
							return state.to_return = current_scope.call_impl(function_name, &inputs[0], &inputs[0] + (node.children[1].children[1].children.size() + 1));
						}
						if (node.children.size() == 2
							&& node.children[1].identifier == Engine::AST_Node_Type::Id
						) {
							auto function_name = node.children[1].text;
							GL::any::fast_any inputs;
							if (1) {
								inputs = evaluate(node.children[0], state, current_scope);
								if (state.throwing != throwing::Nothing) {
									return state.to_return;
								}
							}
							return state.to_return = current_scope.call_impl(function_name, &inputs, &inputs + 1);
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Type_Cast: {
						if (node.children.size() == 2
							&& node.children[0].identifier == Engine::AST_Node_Type::Id
							) {
							auto to_cast = evaluate(node.children[1], state, current_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;

							auto type = current_scope.DetermineType(node.children[0].text);
							if (type == GL::type_of<GL::undefined>()) throw except::eval_error("Type-cast was unable to determine the requested type: " + node.children[0].text, node.location);

							if (auto* BC = current_scope.GetRoot()->try_find_class(type)) {
								return state.to_return = BC->this_m.scope->call(BC->this_m.scope_name, { to_cast });
							}
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Array_Call: {
						if (node.children.size() == 2) {
							auto Who = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;
							auto Where = evaluate(node.children[1], state, current_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;
							return state.to_return = current_scope.call("[]", { Who, Where });
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::If: {
						auto new_scope = current_scope.make_scope();
						if (node.children.size() == 2) { // condition, then
							auto condition = evaluate(node.children[0], state, new_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;
							if (new_scope.cast<bool>(condition)) {
								auto then = evaluate(node.children[1], state, new_scope);
								if (state.throwing != throwing::Nothing) return state.to_return;
								return state.to_return = then;
							}
							else {
								return state.to_return = nullptr;
							}
						}
						if (node.children.size() == 3) { // condition, then, else
							auto condition = evaluate(node.children[0], state, new_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;
							if (new_scope.cast<bool>(condition)) {
								auto then = evaluate(node.children[1], state, new_scope);
								if (state.throwing != throwing::Nothing) return state.to_return;
								return state.to_return = then;
							}
							else {
								auto then = evaluate(node.children[2], state, new_scope);
								if (state.throwing != throwing::Nothing) return state.to_return;
								return state.to_return = then;
							}
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Inline_Array: {
						if ((node.children.size() == 1)
							&& (node.children[0].identifier == Engine::AST_Node_Type::Arg_List)
							) {
							auto& argList = node.children[0];

							std::vector<GL::any::fast_any> inputs;
							std::set<GL::type> types;
							for (auto& child : argList.children) {
								inputs.push_back(evaluate(child, state, current_scope));
								if (state.throwing != throwing::Nothing) return state.to_return;
								types.insert(inputs.back().m_casted_type - GL::type::Reference - GL::type::Const - GL::type::Temporary);
							}

							auto& engine = *current_scope.GetRoot();
							if (types.size() == 1) {
								// vector<type>							
								if (auto* BC = engine.try_find_class(*types.begin()); BC && BC->this_m.is_class()) {
									auto new_vector = engine.call("vector<" + BC->this_m.scope_name + ">", {});
									for (auto& x : inputs) engine.call("push_back", { new_vector, x });
									return state.to_return = new_vector;
								}
								else { // vector<var>		
									auto new_vector = engine.call("vector<var>", {});
									for (auto& x : inputs) engine.call("push_back", { new_vector, x });
									return state.to_return = new_vector;
								}
							}
							else { // vector<var>
								auto new_vector = engine.call("vector<var>", {});
								for (auto& x : inputs) engine.call("push_back", { new_vector, x });
								return state.to_return = new_vector;
							}
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Inline_Map: {
						auto& engine = *current_scope.GetRoot();

						if (node.children.size() == 0) {
							auto new_map = engine.call("map<var,var>", {});
							return state.to_return = new_map;
						}

						if ((node.children.size() == 1)
							&& (node.children[0].identifier == Engine::AST_Node_Type::Arg_List)
							) {
							auto& argList = node.children[0];

							std::vector<std::pair<GL::any::fast_any, GL::any::fast_any>> pairs;
							std::set<GL::type> key_types;
							std::set<GL::type> value_types;

							for (int childIndex = 0; childIndex < argList.children.size(); childIndex++) {
								if (argList.children[childIndex].identifier == Engine::AST_Node_Type::Map_Pair
									&& argList.children[childIndex].children.size() == 2
									) {
									pairs.push_back({
										evaluate(argList.children[childIndex].children[0], state, current_scope),
										evaluate(argList.children[childIndex].children[1], state, current_scope)
										});

									if (state.throwing != throwing::Nothing) return state.to_return;

									key_types.insert(pairs.back().first.m_casted_type - GL::type::Reference - GL::type::Const - GL::type::Temporary);
									value_types.insert(pairs.back().second.m_casted_type - GL::type::Reference - GL::type::Const - GL::type::Temporary);
								}
								else {
									throw except::eval_error("Inline map definitions must follow the `key:pair` format, e.g. [key1:pair1, key2:pair2, ...]", node.location);
								}
							}

							GL::any::fast_any new_map;
							if (key_types.size() == 1) {
								if (auto* BC = engine.try_find_class(*key_types.begin()); BC && BC->this_m.is_class()) {
									if (value_types.size() == 1) {
										if (auto* BC2 = engine.try_find_class(*value_types.begin()); BC2 && BC2->this_m.is_class()) {
											new_map = engine.call("map<" + BC->this_m.scope_name + "," + BC2->this_m.scope_name + ">", {});
										}
										else throw except::eval_error("Inline map value type was not found: " + value_types.begin()->name(), node.location);
									}
									else {
										new_map = engine.call("map<" + BC->this_m.scope_name + ",var>", {});
									}
								}
								else throw except::eval_error("Inline map key type was not found: " + key_types.begin()->name(), node.location);
							}
							else {
								if (value_types.size() == 1) {
									if (auto* BC2 = engine.try_find_class(*value_types.begin()); BC2 && BC2->this_m.is_class()) {
										new_map = engine.call("map<var," + BC2->this_m.scope_name + ">", {});
									}
									else throw except::eval_error("Inline map value type was not found: " + value_types.begin()->name(), node.location);
								}
								else {
									new_map = engine.call("map<var,var>", {});
								}
							}

							for (auto& p : pairs) {
								engine.call("insert", { new_map, p.first, p.second });
							}

							return state.to_return = new_map;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Postfix: {
						if (node.children.size() == 1) {
							auto source = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;

							if (node.tag.cast< PostfixInformation>().is_unit) {
								return state.to_return = current_scope.call(node.tag.cast< PostfixInformation>().unit_name, { source });
							}
							else {
								switch (node.tag.cast< PostfixInformation>().oper) {
								case GL::Engine::Operators::Opers::pre_increment: {
									if (auto BC = current_scope.GetRoot()->try_find_class(source.m_casted_type)) {
										auto copied = BC->this_m.scope->call(BC->this_m.scope_name, { source });
										current_scope.call("++", { source });
										return state.to_return = copied;
									}
									break;
								}
								case GL::Engine::Operators::Opers::pre_decrement: {
									if (auto BC = current_scope.GetRoot()->try_find_class(source.m_casted_type)) {
										auto copied = BC->this_m.scope->call(BC->this_m.scope_name, { source });
										current_scope.call("--", { source });
										return state.to_return = copied;
									}
									break;
								}
								default: break;
								}
								throw except::eval_error("Unhandled postfix operator", node.location);
							}
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Prefix: {
						if (node.children.size() == 1) {
							auto source = evaluate(node.children[0], state, current_scope);
							switch (node.tag.cast< GL::Engine::Operators::Opers>()) {
							case GL::Engine::Operators::Opers::pre_increment: {
								current_scope.call("++", { source });
								return state.to_return = source;
							}
							case GL::Engine::Operators::Opers::pre_decrement: {
								current_scope.call("--", { source });
								return state.to_return = source;
							}
							case GL::Engine::Operators::Opers::unary_minus: {
								current_scope.call("-", { source });
								return state.to_return = source;
							}
							case GL::Engine::Operators::Opers::unary_plus:
							case GL::Engine::Operators::Opers::sum: {
								current_scope.call("+", { source });
								return state.to_return = source;
							}
							default:
								break;
							}

						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::For: {
						if (node.children.size() == 4) { // INIT_COMMAND, WHILE_CONDITION, CONTINUE_COMMAND, REPEAT_BLOCK
							auto new_scope = current_scope.make_scope(); {
								for ((void)evaluate(node.children[0], state, new_scope); new_scope.cast<bool>(evaluate(node.children[1], state, new_scope)); (void)evaluate(node.children[2], state, new_scope)) {
									if (state.throwing != throwing::Nothing) return state.to_return;

									auto new_scope2 = new_scope.make_scope(); {
										(void)evaluate(node.children[3], state, new_scope2);
										if (state.throwing == throwing::Continue) {
											state.throwing = throwing::Nothing;
											continue;
										}
										if (state.throwing == throwing::Break) {
											state.throwing = throwing::Nothing;
											break;
										}
										if (state.throwing != throwing::Nothing) return state.to_return;
									}
								}
							}
							return state.to_return = nullptr;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::While: {
						if (node.children.size() == 2) { // WHILE_CONDITION, REPEAT_BLOCK
							auto new_scope = current_scope.make_scope(); {								
								while (new_scope.cast<bool>(evaluate(node.children[0], state, new_scope))) {
									if (state.throwing != throwing::Nothing) return state.to_return;

									auto new_scope2 = new_scope.make_scope(); {
										(void)evaluate(node.children[1], state, new_scope2);
										if (state.throwing == throwing::Continue) {
											state.throwing = throwing::Nothing;
											continue;
										}
										if (state.throwing == throwing::Break) {
											state.throwing = throwing::Nothing;
											break;
										}
										if (state.throwing != throwing::Nothing) return state.to_return;
									}
								}
							}
							return state.to_return = nullptr;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::PreprocessedSwitch: {
						// for a pre-processed switch, it's effectively a scopeless block that could throw a "break"
						for (int i = 0; i < (node.children.size() - 1); ++i) {
							(void)evaluate(node.children[i], state, current_scope);
							if (state.throwing == throwing::Break) {
								state.throwing = throwing::Nothing;
								return state.to_return = nullptr;
							}
							if (state.throwing != throwing::Nothing) return state.to_return;							
						}
						state.to_return = evaluate(node.children.back(), state, current_scope);
						if (state.throwing == throwing::Break) {
							state.throwing = throwing::Nothing;
							return state.to_return = nullptr;
						}
						return state.to_return;
					}
					case Engine::AST_Node_Type::Switch: {
						auto& info = node.tag.cast< SwitchInformation>();

						if (node.children.size() > 1) {
							state.to_return = nullptr;

							auto switch_on = evaluate(node.children.front(), state, current_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;
							
							size_t this_hash = current_scope.call<size_t>("to_hash", { switch_on | GL::type::Const | GL::type::Reference });

							size_t starting_child_index = 1;
							if (auto f = info.hash_to_child_index.find(this_hash), e = info.hash_to_child_index.end(); f != e) {
								// from the found index on, compile into a block
								starting_child_index = f->second;
							}
							else {
								starting_child_index = 0;
								if (info.default_child_index < node.children.size()) {
									starting_child_index = info.default_child_index;
								}
								// the hash did not match. See if we can evaluate the "==" operation. 
								for (int i = 1; i < node.children.size(); ++i) {
									auto& child = node.children[i];
									if (child.identifier == Engine::AST_Node_Type::Case
										&& child.children.size() == 2
									) {
										auto case_o = evaluate(child.children[0], state, current_scope);
										if (state.throwing != throwing::Nothing) return state.to_return;

										if (current_scope.call<bool>("==", { switch_on | GL::type::Const | GL::type::Reference, case_o | GL::type::Const | GL::type::Reference })) {
											starting_child_index = i;
											break;
										}
										else {
											continue;
										}
									}
								}
								if (starting_child_index == 0) {
									// no matching hash, AND, no default. Therefore, this should compile down to nothing. 
									return state.to_return = nullptr;
								}
							}

							for (size_t i = starting_child_index; i < node.children.size(); ++i) {
								if (node.children[i].identifier == Engine::AST_Node_Type::Case) {
									if (node.children[i].children.size() == 2) {
										state.to_return = evaluate(node.children[i].children[1], state, current_scope);
										if (state.throwing == throwing::Break) {
											state.throwing = throwing::Nothing;
											return state.to_return = nullptr;
										}
										if (state.throwing != throwing::Nothing) return state.to_return;
										continue;
									}
								}
								else if (node.children[i].identifier == Engine::AST_Node_Type::Default) {
									if (node.children[i].children.size() == 1) {
										state.to_return = evaluate(node.children[i].children[0], state, current_scope);
										if (state.throwing == throwing::Break) {
											state.throwing = throwing::Nothing;
											return state.to_return = nullptr;
										}
										if (state.throwing != throwing::Nothing) return state.to_return;
										continue;
									}
								}
								throw except::eval_error("Unhandled switch processor: " + std::string(node.children[i].identifier.ToString()) + " were not handled: " + node.children[i].to_string("", *current_scope.GetRoot()), node.children[i].location);
							}

							return state.to_return;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::JustInTimeCompilation: {
						if (node.children.size() == 1) {
							auto script_o = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;
							GL::string script = current_scope.cast<GL::string>(script_o);

							auto new_node = Parse(script);
							auto r = evaluate(new_node, state, current_scope);
							if (state.throwing != throwing::Nothing) return state.to_return;
							return state.to_return = r;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Ranged_For: {
						if (node.children.size() == 3) { // DECL, RANGE_CONDITION, REPEAT_BLOCK
							auto new_scope = current_scope.make_scope(); {
								auto iter_list = evaluate(node.children[1], state, new_scope);
								if (state.throwing != throwing::Nothing) return state.to_return;

								if ((node.children[0].identifier == Engine::AST_Node_Type::Id 
									&& node.children[0].children.size() == 0)
									|| (node.children[0].identifier == Engine::AST_Node_Type::Var_Decl
										&& node.children[0].children.size() == 1
										&& node.children[0].children[0].identifier == Engine::AST_Node_Type::Id
										&& node.children[0].children[0].children.size() == 0)
								) { // for (x : [...]){ ... }
									GL::string var_name;
									if ((node.children[0].identifier == Engine::AST_Node_Type::Id
										&& node.children[0].children.size() == 0)) {
										var_name = node.children[0].text;
									}
									else if ((node.children[0].identifier == Engine::AST_Node_Type::Var_Decl
										&& node.children[0].children.size() == 1
										&& node.children[0].children[0].identifier == Engine::AST_Node_Type::Id
										&& node.children[0].children[0].children.size() == 0)){
										var_name = node.children[0].children[0].text;
									}
									else {
										throw except::eval_error("Something went wrong with the evalutation", node.location);
									}

									for (auto iterator = new_scope.call("begin", { iter_list }), iterator_end = new_scope.call("end", { iter_list }); new_scope.call<bool>("!=", { iterator, iterator_end }); new_scope.call("++", { iterator })) {
										if (state.throwing != throwing::Nothing) return state.to_return;										
										auto new_scope_2 = new_scope.make_scope(); {
											new_scope_2.insert_object_here(var_name, new_scope.call("get", { iterator }) | GL::type::Reference);

											(void)evaluate(node.children[2], state, new_scope_2);
											if (state.throwing == throwing::Break) {
												state.throwing = throwing::Nothing;
												break;
											}
											if (state.throwing == throwing::Continue) {
												state.throwing = throwing::Nothing;
												continue;
											}
											if (state.throwing != throwing::Nothing) {
												return state.to_return;
											}
										}
									}
									return state.to_return = nullptr;
								}
								else if (node.children[0].identifier == Engine::AST_Node_Type::Assign_Retroactively
									&& node.children[0].children.size() == 2
									&& node.children[0].children[1].identifier == Engine::AST_Node_Type::Var_Decl
									&& node.children[0].children[1].children.size() == 1
									&& node.children[0].children[1].children[0].identifier == Engine::AST_Node_Type::Id
									&& node.children[0].children[1].children[0].children.size() == 0
								) { // for (int x : [...]){ ... }
									(void)evaluate(node.children[0], state, new_scope);
									if (state.throwing != throwing::Nothing) return state.to_return;
									auto var = new_scope.find_object(node.children[0].children[1].children[0].text);

									for (auto iterator = new_scope.call("begin", { iter_list }), iterator_end = new_scope.call("end", { iter_list }); new_scope.call<bool>("!=", { iterator, iterator_end }); new_scope.call("++", { iterator })) {
										if (state.throwing != throwing::Nothing) return state.to_return;
										auto new_scope_2 = new_scope.make_scope(); {
											new_scope.call("=", { var, new_scope.call("get", { iterator }) | GL::type::Reference });

											(void)evaluate(node.children[2], state, new_scope_2);
											if (state.throwing == throwing::Break) {
												state.throwing = throwing::Nothing;
												break;
											}
											if (state.throwing == throwing::Continue) {
												state.throwing = throwing::Nothing;
												continue;
											}
											if (state.throwing != throwing::Nothing) {
												return state.to_return;
											}
										}
									}
									return state.to_return = nullptr;
								}
								else if (node.children[0].identifier == Engine::AST_Node_Type::Assign_Retroactively
									&& node.children[0].children.size() == 3
								) { // nonsense									
									throw except::eval_error("Cannot declare an assigned variable within a ranged-for-loop declaration, such as `for (int i = 0 : [...]){ ... }`. Insead, use a normal variable declaration, such as `for (int i : [...]){...}`", node.location);
								}						
							}
							
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Throw: {
						if (node.children.size() == 1) {
							auto returned = evaluate(node.children[0], state, current_scope);
							if (state.throwing != throwing::Nothing) {
								return state.to_return;
							}
							state.throwing = throwing::Error;
							return state.to_return = returned;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Try: {
						if (node.children.size() >= 1) {
							auto new_scope = current_scope.make_scope();
							GL::any::fast_any captured_thrown;
							try {
								auto returned = evaluate(node.children[0], state, new_scope);
								if (state.throwing == throwing::Error) {
									captured_thrown = state.to_return;
								}
								else {
									state.to_return = returned;
								}
							}
							catch (except::eval_error& e1) {
								captured_thrown = GL::any::fast_any::instance(GL::string(std::string(e1.what())));
							}
							catch (std::exception& e2) {
								captured_thrown = GL::any::fast_any::instance(GL::string(std::string(e2.what())));
							}
							catch (GL::any const& e3) {
								captured_thrown = e3.fast();
							}
							catch (GL::any::fast_any const& e4) {
								captured_thrown = e4;
							}
							catch (...) {
								captured_thrown = GL::any::fast_any::instance(GL::string("Unknown error"));
							}

							state.throwing = throwing::Nothing;
							if (captured_thrown) {
								for (size_t child_index = 1; captured_thrown && (child_index < node.children.size()); ++child_index) {
									if (node.children[child_index].identifier == Engine::AST_Node_Type::Catch) {
										if (node.children[child_index].children.size() == 1) { // catch(...)
											auto new_scope_2 = new_scope.make_scope();
											auto returned = evaluate(node.children[child_index].children[0], state, new_scope_2);
											if (state.throwing != throwing::Nothing) return state.to_return;
											state.to_return = returned;
											captured_thrown = nullptr;
											break;
										}
										else if (node.children[child_index].children.size() == 2) { // catch(e) or catch(int e)
											auto new_scope_2 = new_scope.make_scope();
											if (node.children[child_index].children[0].identifier == Engine::AST_Node_Type::Arg) {
												if (node.children[child_index].children[0].children.size() == 1
													&& node.children[child_index].children[0].children[0].identifier == Engine::AST_Node_Type::Id
												) {
													auto var_name = node.children[child_index].children[0].children[0].text;
													new_scope_2.insert_object_here(var_name, captured_thrown);
												}
												else if (node.children[child_index].children[0].children.size() == 2
													&& node.children[child_index].children[0].children[0].identifier == Engine::AST_Node_Type::Id
													&& node.children[child_index].children[0].children[1].identifier == Engine::AST_Node_Type::Id
												){
													auto var_type_name = node.children[child_index].children[0].children[0].text;
													auto var_name = node.children[child_index].children[0].children[1].text;
													
													if (captured_thrown.can_cast(new_scope_2.DetermineType(var_type_name))) {
														new_scope_2.insert_object_here(var_name, captured_thrown);
													}
													else {
														continue;
													}
												}
											}											
											
											auto returned = evaluate(node.children[child_index].children[1], state, new_scope_2);
											if (state.throwing != throwing::Nothing) {												
												captured_thrown = nullptr;
												break; 
											}
											else {
												state.to_return = returned;
												captured_thrown = nullptr;
												break; //  return state.to_return;
											}
										}
									}
								}	
								if (captured_thrown) {
									// went un-handled, so re-throw
									state.throwing = throwing::Error;
									return state.to_return;
								}
							}
							else {
								throw except::eval_error("Exception caught but nothing was captured", node.location);
							}

							for (size_t child_index = 1; child_index < node.children.size(); ++child_index) {
								if (node.children[child_index].identifier == Engine::AST_Node_Type::Finally) {
									if (node.children[child_index].children.size() == 1) { // finally{ ... }
										auto new_scope_2 = new_scope.make_scope();
										auto returned = evaluate(node.children[child_index].children[0], state, new_scope_2);
										if (state.throwing != throwing::Nothing) return state.to_return;
										return state.to_return = returned;
									}									
								}
							}

							return state.to_return;
						}
						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Lambda: {

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::FunctionBlock: {

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::DeclarationBlock: {

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Do: {

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Parallel: {

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Parallel_For: {

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Parallel_Ranged_For: {

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Value_Range: {

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Inline_Range: {

						throw except::eval_error("Parameters for " + std::string(node.identifier.ToString()) + " were not handled: " + node.to_string("", *current_scope.GetRoot()), node.location);
					}
					case Engine::AST_Node_Type::Namespace: break;
					case Engine::AST_Node_Type::Class: break;
					case Engine::AST_Node_Type::FunctionDecl: break;
					case Engine::AST_Node_Type::Finally: [[fallthrough]];
					case Engine::AST_Node_Type::Catch: [[fallthrough]];
					case Engine::AST_Node_Type::Case: [[fallthrough]];
					case Engine::AST_Node_Type::Default: [[fallthrough]];
			        case Engine::AST_Node_Type::Map_Pair: [[fallthrough]];
					case Engine::AST_Node_Type::Arg_List: [[fallthrough]];
					case Engine::AST_Node_Type::Arg: [[fallthrough]];
					case Engine::AST_Node_Type::Def: [[fallthrough]];
					case Engine::AST_Node_Type::ControlBlock: [[fallthrough]];					
					case Engine::AST_Node_Type::Attr_Decl: [[fallthrough]];
					case Engine::AST_Node_Type::Reference: [[fallthrough]];
					case Engine::AST_Node_Type::Assign_Decl: [[fallthrough]];
					case Engine::AST_Node_Type::Global_Decl: [[fallthrough]];
					case Engine::AST_Node_Type::Compiled: [[fallthrough]];
					case Engine::AST_Node_Type::TypeId: [[fallthrough]];
					default:
						throw except::eval_error("Unhandled Node Type During Evaluation: " + std::string(node.identifier.ToString()), node.location);
					}
					return nullptr;					
				};
				GL::any::fast_any Eval(AbstractSyntaxTreeNode& node, eval_state& state, GL::scope::impl::RootScope& parent_scope)  {					
					preEval_declarations(node, parent_scope);

					while (node.for_each_child([this, &parent_scope](AbstractSyntaxTreeNode& this_node) -> bool {
						return preEval_returnTypes(this_node, parent_scope) || preEval_functionSelections(this_node, parent_scope);
					}, true)) {};

					try {
						return evaluate(node, state, parent_scope);
					}
					catch (except::eval_error& e) {
						state.throwing = GL::Engine::throwing::Error;
						throw e;
					}
					catch (std::exception& e) {
						state.throwing = GL::Engine::throwing::Error;
						throw except::eval_error(std::string(e.what()), node.location);
					}
					catch (GL::any::fast_any& e) {
						state.throwing = GL::Engine::throwing::Return;
						return state.to_return = e;
					}
					catch (GL::any& e) {
						state.throwing = GL::Engine::throwing::Return;
						return state.to_return = e.fast();
					}
					catch (...) {
						state.throwing = GL::Engine::throwing::Error;
						throw except::eval_error("Unknown error", node.location);
					}
				};
			};

		};

	};

};

int main() {
	if (1) {
		for (GL::string& Script : std::vector<GL::string>{
R"(
	namespace TEST {
		foot Foo(){ return 10; }
	}
	return TEST::Foo();
)",R"(
	try{
		auto x = 0;
		while (true){
			++x;
			if (x > 1000){
				throw x;
			}
		}
	}catch(string e){
		e + " caught 1";
	}catch(double e){
		e.to_string + " caught 2";
	}catch(value e){
		e.to_string + " caught 3";
	}catch(int e){
		e.to_string + " caught 4";
	}catch(e){
		e.to_string + " caught 5";
	}finally{
		"superceded?";
	};
)",R"(
	auto t0 = datetime::Now();
	for (auto i = 0; i < 1'000'000; ++i) {
		constexpr auto x0 = 100.0_ft; // 100 ft
		constexpr auto v0 = 10_ft / 1_s; // 10 fps
		constexpr auto a0 = v0 / 1_s; // 10 fps_sq
		constexpr auto t = 5_s; // 5 s
		constexpr auto d = v0 * t + t.pow(2) * a0 * 0.5; // 175 ft
		constexpr auto x = x0 + d; // 275 ft		
#if x != 275_ft
	#error Constexpr resulted in the wrong answer
#endif
	}
	return (datetime::Now() - t0)_ms
)",R"(
	auto t0 = datetime::Now();
	for (auto i = 0; i < 1'000'000; ++i) {
		auto x0 = 100.0_ft; // 100 ft
		auto v0 = 10_ft / 1_s; // 10 fps
		auto a0 = v0 / 1_s; // 10 fps_sq
		auto t = 5_s; // 5 s
		auto d = v0 * t + t.pow(2) * a0 * 0.5; // 175 ft
		auto x = x0 + d; // 275 ft			
	}
	return (datetime::Now() - t0)_ms
)",R"(
	string out;
	for (x : [1,2,3,4,5,6,7,8]){
		out = out.add_to_delim(to_string(x), ", ");
	}
	return out;
)",R"(
	string out;
	for (auto x : [1,2,3,4,5,6,7,8]){
		out = out.add_to_delim(to_string(x), ", ");
	}
	return out;
)",R"(
	string out;
	for (int x : [1,2,3,4,5,6,7,8]){
		out = out.add_to_delim(to_string(x), ", ");
	}
	return out;
)",R"(
	string out;
	for (foot x : [1,2,3,4,5,6,7,8]){
		out = out.add_to_delim(to_string(x), ", ");
	}
	return out;
)",R"(
	auto cmd = "int y; for (auto x = 0; x < 10; ++x){ y += x; } return y;";
	return evaluate(cmd);
)",R"(
	auto cmd = "_ft";
	return evaluate("100.0" + cmd);
)",R"(
	int x = 1;
	switch(1){
		case 0: { x = 0; break; }
		case 1: { x = 1; if (x = 1){ break; } }
		case 2: { x = 2; break; }
		case 3: { x = 3; break; }
		case 4: { x = 4; break; }
		default: { x = -1; break; }
	}

	switch(x){
		case 0: return 0;
		case 1: break;
		case 2: return 2;
		case 3: return 3;
		case 4: return 4;
		default: return -1;
	}
	if (x > 0){
		return x;
	}
	return -1;
)",R"(
	int y;
	for (auto x = 0; x < 10; ++x){
		if (y > 10){
			break;
		}else{
			y += x;	
			continue;
		}
	}
	return y;
)",R"(	
	while (true){
		break;
	}
)",R"(
	int y;
	for (auto x = 0; x < 10; ++x){
		y += x;
	}
	return y;
)",R"(
	int y;
	while (y < 10){
		++y;
	}
	return y;
)",R"(
	auto x = 10;
	auto m = [ x++, ++x + 10, x + 10_ft, (x)_ft ];
	return m;
)",R"(
	auto x = 10;
	auto m = [ "a":x, "b":(x + 10), "c":(x + 10_ft), "d":x ];
	return m;
)",R"(
	auto x = 10;
	if (x >= 10){
		if (x == 10){
			return x;
		}
		x += 1;
	}
	return x + 5;
)",R"(
	auto vector = [1,2,3,4];
	return (foot)vector[1];
)", R"(
	constexpr auto vector = [1,2,3,4];
	return (foot)vector[1];
)", R"(
	auto x = 10;
	if (x >= 10){
		return 10;
	}
	return 20;
)",R"(
	var x = 10;
	return (float)x;
)",R"(
	10_ft; // constexpr `foot`
)",R"(
	var x = 10;
	x += 10;
	return x;
)",R"(
	foot x = 10;
	x = 0_m;
	return x;
)",R"(
	vector<int> out;
	push_back(out, 1);
	push_back(out, 2);
	push_back(out, 3);
	return out;
)",R"(
	vector<int> out;
	out.push_back(1);
	out.push_back(2);
	out.push_back(3);
	return out;
)",R"(
	auto x = 10;
	auto y = 10;
	auto z = 10;
	return x + y + z;
)",R"(
	auto x = 10;
	auto y = 10;
	auto z = 10;
	return (((x + y) * z) - y) + 10;
)",R"(
	auto x = 10;
	auto y = 10;
	auto z = 10;
	return (((x + y) / 10) - 5) + z;
)"
		}) {
			GL::scope::impl::RootScope root;
			root.perform_builtins();
			GL::Engine::ScriptParser::Parser parser(root);
			auto parsed = parser.Parse(Script);

			print(Script);
			print(" >> ");
			print(parsed.to_string("", root));

			GL::Engine::eval_state evaluation_state;
			evaluation_state.throwing = GL::Engine::throwing::Nothing;
			evaluation_state.to_return = nullptr;
			try {
				auto returned = parser.Eval(parsed, evaluation_state, root);
				print("returned: " + root.call<GL::string>("to_string", { returned }));
				if (evaluation_state.throwing != GL::Engine::throwing::Nothing) {
					print("thrown: " + root.call<GL::string>("to_string", { evaluation_state.to_return }));
				}
			}
			catch (std::exception& e) {
				print(e.what());
			}
			print("\n\n");

		}

		
	}



	if (1) {
		GL::scope::impl::RootScope root;
		root.perform_builtins();
		GL::Engine::ScriptParser::Parser parser(root);

		if (1) {
			print(parser.Parse(R"(
return evaluate("100_ft" + "+" + "200_m");
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
return evaluate("${ 100 }_ft + ${ 200 }_m");
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
return evaluate("[100, 200, 300, 400]");
			)").to_string("", root) + "\n\n");



			print(parser.Parse(R"(
constexpr auto x = 10_ft
constexpr auto y = 10_ft + 10_ft;
constexpr auto z = 10_ft - 10_ft;
constexpr auto w = 10_ft * 10_ft;
constexpr auto a = 10_ft / 10_ft;
constexpr auto b = 10_ft == 10_ft;
constexpr auto c = 10_ft != 10_ft;
constexpr auto d = 10_ft > 10_ft;
constexpr auto e = 10_ft < 10_ft;
constexpr auto f = 10_ft >= 10_ft;
constexpr auto g = 10_ft <= 10_ft;
constexpr auto h1 = 10_ft % 3;
constexpr auto h2 = 10_ft % 3_ft;
constexpr auto h3 = 10_sq_ft % 3_ft;
constexpr auto i = 10 && 10_ft;
constexpr auto j = 10 || 10_ft;
#define in(Val) [#Val:Val]
return [ in(x), in(y), in(z), in(w), in(a), in(b), in(c), in(d), in(e), in(f), in(g), in(h1), in(h2), in(h3), in(i), in(j) ];
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#include 10 + 10;			
			)").to_string("", root) + "\n\n");
			
			print(parser.Parse(R"(
auto x = 0;
#include x += 10;			
return x;
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
constexpr auto x = 10;
#include x + 10;
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define TEST 10;
#include constexpr auto x = TEST + 10;
return x;
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define TEST(x) #x;
#include TEST(100);
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
constexpr auto x = 10;
auto y;
#include \
	constexpr auto z = x + 10; \
	#if x \
		y = x; \
	#else \
		#error "ERR1" \
	#endif \
	#include \\
		#if z > x \\
			y = z; \\
		#else \\
			#error "#endif" \\
		#endif

return y + 10;
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
constexpr auto x = 10;
auto y = x + 10;
x += 10;
			)").to_string("", root) + "\n\n");



			print(parser.Parse(R"(
#define CONCAT1(a, b) a ## b
return CONCAT1(defer_, 200); // returns Id_Node{ "defer_200" }
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) #a
return CONCAT2(100); // returns "100"
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define PASS_THROUGH(a) a
#define CONCAT2(a) #a
constexpr auto x = PASS_THROUGH(100); // returns 100
return [CONCAT2(x), x]; // returns \"x\"
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) x + a + a##a
return CONCAT2(VAR);
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) x + #a + a##a
return CONCAT2(VAR);
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) #x + #a + a##a
return CONCAT2(VAR);
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) #x + #a + #(a##a)
return CONCAT2(VAR);
			)").to_string("", root) + "\n\n");
		}
		if (1) {
			print(parser.Parse(R"(
				#ifdef TEST
					#error "ERROR"
				#else
					#warning "SUCCESSFUL"
				#endif
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
					0.0f;
				#ifdef TEST
					#error "ERR"
				#else
					200.0f;
				#endif
					300.0f;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
                #define TEST
					0.0f;
				#ifdef TEST
					100.0f;
				#else
					#error "ERR"
				#endif
					300.0f;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
                #define TEST
				#ifndef TEST
					#error "ERR"
				#else
					100.0f;
				#endif
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
                #if 1
					100.0f;
				#else
					#error "ERR"
				#endif
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
                #if 100
					100.0f;
				#else
					#error "ERR"
				#endif
			)").to_string("", root) + "\n\n");

			// Using basic math as part of the #if macros
			print(parser.Parse(R"(
                #if 1 > 0
					100.0f;
				#else
					#error "ERR"
				#endif
			)").to_string("", root) + "\n\n");

			// Using constexpr values as part of the #if macros
			print(parser.Parse(R"(
				constexpr auto x = 10;
                #if x > 0
					100.0f;
				#else
					#error "ERR"
				#endif
			)").to_string("", root) + "\n\n");

			// Using constexpr values as part of the #if macros
			print(parser.Parse(R"(
				constexpr bool x = true;
                #if x
					100.0f;
				#else
					#error "ERR"
				#endif
			)").to_string("", root) + "\n\n");

			// Using constexpr values as part of the #if macros
			print(parser.Parse(R"(
				constexpr bool x = false;
                #if x
					#error "ERR"
				#else
					100.0f;
				#endif
			)").to_string("", root) + "\n\n");

			// Using constexpr values as part of the #if and #elif macros
			print(parser.Parse(R"(
				constexpr bool x = false;
				constexpr bool y = true;
                #if x
					#error "ERR"
                #elif y
					100.0f;
				#else
					#error "ERR"
				#endif
			)").to_string("", root) + "\n\n");

			// Using constexpr values as part of the #if and #elif macros
			print(parser.Parse(R"(
				constexpr bool x = false;
				constexpr bool y = false;
				constexpr bool z = true;
                #if x
					#error "ERR";
                #elif y
					#error "ERR";
                #elif z
					100.0f;
				#else
					#error "ERR";
				#endif
			)").to_string("", root) + "\n\n");
			
			// Using constexpr values as part of the #if and #elif macros
			print(parser.Parse(R"(
				constexpr bool x = false;
				constexpr bool y = false;
				constexpr bool z = false;
                #if x
					#error "ERR";
                #elif y
					#error "ERR";
                #elif z
					#error "ERR";
				#else
					100.0f;
				#endif

                #if x
					#error "ERR";                
				#else
					100.0f;
				#endif

                #if x
					#error "ERR";                
				#else
					#if y
						#error "ERR";                
					#else
						#if z
							#error "ERR";                
						#else
							100.0f;
						#endif
					#endif
				#endif
			)").to_string("", root) + "\n\n");
			
			// Using a constexpr array and array access functions in the #if macro
			print(parser.Parse(R"(
				constexpr auto x = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
                #if x[0]
					#error "ERR"
				#elif x[2] - 2
					#error "ERR"
				#elif x[9] > x[0]
					100.0f;
				#endif
			)").to_string("", root) + "\n\n");

			// Using a macro ID replacement within the #if macro
			print(parser.Parse(R"(
				#define TEST 100
				#if TEST
					100.0f;
				#else
					#error "ERR"
				#endif
			)").to_string("", root) + "\n\n");

			// Using a macro function within the #if macro
			print(parser.Parse(R"(
				#define TEST(x) x + 1
				#ifdef TEST
					#if TEST(10)
						100.0f;
					#else
						#error "ERR"
					#endif
				#else
					#error "ERR"
				#endif
			)").to_string("", root) + "\n\n");

			// Using a macro function within the #if and #elif macros
			print(parser.Parse(R"(
				#define TEST(x) x - 1
				#ifdef TEST
					#if TEST(0) >= 1
						#error "ERR";
					#elif TEST(1) >= 1
						#error "ERR";
					#elif TEST(2) >= 1	
						100.0f;
					#else
						#error "ERR";
					#endif
				#else
					#error "ERR"
				#endif
			)").to_string("", root) + "\n\n");

			// This will return a (intentional) warning. 
			print(parser.Parse(R"(
#if 1
				if (true) {
					100;
				}
				else{
					#warning "Successfully demonstrates that warnings and errors are collected if they are parsed at all"
				}
#else
				if (true) {
					#error "ERR"
				}
				else{
					#error "ERR"
				}
#endif
			)").to_string("", root) + "\n\n");

			// this will retun an error, since it could not parse the "x" into a constexpr value.
			print(parser.Parse(R"(
				bool x = false;
                #if x
					#error "ERR"
				#else
					100.0f;
				#endif
			)").to_string("", root) + "\n\n");


			// This will return an error, since the #if statement could not parse the 'y' vaue inside the #define statement. 
			// This might be fixable with some effort, but the value seems limited. 
			print(parser.Parse(R"(
				#define Thingy(x,y,z) class x { \
					#if y \
						constexpr auto z = "SUCCESS"; \
					#else \
						constexpr auto z = "OTHERWISE"; \
					#endif \
				}
	
				Thingy(className, true, varName);
			)").to_string("", root) + "\n\n");



			// 
			print(parser.Parse(R"(			
				constexpr value PI = 3.14;
				PI += 10;
				return [PI];
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(			
				constexpr value PI = (double)constant::pi;
				constexpr value A1 = 1000.0 * PI;
				A1 += 1; // this does not throw because A1 ended up NOT being constexpr. It is therefore a runtime object, and can safely be operated on this way. 				
				return [PI,A1];
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				constexpr cubic_foot_per_second QZERO = 1.e-6;
				constexpr value PI = (double)constant::pi;
				constexpr value A1 = 1000.0 * PI;
				constexpr value A2 = 500.0 * 3.141592653589793238462643383279502884197169399375105820974944f;
				constexpr value A3 = 16.0 * 3.141592653589793238462643383279502884197169399375105820974944f;
				constexpr value A4 = 2.0 * 3.141592653589793238462643383279502884197169399375105820974944f;
				constexpr value A8 = 4.61841319859066668690e+00; // 5.74*(PI/4)^.9
				constexpr value A9 = -8.68588963806503655300e-01;  // -2/ln(10)
				constexpr value AA = -1.5634601348517065795e+00; // -2*.9*2/ln(10)
				constexpr value AB = 3.28895476345399058690e-03; // 5.74/(4000^.9)
				constexpr value AC = AA * AB;
				constexpr value CSMALL = 1.e-6;
				constexpr value CBIG = 1.e8;
				return [QZERO,PI,A1,A2,A3,A4,A8,A9,AA,AB,AC,CSMALL,CBIG];
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				auto x = (string)10_ft.length;
				auto y = (int)10_ft;
				return [x, y];
			)").to_string("", root) + "\n\n");
		}
		if(1){
			print(parser.Parse(R"(
				(second)1 / (day)1
			)").to_string("", root) + "\n\n");
		}
		if (1) {
			print(parser.Parse(R"(
				value SECperDAY = 1.0 / ((second)1 / (day)1); 
				return SECperDAY;
			)").to_string("", root) + "\n\n");
		}
		if (1) {
			print(parser.Parse(R"(
				value SECperDAY = 1.0 / ((second)(1) / (day)(1)); 
				return SECperDAY;
			)").to_string("", root) + "\n\n");
		}
		if (1) {
			print(parser.Parse(R"(
				constexpr value SECperDAY = (1.0 / (((second)(1)) / ((day)(1)))); 
				return SECperDAY;
			)").to_string("", root) + "\n\n");
		}
		if (1) {
			print(parser.Parse(R"(
				constexpr value SECperDAY = 1 / (1_s / 1_d); 
				return SECperDAY;
			)").to_string("", root) + "\n\n");
		}

		// Reads an if/else if/else block from input
		if (1) {
			print(parser.Parse(R"(
				if (x){
					++x;
					return x;
				}
				else {
					return y;
				}
				return "FAILURE";
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (x) return x;				
				else return y;				
				return "FAILURE";
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (x) 
					return x;				
				else if (y)
					return y;
				else if (z)
					return z;
				else if (w)
					return w;
				return "Following All If-Else-If Checks...";
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (x){
					++x;
					return x;
				}
				else {
					++x;
				}
				return "INSIDE ELSE";
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (x){
					++x;
				}
				else {
					++x;
					return x;
				}
				return "INSIDE THEN";
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (x){
					++x;
				}
				else {
					++x;
				}
				return "INSIDE NONE";
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (x){
					return "If True";
				}else{
					return "If False";
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (true){
					return "Correct";
				}else{
					return "ISSUE!";
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (false){
					return "ISSUE!";
				}else{
					return "Correct";
				}
			)").to_string("", root) + "\n\n");
		}
		// Reads a return statement from input
		if (1) {
			print(parser.Parse(R"(
				10;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				return 10;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (x){
					return x;
				}
			)").to_string("", root) + "\n\n");
		}
		// Reads a curly-brace C-style block from input
		if (1) {
			print(parser.Parse(R"(
				{
					int x;
					x + 1;
				}
				{
					int y;
					y + 2;					
				}				
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int x;
				{
					x + 1;
				}
				{
					int y;
					y + 2;					
				}			
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int x;
				{
					x + 1;
					{
						x + 2;
					}
				}
				{
					x + 3;
					return x;
				}		
			)").to_string("", root) + "\n\n");
		}
		// switch statement(s)
		if (1) {
			print(parser.Parse(R"(
				switch (x){
				case 0: {
					return 0;
					break;
				}
				case 1: {
					return 1;
					break;
				}
				case 2: {
					return 2;
					break;
				}
				default: {
					break;
				}
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				switch (1){
				case 0 {
					return 0;
					break;
				}
				case 1 {
					return 1;
					break;
				}
				case 2 {
					return 2;
					break;
				}
				default {
					return -1;
					break;
				}
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				switch (1) {
				case 0: 
					return 0;		
				case 1: 
					return 1;			
				case 2: 
					return 2;	
				default: 
					return -1;		
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int x;
				switch (x) {
				case 0: 
					return 0;				
				case 1: {
					x = 0;
					return 1;
				}		
				case 2: 
					x += 20;
				case 3: 
					x += 30;			
				default: 
					return x;				
				};
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int x;
				switch (1) {
				case 0: 
					x += 0;
					// Comment 0
				case 1: 
					x += 1;				
					// Comment 1
				case 2: {
					x += 2;
					break;
					// Comment 2
				}	
				default: 
					x += -1;
					// Comment Default
				}
				return x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int y = 0;
				constexpr auto x = 1;
				switch (x) {
				case 0: 
					y += 0;
					// Comment 0
				case 1: 
					y += 1;				
					// Comment 1
				case 2: {
					y += 2;
					break;
					// Comment 2
				}	
				default: 
					y += -1;
					// Comment Default
				}
				return y;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int y = 0;
				constexpr int x = 1;
				switch (x) {
				case 0: 
					y += 0;
					// Comment 0
				case 1: 
					y += 1;				
					// Comment 1
				case 2: {
					y += 2;
					break;
					// Comment 2
				}	
				default: 
					y += -1;
					// Comment Default
				}
				return y;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				switch (1) {
				case 0: 
					y += 0;
				case 1: {
					if (x){
						break;
					}					
				}
				case 2: {
					y += 2;
					if (true){
						{
							y += 1;
							break;
							y += 3;
						}
					}
				}	
				default: 
					y += -1;
				}
				return y;
			)").to_string("", root) + "\n\n");

		}
		// units and literals
		if (1) {
			print(parser.Parse(R"(
				10_ft
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				10_ft + 12_in
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				return (110_ft + 120_in) / 120_s;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				if (true){
					(10_ft + 10_m + 100_in + 0.01_mi) / (0.05_d);
				}else{
					return 100;
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				1_MG;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				(x)_MG;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				return (1_Mgal)_ac_ft;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				return (foot(110) + inch(120)) / second(120);
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				auto x = 11.0f;
				return ((x)_ft+(12)_in)/(120)_s;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				constexpr auto x = 11.0f;
				return ((x)_ft+(12)_in)/(120)_s;
			)").to_string("", root) + "\n\n");
		}
		// types
		if (1) {
			print(parser.Parse(R"(
				[](int x){};
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				[](int& x){};
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				[](int const& x){};
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				[](const int& x){};
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				[](int&& x){};
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				var x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				auto x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				auto x = 10;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				var x = 10;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int x = 10;
				return x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int& x = 10;
				return x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				int const& x = 10;
				return x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				const int& x = 10;
				return x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				const int x = 10;
				return x;
			)").to_string("", root) + "\n\n");

		}
		// type conversion
		if (1) {
			print(parser.Parse(R"(
				(int)x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				return (int)x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				auto x = (int)10_ft;
			)").to_string("", root) + "\n\n");
		}
		// For, While loops
		if (1) {
			print(parser.Parse(R"(
				for (int i = 0; i < 10; ++i){}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				for (; true;){}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				for (;;){}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				for (;;) ++x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				for (;;) 
					++x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				while (true){}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				while (true) ++x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				while (true) 
					++x;
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				for (x : vector<int>()){}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				for (int x : vector<int>()){}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				for (int& x : vector<int>()){}
			)").to_string("", root) + "\n\n");
		}
		// Try/Catch/Finally
		if (1) {
			print(parser.Parse(R"(
				try{
					auto x = 10;
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				try {
					auto x = 10;
				}
				catch(e){
					return 10;
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				try {
					auto x = 10;
				} catch(e) {
					return e.what();
				} catch(int e2) {
					return e2;
				} catch(...) {
					return e2;
				} finally {
					return 10;
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				try {
					auto x = 10;
				} catch(e) {
					return e.what();
				} catch(int e2) {
					return e2;
				} catch(double e2) {
					return e2;
				} catch(string e2) {
					return e2;
				} catch(...) {
					return e2;
				} finally {
					return 10;
				}
			)").to_string("", root) + "\n\n");
		}
		// Namespaces
		if (1) {
			print(parser.Parse(R"(
				if (x){
					return 1; // comment
					// namespace description
					namespace TEST_IF {
						// namespace description
						namespace IMPL{
							void Function(x) { 
								return 100.0f + x; // Comment 
								namespace FunctionImpl{}
							}
						}
					};	
				}else{
					namespace TEST_ELSE {};
				}
				namespace TEST {};
				namespace TEST2 {};
			)").to_string("", root) + "\n\n");			
			print(parser.Parse(R"(
				namespace std {}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				namespace std {
					namespace string {}
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				namespace std {}
				if (x) {
					namespace std {
						namespace string {}
					}
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				namespace std {
					int x = 10;
					int Function(){ return 10; };
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				namespace std {
					int x = 10;
					int Function(){ return 10; };
					int y;
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				namespace std {
					auto z = 10;
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				namespace std {
					auto z;
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				namespace std {
					int AbsDouble(int i) { 
						switch (i) {
						case 0: return 0;
						case 1: return 2;
						case 2: return 4;
						default: return ((i * 2) > 0) ? (i * 2) : (i * -2);
						}
					};
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				namespace std {
					int AbsDouble(int i) { 
						switch (i) {
						case 0: return 0;
						case 1: return 2;
						case 2: return 4;
						default: return ((i * 2) > 0) ? (i * 2) : (i * -2);
						}
					};
				}
			)").to_string("", root) + "\n\n");
		}
		// Classes
		if (1) {
			print(parser.Parse(R"(
				class example {}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				class example {
					int Function(){ return 10+x+y+10+z+w+20; };		
					auto x;
					auto y = 10;
					int z;
					int w = 10;			
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(				
				class Vector<T0> {					
					T0 x;
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(				
				class Map<T0, T1> {					
					T0 x;
					T1 y;
				}
			)").to_string("", root) + "\n\n");

		}
		// parallel for
		if (1) {
			print(parser.Parse(R"(
				parallel_for (int i = 0; i < 10; ++i){
					return 10;
				}
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				parallel_for (i : [0, 1, 2, 3, 4]){
					return 10;
				}
			)").to_string("", root) + "\n\n");
		}
		// Combined Examples
		if (1) {
			print(parser.Parse(R"(
				constexpr auto x0 = 100.0_ft; // 100 ft
				constexpr auto v0 = 10_ft / 1_s; // 10 fps
				constexpr auto a0 = v0 / 1_s; // 10 fps_sq
				constexpr auto t = 5_s; // 5 s
				constexpr auto d = v0 * t + t.pow(2) * a0 * 0.5; // 175 ft
				constexpr auto d2 = v0 * t + pow(t, 2) * a0 * 0.5; // 175 ft
				constexpr auto x = x0 + d; // 275 ft
				return [x0,v0,a0,t,d,d2,x]
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
#if "TEST1 TEST2".left_and_right_of(" ").first.size >= 4
				return [ "TEST".size(), size("TEST"), "TEST".distance("test"), "TEST".distance("test", false), "TEST1 TEST2".left_and_right_of(" ").first ];				
#else
				#error "CONSTEXPR FAILED"
#endif
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
return [1,2,3,4].begin().get().to_string();
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
return ([1,2,3,4].begin() + 3).get().to_string();
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
return ([1_ft,2_m,3_fps,[4_gpm:4_MG]].begin() + 3).get().to_string();
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
return ([1_ft,2_m,3_fps,[4_gpm:4_MG]].begin() + 3).get().begin().get().first.to_string();
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
constexpr auto data_map = [ "a":1_MGD, "b":2_gpm, "c":3_ft, "d":4_fps, "e":5_L ];
constexpr auto data_map_access_1 = data_map["b"];
constexpr auto data_map_access_2 = (data_map.begin() + 4).get();
#if data_map_access_2.first == "e"
	return data_map_access_2.second;
#else
	#error ERR
#endif
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
constexpr auto data = "TESTING";
return data[0];
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
return "TEST".add_to_delim("ING", "...").split(".");
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(		
constexpr auto LIST = ["a", "b", "c", "d", "e", "f", "g", "h"];
auto MAP = map<int, string>();

// Utilizing a macro function to unroll, rather than using a for-loop.
#define factorial(n) if (n >= 0){ if (n < LIST.size()) { MAP.insert(n, LIST[n]); } factorial(n-1); }
factorial(LIST.size() - 1);
#undef factorial
return MAP;
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(		
var y;
if (constexpr auto x = 100){
	y = x + 1;
}else{
	y = x + 2;
}

auto x = 105;
return y + x;
			)").to_string("", root) + "\n\n");








			print(parser.Parse(R"(				
				for (int i = 0; i < 10; ++i){
					namespace NS {
						int Increment(int& x){ return ++x; }
					}
					return NS::Increment(i);
				}
			)").to_string("", root) + "\n\n");
		}
		// constexpr 
		if (1) {
			print(parser.Parse(R"(
				constexpr auto x = 11.0f;
				constexpr int y = 5;
				constexpr auto z = int();
				constexpr auto w = x + y; 
				constexpr auto a = x + y + z; 
				constexpr auto b = (x + y) * (z + 2); 
				constexpr auto c = 10_ft + x;
				return [x,y,z,w,a,b,c];
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				constexpr double size = ((size_t)10);
				size("TEST", size);
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				constexpr auto x = 11.0f;
				if (x > 0) {
					"TEST1";
				}
				if (x) {
					"TEST2";
				}
				while (x) {
					"TEST3"
				}
				for (auto y = 0; y < x; ++y) {
					"TEST4"
				}
				for (; x; ) {
					"TEST5"
				}
				for (Z : [x, x+2, x+5, x+10_ft]) {
					return Z;
				}
				for (Z : [x:10, 10:x]) {
					return Z;
				}
				constexpr double size = ((size_t)10);
				"TEST".size();
				size("TEST");
				"TEST".size(size);
				size("TEST", size);
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				constexpr auto x = 11.0f;
				constexpr auto arr = [x, x+2, x+5, x+10_ft];
				constexpr auto z = arr[0];
				return [x, arr, z];
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				constexpr auto x = 11.0f;
				constexpr auto map = [(x):(x+2), (10_ft):(100_m+x)];
				constexpr auto w = map[x];
				return [x, map, w];
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				constexpr auto x = 11.0f;
				constexpr auto y = ((x)_MG / 31_d)_gpm;
				return [x, y];
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
				auto x = 11.0f;
				constexpr auto vector = [ (double)x, x+1_m, x+2_ft, x+3_s, (x)_MG ]; // will NOT compile-down to constexpr, since x is itself not constexpr
				constexpr auto w = vector[4]; // will NOT compile-down to constexpr, since x is itself not constexpr
				return [x, vector, w];
			)").to_string("", root) + "\n\n");
			//print(parser.Parse(parser.Preprocess(R"(
			//	// Compile-time absolute value
			//	constexpr double abs_val(double x) {
			//		return x < 0 ? -x : x;
			//	};
			//	return abs_val(-1);
			//)")).to_string("", root) + "\n\n");


		}
		// recursive pre-compilation test
		if (1) {
			print(parser.Parse(R"(
#warning "TEST1"
#warning TEST2
#warning TEST ... TEST 3 
#warning TEST ... \
TEST 4
				return 10;				
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
#error "TEST1"
#error TEST2
#error TEST ... TEST 3 
#error TEST ... \
TEST 4
				return 10;				
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#include "path.h"
#include path.h
#include path
#include www.github.com/path
#include www.github.com/path/path.h
				return 10;				
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
#if 1
	100;
#elif 1
	200;
#else
	300
#endif
			)").to_string("", root) + "\n\n");
			print(parser.Parse(R"(
#pragma once
#pragma unroll
for (;;){}

#pragma parallel
for (;;){}
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define TEST 100;
return TEST;
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define TEST 100;
#define TESTING(x) x + TEST;
TESTING(TEST);
#undef TEST
TESTING(TEST);
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define TEST 100;
#define TESTING(x) x + TEST;
if (TESTING(TEST)){
	10;
	#undef TEST	
}else{
	20;
}
TESTING(TEST);
			)").to_string("", root) + "\n\n");
			
			// this will not throw, because it can be compiled down to the final conclusion
			print(parser.Parse(R"(
#define factorial(n) ((n <= 1) ? 1ULL : (n * factorial(n - 1)))
constexpr auto x1 = factorial(1);
constexpr auto x2 = factorial(2);
constexpr auto x3 = factorial(5);
return x1 + x2 + x3;
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define factorial(n) ((n <= 1) ? 1ULL : (n * factorial(n - 1)))
constexpr auto x3 = factorial(10);
return x3;
			)").to_string("", root) + "\n\n");

			// this will throw, since the maximum recursion depth will be exceeded
			try {
				print(parser.Parse(R"(
#define factorial(n) ((n <= 1) ? 1ULL : (n * factorial(n)))
constexpr auto x1 = factorial(1);
constexpr auto x2 = factorial(2);
constexpr auto x3 = factorial(5);
return x1 + x2 + x3;
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			// this will throw, since the maximum recursion depth will be exceeded
			try {
				print(parser.Parse(R"(
#define factorial(n) ((n <= 1) ? 1ULL : (n * factorial(n - 1)))
constexpr auto x3 = factorial(50);
return x3;
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			// this will NOT throw, since the maximum recursion depth will not be exceeded and the result can be evaluated at compile-time. 
			try {
				print(parser.Parse(R"(
#define factorial(n) ((n <= 1) ? 1ULL : (n * factorial(n - 1)))
return factorial(50);
				)").to_string("", root) + "\n\n");
			}
			catch (std::exception& e) { print(e.what()); }
			
			// this will throw, since the maximum recursion depth will be exceeded
			try {
				print(parser.Parse(R"(
#define factorial(n) ((n <= 1) ? 1ULL : (n * factorial(n - 1)))
int x;
x = 50;
return factorial(x);
				)").to_string("", root) + "\n\n");
			}
			catch (std::exception& e) { print(e.what()); }

			// this will throw, since the replacement of TEST with TEST will go on forever.
			try {
				print(parser.Parse(R"(
#define TEST TEST;
#define TESTING(x) x + TEST;
return TESTING(TEST);
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			// this will throw, since the maximum recursion depth will be exceeded
			try {
				print(parser.Parse(R"(
#define TEST TEST + 1;
#define TESTING(x) x + TEST;
return TESTING(TEST);
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			
			print(parser.Parse(R"(
#define CONCAT1(a, b) a ## b
return CONCAT1(defer_, 200); // returns Id_Node{ "defer_200" }
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) #a
return CONCAT2(100); // returns "100"
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define PASS_THROUGH(a) a
#define CONCAT2(a) #a
constexpr auto x = PASS_THROUGH(100); // returns 100
return CONCAT2(x); // returns "x"
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) x + a + a##a
return CONCAT2(VAR);
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) x + #a + a##a
return CONCAT2(VAR);
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) #x + #a + a##a
return CONCAT2(VAR);
			)").to_string("", root) + "\n\n");

			print(parser.Parse(R"(
#define CONCAT2(a) #x + #a + #(a##a)
return CONCAT2(VAR);
			)").to_string("", root) + "\n\n");

			try {
				print(parser.Parse(R"(
return __DATE__ + "\t" + to_string(__LINE__) + "\t" + __TIME__;
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			try {
				print(parser.Parse(R"(
#define CalculateMetricPrefixV(metric) ((ldouble)std::metric::num / (ldouble)std::metric::den)
return CalculateMetricPrefixV(micro);
				)").to_string("", root) + "\n\n");
			}
			catch (std::exception& e) { print(e.what()); }

			try {
				print(parser.Parse(R"(
#define CalculateMetricPrefixV(metric) std##::##metric##::##num
return CalculateMetricPrefixV(micro);
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			try {
				print(parser.Parse(R"(
#define CalculateMetricPrefixV(metric) std:: ## metric ## ::num
return CalculateMetricPrefixV(micro);
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }
			
			try {
				print(parser.Parse(R"(
DerivedUnitTypeWithMetricPrefixes(meter, length, m, 1.0); 
DerivedUnitType(foot, length, ft, Conversion<meter>(381.0 / 1250.0));
DerivedUnitType(inch, length, in, Conversion<foot>(1.0 / 12.0));
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			try {
				print(parser.Parse(R"(
#define DerivedUnitList \
    DerivedUnitTypeWithMetricPrefixes(meter, length, m, 1.0); \
    DerivedUnitType(foot, length, ft, Conversion<meter>(381.0 / 1250.0)); \
	DerivedUnitType(inch, length, in, Conversion<foot>(1.0 / 12.0));

DerivedUnitList;
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			try {
				print(parser.Parse(R"(
#define DerivedUnitType(type, category, abbreviation, Ratio) \
	class type##category { \
		package unique_pkg() {}; \
		constexpr string abbrev = #abbreviation; \
		constexpr double conversion_ratio = Ratio; \
	}

DerivedUnitType(foot, length, ft, 381.0 / 1250.0);
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			try {
				print(parser.Parse(R"(
#define Conversion(From, ratio_of) (ratio_of) * (From##::conversion_ratio)
#define CalculateMetricPrefixV(metric) (((ldouble)(std::##metric##::num)) / ((ldouble)(std::##metric##::den)))

#define DerivedUnitList \
    DerivedUnitTypeWithMetricPrefixes(meter, length, m, 1.0); \
    DerivedUnitType(foot, length, ft, Conversion(meter, 381.0 / 1250.0)); \
	DerivedUnitType(inch, length, in, Conversion(foot, 1.0 / 12.0));

#define DerivedUnitType(type, category, abbreviation, Ratio) \
	class type { \
		package unique_pkg() {}; \
		constexpr string abbrev = #abbreviation; \
		constexpr double conversion_ratio = Ratio; \
	}

#define DerivedUnitTypeWithMetricPrefix(type, prefix, abbreviation, prefixAbbreviation) \
    class prefix##type { \
        package unique_pkg() {}; \
	    constexpr string abbrev = #prefixAbbreviation + #abbreviation; \
		constexpr double conversion_ratio = type##::conversion_ratio * CalculateMetricPrefixV(prefix); \
        void prefix##type() {}; \
	}

#define DerivedUnitTypeWithMetricPrefixes(type, category, abbreviation, ratio) \
    DerivedUnitType(type, category, abbreviation, ratio); \
	DerivedUnitTypeWithMetricPrefix(type, femto, abbreviation, f); \
	DerivedUnitTypeWithMetricPrefix(type, pico, abbreviation, p); \
	DerivedUnitTypeWithMetricPrefix(type, nano, abbreviation, n); \
	DerivedUnitTypeWithMetricPrefix(type, micro, abbreviation, u); \
	DerivedUnitTypeWithMetricPrefix(type, milli, abbreviation, m); \
	DerivedUnitTypeWithMetricPrefix(type, centi, abbreviation, c); \
	DerivedUnitTypeWithMetricPrefix(type, deci, abbreviation, d); \
	DerivedUnitTypeWithMetricPrefix(type, deca, abbreviation, da); \
	DerivedUnitTypeWithMetricPrefix(type, hecto, abbreviation, h); \
	DerivedUnitTypeWithMetricPrefix(type, kilo, abbreviation, k); \
	DerivedUnitTypeWithMetricPrefix(type, mega, abbreviation, M); \
	DerivedUnitTypeWithMetricPrefix(type, giga, abbreviation, G); \
	DerivedUnitTypeWithMetricPrefix(type, tera, abbreviation, T); \
	DerivedUnitTypeWithMetricPrefix(type, peta, abbreviation, P)

    DerivedUnitList;

	return petameter::abbrev;
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }
			
			// Moderately sized script test, confirming that we can quickly parse a larger text file. 
			try {
				print(parser.Parse(R"(
	try{
		constexpr cubic_foot_per_second QZERO = 1.e-6; // equiv. to 0 flow in CFS
		constexpr value PI = (double)constant::pi;
		constexpr value A1 = 1000.0 * PI;
		constexpr value A2 = 500.0 * 3.141592653589793238462643383279502884197169399375105820974944f;
		constexpr value A3 = 16.0 * 3.141592653589793238462643383279502884197169399375105820974944f;
		constexpr value A4 = 2.0 * 3.141592653589793238462643383279502884197169399375105820974944f;
		constexpr value A8 = 4.61841319859066668690e+00; // 5.74*(PI/4)^.9
		constexpr value A9 = -8.68588963806503655300e-01;  // -2/ln(10)
		constexpr value AA = -1.5634601348517065795e+00; // -2*.9*2/ln(10)
		constexpr value AB = 3.28895476345399058690e-03; // 5.74/(4000^.9)
		constexpr value AC = AA * AB;
		constexpr value CSMALL = 1.e-6;
		constexpr value CBIG = 1.e8;
		constexpr auto MAXERRS = 10;  // Max. input errors reported
		constexpr auto MAXCOUNT = 10; // Max. # of disconnected nodes listed
		constexpr long HASHTABLEMAXSIZE = 128000;
		constexpr auto ALLOC_BLOCK_SIZE = 64000;   /*(62*1024)*/
		constexpr auto NOTFOUND = 0;
		constexpr auto CODEVERSION = 20200;
		constexpr auto MAGICNUMBER = 516114521;
		constexpr auto ENGINE_VERSION = 201; // Used for binary hydraulics file
		constexpr auto EOFMARK = 0x1A; // Use 0x04 for UNIX systems
		constexpr auto MAXTITLE = 3;   // Max. # title lines
		constexpr auto TITLELEN = 79;  // Max. # characters in a title line
		constexpr auto MAXID = 51; //31;   // Max. # characters in ID name (this is very short! Want to fix, but would break current co-op with existing EPAnet files)
		constexpr auto MAXMSG = 255;  // Max. # characters in message text
		constexpr auto MAXLINE = 1024;   // Max. # characters read from input line
		constexpr auto MAXFNAME = 259;  // Max. # characters in file name
		constexpr auto MAXTOKS = 40;   // Max. items per line of input from INP files

		constexpr value FULL = 2;
		constexpr value BIG = 1.E10;
		constexpr value TINY = 1.E-6;
		constexpr value MISSING = -1.E-7;     // Missing value indicator // was -1.E-10, but was too small for constexpr math
		constexpr value DIFFUS = (1.3E-8)_sq_ft / 1_s;     // Diffusivity of chlorine (sq ft/sec)
		constexpr value VISCOS = (1.1E-5)_sq_ft / 1_s;     // Kinematic viscosity of water @ 20 deg C (sq ft/sec)
		constexpr value MINPDIFF = 0.1;        // PDA min. pressure difference (psi or m?)
		constexpr auto SEPSTR = " \t\n\r";  // Token separator characters (space, tab, new line, carriage return)
		constexpr value GPMperCFS = 1.0 / ((gallon_per_minute)1 / (cubic_foot_per_second)1);
		constexpr value AFDperCFS = 1.0 / ((acre_foot_per_day)1 / (cubic_foot_per_second)1);
		constexpr value MGDperCFS = 1.0 / (((million_gallon_per_day)1) / ((cubic_foot_per_second)1));
		constexpr value IMGDperCFS = 1.0 / (((imperial_million_gallon_per_day)1) / ((cubic_foot_per_second)1)); // was 0.5382; // Disagreement between units??
		constexpr value LPSperCFS = 1.0 / (((liter_per_second)1) / ((cubic_foot_per_second)1));
		constexpr value LPMperCFS = 1.0 / (((liter_per_minute)1) / ((cubic_foot_per_second)1));
		constexpr value CMHperCFS = 1.0 / (((cubic_meter_per_hour)1) / ((cubic_foot_per_second)1));
		constexpr value CMDperCFS = 1.0 / (((cubic_meter_per_day)1) / ((cubic_foot_per_second)1));
		constexpr value MLDperCFS = 1.0 / (((megaliter_per_day)1) / ((cubic_foot_per_second)1));
		constexpr value M3perFT3 = 1.0 / (((cubic_meter)1) / ((cubic_foot)1));
		constexpr value LperFT3 = 1.0 / (((liter)1) / ((cubic_foot)1));
		constexpr value MperFT = 1.0 / (((meter)1) / ((foot)1));
		constexpr value PSIperFT = 1.0 / (((pounds_per_square_inch)1) / ((head)1));
		constexpr value KPAperPSI = 1.0 / (((kilopascals)1) / ((pounds_per_square_inch)1));
		constexpr value KWperHP = 1.0 / (((kilowatt)1) / ((horsepower)1));
		constexpr value SECperDAY = 1.0 / ((second)1 / (day)1); 

		constexpr value MAXITER = 200;  // Default max. # hydraulic iterations
		constexpr value HACC = 0.001;    // Default hydraulics convergence ratio
		constexpr foot HTOL = 0.0005;   // Default hydraulic head tolerance (ft)
		constexpr cubic_foot_per_second QTOL = 0.0001;   // Default flow rate tolerance (cfs)
		constexpr value AGETOL = 0.01;   // Default water age tolerance (hrs)
		constexpr value CHEMTOL = 0.01;  // Default concentration tolerance
		constexpr value PAGESIZE = 0;    // Default uses no page breaks
		constexpr value SPGRAV = 1.0;    // Default specific gravity
		constexpr value EPUMP = 75;      // Default pump efficiency
		constexpr auto  DEFPATID = "1";    // Default demand pattern ID
		constexpr value RQTOL = 1E-7;    // Default low flow resistance tolerance
		constexpr value CHECKFREQ = 2;   // Default status check frequency
		constexpr value MAXCHECK = 10;   // Default # iterations for status checks
		constexpr value DAMPLIMIT = 0;   // Default damping threshold
		constexpr cubic_foot_per_second Q_STAGNANT = 0.005_gpm;     // 0.005 gpm = 1.114e-5 cfs

		print([
			QZERO, PI, A1, A2, A3, A4, A8, A9, AA, AB, AC, CSMALL, CBIG							
		]);
		print([
			EOFMARK, BIG, TINY, MISSING, DIFFUS, VISCOS, MINPDIFF, SEPSTR
		]);
		print([
			GPMperCFS, AFDperCFS, MGDperCFS, IMGDperCFS, LPSperCFS, LPMperCFS, CMHperCFS, CMDperCFS, MLDperCFS, M3perFT3, LperFT3, MperFT, PSIperFT, KPAperPSI, KWperHP, SECperDAY
		]);
		print([
			MAXITER, HACC, HTOL, QTOL, DEFPATID, RQTOL, CHECKFREQ, MAXCHECK, DAMPLIMIT, Q_STAGNANT
		]);

		int hydsolve(EN_Project const& pr, int& iter, value& relerr, HydraulicSimulationQuality simQuality)
			/*-------------------------------------------------------------------
			**  Input:   none
			**  Output:  *iter   = # of iterations to reach solution
			**           *relerr = convergence error in solution
			**           returns error code
			**  Purpose: solves network nodal equations for heads and flows
			**           using Todini's Gradient algorithm
			**
			**  Notes:   Status checks on CVs, pumps and pipes to tanks are made
			**           every CheckFreq iteration, up until MaxCheck iterations
			**           are reached. Status checks on control valves are made
			**           every iteration if DampLimit = 0 or only when the
			**           convergence error is at or below DampLimit. If DampLimit
			**           is > 0 then future computed flow changes are only 60% of
			**           their full value. A complete status check on all links
			**           is made when convergence is achieved. If convergence is
			**           not achieved in MaxIter trials and ExtraIter > 0 then
			**           another ExtraIter trials are made with no status changes
			**           made to any links and a warning message is generated.
			**
			**   This procedure calls linsolve() which appears in SMATRIX.C.
			**-----------------------------------------------------------------*/							
		{
			EN_Network const& net = pr.network;
			Hydraul& hyd = pr.hydraul;
			Smatrix& sm = hyd.smatrix;
			Report& rpt = pr.report;

			int    i;							// Node index
			int    errcode = 0;					// Node causing solution error
			int    nextcheck;					// Next status check trial
			int    maxtrials;					// Max. trials for convergence
			value  newerr;						// New convergence error
			int    valveChange;					// Valve status change flag
			int    statChange;					// Non-valve status change flag
			Hydbalance hydbal;					// Hydraulic balance errors
			cubic_foot_per_second fullDemand;   // Full demand for a node (cfs)

			// Initialize status checking & relaxation factor
			nextcheck = hyd.CheckFreq;
			hyd.RelaxFactor = 1.0;

			// Initialize convergence criteria and PDA results
			hydbal.maxheaderror = 0.0_ft;
			hydbal.maxflowchange = 0.0_cfs;
			hyd.DeficientNodes = 0;
			hyd.DemandReduction = 0.0;

			// Repeat iterations until convergence or trial limit is exceeded. (ExtraIter used to increase trials in case of status cycling.)
			if (((SCALER)rpt.Statflag) == FULL) writerelerr(pr, 0, 0);
			maxtrials = hyd.MaxIter;
			if (hyd.ExtraIter > 0) maxtrials += hyd.ExtraIter;
			iter = 1;
			while (iter <= maxtrials) {
				/* Compute coefficient matrices A & F and solve A*H = F
					where H = heads, A = Jacobian coeffs. derived from
					head loss gradients, & F = flow correction terms.
					Solution for H is returned in F from call to linsolve(). */
 				headlosscoeffs(pr); // parallelized
				matrixcoeffs(pr);
				errcode = smatrix_t::linsolve(sm, net.Njuncs);

				// Matrix ill-conditioning problem - if control valve causing problem, fix its status & continue, otherwise quit with no solution.
				if (errcode > 0) {
					if (badvalve(pr, sm.Order[errcode])) continue;
					else break;
				}

				// Update current solution. (Row[i] = row of solution matrix corresponding to node i)
				for (i = 1; i <= net.Njuncs; i++) {
					hyd.NodeHead[i] = sm.B_ft[sm.Row[i]];   // Update heads
				}

				newerr = newflows(pr, hydbal);             // Update flows
				relerr = newerr;

				// Write convergence error to status report if called for
				if (((SCALER)rpt.Statflag) == FULL) {
					writerelerr(pr, iter, relerr);
				}

				// Apply solution damping & check for change in valve status
				hyd.RelaxFactor = 1.0;
				valveChange = false;
				if (hyd.DampLimit > 0.0) {
					if (relerr <= hyd.DampLimit) {
						hyd.RelaxFactor = 0.6;
						valveChange = calc_and_set_prv_and_psv_status(pr);
					}
				}
				else {
					valveChange = calc_and_set_prv_and_psv_status(pr);
				}

				// Check for convergence
				if (hasconverged(pr, relerr, hydbal)) {
					// We have convergence - quit if we are into extra iterations
					if (iter > hyd.MaxIter) break;

					// Quit if no status changes occur
					statChange = false;
					if (valveChange)    statChange = true;
					if (linkstatus(pr)) statChange = true;
					if (pswitch(pr))    statChange = true;
					if (!statChange)    break;

					// We have a status change so continue the iterations
					nextcheck = iter + hyd.CheckFreq;
				}

				// No convergence yet - see if its time for a periodic status check  on pumps, CV's, and pipes connected to tank
				else if ((iter <= hyd.MaxCheck) && (iter == nextcheck)) {
					linkstatus(pr);
					nextcheck += hyd.CheckFreq;
				}
				iter++;
			}

			// Iterations ended - report any errors.
			if (errcode > 0) {
				writehyderr(pr, sm.Order[errcode]); // Ill-conditioned matrix error
				errcode = 110;
			}

			// Store actual junction outflow in NodeDemand & full demand in DemandFlow
			for (i = 1; i <= net.Njuncs; i++) {
#if 1
				fullDemand = hyd.NodeDemand[i];
				hyd.NodeDemand[i] = hyd.DemandFlow[i] + hyd.EmitterFlow[i];
				hyd.DemandFlow[i] = fullDemand;
#else
				#error We should not process this line, ever. 
#endif
			}

			// Save the simulation data for this timestep            
			SaveResultsForTimeStep(pr, simQuality);

			// Save convergence info
			hyd.RelativeError = relerr;
			hyd.MaxHeadError = hydbal.maxheaderror;
			hyd.MaxFlowChange = hydbal.maxflowchange;
			hyd.Iterations = iter;
			return errcode;
		};
		return hydsolve(a,b,c,d);
	}		
		)").to_string("", root) + "\n\n");
			}
			catch (std::exception& e) { print(e.what()); }
			













			try {
				print(parser.Parse(R"(
print(ONE_HUNDRED);

#define as_foot(x) (x)_ft
constexpr auto V = as_foot(100.0);
// #undef as_foot

#define print(x) x + "\"new_line\""
#define ONE_HUNDRED = 100

print(__DATE__); 
print(ONE_HUNDRED); 
print(V); 
print(__TIMESTAMP__);

#undef ONE_HUNDRED
#undef print

print(ONE_HUNDRED);

#define print(x) #x + ": " + x + "\"new_line\"";
print(ONE_HUNDRED);

for (DateTime i = __DATE__; i < __DATE__ + 365_d; ++i) print(i);
				)").to_string("", root) + "\n\n");
			} catch (std::exception& e) { print(e.what()); }

			try {
				print(parser.Parse(R"(
#if 1
	10 + 10;
#else
	20 + 20;
#endif
				)").to_string("", root) + "\n\n");
			}
			catch (std::exception& e) { print(e.what()); }


		}

	}

	if (1) {
		for (GL::string& Script : std::vector<GL::string>{
			R"(
				10_ft; // constexpr `foot`
			)",
			R"(
				(10_ft + 12_in) == (11_ft); // constexpr `bool`
			)",
			R"(
				return (100+12)_in; // constexpr `inch`
			)",
			R"(
				return (x+12)_in; // Postfix{ BinaryFoldRight{x; 12}; "in" };
			)",
            R"(
				return (x ? x : y) + 10; 
			)",
            R"(
				auto x = (y ? ::int() : ::double());
                auto x_1 = apple_banana_123(123, apple, banana);
			)",
			R"(
				auto x = vector<int>();
				auto y = map<int, double>();
			)",
			R"(
                auto z = map<int,vector<int>::iterator>();
			)",
			R"(
                return vector<int>::static_function();
			)",
			R"(
				return [](){ return 10; };
			)",
		    R"(
				return [](int x){ return 10 + x; };
			)",
			R"(
				return [](int x) -> int { return 10 + x; };
			)",
		    R"(
				return [](int x, Vector y) async -> Vector { return 10 + x; };
			)",
			R"(
				return [](int x, vector<int> y) async -> vector<int> { return 10 + x; };
			)",
			R"(
				for (int i = 0; i < 10; i++){
					return "i = ${ i }";
				}
			)",
			R"(
				int y = 50;
				return y - 10 + y - 10 + y;
			)",
			R"(
				int y;
				return "${ 0 / y }";
			)",
			R"(
				int y;
				return "${ y }";
			)",
			R"(
				int y;
				if (y == 0){
					return y;
				}
				else{
					return y + y;
				}
			)",
			R"(
#define assert(x) if (!(x)){ throw("Error at ${ __LINE__ }: " + #x); }
				var y;
				assert(true); // constexpr check will reduce down to a no-op
				assert(10 == 10); // constexpr check will reduce down to a no-op
				assert(10 == 9);
				assert(1); // constexpr check will reduce down to a no-op
				assert(y);
			)",
			R"(
				return foot(10).meter.float.int + 10;
			)",
			R"(
				return string("test").add_to_delim("a", " ");
			)",
			R"(
				return [];
			)",			
            R"(
				return [10, 10, 10];
			)",
			R"(
				return ["ten", 10, [10, "twenty"]];
			)",
			R"(
				return [10:10, 20:20, 30:30, 40:40];
			)",
			R"(
				return ["ten":10, 20:20, 30.0f:30.0l];
			)",
			R"(
				return ["ten":10.0f, 20:"twenty", 30:[3,0], ["forty", "fifty", "sixty"]:70];
			)"
		}) {
			GL::scope::impl::RootScope root;
			root.perform_builtins();
			GL::Engine::ScriptParser::Parser parser(root);

			print(Script);
			print(" >> ");
			GL::Engine2::Compiler::Preprocessor::PreprocessorState state;
			if (auto preprocessor_result = GL::Engine2::Compiler::Preprocessor().Parse(Script)) {
				preprocessor_result->GenerateExpandedCode(state);
				auto expanded_script = state.GetFinalScript();

				auto node = parser.Parse(expanded_script);
				print(node.to_string("\t", root));
			}
			print("");
		}
	}

	if (1) {
		GL::string Script = R"(
			int y = 10;

			10
            "10"
			"${ 10 + 10 }" // correctly reduces to "20"
            "${ 10 + 10 + y }" // correctly reduces to "${ 20 + y }"
            "start${ 10 + 10 + y }middle${ y * y }end"

            0 / 10; // 0 (correct)
            0 / 0; // -nan(ind) (correct?)
            y / 1; // y (correct)
            0 / y; // 0 (correct) 
            y + 0; // y (correct)
            0 + y; // y (correct)
            10 + y - 10; // y (correct)     

			y + 10 + 10 + 10 + 10; // y + 40 (correct)
			y - 10 + 10 - 10 + 10; // y + 0 (correct)
            y - 10 + y - 10 + y; // ((y+y)-20)+y ... correct, but not fully optimized. Would require looking even deeper into nodes than is typical.
			10 * y + 10 * y + 10 * y; // 30*y (correct)
			10 / y / 2; // 5 / y; (correct)
            10 / (10 / y); // y (correct)
            10 / (10 * y); // 1 / y (correct)
			
			if (y > 0) {
				int x = 10;
				x = 100;
				x * x + x;
				return 10 + x + 10;
				return 10;
			}
			if (true) {
				var x;
				x = 100;
				return x;				
			}else{}
			if (0 + 1) {
				auto x;
				x = 100;
				return x;
			}else{}
			if (1){
				for (int i = 0; i < 10; i++){
					return --i++;
				}
			}else{}
			if (!0){
				namespace Test{
					int StaticObject1 = 100;	
					int StaticObject2;	
					auto StaticObject3;	

					namespace Test2{
						class Class1 {
							int StaticObject1;
						}
					};
				};
			}else{}
		)";



		GL::Engine2::Compiler::Preprocessor::PreprocessorState state;
		if (auto preprocessor_result = GL::Engine2::Compiler::Preprocessor().Parse(Script)) {
			GL::scope::impl::RootScope root;
			root.perform_builtins();

			preprocessor_result->GenerateExpandedCode(state);
			auto expanded_script = state.GetFinalScript();
			print(expanded_script);

			GL::Engine::ScriptParser::Parser parser(root);
			auto node = parser.Parse(expanded_script);
			print(node.to_string("", root));
		}


	}




#if 0
    if (auto wb = cweeExcel::OpenExcel("S:\\Engineering\\Monthly Conservation Report\\Analysis File\\DemandSupplyShortage.xlsx")) {        
        if (auto ws = wb->active_sheet()) {
            print(ws->cell("A2")->value<GL::string>());
        }
    }
    if (auto wb = cweeExcel::OpenExcel("S:\\Engineering\\Monthly Conservation Report\\Analysis File\\Demand ProRating\\ProRating Calculator.xlsx")) {        
        for (int sheet_index = 0; sheet_index < wb->sheet_count(); ++sheet_index) {
            if (auto ws = wb->sheet_by_index(sheet_index)) {
                if (auto cell = ws->cell("A1")) {
                    auto str = cell->value<GL::string>();
                    print(str);
                }
            }
        }
    }
#endif

#if 0
    //auto func = [](int x) -> double { return x; };
    //typedef decltype(GL::details::detail::function_signature(&GL::string::length)) function_header;
    //function_header::

    GL::scope::impl::Functions funcs;
    funcs.add_function(GL::make_converter<GL::foot, GL::meter>());
    funcs.add_function(GL::make_converter<GL::meter, GL::foot>());
    funcs.add_function(GL::make_converter<GL::meter, GL::value>());
    funcs.add_function(GL::make_converter<GL::value, GL::meter>());
    funcs.add_function(GL::make_converter<int, double>());
    funcs.add_function(GL::make_converter<int, float>());
    funcs.add_function(GL::make_converter<int, long>());
    funcs.add_function(GL::make_converter<int const&, int>());
    funcs.add_function(GL::make_callable("type_name", [](GL::any const& any_type) -> GL::string { return any_type.m_casted_type.name(); }));
    funcs.add_function(GL::make_callable("type_name", [](GL::type const& any_type) -> GL::string { return any_type.name(); }));
    funcs.add_function(GL::make_callable("type_of", [](GL::any const& any_type) -> GL::type { return any_type.m_casted_type; }));
    funcs.add_function(GL::decl_func(&GL::string::length));
    funcs.add_function(GL::decl_func(&GL::string::capacity));
    funcs.add_function(GL::decl_func(&GL::string::clear));
    funcs.add_function(GL::decl_func(&GL::string::empty));

    funcs.for_each([](GL::Proxy_Function const& f) -> bool {
        print(f->m_signature.display());
        return false;
    });
    funcs.for_each("empty", [](GL::Proxy_Function const& f) -> bool {
        print(f->m_signature.display());
        return false;
    });
    if (1) {
        std::vector < GL::type > types{ GL::type_of<GL::string>() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
    if (1) {
        std::vector < GL::type > types{ GL::type_of<GL::string&>() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
    if (1) {
        std::vector < GL::type > types{ GL::type_of<GL::string const&>() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_EQ(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
    if (1) {
        std::vector < GL::type > types{ GL::type_of<GL::string const&>() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end()));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end()));
    }
    if (1) {
        std::vector < GL::any > types{ GL::any{ GL::string{} } };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
    if (1) {
        std::vector < GL::any::fast_any > types{ GL::any{ GL::string{} }.fast() };
        EXPECT_NE(nullptr, funcs.try_find_callable("length", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
        EXPECT_NE(nullptr, funcs.try_find_callable("clear", types.begin(), types.end(), GL::scope::impl::Functions::free_cast_only));
    }
#endif

#if 0
    if (0) {
        if (GL::stopwatch sw; auto x = sw.debug_timer("std_map 1")) {
            std::map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("std_map 2")) {
            std::map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 0.1")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000; ++i) {
                map[i] = i;
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 0.2")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000; ++i) {
                map.insert_fast(i, int{ i });
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 1.1")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {                
                map[i] = i;
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 1.2")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            auto iter = map.begin();
            auto e = map.end();
            while (iter != e) { ++iter; }
            for (auto& x : map) {}
            for (auto const& x : map) {}
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 2.1")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 2.2")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 3.1")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 3.2")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 4.1")) {
            GL::epoch_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 4.2")) {
            GL::epoch_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                map.insert_fast(i, int{ i });
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 5")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            for (int i = 0; i < 1000000; ++i) {
                map.erase(i);
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("epoch_map 6")) {
            GL::epoch_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map.insert_fast(i, int{ i });
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                map.erase(i);
            });
        }
        if (1) { // this feature is only possible with the epoch_map
            GL::epoch_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                auto ref = map.insert(i, (int)i);
                ref.second++;
                map.erase(i);
                ref.second++;
            });
        }

        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 0")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 1")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 1a")) {
            concurrency::concurrent_unordered_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 2")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 3")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 4")) {
            concurrency::concurrent_unordered_map<int, int> map;
            GL::parallel::For(0, 1000000, [&](int i) {
                map[i] = i;
            });
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 5")) {
            concurrency::concurrent_unordered_map<int, int> map;
            for (int i = 0; i < 1000000; ++i) {
                map[i] = i;
            }
            for (int i = 0; i < 1000000; ++i) {
                map.unsafe_erase(i);
            }
        }
        if (GL::stopwatch sw; auto x = sw.debug_timer("concurrent_unordered_map 6")) {
            concurrency::concurrent_unordered_map<int, int> map;
            std::mutex mut;
            for (int i = 0; i < 1000000; ++i) {
                mut.lock();
                map[i] = i;
                mut.unlock();
            }
            GL::parallel::For(0, 1000000, [&](int i) {
                mut.lock();
                map.unsafe_erase(i);
                mut.unlock();
            });
        }
    }
#endif

    std::thread test_thread([&]() {
        GL::parallel::For(0, 1000000, [&](size_t i) {});
        GL::parallel::Std_For(0, 1000000, [&](size_t i) {});

        GL::stopwatch sw;
        GL::stopwatch loop_sw;
        std::ios_base::sync_with_stdio(false);
        std::cin.tie(NULL);
        CONSOLE_SCREEN_BUFFER_INFO screen; GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

        while (true) {
            loop_sw.reset();

            EXPECT_EQ(GL::type_of<size_t>().is_cpp_type(), true);
            EXPECT_EQ(GL::type_of<size_t>().is_const(), false);
            EXPECT_EQ(GL::type_of<size_t>().is_ref(), false);
            EXPECT_EQ(GL::type_of<size_t>().is_const_ref(), false);

            EXPECT_EQ(GL::type_of<size_t&>().is_cpp_type(), true);
            EXPECT_EQ(GL::type_of<size_t&>().is_const(), false);
            EXPECT_EQ(GL::type_of<size_t&>().is_ref(), true);
            EXPECT_EQ(GL::type_of<size_t&>().is_const_ref(), false);

            EXPECT_EQ(GL::type_of<const size_t>().is_cpp_type(), true);
            EXPECT_EQ(GL::type_of<const size_t>().is_const(), true);
            EXPECT_EQ(GL::type_of<const size_t>().is_ref(), false);
            EXPECT_EQ(GL::type_of<const size_t>().is_const_ref(), false);

            EXPECT_EQ(GL::type_of<const size_t&>().is_cpp_type(), true);
            EXPECT_EQ(GL::type_of<const size_t&>().is_const(), true);
            EXPECT_EQ(GL::type_of<const size_t&>().is_ref(), true);
            EXPECT_EQ(GL::type_of<const size_t&>().is_const_ref(), true);

            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Const).get_hash(), GL::type_of<const size_t>().get_hash());
            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Const | GL::type::Reference).get_hash(), GL::type_of<const size_t&>().get_hash());
            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Temporary).get_hash(), GL::type_of<size_t&&>().get_hash());
            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Const | GL::type::Reference | GL::type::Temporary).get_hash(), (GL::type_of<size_t>() | GL::type::Const | GL::type::Temporary).get_hash());
            EXPECT_EQ((GL::type_of<size_t>() | GL::type::Reference | GL::type::Temporary).get_hash(), (GL::type_of<size_t const&>() | GL::type::Temporary).get_hash());
            EXPECT_EQ(GL::type_of<size_t&&>().is_cpp_type(), (GL::type_of<size_t const&>() | GL::type::Temporary).is_cpp_type());
            EXPECT_EQ(GL::type_of<size_t&&>().is_temp(), (GL::type_of<size_t const&>() | GL::type::Temporary).is_temp());
            EXPECT_EQ(GL::type_of<size_t&&>().is_const(), (GL::type_of<size_t const&>() | GL::type::Temporary).is_const());
            EXPECT_EQ(GL::type_of<size_t&&>().is_ref(), (GL::type_of<size_t const&>() | GL::type::Temporary).is_ref());

            while (1) {
                if (auto timer = sw.debug_timer("1 million scopes with 10 sub-scopes")) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    GL::parallel::For(0, 1000000, [&](size_t) {
                        auto a = program_root.make_scope();
                        auto b = a.make_scope();
                        auto c = b.make_scope();
                        auto d = c.make_scope();
                        auto e = d.make_scope();
                        auto f = e.make_scope();
                        auto g = f.make_scope();
                        auto h = g.make_scope();
                        auto i = h.make_scope();
                        auto j = i.make_scope();
                        auto k = j.make_scope();
                    });
                }
                if (auto timer = sw.debug_timer("example calc")) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    GL::parallel::For(0, 1000000, [&](size_t i) {
                        auto x0 = program_root.call("foot", { GL::any::fast_any::instance(100.0) });
                        auto v0 = program_root.call("/", {
                            program_root.call("foot", { GL::any::fast_any::instance(10) }),
                            program_root.call("second", { GL::any::fast_any::instance(1) })
                            });
                        auto a0 = program_root.call("/", {
                            v0,
                            program_root.call("second", { GL::any::fast_any::instance(1) })
                            });
                        auto t = program_root.call("second", { GL::any::fast_any::instance(5) });
                        auto d = program_root.call("+", {
                            program_root.call("*", {
                                v0,
                                t
                            }),
                            program_root.call("*", {
                                program_root.call("*", {
                                    program_root.call("pow", {
                                        t,
                                        GL::any::fast_any::instance(2)
                                    }),
                                    a0
                                }),
                                GL::any::fast_any::instance(0.5)
                            })
                            });
                        auto x = program_root.call("+", {
                            x0,
                            d
                            });
                        });
                }
                if (auto timer = sw.debug_timer("example calc 2")) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    GL::parallel::For(0, 1000000, [&](size_t i) {
                        auto temp_scope = program_root.make_scope();
                        temp_scope.insert_object_here("x0", temp_scope.call("foot", { GL::any::fast_any::instance(100.0) }));
                        temp_scope.insert_object_here("v0", temp_scope.call("/", {
                            temp_scope.call("foot", { GL::any::fast_any::instance(10) }),
                            temp_scope.call("second", { GL::any::fast_any::instance(1) })
                            }));
                        temp_scope.insert_object_here("a0", temp_scope.call("/", {
                            temp_scope.find_object("v0"),
                            temp_scope.call("second", { GL::any::fast_any::instance(1) })
                            }));
                        temp_scope.insert_object_here("t", temp_scope.call("second", { GL::any::fast_any::instance(5) }));
                        temp_scope.insert_object_here("d", temp_scope.call("+", {
                            temp_scope.call("*", {
                                temp_scope.find_object("v0"),
                                temp_scope.find_object("t")
                            }),
                            temp_scope.call("*", {
                                temp_scope.call("*", {
                                    temp_scope.call("pow", {
                                        temp_scope.find_object("t"),
                                        GL::any::fast_any::instance(2)
                                    }),
                                    temp_scope.find_object("a0")
                                }),
                                GL::any::fast_any::instance(0.5)
                            })
                            }));
                        temp_scope.insert_object_here("x", temp_scope.call("+", {
                            temp_scope.find_object("x0"),
                            temp_scope.find_object("d")
                            }));
                        });
                }
                if (auto timer = sw.debug_timer("example calc (C++ only, for the theoretical 'optimal' performance)")) {
                    GL::parallel::For(0, 1000000, [&](size_t i) {
                        using namespace GL::literals;
                        auto x0 = 100_ft;
                        auto v0 = 10_ft / 1_s;
                        auto a0 = v0 / 1_s;
                        auto t = 5_s;
                        auto d = (v0 * t) + ((t.pow(2) * a0) * 0.5);
                        auto x = x0 + d;
                    });
                }
                if (auto timer = sw.debug_timer("example calc 2 (once only)")) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    auto temp_scope = program_root.make_scope();
                    temp_scope.insert_object_here("x0", temp_scope.call("foot", { GL::any::fast_any::instance(100.0) }));
                    temp_scope.insert_object_here("v0", temp_scope.call("/", {
                        temp_scope.call("foot", { GL::any::fast_any::instance(10) }),
                        temp_scope.call("second", { GL::any::fast_any::instance(1) })
                        }));
                    temp_scope.insert_object_here("a0", temp_scope.call("/", {
                        temp_scope.find_object("v0"),
                        temp_scope.call("second", { GL::any::fast_any::instance(1) })
                        }));
                    temp_scope.insert_object_here("t", temp_scope.call("second", { GL::any::fast_any::instance(5) }));
                    temp_scope.insert_object_here("d", temp_scope.call("+", {
                        temp_scope.call("*", {
                            temp_scope.find_object("v0"),
                            temp_scope.find_object("t")
                        }),
                        temp_scope.call("*", {
                            temp_scope.call("*", {
                                temp_scope.call("pow", {
                                    temp_scope.find_object("t"),
                                    GL::any::fast_any::instance(2)
                                }),
                                temp_scope.find_object("a0")
                            }),
                            GL::any::fast_any::instance(0.5)
                        })
                        }));
                    temp_scope.insert_object_here("x", temp_scope.call("+", {
                        temp_scope.find_object("x0"),
                        temp_scope.find_object("d")
                        }));
                }
                if (auto timer = sw.debug_timer("example calc 2 (once only, from scratch)")) {
                    GL::scope::impl::RootScope
                        program;
                    program.perform_builtins();

                    auto temp_scope = program.make_scope();
                    temp_scope.insert_object_here("x0", temp_scope.call("foot", { GL::any::fast_any::instance(100.0) }));
                    temp_scope.insert_object_here("v0", temp_scope.call("/", {
                        temp_scope.call("foot", { GL::any::fast_any::instance(10) }),
                        temp_scope.call("second", { GL::any::fast_any::instance(1) })
                        }));
                    temp_scope.insert_object_here("a0", temp_scope.call("/", {
                        temp_scope.find_object("v0"),
                        temp_scope.call("second", { GL::any::fast_any::instance(1) })
                        }));
                    temp_scope.insert_object_here("t", temp_scope.call("second", { GL::any::fast_any::instance(5) }));
                    temp_scope.insert_object_here("d", temp_scope.call("+", {
                        temp_scope.call("*", {
                            temp_scope.find_object("v0"),
                            temp_scope.find_object("t")
                        }),
                        temp_scope.call("*", {
                            temp_scope.call("*", {
                                temp_scope.call("pow", {
                                    temp_scope.find_object("t"),
                                    GL::any::fast_any::instance(2)
                                }),
                                temp_scope.find_object("a0")
                            }),
                            GL::any::fast_any::instance(0.5)
                        })
                        }));
                    temp_scope.insert_object_here("x", temp_scope.call("+", {
                        temp_scope.find_object("x0"),
                        temp_scope.find_object("d")
                        }));
                }
                if (auto timer = sw.debug_timer("example calc 2 (sequence, not parallel)"); false) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();
                    for (size_t i = 0; i < 1000000; ++i) {
                        auto temp_scope = program_root.make_scope();
                        temp_scope.insert_object_here("x0", temp_scope.call("foot", { GL::any::fast_any::instance(100.0) }));
                        temp_scope.insert_object_here("v0", temp_scope.call("/", {
                            temp_scope.call("foot", { GL::any::fast_any::instance(10) }),
                            temp_scope.call("second", { GL::any::fast_any::instance(1) })
                            }));
                        temp_scope.insert_object_here("a0", temp_scope.call("/", {
                            temp_scope.find_object("v0"),
                            temp_scope.call("second", { GL::any::fast_any::instance(1) })
                            }));
                        temp_scope.insert_object_here("t", temp_scope.call("second", { GL::any::fast_any::instance(5) }));
                        temp_scope.insert_object_here("d", temp_scope.call("+", {
                            temp_scope.call("*", {
                                temp_scope.find_object("v0"),
                                temp_scope.find_object("t")
                            }),
                            temp_scope.call("*", {
                                temp_scope.call("*", {
                                    temp_scope.call("pow", {
                                        temp_scope.find_object("t"),
                                        GL::any::fast_any::instance(2)
                                    }),
                                    temp_scope.find_object("a0")
                                }),
                                GL::any::fast_any::instance(0.5)
                            })
                            }));
                        temp_scope.insert_object_here("x", temp_scope.call("+", {
                            temp_scope.find_object("x0"),
                            temp_scope.find_object("d")
                            }));
                    };
                }
                if (auto timer = sw.debug_timer("Polymorphism test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    // declare a custom namespace
                    auto& Example = root.make_namespace("Example");

                    // within that namespace is an Animal interface class
                    auto& Animal = Example.make_class("Animal");
                    auto Animal_t = Animal.this_type;
                    Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t } }, GL::type_of<std::string>()));

                    // within that namespace is an Dog impl class
                    auto& Dog = Example.make_class("Dog");
                    auto Dog_t = Dog.this_type;
                    Dog_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                    Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));

                    // within that namespace is an Cat impl class
                    auto& Cat = Example.make_class("Cat");
                    auto Cat_t = Cat.this_type;
                    Cat_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Cat_t));
                    Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));

                    if (1) {
                        auto script_scope = root.make_scope();

                        if (1) {
                            GL::any::fast_any dog_impl = GL::any::fast_any::instance(10);
                            dog_impl.m_casted_type = Dog_t;
                            script_scope.insert_object_here("dog_impl", dog_impl);
                        }
                        if (1) {
                            GL::any::fast_any cat_impl = GL::any::fast_any::instance(10);
                            cat_impl.m_casted_type = Cat_t;
                            script_scope.insert_object_here("cat_impl", cat_impl);
                        }
                        EXPECT_EQ("bark", script_scope.call<std::string>("speak", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("meow", script_scope.call<std::string>("speak", { script_scope.find_object("cat_impl") }));
                    }
                }
                if (auto timer = sw.debug_timer("Var tests")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    if (auto script_scope = root.make_scope()) {
                        GL::any::fast_any x = GL::any::fast_any::instance(100);
                        GL::any::fast_any y = GL::any::fast_any::instance(100);

                        script_scope.call("+=", { x, y });
                        EXPECT_EQ(x.cast<int>(), 200);
                    }

                    if (auto script_scope = root.make_scope()) {
                        GL::var x = GL::var(GL::make_shared<GL::any>(100));
                        GL::var y = GL::var(GL::make_shared<GL::any>(100));

                        EXPECT_EQ(x.get_type(), GL::type_of<int>());

                        script_scope.call("+=", { x.get_data()->fast(), y.get_data()->fast() });
                        EXPECT_EQ(x.get_data()->cast<int>(), 200);

                        script_scope.call("+=", { x.get_data()->fast(), y.get_data()->fast() });
                        EXPECT_EQ(x.get_data()->cast<int>(), 300);
                    }

                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>(100)));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(100)));

                        script_scope.call("+=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 200);
                    }

                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>(0)));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(100)));

                        script_scope.call("=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 100);
                    }

                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>(0)));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(100)));
                        script_scope.call("=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 100);
                    }
                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>()));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(100)));
                        script_scope.call("=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 100);
                    }
                    if (auto script_scope = root.make_scope()) {
                        script_scope.insert_object_here("x", GL::var(GL::make_shared<GL::any>(100)));
                        script_scope.insert_object_here("y", GL::var(GL::make_shared<GL::any>(200)));
                        script_scope.insert_object_here("z", GL::var(GL::make_shared<GL::any>()));
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 100);

                        script_scope.call("=", { script_scope.find_object("x"), script_scope.find_object("y") });
                        EXPECT_EQ(script_scope.find_object("x").cast<int>(), 200);

                        EXPECT_EQ(true, script_scope.find_object("x").can_cast(GL::type_of<GL::var&>()));

                        script_scope.call(":=", { script_scope.find_object("x"), script_scope.find_object("z") });
                        EXPECT_EQ(script_scope.find_object("x").m_casted_type, GL::type_of<GL::var>());
                    }
                    if (auto script_scope = root.make_scope()) {
                        auto empty_var = script_scope.call("var", {  });
                        auto initialized_var = script_scope.call("var", { GL::any::fast_any::instance(100) });
                        auto copied_var = script_scope.call("var", { initialized_var });
                        auto assigned_var = script_scope.call("=", { script_scope.call("var", {  }), initialized_var });

                        EXPECT_EQ(empty_var.m_casted_type, GL::type_of<GL::var>());
                        EXPECT_EQ(initialized_var.m_casted_type, GL::type_of<int>());
                        EXPECT_EQ(copied_var.m_casted_type, GL::type_of<int>());
                        EXPECT_EQ(assigned_var.m_casted_type, GL::type_of<int&>());
                        EXPECT_EQ(initialized_var.cast<int>(), 100);
                        EXPECT_EQ(copied_var.cast<int>(), 100);
                        EXPECT_EQ(assigned_var.cast<int>(), 100);

                        script_scope.call("+=", { initialized_var, GL::any::fast_any::instance(25) });
                        script_scope.call("+=", { copied_var, GL::any::fast_any::instance(25) });
                        script_scope.call("+=", { assigned_var, GL::any::fast_any::instance(25) });

                        EXPECT_EQ(initialized_var.cast<int>(), 175);
                        EXPECT_EQ(copied_var.cast<int>(), 175);
                        EXPECT_EQ(assigned_var.cast<int>(), 175);

                        // handling type-changes when keeping variables locally...
                        assigned_var = script_scope.call(":=", { assigned_var, GL::any::fast_any::instance(std::string("TEST"))});
                        EXPECT_EQ(assigned_var.cast<std::string>(), "TEST");
                        EXPECT_EQ(script_scope.call< GL::string>("type_name", { assigned_var }), "string");

                        // type-changes are automatically handled when handled 100% in-script. 
                        script_scope.emplace_object_here("x", GL::var(GL::make_shared<GL::any>(100.0f)));
                        EXPECT_EQ(script_scope.call< GL::string>("type_name", { script_scope.find_object("x") }), "float");
                        script_scope.call(":=", { script_scope.find_object("x"), GL::any::fast_any::instance(std::string("TEST")) });
                        EXPECT_EQ(script_scope.call< GL::string>("type_name", { script_scope.find_object("x") }), "string");
                    }
                }
                if (auto timer = sw.debug_timer("Polymorphism var test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    // declare a custom namespace
                    auto& Example = root.make_namespace("Example");

                    // within that namespace is an Animal interface class
                    auto& Animal = Example.make_class("Animal");
                    auto Animal_t = Animal.this_type;
                    Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t } }, GL::type_of<std::string>()));

                    // within that namespace is an Dog impl class
                    auto& Dog = Example.make_class("Dog");
                    auto Dog_t = Dog.this_type;
                    Dog_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                    Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));
                    Dog.add_function(GL::make_callable(Dog_t.name(), [Dog_t]() -> GL::any::fast_any {
                        GL::any::fast_any out = GL::any::fast_any::instance(10);
                        out.m_casted_type = Dog_t;
                        return out;
                        }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Dog_t));

                    // within that namespace is an Cat impl class
                    auto& Cat = Example.make_class("Cat");
                    auto Cat_t = Cat.this_type;
                    Cat_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Cat_t));
                    Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));
                    Cat.add_function(GL::make_callable(Cat_t.name(), [Cat_t]() -> GL::any::fast_any {
                        GL::any::fast_any out = GL::any::fast_any::instance(10);
                        out.m_casted_type = Cat_t;
                        return out;
                        }, GL::function_signature::Constructor | GL::function_signature::Async, {}, {}, Cat_t));

                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("dog_impl", script_scope.call("var", { script_scope.call("Dog", {  }) }));
                        script_scope.insert_object_here("cat_impl", script_scope.call("var", { script_scope.call("Cat", {  }) }));
                        EXPECT_EQ("bark", script_scope.call<std::string>("speak", { script_scope.call("dog_impl",{}) }));
                        EXPECT_EQ("meow", script_scope.call<std::string>("speak", { script_scope.call("cat_impl",{}) }));

                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                        EXPECT_EQ(script_scope.call<bool>("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }), true);

                        // To-Do, test for polymorphism with the casted-down type, having lost its identity. 
                        //auto found_impl = script_scope.find_object("dog_impl");
                        //found_impl.m_casted_type = Animal_t;
                        //print(script_scope.call<std::string>("speak", { found_impl }));
                    }


                }
                if (auto timer = sw.debug_timer("Polymorphism dynamic_object var test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string>(), GL::type_of<GL::string>(), false));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string>(), GL::type_of<GL::string>(), true));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string const&>(), GL::type_of<GL::string>(), false));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string const&>(), GL::type_of<GL::string>(), true));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string const&>(), GL::type_of<GL::string const&>(), false));
                    EXPECT_EQ(true, root.can_convert(GL::type_of<std::string const&>(), GL::type_of<GL::string const&>(), true));

                    // declare a custom namespace
                    auto& Example = root.make_namespace("Example");

                    // class Animal {
                    //      bool is_pet = true;
                    //      value& counter = value(0);
                    //      std::string speak() { return "unspecified"; };
                    // };
                    auto& Animal = Example.make_class("Animal");                
                    auto Animal_t = Animal.this_type;                
                    Animal.add_member_object("is_pet", GL::type_of<bool>(), GL::any::fast_any::instance(bool{ true }));                
                    Animal.add_member_object("counter", GL::type_of<GL::value&>(), /*Example.call("reference_cast", { */GL::any::fast_any::instance(GL::value(0.0f)) /*})*/);
                    Animal.initialize_basic_member_functions();                
                    Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t | GL::type::Reference } }, GL::type_of<std::string>()));
                   
                    // class Dog : Animal { // inherits the member objects and functions from Animal
                    //      std::string name = "Ozzy";
                    //      double weight = 24.0;
                    //      std::string speak() { return "bark"; };
                    // };
                    auto& Dog = Example.make_class("Dog");
                    auto Dog_t = Dog.this_type;
                    Dog_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Dog_t));
                    Dog.add_member_object("name", GL::type_of<std::string>(), GL::any::fast_any::instance(std::string("Ozzy")));
                    Dog.add_member_object("weight", GL::type_of<double>(), GL::any::fast_any::instance(24.0));                
                    Dog.initialize_basic_member_functions();
                    Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t | GL::type::Reference } }, GL::type_of<std::string>()));

                    // class Cat : Animal { // inherits the member objects and functions from Animal
                    //      std::string name = "Goosie";
                    //      std::string speak() { return "meow"; }; 
                    // };
                    auto& Cat = Example.make_class("Cat");
                    auto Cat_t = Cat.this_type;
                    Cat_t.add_base(Animal_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Cat_t));
                    Cat.add_member_object("name", GL::type_of<std::string>(), GL::any::fast_any::instance(std::string("Goosie")));
                    Cat.initialize_basic_member_functions();
                    Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t | GL::type::Reference } }, GL::type_of<std::string>()));

                    // class Lion : Cat, Dog { // inherits the member objects and functions from Cat, Dog, and (implied) Animal. Order matters with inheritance. 
                    //      std::string speak() { return "MEOW"; }; 
                    // };
                    auto& Lion = Example.make_class("Lion");
                    auto Lion_t = Lion.this_type;
                    Lion_t.add_base(Cat_t);
                    Lion_t.add_base(Dog_t);
                    EXPECT_EQ(true, Animal_t.is_base_of(Lion_t));
                    EXPECT_EQ(true, Dog_t.is_base_of(Lion_t));
                    EXPECT_EQ(true, Cat_t.is_base_of(Lion_t));
                    Lion.initialize_basic_member_functions();
                    Lion.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "MEOW"; }, 0, {}, { { "rhs", Lion_t | GL::type::Reference } }, GL::type_of<std::string>()));

                    // normal
                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("dog_impl", script_scope.call("Dog", {}));
                        script_scope.insert_object_here("cat_impl", script_scope.call("Cat", {}));
                        EXPECT_EQ("bark", script_scope.call<std::string>("speak", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("meow", script_scope.call<std::string>("speak", { script_scope.find_object("cat_impl") }));

                        EXPECT_EQ(4, script_scope.call<size_t>("length", { script_scope.call("name", {script_scope.find_object("dog_impl")}) }));
                        EXPECT_EQ("Ozzy", script_scope.call<std::string&>("name", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("Goosie", script_scope.call<std::string&>("name", { script_scope.find_object("cat_impl") }));
                        EXPECT_EQ(true, script_scope.call<bool>("is_pet", { script_scope.find_object("cat_impl") }));

                        EXPECT_EQ(24.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_impl") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(100.0) });
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_impl") }));

                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                        EXPECT_EQ(script_scope.call<bool>("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }), true);

                        script_scope.insert_object_here("talk_to", GL::make_callable("", [NearestNS = script_scope.GetNamespace()](GL::any::fast_any rhs) {
                            auto temp_scope = NearestNS->make_scope();
                            return temp_scope.call("speak", { rhs });
                        }, 0, {}, { { "", Animal_t | GL::type::Reference | GL::type::Const } }));
                        EXPECT_EQ(script_scope.call<std::string>("talk_to", { script_scope.find_object("dog_impl") }), "bark");
                        EXPECT_EQ(script_scope.call<std::string>("talk_to", { script_scope.find_object("cat_impl") }), "meow");
                    }

                    // as `var`
                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("dog_impl", script_scope.call("var", { script_scope.call("Dog", {  }) }));
                        script_scope.insert_object_here("cat_impl", script_scope.call("var", { script_scope.call("Cat", {  }) }));
                        EXPECT_EQ("bark", script_scope.call<std::string>("speak", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("meow", script_scope.call<std::string>("speak", { script_scope.find_object("cat_impl") }));

                        EXPECT_EQ("Ozzy", script_scope.call<std::string&>("name", { script_scope.find_object("dog_impl") }));
                        EXPECT_EQ("Goosie", script_scope.call<std::string&>("name", { script_scope.find_object("cat_impl") }));
                        EXPECT_EQ(true, script_scope.call<bool>("is_pet", { script_scope.find_object("cat_impl") }));

                        EXPECT_EQ(24.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_impl") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(100.0) });
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_impl") }));

                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("dog_impl") }).cast < GL::type>(), Dog_t);
                        EXPECT_EQ(script_scope.call("type_of", { script_scope.find_object("cat_impl") }).cast < GL::type>(), Cat_t);
                        EXPECT_EQ(script_scope.call<bool>("is_derived_from", { script_scope.call("type_of", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(Animal_t) }), true);

                        script_scope.insert_object_here("talk_to", GL::make_callable("", [NearestNS = script_scope.GetNamespace()](GL::any::fast_any rhs) {
                            auto temp_scope = NearestNS->make_scope();
                            return temp_scope.call("speak", { rhs });
                        }, 0, {}, { { "", Animal_t | GL::type::Reference | GL::type::Const } }));
                        EXPECT_EQ(script_scope.call<std::string>("talk_to", { script_scope.find_object("dog_impl") }), "bark");
                        EXPECT_EQ(script_scope.call<std::string>("talk_to", { script_scope.find_object("cat_impl") }), "meow");
                    }

                    // test assignment and copy constructors
                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("dog_1", script_scope.call("Dog", {}));
                        EXPECT_EQ(24.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_1") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_1") }), GL::any::fast_any::instance(100.0) });
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_1") }));

                        script_scope.insert_object_here("dog_2", script_scope.call("Dog", {}));
                        EXPECT_EQ(24.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_2") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_2") }), GL::any::fast_any::instance(200.0) });
                        EXPECT_EQ(200.0, script_scope.call<double>("weight", { script_scope.find_object("dog_2") }));

                        script_scope.insert_object_here("dog_3", script_scope.call("Dog", { script_scope.find_object("dog_1") }));
                        EXPECT_EQ(100.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_3") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_3") }), GL::any::fast_any::instance(300.0) });
                        EXPECT_EQ(300.0, script_scope.call<double>("weight", { script_scope.find_object("dog_3") }));
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_1") }));

                        script_scope.insert_object_here("dog_4", script_scope.call("Dog", {}));
                        script_scope.call("=", { script_scope.find_object("dog_4"), script_scope.find_object("dog_1") });
                        EXPECT_EQ(100.0, script_scope.call<double&>("weight", { script_scope.find_object("dog_4") }));
                        script_scope.call("=", { script_scope.call("weight", { script_scope.find_object("dog_4") }), GL::any::fast_any::instance(400.0) });
                        EXPECT_EQ(400.0, script_scope.call<double>("weight", { script_scope.find_object("dog_4") }));
                        EXPECT_EQ(100.0, script_scope.call<double>("weight", { script_scope.find_object("dog_1") }));
                    }
                
                    // test a reference-type member object...
                    if (1) {
                        int i = 1000000;
                        GL::parallel::For(0, i, [&root](int) {
                            auto script_scope = root.make_scope();
                            script_scope.insert_object_here("dog_impl", script_scope.call("Dog", {}));
                            script_scope.call("+=", { script_scope.call("counter", { script_scope.find_object("dog_impl") }), GL::any::fast_any::instance(5) });
                        });
                        if (1) {
                            auto script_scope = root.make_scope();
                            script_scope.insert_object_here("dog_impl", script_scope.call("Dog", {}));
                            EXPECT_EQ((float)(i * 5), (float)script_scope.call<GL::value>("counter", {script_scope.find_object("dog_impl")}));
                        }
                    };

                    // test the double-inheritor of Lion...
                    if (1) {
                        auto script_scope = root.make_scope();
                        script_scope.insert_object_here("lion_impl", script_scope.call("Example::Lion", {}));
                        EXPECT_EQ("Goosie", script_scope.call<std::string&>("name", { script_scope.find_object("lion_impl") }));
                        EXPECT_EQ("MEOW", script_scope.call<std::string>("speak", { script_scope.find_object("lion_impl") }));
                        EXPECT_EQ(true, script_scope.call<bool>("is_pet", { script_scope.find_object("lion_impl") }));

                    }
                }
                if (auto timer = sw.debug_timer("Templated var test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    // declare a custom namespace
                    auto& Shapes = root.make_namespace("Shapes"); {
                        auto& Circle = Shapes.make_class("Circle"); {
                            Circle.add_member_object("radius", GL::type_of<GL::foot>());
                            Circle.add_function(GL::make_callable("area", [&Circle](GL::any::fast_any lhs) -> GL::any::fast_any {
                                auto scope = Circle.GetRoot()->make_scope();
                                return scope.call("square_foot", { scope.call("*", {scope.call("double", { scope.call("constants::pi", {}) }), scope.call("pow", {scope.call("radius", {lhs}), GL::any::fast_any::instance(2)})}) });
                            }, GL::function_signature::Constant, {}, { { "lhs", Circle.this_type + GL::type::Const + GL::type::Reference } }, GL::type_of<GL::square_foot>()));
                            Circle.initialize_basic_member_functions();
                        }
                        auto& Square = Shapes.make_class("Square"); {
                            Square.add_member_object("side", GL::type_of<GL::foot>());
                            Square.add_function(GL::make_callable("area", [&Square](GL::any::fast_any lhs) -> GL::any::fast_any {
                                auto scope = Square.GetRoot()->make_scope();
                                return scope.call("square_foot", { scope.call("pow", {scope.call("side", {lhs}), GL::any::fast_any::instance(2)}) });
                            }, GL::function_signature::Constant, {}, { { "lhs", Square.this_type + GL::type::Const + GL::type::Reference } }, GL::type_of<GL::square_foot>()));
                            Square.initialize_basic_member_functions();
                        }
                        auto& Rectangle = Shapes.make_class("Rectangle"); {
                            Rectangle.add_member_object("width", GL::type_of<GL::foot>());
                            Rectangle.add_member_object("height", GL::type_of<GL::foot>());
                            Rectangle.add_function(GL::make_callable("area", [&Rectangle](GL::any::fast_any lhs) -> GL::any::fast_any {
                                auto scope = Rectangle.GetRoot()->make_scope();
                                return scope.call("square_foot", { scope.call("*", { scope.call("width", {lhs}), scope.call("height", {lhs}) }) });
                            }, GL::function_signature::Constant, {}, { { "lhs", Rectangle.this_type + GL::type::Const + GL::type::Reference } }, GL::type_of<GL::square_foot>()));
                            Rectangle.initialize_basic_member_functions();
                        }
                    }

                    auto& Shape = root.make_class("Shape"); {
                        Shape.template_types = { { "Which", GL::is_template::type<0>("Which")} };
                        Shape.add_function(GL::make_callable("area", [&Shape](GL::any::fast_any lhs) -> GL::any::fast_any {
                            auto scope = Shape.GetRoot()->make_scope();
                            return scope.call("area", { lhs });
                        }, GL::function_signature::Static, {}, { { "lhs", GL::is_template::type<0>("Which") + GL::type::Const + GL::type::Reference }}, GL::type_of<GL::square_foot>()));
                        Shape.initialize_basic_member_functions();
                    }

                    auto Cir = root.call("Shapes::Circle", {});
                    root.call("=", { root.call("radius", {Cir}), GL::any::fast_any::instance(1) });
                    EXPECT_EQ(root.call<GL::square_foot>("Shape<Shapes::Circle>::area", { Cir }), GL::square_foot((float)GL::constants::pi()));
                }
                if (auto timer = sw.debug_timer("Competing Class Name(s) test")) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    EXPECT_EQ(root.call("string", {}).m_casted_type, GL::type_of<GL::string>());
                    if (auto& std_ns = root.make_namespace("std")) {
                        EXPECT_EQ(std_ns.call("string", {}).m_casted_type, GL::type_of<std::string>());
                        if (auto scope = std_ns.make_scope()) {
                            EXPECT_EQ(scope.call("string", {}).m_casted_type, GL::type_of<std::string>());
                        }
                    }
                    EXPECT_EQ(root.call("std::string", {}).m_casted_type, GL::type_of<std::string>());
                    if (auto scope = root.make_scope()) {
                        EXPECT_EQ(scope.call("string", {}).m_casted_type, GL::type_of<GL::string>());
                        if (auto& std_ns = root.make_namespace("std")) {
                            scope.add_using_here(std_ns);
                            EXPECT_EQ(scope.call("string", {}).m_casted_type, GL::type_of<std::string>());
                            if (auto scope2 = scope.make_scope()) {
                                EXPECT_EQ(scope2.call("string", {}).m_casted_type, GL::type_of<std::string>());
                            }
                        }
                    }

                    EXPECT_EQ(root.find_object("string::npos").cast<size_t>(), GL::string::npos);
                    EXPECT_EQ(root.find_object("::string::npos").cast<size_t>(), GL::string::npos);
                    if (auto& std_ns = root.make_namespace("std")) {
                        try {
                            // will crash, since it searches "string" for the object but fails to find it. 
                            EXPECT_EQ(std_ns.find_object("string::npos").cast<size_t>(), GL::string::npos);
                        }
                        catch (...) {}
                        EXPECT_EQ(std_ns.find_object("::string::npos").cast<size_t>(), GL::string::npos);
                    }

                    if (auto& std_ns = root.make_namespace("std")) {
                        // std::pair<T0,T1>
                        if (1) {
                            auto& BaseClass = std_ns.make_class("pair");
                            BaseClass.template_types = { { "T0",GL::is_template::type<0>("T0") }, { "T1",GL::is_template::type<1>("T1") } };
                            BaseClass.add_member_object("first", GL::is_template::type<0>("T0"));
                            BaseClass.add_member_object("second", GL::is_template::type<1>("T1"));
                            BaseClass.initialize_basic_member_functions();
                        }
                        EXPECT_EQ("::pair<int,int>::", dynamic_cast<GL::scope::impl::NamespaceScope*>(root.try_find_class(root.call("pair<int, int>", {}).m_casted_type)->this_m.scope)->path());
                        EXPECT_EQ("::std::pair<int,int>::", dynamic_cast<GL::scope::impl::NamespaceScope*>(root.try_find_class(std_ns.call("pair<int, int>", {}).m_casted_type)->this_m.scope)->path());
                        EXPECT_EQ("::std::pair<int,int>::", dynamic_cast<GL::scope::impl::NamespaceScope*>(root.try_find_class(root.call("std::pair<int, int>", {}).m_casted_type)->this_m.scope)->path());
                        EXPECT_EQ("::std::pair<int,int>::", dynamic_cast<GL::scope::impl::NamespaceScope*>(root.try_find_class(std_ns.call("std::pair<int, int>", {}).m_casted_type)->this_m.scope)->path());
                    }

                }
                if (auto timer = sw.debug_timer("Template classes")) {
                    GL::scope::impl::RootScope 
                        root;
                    root.perform_builtins();

                    if (1) {
                        // ensure that a bad request doesn't hang
                        if (auto this_scope = root.make_scope()) {
                            EXPECT_NE(this_scope.DetermineType("pair"), GL::type_of<GL::undefined>());
                            EXPECT_NE(this_scope.DetermineType("pair<int, int>"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair<int, >"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair<int, "), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair<int, int"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair int, int"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair::int,::int"), GL::type_of<GL::undefined>());
                            EXPECT_EQ(this_scope.DetermineType("pair +="), GL::type_of<GL::undefined>());
                        }
                    }
                    
                    // pair<T0, T1> template class
                    if (1) {
                        if (auto this_scope = root.make_scope()) {
                            auto Pair = this_scope.call("pair<int, string>", {});
                            auto first = this_scope.call("first", { Pair });
                            auto second = this_scope.call("second", { Pair });
                            EXPECT_EQ(false, Pair.m_casted_type.is_cpp_type());
                            EXPECT_EQ(true, first.m_casted_type.is_cpp_type());
                            EXPECT_EQ(true, second.m_casted_type.is_cpp_type());
							EXPECT_EQ(first.m_casted_type, GL::type_of<int&>());
							EXPECT_EQ(second.m_casted_type, GL::type_of<GL::string&>());
                        }
                        if (auto this_scope = root.make_scope()) {
                            auto Pair = this_scope.call("pair<int, pair<int, pair<int, string>>>", {});
                            auto first = this_scope.call("first", { Pair });
                            auto second = this_scope.call("second", { Pair });
							EXPECT_EQ(false, second.m_casted_type.is_cpp_type());
							EXPECT_EQ(false, second.m_casted_type.is_template());
							EXPECT_EQ(true, second.m_casted_type.is_ref());
							EXPECT_EQ(false, second.m_casted_type.is_const());
                            EXPECT_EQ(false, Pair.m_casted_type.is_cpp_type());
                            EXPECT_EQ(true, first.m_casted_type.is_cpp_type());
                            EXPECT_EQ(false, second.m_casted_type.is_cpp_type());
							EXPECT_EQ(first.m_casted_type, GL::type_of<int&>());
							EXPECT_EQ(first.m_casted_type.is_ref(), true);
							EXPECT_EQ(second.m_casted_type.is_ref(), true);
							EXPECT_EQ(second.m_casted_type.get_base_hash(), (this_scope.DetermineType("pair<int, pair<int, string>>") | GL::type::Reference).get_base_hash());
							EXPECT_EQ(second.m_casted_type, (this_scope.DetermineType("pair<int, pair<int, string>>") | GL::type::Reference));
							EXPECT_NE(second.m_casted_type, this_scope.DetermineType("pair<int, pair<int, string>>"));
							print(second.m_casted_type.name());
                        }
                        if (auto this_scope = root.make_scope()) {
                            auto Pair = this_scope.call("pair<vector<int>,pair<int,vector<int>>>", {});
                            auto first = this_scope.call("first", { Pair });
                            auto second = this_scope.call("second", { Pair });
                            EXPECT_EQ(false, Pair.m_casted_type.is_cpp_type());
                            EXPECT_EQ(false, first.m_casted_type.is_cpp_type());
                            EXPECT_EQ(false, second.m_casted_type.is_cpp_type());
							EXPECT_EQ(first.m_casted_type, (this_scope.DetermineType("vector<int>") | GL::type::Reference));
							EXPECT_EQ(second.m_casted_type, (this_scope.DetermineType("pair<int,vector<int>>") | GL::type::Reference));
							print(first.m_casted_type.name());
							print(second.m_casted_type.name());
                        }						
                    }

                    // vector<T0> template class
                    if (1) { 
                        /*
                        auto vec = vector<int>();
                        vec[0] = 10;
                        */
                        if (auto this_scope = root.make_scope()) { 
                            auto vec = this_scope.call("vector<int>", {}); // calling this forcefully initializes the template class, even if it was never initialized before.
                            // EXPECT_EQ(vec.m_casted_type, Class.this_type);

                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(0) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(1) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(2) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(3) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(4) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(5) });
                        
                            print(this_scope.call<size_t>("size", { vec }));
                            print(this_scope.call<int>("[]", { vec, GL::any::fast_any::instance(5) }));
                        
                            print(this_scope.call<GL::string>("to_string", { vec }));
                            print(this_scope.call<size_t>("to_hash", { vec }));

                            this_scope.call("=", { this_scope.call("[]", { vec, GL::any::fast_any::instance(5) }), GL::any::fast_any::instance(50) });
                            print(this_scope.call<int>("[]", { vec, GL::any::fast_any::instance(5) }));
                            print(this_scope.call<GL::string>("to_string", { vec }));

                            this_scope.call("grow_to_at_least", { vec, GL::any::fast_any::instance(10) });
                            print(this_scope.call<GL::string>("to_string", { vec }));

                            for (
                                auto iterator = this_scope.call("begin", { vec }), end = this_scope.call("end", { vec }); 
                                this_scope.call<bool>("!=", { iterator, end }); 
                                this_scope.call("++", { iterator })) 
                            {
                                print(this_scope.call<GL::string>("to_string", { this_scope.call("get", { iterator }) }));
                            }



                            auto iterator = this_scope.call("begin", { vec });
                            print(iterator.m_casted_type.name());
                            print(this_scope.call<GL::string>("to_string", { iterator }));
                            this_scope.call("++", { iterator });
                            print(this_scope.call<GL::string>("to_string", { iterator }));
                            print(this_scope.call<GL::string>("to_string", { this_scope.call("get", { iterator }) }));



                        }
                        if (auto this_scope = root.make_scope()) {                        
                            auto vec = this_scope.call("vector< :: foot>", {}); // calling this forcefully initializes the template class, even if it was never initialized before.
                            // EXPECT_EQ(vec.m_casted_type, Class.this_type);

                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(0) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(1) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(2) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(3) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(4) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(5) });

                            print(this_scope.call<size_t>("size", { vec }));
                            print(this_scope.call<GL::foot>("[]", { vec, GL::any::fast_any::instance(5) }));

                            print(this_scope.call<GL::string>("to_string", { vec }));
                            print(this_scope.call<size_t>("to_hash", { vec }));

                            this_scope.call("=", { this_scope.call("[]", { vec, GL::any::fast_any::instance(5) }), GL::any::fast_any::instance(50) });
                            print(this_scope.call<GL::foot>("[]", { vec, GL::any::fast_any::instance(5) }));
                            print(this_scope.call<GL::string>("to_string", { vec }));

                            this_scope.call("grow_to_at_least", { vec, GL::any::fast_any::instance(10) });
                            print(this_scope.call<GL::string>("to_string", { vec }));
                        }
                        if (auto this_scope = root.make_scope()) {
                            auto vec = this_scope.call("vector<var>", {}); // calling this forcefully initializes the template class, even if it was never initialized before.
                            // EXPECT_EQ(vec.m_casted_type, Class.this_type);

                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(0) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(1) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(2) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(3) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(4) });
                            this_scope.call("push_back", { vec, GL::any::fast_any::instance(5) });

                            print(this_scope.call<size_t>("size", { vec }));
                            print(this_scope.call<GL::string>("to_string", { vec }));
                            print(this_scope.call<size_t>("to_hash", { vec }));

                            this_scope.call("+=", { this_scope.call("[]", { vec, GL::any::fast_any::instance(5) }), GL::any::fast_any::instance(50) });
                            print(this_scope.call<GL::string>("to_string", { vec }));

                            this_scope.call("grow_to_at_least", { vec, GL::any::fast_any::instance(10) });
                            print(this_scope.call<GL::string>("to_string", { vec }));
                        }
                    }

                    // map<T0, T1> template class
                    if (1) {                         
                        // At no point does C++ code instantiate "map<int,value>" -- this happens automatically by even attempting to use or search for it.
                        if (1) {
                            auto vec = root.call("map<int,value>", {});
                            GL::parallel::For(0, 1000000, [&root, &vec](int i) {
                                if (auto this_scope = root.make_scope()) {
                                    auto vec_obj = this_scope.call("[]", { vec, GL::any::fast_any::instance(i % 100) }); // creates a GL::value in the map and returns it
                                    EXPECT_EQ(vec_obj.m_casted_type, GL::type_of<GL::value&>());
                                    this_scope.call("=", { vec_obj, GL::any::fast_any::instance(i % 100) });

                                    auto vec_obj_2 = this_scope.call("[]", { vec, GL::any::fast_any::instance(i % 100) });
                                    EXPECT_EQ(true, this_scope.call<bool>("==", { vec_obj_2, GL::any::fast_any::instance(i % 100) }));
                                }
                            });
                            print(root.call<GL::string>("to_string", { vec }));
                            print(root.call<size_t>("to_hash", { vec }));


                            for (
                                auto iterator = root.call("begin", { vec }), end = root.call("end", { vec });
                                root.call<bool>("!=", { iterator, end });
                                root.call("++", { iterator }))
                            {
                                print(root.call<GL::string>("to_string", { root.call("get", { iterator }) }));
                            }


                        }
                        // Shockingly, map<var,var> worked flawlessly right out of the gate. 
                        // This includes even calling to_string and to_hash on the entire map! Very cool. 
                        if (1) {
                            auto vec = root.call("map<var,var>", {});
                            try{
                                GL::parallel::For(0, 1000000, [&root, &vec](int i) {
                                    if (auto this_scope = root.make_scope()) {
                                        auto vec_obj = this_scope.call("[]", { vec, GL::any::fast_any::instance(GL::foot((float)(i % 100))) }); // creates a GL::value in the map and returns it
                                        switch (i % 3) {
                                        case 0:
                                            this_scope.call("=", { vec_obj, GL::any::fast_any::instance(GL::foot((float)(i % 100))) });
                                            break;
                                        case 1:
                                            this_scope.call("=", { vec_obj, GL::any::fast_any::instance(GL::meter((float)(i % 100))) });
                                            break;
                                        case 2:
                                            this_scope.call("=", { vec_obj, GL::any::fast_any::instance(GL::inch((float)(i % 100))) });
                                            break;
                                        }
                                    }
                                });
                            }
                            catch (std::exception& e) {
                                print(e.what());
                            }
                            print(root.call<GL::string>("to_string", { vec }));
                            print(root.call<size_t>("to_hash", { vec }));

                            for (
                                auto iterator = root.call("begin", { vec }), end = root.call("end", { vec });
                                root.call<bool>("!=", { iterator, end });
                                root.call("++", { iterator }))
                            {
                                print(root.call<GL::string>("to_string", { root.call("get", { iterator }) }));
                            }
                        }

                    }

                    /* // during construction of an implimentation for a template class (e.g. test<int, double>) it should attempt to automatically resolve any sub-types.  
					template <T0, T1> class test{
					    pair<T0,T1> 
							my_pair;
						pair<T0,vector<int>> 
							another_pair;
						pair<T0,vector<T1>> 
							yet_another_pair;

						auto make_pair() const {
							return pair<T0,T1>();
						};
					}; */
                    if (1) {
                        auto& BaseClass = root.make_class("test");
                        BaseClass.template_types = { { "T0", GL::is_template::type<0>("T0") }, { "T1", GL::is_template::type<1>("T1") } };
                        BaseClass.add_member_object("my_pair", BaseClass.DetermineType("pair<T0,T1>"));
                        BaseClass.add_member_object("another_pair", BaseClass.DetermineType("pair<T0,vector<int>>"));
                        BaseClass.add_member_object("yet_another_pair", BaseClass.DetermineType("pair<T0,vector<T1>>"));
                        BaseClass.add_function(GL::make_callable("make_pair", [](GL::any::fast_any rhs) -> GL::any::fast_any {
                            if (auto* Class = GL::scope::GetClass(rhs.m_casted_type)) {
                                return Class->call("pair<T0,T1>", {});
                            }
                            return GL::scope::GetCurrentCaller()->call("pair<T0,T1>");
                        }));
                        BaseClass.initialize_basic_member_functions();

                        auto Pair = root.call("test<int, double>", {});
                        //for (auto& x : dynamic_cast<GL::scope::impl::ClassScope*>(root.try_find_class(Pair.m_casted_type)->this_m.scope)->template_types) {
                        //    print(x.first);
                        //    print(x.second.name());
                        //}
                        // dynamic_cast<GL::scope::impl::ClassScope*>(root.try_find_class(Pair.m_casted_type)->this_m.scope)->DetermineType("pair<T0,T1>");
                        print(root.call<GL::string>("to_string", { root.call("make_pair", { Pair }) }));
						print(root.call("my_pair", { Pair }).m_casted_type.name()); // pair<{0}, {1}>
						print(root.call("another_pair", { Pair }).m_casted_type.name()); // pair<{0}, {1}>
						print(root.call("yet_another_pair", { Pair }).m_casted_type.name()); // pair<{0}, {1}>
						print(root.call<GL::string>("to_string", { root.call("my_pair", { Pair }) }));
                        
                    }

					///*
					//template<T0> class outter {
					//	template<T0> class inner {
					//		static T0 instance(){
					//			return T0();
					//		};
					//	}
					//}
					//*/
					//if (1) {
					//	auto& outter = root.make_class("outter");
					//	outter.template_types = { { "T0", GL::is_template::type<0>("T0") }};
					//	auto& inner = root.make_class("inner");
					//	// inner.template_types = { { "T0", GL::is_template::type<0>("T0") } };
					//	inner.add_function(GL::make_callable("instance", [](GL::any::fast_any const& parent) -> GL::any::fast_any {
					//		if (auto* Class = GL::scope::GetClass(parent.m_casted_type)) {								
					//			if (auto* templateClass = GL::scope::GetClass(Class->template_types[0].second)) {
					//				return templateClass->call(templateClass->this_type.name());
					//			}
					//		}
					//		throw std::runtime_error("Could not trace back");
					//	}, GL::function_signature::Static, {}, { { "", inner.this_type | GL::type::Const | GL::type::Reference } }, GL::is_template::type<0>("T0")));
					//	root.call("outter<int>::inner::instance");
					//}

                    EXPECT_EQ("undefined", root.DetermineType("{0}").name());
					EXPECT_EQ("undefined", root.DetermineType("pair<{0}, {1}>").name());
                }
                if (auto timer = sw.debug_timer("GPU matrix test"); true) {
                    GL::scope::impl::RootScope
                        root;
                    root.perform_builtins();

                    //GL::parallel::For(0, 1000000, [&root](int) {
                        /*
                        auto state = float_matrix::random(20, 20, 1) > 0.4; // will be a uint_matrix
                        auto kernel = float_matrix::constant(1.0f, 3, 3, 1); // will be a float_matrix
                        kernel.write()[4] = 0; // ownership of the writer is guarranteed to follow with the float& accessor
                        for (;;) {
                            auto nHood = float_matrix(state).convolve(kernel);
                            auto C0 = (nHood == 2);
                            auto C1 = (nHood == 3);
                            state *= C0.cast<unsigned int>();
                            state += C1.cast<unsigned int>();
                            print(state.ASCII());
                        }
                        */
                        if (auto this_scope = root.make_scope()) {
                            GL::stopwatch sw;
                            std::deque<float> framerates;
                            std::ios_base::sync_with_stdio(false);
                            std::cin.tie(NULL);
                            CONSOLE_SCREEN_BUFFER_INFO screen; GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
                            CONSOLE_CURSOR_INFO cursorInfo;
                            GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
                            cursorInfo.bVisible = false;
                            SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
                            int game_w = screen.dwSize.X / 2, game_h = screen.dwSize.Y - 3;

                            this_scope.emplace_object_here("state", this_scope.call(">", { this_scope.call("float_matrix::random", { GL::any::fast_any::instance(game_h), GL::any::fast_any::instance(game_w), GL::any::fast_any::instance(1) }), GL::any::fast_any::instance(0.4) }));

                            auto kernel = this_scope.call("float_matrix::constant", { GL::any::fast_any::instance(1.0f), GL::any::fast_any::instance(3), GL::any::fast_any::instance(3), GL::any::fast_any::instance(1) });
                            this_scope.call("=", { this_scope.call("[]", { this_scope.call("write", { kernel }), GL::any::fast_any::instance(4) }), GL::any::fast_any::instance(0.0f) });
                        
                            for (;;) {
                                GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
                                int game_w2 = (screen.dwSize.X / 2), game_h2 = ((screen.dwSize.Y > 3) ? screen.dwSize.Y - 3 : 1);
                                if (game_w2 != game_w || game_h != game_h2) {
                                    game_w = game_w2;
                                    game_h = game_h2;

                                    this_scope.call("=", { this_scope.find_object("state"), this_scope.call("resize", { this_scope.find_object("state"), GL::any::fast_any::instance(game_h), GL::any::fast_any::instance(game_w), GL::any::fast_any::instance(1) }) });
                                    framerates.clear();
                                }

                                sw.reset();
                                if (auto for_scope = this_scope.make_scope()) {
                                    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });
                                    for_scope.emplace_object_here("nHood", for_scope.call("convolve", { for_scope.call("float_matrix", { for_scope.find_object("state") }), kernel }));
                                    for_scope.emplace_object_here("C0", for_scope.call("==", { for_scope.find_object("nHood"), GL::any::fast_any::instance(2) }));
                                    for_scope.emplace_object_here("C1", for_scope.call("==", { for_scope.find_object("nHood"), GL::any::fast_any::instance(3) }));
                                    for_scope.call("*=", { for_scope.find_object("state"), for_scope.find_object("C0") });
                                    for_scope.call("+=", { for_scope.find_object("state"), for_scope.find_object("C1") });
                                    print(this_scope.call<GL::string>("to_string", { for_scope.call("ASCII", { for_scope.find_object("state") }) }));

                                    // auto mat = this_scope.call("float_matrix::random", { GL::any::fast_any::instance(game_h), GL::any::fast_any::instance(game_w), GL::any::fast_any::instance(1) });
                                    // this_scope.emplace_object_here("state", this_scope.call(">", { mat, GL::any::fast_any::instance(0.4) }));

                                    //if (for_scope.call<bool>("<", { for_scope.call("avg", {for_scope.call("float_matrix", {for_scope.find_object("state")})}), GL::any::fast_any::instance(0.1) })) {
                                    //    for_scope.call("+=", { for_scope.find_object("state"), this_scope.call(">", { this_scope.call("float_matrix::random", { GL::any::fast_any::instance(game_h), GL::any::fast_any::instance(game_w), GL::any::fast_any::instance(1) }), GL::any::fast_any::instance(0.4) }) });
                                    //}
                                }

                                auto this_frame = (float)(1.0 / sw.stop());
                                framerates.push_back(this_frame);

                                if (framerates.size() > 10000) framerates.pop_front();
                                std::deque<float> copy(framerates);
                                std::sort(copy.begin(), copy.end());
                                float q0 = 0;
                                float q1 = 0;
                                float q2 = 0;
                                float q3 = 0;
                                float q4 = 0;
                                if (copy.size() >= 4) {
                                    q0 = copy.at(0);
                                    q1 = copy.at(copy.size() / 4);
                                    q2 = copy.at(2 * copy.size() / 4);
                                    q3 = copy.at(3 * copy.size() / 4);
                                    q4 = copy.at(copy.size() - 1);
                                }

                                print(GL::printf("min{ %f }  q1{ %f }  median{ %f }  q2{ %f }  max{ %f }  ", q0, q1, q2, q3, q4) + GL::arena_memory_pool::debug() + "         \t");

                                std::cout << std::flush;
                                while (sw.stop() < (1.0 / 60.0)) {
                                    std::this_thread::yield();
                                }
                            }

                        }
                    //});
                }

                // At this point, it is a full replacement of the "Source.cpp" file and its content. It not only re-impliments everything in there, but appears to be faster, principally leak-free, and easier to use.
                // Some very big wins include:
                //  - the type system being atomic and supporting scripted types as well as C++ types, 
                //  - the units system being smaller & faster to compile, being atomic, and supporting pre-compiled as well as runtime typing, 
                //  - the atomic_shared_ptr being a working version of std::atomic<shared_ptr>, 
                //  - the lock-free fundamental types, including trees, maps, and vectors,
                //  - the parallel CPU computing tools, which support rapid and easy deployment of hundreds to millions of parallel jobs,
                //  - the parallel GPU computing tools, which are still in their infancy and require additional work, but exist as an excellent proof-of-concept,
                //  - the scope system, which supports classes, namespaces, and local scopes, and allows for: 
                //      -  objects, functions, static objects, namespaces and classes, template classes, template functions, dynamic and static typing (with automatic shortest-path type-casting including for const-ness, reference-ness, and more), and polymorphism.
                if (auto timer = sw.debug_timer("Examples"); true) {
                    GL::scope::impl::RootScope program_root;
                    program_root.perform_builtins();

                    // local objects, destroyed when out-of-scope
                    if (auto scope = program_root.make_scope()) {
                        scope.insert_object_here("x", 100); // literal
                        scope.insert_object_here("y", GL::make_shared<int>(100)); // GL::shared
                        scope.insert_object_here("z", std::make_shared<int>(100)); // std::shared
                        scope.insert_object_here("w", GL::any::ref(std::string::npos)); // reference to static object
                    }
                    // static objects, destroyed when the root is destroyed.
                    if (auto& scope = program_root.make_namespace("std")) {
                        scope.insert_object_here("x", 100); // literal
                        scope.insert_object_here("y", GL::make_shared<int>(100)); // GL::shared
                        scope.insert_object_here("z", std::make_shared<int>(100)); // std::shared
                        scope.insert_object_here("w", GL::any::ref(std::string::npos)); // reference to static object
                    }
                    // functions
                    if (auto& scope = program_root.make_namespace("std")) {
                        scope.add_function(GL::make_callable("foo", []() {})); // static lambda function, no return
                        scope.add_function(GL::make_callable("bar", [](int) -> int { return 0; })); // static lambda function, returns
                        scope.add_function(GL::make_callable("zip", [](int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) -> int { return 0; })); // static lambda function, returns, up to 16 input arguments.
                        scope.add_function(GL::make_callable("empty_string", &GL::string::empty_string)); // static class function (only recommended when not overloaded)
                        scope.add_function(GL::make_callable("length", &GL::string::length)); // instanced class function (only recommended when not overloaded)
                        scope.add_function(GL::decl_func(&GL::string::length)); // easy-to-use instanced class function (only recommended when not overloaded)
                        scope.add_function(GL::make_callable("foo", [](int x) -> int { return x + 1; }, { 0 })); // provide default argument values if not provided when called. Defaults are shifted to the end.
                        scope.add_function(GL::make_callable("foo", [](int x, int y) -> int { return x + y; }, { 0 })); // Defaults are shifted to the end. E.g in this example, y is given a default.
                        scope.add_function(GL::make_callable("foo", [](int x, int y, int z) -> int { return x + y + z; }, { 0, 0 })); // Defaults are shifted to the end. E.g in this example, y and z are given defaults.
                        scope.add_function(GL::make_callable("bar", [](int const& rhs) -> int { return 0; }, 0, {}, { { "rhs", GL::type_of<int>() | GL::type::Const | GL::type::Reference } }, GL::type_of<int>())); // you can override the return types and input types, and also provide the argument names...
                        scope.add_function(GL::make_callable("bar", [](GL::any::fast_any const& rhs) -> int { return rhs.cast<int>(); }, 0, {}, { { "rhs", GL::type_of<int>() | GL::type::Const | GL::type::Reference } }, GL::type_of<int>())); // ... which is necessary when creating a function that accepts a non-c++ type or where you want to recieve the "wrapper" for the type. 
                        scope.add_function(GL::make_callable("++", [](GL::any::fast_any const& rhs) -> GL::any::fast_any { ++rhs.cast<int>(); return rhs; }, 0, {}, { { "rhs", GL::type_of<int>() | GL::type::Const | GL::type::Reference } }, GL::type_of<int const&>())); // Does this is also necessary when the intent is to return a reference-type while passing along the wrapper for the object, to ensure the lifetime protection is not lost.                        

                        // note that GL::any is an atomic object, while GL::any::fast_any is non-atomic. fast_any is better for most use-cases, but GL::any is necessary in containers or objects whose underlying value (or type) can change frequently.
                    }
                    // namespaces and classes
                    if (1) {
                        auto& std_namespace = program_root.make_namespace("std");
                        auto& std_string_namespace = program_root.make_namespace("::std::string"); // note that "::" is allowed ahead of any namespace or class specifier, hinting that the search should start at the root.
                        auto& std_string_class = std_namespace.make_class("string"); // note that std_string_class and std_string_namespace will point to the exact same place -- they are the same object!
                        auto& std_string_class_2 = std_namespace.make_class(GL::type_of<std::string>()); // note that std_string_class_2 and std_string_class will point to the exact same place -- they are the same object!
                        // however, the "std" namespace is NOT known from the C++ side -- that must be engrained in the script language through the scope system. 
                        if (auto* BC = program_root.try_find_class(GL::type_of<int>()); BC) {
                            auto& int_class = *dynamic_cast<GL::scope::impl::ClassScope*>(BC->this_m.scope);                            
                        }

                        auto& foobar_class = std_namespace.make_class("foobar"); // creates a class with the following path: "::std::foobar". 
                        foobar_class.add_member_object("int_member", GL::type_of<int>(), GL::any::fast_any::instance(0)); // adds a member object to foobar
                        // note that any members should 100% be added BEFORE initialize_basic_member_functions is called. 
                        foobar_class.initialize_basic_member_functions(); // if the class has never been initialized before, then do so, to give it the basic building-block functions such as: 
                        //                                                   "foobar()", "foobar(foobar const&)", and "foobar& operator=(foobar const&)"                        
                    }
                    // template classes
                    if (1) {
                        auto& Vector_class = program_root.make_class("Vector"); // this is the base class. 
                        Vector_class.template_types = { { "T0", GL::is_template::type<0>("T0") }}; // this action suddenly declares that it is available as a template base to exactly one parameter type.
                        Vector_class.add_member_object("int_member", GL::type_of<int>(), GL::any::fast_any::instance(0)); // this type is always going to be an int, regardless of the template class type.
                        Vector_class.add_member_object("dynamic_member", GL::is_template::type<0>("T0")); // this type is dependant on the template class type.
                        Vector_class.insert_object_here("static_member", []() -> GL::any::fast_any { GL::any::fast_any out; out.m_casted_type = GL::is_template::type<0>("T0"); return out; }()); // this is a static class object with a dynamic type
                        // note that any members should 100% be added BEFORE initialize_basic_member_functions is called. 
                        Vector_class.initialize_basic_member_functions();

                        // Instancing of template types can be done from anywhere that "searches" for things in the scopes. The following will result in the instantiation of the requested class:
                        program_root.find_namespace("Vector<string>");
                        program_root.DetermineType("Vector<string>");
                        program_root.ParsePossiblyScopedName("Vector<string>"); 
                        program_root.call("Vector<string>", {}); // will return a Vector<string>{}
                        program_root.call("dynamic_member", { program_root.call("Vector<string>", {}) }); // will return a GL::string&
                        program_root.find_object("::Vector<string>::static_member"); // will return a GL::string&
                        program_root.call("::Vector<string>::static_member"); // will return a GL::string&
                    }
                    // template functions
                    if (auto& scope = program_root.make_namespace("std")) {
                        scope.add_function(GL::make_callable("zip", [](GL::any::fast_any const& rhs) -> void { print("I was called"); })); // You can also leave the type un-specified to keep the function as a template -- it will accept any type provided to it. 
                        scope.add_function(GL::make_callable("zap", [](GL::any::fast_any const& rhs1, int rhs2, std::shared_ptr<int> rhs3) -> void { print("I was called"); })); // You can mix-and-match types or template arguments as necessary.
                    }
                    // dynamic and static typing
                    if (auto& scope = program_root.make_namespace("std")) {
                        scope.cast<int>(GL::any::fast_any::instance(100)); 
                        scope.cast<int&>(GL::any::fast_any::instance(100));
                        scope.cast<int const&>(GL::any::fast_any::instance(100));

                        scope.cast<double>(GL::any::fast_any::instance(100));
                        scope.cast<double&>(GL::any::fast_any::instance(100));
                        scope.cast<double const&>(GL::any::fast_any::instance(100));

                        scope.cast<GL::foot>(GL::any::fast_any::instance(100));
                        scope.cast<GL::foot&>(GL::any::fast_any::instance(100));
                        scope.cast<GL::foot const&>(GL::any::fast_any::instance(100));                        

                        // due to the type system, the following are all different functions and will be looked-up based on cast rules:
                        scope.add_function(GL::make_callable("foo", [](int&) -> int { return 0; })); 
                        scope.add_function(GL::make_callable("bar", [](int const&) -> int { return 0; }));
                        scope.add_function(GL::make_callable("zip", [](GL::any::fast_any rhs) -> int { return 0; }, 0, {}, { { "rhs", GL::type_of<int&&>()}}, GL::type_of<int>()));
                        scope.add_function(GL::make_callable("zap", [](int) -> int { return 0; }));
                    }
                    // polymorphism
                    if (auto scope = program_root.make_scope()) {
                        auto& Example = scope.GetNamespace()->make_namespace("Example");

                        // within that namespace is an Animal interface class
                        auto& Animal = Example.make_class("Animal");
                        auto Animal_t = Animal.this_type;
                        Animal.add_member_object("is_pet", GL::type_of<bool>(), GL::any::fast_any::instance(bool{ true }));
                        Animal.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "unspecified"; }, 0, {}, { { "rhs", Animal_t } }, GL::type_of<std::string>()));                        
                        Animal.initialize_basic_member_functions();

                        // within that namespace is an Dog impl class
                        auto& Dog = Example.make_class("Dog");
                        auto Dog_t = Dog.this_type;
                        Dog_t.add_base(Animal_t); // note that we are specifying "Animal_t" as the base to Dog_t
                        Dog.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "bark"; }, 0, {}, { { "rhs", Dog_t } }, GL::type_of<std::string>()));
                        Dog.initialize_basic_member_functions();

                        // within that namespace is an Cat impl class
                        auto& Cat = Example.make_class("Cat");
                        auto Cat_t = Cat.this_type;
                        Cat_t.add_base(Animal_t); // note that we are specifying "Animal_t" as the base to Cat_t
                        Cat.add_function(GL::make_callable("speak", [](GL::any::fast_any rhs) -> std::string { return "meow"; }, 0, {}, { { "rhs", Cat_t } }, GL::type_of<std::string>()));
                        Cat.initialize_basic_member_functions();
                    
                        // Cat and Dog inherit from Animal automatically once you specify the base. initialize_basic_member_functions will take care of the rest.                         
                        EXPECT_EQ(program_root.call< std::string >("speak", { program_root.call("::Example::Cat", {}) }), "meow");
                        EXPECT_EQ(program_root.call< bool >("is_pet", { program_root.call("::Example::Dog", {}) }), true);

                        // Cat and Dog will override the "speak" function, even when called in such a way that "forgets" the real static type: 
                        Example.add_function(GL::make_callable("talk_to_animal", [](GL::any::fast_any rhs) -> std::string { return GL::scope::GetCurrentCaller()->GetRoot()->call<std::string>("speak", { rhs }); }, GL::function_signature::Static, {}, { {"rhs", Animal_t | GL::type::Const | GL::type::Reference} }, GL::type_of<std::string>()));
                        EXPECT_EQ(program_root.call<std::string>("::Example::talk_to_animal", { program_root.call("::Example::Cat", {}) }), "meow");
                    }
                }

            }
        }

    });

#if 0
    // Conway's Game of Life, using the GPU. Many times faster than previous approach. From 20-30 fps to 1000-1800 fps. 
    if (1) {
        // reduces the size requirement of the arena memory pool. In exchange though, the largest single allocation is reduced to this same number. Application-dependant decision. 
        // GL::GPU::matrix<float>::maximum_allocation_size() /= 16; // = 1; // /= 16; // 16
        while (0) {
            GL::stopwatch sw;
#if 0
            if (1) { // if (auto timer = sw.debug_timer("Std Linear Allocator (int)")) {
                for (size_t i = 0; i < 1000000; ++i) {
                    auto a = std::make_unique<int>();     // value of 0
                    auto b = std::make_unique<int[]>(10000); // 100 "ints"
                    auto c = std::make_shared<int>();     // value of 0
                    auto d = std::shared_ptr<int[]>(new int[10000], [](int* p) { delete[] p; }); // 100 "ints"
                    ::free(::malloc(sizeof(int) * 10000));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Parallel Allocator (int)")) {
                GL::parallel::For(0, 1000000, [](size_t) {
                    auto a = std::make_unique<int>();     // value of 0
                    auto b = std::make_unique<int[]>(10000); // 100 "ints"
                    auto c = std::make_shared<int>();     // value of 0
                    auto d = std::shared_ptr<int[]>(new int[10000], [](int* p) { delete[] p; }); // 100 "ints"
                    ::free(::malloc(sizeof(int) * 10000));
                    });
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Linear Random Allocator (int)")) {
                for (size_t i = 0; i < 1000; ++i) {
                    auto a = std::make_unique<int>();     // value of 0
                    auto b = std::make_unique<int[]>(i + 1000); // 100 "ints"
                    auto c = std::make_shared<int>();     // value of 0
                    auto d = std::shared_ptr<int[]>(new int[i + 1000], [](int* p) { delete[] p; }); // 100 "ints"
                    ::free(::malloc(sizeof(int) * (i + 1000)));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Parallel Random Allocator (int)")) {
                GL::parallel::For(0, 1000, [](size_t i) {
                    auto a = std::make_unique<int>();     // value of 0
                    auto b = std::make_unique<int[]>(i + 1000); // 100 "ints"
                    auto c = std::make_shared<int>();     // value of 0
                    auto d = std::shared_ptr<int[]>(new int[i + 1000], [](int* p) { delete[] p; }); // 100 "ints"
                    ::free(::malloc(sizeof(int) * (i + 1000)));
                    });
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Linear Allocator (std::string)")) {
                for (size_t i = 0; i < 1000000; ++i) {
                    auto a = std::make_unique<std::string>();     // value of 0
                    auto b = std::make_unique<std::string[]>(100); // 100 "std::strings"
                    auto c = std::make_shared<std::string>();     // value of 0
                    auto d = std::shared_ptr<std::string[]>(new std::string[100], [](std::string* p) { delete[] p; }); // 100 "std::strings"
                    ::free(::malloc(sizeof(std::string)*100));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Parallel Allocator (std::string)")) {
                GL::parallel::For(0, 1000000, [](size_t) {
                    auto a = std::make_unique<std::string>();     // value of 0
                    auto b = std::make_unique<std::string[]>(100); // 100 "std::strings"
                    auto c = std::make_shared<std::string>();     // value of 0
                    auto d = std::shared_ptr<std::string[]>(new std::string[100], [](std::string* p) { delete[] p; }); // 100 "std::strings"
                    ::free(::malloc(sizeof(std::string) * 100));
                });
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Linear Random Allocator (std::string)")) {
                for (size_t i = 0; i < 1000; ++i) {
                    auto a = std::make_unique<std::string>();     // value of 0
                    auto b = std::make_unique<std::string[]>(i + 1000); // 100 "std::strings"
                    auto c = std::make_shared<std::string>();     // value of 0
                    auto d = std::shared_ptr<std::string[]>(new std::string[i + 1000], [](std::string* p) { delete[] p; }); // 100 "std::strings"
                    ::free(::malloc(sizeof(std::string) * (i + 1000)));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Std Parallel Random Allocator (std::string)")) {
                GL::parallel::For(0, 1000, [](size_t i) {
                    auto a = std::make_unique<std::string>();     // value of 0
                    auto b = std::make_unique<std::string[]>(i + 1000); // 100 "std::strings"
                    auto c = std::make_shared<std::string>();     // value of 0
                    auto d = std::shared_ptr<std::string[]>(new std::string[i + 1000], [](std::string* p) { delete[] p; }); // 100 "std::strings"
                    ::free(::malloc(sizeof(std::string) * (i + 1000)));
                    });
            }
#endif  
            if (1) { // if (auto timer = sw.debug_timer("Arena Linear Allocator (int)")) {
                for (size_t i = 0; i < 1000000; ++i) {
                    auto a = GL::arena_memory_pool::make_unique<int>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<int[]>(10000); // 100 "ints"
                    auto c = GL::arena_memory_pool::make_shared<int>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<int[]>(10000); // 100 "ints"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<int>(10000));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Parallel Allocator (int)")) {
                GL::parallel::For(0, 1000000, [](size_t) {
                    auto a = GL::arena_memory_pool::make_unique<int>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<int[]>(10000); // 100 "ints"
                    auto c = GL::arena_memory_pool::make_shared<int>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<int[]>(10000); // 100 "ints"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<int>(10000));
                });
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Linear Random Allocator (int)")) {
                for (size_t i = 0; i < 1000; ++i) {
                    auto a = GL::arena_memory_pool::make_unique<int>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<int[]>((unsigned int)(i + 1000)); // 100 "ints"
                    auto c = GL::arena_memory_pool::make_shared<int>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<int[]>((unsigned int)(i + 1000)); // 100 "ints"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<int>((unsigned int)(i + 1000)));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Parallel Random Allocator (int)")) {
                GL::parallel::For(0, 1000, [](size_t i) {
                    auto a = GL::arena_memory_pool::make_unique<int>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<int[]>((unsigned int)(i + 1000)); // 100 "ints"
                    auto c = GL::arena_memory_pool::make_shared<int>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<int[]>((unsigned int)(i + 1000)); // 100 "ints"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<int>((unsigned int)(i + 1000)));
                    });
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Linear Allocator (std::string)")) {
                for (size_t i = 0; i < 1000000; ++i) {
                    auto a = GL::arena_memory_pool::make_unique<std::string>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<std::string[]>(100); // 100 "std::strings"
                    auto c = GL::arena_memory_pool::make_shared<std::string>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<std::string[]>(100); // 100 "std::strings"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<std::string>(100));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Parallel Allocator (std::string)")) {
                GL::parallel::For(0, 1000000, [](size_t) {
                    //auto a = GL::arena_memory_pool::make_unique<std::string>();     // value of 0
                    //auto b = GL::arena_memory_pool::make_unique<std::string[]>(100); // 100 "std::strings"
                    auto c = GL::arena_memory_pool::make_shared<std::string>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<std::string[]>(100); // 100 "std::strings"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<std::string>(100));
                });
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Linear Random Allocator (std::string)")) {
                for (size_t i = 0; i < 1000; ++i) {
                    auto a = GL::arena_memory_pool::make_unique<std::string>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<std::string[]>((unsigned int)(i + 1000)); // 100 "std::strings"
                    auto c = GL::arena_memory_pool::make_shared<std::string>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<std::string[]>((unsigned int)(i + 1000)); // 100 "std::strings"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<std::string>((unsigned int)(i + 1000)));
                }
            }
            if (1) { // if (auto timer = sw.debug_timer("Arena Parallel Random Allocator (std::string)")) {
                GL::parallel::For(0, 1000, [](size_t i) {
                    auto a = GL::arena_memory_pool::make_unique<std::string>();     // value of 0
                    auto b = GL::arena_memory_pool::make_unique<std::string[]>((unsigned int)(i + 1000)); // 100 "std::strings"
                    auto c = GL::arena_memory_pool::make_shared<std::string>();     // value of 0
                    auto d = GL::arena_memory_pool::make_shared<std::string[]>((unsigned int)(i + 1000)); // 100 "std::strings"
                    //GL::arena_memory_pool::free(GL::arena_memory_pool::malloc<std::string>((unsigned int)(i + 1000)));
                });
            }
        }

        using namespace GL;
        using namespace GL::GPU;

        std::ios_base::sync_with_stdio(false);
        std::cin.tie(NULL);
        CONSOLE_SCREEN_BUFFER_INFO screen; GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

        int game_w = screen.dwSize.X / 2, game_h = screen.dwSize.Y - 3;
        std::deque<float> framerates;

        // Initialize the kernel array
        //print(GL::arena_memory_pool::debug());
        matrix_kernel<unsigned int> kernel(matrix<unsigned int>::from_vector({
            1, 1, 1,
            1, 0, 1,
            1, 1, 1
        }, 3));
        //print(GL::arena_memory_pool::debug());
        auto state = (matrix<float>::random(game_h, game_w, 1) > 0.4f).cast<unsigned int>();
        //print(GL::arena_memory_pool::debug());

        int frame = 1;
        // Run the game of life
        for (;;) {
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screen);
            int game_w2 = (screen.dwSize.X / 2), game_h2 = ((screen.dwSize.Y > 3) ? screen.dwSize.Y - 3 : 1);
            if (game_w2 != game_w || game_h != game_h2) {
                game_w = game_w2;
                game_h = game_h2;

                auto temp = state.resize(game_h, game_w, 1);
                state = temp;

                framerates.clear();
            }

            GL::stopwatch sw;

            // Convolve aligns the kernel ontop of each pixel, multiplies the neighboring pixels by the kernel, and sums the results. The edges are correctly handled using weighted-balancing on the kernel itself.
            auto nHood = state.convolve(static_matrix_kernel<unsigned int>{ &kernel });

            // Generate conditions for life
            // state == 1 && nHood < 2 ->> state = 0
            // state == 1 && nHood > 3 ->> state = 0
            // else if state == 1 ->> state = 1
            // state == 0 && nHood == 3 ->> state = 1
            auto C0 = (nHood == 2);
            auto C1 = (nHood == 3);

            //auto a0 = (state == 1) && (nHood < 2);  // Die of under population
            //auto a1 = (state > 0) && (C0 || C1);   // Continue to live
            //auto a2 = (state <= 0) && C1;           // Reproduction
            //auto a3 = (state == 1) && (nHood > 3);  // Over-population

            // display = (a0 + a1).join(2, a1 + a2).join(2, a3).cast(ArrayTypes::FLOAT);
            //auto R = a0 * a1;
            //auto G = a1 * a2;
            //auto B = a3;

            // Update state
            state *= C0.cast<unsigned int>();
            state += C1.cast<unsigned int>();

            // console_clear();
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });

            // print(state.ASCII().to_string({}, true));
#if 1
#if 0
            auto statef = state.cast<float>();
            auto blur_1 = statef.convolve(matrix<float>::guassian_kernel(3, 3));
            auto blur_2 = blur_1.convolve(matrix<float>::guassian_kernel(7, 7));
            auto blur_3 = blur_2.convolve(matrix<float>::guassian_kernel(11, 11));
            auto blur_4 = blur_3.convolve(matrix<float>::guassian_kernel(23, 23));
            auto blur_5 = blur_4.convolve(matrix<float>::guassian_kernel(53, 53));
            print((statef + blur_1 + blur_2 + blur_3 + blur_4 + blur_5).ASCII().to_string({}, true));
#else
            //// column position (0 to game_w)
            //auto col_pos = (matrix<float>::linear(0, game_w2 * game_h2, game_h2, game_w2, 1) / (float)game_h2).floor().cast<unsigned int>();

            //// row position (0 to game_h)
            //auto row_pos = (matrix<unsigned int>::linear(0, game_w2 * game_h2, game_h2, game_w2, 1) % game_h2);            
            //// UV coordinates for the screen
            //auto screen_U = col_pos.cast<float>() / (float)game_w2;
            //auto screen_V = row_pos.cast<float>() / (float)game_h2;

            //matrix<float>::test_vector(100);
            //matrix<float>::test_vector(1000);
            //matrix<float>::test_vector(10000);
            //matrix<float>::test_vector(100000);
            //matrix<float>::test_vector(1000000);


            class MatrixImage {
            public:
                std::vector<matrix<float>> mip_maps;
                MatrixImage() : mip_maps{} {};
                MatrixImage(matrix<float>&& srce) : mip_maps{} {
                    calculate_mip_maps(std::move(srce));
                };
                MatrixImage(MatrixImage const&) = delete;
                MatrixImage(MatrixImage&&) = delete;
                MatrixImage& operator=(MatrixImage const&) = delete;
                MatrixImage& operator=(MatrixImage&&) = delete;
                ~MatrixImage() = default;

                matrix<float> debug_display() const {
                    matrix<float> out = mip_maps[0];
                    for (int i = 1; i < mip_maps.size(); ++i) {
                        out = out.join(1, mip_maps[i].resize(mip_maps[0].size(0), mip_maps[i].size(1) + 16, 1));
                    }
                    return out;
                };
                matrix<float> sum() const {
                    std::vector<matrix<float>> mips;
                    matrix<float> out = mip_maps[0];
                    for (int i = 1; i < mip_maps.size(); ++i) {
                        mips.emplace_back(mip_maps[i].resize_stretch(mip_maps[0].size(0), mip_maps[0].size(1), 1));
                        out += mips[mips.size() - 1];
                    }
                    return out;
                };

            private:
                void calculate_mip_maps(matrix<float> && srce) {
                    mip_maps.reserve(32);
                    mip_maps.push_back(std::move(srce));
                    const matrix<float>* current = &mip_maps[0];
                    //auto kernel = matrix<float>::guassian_kernel<13, 13>();
                    while ((current->size(0) > 1) && (current->size(1) > 1)) {
                        //auto blurred = current->convolve(kernel);
                        //mip_maps.push_back(blurred.resize_stretch(std::floorf(((float)blurred.size(0) / 2.0f) + 0.5), std::floorf(((float)blurred.size(1) / 2.0f) + 0.5), 1)); //  current->halfsize<false>());

                        mip_maps.push_back(current->halfsize()); // faster but less accurate
                        current = &mip_maps[mip_maps.size() - 1];
                    }
                };

            };
            MatrixImage img(state.cast<float>());
            print(img.sum().resize_stretch(game_h, game_w, 1).ASCII().to_string({}, true));

            //auto texture_y = (screen_U * (float)I5.size(1)).cast<unsigned int>().min(I5.size(1) - 1);
            //auto texture_x = (screen_V * (float)I5.size(0)).cast<unsigned int>().min(I5.size(0) - 1);
            //auto texture_N = ((texture_y * I5.size(0)) + texture_x).min((I5.size(1) * I5.size(0)) - 1);
            //auto scaled = I5.resample(texture_N);

            //auto A = state.resize(game_h2 / 3, game_w2, 1);
            //auto B = state.cast<float>().convolve(matrix<float>::guassian_kernel(5, 5)).resize(game_h2 / 3, game_w2, 1);
            //auto C = state.halfsize().doublesize().resize(game_h2 / 3, game_w2, 1);
            //auto print_me = A.ASCII().join(0, B.ASCII()).join(0, C.ASCII());
            //print(print_me.to_string({}, true));
#endif
#endif
            // state += ((state > 0).cast<float>().convolve(matrix<float>::guassian_kernel<7,7>()) * (matrix<float>::random(game_h, game_w, 1) >= 0.995f).cast<float>()).cast<unsigned int>();
            // state = state.min(1);

            print("");
            auto this_frame = (float)(1.0 / sw.stop());
            framerates.push_back(this_frame);
           
            if (framerates.size() > 10000) framerates.pop_front();
            std::deque<float> copy(framerates);
            std::sort(copy.begin(), copy.end());
            float q0 = 0;
            float q1 = 0;
            float q2 = 0;
            float q3 = 0;
            float q4 = 0;
            if (copy.size() >= 4) {
                q0 = copy.at(0);
                q1 = copy.at(copy.size() / 4);
                q2 = copy.at(2 * copy.size() / 4);
                q3 = copy.at(3 * copy.size() / 4);
                q4 = copy.at(copy.size() - 1);
            }            

            print(GL::printf("min{ %f }  q1{ %f }  median{ %f }  q2{ %f }  max{ %f }  ", q0, q1, q2, q3, q4) + GL::arena_memory_pool::debug() + "         \t");
            std::cout << std::flush;

            // while (sw.stop() < 1.0 / 60.0) {
                // std::this_thread::yield();
            // }
        }
    }
#endif

    test_thread.join();
    return 0;
};

