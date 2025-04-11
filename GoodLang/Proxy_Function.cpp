#pragma once
#include "Proxy_Function.h"
// #include <priority_queue>
#include <queue>
#include <map>
#include <array>

// Type_Conversion_Base, its impl's, & TypeConverter wrapper
namespace GoodLang {
	namespace details {
		DaisyChained_Type_Conversion_Impl::DaisyChained_Type_Conversion_Impl(std::vector<GoodLang::shared_ptr<Type_Conversion_Base>>&& t_converters)
			: Type_Conversion_Base()
			, m_converters(std::forward<std::vector<GoodLang::shared_ptr<Type_Conversion_Base>>>(t_converters))
			, m_cost(0)
		{
			this->m_to = m_converters[m_converters.size() - 1]->to();
			this->m_from = m_converters[0]->from();

			for (auto& converter : m_converters) {
				m_cost += converter->cost();
			}
		};
		Any DaisyChained_Type_Conversion_Impl::convert_down(const Any&) const {
			throw std::runtime_error("DaisyChained_Type_Conversion_Impl is not bidirectional.");
		};
		void DaisyChained_Type_Conversion_Impl::convert_in_place(Any& t_from) const {
			switch (m_converters.size()) {
			case 0:
				return;
			case 1:
				m_converters[0]->convert_in_place(t_from);
				return;
			case 2:
				t_from = m_converters[1]->convert(m_converters[0]->convert(t_from));
				return;
			case 3: 
				t_from = m_converters[2]->convert(m_converters[1]->convert(m_converters[0]->convert(t_from)));
				return;
			case 4:
				t_from = m_converters[3]->convert(m_converters[2]->convert(m_converters[1]->convert(m_converters[0]->convert(t_from))));
				return;
			default:
				for (auto& converter : m_converters) {
					if (converter) {
						converter->convert_in_place(t_from);
					} else { throw exception::bad_any_cast(this->from(), this->to(), __LINE__); }
				}
				return;
			}
		};
		Any DaisyChained_Type_Conversion_Impl::convert(const Any& t_from) const {
			switch (m_converters.size()) {
			case 0:
				return {};
			case 1:
				return m_converters[0]->convert(t_from);
			case 2:
				return m_converters[1]->convert(m_converters[0]->convert(t_from));
			case 3:
				return m_converters[2]->convert(m_converters[1]->convert(m_converters[0]->convert(t_from)));
			case 4:
				return m_converters[3]->convert(m_converters[2]->convert(m_converters[1]->convert(m_converters[0]->convert(t_from))));
			default:
				Any out{ t_from };
				for (auto& converter : m_converters) {
					if (converter) {
						converter->convert_in_place(out);
					}
					else { throw exception::bad_any_cast(this->from(), this->to(), __LINE__); }
				}
				return out;
			}
		};
		bool DaisyChained_Type_Conversion_Impl::bidir() const noexcept { return false; }
		double DaisyChained_Type_Conversion_Impl::cost() const noexcept {
			return m_cost;
		};
		std::string DaisyChained_Type_Conversion_Impl::print() const noexcept {
			std::string out;
			for (auto& converter : m_converters) {
				out += converter->print();
			}
			return out;
		};
		bool DaisyChained_Type_Conversion_Impl::IsDaisyChained() const { return true; };
		size_t DaisyChained_Type_Conversion_Impl::NumConversions() const { return m_converters.size(); }
	};

	namespace {
		// shared lock that prioritizes uncontested shared access and uncontested write access. Contested access prioritizes readers, and fairly orders writers.
		class UniformCostSearchNodeBestPath {
		public:
			UniformCostSearchNodeBestPath() = default;
			UniformCostSearchNodeBestPath(UniformCostSearchNodeBestPath* previous, std::weak_ptr<Type_Info> const& nextNodePath)
				:previousBestPath(previous)
				, thisNodePath(nextNodePath)
			{};
			UniformCostSearchNodeBestPath(UniformCostSearchNodeBestPath const&) = default;
			UniformCostSearchNodeBestPath(UniformCostSearchNodeBestPath&&) = default;
			UniformCostSearchNodeBestPath& operator=(UniformCostSearchNodeBestPath const&) = default;
			UniformCostSearchNodeBestPath& operator=(UniformCostSearchNodeBestPath&&) = default;
			~UniformCostSearchNodeBestPath() = default;

			UniformCostSearchNodeBestPath* previousBestPath{ nullptr };
			std::weak_ptr<Type_Info> thisNodePath;
			size_t cached_size{ 0 };

		private:
			void get_impl(std::vector<std::weak_ptr<Type_Info>>& out) const {
				if (previousBestPath) previousBestPath->get_impl(out);				
				out.push_back(thisNodePath);
			};

		public:
			void get(std::vector<std::weak_ptr<Type_Info>>& out) const {
				out.clear();
				get_impl(out);
			};
			size_t size() const {
				if (cached_size == 0) {
					if (previousBestPath) {
						const_cast<size_t&>(cached_size) = 1 + previousBestPath->size();
					}
					else {
						const_cast<size_t&>(cached_size) = 1;
					}
				}
				return cached_size;
			};
		};
		class UniformCostSearchNode {
		public:
			UniformCostSearchNode() = default;
			UniformCostSearchNode(std::shared_ptr<Type_Info> const& a, double b, UniformCostSearchNodeBestPath* c)
				: thisVertexType(a)
				, distanceFromTarget(std::move(b))
				, bestPath(std::move(c))
			{};
			UniformCostSearchNode(UniformCostSearchNode&&) = default;
			UniformCostSearchNode(UniformCostSearchNode const&) = default;
			UniformCostSearchNode& operator=(UniformCostSearchNode&&) = default;
			UniformCostSearchNode& operator=(UniformCostSearchNode const&) = default;
			~UniformCostSearchNode() = default;
		public:
			std::shared_ptr<Type_Info> thisVertexType;
			double distanceFromTarget; // if not known, then we can simply guess. 
			UniformCostSearchNodeBestPath* bestPath{ nullptr };

		public:
			size_t size() const {
				if (bestPath) {
					return bestPath->size();
				}
				else {
					return 0;
				}
			};
			bool operator()(const UniformCostSearchNode* a, const UniformCostSearchNode* b) const {
				return ((a->size() + 1) > (b->size() + 1)) || (a->distanceFromTarget > b->distanceFromTarget);
			};
			bool operator()(const std::shared_ptr<UniformCostSearchNode>& a, const std::shared_ptr<UniformCostSearchNode>& b) const {
				return ((a->size() + 1) > (b->size() + 1)) || (a->distanceFromTarget > b->distanceFromTarget);
			};
		};
	};

	// Tree that manages a complex graph network of conversion opportunities. 
	// It's task is to organize those conversions, find the minimium or best conversion paths, and then cache the results. 
	// Best, most thread-safe use is to pre-populate the tree with converters before use. 
	// Performance-wise, it caches all potential conversions for each new type all at once, so beware small hick-ups in timing due to this. 
	// Can be fixed by pre-fetching all (or most) of the conversions you plan to use. 
	std::string TypeConverter::print() {
		std::string out;
		for (auto& conv : AllConversions) {
			out += (conv.first->name() + " (" + std::to_string(conv.first->GetHash()) + "): \n");
			for (auto& conv2 : conv.second) {
				auto& pair = conv2.second;
				out += (std::string("\t -> ") + 
					conv2.second.first->name() + " (" + 
					std::to_string(conv2.second.first->GetHash()) + ") " + " cost(" +
					std::to_string(pair.second->cost()) + ") path(" + pair.second->print() + ")\n");
			}
		}
		return out;
	};

	bool TypeConverter::ConverterExists(std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To) {
		auto f1 = AllConversions.find(From);
		if (f1 != AllConversions.end()) {
			auto f2 = f1->second.find(GetHash(To));
			if (f2 != f1->second.end()) {
				return true;
			}
		}
		return false;
	};

	// may return nullptr
	TypeConverter::TypeConverterFunc TypeConverter::GetExistingConverter(std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To) {
		auto f1 = AllConversions.find(From);
		if (f1 != AllConversions.end()) {
			auto f2 = f1->second.find(GetHash(To));
			if (f2 != f1->second.end()) {
				auto& pair = f2->second;
				return pair.second;
			}
		}
		return nullptr;
	};
	bool TypeConverter::TryConvertWithExistingConverter(Any& from, std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To) {		
		// straight look-up
		if (true) {
			auto f1 = AllConversions.find(From);
			if (f1 != AllConversions.end()) {
				auto f2 = f1->second.find(GetHash(To));
				if (f2 != f1->second.end()) {
					auto& pair = f2->second;
					pair.second->convert_in_place(from);
					return true;
				}
			}
		}
		// build (if needed) followed by straight look-up
		if (true) {
			EnsureConversionExists(From, To);

			if (true) {
				auto f1 = AllConversions.find(From);
				if (f1 != AllConversions.end()) {
					auto f2 = f1->second.find(GetHash(To));
					if (f2 != f1->second.end()) {
						auto& pair = f2->second;
						pair.second->convert_in_place(from);
						return true;
					}
				}
			}
		}
		return false;		
	};


	void TypeConverter::EnsureConversionExists(std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To, bool forceBuild){
		if (!forceBuild) {
			if (ConverterExists(From, To)) return;
		}

		// Solves the Uniform Cost Search Algorithm to determine the shortest path for "From" to "To", puts the path in "Out", and returns true. 
		// If no path is possible, returns false.
		static auto CreateConversionPaths{ [this](
			utilities::FastAllocator<UniformCostSearchNode, 1024>& alloc,
			utilities::FastAllocator<UniformCostSearchNodeBestPath, 1024>& alloc2,
			conversionTreeType& AllConversions, std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To) {
				// create the shortest paths from "From" to all possible vertices. 
				std::unordered_map <
					size_t
					, std::pair<std::shared_ptr<Type_Info>, UniformCostSearchNode*>
				> vertices;

				if (1) {
					// create an empty vertex set
					std::priority_queue<
						UniformCostSearchNode*
						, std::vector<UniformCostSearchNode*>
						, UniformCostSearchNode
					> vertexSet;

					// Add the source vertex into the set
					vertexSet.push(alloc.Alloc(From, 0.0, nullptr));

					// is the vertex set empty?
					double conversionCost{ 0 };
					conversionTreeType::iterator f;
					UniformCostSearchNode* smallestDistanceNode;
					conversionTreeType::value_type::second_type::const_iterator fSecondIter;
					conversionTreeType::value_type::second_type::const_iterator fSecondEnd;
					while (vertexSet.size() != 0) {
						// extract the vertex with the smallest distance value from the set
						smallestDistanceNode = std::move(vertexSet.top());
						vertexSet.pop();

						// for each neighbor of the extracted vertex... 
						f = AllConversions.find(smallestDistanceNode->thisVertexType);
						if (f != AllConversions.end()) {
							for (fSecondIter = f->second.cbegin(), fSecondEnd = f->second.cend(); fSecondIter != fSecondEnd; ++fSecondIter){ auto& connection = *fSecondIter;
							// for (const auto& connection : f->second) {
								if (fSecondIter->second.second)
									if (!fSecondIter->second.second->IsDaisyChained()) // do not use daisy-chained functions as candidates for new ones, since it can be harder to determine the actual conversion chain length
										conversionCost = fSecondIter->second.second->cost(); // calculate distance value for the neighbor vertex								
								if (1) {
									// Is the neighbor already in the vertex set? 
									auto& toType = connection.second.first;
									auto& toVertex = vertices[connection.first];
									if (!toVertex.first) toVertex.first = toType;

									if (!toVertex.second) { // Instance it before we start working with it on an as-needed basis
										toVertex.second = alloc.Alloc(
											toType,
											std::numeric_limits<double>::infinity(),
											alloc2.Alloc(nullptr, toType)
										);
									}
									if ((toVertex.second->size() + 1) > (smallestDistanceNode->size() + 1)) {
										toVertex.second->distanceFromTarget = (smallestDistanceNode->distanceFromTarget + conversionCost);
										toVertex.second->bestPath = alloc2.Alloc(smallestDistanceNode->bestPath, toVertex.second->thisVertexType);
										vertexSet.push(toVertex.second);
									}
									else if (toVertex.second->distanceFromTarget > (smallestDistanceNode->distanceFromTarget + conversionCost)) {
										toVertex.second->distanceFromTarget = (smallestDistanceNode->distanceFromTarget + conversionCost);
										toVertex.second->bestPath = alloc2.Alloc(smallestDistanceNode->bestPath, toVertex.second->thisVertexType);
										vertexSet.push(toVertex.second);
									}
								}
							}
						}
					}
				}
				return vertices;
			} };

		// Add conversion for From to a large variety of types...
		if (1) {
			utilities::FastAllocator<UniformCostSearchNodeBestPath, 1024> alloc2;
			utilities::FastAllocator< UniformCostSearchNode, 1024> alloc;
			auto conversions{ CreateConversionPaths(alloc, alloc2, AllConversions, From, To) };

			std::deque<std::tuple<std::shared_ptr<Type_Info>, TypeConverterFunc, size_t, double>> toAdd;

			// All of these are for "From"...
			if (1) {
				std::shared_ptr<Type_Info> currentNodeType;
				std::vector<std::weak_ptr<Type_Info>> pathToFollow;
				std::vector<GoodLang::shared_ptr<details::Type_Conversion_Base>> functors;
				TypeConverterFunc newConverter;
				conversionTreeType::iterator p1;
				conversionTreeType::value_type::second_type::iterator p2;
				size_t pathToFollowSize;
				size_t pathToFollowIndex;
				for (auto& conversion : conversions) {
					auto& ToType = conversion.second.first; // To...

					auto& cost = conversion.second.second->distanceFromTarget; // cost
					auto& path = conversion.second.second->bestPath; // conversion path

					if (path) {
						auto pathSize = path->size();
						if (pathSize >= 1) {
							{
								// make new function, get hard lock, insert	
								if (1) {
									// make a new converter function
									newConverter = nullptr; {
										// convert the "type path" to a actual daisy-chains of weak_ptrs to converter functions
										functors.clear(); {
											currentNodeType = From;
											path->get(pathToFollow);
											functors.reserve(pathToFollow.size() + 1);

											pathToFollowSize = pathToFollow.size();
											for (pathToFollowIndex = 0; pathToFollowIndex < pathToFollowSize; ++pathToFollowIndex) {
												p1 = AllConversions.find(currentNodeType);
												if (p1 != AllConversions.end()) {
													p2 = p1->second.find(GetHash(currentNodeType = pathToFollow[pathToFollowIndex].lock()));
													if (p2 != p1->second.end()) {
														if (p2->second.second) { // something went wrong -- this conversion has failed.
															functors.push_back(p2->second.second);
														}
														else {
															currentNodeType = nullptr;
															break;
														}
													}
													else {
														currentNodeType = nullptr;
														break;
													}
												}
												else {
													currentNodeType = nullptr;
													break;
												}
											}

											if (ToType != currentNodeType) {
												// this failed -- unclear why, but it happened. 
												continue;
											}
										}
										if (functors.size() > 1) {
											newConverter = GoodLang::shared_ptr< details::Type_Conversion_Base >(new details::DaisyChained_Type_Conversion_Impl(std::move(functors)));
										}
										else {
											continue; // do nothing, assuming either the conversion failed or the shorter version was obviously already in the list.
										}
									}

									// insert it (requires hard lock)
									if (newConverter) {
										toAdd.push_back(std::tuple<std::shared_ptr<Type_Info>, TypeConverterFunc, size_t, double>(ToType, newConverter, pathSize, cost));
									}
								}
							}
						}
					}
				}
			}

			if (1) {
				auto& FromPair = AllConversions[From];

				while (!toAdd.empty()) {
					auto& toDo = toAdd.front();
					{
						auto& pair = FromPair[GetHash(std::get<0>(toDo))];

						if (!pair.first) pair.first = std::get<0>(toDo);

						if ((pair.second && (pair.second->NumConversions() < std::get<2>(toDo))) || (pair.second && (pair.second->cost() <= std::get<3>(toDo)))) {
						}
						else {
							pair.second = std::get<1>(toDo);
						}

					}
					toAdd.pop_front();
				}
			}
		}
	};

	// may return nullptr if it could not be built
	TypeConverter::TypeConverterFunc TypeConverter::GetOrBuildConverter(std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To, bool forceBuild) {
		EnsureConversionExists(From, To);
		return GetExistingConverter(From, To);
	};

	// Base -> const Base
	// Base -> Base&
	// Base -> const Base&
	// const Base -> const Base&
	// Base& -> const Base&
	void TypeConverter::AddDefaultConverters(std::weak_ptr<Type_Info> const& Type) {
		if (auto ptr = Type.lock()) {
			auto baseType = ptr->MakeBase().lock();
			if (baseType) {
				auto refType = baseType->MakeRef().lock();
				auto constType = baseType->MakeConst().lock();
				if (refType && constType) {
					auto constRefType = constType->MakeRef().lock();
					if (constRefType) {
						bool ExistsAlready = false;

						// Base -> const Base
						{
							auto& pair = AllConversions[baseType][GetHash(constType)];
							if (!pair.first) pair.first = constType;
							ExistsAlready = pair.second.operator bool();
						}
						if (!ExistsAlready) {
							if (auto func = GoodLang::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
								return x;
								}, baseType, constType, 0.0))) {
								auto& pair = AllConversions[func->from().lock()][GetHash(func->to().lock())];
								if (!pair.first) pair.first = func->to().lock();
								pair.second = func;
							}
						}

						// Base -> Base&
						{
							auto& pair = AllConversions[baseType][GetHash(refType)];
							if (!pair.first) pair.first = refType;
							ExistsAlready = pair.second.operator bool();
						}
						if (!ExistsAlready) {
							if (auto func = GoodLang::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
								return x;
								}, baseType, refType, 0.0))) {
								auto& pair = AllConversions[func->from().lock()][GetHash(func->to().lock())];
								if (!pair.first) pair.first = func->to().lock();
								pair.second = func;
							}
						}

						// Base -> const Base&
						{
							auto& pair = AllConversions[baseType][GetHash(constRefType)];
							if (!pair.first) pair.first = constRefType;
							ExistsAlready = pair.second.operator bool();
						}
						if (!ExistsAlready) {
							if (auto func = GoodLang::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
								return x;
								}, baseType, constRefType, 0.0))) {
								auto& pair = AllConversions[func->from().lock()][GetHash(func->to().lock())];
								if (!pair.first) pair.first = func->to().lock();
								pair.second = func;
							}
						}

						// const Base -> const Base&
						{
							auto& pair = AllConversions[constType][GetHash(constRefType)];
							if (!pair.first) pair.first = constRefType;
							ExistsAlready = pair.second.operator bool();
						}
						if (!ExistsAlready) {
							if (auto func = GoodLang::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
								return x;
								}, constType, constRefType, 0.0))) {
								auto& pair = AllConversions[func->from().lock()][GetHash(func->to().lock())];
								if (!pair.first) pair.first = func->to().lock();
								pair.second = func;
							}
						}

						// Base& -> const Base&
						{
							auto& pair = AllConversions[refType][GetHash(constRefType)];
							if (!pair.first) pair.first = constRefType;
							ExistsAlready = pair.second.operator bool();
						}
						if (!ExistsAlready) {
							if (auto func = GoodLang::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([](Any const& x)->Any {
								return x;
								}, refType, constRefType, 0.0))) {
								auto& pair = AllConversions[func->from().lock()][GetHash(func->to().lock())];
								if (!pair.first) pair.first = func->to().lock();
								pair.second = func;
							}
						}

						// const Base& -> Base
						{
							auto& pair = AllConversions[constRefType][GetHash(baseType)];
							if (!pair.first) pair.first = baseType;
							ExistsAlready = pair.second.operator bool();
						}
						if (!ExistsAlready) {
							auto& copyConstructor = baseType->GetCopyConstructor();
							if (auto func = GoodLang::shared_ptr< details::Type_Conversion_Base >(new details::Custom_Type_Conversion_Impl([&copyConstructor](Any const& x)->Any {
								return copyConstructor(x);
								}, constRefType, baseType))) {
								auto& pair = AllConversions[func->from().lock()][GetHash(func->to().lock())];
								if (!pair.first) pair.first = func->to().lock();
								pair.second = func;
							}
						}
					}
				}
			}
		}
	};

	// Find or make converter to accomplish the request
	TypeConverter::TypeConverterFunc TypeConverter::FindConverter(std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To, bool forceBuild) {
		return GetOrBuildConverter(From, To, forceBuild);
	};

	bool TypeConverter::TryDoConversion(Any& From, std::shared_ptr<Type_Info> const& To) {
		return TryConvertWithExistingConverter(From, From.Type().lock(), To);
	};

	// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
	void TypeConverter::Convert_In_Place(Any& from, std::shared_ptr<Type_Info> const& To) {
		if (To && To->is_any()) {}
		else if (from.IsTypeOf(*To)) {}
		else if (TryDoConversion(from, To)) {}
		else {
			throw exception::bad_any_cast(from.Type(), To, __LINE__);
		}
	};
	Any TypeConverter::Convert(Any const& from, std::shared_ptr<Type_Info> const& To) {
		Any out{ from };
		Convert_In_Place(out, To);
		return out; 
	};


	// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
	double TypeConverter::ConversionCost(Any const& from, std::shared_ptr<Type_Info> const& To) {
		if (To && To->is_any()) {
			return 0;
		}
		else if (from.IsTypeOf(To)) {
			return 0;
		}
		else if (auto f = FindConverter(from.Type().lock(), To)) {
			return f->cost();
		}
		return std::numeric_limits<double>::max();		
	};

	// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
	double TypeConverter::ConversionCost_Fast(Any const& from, std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To) {
		if (To && To->is_any()) {
			return 0;
		}
		else if (from.IsTypeOf(To)) {
			return 0;
		}
		else if (auto f = FindConverter(From, To)) {
			return  f->cost();
		}
		return std::numeric_limits<double>::max();		
	};

	// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
	double TypeConverter::ConversionCost_Fast(std::shared_ptr<Type_Info> const& From, std::shared_ptr<Type_Info> const& To) {
		if (To && To->is_any()) {
			return 0;
		}
		else if (GoodLang::GetHash(From) == GoodLang::GetHash(To)) {
			return 0;
		}
		else if (auto f = FindConverter(From, To)) {
			return  f->cost();
		}
		return std::numeric_limits<double>::max();
	};

	// will return an empty object if the conversion was impossible. (Assumes converting to void is not allowed or desired)
	bool TypeConverter::Converts(Any const& from, std::shared_ptr<Type_Info> const& To) {
		return ConversionCost(from, To) != std::numeric_limits<double>::max();
	};

};

// FunctionSignature, FunctionArgs, & ParamTypes
namespace GoodLang {
	size_t ParamTypes::CalculateHash() {
		size_t out{ 37 };
		return out;
	};
	size_t ParamTypes::CalculateHash(std::vector<std::weak_ptr<Type_Info>> const& t_types) {
		size_t out{ 37 };
		for (auto& x : t_types)
			details::hash_combine(out, GetHash(x));
		return out;
	};
	size_t ParamTypes::CalculateHash(std::vector<Any> const& params) {
		size_t out{ 37 };
		for (auto& x : params) {
			// details::hash_combine(out, GetHash(x.Type()));
			details::hash_combine(out, x.TypeHash());
		}
		return out;
	};
	bool ParamTypes::CanCast(ParamTypes const& to) const {
		if (uniquehash == to.uniquehash) { // exact match
			return true;
		}
		else {
			long long i = to.size() - 1;
			if (i < 0) return true;
			if (i >= size()) return false;
			else {
				auto& toVector = *to.m_types;
				auto& fromVector = *m_types;
				for (; i >= 0; i--) {
					if (auto fromType = fromVector[i].lock()) {
						if (auto toType = toVector[i].lock()) {
							if (!fromType->CanCast(*toType)) {
								return false;
							}
						}
					}
				}
				return true;
			}
		}
	};

	std::vector<std::string> FunctionArgs::DefaultVariableNames(size_t n) {
		auto out = std::vector<std::string>(n, "Param");
		for (int i = 0; i < n; i++) {
			out[i].append(std::to_string(i));
		}
		return out;
	};
	std::vector<std::string> FunctionArgs::DefaultVariableNames(size_t n, std::vector<std::string> const& paramNames) {
		auto out = std::vector<std::string>(n, "Param");
		for (int i = 0; i < n; i++) {
			if (paramNames.size() > i) {
				out[i] = paramNames[i];
			}
			else {
				out[i].append(std::to_string(i));
			}
		}
		return out;
	};

	size_t FunctionSignature::CalculateHash(FunctionArgs const& arguments, std::string const& qualified_name) {
		size_t out{ 37 };
		details::hash_combine(out, arguments.hash());
		details::hash_combine(out, std::hash<std::string>()(qualified_name));
		return out;
	};
	size_t FunctionSignature::CalculateHash(ParamTypes const& arguments, std::string const& qualified_name) {
		size_t out{ 37 };
		details::hash_combine(out, arguments.hash());
		details::hash_combine(out, std::hash<std::string>()(qualified_name));
		return out;
	};
};

// Proxy_Function_Base 
namespace GoodLang {
	namespace details {
		double Proxy_Function_Base::conversion_cost_fast(std::vector<std::shared_ptr<Type_Info>> const& t_FromTypes, ParamTypes const& t_to, TypeConverter& t_conversions) {
			double out{ 0 };

			// Quick return if the types exactly match.
			if (t_to.size() > t_FromTypes.size()) return std::numeric_limits<double>::max();
			// if (t_to.hash() == t_from.hash()) { return 0; } // exact match -- no conversions will happen

			size_t i = 0;
			double conversionCost;
			for (; i < t_to.size(); ++i) {
				conversionCost = t_conversions.ConversionCost_Fast(t_FromTypes[i], t_to[i].lock());
				if (conversionCost == std::numeric_limits<double>::max()) {
					return std::numeric_limits<double>::max();
				}
				else {
					out += conversionCost;
				}
			}
			for (; i < t_FromTypes.size(); ++i) {
				out += details::TypeConversionWorstCaseCost; // large penalty for not using the provided type(s).
			}
			return out;
		};
		double Proxy_Function_Base::conversion_cost_fast(std::vector<Any> const& t_from, std::vector<std::shared_ptr<Type_Info>> const& t_FromTypes, ParamTypes const& t_to, TypeConverter& t_conversions) {
			double out{ 0 };

			// Quick return if the types exactly match.
			if (t_to.size() > t_from.size()) return std::numeric_limits<double>::max();
			// if (t_to.hash() == t_from.hash()) { return 0; } // exact match -- no conversions will happen

			size_t i = 0;
			double conversionCost;
			for (; i < t_to.size(); ++i) {
				conversionCost = t_conversions.ConversionCost_Fast(t_from[i], t_FromTypes[i], t_to[i].lock());
				if (conversionCost == std::numeric_limits<double>::max()) {
					return std::numeric_limits<double>::max();
				}
				else {
					out += conversionCost;
				}
			}
			for (; i < t_from.size(); ++i) {
				out += details::TypeConversionWorstCaseCost; // large penalty for not using the provided type(s).
			}
			return out;
		};
		double Proxy_Function_Base::conversion_cost(std::vector<Any> const& t_from, ParamTypes const& t_to, TypeConverter& t_conversions) {
			double out{ 0 };

			// Quick return if the types exactly match.
			if (t_to.size() > t_from.size()) return std::numeric_limits<double>::max();
			// if (t_to.hash() == t_from.hash()) { return 0; } // exact match -- no conversions will happen

			size_t i = 0;
			double conversionCost;
			for (; i < t_to.size(); ++i) {
				conversionCost = t_conversions.ConversionCost(t_from[i], t_to[i].lock());
				if (conversionCost == std::numeric_limits<double>::max()) {
					return std::numeric_limits<double>::max();
				}
				else {
					out += conversionCost;
				}
			}
			for (; i < t_from.size(); ++i) {
				out += details::TypeConversionWorstCaseCost; // large penalty for not using the provided type(s).
			}
			return out;
		};
		std::vector<Any> Proxy_Function_Base::convert(std::vector<Any> const& t_from, ParamTypes const& t_to, TypeConverter& t_conversions) {
			std::vector<Any> out{ t_from };

			if (t_to.size() > t_from.size()) 
				throw exception::arity_error(t_to.size(), t_to.size(), __LINE__);

			out.resize(t_to.size());

			
			for (long long i = t_to.size() - 1; i >= 0; --i) {
				t_conversions.Convert_In_Place(out[i], t_to[i].lock());
			}
			return out;
		};
		std::vector<Any> Proxy_Function_Base::convert(std::vector<Any> const& t_from, ParamTypes const& t_to) {
			if (t_to.size() > t_from.size()) 
				throw exception::arity_error(t_to.size(), t_to.size(), __LINE__);

			std::vector<Any> out{ t_from };
			out.resize(t_to.size());
			return out;
		};
		Any Proxy_Function_Base::convert(Any& t_from, ParamTypes const& t_to) {
			if (t_to.size() > 1) 
				throw exception::arity_error(t_to.size(), t_to.size(), __LINE__);

			return t_from;
		};
		Any Proxy_Function_Base::convert(Any& t_from, ParamTypes const& t_to, TypeConverter& t_conversions) {
			if (t_to.size() > 1) 
				throw exception::arity_error(t_to.size(), 1, __LINE__);

			return t_conversions.Convert(t_from, t_to[0].lock());
		};

		size_t Proxy_Function_Base::hash() const {
			return m_signature.hash();
		};
		const GoodLang::FunctionSignature& Proxy_Function_Base::GetSignature() const {
			return m_signature;
		};
		GoodLang::FunctionSignature& Proxy_Function_Base::GetSignature() {
			return m_signature;
		};
		size_t Proxy_Function_Base::NumArguments() const {
			return m_signature.Arguments().size();
		};

		// Symbolic "cost" to perform the conversion. Not meant to be precise, but meant to be relative for comparison with other converters.
		double Proxy_Function_Base::conversion_cost(std::vector<Any> const& t_params, TypeConverter& t_conversions) const {
			return Proxy_Function_Base::conversion_cost(t_params, Arguments().Types(), t_conversions);
		};
		// Symbolic "cost" to perform the conversion. Not meant to be precise, but meant to be relative for comparison with other converters.
		double Proxy_Function_Base::conversion_cost_fast(std::vector<Any> const& t_params, std::vector<std::shared_ptr<Type_Info>> const& t_FromTypes, TypeConverter& t_conversions) const {
			return Proxy_Function_Base::conversion_cost_fast(t_params, t_FromTypes, Arguments().Types(), t_conversions);
		};
		// Symbolic "cost" to perform the conversion. Not meant to be precise, but meant to be relative for comparison with other converters.
		double Proxy_Function_Base::conversion_cost_fast(std::vector<std::shared_ptr<Type_Info>> const& t_FromTypes, TypeConverter& t_conversions) const {
			return Proxy_Function_Base::conversion_cost_fast(t_FromTypes, Arguments().Types(), t_conversions);
		};

		// Does want conversions -- ensure types match if possible.
		Any Proxy_Function_Base::operator()(const std::vector<Any>& params, TypeConverter& t_conversions) const {
			if (params.size() >= NumArguments()) {
				return this->do_call(convert(params, t_conversions));
			}
			throw exception::arity_error(static_cast<int>(params.size()), NumArguments(), __LINE__);
		};
		// Does want conversions -- ensure types match if possible.
		Any Proxy_Function_Base::operator()(const std::vector<Any>& params) const {
			if (params.size() >= NumArguments()) {
				return this->do_call(params);
			}
			throw exception::arity_error(static_cast<int>(params.size()), NumArguments(), __LINE__);
		};
		// Does want conversions -- ensure types match if possible.
		Any Proxy_Function_Base::operator()(Any& params) const {
			if (1 >= NumArguments()) {
				return this->do_call(params);
			}
			throw exception::arity_error(1, NumArguments(), __LINE__);
		};
		// Does want conversions -- ensure types match if possible.
		Any Proxy_Function_Base::operator()(Any& params, TypeConverter& t_conversions) const {
			if (1 >= NumArguments()) {
				auto conversion{ convert(params, t_conversions) };
				return this->do_call(conversion);
			}
			throw exception::arity_error(1, NumArguments(), __LINE__);
		};

		// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
		std::vector<Any> Proxy_Function_Base::convert(std::vector<Any> const& t_params, TypeConverter& t_conversions) const {
			return Proxy_Function_Base::convert(t_params, m_signature.Arguments().Types(), t_conversions);
		};
		// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
		std::vector<Any> Proxy_Function_Base::convert(std::vector<Any> const& t_params) const {
			return Proxy_Function_Base::convert(t_params, m_signature.Arguments().Types());
		};
		// Performs the conversion from the input parameters to the necessary types, if possible. Throws otherwise. 
		Any Proxy_Function_Base::convert(Any& t_params) const {
			return Proxy_Function_Base::convert(t_params, m_signature.Arguments().Types());
		};
		Any Proxy_Function_Base::convert(Any& t_params, TypeConverter& t_conversions) const {
			return Proxy_Function_Base::convert(t_params, m_signature.Arguments().Types(), t_conversions);
		};
	};
};

// Proxy Function typedef, make_callable(...), and call(...)
namespace GoodLang {
	Any call(Proxy_Function callable, Any& inputs, TypeConverter& conversionTree) {
		if (callable) {
			return callable->operator()(inputs, conversionTree);
		}
		else {
			throw exception::arity_error(1, -1, __LINE__);
		}
	};
	Any call(Proxy_Function callable, std::vector<Any> const& inputs, TypeConverter& conversionTree) {
		if (callable) {
			if (inputs.size() == 1) {
				return callable->operator()(const_cast<Any&>(inputs[0]), conversionTree);
			}
			else {
				return callable->operator()(inputs, conversionTree);
			}
		}
		else {
			throw exception::arity_error(inputs.size(), -1, __LINE__);
		}
	};
};

// "Functions" definitions
namespace GoodLang {
	Functions::FunctionPtr Functions::at_unsafe(std::string const& key, ParamTypes const& params) const {
		static auto hasher{ std::hash<std::string>() };
		static auto hasher2{ std::hash<ParamTypes>() };

		auto functionMapPtr = m_functions.find(hasher(key));
		if (functionMapPtr != m_functions.end()) {
			auto FunctionSortPtr = functionMapPtr->second.second.find(hasher2(params));
			if (FunctionSortPtr != functionMapPtr->second.second.end()) {
				return FunctionSortPtr->second.second;
			}
		}
		return nullptr;
	};
	Functions::FunctionPtr Functions::operator()(std::string const& key, ParamTypes const& params) const {
		// auto locked{ std::shared_lock(m_mut) };
		return at_unsafe(key, params);
	};
	Functions::FunctionPtr Functions::at(std::string const& key, ParamTypes const& params) const {
		return operator()(key, params);
	};

	Functions::FunctionPtr Functions::emplace(std::string const& key, ParamTypes const& params, Function const& func, bool replaceIfAlreadyExists) {
		static auto hasher{ std::hash<std::string>() };
		static auto hasher2{ std::hash<ParamTypes>() };

		// auto locked{ std::unique_lock(m_mut) };
		auto& ptr = m_functions[hasher(key)].second[hasher2(params)].second;
		if (!ptr || (ptr && replaceIfAlreadyExists))
			ptr = GoodLang::make_shared<Function>(func);
		return ptr;
	};
	Functions::FunctionPtr Functions::emplace(std::string const& key, Function const& func, bool replaceIfAlreadyExists) {
		static auto hasher{ std::hash<std::string>() };
		static auto hasher2{ std::hash<ParamTypes>() };

		// auto locked{ std::unique_lock(m_mut) };
		auto& ptr = m_functions[hasher(key)].second[hasher2(func.m_function->Arguments().Types())].second;
		if (!ptr || (ptr && replaceIfAlreadyExists))
			ptr = GoodLang::make_shared<Function>(func);
		return ptr;
	};

	/* Given a function name and call parameters, will attempt to find an exact-match function, variadic instantiation, or convertable function call, or return nullptr. */
	Proxy_Function Functions::BuildMatch(std::string const& functionName, ParamTypes& Params, TypeConverter& m_typeConverters, bool AllowTemplateInstantiation, bool AllowTypeConversion) {
		static auto hasher{ std::hash<std::string>() };
		static auto hasher2{ std::hash<ParamTypes>() };
		if (auto func = at(functionName, Params)) {
			// cache (or actual) found
			if (func->m_function) {
				return func->m_function;
			}
		}
		if (1) {
			// Three sorted groups of candidates. 
			// Group 1 = exact matches, Group 2 = type conversions, Group 3 = template functions
			std::map< size_t, std::array<std::map<double, FunctionPtr, std::less<double>>, 3>, std::greater<size_t>>
				candidates;

			// Create candidates.
			{
				std::vector<std::shared_ptr<Type_Info>> paramTypes;
				for (auto& x : Params) paramTypes.push_back(x.lock());


				// auto locked{ std::shared_lock(m_mut) }; // LOCKED
				auto& m_func_find = m_functions[hasher(functionName)];
				for (auto& function : m_func_find.second) {
					if (!function.second.second) continue;
					if (!function.second.second->m_function) continue;
					if (function.second.second->m_isCached) continue; // ignoring pre-cached functions. Only interested in "true" functions. 
					bool isTemplateFunc = function.second.second->m_function->GetSignature().IsTemplate();
					bool isExplicitFunc = function.second.second->m_isEplicit;

					auto conversionCost = function.second.second->m_function->conversion_cost_fast(paramTypes, m_typeConverters);
					if (conversionCost >= details::TypeConversionWorstCaseCost) continue;

					if (isTemplateFunc) {
						if (AllowTemplateInstantiation) {
							candidates[function.second.second->m_function->NumArguments()][2][conversionCost] = function.second.second;
						}
					}
					else {
						if (conversionCost == 0) {
							candidates[function.second.second->m_function->NumArguments()][0][conversionCost] = function.second.second;
						}
						else if (AllowTypeConversion && !isExplicitFunc) {
							candidates[function.second.second->m_function->NumArguments()][1][conversionCost] = function.second.second;
						}
					}
				}
			}

			// Get the "cheapest" or fastest conversion option available at this scope, with the largest number of arguments, in order of group (e.g. preference).
			for (auto& numParams : candidates) {
				for (auto& preference_order : numParams.second) {
					for (auto& candidate : preference_order) {
						if (candidate.first >= details::TypeConversionWorstCaseCost) continue;
						if (!candidate.second) continue;

						// ParamTypes ParamTypesToCache{ params };
						Function FunctionToCache{ candidate.second->m_function };
						FunctionToCache.m_isCached = true;
						// if someone already beat us to it, it should return the "current" value
						if (auto func = this->emplace(functionName, Params, FunctionToCache, false)) {
							return func->m_function;
						}
					}
				}
			}
		}
		return nullptr;
	};

	/* Given a function name and call parameters, will attempt to find an exact-match function, variadic instantiation, or convertable function call, or return nullptr. */
	Proxy_Function Functions::BuildMatch(std::string const& functionName, std::vector<Any> const& params, ParamTypes const& Params, TypeConverter& m_typeConverters, bool AllowTemplateInstantiation, bool AllowTypeConversion) {
		static auto hasher{ std::hash<std::string>() };
		static auto hasher2{ std::hash<ParamTypes>() };

		if (auto func = at(functionName, Params)) {
			// cache (or actual) found
			if (func->m_function) {
				return func->m_function;
			}
		}
		if (1) {
			// Three sorted groups of candidates. 
			// Group 1 = exact matches, Group 2 = type conversions, Group 3 = template functions
			thread_local static std::map< size_t, std::array<std::map<double, FunctionPtr, std::less<double>>, 3>, std::greater<size_t>>
				candidates{};
			candidates.clear();

			// Create candidates.
			{
				std::vector<std::shared_ptr<Type_Info>> paramTypes;
				for (auto& x : Params) paramTypes.push_back(x.lock());

				// auto locked{ std::shared_lock(m_mut) }; // LOCKED
				auto& m_func_find = m_functions[hasher(functionName)];
				for (auto& function : m_func_find.second) {
					if (!function.second.second) continue;
					if (!function.second.second->m_function) continue;
					if (function.second.second->m_isCached) continue; // ignoring pre-cached functions. Only interested in "true" functions. 
					bool isTemplateFunc = function.second.second->m_function->GetSignature().IsTemplate();
					bool isExplicitFunc = function.second.second->m_isEplicit;

					auto conversionCost = function.second.second->m_function->conversion_cost_fast(params, paramTypes, m_typeConverters);
					if (conversionCost >= details::TypeConversionWorstCaseCost) continue;

					if (isTemplateFunc) {
						if (AllowTemplateInstantiation) {
							candidates[function.second.second->m_function->NumArguments()][2][conversionCost] = function.second.second;
						}
					}
					else {
						if (conversionCost == 0) {
							candidates[function.second.second->m_function->NumArguments()][0][conversionCost] = function.second.second;
						}
						else if (AllowTypeConversion && !isExplicitFunc) {
							candidates[function.second.second->m_function->NumArguments()][1][conversionCost] = function.second.second;
						}
					}
				}
			}

			// Get the "cheapest" or fastest conversion option available at this scope, with the largest number of arguments, in order of group (e.g. preference).
			for (auto& numParams : candidates) {
				for (auto& preference_order : numParams.second) {
					for (auto& candidate : preference_order) {
						if (candidate.first >= details::TypeConversionWorstCaseCost) continue;
						if (!candidate.second) continue;

						// ParamTypes ParamTypesToCache{ params };
						Function FunctionToCache{ candidate.second->m_function };
						FunctionToCache.m_isCached = true;
						// if someone already beat us to it, it should return the "current" value
						if (auto func = this->emplace(functionName, Params, FunctionToCache, false)) {
							return func->m_function;
						}
					}
				}
			}
		}
		return nullptr;
	};

	Any Functions::Call(std::string const& functionName, std::vector<Any> const& params, TypeConverter& m_typeConverters) {
		if (auto f = BuildMatch(functionName, params, ParamTypes(params), m_typeConverters)) {
			return f->operator()(params, m_typeConverters);
		}
		else {
			std::string params_str;
			for (auto& p : params) {
				std::string className = p.TypeName(); {
					//if (auto classPtr = std::dynamic_pointer_cast<Scope2>(this->FindClass(p.Type()))) {
					//	className = classPtr->GetName();
					//}
				}

				if (params_str.empty()) {
					params_str += className;
				}
				else {
					params_str += ", ";
					params_str += className;
				}
			}
			throw exception::not_found_error(GoodLang::printf("`%s`(%s)", functionName.c_str(), params_str.c_str()));
		}
	};

};