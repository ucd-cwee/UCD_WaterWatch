#pragma once

#include "Fibers.h"

namespace scripting {
	using Type_Info = fibers::Type_Info;
	using Any = fibers::Any;
	template <typename T> __forceinline constexpr Type_Info user_type() { return fibers::user_type<T>(); };

	namespace exception {
		/// \brief Thrown in the event that a Boxed_Value cannot be cast to the desired type
		///
		/// It is used internally during function dispatch and may be used by the end user.
		///
		/// \sa chaiscript::boxed_cast
		class bad_boxed_cast : public std::bad_cast {
		public:
			bad_boxed_cast(Type_Info const& t_from, Type_Info const& t_to, std::string_view const& t_what) noexcept
				: from(t_from)
				, to(t_to)
				, m_what(t_what) {};

			bad_boxed_cast(Type_Info const& t_from, Type_Info const& t_to) noexcept
				: from(t_from)
				, to(t_to)
				, m_what("Cannot perform boxed_cast") {};

			explicit bad_boxed_cast(std::string_view const& t_what) noexcept
				: from(user_type<void>())
				, to(user_type<void>())
				, m_what(t_what) {};

			bad_boxed_cast(const bad_boxed_cast&) noexcept = default;
			~bad_boxed_cast() noexcept override = default;

			/// \brief Description of what error occurred
			const char* what() const noexcept override { return m_what.data(); };

			Type_Info from; ///< Type_Info contained in the Boxed_Value
			Type_Info to; ///< std::type_info of the desired (but failed) result type

		private:
			std::string_view m_what;
		};

		class bad_boxed_type_cast : public bad_boxed_cast {
		public:
			bad_boxed_type_cast(const Type_Info& t_from, const Type_Info& t_to, std::string_view const& t_what) noexcept
				: bad_boxed_cast(t_from, t_to, t_what) {
			}

			bad_boxed_type_cast(const Type_Info& t_from, const Type_Info& t_to) noexcept
				: bad_boxed_cast(t_from, t_to) {
			}

			explicit bad_boxed_type_cast(std::string_view const& w) noexcept
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
				: std::range_error(Units::printf("Arity mismatch: function requires %i parameters, but only %i were provided", t_expected, t_got))
				, got(t_got)
				, expected(t_expected) {
			}

			arity_error(const arity_error&) = default;

			~arity_error() noexcept override = default;

			int got;
			int expected;
		};
	}; // namespace exception

	namespace details {
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
				//return 1;

				static double actualCost{ -1 };
				static std::decay_t<std::tuple_element_t<0, fibers::utilities::function_traits< decltype(std::function(m_func)) >::arguments>> inputObj{};
				if (actualCost < 0) {
					auto startT = clock_ns();
					(void)(m_func(inputObj));
					actualCost = 1.0 + (double)(clock_ns() - startT) / 100.0;
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
					auto startT = clock_ns();
					(void)((To)(inputObj));
					actualCost = 1.0 + (double)(clock_ns() - startT) / 100.0;
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
	private:
		class Node {
		public:
			Type_Info from;
			// does not support deleting type conversions, but that should be OK, since type conversions should be baked-in.
			concurrency::concurrent_unordered_map<
				Type_Info, // to
				std::shared_ptr<details::Type_Conversion_Base> // converter function
			> connections;
			// does not support deleting type conversions, but that should be OK, since type conversions should be baked-in.
			mutable concurrency::concurrent_unordered_map<
				Type_Info, // to
				std::tuple<
				std::shared_ptr<std::vector<Type_Info>> // list of target types to convert to, including the final "to". 
				, fibers::synchronization::impl::InterlockedLong // version of this conversion list
				>
			> cached_conversions;
		};
		// does not support deleting type conversions, but that should be OK, since type conversions should be baked-in.
		concurrency::concurrent_unordered_map<
			Type_Info, // from
			Node // to/connections
		> nodes;

		fibers::synchronization::impl::InterlockedLong version;

	private:
		// Solves Dijkstra's algorithm to determine the shortest path for "From" to "To", puts the path in "Out", and returns true. 
		// If no path is possible, returns false.
		bool TryCreateConversionPath(Type_Info const& From, Type_Info const& To, std::vector<Type_Info>& out) const {
			out.clear();
			if (nodes.count(From) > 0) {

				std::map<
					Type_Info, // FROM vertex
					std::map<
					Type_Info, // TO vertex
					double // distance
					>
				> vertices_and_distances;

				std::map<
					Type_Info, // FROM vertex
					std::vector<Type_Info> // predecessors
				> vertices_and_predecessors;

				std::map<
					Type_Info, // vertex
					double // weight
				> vertices_and_weights;

				std::set< Type_Info > visited;

				for (auto& node : nodes) {
					vertices_and_weights[node.first] = std::numeric_limits<double>::max();
					for (auto& connection : node.second.connections) {
						vertices_and_distances[node.first][connection.first] = connection.second->cost();
					}
				}

				vertices_and_weights[From] = 0;
				vertices_and_predecessors[From] = {};

				Type_Info CurrentVertex = From;
				int numToVisit = 1;
				bool FoundEnd = false;
				int countDown = 0;
				std::multimap<double, Type_Info> sorted;
				while ((visited.size() < vertices_and_distances.size()) && (numToVisit-- >= 1)) {
					// for each adjacent node...
					numToVisit += vertices_and_distances[CurrentVertex].size();
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
					out = vertices_and_predecessors[To];

					std::cout << Units::printf("Converting %s -> %s requires:\n\t%s", From.name(), To.name(), From.name());
					for (auto& x : out) {
						std::cout << Units::printf(" ... %s", x.name());
					}
					std::cout << Units::printf(" (%f)\n", (float)vertices_and_weights[To]);

					return true;
				}
			}
			return false;
		};

	public:
		// add an automatic static or polymorphic conversion
		template <typename FromType, typename ToType> bool AddConverter() {
			auto fromTypeInfo{ user_type<FromType>() };
			auto toTypeInfo{ user_type<ToType>() };

			constexpr static bool is_polymorphic = std::is_base_of< ToType, FromType>::value;
			constexpr static bool is_static = details::impl::is_explicitly_convertible_to<FromType, ToType>::value;
			constexpr static bool is_bidir = details::impl::is_explicitly_convertible_to<ToType, FromType>::value;
			if constexpr (!is_static && !is_polymorphic) {
				return false;
			}

			Node& node = nodes[fromTypeInfo];
			node.from = fromTypeInfo;

			auto& targetLocation = node.connections[toTypeInfo];
			if (!targetLocation) {
				if constexpr (std::is_base_of< ToType, FromType>::value) {
					targetLocation = std::dynamic_pointer_cast<details::Type_Conversion_Base>(std::make_shared<details::Dynamic_Type_Conversion_Impl<FromType, ToType>>());
				}
				else {
					targetLocation = std::dynamic_pointer_cast<details::Type_Conversion_Base>(std::make_shared<details::Static_Type_Conversion_Impl<FromType, ToType>>());
				}

				(void)targetLocation->cost(); // cache the cost to perform this conversion

				node.cached_conversions[toTypeInfo] = {
					std::make_shared<std::vector<Type_Info>>(std::vector<Type_Info>({ toTypeInfo })),
					fibers::synchronization::impl::InterlockedLong(std::numeric_limits<double>::max())
				}; // even if there was a previous cached conversion, override it.

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
			auto toTypeInfo = user_type<fibers::utilities::function_traits< decltype(std::function(t_func)) >::result_type>();
			auto fromTypeInfo = user_type<std::decay_t<std::tuple_element_t<0, fibers::utilities::function_traits< decltype(std::function(t_func)) >::arguments>>>();

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

			Node& node = nodes[fromTypeInfo];
			node.from = fromTypeInfo;

			auto& targetLocation = node.connections[toTypeInfo];
			if (!targetLocation) {
				targetLocation = std::dynamic_pointer_cast<details::Type_Conversion_Base>(std::make_shared<details::Custom_Type_Conversion_Impl<Callable>>(std::move(t_func)));

				(void)targetLocation->cost(); // cache the cost to perform this conversion

				node.cached_conversions[toTypeInfo] = {
					std::make_shared<std::vector<Type_Info>>(std::vector<Type_Info>({ toTypeInfo })),
					fibers::synchronization::impl::InterlockedLong(std::numeric_limits<double>::max())
				}; // even if there was a previous cached conversion, override it.

				version++;

				return true;
			}
			return false;
		};

		/// <summary>
		/// returns true if it could convert "From" to "To" type, and stores the converted answer in "result". Otherwise returns false. 
		/// </summary>
		bool TryConvert(Any const& From, Type_Info const& to, Any& result) const {
			auto fromType = From.Type();
			if (fromType == to) {
				result = From;
				return true;
			}
			else {
				if (nodes.count(fromType) > 0) {
					Node const& node = nodes.at(fromType);

					// If the conversion path does not exist OR is outdated, then re-create it. 
					if (node.cached_conversions.count(to) == 0 || std::get<1>(node.cached_conversions.at(to)).GetValue() < version.GetValue()) {
						auto newCache{ std::make_shared<std::vector<Type_Info>>() };
						if (newCache) {
							auto& newCached = *newCache.get();
							if (TryCreateConversionPath(fromType, to, newCached)) {
								node.cached_conversions.insert({ to, { newCache, version.GetValue() } });
								std::get<0>(node.cached_conversions.at(to)) = newCache;
								std::get<1>(node.cached_conversions.at(to)) = version.GetValue();
							}
							else { // cache the failure -- to prevent repeated Dijkstra searches unless the tree is updated to (hopefully) bridge the gap.
								node.cached_conversions.insert({ to, { {}, version.GetValue() } });
								std::get<0>(node.cached_conversions.at(to)) = {};
								std::get<1>(node.cached_conversions.at(to)) = version.GetValue();
							}
						}
					}

					// try again... hopefully it has been made (for better or worse)
					if (node.cached_conversions.count(to) > 0) {
						try {
							std::shared_ptr<std::vector<Type_Info>> conversion_path = std::get<0>(node.cached_conversions.at(to));
							if (conversion_path && conversion_path->size() > 0) {
								Any currentFrom = From;
								const Node* currentNode = &node;
								for (auto& intermediate_to_type : *conversion_path) {
									currentFrom = currentNode->connections.at(intermediate_to_type)->convert(currentFrom);
									currentNode = &nodes.at(currentFrom.Type());
								}
								result = currentFrom;
								return true;
							}
						}
						catch (...) {
							node.cached_conversions.insert({ to, { {}, version.GetValue() } });
							std::get<0>(node.cached_conversions.at(to)) = {};
							std::get<1>(node.cached_conversions.at(to)) = version.GetValue();
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
		Any Convert(Any const& From, Type_Info const& to) const {
			Any result;
			if (!TryConvert(From, to, result)) {
				throw fibers::exception::bad_any_cast(From.Type(), to);
			}
			return result;
		};
		/// <summary>
		/// Converts "From" to the type of "To" and returns the final conversion. If not possible, then it throws an error. 
		/// </summary>
		template <typename To> To Convert(Any const& From) const { return Convert(From, user_type<To>()).cast(); };

		// Symbolic "cost" to perform the conversion, in 100's of nanoseconds. Not meant to be precise, but meant to be relative for comparison with other converters.
		double ConversionCost(Type_Info const& From, Type_Info const& To) const {
			if (From == To) {
				return 0;
			}
			else {
				if (nodes.count(From) > 0) {
					Node const& node = nodes.at(From);

					// If the conversion path does not exist OR is outdated, then re-create it. 
					if (node.cached_conversions.count(To) == 0 || std::get<1>(node.cached_conversions.at(To)).GetValue() < version.GetValue()) {
						auto newCache{ std::make_shared<std::vector<Type_Info>>() };
						if (newCache) {
							auto& newCached = *newCache.get();
							if (TryCreateConversionPath(From, To, newCached)) {
								node.cached_conversions.insert({ To, { newCache, version.GetValue() } });
								std::get<0>(node.cached_conversions.at(To)) = newCache;
								std::get<1>(node.cached_conversions.at(To)) = version.GetValue();
							}
							else { // cache the failure -- to prevent repeated Dijkstra searches unless the tree is updated to (hopefully) bridge the gap.
								node.cached_conversions.insert({ To, { {}, version.GetValue() } });
								std::get<0>(node.cached_conversions.at(To)) = {};
								std::get<1>(node.cached_conversions.at(To)) = version.GetValue();
							}
						}
					}

					// try again... hopefully it has been made (for better or worse)
					if (node.cached_conversions.count(To) > 0) {
						std::shared_ptr<std::vector<Type_Info>> conversion_path = std::get<0>(node.cached_conversions.at(To));
						if (conversion_path && conversion_path->size() > 0) {
							double cost{ 0 };
							const Node* currentNode = &node;
							for (auto& intermediate_to_type : *conversion_path) {
								cost += currentNode->connections.at(intermediate_to_type)->cost();
								currentNode = &nodes.at(intermediate_to_type);
							}
							return cost;
						}
					}
				}
				return std::numeric_limits<double>::max();
			}
		};
		// Symbolic "cost" to perform the conversion, in 100's of nanoseconds. Not meant to be precise, but meant to be relative for comparison with other converters.
		template <typename From, typename To> double ConversionCost() const { return ConversionCost(user_type<From>(), user_type<To>()); };

		// true if the tree knows how to convert From into To
		bool Converts(Type_Info const& From, Type_Info const& To) const {
			if (From == To) {
				return true;
			}
			else {
				if (nodes.count(From) > 0) {
					Node const& node = nodes.at(From);

					// If the conversion path does not exist OR is outdated, then re-create it. 
					if (node.cached_conversions.count(To) == 0 || std::get<1>(node.cached_conversions.at(To)).GetValue() < version.GetValue()) {
						auto newCache{ std::make_shared<std::vector<Type_Info>>() };
						if (newCache) {
							auto& newCached = *newCache.get();
							if (TryCreateConversionPath(From, To, newCached)) {
								node.cached_conversions.insert({ To, { newCache, version.GetValue() } });
								std::get<0>(node.cached_conversions.at(To)) = newCache;
								std::get<1>(node.cached_conversions.at(To)) = version.GetValue();
							}
							else { // cache the failure -- to prevent repeated Dijkstra searches unless the tree is updated to (hopefully) bridge the gap.
								node.cached_conversions.insert({ To, { {}, version.GetValue() } });
								std::get<0>(node.cached_conversions.at(To)) = {};
								std::get<1>(node.cached_conversions.at(To)) = version.GetValue();
							}
						}
					}

					// try again... hopefully it has been made (for better or worse)
					if (node.cached_conversions.count(To) > 0) {
						return true;
					}
				}
				return false;
			}
		};
		// true if the tree knows how to convert From into To
		bool Converts(Any const& From, Type_Info const& To) const { return Converts(From.Type(), To); };
		// true if the tree knows how to convert From into To
		template <typename From, typename To> bool Converts() const { return Converts(user_type<From>(), user_type<To>()); };
		// true if the tree knows how to convert From into To
		template <typename To> bool Converts(Any const& From) const { return Converts(From, user_type<To>()); };



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
			for (auto& s : *this) {
				h = (h * A) ^ (std::hash<Type_Info>()(s.Type()) * B);
			}
			auto result = h % C;
			return result;
		};

	public:
		Function_Params(Any* t_begin, Any* t_end)
			: m_begin(t_begin)
			, m_end(t_end)
			, hash_value(0)
		{
			hash_value = HashTypes();
		};
		explicit Function_Params(Any& bv)
			: m_begin(&bv)
			, m_end(m_begin + 1) 
			, hash_value(0) 
		{
			hash_value = HashTypes();
		};
		explicit Function_Params(std::vector<Any>& vec)
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
		[[nodiscard]] constexpr std::size_t size() const noexcept { return std::size_t(m_end - m_begin); }
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
		Param_Types() {}

		explicit Param_Types(std::vector<std::pair<std::string, Type_Info>> t_types)
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
			for (size_t i = 0; i < t_rhs.size(); i++) if (m_types[i].second > t_rhs[i].second) return true;
			return false;
		};
		bool operator<(const Param_Types& t_rhs) const {
			if (t_rhs.size() < size()) return true;
			for (size_t i = 0; i < t_rhs.size(); i++) if (m_types[i].second < t_rhs[i].second) return true;
			return false;
		};
		bool operator>=(const Param_Types& t_rhs) const {
			return !operator<(t_rhs);
		};
		bool operator<=(const Param_Types& t_rhs) const {
			return !operator>(t_rhs);
		};

		// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
		std::vector<Any> convert(Function_Params t_params, const Type_Converter_Tree& t_conversions) const {
			auto vals = t_params.to_vector();
			for (size_t i = 0; i < vals.size(); ++i) {
				const auto& name = m_types[i].first;
				const auto& bv = vals[i];
				const auto& ti = m_types[i].second;
				if (!ti.is_undef()) {
					vals[i] = t_conversions.Convert(bv, ti); // success or failure, caches the result for faster future eval's
				}
			}
			return vals;
		};

		// Tests if the conversion from the input parameters to the necessary types is possible. 
		bool converts(Function_Params t_params, const Type_Converter_Tree& t_conversions) const {
			// Quick return if the types exactly match.
			if (t_params.hash() == hash()) { return true; }

			auto vals = t_params.to_vector();
			for (size_t i = 0; i < vals.size(); ++i) {
				const auto& name = m_types[i].first;
				const auto& bv = vals[i];
				const auto& ti = m_types[i].second;
				if (!ti.is_undef()) {
					if (!t_conversions.Converts(bv.Type(), ti)) return false;
				}
			}
			return true;
		};

		// Symbolic "cost" to perform the conversion, in 100's of nanoseconds. Not meant to be precise, but meant to be relative for comparison with other converters.
		double conversion_cost(Function_Params t_params, const Type_Converter_Tree& t_conversions) const {
			double out{ 0 };
			for (size_t i = 0; i < t_params.size(); ++i) {
				const auto& bv = t_params[i];
				const auto& ti = m_types[i].second;
				if (!ti.is_undef()) {
					auto cost = t_conversions.ConversionCost(t_params[i].Type(), ti); // success or failure, caches the result for faster future eval's
					if (cost >= std::numeric_limits<double>::max()) return std::numeric_limits<double>::max();
					else out += cost;
				}
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

			// Symbolic "cost" to perform the conversion, in 100's of nanoseconds. Not meant to be precise, but meant to be relative for comparison with other converters.
			double conversion_cost(Function_Params t_params, const Type_Converter_Tree& t_conversions) const {
				return m_types.conversion_cost(t_params, t_conversions);
			};

			Any operator()(const Function_Params& params, const Type_Converter_Tree& t_conversions) const {
				if (params.size() == m_types.size()) {
					return do_call(convert(params, t_conversions));
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
			bool compare_first_type(const Any& bv, const Type_Converter_Tree& t_conversions) const noexcept {
				if (m_types.size() > 0) {
					return t_conversions.Converts(bv, m_types[0].second);
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
		return callable->operator()(Function_Params{ const_cast<std::vector<Any>&>(inputs) }, conversionTree);
	};




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


    class Scope;
    class Namespace;
	class Class;

    class Scope {		
	public:
		Scope(std::weak_ptr<Scope> parent = std::weak_ptr<Scope>())
			: p_parent{ parent }
		{};
		Scope(Scope const&) = default;
		Scope(Scope&&) = default;
		Scope& operator=(Scope const&) = default;
		Scope& operator=(Scope&&) = default;
		virtual ~Scope() = default;

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
		};
		virtual std::string GetQualifiedNamespace() const {
			std::string path = "::";

			auto parent = p_parent.lock();

			if (!parent) {
				//auto name{ GetName() };
				//if (!name.empty()) {
				//	path = "::" + name + "::";
				//}
				//else {
					path = "::";
				//}
			}
			else {
				auto name{ GetName() };
				if (!name.empty()) {
					path = parent->GetQualifiedNamespace() + name + "::";
				}
				else {
					path = parent->GetQualifiedNamespace();
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
		virtual bool AddChild(std::shared_ptr<Namespace> p_namespace) {
			return p_children.emplace(((Scope*)p_namespace.get())->GetName(), p_namespace);
		};
		virtual bool AddUsing(std::weak_ptr<Namespace> const& p_namespace) {
			auto ptr = p_namespace.lock();
			if (ptr) {
				return p_using.emplace(((Scope*)(ptr.get()))->GetName(), p_namespace);
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
					((Scope*)(child.second.get()))->Print(indentLevel + 2);
				}
			}
			else {
				std::cout << GetName() << std::endl;
				for (auto& child : p_children) {
					((Scope*)(child.second.get()))->Print(indentLevel + GetName().length());
				}
			}

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
					if (FindChildNearestToQualifiedNamespace(std::dynamic_pointer_cast<Scope>(child_namespace.second), fullyQualifiedNamespace, best_scope, best_match_count)) {
						toReturn = true;
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

	public:
		/* Get all named namespaces that are discoverable from the current Scope */
		virtual std::map<std::string, std::weak_ptr<Namespace>> GetAvailableNamespaces(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true) const {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > using namespaces

			std::map<std::string, std::weak_ptr<Namespace>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetAvailableNamespaces(evaluated, false);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (auto ptr = usingScope.second.lock()) {
					for (auto& using_child : ((Scope*)ptr.get())->p_children) {
						if (using_child.second) {
							// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
							for (auto& using_namespace : ((Scope*)using_child.second.get())->GetAvailableNamespaces(evaluated, false)) {
								out[using_namespace.first] = using_namespace.second;
							}
						}
					}
				}
			}

			// get my actual children last...
			for (auto& using_child : this->p_children) {
				if (using_child.second) {
					// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
					for (auto& using_namespace : ((Scope*)using_child.second.get())->GetAvailableNamespaces(evaluated, false)) {
						out[using_namespace.first] = using_namespace.second;
					}
				}
			}

			// also remember to add myself (if I am a namespace)...
			if (IsNamespace()) {
				out[GetName()] = std::dynamic_pointer_cast<Namespace>(p_self.lock());
			}

			return out;
		};

		/* Get all namespaces that may be used to search for an object (e.g this scope, it's USING scopes, its PARENT scopes */
		virtual std::set<std::shared_ptr<Scope>> GetScopesForObjectSearch(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true) const {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > using namespaces

			std::set<std::shared_ptr<Scope>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetScopesForObjectSearch(evaluated, false);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (auto ptr = usingScope.second.lock()) {
					for (auto x : ((Scope*)ptr.get())->GetScopesForObjectSearch(evaluated, false)) {
						out.emplace(x);
					}
				}
			}

			// also remember to add myself (if I am a namespace)...
			out.emplace(p_self.lock());
			
			return out;
		};


	public:
		// Attempts to find a namespace or class with the requested namespace name. 
		// If qualified (e.g. starts with "::") then it attempts to find it from the global root -> down. 
		// If not qualified, then it attempts to find it from the current node -> up.
		// If the scope is not found, it MAY suggest the placement of the new namespace as a child of the provided "best_scope"
		// For example, "::std::string::impl" (assuming impl does not exist) may return the node for "::std::string::")
		virtual bool TryFindScope(std::shared_ptr<Scope>& best_scope, std::string const& Namespace/* = "string"*/) const {
			decltype(auto) available_scopes = GetAvailableNamespaces();

			std::string scopeToFind = Namespace;
			while (scopeToFind.find("::") == 0) { scopeToFind = scopeToFind.substr(2); }
			while (scopeToFind.rfind("::") == (scopeToFind.length()-2)) { scopeToFind = scopeToFind.substr(0, scopeToFind.length() - 2); }

			auto f = available_scopes.find(scopeToFind);
			if (f != available_scopes.end()) {
				best_scope = std::dynamic_pointer_cast<Scope>(f->second.lock());
				if (best_scope) return true;
			}

			// failed to find it -- can we make a recommendation of where to put it?
			for (; scopeToFind.rfind("::") != std::string::npos;) {
				scopeToFind = scopeToFind.substr(0, scopeToFind.rfind("::"));

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

			// return THIS scope
			best_scope = p_self.lock();
			return false;
		};




	protected:
		virtual bool TryFindObjectImpl(std::string const& objectName, std::shared_ptr<fibers::Any>& out, std::set<std::string>& previouslyEvaluated) const {
			auto qualifiedNamespace = this->GetQualifiedNamespace();
			if (previouslyEvaluated.count(qualifiedNamespace) > 0) {
				return false;
			}
			previouslyEvaluated.emplace(qualifiedNamespace);

			std::string scopeName;
			std::string objectNameActual;
			size_t lastOfColons{ 0 };
			if ((lastOfColons = objectName.find_last_of("::")) == std::string::npos) {
				scopeName = "";
				objectNameActual = objectName;

				std::shared_ptr<Scope> p = p_self.lock();
				while (p) {
					auto x = p_objects.at(objectNameActual);
					if (x.has_value()){
						out = x.value();
						return true;
					}
					else {
						// test the using...
						for (auto& usingParent : p->p_using) {
							auto ptr2 = usingParent.second.lock();
							if (ptr2 && std::dynamic_pointer_cast<Scope>(ptr2)->TryFindObjectImpl(objectName, out, previouslyEvaluated)) {
								return true;
							}
						}
						// test the parent...
						p = p->p_parent.lock();
					}
				}

				return false;
			}
			else {
				objectNameActual = objectName.substr(lastOfColons + 1);
				scopeName = objectName.substr(0, lastOfColons - 1);

				std::shared_ptr<Scope> foundScope;
				if (this->TryFindScope(foundScope, scopeName)) {
					auto x = foundScope->p_objects.at(objectNameActual);
					if (x.has_value()) {
						out = x.value();
						return true;
					}
				}
				return false;
			}
		};
	
    public:
		//// Attempts to find a namespace or class with the requested namespace name. 
		//// If qualified (e.g. starts with "::") then it attempts to find it from the global root -> down. 
		//// If not qualified, then it attempts to find it from the current node -> up.
		//// If the scope is not found, it MAY suggest the placement of the new namespace as a child of the provided "best_scope"
		//// For example, "::std::string::impl" (assuming impl does not exist) may return the node for "::std::string::")
		//virtual bool TryFindScope(std::shared_ptr<Scope>& best_scope, std::string const& Namespace/* = "string"*/) const {
		//	std::shared_ptr<Scope> toSearch = p_self.lock();
		//	while (toSearch) {
		//		if (toSearch->TryFindScopeImpl(best_scope, Namespace)) {
		//			return true;
		//		}
		//		// check our using children to see if we can improve this match
		//		for (auto& parent_namespace : toSearch->p_using) {
		//			auto ptr = parent_namespace.second.lock();
		//			if (ptr) {
		//				if (std::dynamic_pointer_cast<Scope>(ptr)->TryFindScopeImpl(best_scope, Namespace)) {
		//					return true;
		//				}
		//			}
		//		}
		//		toSearch = toSearch->p_parent.lock();
		//	}
		//	return false;
		//};

		virtual bool TryFindObject(std::string const& objectName /* x, y, etc. */, std::shared_ptr<fibers::Any>& out) const {
			out = nullptr;
			std::set<std::string> previouslyEvaluated;
			return TryFindObjectImpl(objectName, out, previouslyEvaluated);
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


	};
	class Namespace : public Scope {
	public:
		Namespace(std::weak_ptr<Scope> parent = std::weak_ptr<Scope>(), std::string const& Name = "")
			: Scope(parent)
			, p_Name{ Name }
		{};
		Namespace(Namespace const&) = default;
		Namespace(Namespace&&) = default;
		Namespace& operator=(Namespace const&) = default;
		Namespace& operator=(Namespace&&) = default;
		virtual ~Namespace() = default;

	public:
		std::string
			p_Name; // e.g. "", or "_NAMESPACE_NAME_", or "_CLASS_NAME_"
		fibers::containers::Map<std::string, Type_Info>
			m_postfixes{}; // allowed postfixes (e.g. 10_ft, where "_ft" is the key) to their desired typename. Duplicate are not allowed.
		fibers::containers::Map<std::string, Type_Info> 
			m_typenames{}; // Typenames, declared in (and available from) this namespace. Duplicates are not allowed.
		fibers::containers::Map<
			std::string, // Function Name (e.g. string). 
			std::shared_ptr<fibers::containers::Map<
			    Param_Types, // Function parameters (e.g. {string, Any}, or {Any, Any, Any}). 
			    Proxy_Function
			>>
		> m_functions; // functions. (e.g. `==` or `to_string`). Duplicate names are expected. 

	public:
		virtual bool IsClass() const override { return false; };
		virtual bool IsNamespace() const override { return true; };
		virtual const std::string& GetName() const override {
			return p_Name;
		};
		/* Get all named namespaces that are discoverable from the current Scope */
		virtual std::map<std::string, std::weak_ptr<Namespace>> GetAvailableNamespaces(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true) const override {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > inherited > using > parent scopes

			std::map<std::string, std::weak_ptr<Namespace>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetAvailableNamespaces(evaluated, false);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (auto ptr = usingScope.second.lock()) {
					for (auto& using_child : ((Scope*)ptr.get())->p_children) {
						if (using_child.second) {
							// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
								for (auto& using_namespace : ((Scope*)using_child.second.get())->GetAvailableNamespaces(evaluated, false)) {
									out[GetName() + "::" + using_namespace.first] = using_namespace.second;
									if (requestedScope) out[using_namespace.first] = using_namespace.second;
								}
						}
					}
				}
			}

			// get my actual children last...
			for (auto& using_child : this->p_children) {
				if (using_child.second) {
					// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
						for (auto& using_namespace : ((Scope*)using_child.second.get())->GetAvailableNamespaces(evaluated, false)) {
							out[GetName() + "::" + using_namespace.first] = using_namespace.second;
							if (requestedScope) out[using_namespace.first] = using_namespace.second;
						}
				}
			}

			// also remember to add myself (if I am a namespace)...
			if (IsNamespace()) {
				out[GetName()] = std::dynamic_pointer_cast<Namespace>(p_self.lock());
			}

			return out;
		};

		/* Get all namespaces that may be used to search for an object (e.g this scope, it's USING scopes, its PARENT scopes */
		virtual std::set<std::shared_ptr<Scope>> GetScopesForObjectSearch(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true) const override {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > using namespaces

			std::set<std::shared_ptr<Scope>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetScopesForObjectSearch(evaluated, false);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (auto ptr = usingScope.second.lock()) {
					for (auto x : ptr->GetScopesForObjectSearch(evaluated, false)) {
						out.emplace(x);
					}
				}
			}

			// also remember to add myself (if I am a namespace)...
			out.emplace(p_self.lock());

			return out;
		};
	};
	class Class : public Namespace {
	public:
		Class(
			std::weak_ptr<Scope> parent = std::weak_ptr<Scope>(), 
			std::string const& Name = "",
			std::weak_ptr<Class> inheritance = std::weak_ptr<Class>() // e.g. this class derives from another Class
		)
			: Namespace(parent, Name)
			, DerivedFrom(inheritance)
		{
			if (auto ptr = DerivedFrom.lock()) {
				this->AddUsing(ptr);
			}
		};
		Class(Class const&) = default;
		Class(Class&&) = default;
		Class& operator=(Class const&) = default;
		Class& operator=(Class&&) = default;
		virtual ~Class() = default;

	public:
		std::weak_ptr<Class> DerivedFrom; // e.g. this class derives from another Class

	public:
		virtual bool IsClass() const override { return true; };
		virtual bool IsNamespace() const override { return true; };
		virtual const std::string& GetName() const override {
			return this->p_Name;
		};
		
		/* Get all named namespaces that are discoverable from the current Scope */
		virtual std::map<std::string, std::weak_ptr<Namespace>> GetAvailableNamespaces(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true) const override {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > inherited > using > parent scopes

			std::map<std::string, std::weak_ptr<Namespace>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetAvailableNamespaces(evaluated, false);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (auto ptr = usingScope.second.lock()) {
					for (auto& using_child : ((Scope*)ptr.get())->p_children) {
						if (using_child.second) {
							// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
								for (auto& using_namespace : ((Scope*)using_child.second.get())->GetAvailableNamespaces(evaluated, false)) {
									out[GetName() + "::" + using_namespace.first] = using_namespace.second;
									if (requestedScope) out[using_namespace.first] = using_namespace.second;
								}
						}
					}
				}
			}

			// get my inherited children second... 
			if (auto ptr = this->DerivedFrom.lock()) {
				for (auto& using_child : ((Scope*)ptr.get())->p_children) {
					if (using_child.second) {
						// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
							for (auto& using_namespace : ((Scope*)using_child.second.get())->GetAvailableNamespaces(evaluated, false)) {
								out[GetName() + "::" + using_namespace.first] = using_namespace.second;
								if (requestedScope) out[using_namespace.first] = using_namespace.second;
							}
					}
				}
				out[ptr->GetName()] = ptr;
			}

			// get my actual children last...
			for (auto& using_child : this->p_children) {
				if (using_child.second) {
					// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
						for (auto& using_namespace : ((Scope*)using_child.second.get())->GetAvailableNamespaces(evaluated, false)) {
							out[GetName() + "::" + using_namespace.first] = using_namespace.second;
							if (requestedScope) out[using_namespace.first] = using_namespace.second;
						}
				}
			}

			// also remember to add myself (if I am a namespace)...
			if (IsNamespace()) {
				out[GetName()] = std::dynamic_pointer_cast<Namespace>(p_self.lock());
			}

			return out;
		};

		/* Get all namespaces that may be used to search for an object (e.g this scope, it's USING scopes, its PARENT scopes */
		virtual std::set<std::shared_ptr<Scope>> GetScopesForObjectSearch(std::shared_ptr<std::set<std::shared_ptr<Scope>>> evaluated = std::make_shared<std::set<std::shared_ptr<Scope>>>(), bool requestedScope = true) const override {
			const std::string& myScopeName = GetName();

			evaluated->emplace(this->p_self.lock());

			// locally declared namespaces > using namespaces

			std::set<std::shared_ptr<Scope>> out;

			// get my parent first...
			if (auto ptr = p_parent.lock()) {
				if (evaluated->count(ptr) <= 0)
					out = ptr->GetScopesForObjectSearch(evaluated, false);
			}

			// get my "using" children first... 
			for (auto& usingScope : this->p_using) {
				if (auto ptr = usingScope.second.lock()) {
					for (auto& x : ptr->GetScopesForObjectSearch(evaluated, false)) {
						out.emplace(x);
					}
				}
			}

			// get my inherited children second... 
			if (auto ptr = this->DerivedFrom.lock()) {
				for (auto& using_child : ((Scope*)ptr.get())->p_children) {
					if (using_child.second) {
						// if (evaluated->count(std::dynamic_pointer_cast<Scope>(using_child.second)) <= 0)
						for (auto using_namespace : ((Scope*)using_child.second.get())->GetScopesForObjectSearch(evaluated, false)) {
							out.emplace(using_namespace);
						}
					}
				}
				out.emplace(ptr);
			}

			// also remember to add myself (if I am a namespace)...
			out.emplace(p_self.lock());

			return out;
		};
	};




#if 0

#endif

	class Impl {
	private:
		// Attempts to find or make a namespace with the requested namespace name. 
		// If qualified (e.g. starts with "::") then it attempts to find it from the global root -> down. 
		// If not qualified, then it attempts to find it from the current node -> up.
		// If the scope is not found, it will create the namespace as is most appropriate depending on context.
		static std::weak_ptr<Scope> FindOrMakeNamespaceImpl(std::shared_ptr<Scope> scope, std::string const& Namespace) {
			if (!scope) return std::shared_ptr<Scope>{ nullptr };

			std::shared_ptr<Scope> best_match{ nullptr };
			if (scope->TryFindScope(best_match, Namespace)) {
				// we were successful!
				return best_match;
			}
			else {
				// we could not find it
				if (best_match) {
					auto recommendedPlacementNamespace = best_match->GetQualifiedNamespace();
					if ((Namespace.find("::") == 0)) {
						// qualified namespace -- we have to re-navigate the suggestion name and find the deviation
						// e.g. 
						// best_match == "::std::string::"
						// Namespace == "::std::string::impl::"
						// we need to figure out that the new namespace we create should be named "impl"

						// OR

						// e.g. 
						// best_match == "::"
						// Namespace == "::fibers::example::thing::"
						// we need to figure out that the new namespace we create should be named "fibers" and we need to re-submit this function to loop again

						// ::std::string::impl::test:: 
						// w/in recommended placement of 
						// ::std::string::
						// ->
						// impl

						std::string namespace_to_add;
						std::string namespace_without_overlap_and_leading_colons; {
							std::string namespace_without_overlap; {
								int i = 0;
								for (; i < recommendedPlacementNamespace.size() && i < Namespace.size(); i++) {
									if (recommendedPlacementNamespace[i] != Namespace[i]) {
										break;
									}
								}
								namespace_without_overlap = Namespace.substr(i);
							}
							auto namespace_without_overlap_and_leading_colons = namespace_without_overlap.substr(namespace_without_overlap.find_first_not_of("::"));
							namespace_to_add = namespace_without_overlap_and_leading_colons.substr(0, namespace_without_overlap_and_leading_colons.find("::"));
						}

						auto childScope = std::make_shared<scripting::Namespace>(best_match, namespace_to_add);
						best_match->AddChild(childScope);


						if (namespace_without_overlap_and_leading_colons.length() > namespace_to_add.length() + 2) {
							auto remaining_scope = namespace_without_overlap_and_leading_colons.substr(namespace_to_add.size() + 2);
							//std::cout << remaining_scope << std::endl;
							return FindOrMakeNamespace(std::dynamic_pointer_cast<Scope>(childScope), remaining_scope);
						}
						else {
							return std::dynamic_pointer_cast<Scope>(childScope);
						}
					}
					else {
						// non-qualified namespace -- place it literally where it was suggested

						// e.g. 
						// best_match == "::std::string::"
						// Namespace == "impl"
						// we need to figure out that the new namespace we create should be named "impl"

						// OR

						// e.g. 
						// best_match == "::"
						// Namespace == "fibers::example::thing::"
						// we need to figure out that the new namespace we create should be named "fibers" and we need to re-submit this function to loop again

						auto namespace_without_leading_colons = Namespace.substr(Namespace.find_first_not_of("::"));
						auto namespace_first_named = namespace_without_leading_colons.substr(0, namespace_without_leading_colons.find("::"));

						auto childScope = std::make_shared<scripting::Namespace>(best_match, namespace_first_named);
						best_match->AddChild(childScope);

						if (namespace_without_leading_colons.length() > namespace_first_named.length() + 2) {
							auto remaining_scope = namespace_without_leading_colons.substr(namespace_first_named.size() + 2);
							return FindOrMakeNamespace(std::dynamic_pointer_cast<Scope>(childScope), remaining_scope);
						}
						else {
							return std::dynamic_pointer_cast<Scope>(childScope);
						}
					}
				}
				else {
					// and we don't know where to put it!
					return std::shared_ptr<Scope>{ nullptr };
				}
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

	};



























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

	/* Scope, which may be executed and accessed in parallel by downstream children (e.g. for-each-loop children scopes executing in parallel, accessing objects from their shared parent scope)*/
#if 0
	class Scope {
	private:
		concurrency::concurrent_vector<std::string>
			using_namespaces; // e.g. Using Namespace _NAMESPACE_NAME_;

	private:
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
		Scope(State& p_state) 
			: m_state{ p_state } 
			, m_parent{ nullptr }
			, m_name { "" }
			, m_qualified_name{ "" }
		{};
		Scope(Scope* p_parent)
			: m_state{ p_parent->m_state }
			, m_parent { p_parent }
			, m_name{ "" }
			, m_qualified_name{ createFullyQualifiedNamespace(p_parent->m_qualified_name, "") }
		{};
		Scope(Scope* p_parent, std::string const& p_name)
			: m_state{ p_parent->m_state }
			, m_parent{ p_parent }
			, m_name{ p_name }
			, m_qualified_name{ createFullyQualifiedNamespace(p_parent->m_qualified_name, p_name) }
		{};
		Scope(Scope const&) = default;
		Scope(Scope &&) = default;
		Scope& operator=(Scope const&) = default;
		Scope& operator=(Scope&&) = default;
		~Scope() = default;
	
	public:
		State&
			m_state; // shared engine state
		Scope* const
			m_parent; // parent scope
		std::string
			m_name; // namespace (e.g. "::" or "std::" or "details::")
		std::string
			m_qualified_name; // fully qualified namespace (e.g. "::" or "::std::" or "::std::details::")
		fibers::containers::Map<std::string,Any> 
			m_objects{}; // objects. (e.g. "x" or "alpha"). Duplicate names are not allowed.
		fibers::containers::Map<std::string,Type_Info> 
			m_postfixes{}; // allowed postfixes (e.g. 10_ft, where "_ft" is the key) to their desired typename. Duplicate names are not allowed.
		
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
		> 
			m_functions{}; // functions. (e.g. `==` or `to_string`). Duplicate names are expected. 

		Scope CreateChild(std::string const& childName = "") {
			return Scope(this, childName);
		};









	};
#endif

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

