#pragma once
#include "Precompiled.h"

/*!
General asset type used throughout the software.
*/
enum class valveType_t {
	PRV = 0, // pressure reducing valve
	PSV = BIT(1), // pressure sustaining valve
	FCV = BIT(2), // flow control valve
	GPV = BIT(3), // general purpose valve
	PBV = BIT(4), // pressure break valve
	TCV = BIT(5) // throttle control valve
};
const static std::map<valveType_t, const char*> StringMap_valveType_t = {
	{valveType_t::PRV, "PRV"},
	{valveType_t::PSV, "PSV"},
	{valveType_t::FCV, "FCV"},
	{valveType_t::GPV, "GPV"},
	{valveType_t::PBV, "PBV"},
	{valveType_t::TCV, "TCV"}
};
template<> const static std::map<valveType_t, const char*>& StaticStringMap< valveType_t >() {
	return StringMap_valveType_t;
};

namespace cweeAsset {
	using namespace cwee_units;
	using squared_ft_p_cfs_t = units::unit_t<units::compound_unit<units::squared<units::length::feet>, units::inverse<units::flowrate::cubic_feet_per_second>>>;
	using cfs_p_ft_t = units::unit_t<units::compound_unit<units::flowrate::cubic_feet_per_second, units::inverse<units::length::feet>>>;
	using squared_cfs_p_ft_t = units::unit_t<units::squared<units::compound_unit<units::flowrate::cubic_feet_per_second, units::inverse<units::length::feet>>>>;
	using ft_per_cfs_t = units::unit_t<units::compound_unit<units::length::feet, units::inverse<units::flowrate::cubic_feet_per_second>>>;

	template<value_t which> struct DefaultUnits {};
	template<> struct DefaultUnits<value_t::_ANY_> { using unit = scalar_t; };
	template<> struct DefaultUnits<value_t::_FLOW_> { using unit = gallon_per_minute_t; };
	template<> struct DefaultUnits<value_t::_VELOCITY_> { using unit = feet_per_second_t; };
	template<> struct DefaultUnits<value_t::_HEADLOSS_> { using unit = foot_t; };
	template<> struct DefaultUnits<value_t::_STATUS_> { using unit = scalar_t; };
	template<> struct DefaultUnits<value_t::_ENERGY_> { using unit = kilowatt_t; };
	template<> struct DefaultUnits<value_t::_ENERGY_PRICE_> { using unit = Dollar_per_kilowatt_hour_t; };
	template<> struct DefaultUnits<value_t::_WATER_PRICE_> { using unit = Dollar_per_gallon_t; };
	template<> struct DefaultUnits<value_t::_COST_> { using unit = Dollar_t; };
	template<> struct DefaultUnits<value_t::_SETTING_> { using unit = scalar_t; };
	template<> struct DefaultUnits<value_t::_HEAD_> { using unit = foot_t; };
	template<> struct DefaultUnits<value_t::_DEMAND_> { using unit = gallon_per_minute_t; };
	template<> struct DefaultUnits<value_t::_QUALITY_> { using unit = scalar_t; };
	template<> struct DefaultUnits<value_t::_MASS_FLOW_> { using unit = scalar_t; };
	template<> struct DefaultUnits<value_t::_TIME_> { using unit = second_t; };
	template<> struct DefaultUnits<value_t::_LEVEL_> { using unit = foot_t; };
	template<> struct DefaultUnits<value_t::_EMISSION_INTENSITY_> { using unit = metric_ton_per_day_t; };

	template<value_t which = _ANY_> static constexpr AUTO DefaultUnit() {
		if constexpr (which == _ANY_) { return scalar_t(); }
		else if constexpr (which == _FLOW_) { return gallon_per_minute_t(); }
		else if constexpr (which == _VELOCITY_) { return feet_per_second_t(); }
		else if constexpr (which == _HEADLOSS_) { return foot_t(); }
		else if constexpr (which == _STATUS_) { return scalar_t(); }
		else if constexpr (which == _ENERGY_) { return kilowatt_t(); }
		else if constexpr (which == _ENERGY_PRICE_) { return Dollar_per_kilowatt_hour_t(); }
		else if constexpr (which == _WATER_PRICE_) { return Dollar_per_gallon_t(); }
		else if constexpr (which == _COST_) { return Dollar_t(); }
		else if constexpr (which == _SETTING_) { return scalar_t(); }
		else if constexpr (which == _HEAD_) { return foot_t(); }
		else if constexpr (which == _DEMAND_) { return gallon_per_minute_t(); }
		else if constexpr (which == _QUALITY_) { return scalar_t(); }
		else if constexpr (which == _MASS_FLOW_) { return scalar_t(); }
		else if constexpr (which == _TIME_) { return second_t(); }
		else if constexpr (which == _LEVEL_) { return foot_t(); }
		else if constexpr (which == _EMISSION_INTENSITY_) { return metric_ton_per_day_t(); }
		else return scalar_t();
	};

	static constexpr std::array < asset_t, 7 > AllAssets{
		asset_t::JUNCTION,
		asset_t::RESERVOIR,
		asset_t::PIPE,
		asset_t::PUMP,
		asset_t::VALVE,
		asset_t::DMA,
		asset_t::PUMPSTATION
	};
	static constexpr std::array < value_t, 14 > AllValues{
		_FLOW_,
		_VELOCITY_,
		_HEADLOSS_,
		_STATUS_,
		_ENERGY_,
		_ENERGY_PRICE_,
		_WATER_PRICE_,
		_COST_,
		_SETTING_,
		_LEVEL_,
		_HEAD_,
		_DEMAND_,
		_QUALITY_,
		_MASS_FLOW_
	};
	static constexpr std::array < value_t, 4 > JunctionValues{
		_HEAD_,
		_DEMAND_,
		_QUALITY_,
		_MASS_FLOW_
	};
	static constexpr std::array < value_t, 5 > ReservoirValues{
		_LEVEL_,
		_HEAD_,
		_DEMAND_,
		_QUALITY_,
		_MASS_FLOW_
	};
	static constexpr std::array < value_t, 4 > PipeValues{
		_FLOW_,
		_VELOCITY_,
		_HEADLOSS_,
		_STATUS_
	};
	static constexpr std::array < value_t, 6 > PumpValues{
		_FLOW_,
		_VELOCITY_,
		_HEADLOSS_,
		_SETTING_,
		_STATUS_,
		_ENERGY_
	};
	static constexpr std::array < value_t, 5 > ValveValues{
		_FLOW_,
		_VELOCITY_,
		_HEADLOSS_,
		_SETTING_,
		_STATUS_
	};
	static constexpr std::array < value_t, 3 > DmaValues{
		_DEMAND_,
		_HEAD_,
		_FLOW_
	};
	static constexpr std::array < value_t, 4 > PumpStationValues{
		_FLOW_,
		_ENERGY_,
		_ENERGY_PRICE_,
		_EMISSION_INTENSITY_
	};
	static constexpr AUTO GetAssets() {
		return AllAssets;
	};
	template <asset_t type = asset_t::ANY> static constexpr AUTO GetValues() {
		if constexpr (type == asset_t::JUNCTION) {
			return JunctionValues;
		}
		else if constexpr (type == asset_t::RESERVOIR) {
			return ReservoirValues;
		}
		else if constexpr (type == asset_t::PIPE) {
			return PipeValues;
		}
		else if constexpr (type == asset_t::PUMP) {
			return PumpValues;
		}
		else if constexpr (type == asset_t::VALVE) {
			return ValveValues;
		}
		else if constexpr (type == asset_t::DMA) {
			return DmaValues;
		}
		else if constexpr (type == asset_t::PUMPSTATION) {
			return PumpStationValues;
		}
		else {
			return AllValues;
		}
	};

#define AddValueType(v) { cweeBalancedPattern<typename DefaultUnits<v>::unit> pat; pat.SetInterpolationType(interpolation_t::IT_LEFT_CLAMP); pat.SetBoundaryType(boundary_t::BT_CLAMPED); Values.Emplace(static_cast<int>(v), pat); }

	template <asset_t type> 
	class cweeAssetValueCollection {
	public:
		cweeAssetValueCollection() : Values() {
			if constexpr (type == asset_t::DMA) {
				AddValueType(_DEMAND_);
				AddValueType(_HEAD_);
				AddValueType(_FLOW_);
			} 
			else if constexpr (type == asset_t::PUMPSTATION) {
				AddValueType(_FLOW_);
				AddValueType(_ENERGY_);
				AddValueType(_ENERGY_PRICE_);
				AddValueType(_EMISSION_INTENSITY_);
			}
			else if constexpr (type == asset_t::JUNCTION || type == asset_t::RESERVOIR) {
				AddValueType(_HEAD_);
				AddValueType(_DEMAND_);
				AddValueType(_QUALITY_);
				AddValueType(_MASS_FLOW_);
			}
			else {
				if constexpr (type == asset_t::PIPE || type == asset_t::PUMP || type == asset_t::VALVE) {
					AddValueType(_FLOW_);
					AddValueType(_VELOCITY_);
					AddValueType(_HEADLOSS_);
					AddValueType(_STATUS_);
				}
				if constexpr (type == asset_t::PUMP || type == asset_t::VALVE) {
					AddValueType(_SETTING_);
				}
				if constexpr (type == asset_t::PUMP) {
					AddValueType(_ENERGY_);
				}
			}
		};
		~cweeAssetValueCollection() {};

		cweeThreadedMap<int, cweeAny>	Values;
	};
	class cweeSystemValueCollection {
	public:
		cweeSystemValueCollection() : Values() {
			AddValueType(_HEAD_);
			AddValueType(_DEMAND_);
			AddValueType(_ENERGY_);
			AddValueType(_ENERGY_PRICE_);
			AddValueType(_EMISSION_INTENSITY_);
		};
		~cweeSystemValueCollection() {};

		cweeThreadedMap<int, cweeAny>	Values;
	};

#undef AddValueType

	class cweeAsset {
	public: // construction or destruction
		cweeAsset() : Type(asset_t::ANY) {};
		cweeAsset(asset_t type_m, cweeThreadedMap<int, cweeAny> const& values) : Type(type_m), Values(values), Name(), Nickname(), Description() {};
		virtual	~cweeAsset() {};

		// data members
	public:
		const asset_t					Type; // type of this asset
		cweeInterlocked<cweeStr>		Name; // name of the asset
		cweeInterlocked<cweeStr>		Nickname; // nickname of the asset
		cweeInterlocked<cweeStr>		Description; // name of the asset

		INLINE static size_t			Hash(const asset_t& type, cweeStr const& UniqueID) { return cweeStr::Hash(cweeStr::printf("%i %s", static_cast<int>(type), UniqueID.c_str())); };
		INLINE size_t					Hash() const { AUTO e = Name.GetExclusive(); return Hash(Type, *e); };
			
		template<value_t v> typename DefaultUnits<v>::unit		GetCurrentValue(u64 t0) const {
			typename DefaultUnits<v>::unit out;
			AUTO p = GetValue<v>();
			if (p) {
				out = p->GetCurrentValue(t0);
			}
			return out;
		};
		template<value_t v> cweeSharedPtr<cweeBalancedPattern<typename DefaultUnits<v>::unit>>		GetValue() const {
			cweeSharedPtr<cweeBalancedPattern<typename DefaultUnits<v>::unit>> out = nullptr;
			AUTO anyP = Values.GetPtr(static_cast<int>(v));
			if (anyP && anyP->IsTypeOf<cweeBalancedPattern<typename DefaultUnits<v>::unit>>()) {
				out = anyP->cast<cweeSharedPtr<cweeBalancedPattern<typename DefaultUnits<v>::unit>>>();
			}			
			return out;
		};

	private:
		cweeThreadedMap<int, cweeAny>	Values; // private access due to the type-erasure
		
	};
};

namespace std {
	template <>
	struct hash<cweeAsset::cweeAsset>
	{
		std::size_t operator()(const cweeAsset::cweeAsset& k) const
		{
			return k.Hash();
		}
	};
}



namespace cweeAsset {
	using namespace cwee_units;

	class cweeNode : public cweeAsset {
	public:
		cweeNode() : cweeAsset(), Coordinates(vec3d(0, 0, 0)) {};
		cweeNode(asset_t type_m, cweeThreadedMap<int, cweeAny> const& values) : cweeAsset(type_m, values), Coordinates(vec3d(0,0,0)), Ke() {};
		virtual ~cweeNode() {};
	public:		
		foot_t	GetElevation() {
			AUTO g = Coordinates.Guard();
			return Coordinates->z;
		};
		u64	GetLongitude() {
			AUTO g = Coordinates.Guard();
			return Coordinates->x;
		};
		u64	GetLatitude() {
			AUTO g = Coordinates.Guard();
			return Coordinates->y;
		};

	public:
		cweeInterlocked<vec3d>			Coordinates; // average coordinates of this asset
		cweeInterlocked<double>			Ke;          // emitter coeff.
	};
	class cweeLink : public cweeAsset {
	public:
		cweeLink() : cweeAsset(), StartingAsset(nullptr), EndingAsset(nullptr), Diameter(inch_t(0)), Length(0), Open(true) {};
		cweeLink(asset_t type_m, cweeThreadedMap<int, cweeAny> const& values) : 
			cweeAsset(type_m, values), 
			StartingAsset(nullptr), 
			EndingAsset(nullptr), 
			Diameter(inch_t(0)), 
			Km_MinorLoss(0),
			Kc_Roughness(0),
			Kb_BulkReactionCoeff(0),
			Kw_WallReactionCoeff(0),
			R_FlowResistance(0),
			Rc_ReactionCoeff(0),
			Open(true) {};
		virtual ~cweeLink() {};
	
		foot_t Calculate_HW_Headloss(gallon_per_minute_t flow) const {
			return Calculate_HW_Headloss(Kc_Roughness.Read(), flow, Diameter.Read(), Length.Read());
		};
		scalar_t Calculate_R_FlowResistance(::epanet::HeadLossType HLT, scalar_t HeadlossExponent) {
			switch (Type) {
				case asset_t::PIPE:		
					R_FlowResistance = Calculate_R_FlowResistance(HLT, Kc_Roughness, Diameter, Length, HeadlossExponent); 
					break;			
				case asset_t::PUMP:		R_FlowResistance = ::epanet::CBIG; break;			
				case asset_t::VALVE:	R_FlowResistance = ::epanet::CSMALL; break;			
			};			
			return R_FlowResistance;
		};
		foot_t Calculate_MinorHeadLoss(cubic_foot_per_second_t Q_flow) {
			return Calculate_MinorHeadLoss(Km_MinorLoss, Diameter, Q_flow);
		};
		foot_t Calculate_FlowHeadloss(::epanet::HeadLossType HLT, cubic_foot_per_second_t Q_flow, scalar_t Viscocity = 1.0) {
			return Calculate_FlowHeadloss(HLT, Kc_Roughness, Diameter, Length, Q_flow, Viscocity);
		};

	public:
		cweeSharedPtr<cweeNode>			StartingAsset;
		cweeSharedPtr<cweeNode>			EndingAsset;
		cweeInterlocked<foot_t>			Diameter;
		cweeInterlocked<foot_t>			Length;
		cweeInterlocked<scalar_t>		Km_MinorLoss; // should be units of ft/(cfs^2)
		cweeInterlocked<scalar_t>		Kc_Roughness;
		cweeInterlocked<scalar_t>		Kb_BulkReactionCoeff;
		cweeInterlocked<scalar_t>		Kw_WallReactionCoeff;
		cweeInterlocked<scalar_t>		R_FlowResistance;
		cweeInterlocked<scalar_t>		Rc_ReactionCoeff;
		cweeInterlocked<bool>			Open;

	private:
		static scalar_t Calculate_R_FlowResistance(::epanet::HeadLossType HLT, scalar_t roughness, foot_t diameter, foot_t Length, scalar_t HeadlossExponent) {
			switch (HLT)
			{
			case ::epanet::HW:
				return 4.727 * (double)Length / std::pow((double)roughness, HeadlossExponent) / std::pow((double)diameter, 4.871);
				break;
			case ::epanet::DW:
				return (double)Length / 2.0 / 32.2 / (double)diameter / ::epanet::SQR(cweeMath::PI * ::epanet::SQR((double)diameter) / 4.0);
				break;
			case ::epanet::CM:
				return ::epanet::SQR(4.0 * (double)roughness / (double)(1.49 * cweeMath::PI * ::epanet::SQR((double)diameter))) * std::pow(((double)diameter / 4.0), -1.333) * (double)Length;
			}
			return 0;
		};
		static foot_t Calculate_HW_Headloss(scalar_t roughness, gallon_per_minute_t flow, inch_t innerDiameter, foot_t pipeLength) {
			return 10.5363277 * pipeLength() * std::pow(flow(), 1.852) * std::pow(roughness(), -1.852) * std::pow(innerDiameter(), -4.8655);
		};
		static constexpr foot_t Calculate_MinorHeadLoss(scalar_t K_minorLossCoeff, foot_t Diameter, cubic_foot_per_second_t Q_flow) {
			constexpr AUTO scale = (8.0 / (cweeMath::PI * cweeMath::PI)) / units::constants::g;
			return K_minorLossCoeff * Q_flow * Q_flow * scale / (Diameter * Diameter * Diameter * Diameter);
		};

		static foot_t Calculate_FlowHeadloss(::epanet::HeadLossType HLT, scalar_t K_roughnessCoefficient, foot_t Diameter, foot_t Length, cubic_foot_per_second_t Q_flow, scalar_t Viscocity = 1.0) {
			switch (HLT) {
			default:
			case ::epanet::HeadLossType::HW: {
				return (double)(Calculate_ResistanceCoefficient_A(HLT, K_roughnessCoefficient, Diameter, Length, Q_flow, Viscocity) * (scalar_t)std::pow((double)Q_flow, 1.852));
				break;
			}
			case ::epanet::HeadLossType::DW: {
				return (double)(Calculate_ResistanceCoefficient_A(HLT, K_roughnessCoefficient, Diameter, Length, Q_flow, Viscocity) * (scalar_t)std::pow((double)Q_flow, 2));
				break;
			}
			case ::epanet::HeadLossType::CM: {
				return (double)(Calculate_ResistanceCoefficient_A(HLT, K_roughnessCoefficient, Diameter, Length, Q_flow, Viscocity) * (scalar_t)std::pow((double)Q_flow, 2));
				break;
			}
			}
			return 0;
		};
		static scalar_t Calculate_ResistanceCoefficient_A(::epanet::HeadLossType HLT, scalar_t K_roughnessCoefficient, foot_t Diameter, foot_t Length, cubic_foot_per_second_t Q_flow, scalar_t Viscocity = 1.0) {
			switch (HLT) {
			default:
			case ::epanet::HeadLossType::HW: {
				return 4.727 * std::pow((double)K_roughnessCoefficient, -1.852) * std::pow((double)Diameter, -4.871) * (double)Length;
				break;
			}
			case ::epanet::HeadLossType::DW: {
				return 0.0252 * Calculate_DW_FrictionFactor(K_roughnessCoefficient, Diameter, Q_flow, Viscocity) * std::pow((double)Diameter, -5) * (double)Length;
				break;
			}
			case ::epanet::HeadLossType::CM: {
				return 4.66 * K_roughnessCoefficient * K_roughnessCoefficient * std::pow((double)Diameter, -5.33) * (double)Length;
				break;
			}
			}
			return 0;
		};
		static scalar_t Calculate_DW_FrictionFactor(scalar_t K_roughnessCoefficient, foot_t Diameter, cubic_foot_per_second_t Q_flow, scalar_t Viscocity = 1.0) {
			scalar_t q = 0, e = 0, s = 0, dfdq = 0.0;

			q = (double)units::math::fabs(Q_flow);						// |flow|
			e = (double)(K_roughnessCoefficient / Diameter);			// Relative roughness
			s = (double)(Diameter * Viscocity);							// Viscosity * diameter

			return ::epanet::frictionFactor(q, e, s, &dfdq);
		};
	};

	class cweeJunction final : public cweeNode {
	public:
		static constexpr int maxNumDemands = 10;
		class Demand {
		public:
			cweeInterlocked<cweeStr> Name;
			cweeSharedPtr<cweeBalancedPattern<gallon_per_minute_t>> Pat;
			cweeInterlocked<scalar_t> Multiplier;
			bool isActive() const {
				if (Pat == nullptr) return false;
				else if (Multiplier.Read()() == 0) return false;
				else return true;
			}
			gallon_per_minute_t DemandAt(cweeTime const& t) const {
				if (Pat) {		
					AUTO f = Pat->GetCurrentValue((u64)t);
					AUTO g = Multiplier.Guard();
					return f * (*Multiplier.UnsafeRead());
				}
				return gallon_per_minute_t(0);
			};
		};
	
		cweeJunction() : cweeNode(asset_t::JUNCTION, cweeAssetValueCollection<asset_t::JUNCTION>().Values), Demands() {};
		~cweeJunction() {};

		bool isActive() const {
			AUTO e = Demands.GetExclusive();
			for (int i = 0; i < e->Num(); i++) {
				if (e->operator[](i).isActive()) return true;
			}
			return false;
		};
		gallon_per_minute_t DemandAt(cweeTime const& t) const {
			gallon_per_minute_t out = gallon_per_minute_t(0); u64 tm = (u64)t;
			AUTO e = Demands.GetExclusive();
			for (int i = 0; i < e->Num(); i++) {
				out += e->operator[](i).DemandAt(tm);
			};
			return out;
		};

	public:
		cweeInterlocked<cweeThreadedList< Demand >> Demands;

	};
	class cweePipe final : public cweeLink {
	public:
		cweePipe() : cweeLink(asset_t::PIPE, cweeAssetValueCollection<asset_t::PIPE>().Values), CheckValve(false), pipeVerticeOffsets() {};
		~cweePipe() {};

	public:
		
		cweeInterlocked<bool>			CheckValve;
		cweeThreadedList<vec3d>			GetCoordinates() const {
			cweeThreadedList<vec3d> out;
			if (this->StartingAsset != nullptr && this->EndingAsset != nullptr) {
				AUTO offsets = pipeVerticeOffsets.GetExclusive();
				out = calculateVerticeCoordinates(*offsets, StartingAsset->Coordinates.Read(), EndingAsset->Coordinates.Read());
			}
			return out;
		};	
		void							SetCoordinates(cweeThreadedList<vec3d> const& verticeCoords) {
			if (this->StartingAsset && this->EndingAsset) {
				pipeVerticeOffsets = calculateVerticeOffsets(verticeCoords, StartingAsset->Coordinates, EndingAsset->Coordinates);
			}
			else {
				pipeVerticeOffsets = calculateVerticeOffsets(verticeCoords);
			}
		};
		void							AppendMiddleVertex(vec2d const& coord) {
			if (this->StartingAsset && this->EndingAsset) {
				AUTO offset = calculateVertexOffset(coord, StartingAsset->Coordinates, EndingAsset->Coordinates);
				pipeVerticeOffsets.GetExclusive()->Append(offset);
			}
		};
		foot_t							CalcLength() const {
			AUTO coords = GetCoordinates();
			if (coords.Num() >= 2) {
				foot_t out = 0;
				vec3d coord = coords[0];
				for (int i = 1; i < coords.Num(); i++) {
					AUTO x = units::math::fabs(geocoding->Distance(coords[i].GetVec2(), coord.GetVec2())); // birds-eye distance
					AUTO y = units::math::fabs(foot_t(coords[i].z - coord.z)); // vertical distance -- assumes the pipe burrows straight along the path towards the new height
					AUTO h = units::math::hypot(x, y); 
					out += h;
				}
				return out;
			}
			else {
				return 0;
			}
		};

	private:
		cweeInterlocked<cweeThreadedList<vec2d>> pipeVerticeOffsets;
		//		During reconstruction, an imaginary line must be drawn between the starting and ending coordinate, AND the line must be of non-zero length. 
		//		The length of this line will be called "Len" and must be determined dynamically. 
		//		The y-component of this offset determines where the point is along the line.
		//		The x-component of this offset determines where the point is along the perpendicular axis to this line. 
		//		In either channel, the magnitude 1 is equal to length "Len". 
		//		Example: Len = 100feet, Vertex = {0,-0.5}
		//			This would place the vertex in-line with the pipe's starting and ending coordinates
		//			It would be 50% the length of Len, behind the starting coordinate. 
		//		{0,0} means the starting coordinate, and {0,1} means the ending coordinate
		static vec3d					calculateVertexCoordinate(const vec2d& vertexOffset, const vec3d& startingCoord, const vec3d& endingCoord) {
			if (vertexOffset.x == 0.0000 && vertexOffset.y == 0.0000)
				return startingCoord; // early exit condition
			if (vertexOffset.x == 0.0000 && vertexOffset.y == 1.0000)
				return endingCoord; // early exit condition

			vec3d out;
			vec2d parallelLine;
			vec2d perpendicularLine;
			float Len;

			parallelLine.x = endingCoord.x - startingCoord.x; parallelLine.y = endingCoord.y - startingCoord.y;
			Len = cweeMath::Sqrt(parallelLine.x * parallelLine.x + parallelLine.y * parallelLine.y);
			// parallelLine.Normalize();

			perpendicularLine.x = -parallelLine.y; perpendicularLine.y = parallelLine.x;
			perpendicularLine.Normalize();

			parallelLine *= (vertexOffset.y); // points from start to end.
			perpendicularLine *= (Len * vertexOffset.x); // points COUNTER CLOCKWISE from parallel. I.e. if parallel is pointing straight down, this points to the right. 

			out.x = startingCoord.x + parallelLine.x + perpendicularLine.x;
			out.y = startingCoord.y + parallelLine.y + perpendicularLine.y;
			out.z = startingCoord.z + vertexOffset.y * (endingCoord.z - startingCoord.z); // assumes elevation is a gradient that extends infinitely in the perpendicular direction. 

			return out;
		};
		static vec2d					calculateVertexOffset(const vec3d& vertexCoord, const vec3d& startingCoord, const vec3d& endingCoord) {
			vec2d out;

			vec3d parallelLine; vec3d dist;
			vec3d perpendicularLine;
			double Len;


			dist.x = vertexCoord.x - startingCoord.x;
			dist.y = vertexCoord.y - startingCoord.y;

			parallelLine.x = endingCoord.x - startingCoord.x; parallelLine.y = endingCoord.y - startingCoord.y;
			perpendicularLine.x = -parallelLine.y; perpendicularLine.y = parallelLine.x;
			Len = ::sqrt(parallelLine.x * parallelLine.x + parallelLine.y * parallelLine.y);

			double lenParallel = dist.ScalarProjectionOnto(parallelLine);
			double lenPerpendicular = dist.ScalarProjectionOnto(perpendicularLine);

			lenParallel /= Len;
			lenPerpendicular /= Len;

			out.x = lenPerpendicular;
			out.y = lenParallel;

			return out;
		};
		static vec2d					calculateVertexOffset(const vec2d& vertexCoord, const vec3d& startingCoord, const vec3d& endingCoord) {
			vec2d out;

			vec3d parallelLine; vec3d dist;
			vec3d perpendicularLine;
			double Len;


			dist.x = vertexCoord.x - startingCoord.x;
			dist.y = vertexCoord.y - startingCoord.y;

			parallelLine.x = endingCoord.x - startingCoord.x; parallelLine.y = endingCoord.y - startingCoord.y;
			perpendicularLine.x = -parallelLine.y; perpendicularLine.y = parallelLine.x;
			Len = ::sqrt(parallelLine.x * parallelLine.x + parallelLine.y * parallelLine.y);

			double lenParallel = dist.ScalarProjectionOnto(parallelLine);
			double lenPerpendicular = dist.ScalarProjectionOnto(perpendicularLine);

			lenParallel /= Len;
			lenPerpendicular /= Len;

			out.x = lenPerpendicular;
			out.y = lenParallel;

			return out;
		};
		static cweeThreadedList<vec3d>	calculateVerticeCoordinates(const cweeThreadedList<vec2d>& verticeOffsets, const vec3d& startingCoord, const vec3d& endingCoord) {
			cweeThreadedList<vec3d> out(verticeOffsets.Num() + 16); {
				out.Append(startingCoord);
				for (auto& x : verticeOffsets) out.Append(calculateVertexCoordinate(x, startingCoord, endingCoord));
				out.Append(endingCoord);
			}
			// remove duplicates in-line. (i.e. [0,0 .. 0,0 .. 0,1] --> [0,0 .. 0,1])
			for (int i = 0; (i < out.Num()) && (i + 1 < out.Num());) {
				if (out[i] == out[i + 1]) {
					out.RemoveIndex(i + 1); // maintains order
				}
				else {
					i++;
				}
			}

			return out;
		};
		static cweeThreadedList<vec2d>	calculateVerticeOffsets(const cweeThreadedList<vec3d>& verticeCoords, const vec3d& startingCoord, const vec3d& endingCoord) {
			cweeThreadedList<vec2d> out(verticeCoords.Num() + 16);
			for (auto& x : verticeCoords) {
				out.Append(calculateVertexOffset(x, startingCoord, endingCoord));
			}
			return out;
		};
		static cweeThreadedList<vec2d>	calculateVerticeOffsets(const cweeThreadedList<vec3d>& verticeCoords) {
			cweeThreadedList<vec2d> out(verticeCoords.Num() + 16);
			if (verticeCoords.Num() >= 2) {
				for (auto& x : verticeCoords) {
					out.Append(calculateVertexOffset(x, verticeCoords[0], verticeCoords[verticeCoords.Num() - 1]));
				}
			}
			return out;
		};
	};
	class cweeReservoir final : public cweeNode {
	public:
		cweeReservoir() : cweeNode(asset_t::RESERVOIR, cweeAssetValueCollection<asset_t::RESERVOIR>().Values), 
			Diameter(0), MinLevel(0), MaxLevel(0), HeadPattern(nullptr), VolumeCurve(nullptr), TerminalStorage(false), CanOverflow(false), 
			MixModel(::epanet::MIX1), MixCompartmentFraction(1.0){};
		~cweeReservoir() {};

		foot_t													MaxHead() const { AUTO e = Coordinates.GetExclusive(); return MaxLevel.Read() + (foot_t)e->z; };
		foot_t													MinHead() const { AUTO e = Coordinates.GetExclusive(); return MinLevel.Read() + (foot_t)e->z; };
		million_gallon_t										MaxVolume() const { return VolumeAt(MaxLevel); };
		million_gallon_t										MinVolume() const { return VolumeAt(MinLevel); };
		million_gallon_t										VolumeAt(foot_t level) const { if (VolumeCurve) { return VolumeCurve->GetCurrentValue(level); } else { return cweeEng::VolumeCylinder_gal(Diameter, level); } };
		foot_t													LvlAtVolume(million_gallon_t v) const {
			foot_t lvl;
			if (VolumeCurve)	lvl = VolumeCurve->GetTimeForValue(v);			
			else				lvl = cweeEng::Cylinder_Volume_to_Level_f(v, Diameter);			
			return lvl;
		};

		cweeInterlocked<foot_t>									Diameter;
		cweeInterlocked<foot_t>									MinLevel;
		cweeInterlocked<foot_t>									MaxLevel;
		cweeSharedPtr<cweeBalancedPattern<foot_t>>				HeadPattern;
		cweeSharedPtr<cweeBalancedPattern<gallon_t, foot_t>>	VolumeCurve; 
		cweeInterlocked<bool>									TerminalStorage;
		cweeInterlocked<bool>									CanOverflow;
		cweeInterlocked<::epanet::MixType>						MixModel;
		cweeInterlocked<scalar_t>								MixCompartmentFraction;
	};
	class cweeValve final : public cweeLink {
	public:
		cweeValve() : cweeLink(asset_t::VALVE, cweeAssetValueCollection<asset_t::VALVE>().Values), valveType(valveType_t::PRV), forceFullyOpen(false) {};
		~cweeValve() {};

	public:
		cweeInterlocked<valveType_t>				valveType;
		cweeInterlocked<bool>						forceFullyOpen;
		cweeSharedPtr<cweeBalancedPattern<foot_t, gallon_per_minute_t>> HeadlossCurve;

		static constexpr cwee_units::power::kilowatt_t	energy_generation_potential(cwee_units::flowrate::gallon_per_minute_t const& flowrate, cwee_units::length::foot_t const& headloss) {
			return cweeEng::CentrifugalPumpEnergyDemand_kW(flowrate, headloss, 131); // assumes ~30% efficiency loss in pump-as-turbine operation, fitted from https://www.renewablesfirst.co.uk/hydropower/hydropower-learning-centre/head-and-flow-detailed-review/

			//using namespace cwee_units;
			//if (flowrate < 0_gpm || headloss < 0_ft) { return 0; }
			//AUTO pump_as_turbine_potential = cweeEng::CentrifugalPumpEnergyDemand_kW(flowrate, headloss, 85); // assumes 85% efficiency
			//cweeThreadedList<cweeUnion<double, double, double>> finalData;
			//{
			//	using thisT = cweeUnion<cwee_units::flowrate::cubic_meter_per_second_t, cwee_units::length::meter_t, cwee_units::power::kilowatt_t>;
			//	cweeThreadedList<thisT> data;
			//	
			//	data.Alloc() = thisT(0, 0, 0);
			//	for (int i = 10; i < 100; i+=10) { data.Alloc() = thisT(0, i, 0); }
			//	for (int i = 1; i < 7; i += 1) { data.Alloc() = thisT(i, 0, 0); }
			//	data.Alloc() = thisT(0.34_cms, 2_m, 5_kW);
			//	data.Alloc() = thisT(0.68_cms, 2_m, 10_kW);
			//	data.Alloc() = thisT(1.699_cms, 2_m, 25_kW);
			//	data.Alloc() = thisT(3.398_cms, 2_m, 50_kW);
			//	data.Alloc() = thisT(6.796_cms, 2_m, 100_kW);
			//	data.Alloc() = thisT(0.136_cms, 5_m, 5_kW);
			//	data.Alloc() = thisT(0.272_cms, 5_m, 10_kW);
			//	data.Alloc() = thisT(0.680_cms, 5_m, 25_kW);
			//	data.Alloc() = thisT(1.359_cms, 5_m, 50_kW);
			//	data.Alloc() = thisT(2.718_cms, 5_m, 100_kW);
			//	data.Alloc() = thisT(0.068_cms, 10_m, 5_kW);
			//	data.Alloc() = thisT(0.136_cms, 10_m, 10_kW);
			//	data.Alloc() = thisT(0.340_cms, 10_m, 25_kW);
			//	data.Alloc() = thisT(0.680_cms, 10_m, 50_kW);
			//	data.Alloc() = thisT(1.359_cms, 10_m, 100_kW);
			//	data.Alloc() = thisT(0.014_cms, 50_m, 5_kW);
			//	data.Alloc() = thisT(0.027_cms, 50_m, 10_kW);
			//	data.Alloc() = thisT(0.070_cms, 50_m, 25_kW);
			//	data.Alloc() = thisT(0.136_cms, 50_m, 50_kW);
			//	data.Alloc() = thisT(0.272_cms, 50_m, 100_kW);
			//	data.Alloc() = thisT(0.006_cms, 100_m, 5_kW);
			//	data.Alloc() = thisT(0.014_cms, 100_m, 10_kW);
			//	data.Alloc() = thisT(0.034_cms, 100_m, 25_kW);
			//	data.Alloc() = thisT(0.068_cms, 100_m, 50_kW);
			//	data.Alloc() = thisT(0.136_cms, 100_m, 100_kW);
			//	for (auto& x : data) {
			//		finalData.Append(cweeUnion<double, double, double>(gallon_per_minute_t(x.get<0>())(), foot_t(x.get<1>())(), x.get<2>()()));
			//	}
			//}						
			//AUTO maximumEnergyPotential = alglibwrapper::Interpolator::SparseMatrixInterp(finalData, flowrate(), headloss());
			//constexpr AUTO pump_as_turbine_potential_2 = cweeEng::CentrifugalPumpEnergyDemand_kW(0.136_cms, 100_m, 131);// *0.76;
			//constexpr cubic_meter_per_second_t cfs = 19500_gpm;
		};

	};
	class cweePump final : public cweeLink {
	public:
		enum class pumpType_t {
			SSD = 0,	// single speed drive pump
			VSD = 1,	// variable speed drive pump
			ERT = 2		// energy recovery turbine
		};

		cweePump() : cweeLink(asset_t::PUMP, cweeAssetValueCollection<asset_t::PUMP>().Values), 
			PumpType(pumpType_t::SSD), 
			HeadCurve(nullptr), 
			EfficiencyCurve(nullptr), 
			AverageEfficiencyPercent(70), 
			Power(-1), 
			HeadCurveMode(::epanet::PumpType::NOCURVE) 
		{};
		~cweePump() {};

	public:
		cweeInterlocked<pumpType_t>											PumpType;
		cweeSharedPtr<cweeBalancedPattern<foot_t, gallon_per_minute_t>>		HeadCurve;	
		cweeSharedPtr<cweeBalancedPattern<scalar_t, gallon_per_minute_t>>	EfficiencyCurve;
		cweeInterlocked<scalar_t>											AverageEfficiencyPercent;
		cweeInterlocked<horsepower_t>										Power;
		cweeInterlocked<::epanet::PumpType>									HeadCurveMode;
		cweeSharedPtr<cweeBalancedPattern<scalar_t>>						PreDeterminedStatusPattern;
		cweeSharedPtr<cweeBalancedPattern<metric_ton_per_hour_t>>			EmissionIntensity;
		cweeSharedPtr<cweeBalancedPattern<Dollar_per_kilowatt_hour_t>>		EnergyPrice;
		cweeInterlocked<scalar_t>											EnergyPriceMultiplier;

		metric_ton_per_hour_t												getEmissionIntensity(const u64& time) {
			if (EmissionIntensity != nullptr) {
				return EmissionIntensity->GetCurrentValue(time);
			}
			return 0;
		};
		Dollar_per_kilowatt_hour_t											getEnergyPrice(const u64& time) {
			if (EnergyPrice != nullptr) {
				return EnergyPrice->GetCurrentValue(time) * EnergyPriceMultiplier.Read();
			}
			return 0;
		};
		kilowatt_t EnergyDemandAtFlow(gallon_per_minute_t gpm_flow) {
			return cweeEng::CentrifugalPumpEnergyDemand_kW(gpm_flow, HeadAtFlow(gpm_flow), EfficiencyAtFlow(gpm_flow));
		};
		foot_t HeadAtFlow(gallon_per_minute_t gpm_flow) {			
			if (HeadCurve != nullptr) {				
				return SamplePumpHeadCurve(*HeadCurve, gpm_flow);
			}
			else {
				return SamplePumpHeadCurve(cweeBalancedPattern<foot_t, gallon_per_minute_t>(), gpm_flow);
			}
		};
		gallon_per_minute_t FlowAtHead(foot_t head) {
			if (HeadCurve != nullptr) {
				return SamplePumpHeadCurve(*HeadCurve, head);
			}
			else {
				return SamplePumpHeadCurve(cweeBalancedPattern<foot_t, gallon_per_minute_t>(), head);
			}
		};

		scalar_t EfficiencyAtFlow(cwee_units::flowrate::gallon_per_minute_t gpm_flow) {
			if (EfficiencyCurve != nullptr) {
				return EfficiencyCurve->GetCurrentValue(gpm_flow());
			}
			else {
				return AverageEfficiencyPercent;
			}
		};
		static foot_t SamplePumpHeadCurve(cweeBalancedPattern<foot_t, gallon_per_minute_t> const& curve, gallon_per_minute_t flow) {
			switch (curve.GetNumValues()) {
			case 0: return foot_t(flow() / 5.0f);
			case 1: {
				cweeBalancedPattern<foot_t, gallon_per_minute_t> newCurve = curve;
				AUTO designHead = newCurve.GetValueAtIndex(0);
				AUTO designFlow = newCurve.GetTimeAtIndex(0);

				newCurve.AddUniqueValue(0, designHead * (scalar_t)1.33f); // epanet approach
				newCurve.AddUniqueValue(designFlow * (scalar_t)2.0f, 0); // epanet approach

				return newCurve.GetCurrentValue(flow()); // catmull_rom spline sample
			}
			case 2: {
				cweeBalancedPattern<foot_t, gallon_per_minute_t> newCurve = curve;

				AUTO designHead = (newCurve.GetValueAtIndex(0) + newCurve.GetValueAtIndex(1)) / (scalar_t)2.0;
				AUTO designFlow = (newCurve.GetTimeAtIndex(0) + newCurve.GetTimeAtIndex(1)) / (scalar_t)2.0;;
				AUTO maxFlow = units::math::fmax(designFlow * (scalar_t)2.0f, units::math::fmax(newCurve.GetTimeAtIndex(0), newCurve.GetTimeAtIndex(1)) * (scalar_t)1.5f);
				AUTO maxHead = units::math::fmax(designHead * (scalar_t)1.33f, units::math::fmax(newCurve.GetValueAtIndex(0), newCurve.GetValueAtIndex(1)) * (scalar_t)1.15f);

				newCurve.AddUniqueValue(0, maxHead);
				newCurve.AddUniqueValue(maxFlow, 0);

				return newCurve.GetCurrentValue(flow()); // catmull_rom spline sample
			}
			default: return curve.GetCurrentValue(flow()); // catmull_rom spline sample		
			}
		};
		static gallon_per_minute_t SamplePumpHeadCurve(cweeBalancedPattern<foot_t, gallon_per_minute_t> const& curve, foot_t head) {
			switch (curve.GetNumValues()) {
			case 0: return gallon_per_minute_t(head() * 5.0f);
			case 1: {
				cweeBalancedPattern<foot_t, gallon_per_minute_t> newCurve = curve;
				AUTO designHead = newCurve.GetValueAtIndex(0);
				AUTO designFlow = newCurve.GetTimeAtIndex(0);

				newCurve.AddUniqueValue(0, designHead * (scalar_t)1.33f); // epanet approach
				newCurve.AddUniqueValue(designFlow * (scalar_t)2.0f, 0); // epanet approach

				return newCurve.GetTimeForValue(head); // catmull_rom spline sample
			}
			case 2: {
				cweeBalancedPattern<foot_t, gallon_per_minute_t> newCurve = curve;

				AUTO designHead = (newCurve.GetValueAtIndex(0) + newCurve.GetValueAtIndex(1)) / (scalar_t)2.0;
				AUTO designFlow = (newCurve.GetTimeAtIndex(0) + newCurve.GetTimeAtIndex(1)) / (scalar_t)2.0;;
				AUTO maxFlow = units::math::fmax(designFlow * (scalar_t)2.0f, units::math::fmax(newCurve.GetTimeAtIndex(0), newCurve.GetTimeAtIndex(1)) * (scalar_t)1.5f);
				AUTO maxHead = units::math::fmax(designHead * (scalar_t)1.33f, units::math::fmax(newCurve.GetValueAtIndex(0), newCurve.GetValueAtIndex(1)) * (scalar_t)1.15f);

				newCurve.AddUniqueValue(0, maxHead);
				newCurve.AddUniqueValue(maxFlow, 0);

				return newCurve.GetTimeForValue(head); // catmull_rom spline sample
			}
			default: return curve.GetTimeForValue(head); // catmull_rom spline sample		
			}
		};
	};
	class cweeDMA final : public cweeAsset {
	public:
		enum class direction_t {
			FLOW_IN_DMA = 0,
			FLOW_OUT_DMA = 1,
			FLOW_WITHIN_DMA = 2
		};

		cweeDMA() : cweeAsset(asset_t::DMA, cweeAssetValueCollection<asset_t::DMA>().Values), Nodes(), Links() {};
		~cweeDMA() {};

	public:
		cweeInterlocked < cweeThreadedList<cweeSharedPtr<cweeNode>> > Nodes;
		cweeInterlocked < cweeThreadedList<cweeUnion<cweeSharedPtr<cweeLink>, direction_t >> > Links;

	public:
		cweeDMA						operator+(const cweeDMA& a) const {
			cweeDMA newOut = *this;
			newOut.Nodes.GetExclusive()->Append(*a.Nodes.GetExclusive());
			newOut.Links.GetExclusive()->Append(*a.Links.GetExclusive());
			return newOut;
		};
		cweeDMA&					operator+=(const cweeDMA& a) {
			Nodes.GetExclusive()->Append(*a.Nodes.GetExclusive());
			Links.GetExclusive()->Append(*a.Links.GetExclusive());
			return *this;
		};
		void						clear(void) {
			Nodes.Clear();
			Links.Clear();
		};

		cweeThreadedList < cweeThreadedList<vec3d> > GetConvexBoundaries(float bias = 2) {
			cweeThreadedList<vec3d> out;
			{
				AUTO g = Nodes.Guard();
				for (auto& x : *Nodes.UnsafeRead()) {
					if (x) {
						out.Append(x->Coordinates);
					}
				}
			}
			return CreateConvexBoundaries(out, bias);
		};

	private:
		/*! Given a collection of points, create multiple polygon outlines that encompass those points. */
		static cweeThreadedList < cweeThreadedList<vec3d> > CreateConvexBoundaries(const cweeThreadedList<vec3d>& points, float setting = 2) {
			cweeThreadedList < cweeThreadedList<vec3d> > out(1000);

			// 1. Get the average distance to nearest neighbor for this point cloud. 
			float averageDistanceToNearestNeighbor = 0; cweeMatX distances; int numPoints;
			{
				numPoints = points.Num();
				if (numPoints < 3)
				{
					out.Append(ClusterToPolygon(points, 50));
					return out; // early exit due to poor conditions
				}

				distances.SetSize(numPoints, numPoints); // perfect rectangle of float. 

				for (int row = 0; row < numPoints; row++) {
					for (int col = 0; col < numPoints; col++) {
						if (col == row) { // identity -- distance would be 0 here. 
							distances[row][col] = 0;
							break; // do not need to cross the identity, since this matrix is symmetrical.
						}
						else {
							distances[row][col] = points[row].Distance2d(points[col]);
							if (distances[row][col] < 50.0) distances[row][col] = 50.0; // ensure that junctions ontop of others don't result in poor distribution
							distances[col][row] = distances[row][col]; // matrix is symmetrical.
						}
					}
				}



				averageDistanceToNearestNeighbor = 0;	int numSamples = 0;
	#if 1
				cweeVecX minDistances;
				minDistances.SetSize(numPoints);
				for (int row = 0; row < numPoints; row++) {
					minDistances[row] = cweeMath::INF;
					for (int col = 0; col < numPoints; col++) {
						if (col == row) continue;
						if (distances[row][col] < minDistances[row] && distances[row][col] > 0.0f) {
							minDistances[row] = distances[row][col];
						}
					}
					cweeMath::rollingAverageRef(averageDistanceToNearestNeighbor, minDistances[row], numSamples);
				}
	#else

				for (int row = 0; row < numPoints; row++) {
					for (int col = 0; col < numPoints; col++) {
						if (col == row) continue;
						if (distances[row][col] > 0.0f) {
							cweeMath::rollingAverageRef(averageDistanceToNearestNeighbor, distances[row][col], numSamples);
						}
					}

				}

	#endif
				if (averageDistanceToNearestNeighbor <= 50.0f) averageDistanceToNearestNeighbor = 50.0f;
				else if (averageDistanceToNearestNeighbor > 1000000000) averageDistanceToNearestNeighbor = 1000000000.0f;

			}


			// 2. Group the point clouds into "Major Clusters" based on this average distance to nearest neighbor
			cweeThreadedList < cweeThreadedList<vec3d> > clusters(1000); // assume?
			float dist = averageDistanceToNearestNeighbor;
	#if 1
			{
				{
					// we already calculated the distance between each point in the point cloud. Use this to create our groups. Start is arbitrary.
					float threshold = cweeMath::Fabs(dist * setting);
					cweeThreadedList<int> cluster(numPoints);
					while (1) {
						cluster.Clear();

						// add the next "remaining" index with distances that are less than inifinity (i.e. not already collected)
						for (int row = 0; row < numPoints; row++) {
							if (distances[row][row] < (cweeMath::INF - 1.0f)) {
								cluster.AddUnique(row);
								break;
							}

							if (cluster.Num() > 0) break; // first point in point cloud just added arbitrarily. 
						}

						int prev_size = 0;
						while (1) {
							for (int i = 0; i < cluster.Num(); i++) {
								for (int col = 0; col < numPoints; col++) {
									// for every row (holding this col as i), if the distance is < threshold, we add it to this list UNIQUELY. 
									if (distances[cluster[i]][col] <= threshold)
										cluster.AddUnique(col); // will always attempt to add itself but fail 
								}
							}
							if (cluster.Num() == prev_size) break;
							else prev_size = cluster.Num();
						}

						if (cluster.Num() > 0) {

							// by the end of this loop we have made ONE cluster. We must now recover the VEC3 locations, and delete them from the original. From here out we must be careful. 
							cweeThreadedList<vec3d> clusterWithLocations(cluster.Num() + 16);
							for (auto& x : cluster) {
								clusterWithLocations.Append(points[x]);

								for (int row = 0; row < numPoints; row++) {
									distances[x][row] = cweeMath::INF;
									distances[row][x] = cweeMath::INF;
								}
							}

							// clusters.Append(clusterWithLocations);
							out.Append(clusterWithLocations);
						}
						else {
							// we stopped making clustersTemp --
							break;
						}
					}
				}

				averageDistanceToNearestNeighbor = dist;
			}

	#else 
			{
				for (dist = averageDistanceToNearestNeighbor; dist >= t; dist *= 0.8f) {
					cweeThreadedList < cweeThreadedList<vec3d> > clustersTemp;
					{
						// we already calculated the distance between each point in the point cloud. Use this to create our groups. Start is arbitrary.
						float threshold = dist * setting;
						cweeThreadedList<int> cluster(numPoints);
						while (1) {
							cluster.Clear();

							// add the next "remaining" index with distances that are less than inifinity (i.e. not already collected)
							for (int row = 0; row < numPoints; row++) {
								if (distances[row][row] < (cweeMath::INF - 1.0f)) {
									cluster.AddUnique(row);
									break;
								}

								if (cluster.Num() > 0) break; // first point in point cloud just added arbitrarily. 
							}

							int prev_size = 0;
							while (1) {
								for (int i = 0; i < cluster.Num(); i++) {
									for (int col = 0; col < numPoints; col++) {
										// for every row (holding this col as i), if the distance is < threshold, we add it to this list UNIQUELY. 
										if (distances[cluster[i]][col] < threshold)
											cluster.AddUnique(col); // will always attempt to add itself but fail 
									}
								}
								if (cluster.Num() == prev_size) break;
								else prev_size = cluster.Num();
							}

							if (cluster.Num() > 0) {

								// by the end of this loop we have made ONE cluster. We must now recover the VEC3 locations, and delete them from the original. From here out we must be careful. 
								cweeThreadedList<vec3d> clusterWithLocations(cluster.Num() + 16);
								for (auto& x : cluster) {
									clusterWithLocations.Append(points[x]);

									for (int row = 0; row < numPoints; row++) {
										distances[x][row] = cweeMath::INF;
										distances[row][x] = cweeMath::INF;
									}
								}

								clustersTemp.Append(clusterWithLocations);
							}
							else {
								// we stopped making clustersTemp --
								break;
							}
						}
					}

					if (clusters.Num() == 0 || (clusters.Num() >= clustersTemp.Num() && clustersTemp.Num() > 0)) {
						clusters = clustersTemp;
					}
					else {
						break;
					}
						}
				averageDistanceToNearestNeighbor = dist;
					}

	#endif

			// 3a. "Clusters" have been determined, now each cluster should be converted to a polygon. 
			if (out.Num() > 0) {
				for (auto& cluster : out) {
					cluster = ClusterToPolygon(cluster, averageDistanceToNearestNeighbor);
				}
			}
			else {
				out.SetGranularity(clusters.Num() + 16);
				for (auto& cluster : clusters) {
					out.Append(ClusterToPolygon(cluster, averageDistanceToNearestNeighbor));
				}
			}


			// 3b. Combine overlapping clusters. 
			out = CombineOverlappingPolygons(out, averageDistanceToNearestNeighbor);

			// 3c. Remove internal spikes.
			RemoveInternalSpikesFromConvexPolygons(out);

			return out;
		};

	private:
		static cweeThreadedList<vec3d> ClusterToPolygon(const cweeThreadedList<vec3d>& cluster, float radius) {
			constexpr int numSidesToCircle = 6;
			cweeThreadedList<vec3d> out(cluster.Num() + 16);

			if (cluster.Num() == 1) {
				out.Append(cweeEng::N_SidedCircle(numSidesToCircle * 1, cluster[0], radius)); // * 5
			}
			else if (cluster.Num() == 2) {
				out.Append(cweeEng::N_SidedCircle(numSidesToCircle * 1, cluster[0], radius)); // * 5
				cweeThreadedList<vec3d> consideration = cweeEng::N_SidedCircle(numSidesToCircle * 1, cluster[1], radius);
				for (int i = consideration.Num() - 1; i >= 0; i--) {
					if (cweeEng::IsPointInPolygon(out, consideration[i])) {
						consideration.RemoveIndexFast(i);
					}
				}
				out.Append(consideration);
				cweeEng::ReorderConvexHull(out);
			}
			else if (cluster.Num() < 50) {
				vec3d middle(0, 0, 0);
				int numSamplesX = 0; int numSamplesY = 0;
				for (auto& point : cluster)
				{
					cweeMath::rollingAverageRef(middle.x, point.x, numSamplesX);
					cweeMath::rollingAverageRef(middle.y, point.y, numSamplesY);
				}

				constexpr int numSidesToTriangle = 3;
				for (auto& x : cluster) {
					out.Append(cweeEng::N_SidedCircle(numSidesToCircle, x, radius));
				}

				// order by the 2-D distance to the center
				cweeCurve<vec3d> dist; dist.SetGranularity(out.Num() + 16);
				for (int i = 0; i < out.Num(); i++) {
					dist.AddValue(out[i].Distance2d(middle), out[i]);
				}

				out.Clear();

				// walk the ordered list backwards (i.e. start the polygon as large as we can make it to early-out as many as possible)
				for (int i = dist.GetNumValues() - 1; i >= 0; i--) {
					if (out.Num() < 3) {
						out.Append(*dist.GetValueAddress(i));
					}
					else if (out.Num() == 3) {
						out.Append(*dist.GetValueAddress(i));
						cweeEng::ReorderConvexHull(out);
					}
					else {
						if (!cweeEng::IsPointInPolygon(out, *dist.GetValueAddress(i))) {
							out.Append(*dist.GetValueAddress(i)); // could not early-out. need to add it to the list and re-order the hull. 
							cweeEng::ReorderConvexHull(out);
						}
					}
				}
				cweeEng::ReorderConvexHull(out);
			}
			else { // there are a very large amount of 'points' in this. 
				vec3d middle(0, 0, 0);
				int numSamplesX = 0; int numSamplesY = 0;
				for (auto& point : cluster)
				{
					cweeMath::rollingAverageRef(middle.x, point.x, numSamplesX);
					cweeMath::rollingAverageRef(middle.y, point.y, numSamplesY);
				}

				vec3d dir;
				double R = radius;
				constexpr int numSidesToTriangle = 3;
				for (auto& x : cluster) {
					dir = x;
					dir -= middle;
					dir.Normalize2D();
					dir.x *= radius;
					dir.y *= radius;
					dir.x += x.x;
					dir.y += x.y;

					out.Append(dir);
					// out.Append(cweeEng::N_SidedCircle(numSidesToTriangle, x, radius));
				}

				// order by the 2-D distance to the center
				cweeCurve<vec3d> dist; dist.SetGranularity(out.Num() + 16);
				for (int i = 0; i < out.Num(); i++) {
					dist.AddValue(out[i].Distance2d(middle), out[i]);
				}

				out.Clear();

				// walk the ordered list backwards (i.e. start the polygon as large as we can make it to early-out as many as possible)
				for (int i = dist.GetNumValues() - 1; i >= 0; i--) {
					if (out.Num() < 3) {
						out.Append(*dist.GetValueAddress(i));
					}
					else if (out.Num() == 3) {
						out.Append(*dist.GetValueAddress(i));
						cweeEng::ReorderConvexHull(out);
					}
					else {
						if (!cweeEng::IsPointInPolygon(out, *dist.GetValueAddress(i))) {
							out.Append(*dist.GetValueAddress(i)); // could not early-out. need to add it to the list and re-order the hull. 
							cweeEng::ReorderConvexHull(out);
						}
					}
				}
				cweeEng::ReorderConvexHull(out);
			}

			return out;
		};
		static cweeThreadedList < cweeThreadedList<vec3d> > CombineOverlappingPolygons(const cweeThreadedList < cweeThreadedList<vec3d> >& clusters, float radius = 50) {

			if (clusters.Num() <= 1)
				return clusters;

			// at least 2 polygons
			cweeThreadedList < cweeThreadedList<vec3d> > out;

			out.Append(clusters[0]);

			cweeThreadedList<int> indexes; for (int i = 1; i < clusters.Num(); i++) indexes.Append(i);

			for (int collapseIndex = 0; collapseIndex < out.Num(); collapseIndex++) { // we are collapsing polygons into THIS if any points are inside of it. 	
				bool anyCombinationHappend = false;
				for (auto& i : indexes) {

					const cweeThreadedList<vec3d>& polygon = clusters[i];
					bool intersect = cweeEng::PolygonsOverlap(out[collapseIndex], polygon);
					if (!intersect) {
						// check that the polygon isn't inside of this polygon, or vice-versa
						for (auto& point : polygon) {
							if (cweeEng::IsPointInPolygon(out[collapseIndex], point)) {
								intersect = true;
								break;
							}
						}
						if (!intersect) {
							for (auto& point : out[collapseIndex]) {
								if (cweeEng::IsPointInPolygon(polygon, point)) {
									intersect = true;
									break;
								}
							}
						}
					}

					if (intersect) {
						// we would have stopped on the polygon with which we intersected.
						out[collapseIndex].Append(polygon);

						out[collapseIndex] = ClusterToPolygon(out[collapseIndex], radius);

						indexes.Remove(i);
						collapseIndex--; // this will be incremented in a moment but we don't want that, so we counter it now. 
						anyCombinationHappend = true;

						break;
					}
				}

				if (!anyCombinationHappend) {
					// we would have gone through all of the polygons to have got here.
					// by definition, if we are in this loop then there is content in the indexes remaining. 
					if (indexes.Num() > 0) {
						out.Append(clusters[indexes[0]]);
						indexes.RemoveIndex(0);
					}
				}
			}

			//for (auto& x : out)
				//cweeEng::ReorderConvexHull(x); // ensure these are drawable

			return out;
		};
		static void RemoveInternalSpikesFromConvexPolygons(cweeThreadedList < cweeThreadedList<vec3d> >& clusters) {
			for (auto& cluster : clusters) {

				vec3d avgPosition; int n = 0;
				for (auto& x : cluster) {
					avgPosition += x;
					n++;
				}
				if (n > 0) avgPosition /= n;

				for (int i = 0; i < cluster.Num(); i++) {
					vec3d* a = &cluster[i];
					vec3d* b = &cluster[i];
					vec3d* c = &cluster[i];

					if ((i - 1) < 0) {
						a = &cluster[cluster.Num() - 1];
					}
					else {
						a = &cluster[i - 1];
					}
					if ((i + 1) >= cluster.Num()) {
						c = &cluster[0];
					}
					else {
						c = &cluster[i + 1];
					}

					vec3d altB((a->x + c->x) / 2.0, (a->y + c->y) / 2.0, (a->z + c->z) / 2.0);

					double dist_real = ::abs(b->Distance2d(avgPosition));
					double dist_alt = ::abs(altB.Distance2d(avgPosition));

					if (dist_alt > dist_real) {
						// we would make the polygon "more" convex by removing this point that is causing the internal spike
						cluster.RemoveIndex(i);
						i--;
					}
				}

				//// loop over each set of three, looking at one further down
				//for (int i = 1; i < (cluster.Num() - 1); i++) {
				//	vec3d& a = cluster[i - 1];
				//	vec3d& b = cluster[i];
				//	vec3d& c = cluster[i + 1];
				//	vec3d altB( (a.x + c.x)/2.0, (a.y + c.y) / 2.0, (a.z + c.z) / 2.0);
				//	double dist_real = ::abs(b.Distance2d(avgPosition));
				//	double dist_alt = ::abs(altB.Distance2d(avgPosition));
				//	if (dist_alt > dist_real) {
				//		// we would make the polygon "more" convex by removing this point that is causing the internal spike
				//		cluster.RemoveIndex(i);
				//		i--;
				//	}
				//}
			}
		};

	};
	class cweePumpStation final : public cweeAsset {
	public:
		cweePumpStation() : cweeAsset(asset_t::PUMPSTATION, cweeAssetValueCollection<asset_t::PUMPSTATION>().Values), upstreamDMA(nullptr), downstreamDMA(nullptr), pumps() {};
		~cweePumpStation() {};

	public:
		cweeSharedPtr<cweeDMA>											upstreamDMA;
		cweeSharedPtr<cweeDMA>											downstreamDMA;
		cweeInterlocked<cweeThreadedList<cweeSharedPtr<cweePump>>>		pumps;

	public:
		cweePumpStation													operator+(const cweePumpStation& a) const {
			cweePumpStation newOut = *this;
			newOut.pumps.GetExclusive()->Append(*a.pumps.GetExclusive());
			return newOut;
		};
		cweePumpStation&												operator+=(const cweePumpStation& a) {
			pumps.GetExclusive()->Append(*a.pumps.GetExclusive());
			return *this;
		};
		void															clear(void) {
			pumps.Clear();
			upstreamDMA = nullptr;
			downstreamDMA = nullptr;
		};
		metric_ton_per_hour_t											getAverageEmissionIntensity(const u64& time) {
			metric_ton_per_hour_t out = 0; int n = 0;
			AUTO e = pumps.GetExclusive();
			for (auto& x : *e) {
				if (x) {
					out += x->getEmissionIntensity(time);
					n++;
				}
			}
			if (n > 0) out = out / scalar_t(n);
			return out;
		};
		Dollar_per_kilowatt_hour_t										getAverageEnergyPrice(const u64& time) {
			Dollar_per_kilowatt_hour_t out = 0; int n = 0;
			AUTO e = pumps.GetExclusive();
			for (auto& x : *e) {
				if (x) {
					out += x->getEnergyPrice(time);
					n++;
				}
			}
			if (n > 0) out = out / scalar_t(n);
			return out;
		};
	};

	template <value_t CharacteristicToObserve> class ObservableCharacteristic {
	public:
		ObservableCharacteristic() : asset(nullptr) {};
		ObservableCharacteristic(cweeSharedPtr<cweeAsset> a) : asset(a) {};

		cweeSharedPtr<cweeAsset> asset;
		cweeBalancedPattern<typename DefaultUnits<CharacteristicToObserve>::unit>* GetPattern() const {
			if (asset) {
				return asset->GetValue<CharacteristicToObserve>();
			}
			return nullptr;
		};
		typename DefaultUnits<CharacteristicToObserve>::unit GetCurrentValue(u64 t) const {
			if (asset) {
				AUTO ptr = asset->GetValue<CharacteristicToObserve>();
				if (ptr) {
					return ptr->GetCurrentValue(t);
				}
			}
			return 0;
		};
		AUTO GetCurrentFirstDerivative(u64 t) const {
			AUTO v = ((typename DefaultUnits<CharacteristicToObserve>::unit)0) / units::time::second_t(1);
			if (asset) {
				AUTO ptr = asset->GetValue<CharacteristicToObserve>();
				if (ptr) {
					return ptr->GetCurrentFirstDerivative(t);
				}
			}
			return v;
		};
		second_t EstimateNextTriggerTime(cweeTime t0, typename DefaultUnits<CharacteristicToObserve>::unit threshold) const {
			second_t out = -1_s;
			if (asset) {
				// SPECIALIZATION FOR TANK && TANK LEVEL OBSERVATION
				if constexpr (CharacteristicToObserve == _LEVEL_ || CharacteristicToObserve == _HEAD_) {
					if (asset->Type == asset_t::RESERVOIR) {
						AUTO res = asset.CastReference<cweeReservoir>();
						if (res && !res->TerminalStorage.Read()) {
							if (CharacteristicToObserve == _HEAD_) threshold -= (foot_t)res->Coordinates.GetExclusive()->z;
							foot_t lvl = res->GetCurrentValue<_HEAD_>((u64)t0) - (foot_t)res->Coordinates.GetExclusive()->z;
							gallon_per_minute_t q = res->GetCurrentValue<_DEMAND_>((u64)t0);
							if (units::math::fabs(q) > (gallon_per_minute_t)::epanet::QZERO) {
								// Find time to reach upper or lower control level
								AUTO unit_per_time = q; // positive means it is filling.
								AUTO rise = res->VolumeAt(threshold) - res->VolumeAt(lvl); // positive means we need the tank to fill.
								second_t run_seconds = rise / unit_per_time;;
								if (run_seconds > 0_s) {
									out = run_seconds;
								}
							}
							return out;
						}
					}
				}
								
				// DEFAULT, GENERIC SOLVER
				if (out < 0_s) {
					AUTO unit_per_time = GetCurrentFirstDerivative((u64)t0);
					AUTO rise = threshold - GetCurrentValue((u64)t0); // positive means the threshold would need a positive rate to ever reach it, and negative would require a negative rate;
					second_t run_seconds = rise / unit_per_time;
					if (run_seconds > 0_s) {
						out = run_seconds;
					}
				}				
			}
			return out;
		};

	};
	class cweeControl {
	public:
		cweeControl() : priority(0), name() {};
		virtual ~cweeControl() {};
		virtual cweeSharedPtr<scalar_t> TryGetNextSetting(u64 t1, u64 t0) const = 0;
		virtual cweeThreadedList<cweeSharedPtr<cweeAsset>> GetObservedAssets() const = 0;
		virtual cweeUnion<bool, cweeTime> EstimateNextTriggerTime(cweeTime t0) const = 0;
		cweeInterlocked< double > priority;
		cweeInterlocked<cweeStr> name;
	};
	template <value_t CharacteristicToObserve> class cweeControl_PID final : public cweeControl {
	public:
		cweeControl_PID() : cweeControl(), ObservableCharacteristic(), SetPoint(60), MinSetting(0), MaxSetting(1), Ku(1) {};
		cweeControl_PID(cweeSharedPtr<cweeAsset> asset) : cweeControl(), ObservableCharacteristic(asset), SetPoint(0), MinSetting(0), MaxSetting(1), Ku(1) {};

		cweeSharedPtr<scalar_t> TryGetNextSetting(u64 t1, u64 t0) const override final {
			cweeSharedPtr<scalar_t> out;

			if (t1 > t0) {
				AUTO pv = observableCharacteristic.GetCurrentValue(); // present value
				cweeEng::pidLogic pid;
				pid.setSettingsRange((float)MaxSetting, (float)MinSetting, (float)Ku);
				out = pid.calculate((float)SetPoint, (float)pv, t1 - t0);
			}

			return out;
		};
		cweeThreadedList<cweeSharedPtr<cweeAsset>> GetObservedAssets() const override final {
			cweeThreadedList<cweeSharedPtr<cweeAsset>> out;
			out.Append(observableCharacteristic.asset);
			return out;
		};
		cweeUnion<bool, cweeTime> EstimateNextTriggerTime(cweeTime t0) const override final {
			cweeUnion<bool, cweeTime> out; out.get<0>() = false;
			second_t nextTrigger = observableCharacteristic.EstimateNextTriggerTime(t0, SetPoint.Read());
			if (nextTrigger > 0_s) {
				out.get<0>() = true;
				out.get<1>() = t0 + (u64)nextTrigger;
			}
			return out;
		};
		ObservableCharacteristic<CharacteristicToObserve>						observableCharacteristic;
		cweeInterlocked<typename DefaultUnits<CharacteristicToObserve>::unit>	SetPoint; // i.e. desired pump flow. 
		cweeInterlocked<typename DefaultUnits<value_t::_SETTING_>::unit>		MinSetting = 0; // i.e. 0%
		cweeInterlocked<typename DefaultUnits<value_t::_SETTING_>::unit>		MaxSetting = 1; // i.e 100%
		cweeInterlocked<typename DefaultUnits<value_t::_SETTING_>::unit>		Ku = 1; // reaction speed
	};
	
	/*! Class that contains the fundamental data to check a (say) reservoir's head >= 155 */
	class RuleCondition {
	public:
		RuleCondition() {};
		virtual cweeSharedPtr<cweeAsset> ObservedAsset() const = 0;
		virtual cweeUnion<bool, cweeTime> EstimateNextTriggerTime(cweeTime t0) const = 0;
		virtual ~RuleCondition() {};
		virtual bool IsTrueAt(u64 t) const = 0;
	};
	class cweeControl_Logic final : public cweeControl {
	private:
		/*!
		Rules engine. Recursive method of evaluating bools.
		*/
		class RulesEngine
		{
		public:
			RulesEngine() : Not(*this) {}

			void If(bool sufficientCondition) { sufficientConditions.push_back(sufficientCondition); }
			void NotIf(bool preventingCondition) { preventingConditions.push_back(preventingCondition); }

			class PreventingRulesEngine
			{
			public:
				explicit PreventingRulesEngine(RulesEngine& rulesEngine) : rulesEngine_(rulesEngine) {}
				void If(bool preventingCondition) { rulesEngine_.NotIf(preventingCondition); }
			private:
				RulesEngine& rulesEngine_;
			};
			PreventingRulesEngine Not;

			bool operator()() const
			{
				auto isTrue = [](bool b) { return b; };
				return std::any_of(begin(sufficientConditions), end(sufficientConditions), isTrue) && std::none_of(begin(preventingConditions), end(preventingConditions), isTrue);
			}

		private:
			std::deque<bool> sufficientConditions;
			std::deque<bool> preventingConditions;
		};

	public:
		template <value_t CharacteristicToObserve> class RuleCondition_Impl final : public RuleCondition { public:
			RuleCondition_Impl() :
				subject(nullptr), operand(Optimization_Comparison::EqualsTo), threshold(0) {};
			RuleCondition_Impl(ObservableCharacteristic<CharacteristicToObserve> the_subject, Optimization_Comparison the_operand, typename DefaultUnits<CharacteristicToObserve>::unit the_threshold) :
				subject(the_subject), operand(the_operand), threshold(the_threshold) {};
			ObservableCharacteristic<CharacteristicToObserve>						subject;		// reservoir head, pump flow, pipe status, time, etc.
			cweeInterlocked < Optimization_Comparison >								operand;		// >=, ==, !=, etc.
			cweeInterlocked<typename DefaultUnits<CharacteristicToObserve>::unit >	threshold;		// 155_ft, 0.0_gpm, 1300 seconds since start of day, etc
			bool IsTrueAt(u64 t) const override final {
				typename DefaultUnits<CharacteristicToObserve>::unit pv;
				if constexpr (CharacteristicToObserve == value_t::_TIME_) {
					units::time::second_t secondsSinceStartOfDay = (u64)(cweeTime(t) - cweeTime(t).ToStartOfDay());
					pv = secondsSinceStartOfDay;
				} 
				else {
					pv = subject.GetCurrentValue(t);
				}

				AUTO th = threshold.Read();
				switch (operand.Read()) {
				case Optimization_Comparison::EqualsTo: { return pv == th; }
				case Optimization_Comparison::GreaterThan: { return pv > th; }
				case Optimization_Comparison::GreaterThanOrEqual: { return pv >= th; }
				case Optimization_Comparison::LessThan: { return pv < th; }
				case Optimization_Comparison::LessThanOrEqual: { return pv <= th; }
				case Optimization_Comparison::NotEqual: { return pv != th; }
				default: return false;
				}
			};
			cweeSharedPtr<cweeAsset> ObservedAsset() const override final {
				return subject.asset;
			};
			cweeUnion<bool, cweeTime> EstimateNextTriggerTime(cweeTime t0) const override final {
				cweeUnion<bool, cweeTime> out; out.get<0>() = false;
				if constexpr (CharacteristicToObserve == value_t::_TIME_) {
					units::time::second_t secondsSinceStartOfDay = (u64)(cweeTime(t0) - cweeTime(t0).ToStartOfDay());
					typename DefaultUnits<CharacteristicToObserve>::unit pv = secondsSinceStartOfDay;
					typename DefaultUnits<CharacteristicToObserve>::unit th = threshold.Read();
					if (th > pv) {
						// upcoming today (i.e. upcoming afternoon)
						second_t nextTrigger = th - pv;
						if (nextTrigger > 0_s) {
							out.get<0>() = true;
							out.get<1>() = t0 + (u64)nextTrigger;
						}
					}
					else {
						//upcoming tomorrow (i.e. next morning)
						second_t nextTrigger = th + (second_t)(u64)(cweeTime(t0).ToEndOfDay() - t0);
						if (nextTrigger > 0_s) {
							out.get<0>() = true;
							out.get<1>() = t0 + (u64)nextTrigger;
						}
					}
				}
				else {
					second_t nextTrigger = subject.EstimateNextTriggerTime(t0, threshold.Read());
					if (nextTrigger > 0_s) {
						out.get<0>() = true;
						out.get<1>() = t0 + (u64)nextTrigger;
					}
				}
				return out;
			};
		};
		
		/*!
		Class that contains the equivalent logic of one EPAnet rule or control, but executed utilizing generic logic.
		Does not impliment the result - simply reports the result. (Can have no result if there is no "if false" condition)
		Based on option selected, will utilize EPAnet or Simulations to retrieve inputs and evaluate logic independently.
		*/
		class cweeRule {
		public: // Public construction / destruction
			cweeRule() {};
			~cweeRule() {};

			/* May return nothing at all or nullptr */
			cweeSharedPtr<typename DefaultUnits<value_t::_SETTING_>::unit> TryGetNextSetting(u64 t) const {
				AUTO e = RuleConditions.GetExclusive();
				if (e->Num() > 0) {
					for (auto& row : *e) {
						auto executeRule = RulesEngine{};
						for (auto& rule : row) {
							executeRule.If(rule->IsTrueAt(t));
						}
						if (executeRule()) {
							return trueAnswer;
						}
					}
					return falseAnswer;
				}
				return nullptr;
			};		
			static Optimization_Comparison GuessOperandFromString(const cweeStr& input_string) {
				Optimization_Comparison out = Optimization_Comparison::EqualsTo;

				cweeStr comp = input_string;
				comp.Strip(' ');
				cweeThreadedList<cweeStr> options; options = std::vector<cweeStr>({
					"=", "==", "<>", "!=", ">", "<", "<=",  ">=", "IS", "Is", "is", "at", "AT", "At", "aT", "BELOW", "Below", "below", "ABOVE", "Above", "above", "EQUALS", "Equals", "equals"
					});
				comp = comp.BestMatch(options);

				if (comp == "=" || comp == "==" || comp.Icmp("IS") == 0 || comp.Icmp("AT") == 0 || comp.Icmp("Equals") == 0) {
					out = Optimization_Comparison::EqualsTo;
				}
				else if (comp == "<>" || comp == "!=") {
					out = Optimization_Comparison::NotEqual;
				}
				else if (comp == ">" || comp.Icmp("ABOVE") == 0) {
					out = Optimization_Comparison::GreaterThan;
				}
				else if (comp == "<" || comp.Icmp("BELOW") == 0) {
					out = Optimization_Comparison::LessThan;
				}
				else if (comp == "<=") {
					out = Optimization_Comparison::LessThanOrEqual;
				}
				else if (comp == ">=") {
					out = Optimization_Comparison::GreaterThanOrEqual;
				}
				else out = Optimization_Comparison::EqualsTo;

				return out;
			};
			template <value_t CharacteristicToObserve> static cweeSharedPtr<RuleCondition> MakeCondition(cweeSharedPtr<cweeAsset> the_subject, Optimization_Comparison the_operand, typename DefaultUnits<CharacteristicToObserve>::unit the_threshold) {
				return make_cwee_shared<RuleCondition_Impl<CharacteristicToObserve>>(ObservableCharacteristic<CharacteristicToObserve>(the_subject), the_operand, the_threshold).CastReference<RuleCondition>();
			};
			template <value_t CharacteristicToObserve> static void AppendSequenceOfAnds(cweeSharedPtr<cweeAsset> the_subject, Optimization_Comparison the_operand, typename DefaultUnits<CharacteristicToObserve>::unit the_threshold, cweeThreadedList<cweeSharedPtr<RuleCondition>>& SequenceOfAnds) {
				SequenceOfAnds.Append(MakeCondition< CharacteristicToObserve>(the_subject, the_operand, the_threshold));
			};
			template <value_t CharacteristicToObserve> void MakeIntoBasicLogic(cweeSharedPtr<cweeAsset> the_subject, Optimization_Comparison the_operand, typename DefaultUnits<CharacteristicToObserve>::unit the_threshold) {
				RuleConditions = cweeThreadedList<cweeThreadedList<cweeSharedPtr<RuleCondition>>>();
				AUTO ands = RuleConditions.GetExclusive()->Alloc();
				ands.Append(MakeCondition< CharacteristicToObserve>(the_subject, the_operand, the_threshold));
			};


		public:
			/*!
			List(rows) < List(columns) > RuleConditions.
			All columns must be true for a row to be true.
			If any row is true, the entire rule returns true.
			If no row is true, the entire rule returns false.

			Example:
				IF RESERVOIR 5's HEAD >= 155			// Row 1, Column 1
				AND PUMP 33's FLOW <= 0					// Row 1, Column 2
				OR RESERVOIR 5's HEAD < 5				// Row 2, Column 1
				AND CLOCKTIME > 5:00					// Row 2, Column 2
				OR RESERVOIR 5's HEAD >= 20				// Row 3, Column 1
				AND CLOCKTIME < 23:00					// Row 3, Column 2
				AND CLOCKTIME >= 1:00					// Row 3, Column 3
				THEN (owner from cweeControl) is OPEN	// hasTrueAnswer = true, trueAnswer = 1
				ELSE (owner from cweeControl) is CLOSED // hasFalseAnswer = true, falseAnswer = 0
			*/
			cweeInterlocked < cweeThreadedList<cweeThreadedList<cweeSharedPtr<RuleCondition>>> > RuleConditions;
			/* Optional return value if any row returns true */
			cweeSharedPtr<typename DefaultUnits<value_t::_SETTING_>::unit> trueAnswer;
			/* Optional return value if all rows return false */
			cweeSharedPtr<typename DefaultUnits<value_t::_SETTING_>::unit> falseAnswer;
		};

	public:
		cweeRule Rule;

		cweeControl_Logic() : cweeControl(), Rule() {};

		cweeSharedPtr<scalar_t> TryGetNextSetting(u64 t1, u64 t0) const override final {
			cweeSharedPtr<scalar_t> out = Rule.TryGetNextSetting(t1);
			return out;
		};
		cweeThreadedList<cweeSharedPtr<cweeAsset>> GetObservedAssets() const override final {
			cweeThreadedList<cweeSharedPtr<cweeAsset>> out;
			for (auto& x : *Rule.RuleConditions.GetExclusive()) {
				for (auto& y : x) {
					if (y) {
						out.AddUnique(y->ObservedAsset());
					}
				}
			}
			return out;
		};
		cweeUnion<bool, cweeTime> EstimateNextTriggerTime(cweeTime t0) const override final {
			cweeUnion<bool, cweeTime> out; out.get<0>() = false;
			for (auto& x : *Rule.RuleConditions.GetExclusive()) {
				for (auto& y : x) {
					if (y) {
						AUTO t = y->EstimateNextTriggerTime(t0);
						if (t.get<0>() && t.get<1>() > 0) {
							out.get<0>() = true;
							out.get<1>() = out.get<1>() < t.get<1>() ? out.get<1>() : t.get<1>();
						}
					}
				}
			}
			return out;
		};
	};
	class AssetController {
	public:
		cweeSharedPtr< cweeLink >		Parent;
		cweeInterlocked < cweeThreadedList<cweeSharedPtr< cweeControl >>>	StatusControllers;
		cweeInterlocked < cweeThreadedList<cweeSharedPtr< cweeControl >>>	SettingControllers;

		cweeSharedPtr<scalar_t> TryGetNextStatus(u64 t1, u64 t0) const {			
			AUTO e = StatusControllers.GetExclusive();
			for (auto& x : *e) {
				if (x) {
					AUTO p = x->TryGetNextSetting(t1, t0);
					if (p) {
						return p;
					}
				}
			}
			return nullptr;
		};
		cweeSharedPtr<scalar_t> TryGetNextSetting(u64 t1, u64 t0) const {
			AUTO e = SettingControllers.GetExclusive();
			for (auto& x : *e) {
				if (x) {
					AUTO p = x->TryGetNextSetting(t1, t0);
					if (p) {
						return p;
					}
				}
			}
			return nullptr;
		};
	};

	class Sadjlist {
	public:
		cweeSharedPtr<cweeNode>		node; // connecting node
		// cweeSharedPtr<cweeLink>		link; // connected link
		int							link_index; // connected link or degrees
		cweeSharedPtr<Sadjlist>		next; // next adjacency item
	};

	class cweeHydraulicNetwork {
	public:
		cweeHydraulicNetwork() :
			systemwide(new cweeAsset(asset_t::ANY, cweeSystemValueCollection().Values)),
			assets(std::function<size_t(cweeSharedPtr<cweeAsset> const&)>([](const cweeSharedPtr<cweeAsset>& v)->size_t { if (v) { return v->Hash(); } return 0; })),
			nodes(std::function<size_t(cweeSharedPtr<cweeNode> const&)>([](const cweeSharedPtr<cweeNode>& v)->size_t { if (v) { return v->Hash(); } return 0; })),
			links(std::function<size_t(cweeSharedPtr<cweeLink> const&)>([](const cweeSharedPtr<cweeLink>& v)->size_t { if (v) { return v->Hash(); } return 0; })),
			junctions(std::function<size_t(cweeJunction const&)>([](const cweeJunction& v)->size_t { if (1) { return v.Hash(); } return 0; })),
			reservoirs(std::function<size_t(cweeReservoir const&)>([](const cweeReservoir& v)->size_t { if (1) { return v.Hash(); } return 0; })),
			pipes(std::function<size_t(cweePipe const&)>([](const cweePipe& v)->size_t { if (1) { return v.Hash(); } return 0; })),
			valves(std::function<size_t(cweeValve const&)>([](const cweeValve& v)->size_t { if (1) { return v.Hash(); } return 0; })),
			pumps(std::function<size_t(cweePump const&)>([](const cweePump& v)->size_t { if (1) { return v.Hash(); } return 0; })),
			DMAs(std::function<size_t(cweeDMA const&)>([](const cweeDMA& v)->size_t { if (1) { return v.Hash(); } return 0; })),
			pumpStations(std::function<size_t(cweePumpStation const&)>([](const cweePumpStation& v)->size_t { if (1) { return v.Hash(); } return 0; })),
			// Adjlist(), 
			Adjlist(std::function<size_t(cweeUnion< cweeSharedPtr<cweeNode>, cweeSharedPtr<Sadjlist> > const&)>([](const cweeUnion< cweeSharedPtr<cweeNode>, cweeSharedPtr<Sadjlist> >& v)->size_t { if (v.get<0>()) return v.get<0>()->Hash(); else return 0; })),
			Patterns(std::function<size_t(cweeUnion< cweeStr, cweeAny > const&)>([](const cweeUnion< cweeStr, cweeAny >& v)->size_t { return cweeStr::Hash(v.get<0>()); })),
			Controllers(std::function<size_t(AssetController const&)>([](const AssetController& v)->size_t { if (v.Parent) return v.Parent->Hash(); else return 0; }))
		{};

		cweeSharedPtr<cweeAsset> systemwide;
		cweeThreadedSet< cweeSharedPtr<cweeAsset>, size_t> assets;
		cweeThreadedSet< cweeSharedPtr<cweeNode>, size_t> nodes;
		cweeThreadedSet< cweeSharedPtr<cweeLink>, size_t> links;
		cweeThreadedSet< cweeJunction, size_t> junctions; 
		cweeThreadedSet< cweeReservoir, size_t> reservoirs; 
		cweeThreadedSet< cweePipe, size_t> pipes; 
		cweeThreadedSet< cweeValve, size_t> valves; 
		cweeThreadedSet< cweePump, size_t> pumps; 
		cweeThreadedSet< cweeDMA, size_t> DMAs;
		cweeThreadedSet< cweePumpStation, size_t> pumpStations;
		// cweeInterlocked< cweeThreadedList<cweeSharedPtr<Sadjlist>> > Adjlist; // adjacency list -- used by EPAnet to keep track of connections
		cweeThreadedSet< cweeUnion< cweeSharedPtr<cweeNode>, cweeSharedPtr<Sadjlist> >, size_t > Adjlist; // adjacency list -- used by EPAnet to keep track of connections
		cweeThreadedSet< cweeUnion< cweeStr, cweeAny >, size_t > Patterns;
		cweeThreadedSet< AssetController, size_t > Controllers;


		template<asset_t type> AUTO Alloc(cweeStr const& name = "") {
			if constexpr (type == asset_t::JUNCTION) {
				AUTO p = cweeJunction(); p.Name = name;
				return Append(p);
			}
			else if constexpr (type == asset_t::RESERVOIR) {
				AUTO p = cweeReservoir(); p.Name = name;
				return Append(p); 
			}
			else if constexpr (type == asset_t::PIPE) {
				AUTO p = cweePipe(); p.Name = name;
				return Append(p);
			}
			else if constexpr (type == asset_t::PUMP) {
				AUTO p = cweePump(); p.Name = name;
				return Append(p);
			}
			else if constexpr (type == asset_t::VALVE) {
				AUTO p = cweeValve(); p.Name = name;
				return Append(p);
			}
			else if constexpr (type == asset_t::DMA) {
				AUTO p = cweeDMA(); p.Name = name;
				return Append(p);
			}
			else if constexpr (type == asset_t::PUMPSTATION) {
				AUTO p = cweePumpStation(); p.Name = name;
				return Append(p);
			}
			else {
				static_assert(false, "Cannot alloc any other types that those instantiated here.");
				return 0;
			}
		};

		AUTO Append(cweeJunction const& asset) {
			AUTO p = junctions.Add(asset);
			nodes.Add(p.CastReference<cweeNode>());
			assets.Add(p.CastReference<cweeAsset>());
			return p;
		};
		AUTO Append(cweePipe const& asset) {
			AUTO p = pipes.Add(asset);
			links.Add(p.CastReference<cweeLink>());
			assets.Add(p.CastReference<cweeAsset>());
			return p;
		};
		AUTO Append(cweeReservoir const& asset) {
			AUTO p = reservoirs.Add(asset);
			nodes.Add(p.CastReference<cweeNode>());
			assets.Add(p.CastReference<cweeAsset>());
			return p;
		};
		AUTO Append(cweeValve const& asset) {
			AUTO p = valves.Add(asset);
			links.Add(p.CastReference<cweeLink>());
			assets.Add(p.CastReference<cweeAsset>());
			return p;
		};
		AUTO Append(cweePump const& asset) {
			AUTO p = pumps.Add(asset);
			links.Add(p.CastReference<cweeLink>());
			assets.Add(p.CastReference<cweeAsset>());
			return p;
		};
		AUTO Append(cweeDMA const& asset) {
			AUTO p = DMAs.Add(asset);
			assets.Add(p.CastReference<cweeAsset>());
			return p;
		};
		AUTO Append(cweePumpStation const& asset) {
			AUTO p = pumpStations.Add(asset);
			assets.Add(p.CastReference<cweeAsset>());
			return p;
		};

		AUTO Find(asset_t type, cweeStr const& name) const {
			cweeSharedPtr<cweeAsset> toReturn;
			AUTO p = assets.Find(cweeAsset::Hash(type, name));
			if (p) {
				toReturn = *p;
			}
			return toReturn;
		};
		cweeSharedPtr<cweeNode> FindNode(asset_t type, cweeStr const& name) const {
			cweeSharedPtr<cweeNode> toReturn;
			AUTO p = nodes.Find(cweeAsset::Hash(type, name));
			if (p) {
				toReturn = *p;
			}
			return toReturn;
		};
		cweeSharedPtr<cweeLink> FindLink(asset_t type, cweeStr const& name) const {
			cweeSharedPtr<cweeLink> toReturn;
			AUTO p = links.Find(cweeAsset::Hash(type, name));
			if (p) {
				toReturn = *p;
			}
			return toReturn;
		};
		cweeSharedPtr<cweeNode> FindNode(cweeStr const& name) const {
			{
				AUTO f1 = FindNode(asset_t::JUNCTION, name);
				if (f1) return *f1;
			}
			{
				AUTO f1 = FindNode(asset_t::RESERVOIR, name);
				if (f1) return *f1;
			}
			return nullptr;
		};
		cweeSharedPtr<cweeLink> FindLink(cweeStr const& name) const {
			{
				AUTO f1 = FindLink(asset_t::PIPE, name);
				if (f1) return *f1;
			}
			{
				AUTO f1 = FindLink(asset_t::PUMP, name);
				if (f1) return *f1;
			}
			{
				AUTO f1 = FindLink(asset_t::VALVE, name);
				if (f1) return *f1;
			}
			return nullptr;
		};
		template<asset_t type> AUTO Find(cweeStr const& name) const {
			if constexpr (type == asset_t::JUNCTION) {
				AUTO p = junctions.Find(cweeAsset::Hash(type, name));
				return p;
			} 
			else if constexpr (type == asset_t::RESERVOIR) {
				AUTO p = reservoirs.Find(cweeAsset::Hash(type, name));
				return p;
			} 
			else if constexpr (type == asset_t::PIPE) {
				AUTO p = pipes.Find(cweeAsset::Hash(type, name));
				return p;
			} 
			else if constexpr (type == asset_t::PUMP) {
				AUTO p = pumps.Find(cweeAsset::Hash(type, name));
				return p;
			} 
			else if constexpr (type == asset_t::VALVE) {
				AUTO p = valves.Find(cweeAsset::Hash(type, name));
				return p;
			} 
			else if constexpr (type == asset_t::DMA) {
				AUTO p = DMAs.Find(cweeAsset::Hash(type, name));
				return p;
			} 
			else if constexpr (type == asset_t::PUMPSTATION) {
				AUTO p = pumpStations.Find(cweeAsset::Hash(type, name));
				return p;
			} 
			else {
				static_assert(false, "Cannot find any other types that those instantiated here.");
				return 0;
			}
		};

		void Remove(asset_t type, cweeStr const& name) {
			assets.Remove(assets.Find(cweeAsset::Hash(type, name)));
			nodes.Remove(nodes.Find(cweeAsset::Hash(type, name)));
			links.Remove(links.Find(cweeAsset::Hash(type, name)));
			junctions.Remove(junctions.Find(cweeAsset::Hash(type, name)));
			reservoirs.Remove(reservoirs.Find(cweeAsset::Hash(type, name)));
			pipes.Remove(pipes.Find(cweeAsset::Hash(type, name)));
			valves.Remove(valves.Find(cweeAsset::Hash(type, name)));
			pumps.Remove(pumps.Find(cweeAsset::Hash(type, name)));
			DMAs.Remove(DMAs.Find(cweeAsset::Hash(type, name)));
			pumpStations.Remove(pumpStations.Find(cweeAsset::Hash(type, name)));
		};
	};
	
	// Node Adjacency List Item -- NOT THREAD SAFE
	class Smatrix {
	public:
		cweeList<cweeSharedPtr<cweeNode>> Nodes;
		cweeList<cweeSharedPtr<cweeLink>> Links;		
		std::map<size_t, int> NodeHash;
		std::map<size_t, int> LinkHash;

		cweeList<cfs_p_ft_t>
			Aii;        // Diagonal matrix coeffs.
		cweeList<cfs_p_ft_t>
			Aij;        // Non-zero, off-diagonal matrix coeffs.
		cweeList<units::flowrate::cubic_foot_per_second_t>
			F;          // Right hand side vector
		cweeList<units::length::foot_t>
			B_ft;       // Right hand side vector result
		cweeList<squared_cfs_p_ft_t>
			temp;       // Array used by linear eqn. solver

		int
			Ncoeffs;     // Number of non-zero matrix coeffs
		
		cweeList<int>
			Order,      // Node-to-row of re-ordered matrix
			Row,        // Row-to-node of re-ordered matrix
			Ndx,        // Index of link's coeff. in Aij
			XLNZ,       // Start position of each column in NZSUB
			NZSUB,      // Row index of each coeff. in each column
			LNZ,        // Position of each coeff. in Aij array
			link,       // Array used by linear eqn. solver
			first;      // Array used by linear eqn. solver		
	};
	class smatrix_t {
	public:
		using Padjlist = cweeSharedPtr<Sadjlist>;

		static int						NodeToIndex(cweeSharedPtr<Smatrix> sm, cweeSharedPtr<cweeNode> n) {
			if (!n) return 0; 
			return sm->NodeHash[n->Hash()] + 1;
		};
		static int						LinkToIndex(cweeSharedPtr<Smatrix> sm, cweeSharedPtr<cweeLink> n) {
			if (!n) return 0;
			return sm->LinkHash[n->Hash()] + 1;
		};
		static cweeSharedPtr<cweeNode>	IndexToNode(cweeSharedPtr<Smatrix> sm, int n) {
			n--;
			if (n >= 0 && n < sm->Nodes.Num()) {
				return sm->Nodes[n];
			}
			else {
				std::cout << cweeStr::printf("BAD NODE INDEX AT %i out of %i\n", n, sm->Nodes.Num());
				return nullptr;
			}
		};
		static cweeSharedPtr<cweeLink>	IndexToLink(cweeSharedPtr<Smatrix> sm, int n) {
			n--;
			if (n >= 0 && n < sm->Links.Num()) {
				return sm->Links[n];
			}
			else {
				std::cout << cweeStr::printf("BAD LINK INDEX AT %i out of %i\n", n, sm->Links.Num());
				return nullptr;
			}
		};

		static cweeSharedPtr<Sadjlist>	Get_Sadjlist(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<cweeNode> node) {
			if (node) {
				AUTO adj = net->Adjlist.FindOrMake(node->Hash());
				if (adj) {
					if (!adj->get<0>()) adj->get<0>() = node;
					return adj->get<1>();
				}
			}
			return nullptr;
		};
		static void						Set_Sadjlist(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<cweeNode> node, cweeSharedPtr<Sadjlist> adj) {
			if (node) {
				AUTO adjP = net->Adjlist.FindOrMake(node->Hash());
				if (adjP) {
					if (!adjP->get<0>()) adjP->get<0>() = node;
					adjP->get<1>() = adj;
				}
			}
		};

		static int  createsparse(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm)
			/*
			**--------------------------------------------------------------
			** Input:   none
			** Output:  returns error code
			** Purpose: creates sparse representation of coeff. matrix
			**--------------------------------------------------------------
			*/
		{
			int errcode = 0;

			// Allocate sparse matrix data structures
			errcode = allocsmatrix(net, sm, net->nodes.Num(), net->links.Num());
			if (errcode) return errcode;

			// Build a local version of node-link adjacency lists with parallel links removed
			errcode = localadjlists(net, sm);
			if (errcode) return errcode;

			// Re-order nodes to minimize number of non-zero coeffs. in factorized solution matrix
			ERRCODE(reordernodes(net, sm));

			// Factorize solution matrix by updating adjacency lists with non-zero connections due to fill-ins
			sm->Ncoeffs = net->links.Num();
			ERRCODE(factorize(net, sm));

			// Allocate memory for sparse storage of positions of non-zero coeffs. and store these positions in vector NZSUB
			ERRCODE(storesparse(net, sm, net->junctions.Num()));

			// Free memory used for local adjacency lists and sort row indexes in NZSUB to optimize linsolve()
			freeadjlists(net);
			ERRCODE(sortsparse(sm, net->junctions.Num()));

			// Allocate memory used by linear eqn. solver
			ERRCODE(alloclinsolve(sm, net->nodes.Num()));

			// Re-build adjacency lists for future use
			ERRCODE(buildadjlists(net, sm));
			return errcode;
		};
		static int	buildadjlists(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm)
			/*
			**--------------------------------------------------------------
			** Input:   none
			** Output:  returns error code
			** Purpose: builds linked list of links adjacent to each node
			**--------------------------------------------------------------
			*/
		{
			int       errcode = 0;

			// Create an array of adjacency lists
			freeadjlists(net);

			// For each link, update adjacency lists of its end nodes
			for (auto& link : net->links) {
				AUTO i = link->StartingAsset;
				AUTO j = link->EndingAsset;

				// Include link in start node i's list
				AUTO alink = make_cwee_shared<Sadjlist>();
				alink->node = j;
				//alink->link = link;
				alink->link_index = LinkToIndex(sm, link);
				alink->next = Get_Sadjlist(net, i);
				Set_Sadjlist(net, i, alink);

				// Include link in end node j's list
				alink = make_cwee_shared<Sadjlist>();
				alink->node = i;
				//alink->link = link;
				alink->link_index = LinkToIndex(sm, link);
				alink->next = Get_Sadjlist(net, j);
				Set_Sadjlist(net, j, alink);
			}

			if (errcode) freeadjlists(net);
			return errcode;
		};
		static int  allocsmatrix(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm, int Nnodes, int Nlinks)
			/*
			**--------------------------------------------------------------
			** Input:   none
			** Output:  returns error code
			** Purpose: allocates memory for representing a sparse matrix
			**--------------------------------------------------------------
			*/
		{
			// Memory for linear eqn. solver allocated in alloclinsolve().
			sm->Aij.Clear();
			sm->Aii.Clear();
			sm->F.Clear();
			sm->temp.Clear();
			sm->link.Clear();
			sm->first.Clear();
			sm->Order.Clear();
			sm->Row.Clear();
			sm->Ndx.Clear();

			// Memory for representing sparse matrix data structure
			sm->Order.AssureSize(Nnodes + 1);
			sm->Row.AssureSize(Nnodes + 1);
			sm->Ndx.AssureSize(Nlinks + 1);

			sm->NodeHash.clear();
			sm->LinkHash.clear();
			sm->Nodes.Clear();
			sm->Links.Clear();
			for (auto& j : net->nodes) if (j->Type == asset_t::JUNCTION) sm->NodeHash[j->Hash()] = sm->Nodes.Append(j);
			for (auto& j : net->nodes) if (j->Type == asset_t::RESERVOIR) sm->NodeHash[j->Hash()] = sm->Nodes.Append(j);
			for (auto& j : net->links) sm->LinkHash[j->Hash()] = sm->Links.Append(j);

			return 0;
		};
		static int  alloclinsolve(cweeSharedPtr<Smatrix> sm, int n)
			/*
			**--------------------------------------------------------------
			** Input:   none
			** Output:  returns error code
			** Purpose: allocates memory used by linear eqn. solver.
			**--------------------------------------------------------------
			*/
		{
			sm->Aij.Clear();
			sm->Aii.Clear();
			sm->F.Clear();
			sm->temp.Clear();
			sm->link.Clear();
			sm->first.Clear();

			sm->Aij.AssureSize(sm->Ncoeffs + 1);
			sm->Aii.AssureSize(n + 1);
			sm->F.AssureSize(n + 1);
			sm->temp.AssureSize(n + 1);
			sm->link.AssureSize(n + 1);
			sm->first.AssureSize(n + 1);

			return 0;
		};
		static void freesparse(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm)
			/*
			**----------------------------------------------------------------
			** Input:   None
			** Output:  None
			** Purpose: Frees memory used for sparse matrix storage
			**----------------------------------------------------------------
			*/
		{
			sm->Order.Clear();
			sm->Row.Clear();
			sm->Ndx.Clear();
			sm->XLNZ.Clear();
			sm->NZSUB.Clear();
			sm->LNZ.Clear();
			sm->Aij.Clear();
			sm->Aii.Clear();
			sm->F.Clear();
			sm->temp.Clear();
			sm->link.Clear();
			sm->first.Clear();
		};
		static int  localadjlists(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm)
			/*
			**--------------------------------------------------------------
			** Input:   none
			** Output:  returns error code
			** Purpose: builds linked list of non-parallel links adjacent to each node
			**--------------------------------------------------------------
			*/
		{
			bool    pmark;     // parallel link marker
			int    errcode = 0;

			// Create an array of adjacency lists
			freeadjlists(net);

			// For each link, update adjacency lists of its end nodes
			for (auto& link : net->links) {
				AUTO i = link->StartingAsset;
				AUTO j = link->EndingAsset;

				pmark = paralink(net, sm, i, j, link);  // Parallel link check

				// Include link in start node i's list
				AUTO alink = make_cwee_shared<Sadjlist>();
				if (!pmark) alink->node = j;
				else        alink->node = nullptr;         // Parallel link marker
				alink->link_index = LinkToIndex(sm, link);
				alink->next = Get_Sadjlist(net, i);
				Set_Sadjlist(net, i, alink);

				// Include link in end node j's list
				alink = make_cwee_shared<Sadjlist>();
				if (!pmark) alink->node = i;
				else        alink->node = nullptr;         // Parallel link marker
				alink->link_index = LinkToIndex(sm, link);
				alink->next = Get_Sadjlist(net, j);
				Set_Sadjlist(net, j, alink);
			}

			// Remove parallel links from adjacency lists
			xparalinks(net);
			return errcode;
		};
		static void	freeadjlists(cweeSharedPtr<cweeHydraulicNetwork> net) { net->Adjlist.Clear(); };
		static bool  paralink(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm, cweeSharedPtr<cweeNode> i, cweeSharedPtr<cweeNode> j, cweeSharedPtr<cweeLink> k)
			/*
			**--------------------------------------------------------------
			** Input:   i = index of start node of link
			**          j = index of end node of link
			**          k = link index
			** Output:  returns 1 if link k parallels another link, else 0
			** Purpose: checks for parallel links between nodes i and j
			**
			**--------------------------------------------------------------
			*/
		{
			for (AUTO alink = Get_Sadjlist(net, i); alink != nullptr; alink = alink->next)
			{
				// Link || to k (same end nodes)
				if (alink->node == j)
				{
					// Assign Ndx entry to this link
					sm->Ndx[LinkToIndex(sm, k)] = alink->link_index;
					return true;
				}
			}
			// Ndx entry if link not parallel
			sm->Ndx[LinkToIndex(sm, k)] = LinkToIndex(sm, k);
			return false;
		};
		static void xparalinks(cweeSharedPtr<cweeHydraulicNetwork> net)
			/*
			**--------------------------------------------------------------
			** Input:   none
			** Output:  none
			** Purpose: removes parallel links from nodal adjacency lists
			**--------------------------------------------------------------
			*/
		{
			int    i;
			Padjlist    alink,       // Current item in adjacency list
				blink;       // Previous item in adjacency list

			// Scan adjacency list of each node
			for (auto& node : net->nodes) {
				alink = Get_Sadjlist(net, node);
				blink = nullptr;
				while (alink != nullptr)
				{
					if (alink->node == nullptr)              // Parallel link marker found
					{
						if (blink == nullptr)             // This holds at start of list
						{
							Set_Sadjlist(net, node, alink->next);
							alink = alink->next;
						}
						else                           // This holds for interior of list
						{
							blink->next = alink->next;
							alink = blink->next;
						}
					}
					else
					{
						blink = alink;                // Move to next item in list
						alink = alink->next;
					}
				}
			}
		};
		static int  reordernodes(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm)
			/*
			**--------------------------------------------------------------
			** Input:   none
			** Output:  returns 1 if successful, 0 if not
			** Purpose: re-orders nodes to minimize # of non-zeros that
			**          will appear in factorized solution matrix
			**--------------------------------------------------------------
			*/
		{
			int k, knode, m, njuncs, nlinks;
			int delta = -1;
			int nofsub = 0;
			int maxint = INT_MAX;   //defined in limits.h
			int errcode;
			Padjlist alink;

			// Local versions of node adjacency lists
			cweeList<int>
				adjncy,
				xadj,
				dhead,
				qsize,
				llist,
				marker;

			njuncs = net->junctions.Num();
			nlinks = net->links.Num();

			// Default ordering			
			for (auto& node : net->nodes) {
				AUTO k = NodeToIndex(sm, node);
				sm->Row[k] = k;
				sm->Order[k] = k;
			}

			// Allocate memory
			adjncy.AssureSize(2 * net->links.Num() + 1);
			xadj.AssureSize(net->junctions.Num() + 2);
			dhead.AssureSize(net->junctions.Num() + 1);
			qsize.AssureSize(net->junctions.Num() + 1);
			llist.AssureSize(net->junctions.Num() + 1);
			marker.AssureSize(net->junctions.Num() + 1);

			// Create local versions of node adjacency lists
			xadj[1] = 1;
			m = 1;
			for (auto& node : net->nodes) {
				if (node->Type == asset_t::JUNCTION) {
					k = NodeToIndex(sm, node);
					for (alink = Get_Sadjlist(net, node); alink != nullptr; alink = alink->next) {
						knode = NodeToIndex(sm, alink->node);
						if (knode > 0 && knode <= njuncs)
						{
							adjncy[m] = knode;
							m++;
						}
					}
					xadj[k + 1] = m;
				}
			}

			// Generate a multiple minimum degree node re-ordering
			::epanet::genmmd_t::genmmd(&njuncs, xadj.Ptr(), adjncy.Ptr(), sm->Row.Ptr(), sm->Order.Ptr(), &delta, dhead.Ptr(), qsize.Ptr(), llist.Ptr(), marker.Ptr(), &maxint, &nofsub);

			return 0;
		};
		static int  factorize(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm)
			/*
			**--------------------------------------------------------------
			** Input:   none
			** Output:  returns error code
			** Purpose: symbolically factorizes the solution matrix in
			**          terms of its adjacency lists
			**--------------------------------------------------------------
			*/
		{
			int k, knode;
			int errcode = 0;
			Padjlist alink;

			// Find degree of each junction node
			cweeList<int> degree; degree.AssureSize(net->nodes.Num() + 1);

			// NOTE: For purposes of node re-ordering, Tanks (nodes with indexes above Njuncs) have zero degree of adjacency.
			for (auto& node : net->nodes) {
				if (node->Type == asset_t::JUNCTION) {
					k = NodeToIndex(sm, node);
					for (alink = Get_Sadjlist(net, node); alink != nullptr; alink = alink->next)
					{
						if (alink->node != nullptr) degree[k]++;
					}
				}
			}

			// Augment each junction's adjacency list to account for
			// new connections created when solution matrix is solved.
			// NOTE: Only junctions (indexes <= Njuncs) appear in solution matrix.
			for (auto& node : net->nodes) {
				if (node->Type == asset_t::JUNCTION) { // Examine each junction
					k = NodeToIndex(sm, node);
					knode = sm->Order[k];  // Re-ordered index
					growlist(net, sm, knode, degree);
					degree[knode] = 0;                  // In-activate node
				}
			}

			return errcode;
		};
		static void  growlist(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm, int knode, cweeList<int>& degree)
			/*
			**--------------------------------------------------------------
			** Input:   knode = node index
			** Output:  returns 1 if successful, 0 if not
			** Purpose: creates new entries in knode's adjacency list for
			**          all unlinked pairs of active nodes that are
			**          adjacent to knode
			**--------------------------------------------------------------
			*/
		{
			Padjlist alink;

			// Iterate through all nodes connected to knode
			for (alink = Get_Sadjlist(net, IndexToNode(sm, knode)); alink != nullptr; alink = alink->next)
			{
				if (alink->node) // End node is active
				{
					degree[NodeToIndex(sm, alink->node)]--;           // Reduce degree of adjacency
					newlink(net, sm, alink, degree);
				}
			}
		};
		static void  newlink(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm, Padjlist alink, cweeList<int>& degree)
			/*
			**--------------------------------------------------------------
			** Input:   alink = element of node's adjacency list
			** Output:  returns 1 if successful, 0 if not
			** Purpose: links end of current adjacent link to end nodes of
			**          all links that follow it on adjacency list
			**--------------------------------------------------------------
			*/
		{
			cweeSharedPtr<cweeNode> inode;
			cweeSharedPtr<cweeNode> jnode;
			Padjlist blink;

			// Scan all entries in adjacency list that follow anode.
			inode = alink->node;             // End node of connection to anode
			for (blink = alink->next; blink != nullptr; blink = blink->next)
			{
				jnode = blink->node;          // End node of next connection

				// If jnode still active, and inode not connected to jnode, then add a new connection between inode and jnode.
				if (jnode)  // jnode still active
				{
					if (!linked(net, sm, inode, jnode))      // inode not linked to jnode
					{
						// Since new connection represents a non-zero coeff. in the solution matrix, update the coeff. count.
						sm->Ncoeffs++;

						// Update adjacency lists for inode & jnode to reflect the new connection.
						addlink(net, sm, inode, jnode, sm->Ncoeffs);
						addlink(net, sm, jnode, inode, sm->Ncoeffs);
						degree[NodeToIndex(sm, inode)]++;
						degree[NodeToIndex(sm, jnode)]++;
					}
				}
			}
		};
		static bool  linked(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm, cweeSharedPtr<cweeNode> i, cweeSharedPtr<cweeNode> j)
			/*
			**--------------------------------------------------------------
			** Input:   i = node index
			**          j = node index
			** Output:  returns 1 if nodes i and j are linked, 0 if not
			** Purpose: checks if nodes i and j are already linked.
			**--------------------------------------------------------------
			*/
		{
			Padjlist alink;
			for (alink = Get_Sadjlist(net, i); alink != nullptr; alink = alink->next)
			{
				if (alink->node == j) return true;
			}
			return false;
		};
		static void  addlink(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm, cweeSharedPtr<cweeNode> i, cweeSharedPtr<cweeNode> j, int n)
			/*
			**--------------------------------------------------------------
			** Input:   i = node index
			**          j = node index
			**          n = link index
			** Output:  returns 1 if successful, 0 if not
			** Purpose: augments node i's adjacency list with node j
			**--------------------------------------------------------------
			*/
		{
			Padjlist alink;
			alink = make_cwee_shared<Sadjlist>();
			alink->node = j;
			alink->link_index = n;
			alink->next = Get_Sadjlist(net, i);
			Set_Sadjlist(net, i, alink);
		};
		static int  storesparse(cweeSharedPtr<cweeHydraulicNetwork> net, cweeSharedPtr<Smatrix> sm, int n)
			/*
			**--------------------------------------------------------------
			** Input:   n = number of rows in solution matrix
			** Output:  returns error code
			** Purpose: stores row indexes of non-zeros of each column of
			**          lower triangular portion of factorized matrix
			**--------------------------------------------------------------
			*/
		{
			int i, ii, j, k, l, m;
			int errcode = 0;
			Padjlist alink;

			// Allocate sparse matrix storage
			sm->XLNZ.Clear();
			sm->NZSUB.Clear();
			sm->LNZ.Clear();

			sm->XLNZ.AssureSize(n + 2);
			sm->NZSUB.AssureSize(sm->Ncoeffs + 2);
			sm->LNZ.AssureSize(sm->Ncoeffs + 2);

			// Generate row index pointers for each column of matrix
			k = 0;
			sm->XLNZ[1] = 1;
			for (i = 1; i <= n; i++)            // column
			{
				m = 0;
				ii = sm->Order[i];
				for (alink = Get_Sadjlist(net, IndexToNode(sm, ii)); alink != nullptr; alink = alink->next)
				{
					if (alink->node == nullptr) continue;
					j = sm->Row[NodeToIndex(sm, alink->node)];    // row
					l = alink->link_index;
					if (j > i && j <= n) {
						m++;
						k++;
						sm->NZSUB[k] = j;
						sm->LNZ[k] = l;
					}
				}
				sm->XLNZ[i + 1] = sm->XLNZ[i] + m;
			}
			return errcode;
		};
		static int  sortsparse(cweeSharedPtr<Smatrix> sm, int n)
			/*
			**--------------------------------------------------------------
			** Input:   n = number of rows in solution matrix
			** Output:  returns eror code
			** Purpose: puts row indexes in ascending order in NZSUB
			**--------------------------------------------------------------
			*/
		{
			int  i, k;
			cweeList<int> xlnzt, nzsubt, lnzt, nzt;
			int  errcode = 0;

			auto& LNZ = sm->LNZ;
			auto& XLNZ = sm->XLNZ;
			auto& NZSUB = sm->NZSUB;

			xlnzt.AssureSize(n + 2);
			nzsubt.AssureSize(sm->Ncoeffs + 2);
			lnzt.AssureSize(sm->Ncoeffs + 2);
			nzt.AssureSize(n + 2);

			if (!errcode)
			{
				// Count # non-zeros in each row
				for (i = 1; i <= n; i++)
					nzt[i] = 0;

				for (i = 1; i <= n; i++)
					for (k = XLNZ[i]; k < XLNZ[i + 1]; k++)
						nzt[NZSUB[k]]++;

				xlnzt[1] = 1;
				for (i = 1; i <= n; i++) xlnzt[i + 1] = xlnzt[i] + nzt[i];

				// Transpose matrix twice to order column indexes
				transpose(n, XLNZ, NZSUB, LNZ, xlnzt, nzsubt, lnzt, nzt);
				transpose(n, xlnzt, nzsubt, lnzt, XLNZ, NZSUB, LNZ, nzt);
			}

			return errcode;
		};
		static void transpose(int n, cweeList<int>& il, cweeList<int>& jl, cweeList<int>& xl, cweeList<int>& ilt, cweeList<int>& jlt, cweeList<int>& xlt, cweeList<int>& nzt)
			/*
			**---------------------------------------------------------------------
			** Input:   n = matrix order
			**          il,jl,xl = sparse storage scheme for original matrix
			**          nzt = work array
			** Output:  ilt,jlt,xlt = sparse storage scheme for transposed matrix
			** Purpose: Determines sparse storage scheme for transpose of a matrix
			**---------------------------------------------------------------------
			*/
		{
			int  i, j, k, kk;

			for (i = 1; i <= n; i++) nzt[i] = 0;
			for (i = 1; i <= n; i++)
			{
				for (k = il[i]; k < il[i + 1]; k++)
				{
					j = jl[k];
					kk = ilt[j] + nzt[j];
					jlt[kk] = i;
					xlt[kk] = xl[k];
					nzt[j]++;
				}
			}
		};
		static int  linsolve(cweeSharedPtr<Smatrix> sm, int n)
			/*
			**--------------------------------------------------------------
			** Input:   sm   = sparse matrix struct
						n    = number of equations
			** Output:  sm->F = solution values
			**          returns 0 if solution found, or index of
			**          equation causing system to be ill-conditioned
			** Purpose: solves sparse symmetric system of linear
			**          equations using Cholesky factorization
			**
			** NOTE:   This procedure assumes that the solution matrix has
			**         been symbolically factorized with the positions of
			**         the lower triangular, off-diagonal, non-zero coeffs.
			**         stored in the following integer arrays:
			**            XLNZ  (start position of each column in NZSUB)
			**            NZSUB (row index of each non-zero in each column)
			**            LNZ   (position of each NZSUB entry in Aij array)
			**
			**  This procedure has been adapted from subroutines GSFCT and
			**  GSSLV in the book "Computer Solution of Large Sparse
			**  Positive Definite Systems" by A. George and J. W-H Liu
			**  (Prentice-Hall, 1981).
			**--------------------------------------------------------------
			*/
		{
			using namespace cwee_units;

			auto& Aii = sm->Aii;
			auto& Aij = sm->Aij;
			auto& B_cfs = sm->F;
			auto& temp = sm->temp;
			auto& B_ft = sm->B_ft;
			B_ft.Clear(); B_ft.AssureSize(B_cfs.Num());

			auto& LNZ = sm->LNZ; // indexes
			auto& XLNZ = sm->XLNZ; // indexes
			auto& NZSUB = sm->NZSUB; // indexes
			auto& link = sm->link; // indexes
			auto& first = sm->first; // indexes

			int    i, istop, istrt, isub, j, k, kfirst, newk;
			squared_cfs_p_ft_t diagj_cfs2_p_ft;

			for (auto& x : temp) x = 0;
			for (auto& x : link) x = 0;
			for (auto& x : first) x = 0;

			// Begin numerical factorization of matrix A into L ... Compute column L(*,j) for j = 1,...n
			for (j = 1; j <= n; j++)
			{
				// For each column L(*,k) that affects L(*,j):
				auto diagj = Aij[0] * Aij[0];
				diagj = 0;

				diagj_cfs2_p_ft = 0;
				newk = link[j];
				k = newk;
				while (k != 0)
				{
					// Outer product modification of L(*,j) by L(*,k) starting at first[k] of L(*,k)
					newk = link[k];
					kfirst = first[k];
					auto& ljk = Aij[LNZ[kfirst]];

					diagj += ljk * ljk;

					istrt = kfirst + 1;
					istop = XLNZ[k + 1] - 1;
					if (istop >= istrt)
					{
						// Before modification, update vectors 'first' and 'link' for future modification steps
						first[k] = istrt;
						isub = NZSUB[istrt];
						link[k] = link[isub];
						link[isub] = k;

						// The actual mod is saved in vector 'temp'
						for (i = istrt; i <= istop; i++)
						{
							isub = NZSUB[i];
							temp[isub] += Aij[LNZ[i]] * ljk;
						}
					}
					k = newk;
				}

				// Apply the modifications accumulated in 'temp' to column L(*,j)
				if (diagj >= Aii[j] * (cfs_p_ft_t)1.0) { /* Check for ill - conditioning */
					std::cout << "ILLCONDITIONED!\n";
					std::cout << "\t J : " << j << ", diagj : " << diagj << ", Aii[j] : " << Aii[j] << ", Aii[j]*1 : " << (Aii[j] * (cfs_p_ft_t)1.0) << std::endl;

					// Aii[j] = ::epanet::CSMALL;
					return j;
				}
				else {
					Aii[j] = units::math::sqrt((Aii[j] * (cfs_p_ft_t)1.0) - diagj);
				}
				istrt = XLNZ[j];
				istop = XLNZ[j + 1] - 1;
				if (istop >= istrt)
				{
					first[j] = istrt;
					isub = NZSUB[istrt];
					link[j] = link[isub];
					link[isub] = j;
					for (i = istrt; i <= istop; i++)
					{
						isub = NZSUB[i];
						Aij[LNZ[i]] = ((Aij[LNZ[i]] * (cfs_p_ft_t)1.0) - temp[isub]) / Aii[j];
						temp[isub] = 0.0;
					}
				}
			}      // next j

			// Foward substitution
			for (j = 1; j <= n; j++)
			{
				foot_t bj_ft = B_cfs[j] / Aii[j];
				B_ft[j] = bj_ft;

				istrt = XLNZ[j];
				istop = XLNZ[j + 1] - 1;
				if (istop >= istrt)
				{
					for (i = istrt; i <= istop; i++)
					{
						isub = NZSUB[i];
						B_cfs[isub] -= Aij[LNZ[i]] * bj_ft;
					}
				}
			}

			// Backward substitution
			for (j = n; j >= 1; j--)
			{
				foot_t bj_ft = B_ft[j];

				istrt = XLNZ[j];
				istop = XLNZ[j + 1] - 1;
				if (istop >= istrt) {
					for (i = istrt; i <= istop; i++)
					{
						isub = NZSUB[i];
						bj_ft -= ((Aij[LNZ[i]] * B_ft[isub]) * ((ft_per_cfs_t)1.0)); // RG ... accumulate, with units mult.
					}
				}
				B_ft[j] = (bj_ft / Aii[j]) * (cfs_p_ft_t)1.0; // RG ... undo units mult.
			}

			return 0;
		};
	};
};




//class cweeAssetController {
//	cweeAssetController() : Enabled(false) {};
//	virtual ~cweeAssetController() {};
//
//	virtual float GetSetting() { return 0; };
//	cweeInterlocked<bool>	Enabled;
//};