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
		using SourceFile = GL::string;
		BETTER_ENUM(AST_Node_Type, uint32_t,
			File, Noop,
			Id, Reference, Var_Decl, Assign_Decl, Constant,
			Fun_Call, Unused_Return_Fun_Call,
			Arg_List, Arg,
			Equation,
			Array_Call, Dot_Access,
			Lambda,
			FunctionBlock,
			DeclarationBlock, Block, Scopeless_Block,
			Def,
			If,
			Parallel, Parallel_For, Parallel_Ranged_For, For, Ranged_For, While,
			Inline_Array, Inline_Map,
			Return,
			Prefix, Postfix,
			Break, Continue,
			Map_Pair, Value_Range, Inline_Range,
			Do, Try, Catch, Finally,
			Method,
			Attr_Decl,
			Logical_And, Logical_Or,
			Switch, Case, Default,
			Class, Namespace,
			FunctionDecl,
			BinaryFoldRight, Binary,
			Global_Decl,
			Compiled,
			ControlBlock,
			Assign_Retroactively,
			TypeId,
			JustInTimeCompilation,
			AST_Node_Type_end
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
			constexpr Position() = default;
			constexpr Position(const char* t_pos, const char* t_end) noexcept
				: line(1)
				, col(1)
				, pos(0)
				, m_pos(t_pos)
				, m_end(t_end)
				, m_last_col(1) {
			}
			static std::string_view str(const Position& t_begin, const Position& t_end) noexcept {
				if (t_begin.m_pos != nullptr && t_end.m_pos != nullptr) {
					return std::string_view(t_begin.m_pos, std::size_t(std::distance(t_begin.m_pos, t_end.m_pos)));
				}
				else {
					return {};
				}
			}
			constexpr Position& operator++() noexcept {
				if (m_pos != m_end) {
					if (*m_pos == '\n') {
						++line;
						m_last_col = col;
						col = 1;
					}
					else {
						++col;
					}

					++pos;
					++m_pos;
				}
				return *this;
			}
			constexpr Position& operator--() noexcept {
				--pos;
				--m_pos;
				if (*m_pos == '\n') {
					--line;
					col = m_last_col;
				}
				else {
					--col;
				}
				return *this;
			}
			constexpr Position& operator+=(size_t t_distance) noexcept {
				*this = (*this) + t_distance;
				return *this;
			}
			constexpr Position operator+(size_t t_distance) const noexcept {
				Position ret(*this);
				for (size_t i = 0; i < t_distance; ++i) {
					++ret;
				}
				return ret;
			}
			constexpr Position& operator-=(size_t t_distance) noexcept {
				*this = (*this) - t_distance;
				return *this;
			}
			constexpr Position operator-(size_t t_distance) const noexcept {
				Position ret(*this);
				for (size_t i = 0; i < t_distance; ++i) {
					--ret;
				}
				return ret;
			}
			constexpr bool operator==(const Position& t_rhs) const noexcept { return m_pos == t_rhs.m_pos; }
			constexpr bool operator!=(const Position& t_rhs) const noexcept { return m_pos != t_rhs.m_pos; }
			constexpr bool has_more() const noexcept { return m_pos != m_end; }
			constexpr size_t remaining() const noexcept { return static_cast<size_t>(m_end - m_pos); }
			constexpr const char& operator*() const noexcept {
				if (m_pos == m_end) {
					return ""[0];
				}
				else {
					return *m_pos;
				}
			}

			int line = -1;
			int col = -1;
			int pos = -1;

			GL::string to_string() const {
				return GL::printf("L%iC%i(#%i)", line, col, pos);
			};

		private:
			const char* m_pos = nullptr;
			const char* m_end = nullptr;
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
						std::vector<word_t> out; {
							static std::regex pattern{ R"("[^"]*"|\/\/[^\n]*\n|\/\*[^\*\/]*\*\/|[A-z0-9_:#]+)" };
							size_t position, length;
							for (auto i = std::regex_iterator(text.begin(), text.end(), pattern);
								i != decltype(i)();
								++i)
							{
								const auto& m = *i;
								position = ((text.length() - m.suffix().length()) - m.length());
								length = m.length();
								out.push_back(word_t{ text.substr(position, length), position });
							}
						}
						return out;
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
								Text = ReplaceWith + Text.substr(Where.word.length());
							}
							else if ((Where.pos_start + Where.word.length()) == Text.length()) {
								Text = Text.substr(0, Where.pos_start) + ReplaceWith;
							}
							else {
								Text = Text.substr(0, Where.pos_start) + ReplaceWith + Text.substr(Where.pos_start + Where.word.length());
							}
							return true;
						}

						// in-word replacement when explicitely requested
						if (auto f_p = Where.word.find("##" + Find); f_p != GL::string::npos) {
							auto NewReplaceWith = Replace(Where.word, "##" + Find, ReplaceWith);

							if (Where.pos_start == 0) {
								Text = NewReplaceWith + Text.substr(Where.word.length());
							}
							else if ((Where.pos_start + (Where.word.length())) == Text.length()) {
								Text = Text.substr(0, Where.pos_start) + NewReplaceWith;
							}
							else {
								Text = Text.substr(0, Where.pos_start) + NewReplaceWith + Text.substr(Where.pos_start + (Where.word.length()));
							}
							return true;
						}

						// in-word string replacement when explicitely requested
						if (auto f_p = Where.word.find("#" + Find); f_p != GL::string::npos) {
							auto NewReplaceWith = Replace(Where.word, "#" + Find, "\"" + ExpandCode(ReplaceWith) + "\"");

							if (Where.pos_start == 0) {
								Text = NewReplaceWith + Text.substr(Where.word.length());
							}
							else if ((Where.pos_start + (Where.word.length())) == Text.length()) {
								Text = Text.substr(0, Where.pos_start) + NewReplaceWith;
							}
							else {
								Text = Text.substr(0, Where.pos_start) + NewReplaceWith + Text.substr(Where.pos_start + (Where.word.length()));
							}
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

				PreprocessorTokenPtr Parse(Engine::SourceFile t_input) { return parse(t_input); };

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

				PreprocessorTokenPtr parse(Engine::SourceFile t_input) {
					const auto begin = t_input.empty() ? nullptr : &t_input.front();
					const auto end = begin == nullptr ? nullptr : begin + t_input.size();
					m_position = Engine::Position(begin, end);

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

					/*
					var x;
					*/
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
					/*
					double x;
					*/
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
					/*
					var x = double();
					*/
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
						case Engine::hash("conitnue"):
						case Engine::hash("case"):
						case Engine::hash("default"):
						case Engine::hash("switch"):
						case Engine::hash("try"):
						case Engine::hash("catch"):
						case Engine::hash("finally"):
						case Engine::hash("do"):
						case Engine::hash("evaluate"):
						case Engine::hash("namespace"):
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
									auto bv = buildInt(16, (std::string_view)match, true);
									m_match_stack.push_back(ParseNode{ make_const((std::string_view)match, start, bv), nullptr });
									return true;
								}
								else if (Binary_()) {
									auto match = Engine::Position::str(start, m_position);
									auto bv = buildInt(2, (std::string_view)match, true);
									m_match_stack.push_back(ParseNode{ make_const((std::string_view)match, start, bv), nullptr });
									return true;
								}
								else if (Float_()) {
									auto match = Engine::Position::str(start, m_position);
									auto bv = buildFloat((std::string_view)match);
									m_match_stack.push_back(ParseNode{ make_const((std::string_view)match, start, bv), nullptr });
									return true;
								}
								else {
									IntSuffix_();
									auto match = Engine::Position::str(start, m_position);
									if (!match.empty() && (match[0] == '0')) {
										auto bv = buildInt(8, (std::string_view)match, false);
										m_match_stack.push_back(ParseNode{ make_const((std::string_view)match, start, bv), nullptr });
									}
									else if (!match.empty()) {
										auto bv = buildInt(10, (std::string_view)match, false);
										m_match_stack.push_back(ParseNode{ make_const((std::string_view)match, start, bv), nullptr });
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

							Char(':'); // optional

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

							Char(':'); // optional

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
					ParseNode parse_internal(const GL::string& t_input, GL::scope::impl::BasicScope* currentScope) {
						auto this_scope = currentScope->make_scope();

						const auto begin = t_input.empty() ? nullptr : &t_input.front();
						const auto end = begin == nullptr ? nullptr : begin + t_input.size();
						m_position = Engine::Position(begin, end);

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
		class AbstractSyntaxTreeNode {
		public:
			AbstractSyntaxTreeNode(Engine::AST_Node_Type id) : identifier{ id } {};
			AbstractSyntaxTreeNode(Engine::AST_Node_Type id, GL::string const& t_ast_node_text, Engine::Parse_Location&& t_loc, std::vector<AbstractSyntaxTreeNode> t_children) : text(t_ast_node_text), location(std::forward<Engine::Parse_Location>(t_loc)), identifier{ id }, children(std::forward<std::vector<AbstractSyntaxTreeNode>>(t_children)) {};
			AbstractSyntaxTreeNode() : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Noop) {};
			AbstractSyntaxTreeNode(AbstractSyntaxTreeNode const&) = default;
			AbstractSyntaxTreeNode(AbstractSyntaxTreeNode&&) = default;
			AbstractSyntaxTreeNode& operator=(AbstractSyntaxTreeNode const&) = default;
			AbstractSyntaxTreeNode& operator=(AbstractSyntaxTreeNode&&) = default;
			virtual ~AbstractSyntaxTreeNode() = default;

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
				output;

			/// Prints the contents of an AST node, including its children, recursively
			GL::string to_string(const GL::string& t_prepend = "") const {
				GL::string Text{ text };
				GL::string str = std::string(identifier.ToString());
				GL::string returnType = output.name();
				GL::string locationStr = location.to_string();
				auto out = t_prepend + "(" + str + ") \"" + Text + "\": " + locationStr + " -> " + returnType;
				for (auto& elem : children) { out = out.add_to_delim(elem.to_string(t_prepend + "\t"), "\n"); }
				return out;
			};

		};
		namespace except {
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

		class File_Node final : public AbstractSyntaxTreeNode {
		public:
			File_Node(GL::string const& t_ast_node_text, Engine::Parse_Location&& t_loc, std::vector<AbstractSyntaxTreeNode>&& t_children) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::File, t_ast_node_text, std::forward<Engine::Parse_Location>(t_loc), std::forward<std::vector<AbstractSyntaxTreeNode>>(t_children)) {};
		};
		class Noop_Node final : public AbstractSyntaxTreeNode {
		public:
			Noop_Node(GL::string const& t_ast_node_text, Engine::Parse_Location&& t_loc, std::vector<AbstractSyntaxTreeNode>&& t_children) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Noop, t_ast_node_text, std::forward<Engine::Parse_Location>(t_loc), std::forward<std::vector<AbstractSyntaxTreeNode>>(t_children)) {};
		};
		class Return_Node final : public AbstractSyntaxTreeNode {
		public:
			Return_Node(GL::string const& t_ast_node_text, Engine::Parse_Location&& t_loc, std::vector<AbstractSyntaxTreeNode>&& t_children) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Return, t_ast_node_text, std::forward<Engine::Parse_Location>(t_loc), std::forward<std::vector<AbstractSyntaxTreeNode>>(t_children)) {};
		};
		class Constant_Node final : public AbstractSyntaxTreeNode {
		public:
			Constant_Node(GL::string const& t_ast_node_text, Engine::Parse_Location&& t_loc, std::vector<AbstractSyntaxTreeNode>&& t_children, GL::any const& t_value) : AbstractSyntaxTreeNode(Engine::AST_Node_Type::Constant, t_ast_node_text, std::forward<Engine::Parse_Location>(t_loc), std::forward<std::vector<AbstractSyntaxTreeNode>>(t_children)) {
				this->constant = t_value.fast();
			};
		};

	};


};













int main() {
	while (1) {
		GL::Engine::AbstractSyntaxTreeNode 
			node;
		node.children.resize(16);




		GL::Engine::File_Node node2("", GL::Engine::Parse_Location(), {});
		node2.children.resize(16);

		node.children.push_back(std::move(node2));

		node.children.push_back(GL::Engine::Constant_Node("", {}, {}, 100.0f));
	}

	if (1) {
		GL::string Script = R"(
				print(ONE_HUNDRED); // print(ONE_HUNDRED);

				#define as_foot(x) ##x_ft
				return as_foot(100.0); // return 100.0_ft;
				#undef as_foot

				#define print(x) std::cout << x << std::endl
				#define ONE_HUNDRED = 100

				print(__DATE__); 
				print(ONE_HUNDRED); // std::cout << 100 << std::endl;
				print(__TIMESTAMP__);

				#undef ONE_HUNDRED
				#undef print
				
				print(ONE_HUNDRED); // print(ONE_HUNDRED);

				version ##__DATE__v##__VERSION__
				version #__DATE__v.##__VERSION__

				#define print(x) std::cout << #x + ": " << x << std::endl
				for (DateTime i = __DATE__; i < __DATE__ + 365_d; ++i){
					print(i);	
				}
				// #undef print

				#define I_AM_DEFINED
				#ifdef I_AM_DEFINED
					print("YAY");	
				#else
					print("THIS SHOULD NOT HAPPEN");	
				#endif
				#undef I_AM_DEFINED
				#ifdef I_AM_DEFINED
					print("THIS SHOULD NOT HAPPEN");
				#else
					print("YAY");
				#endif
			)";
		GL::Engine2::Compiler::Preprocessor::PreprocessorState state;
		if (auto preprocessor_result = GL::Engine2::Compiler::Preprocessor().Parse(Script)) {
			preprocessor_result->GenerateExpandedCode(state);
			auto expanded_script = state.GetFinalScript();
			print(expanded_script);
		}
	}

	if (1) {
		GL::string Script = R"(
			int x = 10;
            x = 100;
            return x * x + x;

			for (int i = 0; i < 10; ++i){
				--i++;				
			}

			namespace Test{
				int StaticObject = 100;	
				namespace Test2{

				};
			};
		)";

		GL::Engine2::Compiler::Preprocessor::PreprocessorState state;
		if (auto preprocessor_result = GL::Engine2::Compiler::Preprocessor().Parse(Script)) {
			preprocessor_result->GenerateExpandedCode(state);
			auto expanded_script = state.GetFinalScript();
			print(expanded_script);

			GL::Engine2::Compiler::Interpreter::Parser parser;
			GL::scope::impl::RootScope root;
			root.perform_builtins();
			if (auto node = parser.Parse(expanded_script, &root); node.first && node.second) {
				print(node.first->to_string());
			}
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
