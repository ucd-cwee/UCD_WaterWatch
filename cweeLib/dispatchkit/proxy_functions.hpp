// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef CHAISCRIPT_PROXY_FUNCTIONS_HPP_
#define CHAISCRIPT_PROXY_FUNCTIONS_HPP_

//#include <cassert>
//#include <functional>
//#include <iterator>
//#include <memory>
//#include <stdexcept>
//#include <string>
//#include <type_traits>
//#include <vector>
//
//#include "../chaiscript_defines.hpp"
//#include "boxed_cast.hpp"
//#include "boxed_value.hpp"
//#include "dynamic_object.hpp"
//#include "function_params.hpp"
//#include "proxy_functions_detail.hpp"
//#include "type_info.hpp"

namespace chaiscript {
  class Type_Conversions;
  namespace exception {
    class bad_boxed_cast;
    struct arity_error;
  } // namespace exception
} // namespace chaiscript

namespace chaiscript {
  class Boxed_Number;
  struct AST_Node;

  using AST_NodePtr = chaiscript::unique_ptr<AST_Node>;

  namespace dispatch {
    template<typename FunctionType>
    std::function<FunctionType> functor(chaiscript::shared_ptr<const Proxy_Function_Base> func, const Type_Conversions_State *t_conversions);

    class Param_Types {
    public:
      Param_Types()
          : m_has_types(false) {
      }

      explicit Param_Types(chaiscript::small_vector<std::pair<std::string, Type_Info>> t_types)
          : m_types(std::move(t_types))
          , m_has_types(false) {
        update_has_types();
      }

      void push_front(std::string t_name, Type_Info t_ti) {
        m_types.emplace(m_types.begin(), std::move(t_name), t_ti);
        update_has_types();
      }

      bool operator==(const Param_Types &t_rhs) const noexcept { return m_types == t_rhs.m_types; }

      chaiscript::small_vector<Boxed_Value> convert(Function_Params t_params, const Type_Conversions_State &t_conversions) const {
        auto vals = t_params.to_vector();
        const auto dynamic_object_type_info = user_type<Dynamic_Object>();
        for (size_t i = 0; i < vals.size(); ++i) {
          const auto &name = m_types[i].first;
          if (!name.empty()) {
            const auto &bv = vals[i];

            if (!bv.get_type_info().bare_equal(dynamic_object_type_info)) {
              const auto &ti = m_types[i].second;
              if (!ti.is_undef()) {
                if (!bv.get_type_info().bare_equal(ti)) {
                  if (t_conversions->converts(ti, bv.get_type_info())) {
                    try {
                      // We will not catch any bad_boxed_dynamic_cast that is thrown, let the user get it
                      // either way, we are not responsible if it doesn't work
                      vals[i] = t_conversions->boxed_type_conversion(m_types[i].second, t_conversions.saves(), vals[i]);
                    } catch (...) {
                      try {
                        // try going the other way
                        vals[i] = t_conversions->boxed_type_down_conversion(m_types[i].second, t_conversions.saves(), vals[i]);
                      } catch (const chaiscript::detail::exception::bad_any_cast &) {
                        throw exception::bad_boxed_cast(bv.get_type_info(), *m_types[i].second.bare_type_info());
                      }
                    }
                  }
                }
              }
            }
          }
        }

        return vals;
      }

      // first result: is a match
      // second result: needs conversions
      std::pair<bool, bool> match(const Function_Params &vals, const Type_Conversions_State &t_conversions) const noexcept {
        const auto dynamic_object_type_info = user_type<Dynamic_Object>();
        bool needs_conversion = false;

        if (!m_has_types) {
          return std::make_pair(true, needs_conversion);
        }
        if (vals.size() != m_types.size()) {
          return std::make_pair(false, needs_conversion);
        }

        for (size_t i = 0; i < vals.size(); ++i) {
          const auto &name = m_types[i].first;
          if (!name.empty()) {
            const auto &bv = vals[i];

            if (bv.get_type_info().bare_equal(dynamic_object_type_info)) {
              try {
                const Dynamic_Object &d = boxed_cast<const Dynamic_Object &>(bv, &t_conversions);
                if (!(name == "Dynamic_Object" || d.get_type_name() == name)) {
                  return std::make_pair(false, false);
                }
              } catch (const std::bad_cast &) {
                return std::make_pair(false, false);
              }
            } else {
              const auto &ti = m_types[i].second;
              if (!ti.is_undef()) {
                if (!bv.get_type_info().bare_equal(ti)) {
                  if (!t_conversions->converts(ti, bv.get_type_info())) {
                    return std::make_pair(false, false);
                  } else {
                    needs_conversion = true;
                  }
                }
              } else {
                return std::make_pair(false, false);
              }
            }
          }
        }

        return std::make_pair(true, needs_conversion);
      }

      const chaiscript::small_vector<std::pair<std::string, Type_Info>> &types() const noexcept { return m_types; }

    private:
      void update_has_types() {
        for (const auto &type : m_types) {
          if (!type.first.empty()) {
            m_has_types = true;
            return;
          }
        }

        m_has_types = false;
      }

      chaiscript::small_vector<std::pair<std::string, Type_Info>> m_types;
      bool m_has_types;
    };

    /**
 * Pure virtual base class for all Proxy_Function implementations
 * Proxy_Functions are a type erasure of type safe C++
 * function calls. At runtime parameter types are expected to be
 * tested against passed in types.
 * Dispatch_Engine only knows how to work with Proxy_Function, no other
 * function classes.
 */
    class Proxy_Function_Base {
    public:
      virtual ~Proxy_Function_Base() = default;

      Boxed_Value operator()(const Function_Params &params, const chaiscript::Type_Conversions_State &t_conversions) const {
        if (m_arity < 0 || size_t(m_arity) == params.size()) {
          return do_call(params, t_conversions);
        } else {
          throw exception::arity_error(static_cast<int>(params.size()), m_arity);
        }
      }

      /// Returns a vector containing all of the types of the parameters the function returns/takes
      /// if the function is variadic or takes no arguments (arity of 0 or -1), the returned
      /// value contains exactly 1 Type_Info object: the return type
      /// \returns the types of all parameters.
      const chaiscript::small_vector<Type_Info> &get_param_types() const noexcept { return m_types; }

      virtual bool operator==(const Proxy_Function_Base &) const noexcept = 0;
      virtual bool call_match(const Function_Params &vals, const Type_Conversions_State &t_conversions) const = 0;

      virtual bool is_attribute_function() const noexcept { return false; }

      bool has_arithmetic_param() const noexcept { return m_has_arithmetic_param; }

      virtual chaiscript::small_vector<chaiscript::shared_ptr<const Proxy_Function_Base>> get_contained_functions() const {
        return chaiscript::small_vector<chaiscript::shared_ptr<const Proxy_Function_Base>>();
      }

      //! Return true if the function is a possible match
      //! to the passed in values
      bool filter(const Function_Params &vals, const Type_Conversions_State &t_conversions) const noexcept {
        assert(m_arity == -1 || (m_arity > 0 && static_cast<int>(vals.size()) == m_arity));
        // arity of 0 would have dropped out due to the assert;

        if (m_arity < 0) {
          return true;
        } else if (m_arity > 1) { // arity of 2 or larger
            bool toReturn = true;
            for (int i = vals.size() - 1; i >= 0; --i) {
                toReturn = toReturn && compare_type_to_param(m_types[i+1], vals[i], t_conversions);
                if (!toReturn) break;
            }
            return toReturn;
          // return compare_type_to_param(m_types[1], vals[0], t_conversions) && compare_type_to_param(m_types[2], vals[1], t_conversions);
        } else { // arity of 1
          return compare_type_to_param(m_types[1], vals[0], t_conversions);
        }
      }

      /// \returns the number of arguments the function takes or -1 if it is variadic
      int get_arity() const noexcept { return m_arity; }

      static bool compare_type_to_param(const Type_Info &ti, const Boxed_Value &bv, const Type_Conversions_State &t_conversions) noexcept {
        const auto boxed_value_ti = user_type<Boxed_Value>();
        const auto boxed_number_ti = user_type<Boxed_Number>();
        const auto function_ti = user_type<chaiscript::shared_ptr<const Proxy_Function_Base>>();

        if (ti.is_undef() || ti.bare_equal(boxed_value_ti)
            || (!bv.get_type_info().is_undef()
                && ((ti.bare_equal(boxed_number_ti) && bv.get_type_info().is_arithmetic()) || ti.bare_equal(bv.get_type_info())
                    || bv.get_type_info().bare_equal(function_ti) || t_conversions->converts(ti, bv.get_type_info())))) {
          return true;
        } else {
          return false;
        }
      }

      virtual bool compare_first_type(const Boxed_Value &bv, const Type_Conversions_State &t_conversions) const noexcept {
        /// TODO is m_types guaranteed to be at least 2??
        return compare_type_to_param(m_types[1], bv, t_conversions);
      }

      bool SimilarTo(const Proxy_Function_Base& other) const {
          return (m_types == other.m_types) && (m_arity == other.m_arity) && (m_has_arithmetic_param == other.m_has_arithmetic_param);
      };

      std::string get_description() const noexcept { return descriptor; };
      void set_description(const std::string& newDescription) const noexcept { descriptor = newDescription; };

    protected:
      virtual Boxed_Value do_call(const Function_Params &params, const Type_Conversions_State &t_conversions) const = 0;

      Proxy_Function_Base(chaiscript::small_vector<Type_Info> t_types, int t_arity)
          : m_types(std::move(t_types))
          , m_arity(t_arity)
          , m_has_arithmetic_param(false) {
        for (size_t i = 1; i < m_types.size(); ++i) {
          if (m_types[i].is_arithmetic()) {
            m_has_arithmetic_param = true;
            return;
          }
        }
      }

      static bool compare_types(const chaiscript::small_vector<Type_Info> &tis, const Function_Params &bvs, const Type_Conversions_State &t_conversions) noexcept {
        if (tis.size() - 1 != bvs.size()) {
          return false;
        } else {
          const size_t size = bvs.size();
          for (size_t i = 0; i < size; ++i) {
            if (!compare_type_to_param(tis[i + 1], bvs[i], t_conversions)) {
              return false;
            }
          }
        }
        return true;
      }

      chaiscript::small_vector<Type_Info> m_types;
      int m_arity;
      bool m_has_arithmetic_param;
      mutable std::string descriptor;
    };
  } // namespace dispatch

  /// \brief Common typedef used for passing of any registered function in ChaiScript
  using Proxy_Function = chaiscript::shared_ptr<dispatch::Proxy_Function_Base>;

  /// \brief Const version of Proxy_Function. Points to a const Proxy_Function. This is how most registered functions
  ///        are handled internally.
  using Const_Proxy_Function = chaiscript::shared_ptr<const dispatch::Proxy_Function_Base>;

  namespace exception {
    /// \brief  Exception thrown if a function's guard fails
    class guard_error : public std::runtime_error {
    public:
      guard_error() noexcept
          : std::runtime_error("Guard evaluation failed") {
      }

      guard_error(const guard_error &) = default;

      ~guard_error() noexcept override = default;
    };
  } // namespace exception

  namespace dispatch {
    /// A Proxy_Function implementation that is not type safe, the called function
    /// is expecting a vector<Boxed_Value> that it works with how it chooses.
    class Dynamic_Proxy_Function : public Proxy_Function_Base {
    public:
      Dynamic_Proxy_Function(const int t_arity,
                             chaiscript::shared_ptr<AST_Node> t_parsenode,
                             Param_Types t_param_types = Param_Types(),
                             Proxy_Function t_guard = Proxy_Function())
          : Proxy_Function_Base(build_param_type_list(t_param_types), t_arity)
          , m_param_types(std::move(t_param_types))
          , m_guard(std::move(t_guard))
          , m_parsenode(std::move(t_parsenode)) {
        // assert(t_parsenode);
      }

      bool operator==(const Proxy_Function_Base &rhs) const noexcept override {
        const Dynamic_Proxy_Function *prhs = dynamic_cast<const Dynamic_Proxy_Function *>(&rhs);

        return this == &rhs
            || ((prhs != nullptr) && this->m_arity == prhs->m_arity && !this->m_guard && !prhs->m_guard
                && this->m_param_types == prhs->m_param_types);
      }

      bool call_match(const Function_Params &vals, const Type_Conversions_State &t_conversions) const override {
        return call_match_internal(vals, t_conversions).first;
      }

      bool has_guard() const noexcept { return bool(m_guard); }

      Proxy_Function get_guard() const noexcept { return m_guard; }

      bool has_parse_tree() const noexcept { return static_cast<bool>(m_parsenode); }

      const AST_Node &get_parse_tree() const {
        if (m_parsenode) {
          return *m_parsenode;
        } else {
          throw std::runtime_error("Dynamic_Proxy_Function does not have parse_tree");
        }
      }

    protected:
      bool test_guard(const Function_Params &params, const Type_Conversions_State &t_conversions) const {
        if (m_guard) {
          try {
            return boxed_cast<bool>((*m_guard)(params, t_conversions));
          } catch (const exception::arity_error &) {
            return false;
          } catch (const exception::bad_boxed_cast &) {
            return false;
          }
        } else {
          return true;
        }
      }

      // first result: is a match
      // second result: needs conversions
      std::pair<bool, bool> call_match_internal(const Function_Params &vals, const Type_Conversions_State &t_conversions) const {
        const auto comparison_result = [&]() {
          if (m_arity < 0) {
            return std::make_pair(true, false);
          } else if (vals.size() == size_t(m_arity)) {
            return m_param_types.match(vals, t_conversions);
          } else {
            return std::make_pair(false, false);
          }
        }();

        return std::make_pair(comparison_result.first && test_guard(vals, t_conversions), comparison_result.second);
      }

    private:
      static chaiscript::small_vector<Type_Info> build_param_type_list(const Param_Types &t_types) {
        // For the return type
        chaiscript::small_vector<Type_Info> types{chaiscript::detail::Get_Type_Info<Boxed_Value>::get()};

        for (const auto &t : t_types.types()) {
          if (t.second.is_undef()) {
            types.push_back(chaiscript::detail::Get_Type_Info<Boxed_Value>::get());
          } else {
            types.push_back(t.second);
          }
        }

        return types;
      }

    protected:
      Param_Types m_param_types;

    private:
      Proxy_Function m_guard;
      chaiscript::shared_ptr<AST_Node> m_parsenode;
    };

    template<typename Callable>
    class Dynamic_Proxy_Function_Impl final : public Dynamic_Proxy_Function {
    public:
      Dynamic_Proxy_Function_Impl(Callable t_f,
                                  int t_arity = -1,
                                  chaiscript::shared_ptr<AST_Node> t_parsenode = AST_NodePtr(),
                                  Param_Types t_param_types = Param_Types(),
                                  Proxy_Function t_guard = Proxy_Function())
          : Dynamic_Proxy_Function(t_arity, std::move(t_parsenode), std::move(t_param_types), std::move(t_guard))
          , m_f(std::move(t_f)) {
      }

    protected:
      Boxed_Value do_call(const Function_Params &params, const Type_Conversions_State &t_conversions) const override {
        const auto [is_a_match, needs_conversions] = call_match_internal(params, t_conversions);
        if (is_a_match) {
          if (needs_conversions) {
            return m_f(Function_Params{m_param_types.convert(params, t_conversions)});
          } else {
            return m_f(params);
          }
        } else {
          throw exception::guard_error();
        }
      }

    private:
      Callable m_f;
    };

    template<typename Callable, typename... Arg>
    Proxy_Function make_dynamic_proxy_function(Callable &&c, Arg &&...a) {
      return chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Dynamic_Proxy_Function_Impl<Callable>>(std::forward<Callable>(c),
                                                                                                                     std::forward<Arg>(a)...);
    }

    /// An object used by Bound_Function to represent "_" parameters
    /// of a binding. This allows for unbound parameters during bind.
    struct Placeholder_Object {
    };

    /// An implementation of Proxy_Function that takes a Proxy_Function
    /// and substitutes bound parameters into the parameter list
    /// at runtime, when call() is executed.
    /// it is used for bind(function, param1, _, param2) style calls
    class Bound_Function final : public Proxy_Function_Base {
    public:
      Bound_Function(const Const_Proxy_Function &t_f, const chaiscript::small_vector<Boxed_Value> &t_args)
          : Proxy_Function_Base(build_param_type_info(t_f, t_args),
                                (t_f->get_arity() < 0 ? -1 : static_cast<int>(build_param_type_info(t_f, t_args).size()) - 1))
          , m_f(t_f)
          , m_args(t_args) {
        assert(m_f->get_arity() < 0 || m_f->get_arity() == static_cast<int>(m_args.size()));
      }

      bool operator==(const Proxy_Function_Base &t_f) const noexcept override { return &t_f == this; }

      bool call_match(const Function_Params &vals, const Type_Conversions_State &t_conversions) const override {
        return m_f->call_match(Function_Params(build_param_list(vals)), t_conversions);
      }

      chaiscript::small_vector<Const_Proxy_Function> get_contained_functions() const override { return chaiscript::small_vector<Const_Proxy_Function>{m_f}; }

      chaiscript::small_vector<Boxed_Value> build_param_list(const Function_Params &params) const {
        auto parg = params.begin();
        auto barg = m_args.begin();

        chaiscript::small_vector<Boxed_Value> args;

        while (!(parg == params.end() && barg == m_args.end())) {
          while (barg != m_args.end() && !(barg->get_type_info() == chaiscript::detail::Get_Type_Info<Placeholder_Object>::get())) {
            args.push_back(*barg);
            ++barg;
          }

          if (parg != params.end()) {
            args.push_back(*parg);
            ++parg;
          }

          if (barg != m_args.end() && barg->get_type_info() == chaiscript::detail::Get_Type_Info<Placeholder_Object>::get()) {
            ++barg;
          }
        }
        return args;
      }

    protected:
      static chaiscript::small_vector<Type_Info> build_param_type_info(const Const_Proxy_Function &t_f, const chaiscript::small_vector<Boxed_Value> &t_args) {
        assert(t_f->get_arity() < 0 || t_f->get_arity() == static_cast<int>(t_args.size()));

        if (t_f->get_arity() < 0) {
          return chaiscript::small_vector<Type_Info>();
        }

        const auto types = t_f->get_param_types();
        assert(types.size() == t_args.size() + 1);

        // this analysis warning is invalid in MSVC12 and doesn't exist in MSVC14
        chaiscript::small_vector<Type_Info> retval{types[0]};

        for (size_t i = 0; i < types.size() - 1; ++i) {
          if (t_args[i].get_type_info() == chaiscript::detail::Get_Type_Info<Placeholder_Object>::get()) {
            retval.push_back(types[i + 1]);
          }
        }

        return retval;
      }

      Boxed_Value do_call(const Function_Params &params, const Type_Conversions_State &t_conversions) const override {
        return (*m_f)(Function_Params{build_param_list(params)}, t_conversions);
      }

    private:
      Const_Proxy_Function m_f;
      chaiscript::small_vector<Boxed_Value> m_args;
    };

    class Proxy_Function_Impl_Base : public Proxy_Function_Base {
    public:
      explicit Proxy_Function_Impl_Base(const chaiscript::small_vector<Type_Info> &t_types)
          : Proxy_Function_Base(t_types, static_cast<int>(t_types.size()) - 1) {
      }

      bool call_match(const Function_Params &vals, const Type_Conversions_State &t_conversions) const noexcept override {
        return static_cast<int>(vals.size()) == get_arity()
            && (compare_types(m_types, vals, t_conversions) && compare_types_with_cast(vals, t_conversions));
      }

      virtual bool compare_types_with_cast(const Function_Params &vals, const Type_Conversions_State &t_conversions) const noexcept = 0;
    };

    /// For any callable object
    template<typename Func, typename Callable>
    class Proxy_Function_Callable_Impl final : public Proxy_Function_Impl_Base {
    public:
      explicit Proxy_Function_Callable_Impl(Callable f)
          : Proxy_Function_Impl_Base(detail::build_param_type_list(static_cast<Func *>(nullptr)))
          , m_f(std::move(f)) {
      }

      bool compare_types_with_cast(const Function_Params &vals, const Type_Conversions_State &t_conversions) const noexcept override {
        return detail::compare_types_cast(static_cast<Func *>(nullptr), vals, t_conversions);
      }

      bool operator==(const Proxy_Function_Base &t_func) const noexcept override {
        return dynamic_cast<const Proxy_Function_Callable_Impl<Func, Callable> *>(&t_func) != nullptr;
      }

    protected:
      Boxed_Value do_call(const Function_Params &params, const Type_Conversions_State &t_conversions) const override {
        return detail::call_func(static_cast<Func *>(nullptr), m_f, params, t_conversions);
      }

    private:
      Callable m_f;
    };

    class Assignable_Proxy_Function : public Proxy_Function_Impl_Base {
    public:
      explicit Assignable_Proxy_Function(const chaiscript::small_vector<Type_Info> &t_types)
          : Proxy_Function_Impl_Base(t_types) {
      }

      virtual void assign(const chaiscript::shared_ptr<const Proxy_Function_Base> &t_rhs) = 0;
    };

    template<typename Func>
    class Assignable_Proxy_Function_Impl final : public Assignable_Proxy_Function {
    public:
      Assignable_Proxy_Function_Impl(std::reference_wrapper<std::function<Func>> t_f, chaiscript::shared_ptr<std::function<Func>> t_ptr)
          : Assignable_Proxy_Function(detail::build_param_type_list(static_cast<Func *>(nullptr)))
          , m_f(std::move(t_f))
          , m_shared_ptr_holder(std::move(t_ptr)) {
        assert(!m_shared_ptr_holder || m_shared_ptr_holder.get() == &m_f.get());
      }

      bool compare_types_with_cast(const Function_Params &vals, const Type_Conversions_State &t_conversions) const noexcept override {
        return detail::compare_types_cast(static_cast<Func *>(nullptr), vals, t_conversions);
      }

      bool operator==(const Proxy_Function_Base &t_func) const noexcept override {
        return dynamic_cast<const Assignable_Proxy_Function_Impl<Func> *>(&t_func) != nullptr;
      }

      std::function<Func> internal_function() const { return m_f.get(); }

      void assign(const chaiscript::shared_ptr<const Proxy_Function_Base> &t_rhs) override { m_f.get() = dispatch::functor<Func>(t_rhs, nullptr); }

    protected:
      Boxed_Value do_call(const Function_Params &params, const Type_Conversions_State &t_conversions) const override {
        return detail::call_func(static_cast<Func *>(nullptr), m_f.get(), params, t_conversions);
      }

    private:
      std::reference_wrapper<std::function<Func>> m_f;
      chaiscript::shared_ptr<std::function<Func>> m_shared_ptr_holder;
    };

    /// Attribute getter Proxy_Function implementation
    template<typename T, typename Class>
    class Attribute_Access final : public Proxy_Function_Base {
    public:
      explicit Attribute_Access(T Class::*t_attr)
          : Proxy_Function_Base(param_types(), 1)
          , m_attr(t_attr) {
      }

      bool is_attribute_function() const noexcept override { return true; }

      bool operator==(const Proxy_Function_Base &t_func) const noexcept override {
        const Attribute_Access<T, Class> *aa = dynamic_cast<const Attribute_Access<T, Class> *>(&t_func);

        if (aa) {
          return m_attr == aa->m_attr;
        } else {
          return false;
        }
      }

      bool call_match(const Function_Params &vals, const Type_Conversions_State &) const noexcept override {
        if (vals.size() != 1) {
          return false;
        }
        const auto class_type_info = user_type<Class>();
        return vals[0].get_type_info().bare_equal(class_type_info);
      }

    protected:
      Boxed_Value do_call(const Function_Params &params, const Type_Conversions_State &t_conversions) const override {
        const Boxed_Value &bv = params[0];
        if (bv.is_const()) {
          const Class *o = boxed_cast<const Class *>(bv, &t_conversions);
          return do_call_impl<T>(o);
        } else {
          Class *o = boxed_cast<Class *>(bv, &t_conversions);
          return do_call_impl<T>(o);
        }
      }

    private:
      template<typename Type>
      auto do_call_impl(Class *o) const {
        if constexpr (std::is_pointer<Type>::value) {
          return detail::Handle_Return<Type>::handle(o->*m_attr);
        } else {
          return detail::Handle_Return<typename std::add_lvalue_reference<Type>::type>::handle(o->*m_attr);
        }
      }

      template<typename Type>
      auto do_call_impl(const Class *o) const {
        if constexpr (std::is_pointer<Type>::value) {
          return detail::Handle_Return<const Type>::handle(o->*m_attr);
        } else {
          return detail::Handle_Return<typename std::add_lvalue_reference<typename std::add_const<Type>::type>::type>::handle(o->*m_attr);
        }
      }

      static chaiscript::small_vector<Type_Info> param_types() { return {user_type<T>(), user_type<Class>()}; }

      chaiscript::small_vector<Type_Info> m_param_types{user_type<T>(), user_type<Class>()};
      T Class::*m_attr;
    };
  } // namespace dispatch

  namespace exception {
    /// \brief Exception thrown in the case that a method dispatch fails
    ///        because no matching function was found
    ///
    /// May be thrown due to an arity_error, a guard_error or a bad_boxed_cast
    /// exception
    class dispatch_error : public std::runtime_error {
    public:
      dispatch_error(const Function_Params &t_parameters, chaiscript::small_vector<Const_Proxy_Function> t_functions)
          : std::runtime_error("Error with function dispatch")
          , parameters(t_parameters.to_vector())
          , functions(std::move(t_functions)) {
      }

      dispatch_error(const Function_Params &t_parameters, chaiscript::small_vector<Const_Proxy_Function> t_functions, const std::string &t_desc)
          : std::runtime_error(t_desc)
          , parameters(t_parameters.to_vector())
          , functions(std::move(t_functions)) {
      }

      dispatch_error(const dispatch_error &) = default;
      ~dispatch_error() noexcept override = default;

      chaiscript::small_vector<Boxed_Value> parameters;
      chaiscript::small_vector<Const_Proxy_Function> functions;
    };
  } // namespace exception

  namespace dispatch {
    namespace detail {
      template<typename FuncType>
      bool types_match_except_for_arithmetic(const FuncType &t_func,
                                             const chaiscript::Function_Params &plist,
                                             const Type_Conversions_State &t_conversions) noexcept {
        const chaiscript::small_vector<Type_Info> &types = t_func->get_param_types();

        if (t_func->get_arity() == -1) {
          return false;
        }

        assert(plist.size() == types.size() - 1);

        return std::mismatch(plist.begin(),
                             plist.end(),
                             types.begin() + 1,
                             [&](const Boxed_Value &bv, const Type_Info &ti) {
                               return Proxy_Function_Base::compare_type_to_param(ti, bv, t_conversions)
                                   || (bv.get_type_info().is_arithmetic() && ti.is_arithmetic());
                             })
            == std::make_pair(plist.end(), types.end());
      }

      template<typename InItr, typename Funcs>
      Boxed_Value dispatch_with_conversions(InItr begin,
                                            const InItr &end,
                                            const chaiscript::Function_Params &plist,
                                            const Type_Conversions_State &t_conversions,
                                            const Funcs &t_funcs) {
        InItr matching_func(end);

        while (begin != end) {
          if (types_match_except_for_arithmetic(begin->second, plist, t_conversions)) {
            if (matching_func == end) {
              matching_func = begin;
            } else {
              // handle const members vs non-const member, which is not really ambiguous
              const auto &mat_fun_param_types = matching_func->second->get_param_types();
              const auto &next_fun_param_types = begin->second->get_param_types();

              if (plist[0].is_const() && !mat_fun_param_types[1].is_const() && next_fun_param_types[1].is_const()) {
                matching_func = begin; // keep the new one, the const/non-const matchup is correct
              } else if (!plist[0].is_const() && !mat_fun_param_types[1].is_const() && next_fun_param_types[1].is_const()) {
                // keep the old one, it has a better const/non-const matchup
              } else {
                // ambiguous function call
                throw exception::dispatch_error(plist, chaiscript::small_vector<Const_Proxy_Function>(t_funcs.begin(), t_funcs.end()));
              }
            }
          }

          ++begin;
        }

        if (matching_func == end) {
          // no appropriate function to attempt arithmetic type conversion on
          throw exception::dispatch_error(plist, chaiscript::small_vector<Const_Proxy_Function>(t_funcs.begin(), t_funcs.end()));
        }

        chaiscript::small_vector<Boxed_Value> newplist;
        newplist.reserve(plist.size());

        const chaiscript::small_vector<Type_Info> &tis = matching_func->second->get_param_types();
        std::transform(tis.begin() + 1, tis.end(), plist.begin(), std::back_inserter(newplist), [](const Type_Info &ti, const Boxed_Value &param) -> Boxed_Value {
          if (ti.is_arithmetic() && param.get_type_info().is_arithmetic() && param.get_type_info() != ti) {
            return Boxed_Number(param).get_as(ti).bv;
          } else {
            return param;
          }
        });

        try {
          return (*(matching_func->second))(chaiscript::Function_Params{newplist}, t_conversions);
        } catch (const exception::bad_boxed_cast &) {
          // parameter failed to cast
        } catch (const exception::arity_error &) {
          // invalid num params
        } catch (const exception::guard_error &) {
          // guard failed to allow the function to execute
        }

        //--> RG
        try {          
            if (newplist.size() > 0 && newplist[0].is_type(chaiscript::user_type<chaiscript::dispatch::Proxy_Function_Base>())) {                                               
                auto funcPtr = chaiscript::boxed_cast<chaiscript::shared_ptr<const Proxy_Function_Base>>(newplist[0], &t_conversions);
                if (funcPtr && funcPtr->get_arity() == 0) {                   
                    Boxed_Value casted = funcPtr->operator()(chaiscript::Function_Params{ chaiscript::small_vector<Boxed_Value>() }, t_conversions);
                    newplist[0] = casted;
                    return (*(matching_func->second))(chaiscript::Function_Params{ newplist }, t_conversions);
                }
            }
        }
        catch (const exception::bad_boxed_cast&) {
            // parameter failed to cast
        }
        catch (const exception::arity_error&) {
            // invalid num params
        }
        catch (const exception::guard_error&) {
            // guard failed to allow the function to execute
        }
        //<-- RG

        throw exception::dispatch_error(plist, chaiscript::small_vector<Const_Proxy_Function>(t_funcs.begin(), t_funcs.end()));
      }
    } // namespace detail

    /// Take a vector of functions and a vector of parameters. Attempt to execute
    /// each function against the set of parameters, in order, until a matching
    /// function is found or throw dispatch_error if no matching function is found
    template<typename Funcs>
    Boxed_Value dispatch(const Funcs &funcs, const Function_Params &plist, const Type_Conversions_State &t_conversions) {
      chaiscript::small_vector<std::pair<size_t, const Proxy_Function_Base *>> ordered_funcs;
      ordered_funcs.reserve(funcs.size());

      for (const auto &func : funcs) {
        const auto arity = func->get_arity();

        if (arity == -1) {
          ordered_funcs.emplace_back(plist.size(), func.get());
        } else if (arity == static_cast<int>(plist.size())) {
          size_t numdiffs = 0;
          for (size_t i = 0; i < plist.size(); ++i) {
            if (!func->get_param_types()[i + 1].bare_equal(plist[i].get_type_info())) {
              ++numdiffs;
            }
          }
          ordered_funcs.emplace_back(numdiffs, func.get());
        }
      }

      for (size_t i = 0; i <= plist.size(); ++i) {
        for (const auto &func : ordered_funcs) {
          try {
            if (func.first == i && (i == 0 || func.second->filter(plist, t_conversions))) {
              return (*(func.second))(plist, t_conversions);
            }
          } catch (const exception::bad_boxed_cast &) {
            // parameter failed to cast, try again
          } catch (const exception::arity_error &) {
            // invalid num params, try again
          } catch (const exception::guard_error &) {
            // guard failed to allow the function to execute,
            // try again
          }
        }
      }

      return detail::dispatch_with_conversions(ordered_funcs.cbegin(), ordered_funcs.cend(), plist, t_conversions, funcs);
    }
  } // namespace dispatch
} // namespace chaiscript

#endif
