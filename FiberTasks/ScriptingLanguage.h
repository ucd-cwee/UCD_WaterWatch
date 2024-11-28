#pragma once

#include "Fibers.h"

namespace scripting {
	using Type_Info = std::weak_ptr<fibers::Type_Info>;
	using Any = fibers::Any;
	template <typename T> __forceinline Type_Info user_type() {
		return Units::UnitsDetail::user_type<T>();
		// static auto f{ std::make_shared<fibers::Type_Info>(fibers::user_type<T>()) };
		// return f;
	};

	namespace exception {
		/// \brief Thrown in the event that a Boxed_Value cannot be cast to the desired type
		///
		/// It is used internally during function dispatch and may be used by the end user.
		///
		/// \sa chaiscript::boxed_cast
		class bad_boxed_cast : public std::bad_cast {
		public:
			bad_boxed_cast(Type_Info const& t_from, Type_Info const& t_to, std::string t_what) noexcept
				: from(t_from)
				, to(t_to)
				, m_what(std::move(t_what)) {};

			bad_boxed_cast(Type_Info const& t_from, Type_Info const& t_to) noexcept
				: from(t_from)
				, to(t_to)
				, m_what(Units::printf("Cannot perform boxed_cast from %s to %s", GetTypeName(t_from), GetTypeName(t_to)))
			{};

			bad_boxed_cast(Type_Info const& t_from, Type_Info const& t_to, long currentLine) noexcept
				: from(t_from)
				, to(t_to)
				, m_what(Units::printf("Cannot perform boxed_cast from %s to %s at Line %i", GetTypeName(t_from), GetTypeName(t_to), (int)currentLine))
			{};

			explicit bad_boxed_cast(std::string const& t_what) noexcept
				: from(user_type<void>())
				, to(user_type<void>())
				, m_what(t_what) {};

			bad_boxed_cast(const bad_boxed_cast&) noexcept = default;
			bad_boxed_cast(bad_boxed_cast&&) noexcept = default;
			bad_boxed_cast& operator=(const bad_boxed_cast&) noexcept = default;
			bad_boxed_cast& operator=(bad_boxed_cast&&) noexcept = default;
			~bad_boxed_cast() noexcept override = default;

			/// \brief Description of what error occurred
			const char* what() const noexcept override { return m_what.data(); };

			Type_Info from; ///< Type_Info contained in the Boxed_Value
			Type_Info to; ///< std::type_info of the desired (but failed) result type

		private:
			std::string m_what;
			
			static const char* GetTypeName(Type_Info i) {
				if (auto p = i.lock()) {
					return p->raw_name().c_str();
				}
				return "UNK";
			};
		};

		class bad_boxed_type_cast : public bad_boxed_cast {
		public:
			bad_boxed_type_cast(const Type_Info& t_from, const Type_Info& t_to, std::string const& t_what) noexcept
				: bad_boxed_cast(t_from, t_to, t_what) {
			}

			bad_boxed_type_cast(const Type_Info& t_from, const Type_Info& t_to) noexcept
				: bad_boxed_cast(t_from, t_to) {
			}

			explicit bad_boxed_type_cast(std::string const& w) noexcept
				: bad_boxed_cast(w) {
			}

			bad_boxed_type_cast(const bad_boxed_type_cast&) = default;

			~bad_boxed_type_cast() noexcept override = default;
		};

		struct option_explicit_set : std::runtime_error {
			explicit option_explicit_set(const std::string& t_param_name)
				: std::runtime_error("option explicit set and parameter '" + t_param_name + "' does not exist")
			{}

			option_explicit_set(const option_explicit_set&) = default;

			~option_explicit_set() noexcept override = default;
		};

		/**
		* Exception thrown when there is a mismatch in number of
		* parameters during Proxy_Function execution
		*/
		struct arity_error : std::range_error {
			arity_error(int t_got, int t_expected)
				: std::range_error(
					t_expected >= 0 ?
					Units::printf("Arity mismatch: function requires %i parameters, but only %i were provided", t_expected, t_got)
					: std::string("Function was not found")
				)
				, got(t_got)
				, expected(t_expected) {
			}

			arity_error(const arity_error&) = default;

			~arity_error() noexcept override = default;

			int got;
			int expected;
		};

		/**
		* Exception thrown when there is a mismatch in number of
		* parameters during Proxy_Function execution
		*/
		struct not_found_error : std::runtime_error {
			not_found_error(const std::string& triedToFind)
				: std::runtime_error(
					Units::printf("Could not find \"%s\"", triedToFind.c_str())
				), m_triedToFind(triedToFind)
			{}
			not_found_error(const not_found_error&) = default;
			~not_found_error() noexcept override = default;

			std::string m_triedToFind;
		};
	}; // namespace exception

	namespace details {
		// Tuning parameter. Should be larger than the slowest conversion time. Large values encourages fewer conversions. Smaller values encourages faster conversions.
		static constexpr auto TypeConversionBaselineCost = 100.0; 
		static constexpr auto TypeConversionWorstCaseCost = 1000000000000.0;
		class Type_Conversion_Base {
		public:
			// From -> To
			virtual Any convert(const Any& from) const = 0;
			// To -> From
			virtual Any convert_down(const Any& to) const = 0;

			// returns the actual time (in nanoseconds) to perform the conversion
			virtual double cost() const noexcept { return 0; };

			// to type
			const Type_Info& to() const noexcept { return m_to; }

			// from type
			const Type_Info& from() const noexcept { return m_from; }

			// is bidirectional?
			virtual bool bidir() const noexcept { return true; }

			// is polymorphic conversion?
			virtual bool polymorphic() const noexcept { return false; }

			virtual ~Type_Conversion_Base() = default;

		protected:
			Type_Conversion_Base(Type_Info const& t_to, Type_Info const& t_from) : m_to(t_to), m_from(t_from) {}

		private:
			Type_Info m_to;
			Type_Info m_from;
		};

		template<class Callable>
		class Custom_Type_Conversion_Impl : public Type_Conversion_Base {
		public:
			Custom_Type_Conversion_Impl(Callable t_func)
				: Type_Conversion_Base(
					user_type<fibers::utilities::function_traits< decltype(std::function(t_func)) >::result_type>(),
					user_type<std::decay_t<std::tuple_element_t<0, fibers::utilities::function_traits< decltype(std::function(t_func)) >::arguments>>>()
				)
				, m_func(std::move(t_func))
			{};
			Custom_Type_Conversion_Impl(Callable t_func, Type_Info inboundType, Type_Info outboundType)
				: Type_Conversion_Base(
					outboundType,
					inboundType
				)
				, m_func(std::move(t_func))
			{};

			// To -> From
			Any convert_down(const Any&) const override {
				throw exception::bad_boxed_cast("Custom_Type_Conversion_Impl is not bidirectional.");
			};

			// From -> To
			Any convert(const Any& t_from) const override {
				return m_func(t_from.cast()); // we do not know the exact desired input type, so we hope the auto-cast can figure it out.
			};

			bool bidir() const noexcept override { return false; }

			// returns the actual time (in nanoseconds) to perform the conversion
			double cost() const noexcept override {
				static double actualCost{ -1 };
				static std::decay_t<std::tuple_element_t<0, fibers::utilities::function_traits< decltype(std::function(m_func)) >::arguments>> inputObj{};
				if (actualCost < 0) {
					double temp{ 0 };
					for (int i = 0; i < 10; i++) {
						auto startT = clock_ns();
						(void)(m_func(inputObj));
						temp += (double)(clock_ns() - startT) / 100.0;
					}
					actualCost = TypeConversionBaselineCost + temp / 10.0;
				}
				return actualCost;
			};

		private:
			Callable m_func;

		};

		namespace impl {
			template <class From, class To, class = void>
			struct is_explicitly_convertible_to_impl : std::false_type {};

			template <class From, class To>
			struct is_explicitly_convertible_to_impl<
				From, To, std::void_t<decltype(static_cast<To>(std::declval<From>()))>>
				: std::true_type {};

			template <class From, class To>
			struct is_explicitly_convertible_to
				: is_explicitly_convertible_to_impl<From, To> {};

			template <class From, class To>
			inline constexpr bool is_explicitly_convertible_to_v =
				is_explicitly_convertible_to<From, To>::value;

		};

		template<typename From, typename To>
		class Static_Type_Conversion_Impl : public Type_Conversion_Base {
		private:
			constexpr static bool is_bidir = impl::is_explicitly_convertible_to<To, From>::value;

		public:
			Static_Type_Conversion_Impl()
				: Type_Conversion_Base(user_type<To>(), user_type<From>())
			{};

			// To -> From
			Any convert_down(const Any& t_to) const override {
				if constexpr (is_bidir) {
					return (From)(t_to.cast<To&>());
				}
				else {
					throw exception::bad_boxed_cast("Static_Type_Conversion_Impl was not bidirectional.");
				}
			};

			// From -> To
			Any convert(const Any& t_from) const override {
				return (To)(t_from.cast<From&>());
			};

			bool bidir() const noexcept override { return is_bidir; }

			// returns the actual time (in nanoseconds) to perform the conversion
			double cost() const noexcept override {
				static double actualCost{ -1 };
				static From inputObj{};
				if (actualCost < 0) {
					double temp{ 0 };
					for (int i = 0; i < 10; i++) {
						auto startT = clock_ns();
						(void)((To)(inputObj));
						temp += (double)(clock_ns() - startT) / 100.0;
					}
					actualCost = TypeConversionBaselineCost + temp/10.0;
				}
				return actualCost;
			};
		};

		template<typename ChildType, typename BaseType>
		class Dynamic_Type_Conversion_Impl : public Type_Conversion_Base {
		public:
			Dynamic_Type_Conversion_Impl()
				: Type_Conversion_Base(user_type<BaseType>(), user_type<ChildType>())
			{};

			// BaseType -> ChildType
			Any convert_down(const Any& t_to) const override {
				throw exception::bad_boxed_cast("Dynamic_Type_Conversion_Impl is never bidirectional (Base -> Child). Only may cast from (Child -> Base).");
			};

			// ChildType -> BaseType
			Any convert(const Any& t_from) const override {
				std::shared_ptr<ChildType> ptr{ t_from.cast<std::shared_ptr<ChildType>>() };
				return std::dynamic_pointer_cast<BaseType>(ptr);
			};

			bool bidir() const noexcept override { return false; }

			double cost() const noexcept override { return 0; /* Assumes that dynamic casts are free */ };
		};
	};

	// *THREAD-SAFE* Allows conversion from Types (e.g. double -> int, or std::string -> double)
	// If needed, will gladly follow a conversion chain to achieve its desired result. (e.g. std::string -> double -> int -> uint)
	// Assumes that conversions cannot be deleted, but does allow addition of conversion specializations.
	class Type_Converter_Tree {
	public: // private:
		class Node {
		public:
			scripting::Type_Info from;
			// does not support deleting type conversions, but that should be OK, since type conversions should be baked-in.
			fibers::containers::Map<
				scripting::Type_Info, // to
				std::shared_ptr<details::Type_Conversion_Base> // converter function
			> connections; // CONVERT THIS SECOND, SINCE IT (APPARENTLY) DEPENDS ON THE CACHE RETURN TYPE
			// does not support deleting type conversions, but that should be OK, since type conversions should be baked-in.
			mutable fibers::containers::Map <
				scripting::Type_Info, // to
				std::shared_ptr<std::tuple<
				std::vector<scripting::Type_Info> // list of target types to convert to, including the final "to". 
				, long // version of this conversion list
				>>
				> cached_conversions;
		};
		// does not support deleting type conversions, but that should be OK, since type conversions should be baked-in.
		fibers::containers::Map<
			scripting::Type_Info, // from
			std::shared_ptr<Node> // to/connections
		>
			nodes;

		fibers::synchronization::atomic_number<size_t>
			version;
		// fibers::synchronization::impl::InterlockedLong
			// version;

	private:
		// Solves Dijkstra's algorithm to determine the shortest path for "From" to "To", puts the path in "Out", and returns true. 
		// If no path is possible, returns false.
		bool TryCreateConversionPath(scripting::Type_Info const& From, scripting::Type_Info const& To, std::vector<scripting::Type_Info>& out) const {
			out.clear();
			if (nodes.count(From) > 0) {

				std::map<
					scripting::Type_Info, // FROM vertex
					std::map<
					scripting::Type_Info, // TO vertex
					double // distance
					>
				> vertices_and_distances;

				std::map<
					scripting::Type_Info, // FROM vertex
					std::vector<scripting::Type_Info> // predecessors
				> vertices_and_predecessors;

				std::map<
					scripting::Type_Info, // vertex
					double // weight
				> vertices_and_weights;

				std::set< scripting::Type_Info > visited;

				for (auto& node : nodes) {
					if (node) {
						vertices_and_weights[node->first] = std::numeric_limits<double>::max();
						for (auto& connection : node->second->connections) {
							if (connection) {
								vertices_and_weights[connection->first] = std::numeric_limits<double>::max();
								vertices_and_distances[node->first][connection->first] = connection->second->cost();
							}
							else {
								// BAD
							}
						}
					}
					else {
						// BAD
					}
				}

				vertices_and_weights[From] = 0;
				vertices_and_predecessors[From] = {};

				scripting::Type_Info CurrentVertex = From;
				int numToVisit = 1;
				bool FoundEnd = false;
				int countDown = 0;
				std::multimap<double, scripting::Type_Info> sorted;
				while ((visited.size() < vertices_and_distances.size()) && (numToVisit-- >= 1)) {
					// for each adjacent node...
					numToVisit += (vertices_and_distances[CurrentVertex].size() + 1);
					for (auto& connection : vertices_and_distances[CurrentVertex]) {
						// not previously visited...
						if (visited.count(connection.first) == 0) {
							// the distance to the start node must be calculated...
							auto totalDistanceFromStartToThisVertex = vertices_and_weights[CurrentVertex] + connection.second;
							// and update, if it is now the shortest path.
							if (vertices_and_weights[connection.first] > totalDistanceFromStartToThisVertex) {
								vertices_and_weights[connection.first] = totalDistanceFromStartToThisVertex;

								// update the predecessors
								vertices_and_predecessors[connection.first] = vertices_and_predecessors[CurrentVertex];
								vertices_and_predecessors[connection.first].push_back(connection.first);
							}
						}
					}

					// we have visited this node.
					visited.emplace(CurrentVertex);

					// sort the non-visited nodes by weights
					sorted.clear();
					for (auto& vert : vertices_and_weights) {
						if (visited.count(vert.first) == 0) {
							sorted.emplace(vert.second, vert.first);
						}
					}
					if (sorted.size() > 0) {
						CurrentVertex = sorted.begin()->second;
					}
					else {
						// something went wrong
						if (FoundEnd) break;
						else {
							// something *is* wrong
							std::cout << "SOMETHING IS WRONG HERE" << std::endl;
							return false;
						}
					}

					// if we have the (likely) shortest path From -> To...
					if (CurrentVertex == To) {
						FoundEnd = true;
					}
					// otherwise, handle the countDown.
					else {
						if (!FoundEnd) countDown++;
						else if (countDown-- <= 0) break;
					}
				}

				if (vertices_and_predecessors.count(To) > 0) {
					out = vertices_and_predecessors.operator[](To);

					if (auto p1 = From.lock()) {
						if (auto p2 = To.lock()) {
							std::cout << Units::printf("Converting %s -> %s requires:\n\t%s", p1->name(), p2->name(), p1->name());
							for (auto& x : out) {
								if (auto p3 = x.lock()) {
									std::cout << Units::printf(" ... %s", p3->name());
								}
							}
						}
					}
					std::cout << Units::printf(" (%f)\n", (float)vertices_and_weights[To]);

					return true;
				}
			}
			return false;
		};

	public:
		// Tree Version
		long Version() const { return version.GetValue(); };

		// add an automatic static or polymorphic conversion
		template <typename FromType, typename ToType> bool AddConverter() {
			auto fromTypeInfo{ scripting::user_type<FromType>() };
			auto toTypeInfo{ scripting::user_type<ToType>() };

			constexpr static bool is_polymorphic = std::is_base_of< ToType, FromType>::value;
			constexpr static bool is_static = details::impl::is_explicitly_convertible_to<FromType, ToType>::value;
			constexpr static bool is_bidir = details::impl::is_explicitly_convertible_to<ToType, FromType>::value;
			if constexpr (!is_static && !is_polymorphic) {
				return false;
			}

			std::shared_ptr<Node> node = nodes.get_or_insert(fromTypeInfo, std::make_shared<Node>());
			node->from = fromTypeInfo;


			auto targetLocation = node->connections.at_or(toTypeInfo, nullptr);
			if (!targetLocation) {
				if constexpr (std::is_base_of< ToType, FromType>::value) {
					targetLocation = node->connections.get_or_insert(toTypeInfo, std::dynamic_pointer_cast<details::Type_Conversion_Base>(std::make_shared<details::Dynamic_Type_Conversion_Impl<FromType, ToType>>()));
				}
				else {
					targetLocation = node->connections.get_or_insert(toTypeInfo, std::dynamic_pointer_cast<details::Type_Conversion_Base>(std::make_shared<details::Static_Type_Conversion_Impl<FromType, ToType>>()));
				}

				(void)targetLocation->cost(); // cache the cost to perform this conversion

				node->cached_conversions.emplace(toTypeInfo, std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
					std::vector<scripting::Type_Info>({ toTypeInfo }),
					(long)(std::numeric_limits<double>::max())
					)); // even if there was a previous cached conversion, override it.

					// if this converter was bidirectional, we should explicitely add it to the list.
					// This will be slightly recursive but should end abruptly. 
				if constexpr (is_static && is_bidir) {
					AddConverter<ToType, FromType>();
				}

				version++;

				return true;
			}

			return false;
		};

		// adds a customized conversion (e.g. calls a custom function)
		// tree.AddConverter([](float v) -> double { return v; }))
		// tree.AddConverter([](std::string const& v) -> const char* { return v.c_str(); }))
		template <class Callable> bool AddConverter(Callable t_func) {
			auto toTypeInfo = scripting::user_type<fibers::utilities::function_traits< decltype(std::function(t_func)) >::result_type>();
			auto fromTypeInfo = scripting::user_type<std::decay_t<std::tuple_element_t<0, fibers::utilities::function_traits< decltype(std::function(t_func)) >::arguments>>>();

			constexpr static bool is_polymorphic = std::is_base_of<
				fibers::utilities::function_traits< decltype(std::function(t_func)) >::result_type
				, std::decay_t<std::tuple_element_t<0, fibers::utilities::function_traits< decltype(std::function(t_func)) >::arguments>>
			>::value;
			constexpr static bool is_static = details::impl::is_explicitly_convertible_to<
				std::decay_t<std::tuple_element_t<0, fibers::utilities::function_traits< decltype(std::function(t_func)) >::arguments>>
				, fibers::utilities::function_traits< decltype(std::function(t_func)) >::result_type
			>::value;
			if constexpr (is_static || is_polymorphic) {
				// There is a "cheaper" conversion available using built-in static_cast or dynamic_cast.
				// Assumes that the user-provided function is exclusively performing casting, and not other functions (like counting, tracking, or initialization).
				return AddConverter<
					std::decay_t<std::tuple_element_t<0, fibers::utilities::function_traits< decltype(std::function(t_func)) >::arguments>>
					, fibers::utilities::function_traits< decltype(std::function(t_func)) >::result_type
				>();
			}

			std::shared_ptr<Node> node = nodes.get_or_insert(fromTypeInfo, std::make_shared<Node>());
			node->from = fromTypeInfo;

			auto targetLocation = node->connections.at_or(toTypeInfo, nullptr);
			if (!targetLocation) {
				targetLocation = node->connections.get_or_insert(toTypeInfo, std::dynamic_pointer_cast<details::Type_Conversion_Base>(std::make_shared<details::Custom_Type_Conversion_Impl<Callable>>(std::move(t_func))));
			}
			if (1) {
				(void)targetLocation->cost(); // cache the cost to perform this conversion

				node->cached_conversions.emplace(toTypeInfo, std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
					std::vector<scripting::Type_Info>({ toTypeInfo }),
					(long)(std::numeric_limits<double>::max())
					)); // even if there was a previous cached conversion, override it.

				version++;

				return true;
			}
			return false;
		};

		// adds a customized conversion (e.g. calls a custom function)
		// tree.AddConverter([](float v) -> double { return v; }))
		// tree.AddConverter([](std::string const& v) -> const char* { return v.c_str(); }))
		template <class Callable> bool AddConverter(Callable t_func, Type_Info inboundType, Type_Info outboundType) {
			std::shared_ptr<Node> node = nodes.get_or_insert(inboundType, std::make_shared<Node>());
			node->from = inboundType;

			auto targetLocation = node->connections.at_or(outboundType, nullptr);
			if (!targetLocation) {
				targetLocation = node->connections.get_or_insert(outboundType, std::dynamic_pointer_cast<details::Type_Conversion_Base>(
					std::make_shared<details::Custom_Type_Conversion_Impl<Callable>>(std::move(t_func), inboundType, outboundType)
					));
			}
			if (1) {
				(void)targetLocation->cost(); // cache the cost to perform this conversion

				node->cached_conversions.emplace(outboundType, std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
					std::vector<scripting::Type_Info>({ outboundType }),
					(long)(std::numeric_limits<double>::max())
					)); // even if there was a previous cached conversion, override it.

				version++;

				return true;
			}
			return false;
		};

		/// <summary>
		/// returns true if it could convert "From" to "To" type, and stores the converted answer in "result". Otherwise returns false. 
		/// </summary>
		bool TryConvert(Any const& From, scripting::Type_Info const& to, Any& result) const {
			auto fromType = From.Type().lock();
			if (!fromType) return false;

			if (fromType == to) {
				result = From;
				return true;
			}
			else if (to == scripting::user_type<Any>()) {
				result = From;
				return true;
			}
			else {
				if (auto node_ptr = nodes.at(fromType).value_or(nullptr)) {
					auto& node = *node_ptr;

					// If the conversion path does not exist OR is outdated, then re-create it. 
					auto f = node.cached_conversions.at(to).value_or(nullptr);
					if (!f || std::get<1>(*f) < version.GetValue()) {
						std::vector<scripting::Type_Info> newCached;
						{
							if (TryCreateConversionPath(fromType, to, newCached)) {
								node.cached_conversions.insert({ to,
									std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
										newCached,
										version.GetValue()
									)
								}, false);
							}
							else { // cache the failure -- to prevent repeated Dijkstra searches unless the tree is updated to (hopefully) bridge the gap.
								node.cached_conversions.insert({ to,
									std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
										std::vector<scripting::Type_Info>(),
										version.GetValue()
									)
								}, false);
							}
						}
						f = node.cached_conversions.at_or(to, nullptr);
					}

					// try again... hopefully it has been made (for better or worse)					
					if (f) {
						try {
							std::vector<scripting::Type_Info>& conversion_path = std::get<0>(*f);
							if (conversion_path.size() > 0) {
								Any currentFrom = From;
								std::shared_ptr<Node> currentNode = node_ptr;
								for (auto& intermediate_to_type : conversion_path) {
									if (auto f = currentNode->connections.at_or(intermediate_to_type, nullptr)) {
										currentFrom = f->convert(currentFrom);
										if (auto p = currentFrom.Type().lock()) {
											currentNode = nodes.at(p).value_or(nullptr);
										}
										else {
											throw std::runtime_error("Something went wrong with the analysis at " + std::to_string(__LINE__));
										}
									}
									else {
										throw std::runtime_error("Something went wrong with the analysis at " + std::to_string(__LINE__));
									}
								}
								result = currentFrom;
								return true;
							}
						}
						catch (...) {
							node.cached_conversions.insert({ to, 
								std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
									std::vector<scripting::Type_Info>(), 
									version.GetValue()
								) 
							}, false);
							return false;
						}
					}
				}
				return false;
			}
		}

		/// <summary>
		/// Converts "From" to the type of "To" and returns the final conversion. If not possible, then it throws an error. 
		/// </summary>
		Any Convert(Any const& From, scripting::Type_Info const& to) const {
			Any result;
			if (!TryConvert(From, to, result)) {
				throw fibers::exception::bad_any_cast(From.Type(), to);
			}
			return result;
		};

		/// <summary>
		/// Converts "From" to the type of "To" and returns the final conversion. If not possible, then it throws an error. 
		/// </summary>
		template <typename To> To Convert(Any const& From) const { return Convert(From, scripting::user_type<To>()).cast(); };

		// Symbolic "cost" to perform the conversion, in 100's of nanoseconds. Not meant to be precise, but meant to be relative for comparison with other converters.
		double ConversionCost(scripting::Type_Info const& From, scripting::Type_Info const& To) const {
			if (From == To) {
				return 0;
			}
			else if (To == scripting::user_type<Any>()) {
				return 0;
			}
			else {
				if (auto node_ptr = nodes.at(From).value_or(nullptr)) {
					auto& node = *node_ptr;

					// If the conversion path does not exist OR is outdated, then re-create it. 
					auto f = node.cached_conversions.at(To).value_or(nullptr);
					if (!f || std::get<1>(*f) < version.GetValue()) {
						std::vector<scripting::Type_Info> newCached;
						{
							if (TryCreateConversionPath(From, To, newCached)) {
								node.cached_conversions.insert({ To, std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
									newCached,
									version.GetValue()
								) }, false);
							}
							else { // cache the failure -- to prevent repeated Dijkstra searches unless the tree is updated to (hopefully) bridge the gap.
								node.cached_conversions.insert({ To, std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
									std::vector<scripting::Type_Info>(),
									version.GetValue()
								) }, false);
								return std::numeric_limits<double>::max();
							}
						}
						f = node.cached_conversions.at(To).value_or(nullptr);
					}

					if (f) {
						std::vector<scripting::Type_Info>& conversion_path = std::get<0>(*f);
						if (conversion_path.size() > 0) {
							double cost{ 0 };
							std::shared_ptr<Node> currentNode = node_ptr;
							for (auto& intermediate_to_type : conversion_path) {
								if (auto f = currentNode->connections.at_or(intermediate_to_type, nullptr)) {
									cost += f->cost();
									currentNode = nodes.at_hash(std::hash<scripting::Type_Info>()(intermediate_to_type)).value_or(nullptr);
								}
								else {
									throw std::runtime_error("Something went wrong with the analysis at " + std::to_string(__LINE__));
								}
							}
							return cost;
						}
					}
				}
				return std::numeric_limits<double>::max();
			}
		};
		// Symbolic "cost" to perform the conversion, in 100's of nanoseconds. Not meant to be precise, but meant to be relative for comparison with other converters.
		template <typename From, typename To> double ConversionCost() const { return ConversionCost(scripting::user_type<From>(), scripting::user_type<To>()); };

		// true if the tree knows how to convert From into To
		bool Converts(scripting::Type_Info const& From, scripting::Type_Info const& To) const {
			if (From == To) {
				return true;
			}
			else if (To == scripting::user_type<Any>()) {
				return true;
						}
			else {
				if (auto node_ptr = nodes.at(From).value_or(nullptr)) {
					auto& node = *node_ptr;

					// If the conversion path does not exist OR is outdated, then re-create it. 
					auto f = node.cached_conversions.at(To).value_or(nullptr);
					if (!f || std::get<1>(*f) < version.GetValue()) {
						std::vector<scripting::Type_Info> newCached;
						{
							if (TryCreateConversionPath(From, To, newCached)) {
								node.cached_conversions.insert({ To, std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
									newCached,
									version.GetValue()
								) }, false);
							}
							else { // cache the failure -- to prevent repeated Dijkstra searches unless the tree is updated to (hopefully) bridge the gap.
								node.cached_conversions.insert({ To, std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
									std::vector<scripting::Type_Info>(),
									version.GetValue()
								) }, false);
								return false;
							}
						}
						f = node.cached_conversions.at(To).value_or(nullptr);
					}

					// try again... hopefully it has been made (for better or worse)
					if (f) {
						std::vector<scripting::Type_Info>& conversion_path = std::get<0>(*f);
						if (conversion_path.size() > 0) {
							return true;
						}
				}
			}
				return false;
			}
		};
		// true if the tree knows how to convert From into To
		bool Converts(Any& From, scripting::Type_Info const& To) const {
			if (auto p = From.Type().lock()) {
				return Converts(p, To);
			}
			else {
				return Converts(scripting::user_type<void>(), To);
			}
		};
		// true if the tree knows how to convert From into To
		template <typename From, typename To> bool Converts() const { return Converts(scripting::user_type<From>(), scripting::user_type<To>()); };
		// true if the tree knows how to convert From into To
		template <typename To> bool Converts(Any& From) const { return Converts(From, scripting::user_type<To>()); };

		static Type_Converter_Tree Combine(std::vector<const Type_Converter_Tree*> trees) {
			Type_Converter_Tree out;
			if (trees.size() == 1 && trees[0]) out = *trees[0];
			else {
				for (auto tree_ptr = trees.rbegin(); tree_ptr != trees.rend(); tree_ptr++) {
					if (*tree_ptr) {
						auto& tree = **tree_ptr;
						for (auto& node : tree.nodes) {
							if (node) {
								auto NodePtr = out.nodes.get_or_insert(node->first, std::make_shared<Node>());
								NodePtr->from = node->first;
								for (auto& connection : node->second->connections) {
									if (connection) {
										NodePtr->connections.emplace(connection->first, connection->second, true);
										out.version++;
									}
									else {
										// BAD
									}
								}
								for (auto& cache : node->second->cached_conversions) {
									if (cache) {
										NodePtr->cached_conversions.emplace(cache->first, cache->second);
										out.version++;
									}
									else {
										// BAD
									}
								}
							}
							else {
								// BAD
							}
						}
					}
				}
			}

			return out;
		};
	};

	


	// input values to be offered to a function. Does not need to match the function input arguments -- conversions will take place later, if conversions are published to Type_Converter_Tree.
	class Function_Params {
	private:
		size_t HashTypes() {
			constexpr auto A = 54059; /* a prime */
			constexpr auto B = 76963; /* another prime */
			constexpr auto C = 86969; /* yet another prime */
			constexpr auto FIRSTH = 37; /* also prime */

			size_t h = FIRSTH;
			if (size() > 0) {
				for (auto& s : *this) {
					h = (h * A) ^ (std::hash<scripting::Type_Info>()(s.Type()) * B);
				}
			}
			auto result = h % C;
			return result;
		};

	public:
		Function_Params()
			: m_begin(nullptr)
			, m_end(nullptr)
			, hash_value(0)
		{
			hash_value = HashTypes();
		};
		Function_Params(std::vector<Any>& vec)
			: m_begin(vec.empty() ? nullptr : &vec.front())
			, m_end(vec.empty() ? nullptr : &vec.front() + vec.size())
			, hash_value(0)
		{
			hash_value = HashTypes();
		};

		[[nodiscard]] constexpr const Any& operator[](const std::size_t t_i) const noexcept { return m_begin[t_i]; }
		[[nodiscard]] constexpr const Any* begin() const noexcept { return m_begin; }
		[[nodiscard]] constexpr const Any& front() const noexcept { return *m_begin; }
		[[nodiscard]] constexpr const Any* end() const noexcept { return m_end; }
		[[nodiscard]] constexpr std::size_t size() const noexcept { if (m_begin && m_end) return std::size_t(m_end - m_begin); else return 0; }
		[[nodiscard]] std::vector<Any> to_vector() const {
			std::vector<Any> out;
			out.reserve(m_end - m_begin);
			for (const Any* iter = m_begin; (iter < m_end) && iter; iter++)
				out.push_back(*iter);
			return out;

			// return std::vector<Any>{m_begin, m_end};
		};
		[[nodiscard]] constexpr bool empty() const noexcept { return m_begin == m_end; }
		[[nodiscard]] size_t hash() const noexcept { return hash_value; };

	private:
		const Any* m_begin = nullptr;
		const Any* m_end = nullptr;
		size_t hash_value;
	};

	// Converts provided input parameters into the required arguments for the functions. Memorizes the input types for the provided function.
	class Param_Types {
	private:
		static size_t HashTypes(std::vector<std::pair<std::string, Type_Info>> const& source) {
			constexpr auto A = 54059; /* a prime */
			constexpr auto B = 76963; /* another prime */
			constexpr auto C = 86969; /* yet another prime */
			constexpr auto FIRSTH = 37; /* also prime */

			size_t h = FIRSTH;
			for (auto& s : source) {
				h = (h * A) ^ (std::hash<Type_Info>()(s.second) * B);
			}
			auto result = h % C;
			return result;
		};

	public:
		Param_Types(std::vector<std::pair<std::string, Type_Info>> t_types = std::vector<std::pair<std::string, Type_Info>>())
			: m_types(std::move(t_types)), has_template_type(false), hash_value(0)
		{
			for (auto& t : m_types) {
				if (t.second == user_type<fibers::Any>()) {
					has_template_type = true;
					break;
				}
			}

			hash_value = HashTypes(m_types);

		}

		Param_Types(Function_Params const& params, std::vector<std::pair<std::string, Type_Info>> t_types)
			: m_types(std::move(t_types)), has_template_type(false), hash_value(0)
		{
			int index = 0;
			for (auto& paramType : params) {
				if (index < m_types.size()) {
					//if (m_types[index].second == user_type<Any>()) {
					m_types[index].second = paramType.Type();
					//}	
				}
				else {
					m_types.push_back({ Units::printf("Param%i", index), paramType.Type()});
				}
				index++;
			}

			for (auto& t : m_types) {
				if (t.second == user_type<fibers::Any>()) {
					has_template_type = true;
					break;
				}
			}

			hash_value = HashTypes(m_types);
		}

		Param_Types(Param_Types const& params, std::vector<std::string> t_typeNames)
			: m_types(params.m_types), has_template_type(params.has_template_type), hash_value(params.hash_value)
		{
			for (int i = 0; ((i < m_types.size()) && (i < t_typeNames.size())); i++) {
				m_types[i].first = t_typeNames[i];
			}
		}


		void push_front(std::string t_name, Type_Info t_ti) {
			m_types.emplace(m_types.begin(), std::move(t_name), t_ti);
		}
		void push_back(std::string t_name, Type_Info t_ti) {
			m_types.emplace_back(std::move(t_name), t_ti);
		};

		[[nodiscard]] auto& operator[](const std::size_t t_i) const noexcept { return m_types[t_i]; };
		bool operator==(const Param_Types& t_rhs) const noexcept {
			if (t_rhs.size() != size()) return false;
			for (size_t i = 0; i < t_rhs.size(); i++) if (m_types[i].second != t_rhs[i].second) return false;
			return true;
		};
		bool operator!=(const Param_Types& t_rhs) const {
			return !operator==(t_rhs);
		};
		bool operator>(const Param_Types& t_rhs) const {
			if (t_rhs.size() > size()) return true;
			for (size_t i = 0; i < t_rhs.size(); i++) if (m_types[i].second.lock() > t_rhs[i].second.lock()) return true;
			return false;
		};
		bool operator<(const Param_Types& t_rhs) const {
			if (t_rhs.size() < size()) return true;
			for (size_t i = 0; i < t_rhs.size(); i++) if (m_types[i].second.lock() < t_rhs[i].second.lock()) return true;
			return false;
		};
		bool operator>=(const Param_Types& t_rhs) const {
			return !operator<(t_rhs);
		};
		bool operator<=(const Param_Types& t_rhs) const {
			return !operator>(t_rhs);
		};

		// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
		std::vector<Any> convert(Function_Params t_params, Type_Converter_Tree const& t_conversions) const {
			auto vals = t_params.to_vector();
			if (m_types.size() > vals.size()) throw(exception::arity_error(m_types.size(), vals.size()));
			vals.resize(m_types.size()); // become smaller if necessary
			for (size_t i = 0; i < m_types.size(); ++i) {
				const auto& bv = vals[i];
				const auto& ti = m_types[i].second;
				if (auto p = ti.lock()) {
					if (!p->is_undef()) {
						vals[i] = t_conversions.Convert(bv, p); // success or failure, caches the result for faster future eval's
					}
				}
			}
			return vals;
		};

		// Tests if the conversion from the input parameters to the necessary types is possible. 
		bool converts(Function_Params t_params, Type_Converter_Tree const& t_conversions) const {
			// Quick return if the types exactly match.
			if (t_params.hash() == hash()) { return true; }

			if (m_types.size() > t_params.size()) return false;

			for (size_t i = 0; i < m_types.size(); ++i) {
				//const auto& name = m_types[i].first;
				const auto& bv = t_params[i];
				const auto& ti = m_types[i].second;
				if (auto p = ti.lock()) {
					if (!p->is_undef()) {
						if (!t_conversions.Converts(bv.Type(), p)) return false;
					}
				}
			}
			return true;
		};

		// Symbolic "cost" to perform the conversion, in 100's of nanoseconds. Not meant to be precise, but meant to be relative for comparison with other converters.
		double conversion_cost(Function_Params t_params, Type_Converter_Tree const& t_conversions) const {
			double out{ 0 };

			// Quick return if the types exactly match.
			if (t_params.hash() == hash()) { return 0; }

			if (m_types.size() > t_params.size()) return std::numeric_limits<double>::max();
			size_t i = 0;
			for (; i < m_types.size(); ++i) {
				const auto& bv = t_params[i];
				const auto& ti = m_types[i].second;
				if (auto p = ti.lock()) {
					if (!p->is_undef()) {
						if (!t_conversions.Converts(bv.Type(), p)) return std::numeric_limits<double>::max();
						else {
							auto cost = t_conversions.ConversionCost(bv.Type(), p);
							if (cost == std::numeric_limits<double>::max()) return std::numeric_limits<double>::max();
							else out += cost;
						}
					}
				}
			}
			for (; i < t_params.size(); ++i) {
				out += details::TypeConversionWorstCaseCost; // large penalty for not using the provided type(s).
			}
			return out;
		};

		bool Template() const {
			return has_template_type;
		};

		const std::vector<std::pair<std::string, Type_Info>>& types() const noexcept { return m_types; };
		[[nodiscard]] auto begin() const noexcept { return m_types.begin(); };
		[[nodiscard]] auto& front() const noexcept { return *begin(); };
		[[nodiscard]] auto end() const noexcept { return m_types.end(); };
		[[nodiscard]] std::size_t size() const noexcept { return m_types.size(); };
		[[nodiscard]] bool empty() const noexcept { return m_types.size() == 0; };
		[[nodiscard]] size_t hash() const noexcept { return hash_value; };
	private:
		std::vector<std::pair<std::string, Type_Info>> m_types;
		bool has_template_type;
		size_t hash_value;
	};

};

namespace std {
	template <> struct hash<scripting::Function_Params> {
		std::size_t operator()(const scripting::Function_Params& k) const {
			return k.hash();
		};
	};
	template <> struct hash<scripting::Param_Types> {
		std::size_t operator()(const scripting::Param_Types& k) const {
			return k.hash();
		};
	};
};

namespace scripting {

	namespace details {
		/**
		 * Pure virtual base class for all Proxy_Function implementations
		 * Proxy_Functions are a type erasure of type-safe C++ function calls.
		 * At runtime parameter types are expected to be tested against passed in types.
		 * Dispatch_Engine only knows how to work with Proxy_Function, no other
		 * function classes.
		*/
		class Proxy_Function_Base {
		protected:
			Param_Types m_types;
			Type_Info m_return;

		public:
			virtual ~Proxy_Function_Base() = default;

			/// \returns the number of arguments the function takes
			int get_arity() const noexcept { return m_types.size(); }
			const auto& Argument(size_t N) const noexcept { return m_types[N]; };
			const auto& Arguments() const noexcept { return m_types; };
			const Type_Info& ReturnType() const noexcept { return m_return; };
			void SetReturnType(Type_Info const& out) noexcept { m_return = out; };

			// Symbolic "cost" to perform the conversion, in 100's of nanoseconds. Not meant to be precise, but meant to be relative for comparison with other converters.
			double conversion_cost(Function_Params t_params, const Type_Converter_Tree& t_conversions) const {
				return m_types.conversion_cost(t_params, t_conversions);
			};

			Any operator()(const Function_Params& params, const Type_Converter_Tree& t_conversions) const {
				if (params.size() >= m_types.size()) {
					return do_call(convert(params, t_conversions));
				}
				throw exception::arity_error(static_cast<int>(params.size()), m_types.size());
			};

			Any operator()(const Function_Params& params) const {
				if (params.size() == m_types.size()) {
					return do_call(params.to_vector());
				}
				throw exception::arity_error(static_cast<int>(params.size()), m_types.size());
			};


			bool operator==(const Proxy_Function_Base& other) const noexcept {
				return m_types == other.m_types && m_return == other.m_return; // same signature
			};

			bool call_match(const Function_Params& vals, const Type_Converter_Tree& t_conversions) const {
				return m_types.converts(vals, t_conversions);
			};
			// Faster comparison for just the first parameter, to quickly rule-out if this function is available to the boxed value
			bool compare_first_type(Any& bv, const Type_Converter_Tree& t_conversions) const noexcept {
				if (m_types.size() > 0) {
					if (auto p = m_types[0].second.lock()) {
						return t_conversions.Converts(bv, p);
					}
					else {
						return t_conversions.Converts(bv, scripting::user_type<void>());
					}
				}
				else {
					return false;
				}
			}

		protected:
			// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
			std::vector<Any> convert(Function_Params t_params, const Type_Converter_Tree& t_conversions) const {
				return m_types.convert(t_params, t_conversions);
			};

		protected:
			virtual Any do_call(std::vector<Any> const&) const = 0;

			Proxy_Function_Base(Param_Types t_types, Type_Info t_returns)
				: m_types(std::move(t_types))
				, m_return(std::move(t_returns))
			{}
		};

	};

	/// \brief Common typedef used for passing of any registered function in ChaiScript
	using Proxy_Function = std::shared_ptr<details::Proxy_Function_Base>;

	namespace details {
		/**
	 * Use to call member functions or free static functions
	*/
		template <class Callable>
		class Explicit_Function_Impl : public Proxy_Function_Base {
		protected:
			static Param_Types Get_Arg_Type() {
				using argType = typename fibers::utilities::function_traits<decltype(std::function(F_m))>::arguments;
				using returnType = typename fibers::utilities::function_traits<decltype(std::function(F_m))>::result_type;
				static constexpr auto numArgs{ std::tuple_size_v< argType > };

				std::vector<std::pair<std::string, Type_Info>> t_types;
				if constexpr (numArgs > 0) {
					t_types.push_back({ "Param0", user_type<typename std::tuple_element_t<0, argType>>() });
				}
				if constexpr (numArgs > 1) {
					t_types.push_back({ "Param1", user_type<typename std::tuple_element_t<1, argType>>() });
				}
				if constexpr (numArgs > 2) {
					t_types.push_back({ "Param2", user_type<typename std::tuple_element_t<2, argType>>() });
				}
				if constexpr (numArgs > 3) {
					t_types.push_back({ "Param3", user_type<typename std::tuple_element_t<3, argType>>() });
				}
				if constexpr (numArgs > 4) {
					t_types.push_back({ "Param4", user_type<typename std::tuple_element_t<4, argType>>() });
				}
				if constexpr (numArgs > 5) {
					t_types.push_back({ "Param5", user_type<typename std::tuple_element_t<5, argType>>() });
				}
				if constexpr (numArgs > 6) {
					t_types.push_back({ "Param6", user_type<typename std::tuple_element_t<6, argType>>() });
				}
				if constexpr (numArgs > 7) {
					t_types.push_back({ "Param7", user_type<typename std::tuple_element_t<7, argType>>() });
				}
				if constexpr (numArgs > 8) {
					t_types.push_back({ "Param8", user_type<typename std::tuple_element_t<8, argType>>() });
				}
				if constexpr (numArgs > 9) {
					t_types.push_back({ "Param9", user_type<typename std::tuple_element_t<9, argType>>() });
				}
				if constexpr (numArgs > 10) {
					t_types.push_back({ "Param10", user_type<typename std::tuple_element_t<10, argType>>() });
				}
				if constexpr (numArgs > 11) {
					t_types.push_back({ "Param11", user_type<typename std::tuple_element_t<11, argType>>() });
				}
				if constexpr (numArgs > 12) {
					t_types.push_back({ "Param12", user_type<typename std::tuple_element_t<12, argType>>() });
				}
				if constexpr (numArgs > 13) {
					t_types.push_back({ "Param13", user_type<typename std::tuple_element_t<13, argType>>() });
				}
				if constexpr (numArgs > 14) {
					t_types.push_back({ "Param14", user_type<typename std::tuple_element_t<14, argType>>() });
				}
				if constexpr (numArgs > 15) {
					t_types.push_back({ "Param15", user_type<typename std::tuple_element_t<15, argType>>() });
				}
				return Param_Types{ t_types };
			};
			static Type_Info Get_Return_Type() {
				using argType = typename fibers::utilities::function_traits<decltype(std::function(F_m))>::arguments;
				using returnType = typename fibers::utilities::function_traits<decltype(std::function(F_m))>::result_type;
				static constexpr auto numArgs{ std::tuple_size_v< argType > };

				return user_type< returnType >();
			};

		public:
			Explicit_Function_Impl(Callable F_p)
				: Proxy_Function_Base(Get_Arg_Type(), Get_Return_Type())
				, F_m(std::move(F_p))
			{};
			virtual ~Explicit_Function_Impl() = default;

		protected:
			virtual Any do_call(std::vector<Any> const& r) const override {
				using argType = typename fibers::utilities::function_traits<decltype(std::function(F_m))>::arguments;
				using returnType = typename fibers::utilities::function_traits<decltype(std::function(F_m))>::result_type;
				static constexpr auto numArgs{ std::tuple_size_v< argType > };

				if constexpr (std::is_same_v<returnType, void>) {
					static Any temp;
					if constexpr (numArgs == 16) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
						);
					}
					else if constexpr (numArgs == 15) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
						);
					}
					else if constexpr (numArgs == 14) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
						);
					}
					else if constexpr (numArgs == 13) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
						);
					}
					else if constexpr (numArgs == 12) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
						);
					}
					else if constexpr (numArgs == 11) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
						);
					}
					else if constexpr (numArgs == 10) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
						);
					}
					else if constexpr (numArgs == 9) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
						);
					}
					else if constexpr (numArgs == 8) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
						);
					}
					else if constexpr (numArgs == 7) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
						);
					}
					else if constexpr (numArgs == 6) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
						);
					}
					else if constexpr (numArgs == 5) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
						);
					}
					else if constexpr (numArgs == 4) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
						);
					}
					else if constexpr (numArgs == 3) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast()
						);
					}
					else if constexpr (numArgs == 2) {
						F_m(
							r[0].cast(), r[1].cast()
						);
					}
					else if constexpr (numArgs == 1) {
						F_m(
							r[0].cast()
						);
					}
					else if constexpr (numArgs <= 0) {
						F_m();
					}
					return temp;
				}
				else {

					if constexpr (numArgs == 16) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
						);
					}
					else if constexpr (numArgs == 15) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
						);
					}
					else if constexpr (numArgs == 14) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
						);
					}
					else if constexpr (numArgs == 13) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
						);
					}
					else if constexpr (numArgs == 12) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
						);
					}
					else if constexpr (numArgs == 11) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
						);
					}
					else if constexpr (numArgs == 10) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
						);
					}
					else if constexpr (numArgs == 9) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
						);
					}
					else if constexpr (numArgs == 8) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
						);
					}
					else if constexpr (numArgs == 7) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
						);
					}
					else if constexpr (numArgs == 6) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
						);
					}
					else if constexpr (numArgs == 5) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
						);
					}
					else if constexpr (numArgs == 4) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
						);
					}
					else if constexpr (numArgs == 3) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast()
						);
					}
					else if constexpr (numArgs == 2) {
						return F_m(
							r[0].cast(), r[1].cast()
						);
					}
					else if constexpr (numArgs == 1) {
						return F_m(
							r[0].cast()
						);
					}
					else if constexpr (numArgs <= 0) {
						return F_m();
					}
				}

			};
			Callable F_m;
		};

	#if 0
		template <class Callable>
		class Internal_Function_Impl : public Proxy_Function_Base {
		public:
			Internal_Function_Impl(std::vector<std::pair<std::string, Type_Info>> const& inputs, Type_Info const& ExpectedOutcome)
				: Proxy_Function_Base(Param_Types(inputs), ExpectedOutcome)
				, m_inputs(inputs)
				, m_expectedOutcome(ExpectedOutcome)
			{};
			virtual ~Internal_Function_Impl() = default;

		protected:
			virtual Any do_call(std::vector<Any> const& r) const override {





				using argType = typename fibers::utilities::function_traits<decltype(std::function(F_m))>::arguments;
				using returnType = typename fibers::utilities::function_traits<decltype(std::function(F_m))>::result_type;
				static constexpr auto numArgs{ std::tuple_size_v< argType > };




				if constexpr (std::is_same_v<returnType, void>) {
					static Any temp;
					if constexpr (numArgs == 16) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
						);
					}
					else if constexpr (numArgs == 15) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
						);
					}
					else if constexpr (numArgs == 14) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
						);
					}
					else if constexpr (numArgs == 13) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
						);
					}
					else if constexpr (numArgs == 12) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
						);
					}
					else if constexpr (numArgs == 11) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
						);
					}
					else if constexpr (numArgs == 10) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
						);
					}
					else if constexpr (numArgs == 9) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
						);
					}
					else if constexpr (numArgs == 8) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
						);
					}
					else if constexpr (numArgs == 7) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
						);
					}
					else if constexpr (numArgs == 6) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
						);
					}
					else if constexpr (numArgs == 5) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
						);
					}
					else if constexpr (numArgs == 4) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
						);
					}
					else if constexpr (numArgs == 3) {
						F_m(
							r[0].cast(), r[1].cast(), r[2].cast()
						);
					}
					else if constexpr (numArgs == 2) {
						F_m(
							r[0].cast(), r[1].cast()
						);
					}
					else if constexpr (numArgs == 1) {
						F_m(
							r[0].cast()
						);
					}
					else if constexpr (numArgs <= 0) {
						F_m();
					}
					return temp;
				}
				else {

					if constexpr (numArgs == 16) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
						);
					}
					else if constexpr (numArgs == 15) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
						);
					}
					else if constexpr (numArgs == 14) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
						);
					}
					else if constexpr (numArgs == 13) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
						);
					}
					else if constexpr (numArgs == 12) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
						);
					}
					else if constexpr (numArgs == 11) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
						);
					}
					else if constexpr (numArgs == 10) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
						);
					}
					else if constexpr (numArgs == 9) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
						);
					}
					else if constexpr (numArgs == 8) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
						);
					}
					else if constexpr (numArgs == 7) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
						);
					}
					else if constexpr (numArgs == 6) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
						);
					}
					else if constexpr (numArgs == 5) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
						);
					}
					else if constexpr (numArgs == 4) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
						);
					}
					else if constexpr (numArgs == 3) {
						return F_m(
							r[0].cast(), r[1].cast(), r[2].cast()
						);
					}
					else if constexpr (numArgs == 2) {
						return F_m(
							r[0].cast(), r[1].cast()
						);
					}
					else if constexpr (numArgs == 1) {
						return F_m(
							r[0].cast()
						);
					}
					else if constexpr (numArgs <= 0) {
						return F_m();
					}
				}

			};

			std::vector<std::pair<std::string, Type_Info>> m_inputs;
			Type_Info m_expectedOutcome;
		};
	#endif

		/**
		 * Use to call member objects:
		 * struct Test{ public: std::string attr; }
		 * var& func = fibers::details::Attribute_Access_Impl(&Test::attr);
		 * assert(func(Test{ "STR" }).cast<std::string>() == "STR");
		*/
		template <typename T, class Class>
		class Attribute_Access_Impl : public Proxy_Function_Base {
		protected:
			static Param_Types Get_Arg_Type() {
				std::vector<std::pair<std::string, Type_Info>> t_types{ { "parent", user_type<Class>() } };
				return Param_Types{ t_types };
			};
			static Type_Info Get_Return_Type() {
				return user_type< T >();
			};

		public:
			Attribute_Access_Impl(T Class::* t_attr)
				: Proxy_Function_Base(Get_Arg_Type(), Get_Return_Type())
				, m_attr(t_attr)
			{};
			virtual ~Attribute_Access_Impl() = default;

		protected:
			// assumes conversion already happened
			virtual Any do_call(std::vector<Any> const& r) const override {
				if (r.size() < 1) throw(exception::arity_error(0, 1));
				return do_call_impl(r[0].cast<std::shared_ptr<Class>>());
			};

			auto& do_call_impl_impl(Class* o) const {
				return o->*m_attr;
			};

			Any do_call_impl(std::shared_ptr<Class> o) const {
				if constexpr (std::is_same_v<void, T>) {
					// void? Return void.
					return Any();
				}
				else if constexpr (std::is_same_v<Any, typename std::remove_reference_t<T>>) {
					// Any? Return reference to the underlying value, NOT a reference to the Any.
					return do_call_impl_impl(o.get());
				}
				else if constexpr (std::is_pointer<T>::value) {
					// Pointer? Wrap it as a shared pointer.
					using Type = typename std::remove_pointer<T>::type;
					decltype(auto) ptr = do_call_impl_impl(o.get());
					if (ptr) {
						return std::shared_ptr<Type>(ptr, [=](Type*) { (void)o.get(); /* do nothing */ });
					}
					else {
						return Any();
					}
				}
				else {
					// Reference? Wrap it as a shared pointer.
					using Type = typename std::remove_reference<T>::type;
					return std::shared_ptr<Type>(&(do_call_impl_impl(o.get())), [=](Type*) { (void)o.get(); /* do nothing */ });
				}
			};

			T Class::* m_attr;
		};

		namespace detail {
			/**
			 * Use to call member functions:
			 * struct Test{ public: std::string attr(){ return "TEST"; }; }
			 * var& func = fibers::details::Attribute_Access_Impl(&Test::attr);
			 * assert(func(Test{}).cast<std::string>() == "TEST");
			*/
			template <typename R, typename Class, typename... T>
			class VolatileConst_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				static Param_Types Get_Arg_Type() {
					std::vector<std::pair<std::string, Type_Info>> t_types{ { "parent", user_type<Class>() } };
					if constexpr (numArgs > 0) {
						t_types.push_back({ "Param0", user_type<typename std::tuple_element_t<1, argType>>() });
					}
					if constexpr (numArgs > 1) {
						t_types.push_back({ "Param1", user_type<typename std::tuple_element_t<2, argType>>() });
					}
					if constexpr (numArgs > 2) {
						t_types.push_back({ "Param2", user_type<typename std::tuple_element_t<3, argType>>() });
					}
					if constexpr (numArgs > 3) {
						t_types.push_back({ "Param3", user_type<typename std::tuple_element_t<4, argType>>() });
					}
					if constexpr (numArgs > 4) {
						t_types.push_back({ "Param4", user_type<typename std::tuple_element_t<5, argType>>() });
					}
					if constexpr (numArgs > 5) {
						t_types.push_back({ "Param5", user_type<typename std::tuple_element_t<6, argType>>() });
					}
					if constexpr (numArgs > 6) {
						t_types.push_back({ "Param6", user_type<typename std::tuple_element_t<7, argType>>() });
					}
					if constexpr (numArgs > 7) {
						t_types.push_back({ "Param7", user_type<typename std::tuple_element_t<8, argType>>() });
					}
					if constexpr (numArgs > 8) {
						t_types.push_back({ "Param8", user_type<typename std::tuple_element_t<9, argType>>() });
					}
					if constexpr (numArgs > 9) {
						t_types.push_back({ "Param9", user_type<typename std::tuple_element_t<10, argType>>() });
					}
					if constexpr (numArgs > 10) {
						t_types.push_back({ "Param10", user_type<typename std::tuple_element_t<11, argType>>() });
					}
					if constexpr (numArgs > 11) {
						t_types.push_back({ "Param11", user_type<typename std::tuple_element_t<12, argType>>() });
					}
					if constexpr (numArgs > 12) {
						t_types.push_back({ "Param12", user_type<typename std::tuple_element_t<13, argType>>() });
					}
					if constexpr (numArgs > 13) {
						t_types.push_back({ "Param13", user_type<typename std::tuple_element_t<14, argType>>() });
					}
					if constexpr (numArgs > 14) {
						t_types.push_back({ "Param14", user_type<typename std::tuple_element_t<15, argType>>() });
					}
					if constexpr (numArgs > 15) {
						t_types.push_back({ "Param15", user_type<typename std::tuple_element_t<16, argType>>() });
					}
					return Param_Types{ t_types };
				};
				static Type_Info Get_Return_Type() {
					return user_type< R >();
				};

			public:
				VolatileConst_Member_Function_Impl(R(Class::* f)(T...) volatile const)
					: Proxy_Function_Base(Get_Arg_Type(), Get_Return_Type())
					, m_attr(std::move(f)) {};
				virtual ~VolatileConst_Member_Function_Impl() = default;

			protected:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					if (r.size() < (numArgs + 1)) throw(exception::arity_error(r.size(), numArgs + 1));

					std::vector<Any> temp;
					for (int i = 1; i < r.size(); i++) temp.push_back(r[i]);

					return do_call_impl(r[0].cast<std::shared_ptr<Class>>(), temp);
				};

				decltype(auto) do_call_impl_impl(Class* o, std::vector<Any> const& r) const {
					if constexpr (numArgs == 0) {
						return (R)(o->*m_attr)();
					}
					else if constexpr (numArgs == 1) {
						return (R)(o->*m_attr)(
							r[0].cast()
							);
					}
					else if constexpr (numArgs == 2) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast()
							);
					}
					else if constexpr (numArgs == 3) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast()
							);
					}
					else if constexpr (numArgs == 4) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
					}
					else if constexpr (numArgs == 5) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
							);
					}
					else if constexpr (numArgs == 6) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
							);
					}
					else if constexpr (numArgs == 7) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
							);
					}
					else if constexpr (numArgs == 8) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
					}
					else if constexpr (numArgs == 9) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
							);
					}
					else if constexpr (numArgs == 10) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
							);
					}
					else if constexpr (numArgs == 11) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
							);
					}
					else if constexpr (numArgs == 12) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
					}
					else if constexpr (numArgs == 13) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
							);
					}
					else if constexpr (numArgs == 14) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
							);
					}
					else if constexpr (numArgs == 15) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
							);
					}
					else if constexpr (numArgs == 16) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
					}
				};

				Any do_call_impl(std::shared_ptr<Class> o, std::vector<Any> const& r) const {
					if constexpr (std::is_same_v<void, R>) {
						do_call_impl_impl(o.get(), r);
						return Any();
					}
					else {
						R returned_obj{ do_call_impl_impl(o.get(), r) };
						using Type = typename std::decay_t<decltype(returned_obj)>;

						if constexpr (std::is_same_v<Any, typename std::remove_reference_t<Type>>) {
							// Any? Return reference to the underlying value, NOT a reference to the Any.
							return returned_obj;
						}
						else if constexpr (std::is_pointer<R>::value) {
							// Pointer? Wrap it as a shared pointer.
							using Type2 = typename std::remove_pointer<R>::type;
							if (returned_obj) {
								return std::shared_ptr<Type2>(returned_obj, [=](Type2*) { (void)o.get(); /* do nothing */ });
							}
							else {
								return Any();
							}
						}
						else if constexpr (std::is_reference<R>::value) {
							// Reference? Wrap it as a shared pointer.
							using Type2 = typename std::remove_reference<R>::type;
							return std::shared_ptr<Type2>(&returned_obj, [=](Type2*) { (void)o.get(); /* do nothing */ });
						}
						else {
							return std::move(returned_obj);
						}
					}
				};

				R(Class::* m_attr)(T...) volatile const;
			};
			template <typename R, typename Class, typename... T>
			class Volatile_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				static Param_Types Get_Arg_Type() {
					std::vector<std::pair<std::string, Type_Info>> t_types{ { "parent", user_type<Class>() } };
					if constexpr (numArgs > 0) {
						t_types.push_back({ "Param0", user_type<typename std::tuple_element_t<1, argType>>() });
					}
					if constexpr (numArgs > 1) {
						t_types.push_back({ "Param1", user_type<typename std::tuple_element_t<2, argType>>() });
					}
					if constexpr (numArgs > 2) {
						t_types.push_back({ "Param2", user_type<typename std::tuple_element_t<3, argType>>() });
					}
					if constexpr (numArgs > 3) {
						t_types.push_back({ "Param3", user_type<typename std::tuple_element_t<4, argType>>() });
					}
					if constexpr (numArgs > 4) {
						t_types.push_back({ "Param4", user_type<typename std::tuple_element_t<5, argType>>() });
					}
					if constexpr (numArgs > 5) {
						t_types.push_back({ "Param5", user_type<typename std::tuple_element_t<6, argType>>() });
					}
					if constexpr (numArgs > 6) {
						t_types.push_back({ "Param6", user_type<typename std::tuple_element_t<7, argType>>() });
					}
					if constexpr (numArgs > 7) {
						t_types.push_back({ "Param7", user_type<typename std::tuple_element_t<8, argType>>() });
					}
					if constexpr (numArgs > 8) {
						t_types.push_back({ "Param8", user_type<typename std::tuple_element_t<9, argType>>() });
					}
					if constexpr (numArgs > 9) {
						t_types.push_back({ "Param9", user_type<typename std::tuple_element_t<10, argType>>() });
					}
					if constexpr (numArgs > 10) {
						t_types.push_back({ "Param10", user_type<typename std::tuple_element_t<11, argType>>() });
					}
					if constexpr (numArgs > 11) {
						t_types.push_back({ "Param11", user_type<typename std::tuple_element_t<12, argType>>() });
					}
					if constexpr (numArgs > 12) {
						t_types.push_back({ "Param12", user_type<typename std::tuple_element_t<13, argType>>() });
					}
					if constexpr (numArgs > 13) {
						t_types.push_back({ "Param13", user_type<typename std::tuple_element_t<14, argType>>() });
					}
					if constexpr (numArgs > 14) {
						t_types.push_back({ "Param14", user_type<typename std::tuple_element_t<15, argType>>() });
					}
					if constexpr (numArgs > 15) {
						t_types.push_back({ "Param15", user_type<typename std::tuple_element_t<16, argType>>() });
					}
					return Param_Types{ t_types };
				};
				static Type_Info Get_Return_Type() {
					return user_type< R >();
				};

			public:
				Volatile_Member_Function_Impl(R(Class::* f)(T...) volatile)
					: Proxy_Function_Base(Get_Arg_Type(), Get_Return_Type())
					, m_attr(std::move(f)) {};
				virtual ~Volatile_Member_Function_Impl() = default;

			protected:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					if (r.size() < (numArgs + 1)) throw(exception::arity_error(r.size(), numArgs + 1));

					std::vector<Any> temp;
					for (int i = 1; i < r.size(); i++) temp.push_back(r[i]);

					return do_call_impl(r[0].cast<std::shared_ptr<Class>>(), temp);
				};

				decltype(auto) do_call_impl_impl(Class* o, std::vector<Any> const& r) const {
					if constexpr (numArgs == 0) {
						return (R)(o->*m_attr)();
					}
					else if constexpr (numArgs == 1) {
						return (R)(o->*m_attr)(
							r[0].cast()
							);
					}
					else if constexpr (numArgs == 2) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast()
							);
					}
					else if constexpr (numArgs == 3) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast()
							);
					}
					else if constexpr (numArgs == 4) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
					}
					else if constexpr (numArgs == 5) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
							);
					}
					else if constexpr (numArgs == 6) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
							);
					}
					else if constexpr (numArgs == 7) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
							);
					}
					else if constexpr (numArgs == 8) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
					}
					else if constexpr (numArgs == 9) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
							);
					}
					else if constexpr (numArgs == 10) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
							);
					}
					else if constexpr (numArgs == 11) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
							);
					}
					else if constexpr (numArgs == 12) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
					}
					else if constexpr (numArgs == 13) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
							);
					}
					else if constexpr (numArgs == 14) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
							);
					}
					else if constexpr (numArgs == 15) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
							);
					}
					else if constexpr (numArgs == 16) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
					}
				};

				Any do_call_impl(std::shared_ptr<Class> o, std::vector<Any> const& r) const {
					if constexpr (std::is_same_v<void, R>) {
						do_call_impl_impl(o.get(), r);
						return Any();
					}
					else {
						R returned_obj{ do_call_impl_impl(o.get(), r) };
						using Type = typename std::decay_t<decltype(returned_obj)>;

						if constexpr (std::is_same_v<Any, typename std::remove_reference_t<Type>>) {
							// Any? Return reference to the underlying value, NOT a reference to the Any.
							return returned_obj;
						}
						else if constexpr (std::is_pointer<R>::value) {
							// Pointer? Wrap it as a shared pointer.
							using Type2 = typename std::remove_pointer<R>::type;
							if (returned_obj) {
								return std::shared_ptr<Type2>(returned_obj, [=](Type2*) { (void)o.get(); /* do nothing */ });
							}
							else {
								return Any();
							}
						}
						else if constexpr (std::is_reference<R>::value) {
							// Reference? Wrap it as a shared pointer.
							using Type2 = typename std::remove_reference<R>::type;
							return std::shared_ptr<Type2>(&returned_obj, [=](Type2*) { (void)o.get(); /* do nothing */ });
						}
						else {
							return std::move(returned_obj);
						}
					}
				};

				R(Class::* m_attr)(T...) volatile;
			};
			template <typename R, typename Class, typename... T>
			class Const_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				static Param_Types Get_Arg_Type() {
					std::vector<std::pair<std::string, Type_Info>> t_types{ { "parent", user_type<Class>() } };
					if constexpr (numArgs > 0) {
						t_types.push_back({ "Param0", user_type<typename std::tuple_element_t<1, argType>>() });
					}
					if constexpr (numArgs > 1) {
						t_types.push_back({ "Param1", user_type<typename std::tuple_element_t<2, argType>>() });
					}
					if constexpr (numArgs > 2) {
						t_types.push_back({ "Param2", user_type<typename std::tuple_element_t<3, argType>>() });
					}
					if constexpr (numArgs > 3) {
						t_types.push_back({ "Param3", user_type<typename std::tuple_element_t<4, argType>>() });
					}
					if constexpr (numArgs > 4) {
						t_types.push_back({ "Param4", user_type<typename std::tuple_element_t<5, argType>>() });
					}
					if constexpr (numArgs > 5) {
						t_types.push_back({ "Param5", user_type<typename std::tuple_element_t<6, argType>>() });
					}
					if constexpr (numArgs > 6) {
						t_types.push_back({ "Param6", user_type<typename std::tuple_element_t<7, argType>>() });
					}
					if constexpr (numArgs > 7) {
						t_types.push_back({ "Param7", user_type<typename std::tuple_element_t<8, argType>>() });
					}
					if constexpr (numArgs > 8) {
						t_types.push_back({ "Param8", user_type<typename std::tuple_element_t<9, argType>>() });
					}
					if constexpr (numArgs > 9) {
						t_types.push_back({ "Param9", user_type<typename std::tuple_element_t<10, argType>>() });
					}
					if constexpr (numArgs > 10) {
						t_types.push_back({ "Param10", user_type<typename std::tuple_element_t<11, argType>>() });
					}
					if constexpr (numArgs > 11) {
						t_types.push_back({ "Param11", user_type<typename std::tuple_element_t<12, argType>>() });
					}
					if constexpr (numArgs > 12) {
						t_types.push_back({ "Param12", user_type<typename std::tuple_element_t<13, argType>>() });
					}
					if constexpr (numArgs > 13) {
						t_types.push_back({ "Param13", user_type<typename std::tuple_element_t<14, argType>>() });
					}
					if constexpr (numArgs > 14) {
						t_types.push_back({ "Param14", user_type<typename std::tuple_element_t<15, argType>>() });
					}
					if constexpr (numArgs > 15) {
						t_types.push_back({ "Param15", user_type<typename std::tuple_element_t<16, argType>>() });
					}
					return Param_Types{ t_types };
				};
				static Type_Info Get_Return_Type() {
					return user_type< R >();
				};

			public:
				Const_Member_Function_Impl(R(Class::* f)(T...) const)
					: Proxy_Function_Base(Get_Arg_Type(), Get_Return_Type())
					, m_attr(std::move(f)) {};
				virtual ~Const_Member_Function_Impl() = default;

			protected:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					if (r.size() < (numArgs + 1)) throw(exception::arity_error(r.size(), numArgs + 1));

					std::vector<Any> temp;
					for (int i = 1; i < r.size(); i++) temp.push_back(r[i]);

					return do_call_impl(r[0].cast<std::shared_ptr<Class>>(), temp);
				};

				decltype(auto) do_call_impl_impl(Class* o, std::vector<Any> const& r) const {
					if constexpr (numArgs == 0) {
						return (R)(o->*m_attr)();
					}
					else if constexpr (numArgs == 1) {
						return (R)(o->*m_attr)(
							r[0].cast()
							);
					}
					else if constexpr (numArgs == 2) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast()
							);
					}
					else if constexpr (numArgs == 3) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast()
							);
					}
					else if constexpr (numArgs == 4) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
					}
					else if constexpr (numArgs == 5) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
							);
					}
					else if constexpr (numArgs == 6) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
							);
					}
					else if constexpr (numArgs == 7) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
							);
					}
					else if constexpr (numArgs == 8) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
					}
					else if constexpr (numArgs == 9) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
							);
					}
					else if constexpr (numArgs == 10) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
							);
					}
					else if constexpr (numArgs == 11) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
							);
					}
					else if constexpr (numArgs == 12) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
					}
					else if constexpr (numArgs == 13) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
							);
					}
					else if constexpr (numArgs == 14) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
							);
					}
					else if constexpr (numArgs == 15) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
							);
					}
					else if constexpr (numArgs == 16) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
					}
				};

				Any do_call_impl(std::shared_ptr<Class> o, std::vector<Any> const& r) const {
					if constexpr (std::is_same_v<void, R>) {
						do_call_impl_impl(o.get(), r);
						return Any();
					}
					else {
						R returned_obj{ do_call_impl_impl(o.get(), r) };
						using Type = typename std::decay_t<decltype(returned_obj)>;

						if constexpr (std::is_same_v<Any, typename std::remove_reference_t<Type>>) {
							// Any? Return reference to the underlying value, NOT a reference to the Any.
							return returned_obj;
						}
						else if constexpr (std::is_pointer<R>::value) {
							// Pointer? Wrap it as a shared pointer.
							using Type2 = typename std::remove_pointer<R>::type;
							if (returned_obj) {
								return std::shared_ptr<Type2>(returned_obj, [=](Type2*) { (void)o.get(); /* do nothing */ });
							}
							else {
								return Any();
							}
						}
						else if constexpr (std::is_reference<R>::value) {
							// Reference? Wrap it as a shared pointer.
							using Type2 = typename std::remove_reference<R>::type;
							return std::shared_ptr<Type2>(&returned_obj, [=](Type2*) { (void)o.get(); /* do nothing */ });
						}
						else {
							return std::move(returned_obj);
						}
					}
				};

				R(Class::* m_attr)(T...) const;
			};
			template <typename R, typename Class, typename... T>
			class Default_Member_Function_Impl : public Proxy_Function_Base {
			public:
				using argType = std::tuple<Class, T...>;
				static constexpr auto numArgs = std::tuple_size_v<argType> -1;
				static Param_Types Get_Arg_Type() {
					std::vector<std::pair<std::string, Type_Info>> t_types{ { "parent", user_type<Class>() } };
					if constexpr (numArgs > 0) {
						t_types.push_back({ "Param0", user_type<typename std::tuple_element_t<1, argType>>() });
					}
					if constexpr (numArgs > 1) {
						t_types.push_back({ "Param1", user_type<typename std::tuple_element_t<2, argType>>() });
					}
					if constexpr (numArgs > 2) {
						t_types.push_back({ "Param2", user_type<typename std::tuple_element_t<3, argType>>() });
					}
					if constexpr (numArgs > 3) {
						t_types.push_back({ "Param3", user_type<typename std::tuple_element_t<4, argType>>() });
					}
					if constexpr (numArgs > 4) {
						t_types.push_back({ "Param4", user_type<typename std::tuple_element_t<5, argType>>() });
					}
					if constexpr (numArgs > 5) {
						t_types.push_back({ "Param5", user_type<typename std::tuple_element_t<6, argType>>() });
					}
					if constexpr (numArgs > 6) {
						t_types.push_back({ "Param6", user_type<typename std::tuple_element_t<7, argType>>() });
					}
					if constexpr (numArgs > 7) {
						t_types.push_back({ "Param7", user_type<typename std::tuple_element_t<8, argType>>() });
					}
					if constexpr (numArgs > 8) {
						t_types.push_back({ "Param8", user_type<typename std::tuple_element_t<9, argType>>() });
					}
					if constexpr (numArgs > 9) {
						t_types.push_back({ "Param9", user_type<typename std::tuple_element_t<10, argType>>() });
					}
					if constexpr (numArgs > 10) {
						t_types.push_back({ "Param10", user_type<typename std::tuple_element_t<11, argType>>() });
					}
					if constexpr (numArgs > 11) {
						t_types.push_back({ "Param11", user_type<typename std::tuple_element_t<12, argType>>() });
					}
					if constexpr (numArgs > 12) {
						t_types.push_back({ "Param12", user_type<typename std::tuple_element_t<13, argType>>() });
					}
					if constexpr (numArgs > 13) {
						t_types.push_back({ "Param13", user_type<typename std::tuple_element_t<14, argType>>() });
					}
					if constexpr (numArgs > 14) {
						t_types.push_back({ "Param14", user_type<typename std::tuple_element_t<15, argType>>() });
					}
					if constexpr (numArgs > 15) {
						t_types.push_back({ "Param15", user_type<typename std::tuple_element_t<16, argType>>() });
					}
					return Param_Types{ t_types };
				};
				static Type_Info Get_Return_Type() {
					return user_type< R >();
				};

			public:
				Default_Member_Function_Impl(R(Class::* f)(T...))
					: Proxy_Function_Base(Get_Arg_Type(), Get_Return_Type())
					, m_attr(std::move(f)) {};
				virtual ~Default_Member_Function_Impl() = default;

			private:
				// assumes conversion already happened
				virtual Any do_call(std::vector<Any> const& r) const override {
					if (r.size() < (numArgs + 1)) throw(exception::arity_error(r.size(), numArgs + 1));

					std::vector<Any> temp;
					for (int i = 1; i < r.size(); i++) temp.push_back(r[i]);

					return do_call_impl(r[0].cast<std::shared_ptr<Class>>(), temp);
				};

				decltype(auto) do_call_impl_impl(Class* o, std::vector<Any> const& r) const {
					if constexpr (numArgs == 0) {
						return (R)(o->*m_attr)();
					}
					else if constexpr (numArgs == 1) {
						return (R)(o->*m_attr)(
							r[0].cast()
							);
					}
					else if constexpr (numArgs == 2) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast()
							);
					}
					else if constexpr (numArgs == 3) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast()
							);
					}
					else if constexpr (numArgs == 4) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
							);
					}
					else if constexpr (numArgs == 5) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast()
							);
					}
					else if constexpr (numArgs == 6) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast()
							);
					}
					else if constexpr (numArgs == 7) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast()
							);
					}
					else if constexpr (numArgs == 8) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
							);
					}
					else if constexpr (numArgs == 9) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast()
							);
					}
					else if constexpr (numArgs == 10) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast()
							);
					}
					else if constexpr (numArgs == 11) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast()
							);
					}
					else if constexpr (numArgs == 12) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
							);
					}
					else if constexpr (numArgs == 13) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast()
							);
					}
					else if constexpr (numArgs == 14) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast()
							);
					}
					else if constexpr (numArgs == 15) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast()
							);
					}
					else if constexpr (numArgs == 16) {
						return (R)(o->*m_attr)(
							r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
							r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
							r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
							r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
							);
					}
				};

				Any do_call_impl(std::shared_ptr<Class> o, std::vector<Any> const& r) const {
					if constexpr (std::is_same_v<void, R>) {
						do_call_impl_impl(o.get(), r);
						return Any();
					}
					else {
						R returned_obj{ do_call_impl_impl(o.get(), r) };
						using Type = typename std::decay_t<decltype(returned_obj)>;

						if constexpr (std::is_same_v<Any, typename std::remove_reference_t<Type>>) {
							// Any? Return reference to the underlying value, NOT a reference to the Any.
							return returned_obj;
						}
						else if constexpr (std::is_pointer<R>::value) {
							// Pointer? Wrap it as a shared pointer.
							using Type2 = typename std::remove_pointer<R>::type;
							if (returned_obj) {
								return std::shared_ptr<Type2>(returned_obj, [=](Type2*) { (void)o.get(); /* do nothing */ });
							}
							else {
								return Any();
							}
						}
						else if constexpr (std::is_reference<R>::value) {
							// Reference? Wrap it as a shared pointer.
							using Type2 = typename std::remove_reference<R>::type;
							return std::shared_ptr<Type2>(&returned_obj, [=](Type2*) { (void)o.get(); /* do nothing */ });
						}
						else {
							return std::move(returned_obj);
						}
					}
				};

				R(Class::* m_attr)(T...);
			};
		};

		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) volatile const) {
			auto* function_impl = new detail::VolatileConst_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) volatile) {
			auto* function_impl = new detail::Volatile_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...) const) {
			auto* function_impl = new detail::Const_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};
		template<typename Ret, typename Class, typename... Param>
		Proxy_Function Member_Function_Impl(Ret(Class::* f)(Param...)) {
			auto* function_impl = new detail::Default_Member_Function_Impl(f);
			auto ptr{ std::static_pointer_cast<Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		};

		/**
		 * Use to call member functions:
		 * struct Test{ public: std::string attr(){ return "TEST"; }; }
		 * var& func = fibers::details::Attribute_Access_Impl(&Test::attr);
		 * assert(func(Test{}).cast<std::string>() == "TEST");
		*/
		template <typename R, typename... T>
		class Static_Function_Impl : public Proxy_Function_Base {
		public:
			using argType = std::tuple<R, T...>;
			static constexpr auto numArgs = std::tuple_size_v<argType> -1;
			static Param_Types Get_Arg_Type() {
				std::vector<std::pair<std::string, Type_Info>> t_types{};
				if constexpr (numArgs > 0) {
					t_types.push_back({ "Param0", user_type<typename std::tuple_element_t<1, argType>>() });
				}
				if constexpr (numArgs > 1) {
					t_types.push_back({ "Param1", user_type<typename std::tuple_element_t<2, argType>>() });
				}
				if constexpr (numArgs > 2) {
					t_types.push_back({ "Param2", user_type<typename std::tuple_element_t<3, argType>>() });
				}
				if constexpr (numArgs > 3) {
					t_types.push_back({ "Param3", user_type<typename std::tuple_element_t<4, argType>>() });
				}
				if constexpr (numArgs > 4) {
					t_types.push_back({ "Param4", user_type<typename std::tuple_element_t<5, argType>>() });
				}
				if constexpr (numArgs > 5) {
					t_types.push_back({ "Param5", user_type<typename std::tuple_element_t<6, argType>>() });
				}
				if constexpr (numArgs > 6) {
					t_types.push_back({ "Param6", user_type<typename std::tuple_element_t<7, argType>>() });
				}
				if constexpr (numArgs > 7) {
					t_types.push_back({ "Param7", user_type<typename std::tuple_element_t<8, argType>>() });
				}
				if constexpr (numArgs > 8) {
					t_types.push_back({ "Param8", user_type<typename std::tuple_element_t<9, argType>>() });
				}
				if constexpr (numArgs > 9) {
					t_types.push_back({ "Param9", user_type<typename std::tuple_element_t<10, argType>>() });
				}
				if constexpr (numArgs > 10) {
					t_types.push_back({ "Param10", user_type<typename std::tuple_element_t<11, argType>>() });
				}
				if constexpr (numArgs > 11) {
					t_types.push_back({ "Param11", user_type<typename std::tuple_element_t<12, argType>>() });
				}
				if constexpr (numArgs > 12) {
					t_types.push_back({ "Param12", user_type<typename std::tuple_element_t<13, argType>>() });
				}
				if constexpr (numArgs > 13) {
					t_types.push_back({ "Param13", user_type<typename std::tuple_element_t<14, argType>>() });
				}
				if constexpr (numArgs > 14) {
					t_types.push_back({ "Param14", user_type<typename std::tuple_element_t<15, argType>>() });
				}
				if constexpr (numArgs > 15) {
					t_types.push_back({ "Param15", user_type<typename std::tuple_element_t<16, argType>>() });
				}
				return Param_Types{ t_types };
			};
			static Type_Info Get_Return_Type() {
				return user_type< R >();
			};

		public:
			Static_Function_Impl(R(*f)(T...))
				: Proxy_Function_Base(Get_Arg_Type(), Get_Return_Type())
				, m_attr(std::move(f)) {};
			virtual ~Static_Function_Impl() = default;

		protected:
			// assumes conversion already happened
			virtual Any do_call(std::vector<Any> const& r) const override {
				if (r.size() < numArgs) throw(exception::arity_error(r.size(), numArgs));
				return do_call_impl(r);
			};

			decltype(auto) do_call_impl_impl(std::vector<Any> const& r) const {
				if constexpr (numArgs == 0) {
					return (*m_attr)();
				}
				else if constexpr (numArgs == 1) {
					return (*m_attr)(
						r[0].cast()
						);
				}
				else if constexpr (numArgs == 2) {
					return (*m_attr)(
						r[0].cast(), r[1].cast()
						);
				}
				else if constexpr (numArgs == 3) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast()
						);
				}
				else if constexpr (numArgs == 4) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast()
						);
				}
				else if constexpr (numArgs == 5) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast()
						);
				}
				else if constexpr (numArgs == 6) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast()
						);
				}
				else if constexpr (numArgs == 7) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast()
						);
				}
				else if constexpr (numArgs == 8) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast()
						);
				}
				else if constexpr (numArgs == 9) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
						r[8].cast()
						);
				}
				else if constexpr (numArgs == 10) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
						r[8].cast(), r[9].cast()
						);
				}
				else if constexpr (numArgs == 11) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
						r[8].cast(), r[9].cast(), r[10].cast()
						);
				}
				else if constexpr (numArgs == 12) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
						r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast()
						);
				}
				else if constexpr (numArgs == 13) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
						r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
						r[12].cast()
						);
				}
				else if constexpr (numArgs == 14) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
						r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
						r[12].cast(), r[13].cast()
						);
				}
				else if constexpr (numArgs == 15) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
						r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
						r[12].cast(), r[13].cast(), r[14].cast()
						);
				}
				else if constexpr (numArgs == 16) {
					return (*m_attr)(
						r[0].cast(), r[1].cast(), r[2].cast(), r[3].cast(),
						r[4].cast(), r[5].cast(), r[6].cast(), r[7].cast(),
						r[8].cast(), r[9].cast(), r[10].cast(), r[11].cast(),
						r[12].cast(), r[13].cast(), r[14].cast(), r[15].cast()
						);
				}
			};

			Any do_call_impl(std::vector<Any> const& r) const {
				decltype(auto) returned_obj = do_call_impl_impl(r);
				using Type = typename std::decay_t<decltype(returned_obj)>;

				if constexpr (std::is_same_v<void, Type>) {
					// void? Return void.
					return Any();
				}
				else if constexpr (std::is_same_v<Any, typename std::remove_reference_t<Type>>) {
					// Any? Return reference to the underlying value, NOT a reference to the Any.
					return returned_obj;
				}
				else if constexpr (std::is_pointer<Type>::value) {
					// Pointer? Wrap it as a shared pointer.
					using Type2 = typename std::remove_pointer<Type>::type;
					if (returned_obj) {
						return std::shared_ptr<Type2>(returned_obj, [=](Type2*) { /* do nothing */ });
					}
					else {
						return Any();
					}
				}
				else if constexpr (std::is_reference<Type>::value) {
					// Reference? Wrap it as a shared pointer.
					using Type2 = typename std::remove_reference<Type>::type;
					return std::shared_ptr<Type2>(&returned_obj, [=](Type2*) { /* do nothing */ });
				}
				else {
					return std::move(returned_obj);
				}
			};

			R(*m_attr)(T...);
		};

		namespace detail {
			template <typename T>
			struct is_static_member_function : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...)> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const volatile> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) volatile> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...)&> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const&> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...)&&> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const&&> : std::false_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) noexcept> : std::true_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const noexcept> : std::true_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) volatile noexcept> : std::true_type {};

			template <typename R, typename C, typename... Args>
			struct is_static_member_function<R(C::*)(Args...) const volatile noexcept> : std::true_type {};


			template<typename... Param> struct Function_Params {};

			template<typename Ret, typename Class, typename Params, bool IsMember = false, bool IsMemberObject = false, bool IsObject = false>
			struct Function_Signature {
				using Param_Types = Params;
				using Class_Type = Class;
				using Return_Type = Ret;

				constexpr static const bool is_object = IsObject; // e.g. lambda object
				constexpr static const bool is_member = IsMember; // e.g. first param MUST be an alive Class type. May be function or parameter.
				constexpr static const bool is_member_object = IsMemberObject; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
				constexpr static const bool is_member_function = !is_member_object && is_member; // e.g. first param MUST be an alive Class type. Will be a parameter of the Class.
				constexpr static const bool is_static_member_function = std::is_same_v< Class_Type, void>; // e.g. free function

				template<typename T> constexpr Function_Signature(T&&) noexcept { };
				constexpr Function_Signature() noexcept = default;
			};

			// Free functions
			template<typename Ret, typename... Param>
			Function_Signature(Ret(*f)(Param...))
				->Function_Signature<Ret, void, Function_Params<Param...>>; // static function

			// no reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile)
				->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile const)
				->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...))
				->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) const)
				->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true>; // member function

			// & reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile&)
				->Function_Signature<Ret, Class, Function_Params<volatile Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile const&)
				->Function_Signature<Ret, Class, Function_Params<volatile const Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...)&)
				->Function_Signature<Ret, Class, Function_Params<Class&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) const&)
				->Function_Signature<Ret, Class, Function_Params<const Class&, Param...>, true>; // member function

			// && reference specifier
			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile&&)
				->Function_Signature<Ret, Class, Function_Params<volatile Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) volatile const&&)
				->Function_Signature<Ret, Class, Function_Params<volatile const Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...)&&)
				->Function_Signature<Ret, Class, Function_Params<Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class, typename... Param>
			Function_Signature(Ret(Class::* f)(Param...) const&&)
				->Function_Signature<Ret, Class, Function_Params<const Class&&, Param...>, true>; // member function

			template<typename Ret, typename Class>
			Function_Signature(Ret Class::* f)
				->Function_Signature<Ret, Class, Function_Params<Class&>, true, true>; // member object

			// primary template handles types that have no nested ::type member:
			template<class, class = std::void_t<>>
			struct has_call_operator : std::false_type {};

			// specialization recognizes types that do have a nested ::type member:
			template<class T>
			struct has_call_operator<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

			template<typename Func>
			auto function_signature(const Func& f) {
				if constexpr (has_call_operator<Func>::value) {
					return Function_Signature<
						typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Return_Type,
						typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Class_Type,
						typename decltype(Function_Signature{ &std::decay_t<Func>::operator() })::Param_Types,
						false,
						false,
						true
					> {};
				}
				else {
					return Function_Signature{ f };
				}
			};

			template<typename Obj, typename Param1, typename... Rest>
			Param1 get_first_param(Function_Params<Param1, Rest...>, Obj&& obj) {
				return static_cast<Param1>(std::forward<Obj>(obj));
			};

		} // namespace chaiscript::dispatch::detail
	};

	// Convert nearly any function or function pointer to a callable, generic proxy function. 
	template<typename Func> Proxy_Function make_callable(Func&& func) {
		using function_header = decltype(details::detail::function_signature(func));

		static constexpr const bool is_static_member_function = function_header::is_static_member_function;
		static constexpr const bool is_member = function_header::is_member;
		static constexpr const bool is_object = function_header::is_object;
		static constexpr const bool is_member_object = function_header::is_member_object;

		if constexpr (is_object) {
			// function objects
			auto* function_impl = new details::Explicit_Function_Impl(std::forward<Func>(func));
			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else if constexpr (is_member_object) {
			// member objects
			auto* function_impl = new details::Attribute_Access_Impl(std::forward<Func>(func));
			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else if constexpr (is_member && !is_member_object) {
			// member functions
			return details::Member_Function_Impl(std::forward<Func>(func));
		}
		else if constexpr (is_static_member_function) {
			// static function pointers
			auto* function_impl = new details::Static_Function_Impl(std::forward<Func>(func));
			auto ptr{ std::static_pointer_cast<details::Proxy_Function_Base>(std::shared_ptr<typename std::remove_pointer<decltype(function_impl)>::type>(function_impl)) };
			return ptr;
		}
		else {
			throw std::runtime_error("Did not handle conversion of provided function to a PROXY_FUNCTION.");
		}
	};


	// Call a generic, proxy function using a vector of inputs (may be empty), which will be converted as necessary to the expected types. 
	__forceinline Any call(Proxy_Function callable, std::vector<Any> const& inputs, Type_Converter_Tree const& conversionTree) {
		if (callable) {
			return callable->operator()(Function_Params{ const_cast<std::vector<Any>&>(inputs) }, conversionTree);
		}
		else {
			throw exception::arity_error(inputs.size(), -1);
		}
	};

	// Call a generic, proxy function using a vector of inputs (may be empty), which will be converted as necessary to the expected types. 
	__forceinline Any call(Proxy_Function callable, Param_Types callable_params, std::vector<Any> const& inputs, Type_Converter_Tree const& conversionTree) {
		auto converted = callable_params.convert(Function_Params{ const_cast<std::vector<Any>&>(inputs) }, conversionTree);
		return call(std::move(callable), std::move(converted), conversionTree);
	};
	
#if 0
	/*
	// If a function is namespaced in a class, that means it's a free function in the namespace of _CLASS_NAME_, whose first parameter is to be that class type. 
	def _CLASS_NAME_::_FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info { ... };
		e.g.
		return _FUNCTION_NAME_( _CLASS_NAME_(),  ... );
		or
		return _CLASS_NAME_()._FUNCTION_NAME_(...);

	// If a function is namespaced in a class but noted as "static", that means it is a free-function, within the namespace of _CLASS_NAME_.
	def static _CLASS_NAME_::_FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info { ... };
		e.g.
		return _CLASS_NAME_::_FUNCTION_NAME_( ... );

	// If a function is declared but not namespaced, then it is assumed to be a free function declared in the current namespace
	def _FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info { ... };
		e.g.
		return _FUNCTION_NAME_( ... );

	// if a function is namespaced in a namespace, then it is assumed to be a free function
	def _NAMESPACE_NAME_::_FUNCTION_NAME_(Type_Info _PARAM_NAME_, ...) -> Type_Info {};
		e.g.
		return _NAMESPACE_NAME_::_FUNCTION_NAME_();
		or
		return (...)._FUNCTION_NAME_(...);

	namespace { // e.g. global namespace
		class _CLASS_NAME_ {
			def _FUNCTION_NAME_(...) -> void {}; 
				e.g.
				return ::_CLASS_NAME_()._FUNCTION_NAME_( ... );
				or
				return ::_CLASS_NAME_::_FUNCTION_NAME_( ::_CLASS_NAME_(), ...);

			def static _FUNCTION_NAME_(...) -> void {};
				e.g.
				return ::_CLASS_NAME_()._FUNCTION_NAME_( ... );
				or
				return ::_CLASS_NAME_::_FUNCTION_NAME_( ... );

			def _CLASS_NAME_() -> _CLASS_NAME_ {}; // EXCEPTION TO THE ABOVE RULES. IF THE FUNCTION NAME MATCHES THE CLASS NAME, THEN IT'S A FUNCTION ADDED TO THE PARENT NAMESPACE
				e.g. 
				return ::_CLASS_NAME_();

			def _CLASS_NAME_(...) -> _CLASS_NAME_ {}; // EXCEPTION TO THE ABOVE RULES. IF THE FUNCTION NAME MATCHES THE CLASS NAME, THEN IT'S A FUNCTION ADDED TO THE PARENT NAMESPACE
				e.g.
				return ::_CLASS_NAME_(...);

			Type_Info _PARAMETER_NAME_; // parameter within the class, accessible from an instance of it. 
				e.g.
				return ::_CLASS_NAME_()._PARAMETER_NAME_;
				or 
				return ::_CLASS_NAME_::_PARAMETER_NAME_( ::_CLASS_NAME_() );

			static Type_Info _VARIABLE_NAME_; // static variable associated to a class, accessible from an instance of it or from the class name.
				e.g.
				return ::_CLASS_NAME_()._VARIABLE_NAME_;
				or
				return ::_CLASS_NAME_::_VARIABLE_NAME_;

			class _CLASS_NAME_ { ... } // may declare classes within a class
			namespace _NAMESPACE_NAME_ { ... } // may declare namespaces within a class

		};
	};

	namespace { // e.g. global namespace
		namespace _NAMESPACE_NAME_ {
			def _FUNCTION_NAME_() -> void {}; // IMPLIED TO BE STATIC SINCE THIS IS A NAMESPACE, AND NOT A CLASS
				e.g.
				return ::_NAMESPACE_NAME_::_FUNCTION_NAME_();

			def _FUNCTION_NAME_(...) -> void {}; // IMPLIED TO BE STATIC SINCE THIS IS A NAMESPACE, AND NOT A CLASS
				e.g.
				return ::_NAMESPACE_NAME_::_FUNCTION_NAME_(...);

			Type_Info _VARIABLE_NAME_; // global variable within the namespace, accessible outside of it. IMPLIED TO BE STATIC SINCE THIS IS A NAMESPACE, AND NOT A CLASS
				e.g.
				return ::_NAMESPACE_NAME_::_VARIABLE_NAME_;

			class _CLASS_NAME_ { ... } // may declare classes within a namespace
			namespace _NAMESPACE_NAME_ { ... } // may declare namespaces within a namespace


		};
	};

	// If a function is delcared in a namespace, how that namespace begins determines the ownership behavior
	namespace _NAMESPACE_NAME_ { // e.g. current parent namespace
		def _FUNCTION_NAME_(...) ->  void {}; 
			e.g. 
			::_NAMESPACE_NAME_::_FUNCTION_NAME_(...); // incorporates the _NAMESPACE_NAME_

		def ::_FUNCTION_NAME_(...) ->  void {};
			e.g.
			::_FUNCTION_NAME_(...); // ignores the _NAMESPACE_NAME_

		def ::_NEW_NAMESPACE_NAME_::_FUNCTION_NAME_(...) ->  void {};
			e.g.
			::_NEW_NAMESPACE_NAME_::_FUNCTION_NAME_(...); // ignores the _NAMESPACE_NAME_
	};

	// A possible use of that feature would be extend global functions:
	namespace _CLASS_NAME_ { // e.g. current parent namespace
		def ::to_string() ->  void {}; // non-static implies the first parameter must be a _CLASS_NAME_
			e.g.
			_CLASS_NAME_().to_string();
			or 
			to_string(_CLASS_NAME_());

		def static ::to_json(_CLASS_NAME_ a) -> JSON {};
			e.g.
			to_json(_CLASS_NAME_()); // as a static, global function
			or
			_CLASS_NAME_().to_json; // as a static function called on a class_name object

		def static ::`==`(_CLASS_NAME_ a, _CLASS_NAME_ b) -> bool {};
			e.g.
			_CLASS_NAME_() == _CLASS_NAME_();
			or
			`==`( _CLASS_NAME_(), _CLASS_NAME_() );
	};



	*/
	class Functions {
	private:
		mutable std::shared_ptr <
			std::pair < fibers::synchronization::shared_mutex < fibers::synchronization::mutex>,
			fibers::containers::Map<
			std::string, // Function Name (e.g. string). 
			std::shared_ptr<fibers::containers::Map<
			    Param_Types, // Function parameters (e.g. {string, Any}, or {Any, Any, Any}). 
			    Proxy_Function
			>>>
		>> m_functions;
		

	public:
		Functions() : m_functions(std::make_shared<std::pair < fibers::synchronization::shared_mutex < fibers::synchronization::mutex>,
			fibers::containers::Map<
			std::string, // Function Name (e.g. string). 
			std::shared_ptr<fibers::containers::Map<
			Param_Types, // Function parameters (e.g. {string, Any}, or {Any, Any, Any}). 
			Proxy_Function
			>>>>>()) {};
		Functions(Functions const&) = default;
		Functions(Functions &&) = default;
		Functions& operator=(Functions const&) = default;
		Functions& operator=(Functions&&) = default;
		~Functions() = default;

		std::shared_ptr< fibers::containers::Map<Param_Types, Proxy_Function>> operator[](std::string const& key) {
			std::shared_ptr<fibers::containers::Map<Param_Types, Proxy_Function>> temp;
			while (true) {
				if (1) {
					auto locked{ std::shared_lock(m_functions->first) };
					auto optionalF = m_functions->second.at(key);
					if (optionalF.has_value()) return optionalF.value();
				}
				if (!temp) temp = std::make_shared<fibers::containers::Map<Param_Types, Proxy_Function>>();
				if (1) {
					auto locked{ std::unique_lock(m_functions->first) };					
					if (m_functions->second.emplace(key, temp, false)) {
						m_functions->second.TryCleanupUnusedMemory();
					}
				}
			}
		};
		std::shared_ptr< fibers::containers::Map<Param_Types, Proxy_Function>> operator[](std::string const& key) const {
			auto locked{ std::shared_lock(m_functions->first) };
			auto optionalF = m_functions->second.at(key);
			if (optionalF.has_value()) return optionalF.value();			
			return nullptr;
		};
		std::shared_ptr< fibers::containers::Map<Param_Types, Proxy_Function>> operator()(std::string const& key) { return operator[](key); };
		std::shared_ptr< fibers::containers::Map<Param_Types, Proxy_Function>> operator()(std::string const& key) const { return operator[](key); };
		std::shared_ptr< fibers::containers::Map<Param_Types, Proxy_Function>> at(std::string const& key) const { return operator()(key); };

		std::pair< Param_Types, Proxy_Function> operator()(std::string const& key, Function_Params const& params) {
			if (auto ptr = operator()(key)) {
				defer(ptr->TryCleanupUnusedMemory());
				auto optionalF = ptr->pair_at_hash(params.hash());
				if (optionalF.has_value()) return optionalF.value();
			}
			return {};
		};
		std::pair< Param_Types, Proxy_Function> operator()(std::string const& key, Function_Params const& params) const {
			if (auto ptr = operator()(key)) {
				defer(ptr->TryCleanupUnusedMemory());
				auto optionalF = ptr->pair_at_hash(params.hash());
				if (optionalF.has_value()) return optionalF.value();
			}
			return {};
		};
		std::pair< Param_Types, Proxy_Function> at(std::string const& key, Function_Params const& params) const {
			return operator()(key, params);
		};

		Proxy_Function operator()(std::string const& key, Param_Types const& params) {
			if (auto ptr = operator()(key)) {
				defer(ptr->TryCleanupUnusedMemory());
				auto optionalF = ptr->at(params);
				if (optionalF.has_value()) return optionalF.value();
			}
			return nullptr;
		};
		Proxy_Function operator()(std::string const& key, Param_Types const& params) const {
			if (auto ptr = operator()(key)) {
				defer(ptr->TryCleanupUnusedMemory());
				auto optionalF = ptr->at(params);
				if (optionalF.has_value()) return optionalF.value();
			}
			return nullptr;
		};
		Proxy_Function at(std::string const& key, Param_Types const& params) const { return operator()(key, params); };

		bool emplace(std::string const& key, Proxy_Function func, bool replaceIfAlreadyExists = false) {
			if (func) {
				if (auto ptr = operator()(key)) {
					defer(ptr->TryCleanupUnusedMemory());
					return ptr->emplace(func->Arguments(), func, replaceIfAlreadyExists);
				}
			}
			return false;
		};
		bool emplace(std::string const& key, Proxy_Function func, Param_Types const& params, bool replaceIfAlreadyExists = false) {
			if (func) {
				if (auto ptr = operator()(key)) {
					defer(ptr->TryCleanupUnusedMemory());
					return ptr->emplace(params, func, replaceIfAlreadyExists);
				}
			}
			return false;
		};
		bool emplace(std::string const& key, Param_Types const& params, Proxy_Function func, bool replaceIfAlreadyExists = false) {
			if (func) {
				if (auto ptr = operator()(key)) {
					defer(ptr->TryCleanupUnusedMemory());
					return ptr->emplace(params, func, replaceIfAlreadyExists);
				}
			}
			return false;
		};
		bool emplace(std::string const& key, Proxy_Function func, std::vector<std::string> const& params, bool replaceIfAlreadyExists = false) {
			if (func) {
				return emplace(key, func, Param_Types(func->Arguments(), params), replaceIfAlreadyExists);
			}
			return false;
		};
		bool emplace(std::string const& key, std::vector<std::string> const& params, Proxy_Function func, bool replaceIfAlreadyExists = false) {
			if (func) {
				return emplace(key, func, Param_Types(func->Arguments(), params), replaceIfAlreadyExists);
			}
			return false;
		};

		/* Given a function name and call parameters, will attempt to find an exact-match function, variadic instantiation, or convertable function call, or return nullptr. */
		std::pair<Param_Types, Proxy_Function> BuildMatch(std::string const& functionName, scripting::Function_Params const& params, Type_Converter_Tree const& m_typeConverters = Type_Converter_Tree(), bool AllowTemplateInstantiation = true, bool AllowTypeConversion = true) {
			auto [directFindParams, directFind] = at(functionName, params);
			
			if (directFind) {
				// exact match found
				return { directFindParams, directFind };
			}
			else {
				std::multimap<double, std::tuple<Proxy_Function, Param_Types>> candidates;

				if (AllowTypeConversion) {
					if (auto ptr = this->operator[](functionName)) {
						if (ptr) {
							for (auto& function : *ptr) {
								if (function) {
									if (function->first.Template()) {
										// already tried this...
									}
									else {
										if (function->first.converts(params, m_typeConverters)) {
											candidates.insert({ function->first.conversion_cost(params, m_typeConverters), { function->second, function->first } });
										}
									}
								}
							}
						}
					}
				}

				// Get the "cheapest" or fastest conversion option available at this scope.
				//for (auto& candidate : candidates) {
				//	auto& func = std::get<0>(candidate.second);
				//	auto& param = std::get<1>(candidate.second);
				//	try {
				//		auto newParms = scripting::Param_Types(params, func->Arguments().types());
				//		if (emplace(functionName, newParms, func, false)) {
				//			successfullyAddedFunction = true;
				//			break;
				//		}
				//	}
				//	catch (scripting::exception::arity_error err) {}
				//	catch (scripting::exception::bad_boxed_cast err) {}
				//}
				//candidates.clear();
				//if (successfullyAddedFunction) {
					//return at(functionName, params);
				//}

				if (AllowTemplateInstantiation) {
					if (auto ptr = this->operator[](functionName)) {
						if (ptr) {
							for (auto& function : *ptr) {
								if (function) {
									if (function->first.Template()) {
										if (function->first.converts(params, m_typeConverters)) {
											candidates.insert({ function->first.conversion_cost(params, m_typeConverters), { function->second, function->first } });
										}
									}
									else {
										// must be perfect match -- requires casting. We can try that, but only if no template exists that would work instead.
									}
								}
							}
						}
					}
				}

				// Get the "cheapest" or fastest conversion option available at this scope.
				for (auto& candidate : candidates) {
					if (candidate.first == std::numeric_limits<double>::max()) { break; }
					auto& func = std::get<0>(candidate.second);
					auto& param = std::get<1>(candidate.second);
					
					try {
						auto newParms = scripting::Param_Types(params, param/*func->Arguments()*/.types());
						if (emplace(functionName, newParms, func, false)) {
							return { newParms, func };
						}
					}
					catch (scripting::exception::arity_error const& err) {}
					catch (scripting::exception::bad_boxed_cast const& err) {}
				}
			}
			return {};
		};

	};
#endif
	
	class Functions2 {
	public:
		using FunctionActual = std::pair<Proxy_Function, Param_Types>; // this can never be seperated. The underlying function needs this conversion to function properly. 
		using FunctionActualPtr = std::shared_ptr<FunctionActual>;
		using FunctionSort = fibers::containers::Map< Param_Types, FunctionActualPtr>; // sorts actual functions by filters, to speed-up filtering and finding with repeat conversions
		using FunctionMap = fibers::containers::Map< std::string, std::shared_ptr< FunctionSort > >;
		
		mutable std::shared_ptr< FunctionMap > m_functions{ std::make_shared<FunctionMap>() };

		std::shared_ptr< FunctionSort > operator[](std::string const& key) {
			std::shared_ptr< FunctionSort > temp;
			while (true) {
				auto optionalF = m_functions->at(key);
				if (optionalF.has_value()) return optionalF.value();

				if (!temp) temp = std::make_shared<FunctionSort>();
				if (1) {
					if (m_functions->emplace(key, temp, false)) {
						m_functions->TryCleanupUnusedMemory();
					}
				}
			}
		};
		std::shared_ptr< FunctionSort > operator[](std::string const& key) const {
			auto optionalF = m_functions->at(key);
			if (optionalF.has_value()) return optionalF.value();
			return nullptr;
		};
		std::shared_ptr< FunctionSort > operator()(std::string const& key) { return operator[](key); };
		std::shared_ptr< FunctionSort > operator()(std::string const& key) const { return operator[](key); };
		std::shared_ptr< FunctionSort > at(std::string const& key) const { return operator()(key); };

		FunctionActualPtr operator()(std::string const& key, Function_Params const& params) const {
			if (auto ptr = operator()(key)) {
				defer(ptr->TryCleanupUnusedMemory());
				auto optionalF = ptr->at_hash(params.hash());
				if (optionalF.has_value()) return optionalF.value();
			}
			return nullptr;
		};
		FunctionActualPtr at(std::string const& key, Function_Params const& params) const {
			return operator()(key, params);
		};

		FunctionActualPtr operator()(std::string const& key, Param_Types const& params) const {
			if (auto ptr = operator()(key)) {
				defer(ptr->TryCleanupUnusedMemory());
				auto optionalF = ptr->at(params);
				if (optionalF.has_value()) return optionalF.value();
			}
			return nullptr;
		};
		FunctionActualPtr at(std::string const& key, Param_Types const& params) const {
			return operator()(key, params);
		};

		bool emplace(std::string const& key, Param_Types const& params, Proxy_Function func, bool replaceIfAlreadyExists = false) {
			if (func) {
				if (auto ptr = operator()(key)) {
					defer(ptr->TryCleanupUnusedMemory());
					return ptr->emplace(params, std::make_shared<FunctionActual>(func, params), replaceIfAlreadyExists);
				}
			}
			return false;
		};
		bool emplace(std::string const& key, Proxy_Function func, Param_Types const& params, bool replaceIfAlreadyExists = false) { return emplace(key, params, func, replaceIfAlreadyExists); };
		bool emplace(std::string const& key, Proxy_Function func, bool replaceIfAlreadyExists = false) { if (func) return emplace(key, func->Arguments(), func, replaceIfAlreadyExists); else return false; };
		bool emplace(std::string const& key, Proxy_Function func, std::vector<std::string> const& params, bool replaceIfAlreadyExists = false) { if (func) return emplace(key, func, Param_Types(func->Arguments(), params), replaceIfAlreadyExists); else return false; };
		bool emplace(std::string const& key, std::vector<std::string> const& params, Proxy_Function func, bool replaceIfAlreadyExists = false) { if (func) return emplace(key, func, Param_Types(func->Arguments(), params), replaceIfAlreadyExists); else return false; };

		bool emplace_free(std::string const& key, Param_Types const& params, Proxy_Function func, bool replaceIfAlreadyExists = false) {
			if (func) {
				if (auto ptr = operator()(key)) {
					defer(ptr->TryCleanupUnusedMemory());
					return ptr->emplace(params, std::make_shared<FunctionActual>(func, params), replaceIfAlreadyExists);
				}
			}
			return false;
		};
		bool emplace_free(std::string const& key, Proxy_Function func, Param_Types const& params, bool replaceIfAlreadyExists = false) { return emplace_free(key, params, func, replaceIfAlreadyExists); };
		bool emplace_free(std::string const& key, Proxy_Function func, bool replaceIfAlreadyExists = false) { if (func) return emplace_free(key, func->Arguments(), func, replaceIfAlreadyExists); else return false; };
		bool emplace_free(std::string const& key, Proxy_Function func, std::vector<std::string> const& params, bool replaceIfAlreadyExists = false) { if (func) return emplace_free(key, func, Param_Types(func->Arguments(), params), replaceIfAlreadyExists); else return false; };
		bool emplace_free(std::string const& key, std::vector<std::string> const& params, Proxy_Function func, bool replaceIfAlreadyExists = false) { if (func) return emplace_free(key, func, Param_Types(func->Arguments(), params), replaceIfAlreadyExists); else return false; };

		/* Given a function name and call parameters, will attempt to find an exact-match function, variadic instantiation, or convertable function call, or return nullptr. */
		scripting::Functions2::FunctionActualPtr BuildMatch(std::string const& functionName, scripting::Function_Params const& params, Type_Converter_Tree const& m_typeConverters = Type_Converter_Tree(), bool AllowTemplateInstantiation = true, bool AllowTypeConversion = true) {
			if (auto func = at(functionName, params)) {
				// exact match found
				return func;
			}
			else {
				std::multimap<double, scripting::Functions2::FunctionActualPtr> candidates;

				if (AllowTypeConversion) {
					if (auto ptr = this->operator[](functionName)) {
						if (ptr) {
							for (auto& function : *ptr) {
								if (function) {
									if (function->first.Template()) {
										// already tried this...
									}
									else {
										if (function->first.converts(params, m_typeConverters)) {
											candidates.insert({ function->first.conversion_cost(params, m_typeConverters), function->second });
										}
									}
								}
							}
						}
					}
				}

				if (AllowTemplateInstantiation) {
					if (auto ptr = this->operator[](functionName)) {
						if (ptr) {
							for (auto& function : *ptr) {
								if (function) {
									if (function->first.Template()) {
										if (function->first.converts(params, m_typeConverters)) {
											candidates.insert({ function->first.conversion_cost(params, m_typeConverters), function->second });
										}
									}
									else {
										// must be perfect match -- requires casting. We can try that, but only if no template exists that would work instead.
									}
								}
							}
						}
					}
				}

				// Get the "cheapest" or fastest conversion option available at this scope.
				for (auto& candidate : candidates) {
					if (candidate.first == std::numeric_limits<double>::max()) { break; }
					auto& func = candidate.second->first;
					auto& param = candidate.second->second;

					try {
						auto newParms = scripting::Param_Types(params, param.types());

						if (func) {
							if (auto ptr = operator()(functionName)) {
								defer(ptr->TryCleanupUnusedMemory());
								if (ptr->emplace(newParms, candidate.second, false)) {
									return candidate.second;
								}
								else {
									// try again -- someone beat us to the punch. It'll (probably) be available this time. 
									return BuildMatch(functionName, params, m_typeConverters, AllowTemplateInstantiation, AllowTypeConversion);
								}
							}
						}
					}
					catch (scripting::exception::arity_error const& err) {}
					catch (scripting::exception::bad_boxed_cast const& err) {}
				}
			}
			return nullptr;
		};

	};

	class Impl;
#if 0
    class Scope;
    class Namespace;
	class Class;

	// keep updating so that the return value from Functions makes useful sense (Function with specialized params)

    class Scope {		
	public:
		friend class Namespace;
		friend class Class;
		friend class Impl;
		Scope(std::weak_ptr<Scope> parent = std::weak_ptr<Scope>())
			: p_parent{ parent }
			, p_temp_tree()
		{
			auto randN = [](double min, double max) -> double {
				return (((double)std::rand() / (double)RAND_MAX) * (max - min)) + min;
			};
			for (int i = 0; i < 16; i++) {
				p_NameRand += (char)(int)randN((int)('A'), (int)('Z'));
			}
		};
		Scope(Scope const&) = default;
		Scope(Scope&&) = default;
		Scope& operator=(Scope const&) = default;
		Scope& operator=(Scope&&) = default;
		virtual ~Scope() = default;

	private:
		std::string
			p_NameRand;
		mutable fibers::synchronization::atomic_number<size_t>
			p_temp_tree_hash;
		mutable Type_Converter_Tree
			p_temp_tree;

	public:
		std::weak_ptr<Scope>
			p_self; // shared_pointer to itself
		std::weak_ptr<Scope>
			p_parent; // parent scope, for navigation. 
		fibers::containers::Map<std::string, std::shared_ptr<Namespace>>
			p_children; // children namespaces - may be classes or namespaces. By this design, imported namespaces may be "unloaded" on scope unloading, which is on purpose.
		fibers::containers::Map<std::string, std::shared_ptr<fibers::Any>>
			p_objects; // scopes of all types may declare objects. Namespace objects may be global objects, but still. 
		fibers::containers::Map<std::string, std::weak_ptr<Namespace>>
			p_using; // allows this scope to use the children of other scopes as if they were their own.

		
	public:

		virtual bool IsClass() const { return false; }; // classes are specialized namespaces
		virtual bool IsNamespace() const { return false; }; // namespaces may declare functions and types
		virtual bool IsBasicScope() const { return !IsNamespace(); }; // basic scopes may only store local objects

		virtual const std::string& GetName() const {
			static std::string emptyString{};
			return emptyString;
			// return p_NameRand;
		};
		virtual std::string GetQualifiedNamespace(bool GetUniqueQualifier = false) const {
			std::string path = "::";

			auto parent = p_parent.lock();

			if (!parent) {
				path = "::";
			}
			else {
				auto name{ GetName() };
				if (GetUniqueQualifier) {
					if (name == "") {
						name = p_NameRand;
					}
					else {
						name = p_NameRand + "_" + name;
					}
				}
				if (!name.empty()) {
					path = parent->GetQualifiedNamespace(GetUniqueQualifier) + name + "::";
				}
				else {
					path = parent->GetQualifiedNamespace(GetUniqueQualifier);
				}
			}

			while (path.find("::::") != std::string::npos) {
				size_t start_pos = 0;
				while ((start_pos = path.find("::::", start_pos)) != std::string::npos) {
					path = path.replace(start_pos, 4, "::");
					start_pos += 2; // In case 'to' contains 'from', like replacing 'x' with 'yx'
				}
			}

			return path;
		};
		// Record that a namespace is a child of the current Scope. This creates a hard link, ensuring the lifetime of the child is associated to the lifetime of the parent. You MUST NEVER assign a child-parent-child loop, otherwise it will never free its memory. 
		virtual bool AddChild(std::weak_ptr<Namespace> const& p_namespace, bool overwriteIfExists = true) { 
			// If so... we are done. 
			if (auto ptr = p_namespace.lock()) {
				if (std::dynamic_pointer_cast<Scope>(ptr)->IsNamespace()) {
					return p_children.emplace(((Scope*)(ptr.get()))->GetName(), ptr, overwriteIfExists);
				}
			}
			return false;	
		};
		// Identify that a Namespace is being "Used" by the current scope, meaning the Used's children will be easily found in the current scope without name-qualifications.
		virtual bool AddUsing(std::weak_ptr<Namespace> const& p_namespace) {
			if (auto ptr = p_namespace.lock()) {
				if (std::dynamic_pointer_cast<Scope>(ptr)->IsNamespace()) {
					return p_using.emplace(((Scope*)(ptr.get()))->p_NameRand, p_namespace);
				}
			}
			return false;
		};
		virtual void Print(int indentLevel = 0) const {
			int IndentLevel = 0;
			for (; IndentLevel < indentLevel; IndentLevel++) {
				std::cout << " ";
			}
			if (GetName() == "") {
				std::cout << "::" << std::endl;
				for (auto& child : p_children) {
					if (child && child->second) {
						((Scope*)(child->second.get()))->Print(indentLevel + 2);
					}
				}
				for (auto& child_parent : p_using) {
					if (child_parent) {
						if (auto this_parent = std::dynamic_pointer_cast<Scope>(child_parent->second.lock())) {
							for (auto& child2 : this_parent->p_children) {
								if (child2 && child2->second) {
									((Scope*)(child2->second.get()))->Print(indentLevel + 2);
								}
							}
						}
					}
				}
			}
			else {
				std::cout << GetName() << std::endl;
				for (auto& child : p_children) {
					if (child && child->second) {
						((Scope*)(child->second.get()))->Print(indentLevel + GetName().length());
					}
				}
				for (auto& child_parent : p_using) {
					if (child_parent) {
						if (auto this_parent = std::dynamic_pointer_cast<Scope>(child_parent->second.lock())) {
							for (auto& child2 : this_parent->p_children) {
								if (child2 && child2->second) {
									((Scope*)(child2->second.get()))->Print(indentLevel + GetName().length());
								}
							}
						}
					}
				}
			}
		};

		// Return the conversion tree(s) from the nearest Global namespace
		virtual Type_Converter_Tree* TypeConversionTree() {
			if (auto parent = p_parent.lock()) return parent->TypeConversionTree();
			return nullptr;
		};
		virtual const Type_Converter_Tree* TypeConversionTree() const {
			if (auto parent = p_parent.lock()) return parent->TypeConversionTree();
			return nullptr;
		};

		// Return the conversion tree(s) from the nearest Global namespace
		virtual void TypeConversionTrees(std::vector<Type_Converter_Tree*>& out, std::set<size_t>& previous) {
			if (previous.find((size_t)p_self.lock().get()) == previous.end()) {
				previous.insert((size_t)p_self.lock().get());
				for (auto& Using : p_using) {
					if (Using) {
						if (auto ptr = std::dynamic_pointer_cast<Scope>(Using->second.lock())) {
							ptr->TypeConversionTrees(out, previous);
						}
					}
				}
				if (auto parent = p_parent.lock()) parent->TypeConversionTrees(out, previous);				
			}
		};
		// Return the conversion tree(s) from the nearest Global namespace
		virtual void TypeConversionTrees(std::vector<const Type_Converter_Tree*>& out, std::set<size_t>& previous) const {
			if (previous.find((size_t)p_self.lock().get()) == previous.end()) {
				previous.insert((size_t)p_self.lock().get());
				for (auto& Using : p_using) {
					if (Using) {
						if (auto ptr = std::dynamic_pointer_cast<Scope>(Using->second.lock())) {
							ptr->TypeConversionTrees(out, previous);
						}
					}
				}
				if (auto parent = p_parent.lock()) parent->TypeConversionTrees(out, previous);
			}
		};

		// Get all applicable Conversion Trees for this scope. We allow multiple trees (from multiple Imports, for example) to co-exist and they are combined per-Scope on an as-needed basis.
		std::vector<const Type_Converter_Tree*> TypeConversionTrees() const {
			std::vector<const Type_Converter_Tree*> out;

			// Get Initial List (May contain duplicates)
			{
				std::set<size_t> previous{};
				TypeConversionTrees(out, previous);
			}

			// Unique List
			{
				std::set< const Type_Converter_Tree*> out2{};

				for (auto& x : out) out2.insert(x);

				out.clear();

				for (auto& x : out2) if (x != nullptr) out.push_back(x);
			}

			return out;
		};
		
		/* 
		Each Scope has a local copy of a Type_Converter_Tree. However, if there is no difference between the Type_Converter_Tree for this scope or it's parent, then it returns the parent's Type_Converter_Tree. 
		This is designed to encourage re-use of similar Type_Converter_Tree's and prevent time wasted on their construction. 
		*/
		Type_Converter_Tree const& GetCombinedTypeConversionTree() const {
			auto HashTrees = [](std::vector<const Type_Converter_Tree*> const& trees) -> size_t {
				constexpr auto A = 54059; /* a prime */
				constexpr auto B = 76963; /* another prime */
				constexpr auto C = 86969; /* yet another prime */
				constexpr auto FIRSTH = 37; /* also prime */

				size_t h = FIRSTH;
				if (trees.size() > 0)
					for (auto& s : trees)
						if (s) {
							h = (h * A) ^ ((size_t)s * B);
							h = (h * A) ^ (s->Version() * B);
						}

				auto result = h % C;
				return result;
			};

#if 0
			auto trees = TypeConversionTrees();
			auto hash = HashTrees(trees);
			auto prevHash = this->p_temp_tree_hash;
			if (prevHash != hash) {
				p_temp_tree_hash = hash;
				return p_temp_tree = Type_Converter_Tree::Combine(trees);
			}
			else {
				return p_temp_tree;
			}
#else
			// get the highest parent where the conversion trees don't change;
			if (std::shared_ptr<Scope> highestSuccess = p_self.lock()) {
				auto trees = highestSuccess->TypeConversionTrees();
				auto highestSuccessHash = HashTrees(trees);
				while (std::shared_ptr<Scope> parentAttempt = highestSuccess->p_parent.lock()) {
					auto parentHash = HashTrees(parentAttempt->TypeConversionTrees());
					if (parentHash != highestSuccessHash) {
						break;
					}
					else {
						highestSuccess = parentAttempt;
					}
				}

				while (true) {
					// something changed with some of these 
					auto prevHash = highestSuccess->p_temp_tree_hash;
					if (prevHash != highestSuccessHash) {
						highestSuccess->p_temp_tree_hash = highestSuccessHash;
						return highestSuccess->p_temp_tree = Type_Converter_Tree::Combine(trees);
					}
					else {
						return highestSuccess->p_temp_tree;
					}
				}

				return highestSuccess->p_temp_tree;
			}
			return this->p_temp_tree;

			//while (true) {
			//	auto trees = TypeConversionTrees();
			//	size_t Hash = HashTrees(trees);
			//	// something changed with some of these 
			//	if (p_temp_tree_hash != Hash) {
			//		p_temp_tree_hash = Hash;
			//		if (p_temp_tree_hash == Hash) {
			//			p_temp_tree = Type_Converter_Tree::Combine(trees);
			//			return p_temp_tree;
			//		}
			//	}
			//	else {
			//		return p_temp_tree;
			//	}
			//}
#endif
		};

	protected:
		static std::string FixQualifiedNamespaceString(std::string const& Namespace) {
			auto path = Namespace + "::";
			while (path.find("::::") != std::string::npos) {
				size_t start_pos = 0;
				while ((start_pos = path.find("::::", start_pos)) != std::string::npos) {
					path = path.replace(start_pos, 4, "::");
					start_pos += 2; // In case 'to' contains 'from', like replacing 'x' with 'yx'
				}
			}
			return path;
		};

		// get the nearest parent whose scope is equivalent to the global scope.
		// e.g. ::{ scope 1 }::{ scope 2}::Namespace1::Namespace2::{ scope 3 }->GetGlobalScope() would return { scope 2 }.
		std::weak_ptr<Scope> GetGlobalScope() const {
			std::shared_ptr<Scope> out = p_self.lock();			
			while (out) {
				auto parent = out->p_parent.lock();
				if (!parent) break;

				if (out->GetQualifiedNamespace() == "::") return out;
				out = parent;
			}
			
			return out;
		};
		// determines if the namespace qualifies itself as relative to the global (e.g. starts with "::")
		static bool IsNamespaceQualified(std::string const& Namespace/* = "::std::string::"*/) {
			return (Namespace.find("::") == 0);
		};
		// find the child nearest to the current node with the specified, fully-qualified namespace
		static bool FindChildNearestToQualifiedNamespace(std::shared_ptr<Scope> current_scope, std::string const& fullyQualifiedNamespace/* = "::std::string::"*/, std::shared_ptr<Scope>& best_scope, int& best_match_count) {
			if (!current_scope) { return false; }

			auto qualifiedNamespace = current_scope->GetQualifiedNamespace();

			int overlap = fullyQualifiedNamespace._Starts_with(qualifiedNamespace) ? qualifiedNamespace.length() : 0;
			
			bool toReturn = false;

			if (overlap > best_match_count) {
				best_match_count = overlap;
				best_scope = current_scope;
				toReturn = true;

				// check our children to see if we can improve this match
				for (auto& child_namespace : current_scope->p_children) {
					if (child_namespace) {
						if (FindChildNearestToQualifiedNamespace(std::dynamic_pointer_cast<Scope>(child_namespace->second), fullyQualifiedNamespace, best_scope, best_match_count)) {
							toReturn = true;
						}
					}
				}
			}
			return toReturn;
		};
		// find the child nearest to the current node with the generic, NOT fully-qualified namespace
		static bool FindChildNearestToNamespace(std::shared_ptr<Scope> current_scope, std::string const& Namespace/* = "string"*/, std::shared_ptr<Scope>& best_scope) {
			while (current_scope) {
				if (current_scope->GetName() == Namespace) {
					best_scope = current_scope;
					return true;
				}
				if (!best_scope) { best_scope = current_scope; }
				if (!current_scope) { return false; }
				if (Namespace == "") {
					return true;
				}

				std::string namespaceToFind = current_scope->GetQualifiedNamespace() + Namespace + "::";

				int bestMatch = -1;
				std::shared_ptr<Scope> bestScopeTemp{ nullptr };
				if (FindChildNearestToQualifiedNamespace(current_scope, namespaceToFind, bestScopeTemp, bestMatch)) {
					if (bestScopeTemp->GetQualifiedNamespace() == namespaceToFind) {
						// awesome, it was found in our children

						best_scope = bestScopeTemp;

						return true;
					}
					else {
						current_scope = current_scope->p_parent.lock();
					}
				}
				else {
					// not found... try our parent. 
					current_scope = current_scope->p_parent.lock();
				}
			}
			return false;
		};
	
    protected:
		/* 
		Get all namespaces that may be used to search for an object (e.g this scope, it's USING scopes, its PARENT scopes 
		Ordered from ParentScope -> UsingScope -> ThisScope
		*/
		virtual std::map<int, std::shared_ptr<Scope>> GetScopesForObjectSearchImpl(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true) const {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > using namespaces

			std::map<int, std::shared_ptr<Scope>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetScopesForObjectSearchImpl(evaluated, false);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = usingScope->second.lock()) {
						for (auto x : ((Scope*)ptr.get())->GetScopesForObjectSearchImpl(evaluated, false)) {
							out.emplace(out.size(), x.second);
						}
					}
				}
			}

			// also remember to add myself (if I am a namespace)...
			out.emplace(out.size(), p_self.lock());

			return out;
		};
		/* Get all named namespaces that are discoverable from the current Scope */
		virtual std::map<std::string, std::weak_ptr<Namespace>> GetAvailableNamespacesImpl(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true, bool useUniqueNames = false) const {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > using namespaces

			std::map<std::string, std::weak_ptr<Namespace>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = usingScope->second.lock()) {
						for (auto& using_child : ((Scope*)ptr.get())->p_children) {
							if (using_child && using_child->second) {
								for (auto& using_namespace : ((Scope*)using_child->second.get())->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames)) {
									if (auto ptr = using_namespace.second.lock()) out[using_namespace.first] = ptr;
								}
							}
						}
					}
				}
			}

			// get my actual children last...
			for (auto& using_child : this->p_children) {
				if (using_child && using_child->second) {
					for (auto& using_namespace : ((Scope*)using_child->second.get())->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames)) {
						if (auto ptr = using_namespace.second.lock()) out[using_namespace.first] = ptr;
					}
				}
			}

			// also remember to add myself (if I am a namespace)...
			if (IsNamespace()) {
				if (useUniqueNames) {
					out[this->p_NameRand + "_" + GetName()] = std::dynamic_pointer_cast<Namespace>(p_self.lock()); 
				}else{
					out[GetName()] = std::dynamic_pointer_cast<Namespace>(p_self.lock());
				}
			}

			return out;
		};

	public:
		/* Get all named namespaces that are discoverable from the current Scope */
		virtual std::map<std::string, std::weak_ptr<Namespace>> GetAvailableNamespaces(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true, bool useUniqueNames = false) const {
			auto fixNamespace = [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}
				return x;
			};

			std::map<std::string, std::weak_ptr<Namespace>> out;
			for (auto& x : GetAvailableNamespacesImpl(evaluated, requestedScope, useUniqueNames)) out[fixNamespace(x.first)] = x.second;
			return out;
		};

	public:
		/* Get all namespaces that may be used to search for an object (e.g this scope, it's USING scopes, its PARENT scopes), in order of ThisScope -> UsingScope -> ParentScope */
		virtual std::vector<std::shared_ptr<Scope>> GetScopesForObjectSearch(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>()) const {
			std::vector<std::shared_ptr<Scope>> out;
			auto init = GetScopesForObjectSearchImpl(evaluated);
			out.reserve(init.size() + 1);
			for (auto iter = init.rbegin(); iter != init.rend(); iter++) out.push_back(iter->second);
			return out;
		};

	public:
		virtual size_t GetCurrentVersion(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>()) const {
			constexpr auto A = 54059; /* a prime */
			constexpr auto B = 76963; /* another prime */
			constexpr auto C = 86969; /* yet another prime */
			constexpr auto FIRSTH = 37; /* also prime */
			size_t h = FIRSTH;

			evaluated->emplace(this->p_self.lock());

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					h = (h * A) ^ (ptr->GetCurrentVersion(evaluated)* B);
			}

			// get my "using" children... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = std::dynamic_pointer_cast<Scope>(usingScope->second.lock())) {
						h = (h * A) ^ (ptr->GetCurrentVersion(evaluated) * B);
					}
				}
			}

			return h % C;
		};

	protected:
		// Attempts to find a namespace or class with the requested namespace name. 
		// If qualified (e.g. starts with "::") then it attempts to find it from the global root -> down. 
		// If not qualified, then it attempts to find it from the current node -> up.
		// If the scope is not found, it MAY suggest the placement of the new namespace as a child of the provided "best_scope"
		// For example, "::std::string::impl" (assuming impl does not exist) may return the node for "::std::string::")
		virtual bool TryFindScope(std::shared_ptr<Scope>& best_scope, std::string const& Namespace, std::vector<std::string>* namespaceListToAppend = nullptr) const {
			decltype(auto) available_scopes = GetAvailableNamespaces();

			std::string scopeToFind = Namespace;
			while (scopeToFind.size() >= 2 && (scopeToFind.find("::") == 0)) { scopeToFind = scopeToFind.substr(2); }
			while (scopeToFind.size() >= 2 && (scopeToFind.rfind("::") == (scopeToFind.length()-2))) { scopeToFind = scopeToFind.substr(0, scopeToFind.length() - 2); }

			auto f = available_scopes.find(scopeToFind);
			if (f != available_scopes.end()) {
				best_scope = std::dynamic_pointer_cast<Scope>(f->second.lock());
				if (best_scope) return true;
			}

			// failed to find it -- can we make a recommendation of where to put it?
			size_t pos;
			for (pos = scopeToFind.rfind("::"); pos != std::string::npos; pos = scopeToFind.rfind("::")) {
				std::string removedScopeName = scopeToFind.substr(pos + 2);

				if (namespaceListToAppend)
					namespaceListToAppend->push_back(removedScopeName);

				scopeToFind = scopeToFind.substr(0, pos);

				f = available_scopes.find(scopeToFind);
				if (f != available_scopes.end()) {
					best_scope = std::dynamic_pointer_cast<Scope>(f->second.lock());
					if (best_scope) return false;
				}
			}

			// we should be on the last "real" scope name
			f = available_scopes.find(scopeToFind);
			if (f != available_scopes.end()) {
				best_scope = std::dynamic_pointer_cast<Scope>(f->second.lock());
				if (best_scope) return false;
			}

			if (namespaceListToAppend)
				namespaceListToAppend->push_back(scopeToFind);

			// return THIS scope
			best_scope = p_self.lock();
			return false;
		};
		virtual const fibers::containers::Map<std::string, std::weak_ptr<Class>>* GetPostFixes() const {
			return nullptr;
		};

	public:
		// Attempts to find a namespace or class with the requested namespace name (may be a qualified namespace). 
		virtual std::shared_ptr<Namespace> FindNamespace(std::string const& Namespace/* = "string"*/) const {
			std::shared_ptr<Scope> foundScope;
			if (this->TryFindScope(foundScope, Namespace)) {
				if (foundScope->IsNamespace()) {
					return std::dynamic_pointer_cast<scripting::Namespace>(foundScope);
				}
			}
			return nullptr;
		};

	protected:
		virtual Type_Info GetClassType() const { return {}; };
		virtual bool TryFindObjectImpl(std::string const& objectName, std::shared_ptr<fibers::Any>& out) const {

			size_t lastOfColons{ 0 };
			if ((lastOfColons = objectName.find_last_of("::")) == std::string::npos) {
				// just a normal var name

				for (auto& scope : GetScopesForObjectSearch()) {
					auto found = scope->p_objects.at(objectName);
					if (found.has_value()) {
						out = found.value();
						return true;
					}
				}
				return false;
			}
			else {
				std::string objectNameActual{ objectName.substr(lastOfColons + 1) };
				std::string scopeName{ objectName.substr(0, lastOfColons - 1) };

				std::shared_ptr<Scope> foundScope;
				if (this->TryFindScope(foundScope, scopeName)) {
					return foundScope->TryFindObjectImpl(objectNameActual, out);
				}
				return false;
			}
		};
		virtual const Functions* GetFunctions() const { return nullptr; };
		virtual Functions* GetFunctions() { return nullptr; };
		virtual bool TryFindFunctionImpl(std::string const& functionName, scripting::Function_Params const& params, Type_Converter_Tree const& m_conversionTree, Proxy_Function& out) const {
			size_t lastOfColons{ 0 };
			if ((lastOfColons = functionName.find_last_of("::")) == std::string::npos) {
				std::shared_ptr<Scope> firstParamScopePtr{ nullptr };
				std::shared_ptr<Scope> constructorScopePtr{ nullptr };

				// FIRST SEARCH DOES ALLOW FOR CONVERSIONS, BUT NO TEMPLATES
				if (1) {
					std::multimap<double, Proxy_Function> sort;

					for (auto& scope : GetScopesForObjectSearch()) {
						if (auto* ptr = scope->GetFunctions()) {
							auto [funcParams, func] = ptr->BuildMatch(functionName, params, m_conversionTree, false, true);
							if (func) {
								sort.emplace(funcParams.conversion_cost(params, m_conversionTree), func);
							}
						}
					}
					{
						auto firstParam = params.begin();
						if (firstParam != params.end()) {
							firstParamScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(firstParam->Type()));
						}						
					}
					if (firstParamScopePtr) {
						for (auto& scope : firstParamScopePtr->GetScopesForObjectSearch()) {
							if (auto* ptr = scope->GetFunctions()) {
								auto [funcParams, func] = ptr->BuildMatch(functionName, params, m_conversionTree, false, true);
								if (func) {
									sort.emplace(funcParams.conversion_cost(params, m_conversionTree), func);
								}
							}
						}
					}
					// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, BUT NO TEMPLATES)
					if (constructorScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(functionName))) {
						// Is there a pre-defined constructor that this could work with?
						if (auto* functions = constructorScopePtr->GetFunctions()) {
							auto [funcParams, func] = functions->BuildMatch(functionName, params, m_conversionTree, false, true);
							if (func) {
								sort.emplace(funcParams.conversion_cost(params, m_conversionTree), func);
							}
						}

						// Can the conversion tree do this itself, without any help?
						if (params.size() == 1) {
							if (m_conversionTree.Converts(params[0].Type(), constructorScopePtr->GetClassType())) {
								auto func = make_callable([tree = m_conversionTree, toType = constructorScopePtr->GetClassType()](Any const& from)->Any {
									if (tree.Converts(from.Type(), toType)) {
										try {
											return tree.Convert(from, toType);
										}
										catch (exception::bad_boxed_cast const& e) {
											std::cout << "Conversion is no longer available" << std::endl;
											throw exception::not_found_error("Conversion is no longer available");
										}
									}
									else {
										std::cout << "Conversion is no longer available" << std::endl;
										throw exception::not_found_error("Conversion is no longer available");
										// return Any();
									}
								});
								sort.emplace(func->conversion_cost(params, m_conversionTree), func);
							}
						}
					}
					for (auto& s : sort) { if (s.first != std::numeric_limits<double>::max()) { out = s.second; return true; } }
				}

				// SECOND SEARCH ALLOWS FOR TEMPLATE FUNCTIONS
				if (1) {
					std::multimap<double, Proxy_Function> sort;
					for (auto& scope : GetScopesForObjectSearch()) {
						if (auto* ptr = scope->GetFunctions()) {
							auto [funcParams, func] = ptr->BuildMatch(functionName, params, m_conversionTree, true, true);
							if (func) {
								sort.emplace(funcParams.conversion_cost(params, m_conversionTree), func);
							}
						}
					}
					if (firstParamScopePtr) {
						for (auto& scope : firstParamScopePtr->GetScopesForObjectSearch()) {
							if (auto* ptr = scope->GetFunctions()) {
								auto [funcParams, func] = ptr->BuildMatch(functionName, params, m_conversionTree, true, true);
								if (func) {
									sort.emplace(funcParams.conversion_cost(params, m_conversionTree), func);
								}
							}
						}
					}
					// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, AND TEMPLATES)
					if (constructorScopePtr = std::dynamic_pointer_cast<Scope>(this->FindClass(functionName))) {
						// Is there a pre-defined constructor that this could work with?
						if (auto* functions = constructorScopePtr->GetFunctions()) {
							auto [funcParams, func] = functions->BuildMatch(functionName, params, m_conversionTree, true, true);
							if (func) {
								sort.emplace(funcParams.conversion_cost(params, m_conversionTree), func);
							}
						}

						// Can the conversion tree do this itself, without any help?
						if (params.size() == 1 && firstParamScopePtr) {
							if (m_conversionTree.Converts(firstParamScopePtr->GetClassType(), constructorScopePtr->GetClassType())) {
								auto func = make_callable([tree = m_conversionTree, toType = constructorScopePtr->GetClassType()](Any const& from)->Any {
									if (tree.Converts(from.Type(), toType)) {
										try {
											return tree.Convert(from, toType);
										}
										catch (exception::bad_boxed_cast const& e) {
											std::cout << Units::printf("This conversion is no longer available { %i }\n", __LINE__);
											throw exception::not_found_error(std::string("Conversion is no longer available: ") + e.what());
										}
									}
									else {
										std::cout << Units::printf("This conversion is no longer available { %i }\n", __LINE__);
										throw exception::not_found_error("Conversion is no longer available");
										// return Any();
									}
								});

								sort.emplace(func->conversion_cost(params, m_conversionTree), func);
							}
						}
					}
					for (auto& s : sort) { if (s.first != std::numeric_limits<double>::max()) { out = s.second; return true; } }
				}

				// IF ALL SEARCHES FAILED, PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS
				if (1) {
					if (constructorScopePtr) {
						if (auto* functions = constructorScopePtr->GetFunctions()) {
							auto [funcParams, func] = functions->BuildMatch(functionName, params, m_conversionTree, true, true);
							if (func) {
								if (funcParams.conversion_cost(params, m_conversionTree) != std::numeric_limits<double>::max()) {
									out = func;
									return true;
								}
							}
						}
					}					
				}

				return false;
			}
			else {
				std::string functionNameActual{ functionName.substr(lastOfColons + 1) };
				std::string scopeName{ functionName.substr(0, lastOfColons - 1) };

				std::shared_ptr<Scope> foundScope;
				if (this->TryFindScope(foundScope, scopeName)) {
					return foundScope->TryFindFunctionImpl(functionNameActual, params, m_conversionTree, out);
				}
				return false;
			}
		};

    public:
		virtual std::shared_ptr<fibers::Any> FindObject(std::string const& objectName /* x, y, etc. */) const {
			std::shared_ptr<fibers::Any> out{ nullptr };
			if (TryFindObjectImpl(objectName, out)) {
				return out;
			}
			else {
				return nullptr;
			}
		};
		virtual bool AddObject(std::string const& objectName, std::shared_ptr<fibers::Any> toAdd, bool overwriteIfExists = true) {
			std::string scopeName;
			std::string objectNameActual;
			size_t lastOfColons{ 0 };
			if ((lastOfColons = objectName.find_last_of("::")) == std::string::npos) {
				return p_objects.emplace(objectName, toAdd, overwriteIfExists);
			}
			else {
				objectNameActual = objectName.substr(lastOfColons + 1);
				scopeName = objectName.substr(0, lastOfColons - 1);
				std::shared_ptr<Scope> foundScope;
				if (this->TryFindScope(foundScope, scopeName)) {
					return foundScope->p_objects.emplace(std::move(objectNameActual), toAdd, overwriteIfExists);
				}
				return false;
			}
		};

		virtual Proxy_Function FindFunction(std::string const& functionName /* x, y, etc. */, scripting::Function_Params const& params) const {
			Proxy_Function out{ nullptr };
			if (TryFindFunctionImpl(functionName, params, this->GetCombinedTypeConversionTree(), out)) {
				return out;
			}
			else {
				return nullptr;
			}
		};
		virtual Proxy_Function FindFunction(std::string const& functionName /* x, y, etc. */, scripting::Function_Params const& params) {
			Proxy_Function out{ nullptr };
			if (TryFindFunctionImpl(functionName, params, this->GetCombinedTypeConversionTree(), out)) {
				return out;
			}
			else {
				return nullptr;
			}
		};

		// Attempts to find a class with the requested class name (may be a qualified namespace).
		virtual /*Type_Info*/ std::shared_ptr<Class> FindClass(std::string const& functionName) const {
			auto f = std::dynamic_pointer_cast<Scope>(FindNamespace(functionName));
			if (f && f->IsClass()) {
				return std::dynamic_pointer_cast<Class>(f);
			}
			return nullptr;
		};

		// Attempts to find a class with the requested class name (may be a qualified namespace).
		virtual /*Type_Info*/ std::shared_ptr<Class> FindClass(scripting::Type_Info classType) const {
			for (auto& Namespace : GetAvailableNamespaces(std::make_shared<std::set<std::shared_ptr<Scope>>>(), true, true)) { // forced to check ALL possible namespaces, since we are searching based on the ClassPtr, and not its name. 
				if (auto ptr = std::dynamic_pointer_cast<Scope>(Namespace.second.lock())) {
					if (ptr->IsClass()) {
						if (classType == ptr->GetClassType()) {
							return std::dynamic_pointer_cast<Class>(ptr);
						}
					}
				}
			}
			return nullptr;
		};

		// Attempts to find a class that has been requested based on the postfix (e.g. "_ft", "l", "f", "ll", "_m", or "_gal")
		virtual std::shared_ptr<Class> GetPostfix(std::string const& nonqualified_postfix /* = "_ft" */) {
			for (auto& scope : GetScopesForObjectSearch()) {
				if (auto* postfixes = scope->GetPostFixes()) {
					auto optional = postfixes->at(nonqualified_postfix);
					if (optional.has_value()) {
						if (auto ptr = optional.value().lock()) {
							return ptr;
						}
					}
				}
			}
			return nullptr;
		};

		// Call a generic, proxy function using a vector of inputs (may be empty), which will be converted as necessary to the expected types. 
		virtual Any CallFunction(std::string const& functionName, std::vector<Any> const& inputs) const {
			Proxy_Function out{ nullptr };
			auto& tree{ this->GetCombinedTypeConversionTree() };
			if (TryFindFunctionImpl(functionName, const_cast<std::vector<Any>&>(inputs), tree, out)) {
				return out->operator()(Function_Params{ const_cast<std::vector<Any>&>(inputs) }, tree);
			}
			else {
				throw exception::not_found_error(functionName);
			}
		};
		virtual Any CallFunction(std::string const& functionName, std::vector<Any> const& inputs, Type_Converter_Tree const& m_conversionTree) const {
			Proxy_Function out{ nullptr };
			if (TryFindFunctionImpl(functionName, const_cast<std::vector<Any>&>(inputs), m_conversionTree, out)) {
				return out->operator()(Function_Params{ const_cast<std::vector<Any>&>(inputs) }, m_conversionTree);
			}
			else {
				throw exception::not_found_error(functionName);
			}
		};

	};
	class Namespace : public Scope {
	public:
		friend class Scope;
		friend class Class;

		Namespace(std::weak_ptr<Scope> parent = std::weak_ptr<Scope>(), std::string const& Name = "")
			: Scope(parent)
			, p_Name{ Name }
		{};
		Namespace(Namespace const&) = default;
		Namespace(Namespace&&) = default;
		Namespace& operator=(Namespace const&) = default;
		Namespace& operator=(Namespace&&) = default;
		virtual ~Namespace() = default;

	protected:
		virtual const Functions* GetFunctions() const override { return &m_functions; };
		virtual Functions* GetFunctions() override { return &m_functions; };

	public:
		std::string
			p_Name; // e.g. "", or "_NAMESPACE_NAME_", or "_CLASS_NAME_"
		fibers::containers::Map<std::string, std::weak_ptr<Class>>
			m_postfixes{}; // allowed postfixes (e.g. 10_ft, where "_ft" is the key) to their desired typename. Duplicate are not allowed.
		Functions
			m_functions; // functions. (e.g. `==` or `to_string`). Duplicate names are expected. 

	public:
		virtual bool IsClass() const override { return false; };
		virtual bool IsNamespace() const override { return true; };
		virtual const std::string& GetName() const override {
			return p_Name;
		};
		/* Get all named namespaces that are discoverable from the current Scope */
	protected:
		virtual std::map<std::string, std::weak_ptr<Namespace>> GetAvailableNamespacesImpl(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true, bool useUniqueNames = false) const override {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > inherited > using > parent scopes

			std::map<std::string, std::weak_ptr<Namespace>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames); // GetAvailableNamespaces
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = usingScope->second.lock()) {
						for (auto& using_child : ((Scope*)ptr.get())->p_children) {
							if (using_child && using_child->second) {
								for (auto& using_namespace : ((Scope*)using_child->second.get())->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames)) { // GetAvailableNamespaces
									if (auto ptr = using_namespace.second.lock()) {
										out[GetName() + "::" + using_namespace.first] = ptr;
										if (requestedScope) out[using_namespace.first] = ptr;
									}
								}
							}
						}
					}
				}
			}

			// get my actual children last...
			for (auto& using_child : this->p_children) {
				if (using_child && using_child->second) {
					for (auto& using_namespace : ((Scope*)using_child->second.get())->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames)) { // GetAvailableNamespaces
						if (auto ptr = using_namespace.second.lock()) {
							out[GetName() + "::" + using_namespace.first] = ptr;
							if (requestedScope) out[using_namespace.first] = ptr;
						}
					}
				}
			}

			// also remember to add myself (if I am a namespace)...
			if (useUniqueNames) {
				out[this->p_NameRand + "_" + GetName()] = std::dynamic_pointer_cast<Namespace>(p_self.lock());
			}
			else {
				out[GetName()] = std::dynamic_pointer_cast<Namespace>(p_self.lock());
			}

			return out;
		};
	
	protected:
		virtual const fibers::containers::Map<std::string, std::weak_ptr<Class>>* GetPostFixes() const override {
			return &m_postfixes;
		};

		/*
		Get all namespaces that may be used to search for an object (e.g this scope, it's USING scopes, its PARENT scopes
		Ordered from ParentScope -> UsingScope -> ThisScope
		*/
		virtual std::map<int, std::shared_ptr<Scope>> GetScopesForObjectSearchImpl(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true) const override {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > using namespaces

			std::map<int, std::shared_ptr<Scope>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetScopesForObjectSearchImpl(evaluated, false);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = usingScope->second.lock()) {
						for (auto x : ptr->GetScopesForObjectSearchImpl(evaluated, false)) {
							out.emplace(out.size(), x.second);
						}
					}
				}
			}

			// also remember to add myself (if I am a namespace)...
			out.emplace(out.size(), p_self.lock());

			return out;
		};

	public:
		virtual size_t GetCurrentVersion(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>()) const override {
			constexpr auto A = 54059; /* a prime */
			constexpr auto B = 76963; /* another prime */
			constexpr auto C = 86969; /* yet another prime */
			constexpr auto FIRSTH = 37; /* also prime */
			size_t h = FIRSTH;

			evaluated->emplace(this->p_self.lock());

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					h = (h * A) ^ (ptr->GetCurrentVersion(evaluated) * B);
			}

			// get my "using" children... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = usingScope->second.lock()) {
						h = (h * A) ^ (ptr->GetCurrentVersion(evaluated) * B);
					}
				}
			}

			return h % C;
		};

    };
	class Class : public Namespace {
	public:
		friend class Scope;
		friend class Namespace;

		Class(
			std::weak_ptr<Scope> parent = std::weak_ptr<Scope>(), 
			std::string const& Name = "",
			std::weak_ptr<Class> inheritance = std::weak_ptr<Class>(), // e.g. this class derives from another Class
			fibers::Type_Info type = fibers::user_type<void>()
		)
			: Namespace(parent, Name)
			, DerivedFrom(inheritance)
		{
			if (auto ptr = DerivedFrom.lock()) {
				this->AddUsing(ptr);
			}

			if (type == user_type<void>()) {
				ClassType = std::make_shared<fibers::Type_Info>(this->p_NameRand /*GetQualifiedNamespace()*/, Name);
			}
			else {
				ClassType = std::make_shared<fibers::Type_Info>(type);
			}
		};
		Class(
			std::weak_ptr<Scope> parent,
			std::string const& Name,
			fibers::Type_Info type
		)
			: Namespace(parent, Name)
			, DerivedFrom(std::weak_ptr<Class>())
		{
			if (auto ptr = DerivedFrom.lock()) {
				this->AddUsing(ptr);
			}

			if (type == user_type<void>()) {
				ClassType = std::make_shared<fibers::Type_Info>(GetQualifiedNamespace(), Name);
			}
			else {
				ClassType = std::make_shared<fibers::Type_Info>(type);
			}
		};
		Class(
			fibers::Type_Info type,
			std::weak_ptr<Scope> parent,
			std::string const& Name,
			std::weak_ptr<Class> inheritance = std::weak_ptr<Class>() // e.g. this class derives from another Class
		)
			: Namespace(parent, Name)
			, DerivedFrom(inheritance)
		{
			if (auto ptr = DerivedFrom.lock()) {
				this->AddUsing(ptr);
			}

			if (type == user_type<void>()) {
				ClassType = std::make_shared<fibers::Type_Info>(GetQualifiedNamespace(), Name);
			}
			else {
				ClassType = std::make_shared<fibers::Type_Info>(type);
			}
		};

		Class(Class const&) = default;
		Class(Class&&) = default;
		Class& operator=(Class const&) = default;
		Class& operator=(Class&&) = default;
		virtual ~Class() = default;

	public:
		std::weak_ptr<Class> DerivedFrom; // e.g. this class derives from another Class
		std::shared_ptr<fibers::Type_Info> ClassType;

	public:
		virtual bool IsClass() const override { return true; };
		virtual bool IsNamespace() const override { return true; };
		virtual const std::string& GetName() const override {
			return this->p_Name;
		};
		virtual Type_Info GetClassType() const { return ClassType; };

	protected:
		/* Get all named namespaces that are discoverable from the current Scope */
		virtual std::map<std::string, std::weak_ptr<Namespace>> GetAvailableNamespacesImpl(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true, bool useUniqueNames = false) const override {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > inherited > using > parent scopes

			std::map<std::string, std::weak_ptr<Namespace>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = usingScope->second.lock()) {
						for (auto& using_child : ((Scope*)ptr.get())->p_children) {
							if (using_child && using_child->second) {
								// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
								for (auto& using_namespace : ((Scope*)using_child->second.get())->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames)) {
									if (auto ptr = using_namespace.second.lock()) {
										out[GetName() + "::" + using_namespace.first] = ptr;
										if (requestedScope) out[using_namespace.first] = ptr;
									}
								}
							}
						}
					}
				}
			}

			// get my inherited children second... 
			if (auto ptr = this->DerivedFrom.lock()) {
				for (auto& using_child : ((Scope*)ptr.get())->p_children) {
					if (using_child && using_child->second) {
						// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
							for (auto& using_namespace : ((Scope*)using_child->second.get())->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames)) {
								if (auto ptr = using_namespace.second.lock()) {
									out[GetName() + "::" + using_namespace.first] = ptr;
									if (requestedScope) out[using_namespace.first] = ptr;
								}
							}
					}
				}
				out[ptr->GetName()] = ptr;
			}

			// get my actual children last...
			for (auto& using_child : this->p_children) {
				if (using_child && using_child->second) {
					// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
						for (auto& using_namespace : ((Scope*)using_child->second.get())->GetAvailableNamespacesImpl(evaluated, false, useUniqueNames)) {
							if (auto ptr = using_namespace.second.lock()) {
								out[GetName() + "::" + using_namespace.first] = ptr;
								if (requestedScope) out[using_namespace.first] = ptr;
							}
						}
				}
			}

			// also remember to add myself (if I am a namespace)...
			if (useUniqueNames) {
				out[this->p_NameRand + "_" + GetName()] = std::dynamic_pointer_cast<Namespace>(p_self.lock());
			}
			else {
				out[GetName()] = std::dynamic_pointer_cast<Namespace>(p_self.lock());
			}

			return out;
		};
	
	protected:
		/*
		Get all namespaces that may be used to search for an object (e.g this scope, it's USING scopes, its PARENT scopes
		Ordered from ParentScope -> UsingScope -> ThisScope
		*/		
		virtual std::map<int, std::shared_ptr<Scope>> GetScopesForObjectSearchImpl(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true) const override {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > using namespaces

			std::map<int, std::shared_ptr<Scope>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetScopesForObjectSearchImpl(evaluated, false);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = usingScope->second.lock()) {
						for (auto& x : ptr->GetScopesForObjectSearchImpl(evaluated, false)) {
							out.emplace(out.size(), x.second);
						}
					}
				}
			}

			// get my inherited children second... 
			if (auto ptr = this->DerivedFrom.lock()) {
				for (auto& using_child : ((Scope*)ptr.get())->p_children) {
					if (using_child && using_child->second) {
						// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
						for (auto using_namespace : ((Scope*)using_child->second.get())->GetScopesForObjectSearchImpl(evaluated, false)) {
							out.emplace(out.size(), using_namespace.second);
						}
					}
				}
				out.emplace(out.size(), ptr);
			}

			// also remember to add myself (if I am a namespace)...
			out.emplace(out.size(), p_self.lock());

			return out;
		};
	
    public:
		virtual size_t GetCurrentVersion(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>()) const override {
			constexpr auto A = 54059; /* a prime */
			constexpr auto B = 76963; /* another prime */
			constexpr auto C = 86969; /* yet another prime */
			constexpr auto FIRSTH = 37; /* also prime */
			size_t h = FIRSTH;

			evaluated->emplace(this->p_self.lock());

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					h = (h * A) ^ (ptr->GetCurrentVersion(evaluated) * B);
			}

			// get my "using" children... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = usingScope->second.lock()) {
						h = (h * A) ^ (ptr->GetCurrentVersion(evaluated) * B);
					}
				}
			}

			// get my inherited children second... 
			if (auto ptr = this->DerivedFrom.lock()) {
				for (auto& using_child : ((Scope*)ptr.get())->p_children) {
					if (using_child && using_child->second) {
						h = (h * A) ^ (((Scope*)using_child->second.get())->GetCurrentVersion(evaluated) * B);
					}
				}
				h = (h * A) ^ (ptr->GetCurrentVersion(evaluated) * B);
			}

			return h % C;
		};

	};
	class Global final : public Namespace {
	public:
		friend class Scope;
		friend class Namespace;

		Global()
			: Namespace({}, "")
			, tree()
		{};

		Global(Global const&) = default;
		Global(Global&&) = default;
		Global& operator=(Global const&) = default;
		Global& operator=(Global&&) = default;
		~Global() = default; // not virtual b/c is final

	public:
		void AddBuiltIns() {
			auto defineBuiltInType = [this](auto typeImpl, std::string const& Name) -> void {
				// make it a class
				auto classPtr = std::dynamic_pointer_cast<Class>(this->p_children.get_or_insert(Name, std::make_shared<Class>(user_type<decltype(typeImpl)>(), this->p_self, Name)));
				classPtr->p_self = classPtr;

				// add converters
				this->tree.AddConverter<decltype(typeImpl), char>();
				this->tree.AddConverter<decltype(typeImpl), int>();
				this->tree.AddConverter<decltype(typeImpl), long>();
				this->tree.AddConverter<decltype(typeImpl), float>();
				this->tree.AddConverter<decltype(typeImpl), double>();
				this->tree.AddConverter<decltype(typeImpl), size_t>();
				this->tree.AddConverter<decltype(typeImpl), fibers::containers::number < double > >();
				this->tree.AddConverter<decltype(typeImpl), signed char>();
				this->tree.AddConverter<decltype(typeImpl), unsigned char>();
				this->tree.AddConverter<decltype(typeImpl), char16_t >();
				this->tree.AddConverter<decltype(typeImpl), char32_t >();
				this->tree.AddConverter<decltype(typeImpl), wchar_t >();
				this->tree.AddConverter<decltype(typeImpl), short>();
				this->tree.AddConverter<decltype(typeImpl), unsigned short>();
				this->tree.AddConverter<decltype(typeImpl), unsigned int>();
				this->tree.AddConverter<decltype(typeImpl), unsigned long>();
				this->tree.AddConverter<decltype(typeImpl), long long>();
				this->tree.AddConverter<decltype(typeImpl), long double>();
				this->tree.AddConverter([](decltype(typeImpl) o) -> std::string { return std::to_string(o); });

				// Constructors
				classPtr->m_functions.emplace(Name, make_callable([typeImplCopy = typeImpl]() -> decltype(typeImpl) { return typeImplCopy; })); // Default Constructor
				classPtr->m_functions.emplace(Name, make_callable([](decltype(typeImpl) const& o) -> decltype(typeImpl) { return (decltype(typeImpl))(o); })); // Copy Constructor

				// Comparisons
				classPtr->m_functions.emplace("==", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x == y; }));
				classPtr->m_functions.emplace("!=", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x != y; }));
				classPtr->m_functions.emplace("<", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x < y; }));
				classPtr->m_functions.emplace("<=", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x <= y; }));
				classPtr->m_functions.emplace(">", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x > y; }));
				classPtr->m_functions.emplace(">=", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x >= y; }));
				classPtr->m_functions.emplace("+", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x + y; }));
				classPtr->m_functions.emplace("-", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x - y; }));
				classPtr->m_functions.emplace("*", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x * y; }));
				classPtr->m_functions.emplace("/", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { if (y == 0) return std::numeric_limits<decltype(typeImpl)>::max(); else return x / y; }));
				classPtr->m_functions.emplace("+=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x += y; }));
				classPtr->m_functions.emplace("-=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x -= y; }));
				classPtr->m_functions.emplace("*=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x *= y; }));
				classPtr->m_functions.emplace("/=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { if (y == 0) x = std::numeric_limits<decltype(typeImpl)>::max(); else x /= y; }));


				// Functions
				classPtr->m_functions.emplace("max", make_callable([]() { return std::numeric_limits<decltype(typeImpl)>::max(); }));
				classPtr->m_functions.emplace("min", make_callable([]() { return std::numeric_limits<decltype(typeImpl)>::lowest(); }));
				classPtr->m_functions.emplace("to_string", make_callable([](decltype(typeImpl) const& o) -> std::string { return std::to_string(o); }));
				
			};

			// Built-in types
			if (1) {
#define DefineBuiltInType(V) defineBuiltInType(##V(0), #V);
				DefineBuiltInType(bool);
				FindClass("bool")->m_functions.emplace("to_string", make_callable([](bool const& o) -> std::string { if (o) { return "true"; } else { return "false"; } }));
				DefineBuiltInType(char);
				DefineBuiltInType(int);
				DefineBuiltInType(long);
				DefineBuiltInType(float);
				DefineBuiltInType(double);
				DefineBuiltInType(size_t);
				defineBuiltInType(fibers::containers::number<double>(), "Number");
				DefineBuiltInType(char16_t);
				DefineBuiltInType(char32_t);
				DefineBuiltInType(wchar_t);
				DefineBuiltInType(short);
				defineBuiltInType(unsigned char(0), "uchar");
				defineBuiltInType(unsigned short(0), "ushort");
				defineBuiltInType(unsigned int(0), "uint");
				defineBuiltInType(unsigned long(0), "ulong");
				defineBuiltInType(long long(0), "llong");
				defineBuiltInType(long double(), "ldouble");
#undef DefineBuiltInType

				// String
				if (1) {
					auto string_namespace = std::dynamic_pointer_cast<Class>(this->p_children.get_or_insert("string", std::make_shared<Class>(user_type<std::string>(), this->p_self, "string")));
					string_namespace->p_self = string_namespace;

					// Constructors
					string_namespace->m_functions.emplace("string", scripting::make_callable([]() -> std::string { return ""; })); // Default Constructor
					string_namespace->m_functions.emplace("string", scripting::make_callable([](std::string const& x) -> std::string { return x; })); // Copy Constructor
					
					// Comparisons
					string_namespace->m_functions.emplace("==", scripting::make_callable([](std::string const& x, std::string const& y) { return x == y; }));
					string_namespace->m_functions.emplace("!=", scripting::make_callable([](std::string const& x, std::string const& y) { return x != y; }));
					string_namespace->m_functions.emplace("<", scripting::make_callable([](std::string const& x, std::string const& y) { return x < y; }));
					string_namespace->m_functions.emplace("<=", scripting::make_callable([](std::string const& x, std::string const& y) { return x <= y; }));
					string_namespace->m_functions.emplace(">", scripting::make_callable([](std::string const& x, std::string const& y) { return x > y; }));
					string_namespace->m_functions.emplace(">=", scripting::make_callable([](std::string const& x, std::string const& y) { return x >= y; }));
					string_namespace->m_functions.emplace("+", scripting::make_callable([](std::string const& x, std::string const& y) { return x + y; }));

					// Functions
					string_namespace->m_functions.emplace("length", scripting::make_callable([](std::string const& x) -> size_t { return x.length(); }));
					string_namespace->m_functions.emplace("substr", scripting::make_callable([](std::string const& x, size_t Off) -> std::string { return x.substr(Off); }), { "input", "Off" });
					string_namespace->m_functions.emplace("substr", scripting::make_callable([](std::string const& x, size_t Off, size_t Count) -> std::string { return x.substr(Off, Count); }), { "input", "Off", "Count"});
					string_namespace->m_functions.emplace("size", scripting::make_callable([](std::string const& x) -> size_t { return x.size(); }));
					string_namespace->m_functions.emplace("front", scripting::make_callable([](std::string const& x) -> char { return x.front(); }));
					string_namespace->m_functions.emplace("back", scripting::make_callable([](std::string const& x) -> char { return x.back(); }));
					string_namespace->m_functions.emplace("at", scripting::make_callable([](std::string const& x, size_t Off) -> char { return x.at(Off); }), { "input", "Off" });
					string_namespace->m_functions.emplace("to_string", make_callable([](std::string const& o) -> std::string { return o; }));

					// Objects or Constants
					string_namespace->AddObject("npos", std::make_shared<Any>(std::string::npos));
				}


			}

			// Built-In functions
			if (1) {
				// Returns the type of the Any obj
				this->m_functions.emplace("Type", scripting::Param_Types({ { std::string("obj"), scripting::user_type<Any>() } }), scripting::make_callable(
					[](Any const& x) -> fibers::Type_Info {
						if (auto p = x.Type().lock())
							return *p;
						else
							return fibers::user_type<void>();
					}
				), true);
				
				// Returns a stringified version of the provided Any obj. This is meant to be a fall-back. 
				this->m_functions.emplace("to_string", scripting::Param_Types({ { "obj", user_type<Any>() } }), scripting::make_callable(
					[](Any const& x) -> std::string { return Units::printf("`%s`", x.TypeName()); }
				));
			}
		};

	public:

		// Return the conversion tree(s) from the nearest Global namespace
		virtual void TypeConversionTrees(std::vector<Type_Converter_Tree*>& out, std::set<size_t>& previous)  override {
			if (previous.find((size_t)p_self.lock().get()) == previous.end()) {
				previous.insert((size_t)p_self.lock().get());
				for (auto& Using : p_using) {
					if (Using) {
						if (auto ptr = std::dynamic_pointer_cast<Scope>(Using->second.lock())) {
							ptr->TypeConversionTrees(out, previous);
						}
					}
				}
				out.push_back(&tree);
			}

		};
		// Return the conversion tree(s) from the nearest Global namespace
		virtual void TypeConversionTrees(std::vector<const Type_Converter_Tree*>& out, std::set<size_t>& previous) const override {
			if (previous.find((size_t)p_self.lock().get()) == previous.end()) {
				previous.insert((size_t)p_self.lock().get());
				for (auto& Using : p_using) {
					if (Using) {
						if (auto ptr = std::dynamic_pointer_cast<Scope>(Using->second.lock())) {
							ptr->TypeConversionTrees(out, previous);
						}
					}
				}
				out.push_back(&tree);
			}

		};
		virtual Type_Converter_Tree* TypeConversionTree() override {
			return &tree;
		};
		virtual const Type_Converter_Tree* TypeConversionTree() const override {
			return &tree;
		};

	public:
		Type_Converter_Tree tree;
		mutable fibers::synchronization::atomic_number<size_t> p_version;

		virtual size_t GetCurrentVersion(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>()) const override {
			constexpr auto A = 54059; /* a prime */
			constexpr auto B = 76963; /* another prime */
			constexpr auto C = 86969; /* yet another prime */
			constexpr auto FIRSTH = 37; /* also prime */
			size_t h = FIRSTH;

			evaluated->emplace(this->p_self.lock());

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					h = (h * A) ^ (ptr->GetCurrentVersion(evaluated) * B);
			}

			// get my "using" children... 
			for (auto& usingScope : this->p_using) {
				if (usingScope) {
					if (auto ptr = usingScope->second.lock()) {
						h = (h * A) ^ (ptr->GetCurrentVersion(evaluated) * B);
					}
				}
			}

			// also remember to add myself (if I am a Global)...
			h = (h * A) ^ (p_version.load() * B);

			return h % C;
		};
	};
#endif

	class Scope2;
	class Namespace2;
	class Class2;
	class Global2;

	class Scope2 {
	public:
		friend class Namespace2;
		friend class Class2;
		friend class Global2;

		Scope2(std::shared_ptr<Scope2> const& parent)
			: p_UniqueName(" _ _ _ _ _ _")
			, p_self()
			, p_parent(parent)
			, p_namespace()
			, p_library()
			, p_using()
		{
			static auto randN{ [](double min, double max) -> double { return (((double)std::rand() / (double)RAND_MAX) * (max - min)) + min; } };
			for (int i = 0; i < 12; i++) {
				if (i < 2)
					p_UniqueName[i] = (char)(int)randN((int)('A'), (int)('Z'));
				else if (i < 6)
					p_UniqueName[i] = (char)(int)randN((int)('0'), (int)('9'));
				else if (i < 8)
					p_UniqueName[i] = (char)(int)randN((int)('A'), (int)('Z'));
				else if (i < 12)
					p_UniqueName[i] = (char)(int)randN((int)('0'), (int)('9'));
			};
			
			if (auto p = p_parent.lock()) { p_namespace = p->GetNamespaceImpl(); }
			else { p_namespace = std::dynamic_pointer_cast<Namespace2>(p_self.lock()); }

			if (auto p = p_parent.lock()) { p_library = p->GetLibraryImpl(); }
			else { p_library = std::dynamic_pointer_cast<Global2>(p_self.lock()); }

			qualifiedNamespaceWithQualifiers = GetQualifiedNamespaceImpl(true);			
			qualifiedNamespaceWithoutQualifiers = GetQualifiedNamespaceImpl();
		};
		Scope2(std::shared_ptr<Namespace2> const& parent) : Scope2(std::dynamic_pointer_cast<Scope2>(parent)) {};
		Scope2(std::shared_ptr<Class2> const& parent) : Scope2(std::dynamic_pointer_cast<Scope2>(parent)) {};
		Scope2(std::shared_ptr<Global2> const& parent) : Scope2(std::dynamic_pointer_cast<Scope2>(parent)) {};
		
		Scope2(Scope2 const&) = default;
		Scope2(Scope2&&) = default;
		Scope2& operator=(Scope2 const&) = default;
		Scope2& operator=(Scope2&&) = default;
		virtual ~Scope2() = default;

	private:
		std::string // randomly generated, truly unique name. 
			p_UniqueName; 
	public:
		class Hasher {
		public:
			Hasher() = default;
			~Hasher() = default;
			Hasher(Hasher const&) = default;
			Hasher(Hasher &&) = default;
			Hasher& operator=(Hasher const&) = default;
			Hasher& operator=(Hasher&&) = default;

			size_t operator()(std::weak_ptr<Scope2> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope2>>()(ptr.lock());

				//static auto hasher{ std::hash<std::string>() };
				//if (auto p = ptr.lock()) {
				//	return hasher(p->p_UniqueName);
				//} else {
				//	return 37; // prime
				//}
			};
			size_t operator()(std::weak_ptr<Namespace2> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope2>>()(std::dynamic_pointer_cast<Scope2>(ptr.lock()));

				//static auto hasher{ std::hash<std::string>() };
				//if (auto p = ptr.lock()) {
				//	if (auto p2 = (Scope2*)(void*)p.get()) {
				//		return hasher(p2->p_UniqueName);
				//	}					
				//}
				//else {
				//	return 37; // prime
				//}
			};
			size_t operator()(std::weak_ptr<Class2> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope2>>()(std::dynamic_pointer_cast<Scope2>(ptr.lock()));

				//static auto hasher{ std::hash<std::string>() };
				//if (auto p = ptr.lock()) {
				//	if (auto p2 = (Scope2*)(void*)p.get()) {
				//		return hasher(p2->p_UniqueName);
				//	}
				//}
				//else {
				//	return 37; // prime
				//}
			};
			size_t operator()(std::weak_ptr<Global2> const& ptr) const noexcept {
				return std::hash<std::shared_ptr<Scope2>>()(std::dynamic_pointer_cast<Scope2>(ptr.lock()));

				//static auto hasher{ std::hash<std::string>() };
				//if (auto p = ptr.lock()) {
				//	if (auto p2 = (Scope2*)(void*)p.get()) {
				//		return hasher(p2->p_UniqueName);
				//	}
				//}
				//else {
				//	return 37; // prime
				//}
			};
			
			size_t operator()(std::shared_ptr<Scope2> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope2>>()(p);

				//static auto hasher{ std::hash<std::string>() };
				//if (p) {
				//	return hasher(p->p_UniqueName);
				//}
				//else {
				//	return 37; // prime
				//}
			};
			size_t operator()(std::shared_ptr<Namespace2> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope2>>()(std::dynamic_pointer_cast<Scope2>(p));

				//static auto hasher{ std::hash<std::string>() };
				//if (p) {
				//	if (auto p2 = (Scope2*)(void*)p.get()) {
				//		return hasher(p2->p_UniqueName);
				//	}
				//}
				//else {
				//	return 37; // prime
				//}
			};
			size_t operator()(std::shared_ptr<Class2> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope2>>()(std::dynamic_pointer_cast<Scope2>(p));

				//static auto hasher{ std::hash<std::string>() };
				//if (p) {
				//	if (auto p2 = (Scope2*)(void*)p.get()) {
				//		return hasher(p2->p_UniqueName);
				//	}
				//}
				//else {
				//	return 37; // prime
				//}
			};
			size_t operator()(std::shared_ptr<Global2> const& p) const noexcept {
				return std::hash<std::shared_ptr<Scope2>>()(std::dynamic_pointer_cast<Scope2>(p));

				//static auto hasher{ std::hash<std::string>() };
				//if (p) {
				//	if (auto p2 = (Scope2*)(void*)p.get()) {
				//		return hasher(p2->p_UniqueName);
				//	}
				//}
				//else {
				//	return 37; // prime
				//}
			};
		};

	private:
		std::string GetQualifiedNamespaceImpl(bool GetUniqueQualifier = false) const {
			std::string path = "::";

			auto parent = p_parent.lock();

			if (!parent) {
				path = "::";
			}
			else {
				auto name{ GetName() };
				if (GetUniqueQualifier) {
					if (name == "") {
						name = p_UniqueName;
					}
					else {
						name = p_UniqueName + "_" + name;
					}
				}
				if (!name.empty()) {
					path = parent->GetQualifiedNamespace(GetUniqueQualifier) + name + "::";
				}
				else {
					path = parent->GetQualifiedNamespace(GetUniqueQualifier);
				}
			}

			while (path.find("::::") != std::string::npos) {
				size_t start_pos = 0;
				while ((start_pos = path.find("::::", start_pos)) != std::string::npos) {
					path = path.replace(start_pos, 4, "::");
					start_pos += 2; // In case 'to' contains 'from', like replacing 'x' with 'yx'
				}
			}

			return path;
		};

	private:
		std::weak_ptr<Scope2> // shared_pointer to itself. MUST be set immediately after creating the Scope/Class/Namespace/Global.
			p_self{}; 
		std::string
			qualifiedNamespaceWithQualifiers{};
		std::string
			qualifiedNamespaceWithoutQualifiers{};

	public:
		void SetSelf(std::shared_ptr<Scope2>& p) { p_self = p; };
		virtual bool IsClass() const { return false; };
		virtual bool IsNamespace() const { return false; };
		virtual std::string GetName() const { return ""; };
		std::string const& GetQualifiedNamespace(bool GetUniqueQualifier = false) const {
			if (GetUniqueQualifier) {
				return qualifiedNamespaceWithQualifiers;
			}
			else {
				return qualifiedNamespaceWithoutQualifiers;
			}
		};

	private:
		std::weak_ptr<Scope2> // parent scope, for navigation. Could be anything, or null.
			p_parent{}; 
		std::weak_ptr<Namespace2> // parent's parent's ... parent's scope. The logical result of asking for p_parent on repeat until you get the first Namespace2 type. 
			p_namespace{};
		// if Namespace or Class or Global, returns self. Otherwise, returns the parent's Namespace. 
		std::weak_ptr<Namespace2> GetNamespaceImpl() const {
			if (auto p = std::dynamic_pointer_cast<Namespace2>(p_self.lock())) {
				return p;
			}
			else {
				return p_namespace;
			}
		};

	public:
		// if Namespace or Class or Global, returns self. Otherwise, returns the parent's Namespace. 
		std::shared_ptr<Namespace2> GetNamespace() const { return p_namespace.lock(); };
	
    private:
		fibers::containers::Map<std::string, std::shared_ptr<fibers::Any>>
			p_objects; // scopes of all types may declare objects. Namespace objects may be global objects, but still. 

	public:
		// try and find the object with the requested key.
		std::shared_ptr<fibers::Any> GetObj(std::string const& name) const {
			return p_objects.at_or(name, nullptr);
		};
		// Returns true if successful. Returns false is replaceIfExisting==false and the object already existed on the Scope.
		bool AddObj(std::string const& name, std::shared_ptr<fibers::Any> const& obj, bool replaceIfExisting = true) {
			return p_objects.emplace(name, obj, replaceIfExisting);
		};
		// Returns true if successful.
		bool EraseObj(std::string const& name) {
			return p_objects.erase(name);
		};
		// Returns true if successful.
		bool EraseObj(std::shared_ptr<fibers::Any> const& Obj) {
			std::string key;
			bool doErasure = false;
			for (auto& obj : p_objects) {
				if (obj && (obj->second == Obj)) {
					key = obj->first;
					doErasure = true;
					break;
				}
			}
			if (doErasure) return EraseObj(key);
			else return false;
		};

	private:
		std::weak_ptr<Global2> // parent's parent's ... parent's scope. The logical result of asking for p_parent on repeat until you get the end. 
			p_library{}; 
		// if Global, returns self. Otherwise, returns the parent's Library. 
		std::weak_ptr<Global2> GetLibraryImpl() const {
			if (auto p = std::dynamic_pointer_cast<Global2>(p_self.lock())) {
				return p;
			}
			else {
				return p_library;
			}
		};

	public:
		// if Global, returns self. Otherwise, returns the parent's Library. 
		std::shared_ptr<Global2> GetLibrary() const { 
			if (auto p = p_library.lock())
				return p;
			else if (auto p = std::dynamic_pointer_cast<Global2>(p_self.lock()))
				return p;
			else
				return nullptr;
		};

	public:
		fibers::containers::Map< size_t, std::weak_ptr<Namespace2>> // allows this scope to use the children of other scopes as if they were their own.
			p_using; 
		// the Library should know about our "using" list
		virtual bool RecordUsing(std::shared_ptr<Namespace2> ptr, bool overrideIfExists = true) {
			if (auto p = std::dynamic_pointer_cast<Scope2>(GetLibrary())) {
				return p->RecordUsing(ptr, overrideIfExists);
			}
			return false;
		};

	public:
		// allows this scope to use the children of other scopes as if they were their own.
		bool AddUsing(std::weak_ptr<Namespace2> namespacePtr) {
			if (auto p = std::dynamic_pointer_cast<Scope2>(namespacePtr.lock())) {
				if (p_using.emplace(Hasher()(p), namespacePtr)) {
					if (p->IsNamespace()) {
						// if this "namespacePtr" belongs to our same library, then we do not care. We only care to track other libraries being used.
						if (p->GetLibrary() != this->GetLibrary()) {
							(void)RecordUsing(std::dynamic_pointer_cast<Namespace2>(p));
						}
					}
					return true;
				}
			}
			return false;
		};
		
	private:
		fibers::containers::Map<std::string, 
			std::shared_ptr<
			    fibers::containers::Map<size_t, std::shared_ptr<Namespace2>>
			>
		> // children namespaces - may be classes or namespaces. By this design, imported namespaces may be "unloaded" on scope unloading, which is on purpose.
			p_children;
		// the Library should know about our "Class" list
		virtual bool RecordClass(std::shared_ptr<Class2> ptr, bool overrideIfExists = true) {
			if (auto p = std::dynamic_pointer_cast<Scope2>(GetLibrary())) {
				return p->RecordClass(ptr, overrideIfExists);
			}			
			return false;
		};

	public:
		bool AddChild(std::shared_ptr<Namespace2> NamespacePtr) {
			if (auto p = std::dynamic_pointer_cast<Scope2>(NamespacePtr)) {
				auto name = p->GetName();
				auto ptr = p_children.get_or_insert(name, std::make_shared<fibers::containers::Map<size_t, std::shared_ptr<Namespace2>>>());
				if (ptr->emplace(Hasher()(NamespacePtr), NamespacePtr)) {
					if (p->IsClass()) {
						(void)RecordClass(std::dynamic_pointer_cast<Class2>(p));
					}
					return true;
				}
			}
			return false;
		};

	private:
		virtual void RemoveStaleReferences() {
			// p_using
			while (true) {
				size_t toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_using) {
					if (ref) {
						if (ref->second.expired()) {
							toRemove = ref->first;
							doRemoval = true;
							break;
						}
					}
				}
				if (doRemoval) {
					p_using.erase(toRemove);
				}
				else {
					break;
				}
			}
		};

		virtual bool TryFindNearestScopeWhere(
			std::shared_ptr<Scope2>& bestMatch,
			std::function<bool(std::shared_ptr<Scope2> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope2> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope2> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope2> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope2> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
				checkedSelf.emplace(selfPtr);
				if (1) {
					if (auto p = std::dynamic_pointer_cast<Scope2>(selfPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}

			// test my "using" namespaces and their children.
			for (auto& childNamespace : this->p_using) {
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope2>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test all of my parents
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!checkedSelf.count(parentPtr) >= 1) {
					checkedSelf.emplace(parentPtr);
					if (1) {
						if (auto p = std::dynamic_pointer_cast<Scope2>(parentPtr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
				parentPtr = parentPtr->p_parent.lock();
			}

			// Test my children themselves.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope2>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
								checkedSelf.emplace(ptr);
								if (1) {
									if (auto p = std::dynamic_pointer_cast<Scope2>(ptr)) {
										if (func(p)) {
											bestMatch = p;
											return true;
										}
									}
								}
							}
							else {
								break; // we've checked this before! Quick, get out. 
							}
						}
					}
				}
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope2>(innerChildNamespace->second)) {
								if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};


		virtual bool TryFindNearestNamespaceWhere(
			std::shared_ptr<Namespace2>& bestMatch,
			std::function<bool(std::shared_ptr<Namespace2> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope2> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope2> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope2> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope2> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr)>=1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
				checkedSelf.emplace(selfPtr);
				if (this->IsNamespace()) {
					if (auto p = std::dynamic_pointer_cast<Namespace2>(selfPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}

			// test my "using" namespaces and their children.
			for (auto& childNamespace : this->p_using) {
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope2>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test all of my parents
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!checkedSelf.count(parentPtr) >= 1) {
					checkedSelf.emplace(parentPtr);
					if (parentPtr->IsNamespace()) {
						if (auto p = std::dynamic_pointer_cast<Namespace2>(parentPtr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
				parentPtr = parentPtr->p_parent.lock();				
			}

			// Test my children themselves.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope2>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
								checkedSelf.emplace(ptr);
								if (ptr->IsNamespace()) {
									if (auto p = std::dynamic_pointer_cast<Namespace2>(ptr)) {
										if (func(p)) {
											bestMatch = p;
											return true;
										}
									}
								}
							}
							else {
								break; // we've checked this before! Quick, get out. 
							}
						}
					}
				}
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope2>(innerChildNamespace->second)) {
								if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};
	
    protected:
		virtual Type_Info GetClassType() const { return Type_Info(); };

	public:
		virtual bool AddFunction(std::string const& name, Proxy_Function function, bool overrideIfAlreadyExists) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(GetNamespace())) {
				return namespacePtr->AddFunction(name, function, overrideIfAlreadyExists);
			}
			else {
				return false;
			}
		};
		virtual bool AddFunction(std::string const& name, Proxy_Function function, Param_Types const& params, bool overrideIfAlreadyExists = true) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(GetNamespace())) {
				return namespacePtr->AddFunction(name, function, params, overrideIfAlreadyExists);
			}
			else {
				return false;
			}
		};

	private:
		virtual bool AddFreeFunction(std::string const& name, Proxy_Function function, bool overrideIfAlreadyExists = true) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(GetNamespace())) {
				return namespacePtr->AddFreeFunction(name, function, overrideIfAlreadyExists);
			}
			else {
				return false;
			}
		};
		virtual bool AddFreeFunction(std::string const& name, Proxy_Function function, Param_Types const& params, bool overrideIfAlreadyExists = true) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(GetNamespace())) {
				return namespacePtr->AddFreeFunction(name, function, params, overrideIfAlreadyExists);
			}
			else {
				return false;
			}
		};

	public:
#if 0
		template<typename FromType, typename ToType>
		bool AddConstructors(bool overrideIfAlreadyExists = true) {
			auto fromTypeInfo{ scripting::user_type<FromType>() };
			auto toTypeInfo{ scripting::user_type<ToType>() };

			// Ensure these two classes have been added already
			auto fromClass = std::dynamic_pointer_cast<Scope2>(this->FindClass(fromTypeInfo));
			auto toClass = std::dynamic_pointer_cast<Scope2>(this->FindClass(toTypeInfo));
			if (!fromClass) throw exception::not_found_error("Could not find the \"From\" Class for Constructor"); 
			if (!toClass) throw exception::not_found_error("Could not find the \"To\" Class for Constructor"); 

			constexpr static bool is_polymorphic = std::is_base_of< ToType, FromType>::value;
			constexpr static bool is_static = details::impl::is_explicitly_convertible_to<FromType, ToType>::value;
			constexpr static bool is_bidir = details::impl::is_explicitly_convertible_to<ToType, FromType>::value;
			if constexpr (!is_static && !is_polymorphic) {
				return false;
			}

			// static conversion
			toClass->AddFunction(toClass->GetName(), make_callable([](FromType const& from) -> ToType { return (FromType)from; }));

			// polymorphic conversion
			toClass->AddFreeFunction(toClass->GetName(), make_callable([](Any const& from) -> std::shared_ptr<ToType> {
				return std::dynamic_pointer_cast<ToType>(from.cast<std::shared_ptr<FromType>>());
			}), { {"from", fromTypeInfo } });








			value_namespace->AddFunction("value", make_callable([](Units::value const& makeCopy) -> Units::value { return makeCopy; }));










			std::shared_ptr<Node> node = nodes.get_or_insert(fromTypeInfo, std::make_shared<Node>());
			node->from = fromTypeInfo;

			auto targetLocation = node->connections.at_or(toTypeInfo, nullptr);
			if (!targetLocation) {
				if constexpr (std::is_base_of< ToType, FromType>::value) {
					targetLocation = node->connections.get_or_insert(toTypeInfo, std::dynamic_pointer_cast<details::Type_Conversion_Base>(std::make_shared<details::Dynamic_Type_Conversion_Impl<FromType, ToType>>()));
				}
				else {
					targetLocation = node->connections.get_or_insert(toTypeInfo, std::dynamic_pointer_cast<details::Type_Conversion_Base>(std::make_shared<details::Static_Type_Conversion_Impl<FromType, ToType>>()));
				}

				(void)targetLocation->cost(); // cache the cost to perform this conversion

				node->cached_conversions.emplace(toTypeInfo, std::make_shared<std::tuple<std::vector<scripting::Type_Info>, long>>(
					std::vector<scripting::Type_Info>({ toTypeInfo }),
					(long)(std::numeric_limits<double>::max())
					)); // even if there was a previous cached conversion, override it.

					// if this converter was bidirectional, we should explicitely add it to the list.
					// This will be slightly recursive but should end abruptly. 
				if constexpr (is_static && is_bidir) {
					AddConverter<ToType, FromType>();
				}

				version++;

				return true;
			}

			return false;


			//auto FromType = user_type<From>();
			//auto ToType = user_type<To>();

			//GetTypeConverterTree()->AddConverter();










			//if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(GetNamespace())) {
			//	return namespacePtr->AddFunction(name, function, params, overrideIfAlreadyExists);
			//}
			//else {
			//	return false;
			//}
		};
#endif


		virtual std::shared_ptr< Functions2 > GetFunctions() const {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(GetNamespace())) {
				return namespacePtr->GetFunctions();
			}
			else {
				return nullptr;
			}
		};
		virtual std::shared_ptr< Functions2::FunctionSort > GetFunctions(std::string const& name) const {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(GetNamespace())) {
				return namespacePtr->GetFunctions(name);
			}
			else {
				return nullptr;
			}
		};
		virtual Functions2::FunctionActualPtr GetFunction(std::string const& name, Function_Params const& params) {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(GetNamespace())) {
				return namespacePtr->GetFunction(name, params);
			}
			else {
				return {};
			}
		};
		virtual Functions2::FunctionActualPtr GetFunction(std::string const& name, Function_Params const& params, Type_Converter_Tree const& tree)  {
			if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(GetNamespace())) {
				return namespacePtr->GetFunction(name, params, tree);
			}
			else {
				return {};
			}
		};

	public:
		std::shared_ptr<Scope2> FindNearestScopeWhere(std::function<bool(std::shared_ptr<Scope2> const&)> const& func) const {
			std::shared_ptr<Scope2> out;
			if (TryFindNearestScopeWhere(out, func)) {
				return out;
			}
			else {
				return nullptr;
			}
		};
		std::shared_ptr<Namespace2> FindNearestNamespaceWhere(std::function<bool(std::shared_ptr<Namespace2> const&)> const& func) const {
			std::shared_ptr<Namespace2> out;
			if (TryFindNearestNamespaceWhere(out, func)) {
				return out;
			}
			else {
				return nullptr;
			}
		};
		std::shared_ptr<Namespace2> FindNamespace(std::string QualifiedOrUnqualifiedNamespaceName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			
			QualifiedOrUnqualifiedNamespaceName = fixNamespace(QualifiedOrUnqualifiedNamespaceName);

			if (QualifiedOrUnqualifiedNamespaceName == "" || QualifiedOrUnqualifiedNamespaceName == "::") { return std::dynamic_pointer_cast<Namespace2>(this->GetLibrary()); }

			std::shared_ptr<Namespace2> out;
#if 1
			long long len = QualifiedOrUnqualifiedNamespaceName.length();
			if (TryFindNearestNamespaceWhere(out, [&len, tryFind = QualifiedOrUnqualifiedNamespaceName](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
				auto qualifiedName = std::dynamic_pointer_cast<Scope2>(namespacePtr)->GetQualifiedNamespace();

				// remove "::" from end
				while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

				long long qualifiedNameLen = qualifiedName.length();
				auto F = qualifiedName.find(tryFind);
				if ((F != std::string::npos) && (F == (qualifiedNameLen - len))) return true;				
				return false;
			})) {
				return out;
			}
#else
			auto firstOfColons = QualifiedOrUnqualifiedNamespaceName.find_first_of("::");
			if (firstOfColons == std::string::npos) {
				// straight name
				if (TryFindNearestNamespaceWhere(out, [tryFind = QualifiedOrUnqualifiedNamespaceName](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					long long len = tryFind.length();
					auto qualifiedName = std::dynamic_pointer_cast<Scope2>(namespacePtr)->GetQualifiedNamespace();

					// remove "::" from end
					while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

					long long qualifiedNameLen = qualifiedName.length();
					auto F = qualifiedName.find(tryFind);
					if (F != std::string::npos) {
						if (F == (qualifiedNameLen - len)) {
							return true;
						}
					}
					return false;
				})) {
					return out;
				}
			}
			else {
				// split the problem into pieces
				std::vector<std::string> parts; {
					std::string parts_str = QualifiedOrUnqualifiedNamespaceName;
					std::string prev = "";
					while (firstOfColons != std::string::npos) {
						parts.push_back(parts_str.substr(0, firstOfColons)); // std
						parts_str = parts_str.substr(firstOfColons + 2); // string::...
						firstOfColons = parts_str.find("::");
					}
					parts.push_back(parts_str);
				}

				// Find the first part
				auto partPtr = FindNamespace(parts[0]);
				for (int i = 1; i < parts.size(); i++) {
					if (auto p = std::dynamic_pointer_cast<Scope2>(partPtr)) {
						if (auto ptr = p->p_children.at_or(parts[i], nullptr)) {
							if (ptr->size() >= 1) {
								// all of these are *technically* compatible... 
								auto subPtr = ptr->first();
								if (subPtr && subPtr->second) {
									partPtr = subPtr->second;
								}
								else {
									partPtr = nullptr;
									break;
								}
							}
							else {
								partPtr = nullptr;
								break;
							}
						}
						else {
							partPtr = nullptr;
							break;
						}
					}
				}
				if (auto p = std::dynamic_pointer_cast<Scope2>(partPtr)) {
					if (p->TryFindNearestNamespaceWhere(out, [tryFind = QualifiedOrUnqualifiedNamespaceName](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
						long long len = tryFind.length();
						auto qualifiedName = std::dynamic_pointer_cast<Scope2>(namespacePtr)->GetQualifiedNamespace();

						// remove "::" from end
						while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

						long long qualifiedNameLen = qualifiedName.length();
						auto F = qualifiedName.find(tryFind);
						if (F != std::string::npos) {
							if (F == (qualifiedNameLen - len)) {
								return true;
							}
						}
						return false;
					})) {
						return out;
					}
				}

				// try as-requested. This probably made the search slower, as a result. 
				if (this->TryFindNearestNamespaceWhere(out, [tryFind = QualifiedOrUnqualifiedNamespaceName](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					long long len = tryFind.length();
					auto qualifiedName = std::dynamic_pointer_cast<Scope2>(namespacePtr)->GetQualifiedNamespace();

					// remove "::" from end
					while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

					long long qualifiedNameLen = qualifiedName.length();
					auto F = qualifiedName.find(tryFind);
					if (F != std::string::npos) {
						if (F == (qualifiedNameLen - len)) {
							return true;
						}
					}
					return false;
				})) { return out; }				
			}
#endif		
			return nullptr;			
		};
		std::shared_ptr<Class2> FindClass(fibers::Type_Info typeInfo) const {
			std::shared_ptr<Namespace2> out;
			if (TryFindNearestNamespaceWhere(out, [tryFind = std::move(typeInfo)](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
				if (auto ptr = std::dynamic_pointer_cast<Scope2>(namespacePtr)) {
					if (ptr->IsClass()) {
						if (tryFind == ptr->GetClassType()) {
							return true;
						}
					}
				}
				return false;
			})) {
				return std::dynamic_pointer_cast<Class2>(out);
			}
			else {
				return nullptr;
			}
		};
		std::shared_ptr<Class2> FindClass(Type_Info typeInfo) const {
			std::shared_ptr<Namespace2> out;
			if (TryFindNearestNamespaceWhere(out, [tryFind = std::move(typeInfo)](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
				if (auto ptr = std::dynamic_pointer_cast<Scope2>(namespacePtr)) {
					if (ptr->IsClass()) {
						if (tryFind == ptr->GetClassType()) {
							return true;
						}
					}
				}
				return false;
			})) {
				return std::dynamic_pointer_cast<Class2>(out);
			}
			else {
				return nullptr;
			}
		};
		std::shared_ptr<Class2> FindClass(std::string QualifiedOrUnqualifiedNamespaceName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };

			QualifiedOrUnqualifiedNamespaceName = fixNamespace(QualifiedOrUnqualifiedNamespaceName);

			if (QualifiedOrUnqualifiedNamespaceName == "" || QualifiedOrUnqualifiedNamespaceName == "::") { return nullptr; }

			std::shared_ptr<Namespace2> out;

			long long len = QualifiedOrUnqualifiedNamespaceName.length();
			if (TryFindNearestNamespaceWhere(out, [&len, tryFind = QualifiedOrUnqualifiedNamespaceName](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
				if (std::dynamic_pointer_cast<Scope2>(namespacePtr)->IsClass()) {
					auto qualifiedName = std::dynamic_pointer_cast<Scope2>(namespacePtr)->GetQualifiedNamespace();

					// remove "::" from end
					while (qualifiedName.size() >= 2 && (qualifiedName.rfind("::") == (qualifiedName.length() - 2))) { qualifiedName = qualifiedName.substr(0, qualifiedName.length() - 2); }

					long long qualifiedNameLen = qualifiedName.length();
					auto F = qualifiedName.find(tryFind);

					if ((F != std::string::npos) && (F == (qualifiedNameLen - len))) return true;
				}
				return false;
			})) {
				return std::dynamic_pointer_cast<Class2>(out);
			}
			return nullptr;
		};

		std::shared_ptr<Namespace2> FindNamespaceWithObj(std::string objName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			objName = fixNamespace(objName);

			auto lastOfColons = objName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				std::shared_ptr<Namespace2> out;
				if (TryFindNearestNamespaceWhere(out, [&objName](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					if (auto ptr = std::dynamic_pointer_cast<Scope2>(namespacePtr)) {
						if (auto objFound = ptr->GetObj(objName)) {
							return true;
						}
					}
					return false;
				})) {
					return out;
				}
				else {
					return nullptr;
				}
			}
			else {
				std::string Namespace = objName.substr(0, lastOfColons-1);
				objName = objName.substr(lastOfColons + 1);
				if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(FindNamespace(Namespace))) {
					return namespacePtr->FindNamespaceWithObj(objName);
				}
				else {
					return nullptr;
				}
			}
		};
		std::shared_ptr<fibers::Any> FindObj(std::string objName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			objName = fixNamespace(objName);

			auto lastOfColons = objName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				if (auto ptr = std::dynamic_pointer_cast<Scope2>(FindNamespaceWithObj(objName))) {
					return ptr->GetObj(objName);
				}
				else {
					return nullptr;
				}
			}
			else {
				std::string Namespace = objName.substr(0, lastOfColons-1);
				objName = objName.substr(lastOfColons + 1);
				if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(FindNamespace(Namespace))) {
					return namespacePtr->FindObj(objName);
				}
				else {
					return nullptr;
				}
			}
		};

		std::shared_ptr<Namespace2> FindNamespaceWithFunction(std::string functionName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				std::shared_ptr<Namespace2> out;
				if (TryFindNearestNamespaceWhere(out, [&functionName](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					if (auto ptr = std::dynamic_pointer_cast<Scope2>(namespacePtr)) {
						if (auto objFound = ptr->GetFunctions(functionName)) {
							return true;
						}
					}
					return false;
				})) {
					return out;
				}
				else {
					return nullptr;
				}
			}
			else {
				std::string Namespace = functionName.substr(0, lastOfColons - 1);
				functionName = functionName.substr(lastOfColons + 1);
				if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(FindNamespace(Namespace))) {
					return namespacePtr->FindNamespaceWithFunction(functionName);
				}
				else {
					return nullptr;
				}
			}
		};
		std::shared_ptr< Functions2::FunctionSort > FindFunctions(std::string functionName) const {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				if (auto ptr = std::dynamic_pointer_cast<Scope2>(FindNamespaceWithFunction(functionName))) {
					return ptr->GetFunctions(functionName);
				}
				else {
					return nullptr;
				}
			}
			else {
				std::string Namespace = functionName.substr(0, lastOfColons - 1);
				functionName = functionName.substr(lastOfColons + 1);

				if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(FindNamespace(Namespace))) {
					return namespacePtr->FindFunctions(functionName);
				}
				else {
					return nullptr;
				}
			}
		};

		std::shared_ptr<Namespace2> FindNamespaceWithFunction(std::string functionName, Function_Params const& params, Type_Converter_Tree const& tree) {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				std::shared_ptr<Namespace2> out;
				if (TryFindNearestNamespaceWhere(out, [&functionName, &params, &tree](std::shared_ptr<Namespace2> const& namespacePtr)->bool {
					if (auto ptr = std::dynamic_pointer_cast<Scope2>(namespacePtr)) {
						if (auto func = ptr->GetFunction(functionName, params, tree)) {
							return true;
						}
					}
					return false;
				})) {
					return out;
				}
				else {
					return nullptr;
				}
			}
			else {
				std::string Namespace = functionName.substr(0, lastOfColons - 1);
				functionName = functionName.substr(lastOfColons + 1);
				if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(FindNamespace(Namespace))) {
					return namespacePtr->FindNamespaceWithFunction(functionName, params, tree);
				}
				else {
					return nullptr;
				}
			}
		};
		Functions2::FunctionActualPtr FindFunction(std::string functionName, Function_Params const& params, Type_Converter_Tree const& tree) {
			static auto fixNamespace{ [](std::string x) -> std::string {
				while (x.find("::") == 0 && x.length() > 2) {
					x = x.substr(2);
				}

				while (x.size() >= 2 && (x.rfind("::") == (x.length() - 2))) { x = x.substr(0, x.length() - 2); }

				return x;
			} };
			functionName = fixNamespace(functionName);

			auto lastOfColons = functionName.find_last_of("::");
			if (lastOfColons == std::string::npos) {
				if (auto ptr = std::dynamic_pointer_cast<Scope2>(FindNamespaceWithFunction(functionName, params, tree))) {
					return ptr->GetFunction(functionName, params, tree);
				}
				else {
					return {};
				}
			}
			else {
				std::string Namespace = functionName.substr(0, lastOfColons - 1);
				functionName = functionName.substr(lastOfColons + 1);

				if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(FindNamespace(Namespace))) {
					return namespacePtr->FindFunction(functionName, params, tree);
				}
				else {
					return {};
				}
			}
		};

		virtual std::shared_ptr<Type_Converter_Tree> GetTypeConverterTree() const {
			if (auto p = std::dynamic_pointer_cast<Scope2>(GetLibrary())) {
				return p->GetTypeConverterTree();
			}
			else {
				return nullptr;
			}
		};

		std::shared_ptr<Namespace2> FindNamespaceWithFunction(std::string functionName, Function_Params const& params) {
			return FindNamespaceWithFunction(functionName, params, *GetTypeConverterTree());
		};
		Functions2::FunctionActualPtr FindFunction(std::string functionName, Function_Params const& params) {
			auto tree = GetTypeConverterTree();
			return FindFunction(functionName, params, *tree);
		};

		std::vector<std::shared_ptr<Scope2>> GetScopesForObjectSearch() const {
			std::vector<std::shared_ptr<Scope2>> out;
			// will loop over all available scopes in the order we like
			(void)FindNearestScopeWhere([&](std::shared_ptr<Scope2> const& ptr) -> bool {
				out.push_back(ptr);
				return false;
			});
			return out;
		};

		bool TryFindFunctionImpl(std::string const& functionName, scripting::Function_Params const& params, std::shared_ptr<Type_Converter_Tree> const& m_conversionTree, Functions2::FunctionActualPtr& out) const {
			if (!m_conversionTree) return false;
			
			size_t lastOfColons{ 0 };
			if ((lastOfColons = functionName.find_last_of("::")) == std::string::npos) {
				std::shared_ptr<Scope2> firstParamScopePtr{ nullptr };
				std::shared_ptr<Scope2> constructorScopePtr{ nullptr };

				// FIRST SEARCH DOES ALLOW FOR CONVERSIONS, BUT NO TEMPLATES
				if (1) {
					std::multimap<double, Functions2::FunctionActualPtr> sort;

					// FIRST, WE CHECK TO SEE IF THE DESIRED FUNCTION IS AVAILABLE FROM THE CLASS OF THE FIRST PARAM (e.g. to_string(Units::foot()) would search the Units::foot class before anything else)				
					{
						auto firstParam = params.begin();
						if (firstParam != params.end()) {
							firstParamScopePtr = std::dynamic_pointer_cast<Scope2>(this->FindClass(firstParam->Type()));
						}
					}
					// While we normally try to minimize the conversion cost, 
					if (firstParamScopePtr) {
						if (auto ptr = firstParamScopePtr->GetFunctions()) {
							if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
								if (func->second.conversion_cost(params, *m_conversionTree) != std::numeric_limits<double>::max()) {
									// The function is available and requires (potentially) conversion of other parameters. 
									out = func;
									return true;
								}
							}
						}
					}
					

					for (auto& scope : GetScopesForObjectSearch()) {
						if (scope) {
							if (auto ptr = scope->GetFunctions()) {
								if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
									sort.emplace(func->second.conversion_cost(params, *m_conversionTree), func);
								}
							}
						}						
					}
					//{
					//	auto firstParam = params.begin();
					//	if (firstParam != params.end()) {
					//		firstParamScopePtr = std::dynamic_pointer_cast<Scope2>(this->FindClass(firstParam->Type()));
					//	}
					//}
					//if (firstParamScopePtr) {
					//	for (auto& scope : firstParamScopePtr->GetScopesForObjectSearch()) {
					//		if (scope) {
					//			if (auto ptr = scope->GetFunctions()) {
					//				if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
					//					sort.emplace(func->second.conversion_cost(params, *m_conversionTree), func);
					//				}
					//			}
					//		}
					//	}
					//}

					// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, BUT NO TEMPLATES)
					if (constructorScopePtr = std::dynamic_pointer_cast<Scope2>(this->FindClass(functionName))) {
						// Is there a pre-defined constructor that this could work with?
						if (auto functions = constructorScopePtr->GetFunctions()) {
							if (auto func = functions->BuildMatch(functionName, params, *m_conversionTree, false, true)) {
								sort.emplace(func->second.conversion_cost(params, *m_conversionTree), func);
							}
						}

						// Can the conversion tree do this itself, without any help?
						//if (params.size() == 1) {
						//	if (m_conversionTree->Converts(params[0].Type(), constructorScopePtr->GetClassType())) {
						//		auto func = make_callable([tree = std::weak_ptr<Type_Converter_Tree>(m_conversionTree), toType = constructorScopePtr->GetClassType()](Any const& from)->Any {
						//			if (auto Tree = tree.lock()) {
						//				if (Tree->Converts(from.Type(), toType)) {
						//					try {
						//						return Tree->Convert(from, toType);
						//					}
						//					catch (exception::bad_boxed_cast const& e) {
						//						std::cout << "Conversion is no longer available" << std::endl;
						//						throw exception::not_found_error("Conversion is no longer available");
						//					}
						//				}
						//			}
						//			std::cout << "Conversion is no longer available" << std::endl;
						//			throw exception::not_found_error("Conversion is no longer available");									
						//		});
						//		func->SetReturnType(constructorScopePtr->GetClassType());
						//		sort.emplace(func->conversion_cost(params, *m_conversionTree), std::make_shared<Functions2::FunctionActual>(func, scripting::Param_Types(
						//			{ {"From", params[0].Type() } }
						//		)));
						//	}
						//}
					}
					for (auto& s : sort) { if (s.first != std::numeric_limits<double>::max()) { out = s.second; return true; } }
				}

				// SECOND SEARCH ALLOWS FOR TEMPLATE FUNCTIONS
				if (1) {
					std::multimap<double, Functions2::FunctionActualPtr> sort;
					for (auto& scope : GetScopesForObjectSearch()) {
						if (scope) {
							if (auto ptr = scope->GetFunctions()) {
								if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
									sort.emplace(func->second.conversion_cost(params, *m_conversionTree), func);
								}
							}
						}
					}
					if (firstParamScopePtr) {
						for (auto& scope : firstParamScopePtr->GetScopesForObjectSearch()) {
							if (scope) {
								if (auto ptr = scope->GetFunctions()) {
									if (auto func = ptr->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
										sort.emplace(func->second.conversion_cost(params, *m_conversionTree), func);
									}
								}
							}
						}
					}
					// PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS (ALLOW FOR CONVERSIONS, AND TEMPLATES)
					if (constructorScopePtr = std::dynamic_pointer_cast<Scope2>(this->FindClass(functionName))) {
						// Is there a pre-defined constructor that this could work with?
						if (auto functions = constructorScopePtr->GetFunctions()) {
							if (auto func = functions->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
								sort.emplace(func->second.conversion_cost(params, *m_conversionTree), func);
							}
						}

						// Can the conversion tree do this itself, without any help?
						//if (params.size() == 1 && firstParamScopePtr) {
						//	if (m_conversionTree->Converts(firstParamScopePtr->GetClassType(), constructorScopePtr->GetClassType())) {
						//		auto func = make_callable([tree = std::weak_ptr<Type_Converter_Tree>(m_conversionTree), toType = constructorScopePtr->GetClassType()](Any const& from)->Any {
						//			if (auto Tree = tree.lock()) {
						//				if (Tree->Converts(from.Type(), toType)) {
						//					try {
						//						return Tree->Convert(from, toType);
						//					}
						//					catch (exception::bad_boxed_cast const& e) {
						//						std::cout << Units::printf("This conversion is no longer available { %i }\n", __LINE__);
						//						throw exception::not_found_error(std::string("Conversion is no longer available: ") + e.what());
						//					}
						//				}
						//			}
						//			std::cout << Units::printf("This conversion is no longer available { %i }\n", __LINE__);
						//			throw exception::not_found_error("Conversion is no longer available");
						//		});
						//		func->SetReturnType(constructorScopePtr->GetClassType());
						//		sort.emplace(func->conversion_cost(params, *m_conversionTree), std::make_shared<Functions2::FunctionActual>(func, scripting::Param_Types(
						//			{ {"From", firstParamScopePtr->GetClassType() } }
						//		)));
						//	}
						//}
					}
					for (auto& s : sort) { if (s.first != std::numeric_limits<double>::max()) { out = s.second; return true; } }
				}

				// IF ALL SEARCHES FAILED, PERHAPS THE USER MEANT TO CALL THE CONSTRUCTOR FOR A CLASS
				if (1) {
					if (constructorScopePtr) {
						if (auto functions = constructorScopePtr->GetFunctions()) {
							if (auto func = functions->BuildMatch(functionName, params, *m_conversionTree, true, true)) {
								if (func->second.conversion_cost(params, *m_conversionTree) != std::numeric_limits<double>::max()) {
									out = func;
									return true;
								}
							}
						}
					}
				}

				return false;
			}
			else {
				std::string functionNameActual{ functionName.substr(lastOfColons + 1) };
				std::string scopeName{ functionName.substr(0, lastOfColons - 1) };

				if (auto namespacePtr = std::dynamic_pointer_cast<Scope2>(FindNamespace(functionName))) {
					return namespacePtr->TryFindFunctionImpl(functionNameActual, params, m_conversionTree, out);
				}

				return false;
			}
		};

		Functions2::FunctionActualPtr BuildFunction(std::string const& functionName, scripting::Function_Params const& params, std::shared_ptr<Type_Converter_Tree> const& m_conversionTree) const {
			Functions2::FunctionActualPtr out{ nullptr };
			if (TryFindFunctionImpl(functionName, params, m_conversionTree, out)) {
				return out;
			}
			else {
				return nullptr;
			}
		};
		Functions2::FunctionActualPtr BuildFunction(std::string const& functionName, scripting::Function_Params const& params) const {
			return BuildFunction(functionName, params, GetTypeConverterTree());
		};
		Any CallFunction(std::string const& functionName, scripting::Function_Params const& params) const {
			auto tree = GetTypeConverterTree(); // builds and caches the tree. Updates the tree only if the situation has changed (new functions, new classes, or new Using statements)
			if (tree) {
				if (auto func = BuildFunction(functionName, params, tree)) {
					auto converted = func->second.convert(params, *tree);
					return scripting::call(func->first, converted, *tree);
				}
				else {
					throw exception::not_found_error(functionName);
				}
			}
			throw std::runtime_error("Scope was invalid");
		};
		Any CallFunction(std::string const& functionName, std::vector<Any> const& params) const {
			scripting::Function_Params Params{ const_cast<std::vector<Any>&>(params) };
			Any toReturn;
			toReturn = CallFunction(functionName, Params);
			return toReturn;
		};

		template <typename T>
		T Cast(Any const& from) const {
			auto ToType = user_type<T>();
			auto FromType = from.Type();

			// see if we can convert (fastest option)
			auto Tree = this->GetTypeConverterTree();
			if (Tree && Tree->Converts(FromType, ToType)) {
				return Tree->Convert(from, ToType).cast<T>();
			}

			auto ToClass = std::dynamic_pointer_cast<Scope2>(this->FindClass(user_type<T>()));
			if (ToClass) {
				// see if he can convert (fastest option)
				auto Tree2 = ToClass->GetTypeConverterTree();
				if (Tree2 && Tree2->Converts(FromType, ToType)) {
					return Tree2->Convert(from, ToType).cast<T>();
				}

				// search for a function that can do it
				if (1) {
					std::vector<Any> params = { from };

					// call a functor from our scope
					try {
						return this->CallFunction(ToClass->GetName(), params).cast<T>();
					}
					catch (exception::not_found_error) {}

					// call a functor from their scope
					try {
						return ToClass->CallFunction(ToClass->GetName(), params).cast<T>();
					}
					catch (exception::not_found_error) {}
				}

				// Failure
				throw exception::not_found_error(ToClass->GetName());
			}

			// Failure
			throw exception::not_found_error(user_type<T>().lock()->name());			
		};

		
	};

	class Namespace2 : public Scope2 {
	public:
		friend class Class2;
		friend class Global2;

		Namespace2(std::shared_ptr<Scope2> const& parent, std::string const& Name)
			: Scope2(parent)
			, p_Name(Name)
		{
			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};
		Namespace2(std::shared_ptr<Namespace2> const& parent, std::string const& Name) : Namespace2(std::dynamic_pointer_cast<Scope2>(parent), Name) {};
		Namespace2(std::shared_ptr<Class2> const& parent, std::string const& Name) : Namespace2(std::dynamic_pointer_cast<Scope2>(parent), Name) {};
		Namespace2(std::shared_ptr<Global2> const& parent, std::string const& Name) : Namespace2(std::dynamic_pointer_cast<Scope2>(parent), Name) {};
		
		virtual ~Namespace2() {};
		void SetSelf(std::shared_ptr<Namespace2>& p) { this->p_self = std::dynamic_pointer_cast<Scope2>(p); };
		virtual bool IsClass() const override { return false; };
		virtual bool IsNamespace() const override { return true; };
		virtual std::string GetName() const override { return p_Name; };

	private:
		std::string  // e.g. "", or "_NAMESPACE_NAME_", or "_CLASS_NAME_"
			p_Name;
	public:

	private:
		fibers::containers::Map<std::string, std::weak_ptr<Class2>> // allowed postfixes (e.g. 10_ft, where "_ft" is the key) to their desired typename. Duplicate are not allowed.
			p_postfixes; 

	public:


	private:
		std::shared_ptr<Functions2> // functions. (e.g. `==` or `to_string`). Duplicate names are expected. 
			p_functions{ std::make_shared<Functions2>() };

	private:
		virtual void RemoveStaleReferences() override {
			// p_using
			while (true) {
				size_t toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_using) {
					if (ref) {
						if (ref->second.expired()) {
							toRemove = ref->first;
							doRemoval = true;
							break;
						}
					}
				}
				if (doRemoval) {
					p_using.erase(toRemove);
				}
				else {
					break;
				}
			}

			// p_postfixes
			while (true) {
				std::string toRemove{};
				bool doRemoval = false;
				for (auto& ref : p_postfixes) {
					if (ref) {
						if (ref->second.expired()) {
							toRemove = ref->first;
							doRemoval = true;
							break;
						}
					}
				}
				if (doRemoval) {
					p_postfixes.erase(toRemove);
				}
				else {
					break;
				}
			}

			// p_functions
			

		};
		virtual bool RecordFunction(std::string const& Name, Proxy_Function ptr, bool overrideIfExists = true) {
			if (auto p = std::dynamic_pointer_cast<Namespace2>(GetLibrary())) {
				return p->RecordFunction(Name, ptr, overrideIfExists);
			}
			return false;
		};
	public:
		virtual bool AddFunction(std::string const& name, Proxy_Function function, bool overrideIfAlreadyExists = false) override {
			defer(this->RecordFunction(name, function, overrideIfAlreadyExists));
			return p_functions->emplace(name, function, overrideIfAlreadyExists);
		};
		virtual bool AddFunction(std::string const& name, Proxy_Function function, Param_Types const& params, bool overrideIfAlreadyExists = false) override {
			defer(this->RecordFunction(name, function, overrideIfAlreadyExists));
			return p_functions->emplace(name, function, params, overrideIfAlreadyExists);
		};

	private:
		virtual bool AddFreeFunction(std::string const& name, Proxy_Function function, bool overrideIfAlreadyExists = true) override {
			defer(this->RecordFunction(name, function, overrideIfAlreadyExists));
			return p_functions->emplace_free(name, function, overrideIfAlreadyExists);
		};
		virtual bool AddFreeFunction(std::string const& name, Proxy_Function function, Param_Types const& params, bool overrideIfAlreadyExists = true) override {
			defer(this->RecordFunction(name, function, overrideIfAlreadyExists));
			return p_functions->emplace_free(name, function, params, overrideIfAlreadyExists);
		};

	public:

		virtual std::shared_ptr< Functions2 > GetFunctions() const override {
			return p_functions;
		};
		virtual std::shared_ptr< Functions2::FunctionSort > GetFunctions(std::string const& name) const override {
			return p_functions->operator()(name);
		};
		virtual Functions2::FunctionActualPtr GetFunction(std::string const& name, Function_Params const& params, Type_Converter_Tree const& tree) override {
			return p_functions->BuildMatch(name, params, tree);
		};
		virtual Functions2::FunctionActualPtr GetFunction(std::string const& name, Function_Params const& params) override {
			auto tree = GetTypeConverterTree();
			return GetFunction(name, params, *tree);
		};

	};

	class Class2 final : public Namespace2 {
	public:
		friend class Global2;

		Class2(
			std::shared_ptr<Scope2> const& parent
			, std::string const& Name
			, Type_Info type = user_type<void>()
			, std::weak_ptr<Class2> inheritance = std::weak_ptr<Class2>() // e.g. this class derives from another Class
		)
			: Namespace2(parent, Name)
			, DerivedFrom(inheritance)
		{
			if (auto ptr = std::dynamic_pointer_cast<Namespace2>(DerivedFrom.lock())) {
				this->AddUsing(ptr);
			}

			if (type == user_type<void>()) {
				ClassType = std::make_shared<fibers::Type_Info>(this->p_UniqueName, Name);
			}
			else {
				ClassType = type.lock();
			}

			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};

		Class2(std::shared_ptr<Namespace2> const& parent, std::string const& Name, Type_Info type = user_type<void>(), std::weak_ptr<Class2> inheritance = std::weak_ptr<Class2>()) 
			: Class2(std::dynamic_pointer_cast<Scope2>(parent), Name, type, inheritance) {};
		Class2(std::shared_ptr<Class2> const& parent, std::string const& Name, Type_Info type = user_type<void>(), std::weak_ptr<Class2> inheritance = std::weak_ptr<Class2>()) 
			: Class2(std::dynamic_pointer_cast<Scope2>(parent), Name, type, inheritance) {};
		Class2(std::shared_ptr<Global2> const& parent, std::string const& Name, Type_Info type = user_type<void>(), std::weak_ptr<Class2> inheritance = std::weak_ptr<Class2>()) 
			: Class2(std::dynamic_pointer_cast<Scope2>(parent), Name, type, inheritance) {};

		virtual ~Class2() {};
		void SetSelf(std::shared_ptr<Class2>& p) { this->p_self = std::dynamic_pointer_cast<Scope2>(p); };
		virtual bool IsClass() const override { return true; };

	private:
		std::weak_ptr<Class2> 
			DerivedFrom; // e.g. this class derives from another Class
		std::shared_ptr<fibers::Type_Info> 
			ClassType;

	public:
		virtual Type_Info GetClassType() const override { return ClassType; };

	private:
		virtual bool TryFindNearestScopeWhere(
			std::shared_ptr<Scope2>& bestMatch,
			std::function<bool(std::shared_ptr<Scope2> const&)> const& func,
			std::unordered_set< std::shared_ptr<Scope2> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope2> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope2> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope2> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr) >= 1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
				checkedSelf.emplace(selfPtr);
				if (1) {
					if (auto p = std::dynamic_pointer_cast<Scope2>(selfPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}

			// test my "using" namespaces and their children.
			for (auto& childNamespace : this->p_using) {
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope2>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test my inherited namespace.
			if (auto p = std::dynamic_pointer_cast<Scope2>(DerivedFrom.lock())) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}


			// test all of my parents
			auto parentPtr = this->p_parent.lock();
			while (parentPtr) {
				if (!checkedSelf.count(parentPtr) >= 1) {
					checkedSelf.emplace(parentPtr);
					if (1) {
						if (auto p = std::dynamic_pointer_cast<Scope2>(parentPtr)) {
							if (func(p)) {
								bestMatch = p;
								return true;
							}
						}
					}
				}
				else {
					break; // we've checked this before! Quick, get out. 
				}
				parentPtr = parentPtr->p_parent.lock();
			}

			// Test my children themselves.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope2>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
								checkedSelf.emplace(ptr);
								if (1) {
									if (auto p = std::dynamic_pointer_cast<Scope2>(ptr)) {
										if (func(p)) {
											bestMatch = p;
											return true;
										}
									}
								}
							}
							else {
								break; // we've checked this before! Quick, get out. 
							}
						}
					}
				}
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope2>(innerChildNamespace->second)) {
								if (p->TryFindNearestScopeWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
									return true;
								}
							}
						}
					}
				}
			}

			return false;
		};

		virtual bool TryFindNearestNamespaceWhere(
			std::shared_ptr<Namespace2>& bestMatch, 
			std::function<bool(std::shared_ptr<Namespace2> const&)> const& func, 
			std::unordered_set< std::shared_ptr<Scope2> > const& CheckedSelf = {},
			std::unordered_set< std::shared_ptr<Scope2> > const& CheckedAll = {}
		) const {
			auto& checkedSelf = const_cast<std::unordered_set< std::shared_ptr<Scope2> >&>(CheckedSelf);
			auto& checkedAll = const_cast<std::unordered_set< std::shared_ptr<Scope2> >&>(CheckedAll);
			auto selfPtr = this->p_self.lock();

			// Prevent Duplication
			if (checkedAll.count(selfPtr)>=1) { return false; }
			checkedAll.emplace(selfPtr);

			// test myself			
			if (!checkedSelf.count(selfPtr) >= 1) {
				checkedSelf.emplace(selfPtr);
				if (this->IsNamespace()) {
					if (auto p = std::dynamic_pointer_cast<Namespace2>(selfPtr)) {
						if (func(p)) {
							bestMatch = p;
							return true;
						}
					}
				}
			}

			// test my "using" namespaces and their children.
			for (auto& childNamespace : this->p_using) {
				if (childNamespace) {
					if (auto p = std::dynamic_pointer_cast<Scope2>(childNamespace->second.lock())) {
						if (p && p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
							return true;
						}
					}
				}
			}

			// test my inherited namespace.
			if (auto p = std::dynamic_pointer_cast<Scope2>(DerivedFrom.lock())) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test all of my parents 
			if (1) {
				auto parentPtr = this->p_parent.lock();
				while (parentPtr) {
					if (!checkedSelf.count(parentPtr) >= 1) {
						checkedSelf.emplace(parentPtr);
						if (parentPtr->IsNamespace()) {
							if (auto p = std::dynamic_pointer_cast<Namespace2>(parentPtr)) {
								if (func(p)) {
									bestMatch = p;
									return true;
								}
							}
						}
					}
					else {
						break; // we've checked this before! Quick, get out. 
					}
					parentPtr = parentPtr->p_parent.lock();
				}
			}

			// test my children themselves
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace->second) {
							auto ptr = std::dynamic_pointer_cast<Scope2>(innerChildNamespace->second);
							if (!checkedSelf.count(ptr) >= 1) {
								checkedSelf.emplace(ptr);
								if (ptr->IsNamespace()) {
									if (auto p = std::dynamic_pointer_cast<Namespace2>(ptr)) {
										if (func(p)) {
											bestMatch = p;
											return true;
										}
									}
								}
							}
							else {
								break; // we've checked this before! Quick, get out. 
							}
						}
					}
				}
			}

			// test my parents and their children
			if (auto p = this->p_parent.lock()) {
				if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
					return true;
				}
			}

			// test my children's children.
			for (auto& childNamespace : this->p_children) {
				if (childNamespace && childNamespace->second) {
					for (auto& innerChildNamespace : *childNamespace->second) {
						if (innerChildNamespace) {
							if (auto p = std::dynamic_pointer_cast<Scope2>(innerChildNamespace->second)) {
								if (p->TryFindNearestNamespaceWhere(bestMatch, func, CheckedSelf, CheckedAll)) {
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

	class Global2 final : public Namespace2 {
	public:
		Global2()
			: Namespace2(std::shared_ptr<Scope2>(), "")
		{
			qualifiedNamespaceWithQualifiers = this->GetQualifiedNamespaceImpl(true);
			qualifiedNamespaceWithoutQualifiers = this->GetQualifiedNamespaceImpl();
		};
		virtual ~Global2() {};
		void SetSelf(std::shared_ptr<Global2>& p) { this->p_self = std::dynamic_pointer_cast<Scope2>(p); };

		std::vector<std::weak_ptr<Class2>> GetClasses() const {
			std::vector<std::weak_ptr<Class2>> out;
			out.reserve(Classes.size() + 16);
			static auto badHash{ Scope2::Hasher()(std::weak_ptr<Scope2>()) };

			bool DoCleanup = false;

			size_t hash{ 0 };
			for (auto& x : Classes) {
				if (x) {
					hash = Scope2::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						out.push_back(x->second);
					}
				}
			}

			if (DoCleanup) {
				//const_cast<Global2*>(this)->CleanupRequested.CompareExchange(0, 1);
				const_cast<Global2*>(this)->RemoveStaleReferences();
			}

			return out;
		};
		std::vector<std::weak_ptr<Namespace2>> GetUsing() const {
			std::vector<std::weak_ptr<Namespace2>> out;
			out.reserve(Classes.size() + 16);
			static auto badHash{ Scope2::Hasher()(std::weak_ptr<Scope2>()) };

			bool DoCleanup = false;

			size_t hash{ 0 };
			for (auto& x : Usings) {
				if (x) {
					hash = Scope2::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						out.push_back(x->second);
					}
				}
			}

			if (DoCleanup) {
				//const_cast<Global2*>(this)->CleanupRequested.CompareExchange(0, 1);
				const_cast<Global2*>(this)->RemoveStaleReferences();
			}

			return out;
		};
	
	public:
		void AddBuiltIns() {
			auto defineBuiltInType = [this](auto typeImpl, std::string const& Name) -> void {
				// make it a class
				std::shared_ptr<Class2> classPtr; {
					classPtr.reset(new Class2(this->p_self.lock(), Name, user_type<decltype(typeImpl)>()));
				}
				classPtr->SetSelf(classPtr);
				this->AddChild(classPtr);

				// add converters
				classPtr->AddFunction(Name, make_callable([](bool from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](char from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](int from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](long from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](float from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](double from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](size_t from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](fibers::containers::number < double > from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](signed char from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned char from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](char16_t from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](char32_t from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](wchar_t from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](short from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned short from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned int from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](unsigned long from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](long long from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				classPtr->AddFunction(Name, make_callable([](long double from) -> decltype(typeImpl) { return (decltype(typeImpl))from; }));
				
				// Constructors
				classPtr->AddFunction(Name, make_callable([]() ->  decltype(typeImpl) { return  decltype(typeImpl){}; }));
				classPtr->AddFunction(Name, make_callable([](decltype(typeImpl) const& makeCopy) ->  decltype(typeImpl) { return makeCopy; }));
				Type_Info thisType = user_type<decltype(typeImpl)>();
				std::vector<std::pair<std::string, Type_Info>> temp; {
					std::pair<std::string, Type_Info> tempPair{ std::string(), Type_Info() };
					tempPair.first = "o";
					tempPair.second = thisType;
					temp.push_back(tempPair);
					temp.push_back(tempPair);
				}
				classPtr->AddFunction("=", make_callable([](Any const& a, decltype(typeImpl) const& b) -> Any { decltype(typeImpl)& x = a.cast(); x = b; return a; }), Param_Types(temp));

				// Comparisons
				classPtr->AddFunction("==", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x == y; }));
				classPtr->AddFunction("!=", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x != y; }));
				classPtr->AddFunction("<", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x < y; }));
				classPtr->AddFunction("<=", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x <= y; }));
				classPtr->AddFunction(">", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x > y; }));
				classPtr->AddFunction(">=", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> bool { return x >= y; }));
				classPtr->AddFunction("+", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x + y; }));
				classPtr->AddFunction("-", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x - y; }));
				classPtr->AddFunction("*", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { return x * y; }));
				classPtr->AddFunction("/", scripting::make_callable([](decltype(typeImpl) const& x, decltype(typeImpl) const& y) -> decltype(typeImpl) { if (y == 0) return std::numeric_limits<decltype(typeImpl)>::max(); else return x / y; }));
				classPtr->AddFunction("+=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x += y; }));
				classPtr->AddFunction("-=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x -= y; }));
				classPtr->AddFunction("*=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { x *= y; }));
				classPtr->AddFunction("/=", scripting::make_callable([](decltype(typeImpl)& x, decltype(typeImpl) const& y) -> void { if (y == 0) x = std::numeric_limits<decltype(typeImpl)>::max(); else x /= y; }));

				// Functions
				classPtr->AddFunction("max", make_callable([]() { return std::numeric_limits<decltype(typeImpl)>::max(); }));
				classPtr->AddFunction("min", make_callable([]() { return std::numeric_limits<decltype(typeImpl)>::lowest(); }));
				classPtr->AddFunction("to_string", make_callable([](decltype(typeImpl) const& o) -> std::string { return std::to_string(o); }));

			};

			// Built-in types
			if (1) {
#define DefineBuiltInType(V) defineBuiltInType(##V(0), #V);
				DefineBuiltInType(bool);
				DefineBuiltInType(char);
				DefineBuiltInType(int);
				DefineBuiltInType(long);
				DefineBuiltInType(float);
				DefineBuiltInType(double);
				DefineBuiltInType(size_t);
				defineBuiltInType(fibers::containers::number<double>(), "Number");
				DefineBuiltInType(char16_t);
				DefineBuiltInType(char32_t);
				DefineBuiltInType(wchar_t);
				DefineBuiltInType(short);
				defineBuiltInType(unsigned char(0), "uchar");
				defineBuiltInType(unsigned short(0), "ushort");
				defineBuiltInType(unsigned int(0), "uint");
				defineBuiltInType(unsigned long(0), "ulong");
				defineBuiltInType(long long(0), "llong");
				defineBuiltInType(long double(), "ldouble");
#undef DefineBuiltInType

				// String
				if (1) {
					// make it a class
					std::shared_ptr<Class2> classPtr; {
						classPtr.reset(new Class2(this->p_self.lock(), "string", user_type<std::string>()));
					}
					classPtr->SetSelf(classPtr);
					this->AddChild(classPtr);

					// add converters
					classPtr->AddFunction("string", make_callable([](bool from) -> std::string { if (from) return "true"; else return "false"; }));
					classPtr->AddFunction("string", make_callable([](char from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](int from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](long from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](float from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](double from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](size_t from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](fibers::containers::number < double > from) -> std::string { return std::to_string(from.load()); }));
					classPtr->AddFunction("string", make_callable([](signed char from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned char from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](char16_t from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](char32_t from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](wchar_t from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](short from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned short from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned int from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](unsigned long from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](long long from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([](long double from) -> std::string { return std::to_string(from); }));
					classPtr->AddFunction("string", make_callable([self = classPtr->p_self](fibers::Type_Info from) -> std::string {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();						
						return from.name();
					}));

					// Constructors
					classPtr->AddFunction("string", make_callable([]() -> std::string { return std::string{}; }));
					classPtr->AddFunction("string", make_callable([](std::string const& makeCopy) -> std::string { return makeCopy; }));
					classPtr->AddFunction("=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out = b; return a; }), Param_Types({ {std::string("a"), user_type<std::string>() }, {std::string("b"), user_type<std::string>() } }));

					// Comparisons
					classPtr->AddFunction("==", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x == y; }));
					classPtr->AddFunction("!=", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x != y; }));
					classPtr->AddFunction("<", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x < y; }));
					classPtr->AddFunction("<=", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x <= y; }));
					classPtr->AddFunction(">", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x > y; }));
					classPtr->AddFunction(">=", scripting::make_callable([](std::string const& x, std::string const& y) -> bool { return x >= y; }));
					classPtr->AddFunction("+", scripting::make_callable([](std::string const& x, std::string const& y) -> std::string { return x + y; }));
    				classPtr->AddFunction("+=", make_callable([](Any const& a, std::string const& b) -> Any { std::string& out = a.cast(); out += b; return a; }), Param_Types({ {std::string("a"), user_type<std::string>() }, {std::string("b"), user_type<std::string>() } }));

					// Functions
					classPtr->AddFunction("length", make_callable([](std::string const& a) -> size_t { return a.length(); }));
					classPtr->AddFunction("size", make_callable([](std::string const& a) -> size_t { return a.size(); }));
					classPtr->AddFunction("[]", make_callable([](std::string const& a, size_t index) -> char { return a[index]; }));
					classPtr->AddFunction("front", make_callable([](std::string const& a) -> char { return a.front(); }));
					classPtr->AddFunction("back", make_callable([](std::string const& a) -> char { return a.back(); }));
					classPtr->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind) -> size_t { return a.find(toFind); }));
					classPtr->AddFunction("find", make_callable([](std::string const& a, std::string const& toFind, size_t startPos) -> size_t { return a.find(toFind, startPos); }));
					classPtr->AddFunction("substr", make_callable([](std::string const& x, size_t Off) -> std::string { return x.substr(Off); })/*, { "input", "Off" }*/);
					classPtr->AddFunction("substr", make_callable([](std::string const& x, size_t Off, size_t Count) -> std::string { return x.substr(Off, Count); })/*, { "input", "Off", "Count" }*/);
					classPtr->AddFunction("to_string", make_callable([](std::string const& o) -> std::string { return o; }));

					// Objects or Constants
					classPtr->AddObj("npos", std::make_shared<Any>(std::string::npos));
				}

				// Type
				if (1) {
					// make it a class
					std::shared_ptr<Class2> classPtr; {
						classPtr.reset(new Class2(this->p_self.lock(), "Type_Info", user_type<fibers::Type_Info>()));
					}
					classPtr->p_self = classPtr;
					this->AddChild(classPtr);

					// Constructors
					classPtr->AddFunction("Type_Info", make_callable([]() -> fibers::Type_Info { return fibers::Type_Info{}; }));
					classPtr->AddFunction("Type_Info", make_callable([](fibers::Type_Info const& makeCopy) -> fibers::Type_Info { return makeCopy; }));
					classPtr->AddFunction("=", make_callable([](Any const& a, fibers::Type_Info const& b) -> Any { fibers::Type_Info& out = a.cast(); out = b; return a; }), Param_Types({ {std::string("a"), user_type<fibers::Type_Info>() }, {std::string("b"), user_type<fibers::Type_Info>() } }));

					// Comparisons
					classPtr->AddFunction("==", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x == y; }));
					classPtr->AddFunction("!=", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x != y; }));
					classPtr->AddFunction("<", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x < y; }));
					classPtr->AddFunction("<=", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x <= y; }));
					classPtr->AddFunction(">", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x > y; }));
					classPtr->AddFunction(">=", scripting::make_callable([](fibers::Type_Info const& x, fibers::Type_Info const& y) -> bool { return x >= y; }));

					// Functions
					classPtr->AddFunction("to_string", make_callable([self = classPtr->p_self](fibers::Type_Info const& from) -> std::string {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
						return from.name();
					}));
					classPtr->AddFunction("name", make_callable([self = classPtr->p_self](fibers::Type_Info const& from)->std::string {
						if (auto p = self.lock()) if (auto p2 = p->FindClass(from)) return p2->GetName();
						return from.name();
					}));
					classPtr->AddFunction("cpp_name", make_callable([self = classPtr->p_self](fibers::Type_Info const& from)->std::string {
						return from.name();
					}));
					classPtr->AddFunction("is_undef", make_callable([self = classPtr->p_self](fibers::Type_Info const& from)-> bool {
						return from.is_undef();
					}));
					classPtr->AddFunction("is_void", make_callable([self = classPtr->p_self](fibers::Type_Info const& from)-> bool {
						return from.is_void();
					}));
				}
			}

			// Built-In functions
			if (1) {
				// Returns the type of Any object. By not specifying the type, the Any is treated like a Template
				this->AddFunction("Type", make_callable([](Any const& obj) -> fibers::Type_Info {
					if (auto p = obj.Type().lock())
						return *p;
					else
						return fibers::user_type<void>();
				}));

				// Returns a stringified version of the provided Any obj. This is meant to be a fall-back template whenever no specialization is available. 
				this->AddFunction("to_string", make_callable([](Any const& x) -> std::string {
					return Units::printf("`%s`", x.TypeName());
				}));
			}
        };

	private:
		void GetClasses(std::unordered_map<size_t, std::weak_ptr<Class2>>& out) const {
			bool DoCleanup = false;
			static auto badHash{ Scope2::Hasher()(std::weak_ptr<Scope2>()) };

			size_t hash{ 0 };
			for (auto& x : Classes) {
				if (x) {
					hash = Scope2::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						out.insert({ hash, x->second });
					}
				}
			}

			if (DoCleanup) {
				const_cast<Global2*>(this)->RemoveStaleReferences();
			}
		};
		void GetAllAvailableClassesImpl(
			std::unordered_map<size_t, std::weak_ptr<Class2>>& out,
			std::unordered_map<size_t, std::weak_ptr<Scope2>>& uniqueLibraries
		) const {
			auto hashed{ Scope2::Hasher()(this->p_self) };
			if (uniqueLibraries.count(hashed) > 0) return;
			uniqueLibraries.insert({ hashed, this->p_self });

			this->GetClasses(out);
			static auto badHash{ Scope2::Hasher()(std::weak_ptr<Scope2>()) };
			size_t hash{ 0 };
			bool DoCleanup = false;
			for (auto& x : Usings) {
				if (x) {
					hash = Scope2::Hasher()(x->second);
					if (hash == badHash) {
						DoCleanup = true;
					}
					else {
						if (auto ptr = x->second.lock()) {
							if (auto p2 = ptr->GetLibrary()) {
								p2->GetAllAvailableClassesImpl(out, uniqueLibraries);
							}
						}
					}
				}
			}
			if (DoCleanup) {
				//const_cast<Global2*>(this)->CleanupRequested.CompareExchange(0, 1);
				const_cast<Global2*>(this)->RemoveStaleReferences();
			}
		};
	
	private:
		// Searches for all classes that are defined in the current and "used" libraries. 
		std::map<Type_Info, std::weak_ptr<Class2>> GetAllAvailableClassesImpl() const {
			thread_local static std::unordered_map<size_t, std::weak_ptr<Class2>> allClasses{};
			thread_local static std::unordered_map<size_t, std::weak_ptr<Scope2>> uniqueList{};
			defer(allClasses.clear());
			defer(uniqueList.clear());

			GetAllAvailableClassesImpl(allClasses, uniqueList);
			{
				std::map<Type_Info, std::weak_ptr<Class2>> out;
				for (auto& x : allClasses) {
					if (auto p = std::dynamic_pointer_cast<Scope2>(x.second.lock())) {
						out[p->GetClassType()] = x.second;
					}					
				}
				return out;
			}
		};
	public:
		std::shared_ptr<std::map<Type_Info, std::weak_ptr<Class2>>> GetAllAvailableClasses() const {
			auto oldVersion = CachedClassListVersion.load();
			if (oldVersion != RecordVersion) {
				auto guard{ std::unique_lock(const_cast<Global2*>(this)->CachedClassListMutex) };
				if (const_cast<Global2*>(this)->CachedClassListVersion.CompareExchange(oldVersion, RecordVersion)) {
					return const_cast<Global2*>(this)->CachedClassList = std::make_shared<std::map<Type_Info, std::weak_ptr<Class2>>>(GetAllAvailableClassesImpl());
				}
			}
			
			if (1) {
				auto guard{ std::shared_lock(const_cast<Global2*>(this)->CachedClassListMutex) };
				return CachedClassList;
			}
		};
	private:
		// Creates a tree of type-converter functions using the classes found with GetAllAvailableClasses()
		void CreateTypeConverterTree(std::shared_ptr<Type_Converter_Tree>& out) const {
			if (auto classes = GetAllAvailableClasses()) {
				for (auto& FoundClass : *classes) {
					auto& outputType = FoundClass.first;
					if (auto p = FoundClass.second.lock()) {
						// Type Conversions are identical to Constructors with one input type. Therefore ...
						auto className = p->GetName();						
						// ... find all constructors ...
						if (auto constructors = p->GetFunctions(className)) {
							for (auto& constructor : *constructors) {
								if (constructor && constructor->second) {
									// ... whose inputs are size of 1 ...
									if (constructor->first.size() == 1) {
										// FIX ME! FIX ME:
										auto& inputType = constructor->first[0].second;
										out->AddConverter([func = constructor->second, this](Any const& input)->Any {
											std::vector<Any> params = { input }; 
											//if (auto Tree = this->CachedTypeConverterTree)
											//	return scripting::call(func->first, params, *Tree);
											//else 
												return func->first->operator()(params);											
										}, inputType, outputType);
									}
								}
							}
						}
					}
				}
			}
		};

	public:
		virtual std::shared_ptr<Type_Converter_Tree> GetTypeConverterTree() const override {
			auto oldVersion = CachedTypeConverterTreeVersion.load();
			if (oldVersion != RecordVersion) {
				auto guard{ std::unique_lock(const_cast<Global2*>(this)->CachedTypeConverterTreeMutex) };
				if (const_cast<Global2*>(this)->CachedTypeConverterTreeVersion.CompareExchange(oldVersion, RecordVersion)) {
					auto tree = std::make_shared<Type_Converter_Tree>();
					CreateTypeConverterTree(tree);
					return const_cast<Global2*>(this)->CachedTypeConverterTree = tree;
				}
			}

			if (1) {
				auto guard{ std::shared_lock(const_cast<Global2*>(this)->CachedTypeConverterTreeMutex) };
				return CachedTypeConverterTree;
			}
		};

	private:
		fibers::containers::Map<size_t, std::weak_ptr<Class2>> // collection of all classes that are added as "children" of this library
			Classes; 
		fibers::containers::Map<size_t, std::weak_ptr<Namespace2>> // collection of all namespaces that are added being "used" by this library
			Usings;
		fibers::containers::Map<size_t, std::pair<std::string, std::weak_ptr<details::Proxy_Function_Base>>> // collection of all namespaces that are added being "used" by this library
			Functions;

		std::shared_ptr<Type_Converter_Tree>
			CachedTypeConverterTree{ std::make_shared<Type_Converter_Tree>() };
		fibers::containers::number<unsigned __int64>
			CachedTypeConverterTreeVersion{ 0 };
		fibers::synchronization::shared_mutex<fibers::synchronization::mutex>
			CachedTypeConverterTreeMutex{};

		std::shared_ptr<std::map<Type_Info, std::weak_ptr<Class2>>>
			CachedClassList{ std::make_shared<std::map<Type_Info, std::weak_ptr<Class2>>>() };
		fibers::containers::number<unsigned __int64>
			CachedClassListVersion{ 0 };
		fibers::synchronization::shared_mutex<fibers::synchronization::mutex>
			CachedClassListMutex{};

		//fibers::containers::number<unsigned int> 
			//CleanupRequested{ 0 };
		fibers::containers::number<unsigned __int64>
			CleanupVersion{ 0 };
		fibers::containers::number<unsigned __int64>
			RecordVersion{ 0 };

		virtual void RemoveStaleReferences() override {
			auto oldVersion = CleanupVersion.load();
			if (oldVersion != RecordVersion) {
				if (CleanupVersion.CompareExchange(oldVersion, RecordVersion)) {
					// p_using
					while (true) {
						size_t toRemove{};
						bool doRemoval = false;
						for (auto& ref : p_using) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove = ref->first;
									doRemoval = true;
									break;
								}
							}
						}
						if (doRemoval) {
							p_using.erase(toRemove);
						}
						else {
							break;
						}
					}

					// p_postfixes
					while (true) {
						std::string toRemove{};
						bool doRemoval = false;
						for (auto& ref : p_postfixes) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove = ref->first;
									doRemoval = true;
									break;
								}
							}
						}
						if (doRemoval) {
							p_postfixes.erase(toRemove);
						}
						else {
							break;
						}
					}

					// Classes
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Classes) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove.push_back(ref->first);
								}
							}
						}
						Classes.erase(toRemove);
						toRemove.clear();
					}

					// Usings
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Usings) {
							if (ref) {
								if (ref->second.expired()) {
									toRemove.push_back(ref->first);
								}
							}
						}
						Usings.erase(toRemove);
						toRemove.clear();
					}

					// Functions
					if (1) {
						thread_local static std::vector<size_t> toRemove{};
						for (auto& ref : Functions) {
							if (ref) {
								if (ref->second.second.expired()) {
									toRemove.push_back(ref->first);
								}
							}
						}
						Functions.erase(toRemove);
						toRemove.clear();
					}
				}
			}
		};

		virtual bool RecordClass(std::shared_ptr<Class2> ptr, bool overrideIfExists = true) override {
			//if (CleanupRequested.CompareExchange(1, 0)) RemoveStaleReferences();			
			if (Classes.emplace(Scope2::Hasher()(ptr), ptr, overrideIfExists)){
				RecordVersion++;
				return true;
			}
			return false;
		};

		virtual bool RecordUsing(std::shared_ptr<Namespace2> ptr, bool overrideIfExists = true) override {
			//if (CleanupRequested.CompareExchange(1, 0)) RemoveStaleReferences();			
			if (Usings.emplace(Scope2::Hasher()(ptr), ptr, overrideIfExists)) {
				RecordVersion++;
				return true;
			}
			return false;
		};

		virtual bool RecordFunction(std::string const& Name, Proxy_Function ptr, bool overrideIfExists = true) override {
			//if (CleanupRequested.CompareExchange(1, 0)) RemoveStaleReferences();			
			if (Functions.emplace(std::hash<Proxy_Function>()(ptr), { Name, ptr }, overrideIfExists)) {
				RecordVersion++;
				return true;
			}
			return false;
		};


	};

#if 0
	class Impl {
	private:
		// Attempts to find or make a namespace with the requested namespace name. 
		// If qualified (e.g. starts with "::") then it attempts to find it from the global root -> down. 
		// If not qualified, then it attempts to find it from the current node -> up.
		// If the scope is not found, it will create the namespace as is most appropriate depending on context.
		static std::weak_ptr<Scope> FindOrMakeNamespaceImpl(std::shared_ptr<Scope> scope, std::string const& Namespace) {
			if (!scope) return std::shared_ptr<Scope>{ nullptr };

			std::shared_ptr<Scope> best_match{ nullptr };
			std::vector<std::string> namespaces;
			if (scope->TryFindScope(best_match, Namespace, &namespaces)) {
				// we were successful!
				return best_match;
			}
			else {
				std::reverse(namespaces.begin(), namespaces.end());
				for (auto& Namespaces : namespaces) {
					auto childScope = std::make_shared<scripting::Namespace>(best_match, Namespaces);
					childScope->p_self = childScope;
					best_match->AddChild(childScope);
					return FindOrMakeNamespaceImpl(scope, Namespace);
				}

				auto childScope = std::make_shared<scripting::Namespace>(best_match, Namespace);
				childScope->p_self = childScope;
				best_match->AddChild(childScope);
				return FindOrMakeNamespaceImpl(scope, Namespace);
			}
		};
	
    public:
		// Attempts to find or make a namespace with the requested namespace name. 
		// If qualified (e.g. starts with "::") then it attempts to find it from the global root -> down. 
		// If not qualified, then it attempts to find it from the current node -> up.
		// If the scope is not found, it will create the namespace as is most appropriate depending on context.
		static std::weak_ptr<Scope> FindOrMakeNamespace(std::weak_ptr<Scope> scope, std::string const& Namespace) {
			auto locked_scope = scope.lock();
			auto p = FindOrMakeNamespaceImpl(locked_scope, Namespace).lock();
			std::string fullyQualifiedName;
			while (p && fullyQualifiedName != p->GetQualifiedNamespace()) {
				fullyQualifiedName = p->GetQualifiedNamespace();
				p = FindOrMakeNamespaceImpl(locked_scope, Namespace).lock();
			}
			return p;
		};

		template<typename To> 
		static auto Cast(Any const& From, std::weak_ptr<Scope> scope) {
			using TT = typename Any::DataCaster::is_SharedPtr_class<To>::type;
			using TTT = typename fibers::get_type<To>::type;
			constexpr bool is_shared_ptr = TT::value;
			constexpr bool is_ptr = std::is_pointer_v<To>;
			constexpr bool is_ref = std::is_reference_v<To>;
			if constexpr (is_shared_ptr) {
				if (auto ScopePtr = scope.lock()) {
					if (auto foundScope = ScopePtr->FindClass(user_type<TTT>())) {
						auto tree1 = ScopePtr->GetCombinedTypeConversionTree();
						auto tree2 = foundScope->GetCombinedTypeConversionTree();
						auto tree = Type_Converter_Tree::Combine({ &tree2, &tree1 });
						try {
							if (Any f = ScopePtr->CallFunction(foundScope->GetName(), { From }, tree)) {
								return f.cast<std::shared_ptr<TTT>>();
							}
						} catch (exception::not_found_error const& e) {}

						try {
							if (Any f = foundScope->CallFunction(foundScope->GetName(), { From }, tree)) {
								return f.cast<std::shared_ptr<TTT>>();
							}
						} catch (exception::not_found_error const& e) {}

						try {
							if (tree.Converts(From, user_type<TTT>())) {
								if (Any f = tree.Convert(From, user_type<TTT>())) {
									return f.cast<std::shared_ptr<TTT>>();
								}								
							}	
						} catch (exception::not_found_error const& e) {}

						try {
							if (tree1.Converts(From, user_type<TTT>())) {
								if (Any f = tree1.Convert(From, user_type<TTT>())) {
									return f.cast<std::shared_ptr<TTT>>();
								}								
							}	
						} catch (exception::not_found_error const& e) {}
					}
					else {
						auto Type = fibers::user_type<typename std::remove_reference<typename std::remove_pointer<To>::type>::type>();
						throw exception::not_found_error(Type.name());
					}
				}
				else {
					throw exception::not_found_error("Scope Was Nullptr");
				}
				throw(exception::bad_boxed_cast(From.Type(), user_type<TTT>(), __LINE__));
			}
			else {
				if (auto ScopePtr = scope.lock()) {
					if (auto foundScope = ScopePtr->FindClass(user_type<typename std::remove_reference<typename std::remove_pointer<To>::type>::type>())) {
						auto tree1 = ScopePtr->GetCombinedTypeConversionTree();
						auto tree2 = foundScope->GetCombinedTypeConversionTree();
						auto tree = Type_Converter_Tree::Combine({ &tree2, &tree1 });

						std::string scopeName = foundScope->GetName();

						try {
							if (Any f = ScopePtr->CallFunction(scopeName, { From }, tree)) {
								return f.cast<typename std::remove_reference<typename std::remove_pointer<To>::type>::type>();
							}
						} catch (exception::not_found_error const& e) {}

						try {
							if (Any f = foundScope->CallFunction(scopeName, { From }, tree)) {
								return f.cast<typename std::remove_reference<typename std::remove_pointer<To>::type>::type>();
							}
						} catch (exception::not_found_error const& e) {}

						try {
							if (tree.Converts(From.Type(), user_type<TTT>())) {
								if (Any f = tree.Convert(From, user_type<TTT>())) {
									return f.cast<typename std::remove_reference<typename std::remove_pointer<To>::type>::type>();
								}								
							}	
						} catch (exception::not_found_error const& e) {}

						try {
							if (tree1.Converts(From.Type(), user_type<TTT>())) {
								if (Any f = tree1.Convert(From, user_type<TTT>())) {
									return f.cast<typename std::remove_reference<typename std::remove_pointer<To>::type>::type>();
								}
							}
						} catch (exception::not_found_error const& e) {}

						throw exception::bad_boxed_cast(From.Type(), user_type<TTT>(), __LINE__);
					}
					else {
						auto Type = fibers::user_type<typename std::remove_reference<typename std::remove_pointer<To>::type>::type>();
						throw exception::not_found_error(Type.name());
					}
				}
				else {
					throw exception::not_found_error("Scope Was Nullptr");
				}
			}
		};

		static Any Cast(Any const& From, Type_Info const& To, std::weak_ptr<Scope> scope) {
			if (auto ScopePtr = scope.lock()) {
				if (auto foundScope = ScopePtr->FindClass(To)) {
					auto tree1 = ScopePtr->GetCombinedTypeConversionTree();
					auto tree2 = foundScope->GetCombinedTypeConversionTree();
					auto tree = Type_Converter_Tree::Combine({ &tree2, &tree1 });

					try {
						if (Any f = ScopePtr->CallFunction(foundScope->GetName(), { From }, tree)) {
							return f;
						}
					} catch (exception::not_found_error e) {}

					try {
						if (Any f = foundScope->CallFunction(foundScope->GetName(), { From }, tree)) {
							return f;
						}
					} catch (exception::not_found_error const& e) {}

					try {
						if (tree.Converts(From.Type(), To)) {
							if (Any f = tree.Convert(From, To)) {
								return f;
							}		
						}	
					} catch (exception::not_found_error const& e) {}

					try {
						if (tree1.Converts(From.Type(), To)) {
							if (Any f = tree1.Convert(From, To)) {
								return f;
							}		
						}	
					} catch (exception::not_found_error const& e) {}

					throw exception::bad_boxed_cast(From.Type(), To, __LINE__);
				}
				else {
					if (auto p = To.lock()) {
						throw exception::not_found_error(p->name());
					}
				}
			}
			else {
				throw exception::not_found_error("Scope Was Nullptr");
			}
			throw(exception::bad_boxed_cast(From.Type(), To, __LINE__));
		};





	};
#endif

#if 0
	/* Engine state shared by all scopes or namespaces, which may be accessed in parallel by multiple parallel-execution scripts or parallel-execution scopes */
	class State {
	private:

	public:
		Type_Converter_Tree
			m_typeConverters; // built-in C++ type conversions (static, custom, etc.) Does not YET support conversion using ProxyFunctions. 

		fibers::containers::Map<
			std::string, // Type Name
			Type_Info // Type Info
		> m_typenames; // Typenames. Duplicates are not allowed.


	};

	class ScriptingState {
	private:
		Type_Converter_Tree 
			m_typeConverters; // built-in C++ type conversions (static, custom, etc.) Does not YET support conversion using ProxyFunctions. 

		/*
		To handle the following case:
		def ToString(Any x){ Meant to be a fall-back when no other specialization is available };
		def ToString(cweeStr x){ Specialization, should be used when all parameters match exactly };
		def ToString(double x){ Specialization, should be used when all parameters match exactly };

		The engine should utilize template specialization, similar to C++.
		Functions with template parameters (Param_Types().Template() == true) should be re-submitted to the matrix, if successfully used, with specializations for their successfully used types.

		Example 1:
			Let's say user shows up with the following:
				Function Name = "ToString";
				Function_Params = { cweeStr };
			Lookup should return:
				ToString(cweeStr);
			Because the search was matched exactly, we are done.

		Example 2:
			Let's say user shows up with the following:
				Function Name = "ToString";
				Function_Params = { int };
			Lookup should return:
				ToString(Any);
			Because the parameters do not exactly match ({Any} != {int}), after running the function, if successful, and a specialized version should be added to the map:
				ToString(int) -> which maps to -> ToString(Any);


		*/
		fibers::containers::Map<
			std::string, // Function Name (e.g. cweeStr). 
			std::shared_ptr<fibers::containers::Map<
			    Param_Types, // Function parameters (e.g. {cweeStr, Any}, or {Any, Any, Any}). 
			    Proxy_Function
			>>
		> m_functions; // functions. (e.g. `==` or `to_string`). Duplicate names are expected. 

		fibers::containers::Map<
			std::string, // Type Name
			Type_Info // Type Info
		> m_typenames; // Typenames. Duplicates are not allowed.

		fibers::containers::Map<
			std::string, 
			Any
		> m_objects; // objects. (e.g. "x" or "alpha"). Duplicate names are not allowed.

		fibers::containers::Map<
			std::string, 
			Type_Info
		> m_postfixes; // allowed postfixes (e.g. 10_ft, where "_ft" is the key) to their desired typename. Duplicate names are not allowed.

	public:
		auto& GetConversionTree() { return m_typeConverters; };
		const auto& GetConversionTree() const { return m_typeConverters; };

		auto& GetFunctions() { return m_functions; };
		const auto& GetFunctions() const { return m_functions; };

		auto& GetTypes() { return m_typenames; };
		const auto& GetTypes() const { return m_typenames; };

		auto& GetObjects() { return m_objects; };
		const auto& GetObjects() const { return m_objects; };

		auto& GetPostFixes() { return m_postfixes; };
		const auto& GetPostFixes() const { return m_postfixes; };

		bool AddType(std::string const& qualified_name, Type_Info const& type) {
			return m_typenames.emplace(qualified_name, type);
		};
		std::optional<Type_Info> GetType(std::string const& qualified_name) const {
			return m_typenames.at(qualified_name);
		};
		bool EraseType(std::string const& qualified_name) {
			return m_typenames.erase(qualified_name);
		};

		bool AddPostfix(std::string const& qualified_postix, Type_Info const& type) {
			return m_postfixes.emplace(qualified_postix, type);
		};
		std::optional<Type_Info> GetPostfix(std::string const& qualified_postix) {
			return m_postfixes.at(qualified_postix);
		};
		bool ErasePostfix(std::string const& qualified_postix) {
			return m_postfixes.erase(qualified_postix);
		};

		bool AddObj(std::string const& qualified_name, Any const& obj) {
			return m_objects.emplace(qualified_name, obj);
		};
		std::optional<Any> GetObj(std::string const& qualified_name) const {
			return m_objects.at(qualified_name);
		};
		bool EraseObj(std::string const& qualified_name) {
			return m_objects.erase(qualified_name);
		};

		bool AddFunction(std::string const& qualified_name, Proxy_Function const& obj) {
			auto& args = obj->Arguments();
			while (true) {
				if (!m_functions.contains(qualified_name)) {
					m_functions.emplace(qualified_name, std::make_shared<fibers::containers::Map< Param_Types, Proxy_Function>>());
				}

				auto function_param_selector = m_functions.at(qualified_name);
				if (function_param_selector.has_value()) {
					return function_param_selector.value()->emplace(args, obj);
				}
			}
		};
		bool AddFunction(std::string const& qualified_name, Proxy_Function const& obj, Param_Types const& parms) {
			while (true) {
				if (!m_functions.contains(qualified_name)) {
					m_functions.emplace(qualified_name, std::make_shared<fibers::containers::Map< Param_Types, Proxy_Function>>());
				}

				auto function_param_selector = m_functions.at(qualified_name);
				if (function_param_selector.has_value()) {
					return function_param_selector.value()->emplace(parms, obj);
				}
			}
		};
		std::optional<Proxy_Function> GetFunction(std::string const& qualified_name, Function_Params const& parms) const {
			auto function_param_selector = m_functions.at(qualified_name);
			if (function_param_selector.has_value()) {
				return function_param_selector.value()->at_hash(parms.hash());
			}
			return std::nullopt;
		}; 
		std::optional<Proxy_Function> GetFunction(std::string const& qualified_name, std::vector<fibers::Any> & parms) const {
			return GetFunction(qualified_name, Function_Params{ parms });
		};
		std::optional<Proxy_Function> GetFunction(std::string const& qualified_name, Param_Types const& parms) const {
			auto function_param_selector = m_functions.at(qualified_name);
			if (function_param_selector.has_value()) {
				return function_param_selector.value()->at(parms);
			}
			return std::nullopt;
		};
		std::shared_ptr<fibers::containers::Map<Param_Types, Proxy_Function>> GetFunctions(std::string const& qualified_name) const {
			auto function_param_selector = m_functions.at(qualified_name);
			if (function_param_selector.has_value()) {
				return function_param_selector.value();
			}
			return nullptr;
		};
		bool EraseFunction(std::string const& qualified_name, Param_Types const& obj) {
			auto function_param_selector = m_functions.at(qualified_name);
			if (function_param_selector.has_value()) {
				function_param_selector.value()->erase(obj);
			}
			return false;
		};
		bool EraseAllFunction(std::string const& qualified_name) {
			return m_functions.erase(qualified_name);
		};





	};

	class Stack {
	private:
		Stack*
			m_parent{ nullptr }; // parent scope -- may be empty.
		std::shared_ptr<fibers::containers::Stack<Stack>>
			m_children{ std::make_shared<fibers::containers::Stack<Stack>>() }; // children that are within this scope. 
		std::string
			m_name{ "" }; // namespace name -- may be empty.
	
	protected:
		std::string
			fullyQualifiedNamespace;
		static std::string createFullyQualifiedNamespace(std::string parentNamespace, std::string newName) {
			if (newName == "") {
				return parentNamespace;
			}
			else if (parentNamespace == "") {
				return newName;
			}
			else {
				std::string temp = parentNamespace + "::" + newName;

				while (temp.find("::::") != std::string::npos) {
					size_t start_pos = 0;
					while ((start_pos = temp.find("::::", start_pos)) != std::string::npos) {
						temp = temp.replace(start_pos, 4, "::");
						start_pos += 2; // In case 'to' contains 'from', like replacing 'x' with 'yx'
					}
				}

				while (temp.find("::") == 0) {
					size_t start_pos = 0;
					if ((start_pos = temp.find("::", start_pos)) == 0) {
						temp = temp.replace(start_pos, 2, "");
					}
				}

				return temp;
			}
		};
	
	public:
		Stack() : fullyQualifiedNamespace("") {};
		Stack(std::string const& p_name) : fullyQualifiedNamespace(p_name), m_name(p_name) {};
		Stack(std::string p_name, Stack* p_parentScope) : fullyQualifiedNamespace(createFullyQualifiedNamespace(p_parentScope->fullyQualifiedNamespace, p_name)), m_name(p_name), m_parent(p_parentScope) {};
		Stack(Stack const&) = default;
		Stack(Stack&&) = default;
		Stack& operator=(Stack const&) = default;
		Stack& operator=(Stack&&) = default;
		~Stack() = default;

		std::shared_ptr<Stack> push_scope(std::string namespaceName = "") {
			std::shared_ptr<Stack> NewScope = std::make_shared<Stack>(std::move(namespaceName), this);
			m_children->push(NewScope);
			return NewScope;
		};
		bool pop_scope() {
			return m_children->try_pop();
		};

	};
#endif

#if 0
	// A "namespace" is any (optionally named) scope, which may contains: 
	// Objects (including function objects), 
	// Function Definitions (e.g. static functions from C++ or internally),
	// Postfixes (e.g. 10_ft, where "_ft" is the provided key),
	// Children and parent scopes. Global scope has no parent.
	class Namespace {
	private:
		Namespace*
			m_parent{ nullptr }; // parent scope -- may be empty.
		std::shared_ptr<fibers::containers::Stack<Namespace>>
			m_children{ std::make_shared<fibers::containers::Stack<Namespace>>() }; // children that are within this scope. 
		std::string
			m_name{ "" }; // namespace name -- may be empty.
		concurrency::concurrent_unordered_multimap<std::string, Proxy_Function> 
			m_functions; // functions in this namespace. Duplicate names are allowed.
		concurrency::concurrent_unordered_map<std::string, Any> 
			m_objects; // objects in this namespace (e.g. "x" or "alpha"). Duplicate names are not allowed.
		concurrency::concurrent_unordered_map<std::string, Type_Info> 
			m_postfixes; // allowed postfixes (e.g. 10_ft, where "_ft" is the key) to their desired typename. Duplicate names are not allowed.

	protected:
		std::string
			fullyQualifiedNamespace;
		static std::string createFullyQualifiedNamespace(std::string parentNamespace, std::string newName) {
			if (newName == "") {
				return parentNamespace;
			}
			else if (parentNamespace == "") {
				return newName;
			}
			else {
				std::string temp = parentNamespace + "::" + newName;

				while (temp.find("::::") != std::string::npos) {
					size_t start_pos = 0;
					while ((start_pos = temp.find("::::", start_pos)) != std::string::npos) {
						temp = temp.replace(start_pos, 4, "::");
						start_pos += 2; // In case 'to' contains 'from', like replacing 'x' with 'yx'
					}
				}

				while (temp.find("::") == 0) {
					size_t start_pos = 0;
					if ((start_pos = temp.find("::", start_pos)) == 0) {
						temp = temp.replace(start_pos, 2, "");
					}
				}

				return temp;
			}
		};

	public:
		Namespace() : fullyQualifiedNamespace("") {};
		Namespace(std::string const& p_name) : fullyQualifiedNamespace(p_name), m_name(p_name) {};
		Namespace(std::string p_name, Namespace* p_parentScope) : fullyQualifiedNamespace(createFullyQualifiedNamespace(p_parentScope->fullyQualifiedNamespace, p_name)), m_name(p_name), m_parent(p_parentScope) {};
		Namespace(Namespace const&) = default;
		Namespace(Namespace &&) = default;
		Namespace& operator=(Namespace const&) = default;
		Namespace& operator=(Namespace&&) = default;
		~Namespace() = default;



		std::shared_ptr<Namespace> push_scope(std::string namespaceName = "") {
			std::shared_ptr<Namespace> NewScope = std::make_shared<Namespace>(std::move(namespaceName), this);
			m_children->push(NewScope);
			return NewScope;
		};
		bool pop_scope() {
			return m_children->try_pop();
		};
		// recurseively searches up the scope until it finds the desired key. Does NOT search the children of the parent(s). 
		bool TryFindPostfix(std::string const& postfix_key, Type_Info& out) {
			auto iter{ m_postfixes.find(postfix_key) };
			if (iter != m_postfixes.end()) {
				out = iter->second;
				return true;
			}
			else if (m_parent) {
				return m_parent->TryFindPostfix(postfix_key, out);
			}
			else {
				return false;
			}
		};
		// recurseively searches up the scope until it finds the desired key. Does NOT search the children of the parent(s). 
		bool TryFindObject(std::string const& objectName, Any& out) {
			auto iter{ m_objects.find(objectName) };
			if (iter != m_objects.end()) {
				out = iter->second;
				return true;
			}
			else if (m_parent) {
				return m_parent->TryFindObject(objectName, out);
			}
			else {
				return false;
			}
		};
		// recurseively searches up the scope until it finds the desired key. Does NOT search the children of the parent(s). 
		bool TryFindFunction(std::string const& functionName, Proxy_Function& out) {
			auto iter{ m_functions.find(functionName) };
			if (iter != m_functions.end()) {
				out = iter->second;
				return true;
			}
			else if (m_parent) {
				return m_parent->TryFindFunction(functionName, out);
			}
			else {
				return false;
			}
		};
		
		std::string_view Name() const {
			return m_name;
		};
		
		std::string_view QualifiedName() const {
			return fullyQualifiedNamespace;
		};

		size_t NumChildrenScopes() const {
			return m_children->size();
		};

		void AddObject(std::string name, Any && obj) {
			m_objects.insert(std::pair(std::move(name), std::forward<Any>(obj)));
		};
		void AddObject(std::string const& name, Any const& obj) {
			m_objects.insert(std::pair(name, obj));
		};



	};
	
#endif

#if 0










	template<class T>
	using SmallVector = std::vector<T>;

	class Stack_Holder_Impl {
	public:



		using Scope = utility::QuickFlatMap<std::string, Boxed_Value, str_equal>;
		using StackData = SmallVector<Scope>;
		using Stacks = SmallVector<StackData>;
		using Call_Param_List = SmallVector<Boxed_Value>;
		using Call_Params = SmallVector<Call_Param_List>;

		Stack_Holder_Impl() {
			push_stack();
			push_call_params();
		};

		void push_stack_data() {
			stacks.back().emplace_back();
		};

		void push_stack() { stacks.emplace_back(1); };

		void push_call_params() {
			call_params.emplace_back();
		};

		Stacks stacks;
		Call_Params call_params;
		int call_depth = 0;
	};

	using Stack_Holder = Stack_Holder_Impl;






	struct File_Position {
		int line = 0;
		int column = 0;

		constexpr File_Position(int t_file_line, int t_file_column) noexcept
			: line(t_file_line)
			, column(t_file_column) {
		};
		constexpr File_Position() noexcept = default;
	};
	struct Parse_Location {
		Parse_Location(std::string t_fname = "", const int t_start_line = 0, const int t_start_col = 0, const int t_end_line = 0, const int t_end_col = 0)
			: start(t_start_line, t_start_col)
			, end(t_end_line, t_end_col)
			, filename(std::make_shared<std::string>(std::move(t_fname))) {
		};

		Parse_Location(std::shared_ptr<std::string> t_fname,
			const int t_start_line = 0,
			const int t_start_col = 0,
			const int t_end_line = 0,
			const int t_end_col = 0)
			: start(t_start_line, t_start_col)
			, end(t_end_line, t_end_col)
			, filename(std::move(t_fname)) {
		};

		File_Position start;
		File_Position end;
		std::shared_ptr<std::string> filename;
	};
	
	class AST_Node;
	BETTER_ENUM(AST_Node_Type, uint8_t,
		Id,
		Fun_Call,
		Unused_Return_Fun_Call,
		Arg_List,
		Equation,
		Var_Decl,
		Assign_Decl,
		Array_Call,
		Dot_Access,
		Lambda,
		Block,
		Scopeless_Block,
		Def,
		While,
		If,
		For,
		Ranged_For,
		Inline_Array,
		Inline_Map,
		Return,
		File,
		Prefix,
		Break,
		Continue,
		Map_Pair,
		Value_Range,
		Inline_Range,
		Do,
		Try,
		Catch,
		Finally,
		Method,
		Attr_Decl,
		Logical_And,
		Logical_Or,
		Reference,
		Switch,
		Case,
		Default,
		Noop,
		Class,
		Binary,
		Arg,
		Global_Decl,
		Constant,
		Compiled,
		ControlBlock,
		Postfix,
		Assign_Retroactively,
		Parallel,
		AST_Node_Type_end
	);
	using AST_NodePtr = std::unique_ptr<AST_Node>;
	struct AST_Node {
	public:
		const AST_Node_Type identifier;
		const std::string text;
		fibers::Type_Info potentialReturnType;
		Parse_Location location;

		const std::string& filename() const noexcept { return *location.filename; }

		const File_Position& start() const noexcept { return location.start; }

		const File_Position& end() const noexcept { return location.end; }

		std::string pretty_print() const {
			std::ostringstream oss;

			oss << text;

			for (auto& elem : get_children()) {
				oss << elem.get().pretty_print() << ' ';
			}

			return oss.str();
		}
		
		virtual std::vector<AST_Node*> get_children() const = 0;
		virtual fibers::Any eval(const chaiscript::detail::Dispatch_State& t_e) const = 0;

		/// Prints the contents of an AST node, including its children, recursively
		std::string ToString(const std::string& t_prepend = "") const {
			std::ostringstream oss;

			oss << t_prepend << "(" << ast_node_type_to_string(this->identifier) << ") " << this->text << " : " << this->location.start.line
				<< ", " << this->location.start.column << '\n';

			for (auto& elem : get_children()) {
				oss << elem.get().to_string(t_prepend + "  ");
			}
			return oss.str();
		}

		virtual ~AST_Node() noexcept = default;
		AST_Node(AST_Node&&) = default;
		AST_Node& operator=(AST_Node&&) = delete;
		AST_Node(const AST_Node&) = delete;
		AST_Node& operator=(const AST_Node&) = delete;

	protected:
		AST_Node(std::string t_ast_node_text, AST_Node_Type t_id, Parse_Location t_loc)
			: identifier(t_id)
			, text(std::move(t_ast_node_text))
			, location(std::move(t_loc))
			, potentialReturnType(Type_Info(), t_id, false)
		{
		}
	};


#endif



};

