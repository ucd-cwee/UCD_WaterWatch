#pragma once

#pragma region "INCLUDES"
#pragma hdrstop
#include "Precompiled.h"
#pragma endregion

#pragma region "cweeScheduler"
template<class shared_data, class policy_data>
class cweeScheduler {
public:
	static cweeJob run(
		cweeSharedPtr<shared_data> sharedData,
		std::function<cweeThreadedList<policy_data>(shared_data&)> ScheduleIteration,
		std::function<void(policy_data&)> EvaluatePolicy,
		std::function<void(shared_data&, cweeThreadedList<policy_data>&)> FinishIteration,
		std::function<bool(shared_data&)> StopOptimization
	) {
		return perform(cweeSharedPtr<cweeSchedulerData>(new cweeSchedulerData(std::move(sharedData), std::move(ScheduleIteration), std::move(EvaluatePolicy), std::move(FinishIteration), std::move(StopOptimization))));
	};
	static cweeJob run_async(
		cweeSharedPtr<shared_data> sharedData,
		std::function<cweeThreadedList<policy_data>(shared_data&)> ScheduleIteration,
		std::function<void(policy_data&)> EvaluatePolicy,
		std::function<void(shared_data&, cweeThreadedList<policy_data>&)> FinishIteration,
		std::function<bool(shared_data&)> StopOptimization
	) {
		return async_perform(cweeSharedPtr<cweeSchedulerData>(new cweeSchedulerData(std::move(sharedData), std::move(ScheduleIteration), std::move(EvaluatePolicy), std::move(FinishIteration), std::move(StopOptimization))));
	};

private:
	class cweeSchedulerData {
	public:
		cweeSchedulerData() {};
		cweeSchedulerData(cweeSharedPtr<shared_data> sharedData, std::function<cweeThreadedList<policy_data>(shared_data&)> ScheduleIteration,
			std::function<void(policy_data&)> EvaluatePolicy, std::function<void(shared_data&, cweeThreadedList<policy_data>&)> FinishIteration,
			std::function<bool(shared_data&)> ReviewStopConditions
		) : m_sharedData(std::move(sharedData)),
			m_ScheduleIteration(std::move(ScheduleIteration)),
			m_EvaluatePolicy(std::move(EvaluatePolicy)),
			m_FinishIteration(std::move(FinishIteration)),
			m_ReviewStopConditions(std::move(ReviewStopConditions))
		{};

		cweeSharedPtr<shared_data> m_sharedData;
		std::function<cweeThreadedList<policy_data>(shared_data&)> m_ScheduleIteration;
		std::function<void(policy_data&)> m_EvaluatePolicy;
		std::function<void(shared_data&, cweeThreadedList<policy_data>&)> m_FinishIteration;
		std::function<bool(shared_data&)> m_ReviewStopConditions;
	};
	static cweeJob perform(cweeSharedPtr<cweeSchedulerData> in) {
		cweeJob finalJob([](cweeSharedPtr<cweeSchedulerData> in2) -> cweeAny { return cweeAny(in2->m_sharedData); }, in);
		cweeJob schedulerJob(in->m_ScheduleIteration, in->m_sharedData);
		schedulerJob.ContinueWith(cweeJob([](cweeSharedPtr<cweeSchedulerData> in2, cweeJob& schedulerResults, cweeJob& finalJob2) {
			cweeAny schedulerResult(schedulerResults.GetResult());
			cweeThreadedList<policy_data>& schedule = schedulerResult.cast();
			cweeSharedPtr<cweeSysInterlockedInteger> counter = make_cwee_shared<cweeSysInterlockedInteger>();
			if (schedule.Num() <= 0) Invoke(finalJob2);
			else for (int i = 0; i < schedule.Num(); i++) {
				Invoke(cweeJob([](cweeThreadedList<policy_data>& policies, int which, cweeSharedPtr<cweeSchedulerData> in3, cweeSysInterlockedInteger& count, cweeJob& scheduler, cweeJob& finalJob3) {
					in3->m_EvaluatePolicy(policies[which]); // evaluate 1 of n policies...
					if (count.Increment() >= policies.Num()) { // after we have finished all policies..
						in3->m_FinishIteration(*(in3->m_sharedData.Get()), policies); // first, evaluate policies, then...
						if (!in3->m_ReviewStopConditions(*(in3->m_sharedData.Get()))) Invoke(scheduler); // loop again, or...
						else Invoke(finalJob3); // end optimization.
					}
					}, schedulerResult, i, in2, counter, schedulerResults, finalJob2));
			}
			}, in, schedulerJob, finalJob));
		Invoke(schedulerJob);
		return finalJob;
	};
	static cweeJob async_perform(cweeSharedPtr<cweeSchedulerData> in) {
		cweeJob finalJob([](cweeSharedPtr<cweeSchedulerData> in2) -> cweeAny { return cweeAny(in2->m_sharedData); }, in);
		cweeJob schedulerJob(in->m_ScheduleIteration, in->m_sharedData);
		schedulerJob.ContinueWith(cweeJob([](cweeSharedPtr<cweeSchedulerData> in2, cweeJob& schedulerResults, cweeJob& finalJob2) {
			cweeAny schedulerResult(schedulerResults.GetResult());
			cweeThreadedList<policy_data>& schedule = schedulerResult.cast();
			cweeSharedPtr<cweeSysInterlockedInteger> counter = make_cwee_shared<cweeSysInterlockedInteger>();
			if (schedule.Num() <= 0) AsyncInvoke(finalJob2);
			else for (int i = 0; i < schedule.Num(); i++) {
				AsyncInvoke(cweeJob([](cweeThreadedList<policy_data>& policies, int which, cweeSharedPtr<cweeSchedulerData> in3, cweeSysInterlockedInteger& count, cweeJob& scheduler, cweeJob& finalJob3) {
					in3->m_EvaluatePolicy(policies[which]); // evaluate 1 of n policies...
					if (count.Increment() >= policies.Num()) { // after we have finished all policies..
						in3->m_FinishIteration(*(in3->m_sharedData.Get()), policies); // first, evaluate policies, then...
						if (!in3->m_ReviewStopConditions(*(in3->m_sharedData.Get()))) AsyncInvoke(scheduler); // loop again, or...
						else AsyncInvoke(finalJob3); // end optimization.
					}
					}, schedulerResult, i, in2, counter, schedulerResults, finalJob2));
			}
			}, in, schedulerJob, finalJob));
		AsyncInvoke(schedulerJob);
		return finalJob;
	};
	static cweeJob& AsyncInvoke(cweeJob& j) {
		return j.AsyncForceInvoke(); // Async
	};
	static cweeJob& AsyncInvoke(cweeJob&& j) {
		return j.AsyncForceInvoke(); // Async
	};
	static cweeJob& Invoke(cweeJob& j) {
		return j.ForceInvoke(); // Async
	};
	static cweeJob& Invoke(cweeJob&& j) {
		return j.ForceInvoke(); // Async
	};

};
#pragma endregion

#pragma region "OptimizationManagementTools"
#define useOptimizedMatrixForCweeOptimization
template <int num_dimensions_d, int num_particles_p> class OptimizationManagementTools {
public:
	static u64 default_constraint() { return cweeMath::Pow64(cweeMath::INF, 0.25f) / num_dimensions(); };
	static u64 default_performance() { return cweeMath::INF; };

	class ParticleHistory {
	public:
		static constexpr int num_dimensions() { return num_dimensions_d; };
		static constexpr int num_particles() { return num_particles_p; };
		cweeAny& tag(void* p) { return *tags[p]; };
		cweeAny& tag() { return *tags[(void*)this]; };
		ParticleHistory() { inputs.AssureSize(num_dimensions(), 0); };
		bool ParticleLikelyProcessed() { return performance != OptimizationManagementTools<num_dimensions_d, num_particles_p>::default_performance(); };
	public:
		cweeThreadedList<u64> inputs;
		u64 performance = OptimizationManagementTools<num_dimensions_d, num_particles_p>::default_performance();
	protected: // data
		cweeThreadedMap<void*, cweeAny> tags;
		friend class OptimizationManagementTools;
		friend class IterationHistory;
		friend class OptimizationManagementTools_Details;
	};
	class IterationHistory {
	public:
		static constexpr int num_dimensions() { return num_dimensions_d; };
		static constexpr int num_particles() { return num_particles_p; };
		ParticleHistory BestParticle(bool FindMinimum = true) {
			ParticleHistory out;
			u64 performance = FindMinimum ? OptimizationManagementTools<num_dimensions_d, num_particles_p>::default_performance() : -OptimizationManagementTools<num_dimensions_d, num_particles_p>::default_performance();
			for (auto& p : particles) {
				if (p.ParticleLikelyProcessed()) {
					if (FindMinimum) {
						if (performance >= p.performance) {
							out = p;
							performance = p.performance;
						}
					}
					else {
						if (performance <= p.performance) {
							out = p;
							performance = p.performance;
						}
					}
				}
			};
			return out;
		};
		IterationHistory() { particles.AssureSize(num_particles_p, ParticleHistory()); };
		cweeAny& tag(void* p) { return *tags[p]; };
		cweeAny& tag() { return *tags[(void*)this]; };
		int& iteration_number() { return iterationNumber; };
	public: // data
		cweeThreadedList< ParticleHistory > particles;
		int iterationNumber = 0;
		cweeStr optimizationType;
	protected:
		cweeThreadedMap<void*, cweeAny> tags;
		friend class OptimizationManagementTools_Details;
		friend class OptimizationManagementTools;
	};
	class OptimizationManagementTools_Details {
	public:
		static constexpr int num_dimensions() { return num_dimensions_d; };
		static constexpr int num_particles() { return num_particles_p; };
		OptimizationManagementTools_Details() {
			static_assert(num_dimensions_d >= 1, "OptimizationManagementTools_Details requires the number of dimensions in optimization problem to be equal to or greater than 1. Otherwise there is no need for an optimization as nothing can be tested.");
			upper_constraints.AssureSize(num_dimensions_d, OptimizationManagementTools<num_dimensions_d, num_particles_p>::default_constraint());
			lower_constraints.AssureSize(num_dimensions_d, -OptimizationManagementTools<num_dimensions_d, num_particles_p>::default_constraint());
			AUTO p = Curve(); {
				p.SetBoundaryType(boundary_t::BT_FREE);
			}
			performances.AssureSize(num_dimensions_d, p);
		};
		cweeAny& tag(void* p) { return *tags[p]; };
		cweeAny& tag() { return *tags[(void*)this]; };

	protected:
		cweeThreadedList<float> upper_constraints;
		cweeThreadedList<float> lower_constraints;

		cweeThreadedList<Curve>	performances;

		cweeThreadedList<IterationHistory> iterations;
		bool iterations_0_primary = true;
		cweeThreadedMap<void*, cweeAny> tags;

		friend class OptimizationManagementTools;
	};

	static constexpr int num_dimensions() { return num_dimensions_d; };
	static constexpr int num_particles() { return num_particles_p; };
	int num_iterations() { return (iterations().Num() <= 0 ? 0 : CurrentIteration().iterationNumber); };
	IterationHistory& AllocIteration() {
		if (num_iterations() >= 1) {
			cweeThreadedList<Curve>& perform = this->performances();
			for (auto& p : perform) {
				p.Clear();
				p.SetBoundaryType(boundary_t::BT_FREE);
			}
			IterationHistory& iter = CurrentIteration();
			for (auto& p : iter.particles) {
				for (int i = 0; i < num_dimensions(); i++) {
					perform[i].AddUniqueValue(p.inputs[i], p.performance);
				}
			}
		}
#ifdef useOptimizedMatrixForCweeOptimization
		IterationHistory* out(nullptr);
		if (iterations().Num() == 0) {
			iterations_0_primary() = true;
			out = &iterations().Alloc();
			out->iterationNumber = iterations().Num();
		}
		else if (iterations().Num() == 1) {
			iterations_0_primary() = false;
			out = &iterations().Alloc();
			out->iterationNumber = iterations().Num();
		}
		else {
			iterations_0_primary() = !iterations_0_primary();
			out = &iterations()[iterations_0_primary() ? 0 : 1];
			out->iterationNumber = iterations()[iterations_0_primary() ? 1 : 0].iterationNumber + 1;
		}
		out->optimizationType = OptimizationType();
		return *out;
#else
		iterations_0_primary() = true;
		AUTO iter = iterations().Alloc();
		iter.iterationNumber = iterations().Num();
		iter.optimizationType = OptimizationType();
		return iter;
#endif
	};
	virtual void StartIteration(OptimizationManagementTools<num_dimensions_d, num_particles_p>::IterationHistory& iter) = 0;
	virtual cweeStr OptimizationType() { return cweeStr(cweeAny::TypeNameOf<decltype(this)>()); };
	IterationHistory& CurrentIteration() {
#ifdef useOptimizedMatrixForCweeOptimization
		return iterations()[cweeMath::min(iterations().Num() - 1, iterations_0_primary() ? 0 : 1)];
#else
		return iterations()[iterations().Num() - 1];
#endif			
	};
	virtual ParticleHistory BestParticle() = 0;
	virtual cweeThreadedList<ParticleHistory> BestParticles(int num) = 0;
	float GetRandomAtDimension(int dimension) { return cweeRandomFloat(lower_constraints()[dimension], upper_constraints()[dimension]); };
	float GetRecommendedDimensionValue(int dimension, u64 startWith) {
		Curve& dimensionPerformance = performances()[dimension];
		if (dimensionPerformance.GetNumValues() >= 2) {
			u64 x_n = startWith, derivative = 0, x_n1 = 0, f_xn = 0;

			for (int i = 0; i < 20; i++) {
				derivative = dimensionPerformance.GetCurrentSecondDerivative(startWith);
				f_xn = dimensionPerformance.GetCurrentFirstDerivative(x_n);
				x_n1 = x_n - f_xn / derivative;
				x_n = x_n1;

				if (x_n < lower_constraints()[dimension]) return lower_constraints()[dimension];
				if (x_n > upper_constraints()[dimension]) return upper_constraints()[dimension];
			}
			return (float)cweeMath::Fmin(cweeMath::Fmax(x_n, lower_constraints()[dimension]), upper_constraints()[dimension]);
		}
		else {
			return GetRandomAtDimension(dimension);
		}
	};

protected: // inheritedonly methods
	ParticleHistory BestParticleImpl(bool FindMinimum) {
		ParticleHistory out;
		u64 performance = FindMinimum ? OptimizationManagementTools<num_dimensions_d, num_particles_p>::default_performance() : -OptimizationManagementTools<num_dimensions_d, num_particles_p>::default_performance();
		for (auto& i : iterations()) {
			AUTO p = i.BestParticle(FindMinimum);
			if (p.ParticleLikelyProcessed()) {
				if (FindMinimum) {
					if (performance >= p.performance) {
						out = p;
						performance = p.performance;
					}
				}
				else {
					if (performance <= p.performance) {
						out = p;
						performance = p.performance;
					}
				}
			}
		};
		return out;
	};
	cweeThreadedList<ParticleHistory> BestParticlesImpl(int num, bool FindMinimum) {
		cweeThreadedList<ParticleHistory> result;
		cweeThreadedList<ParticleHistory*> out;
		cweeThreadedList<u64> performances;
		for (auto& i : iterations()) {
			for (auto& j : i.particles) {
				if (j.ParticleLikelyProcessed()) {
					out.Append(&j);
					performances.Append(j.performance);
				}
			}
		};
		performances.Sort();

		if (FindMinimum) {
			for (int i = 0; i < performances.Num() && result.Num() < num; i++) {
				u64& thisPerf = performances[i];
				for (int j = 0; j < out.Num() && result.Num() < num; j++) {
					if (out[j]->performance == thisPerf) {
						result.Append(*out[j]);
					}
				}
			}
		}
		else {
			for (int i = performances.Num() - 1; i >= 0 && result.Num() < num; i--) {
				u64& thisPerf = performances[i];
				for (int j = 0; j < out.Num() && result.Num() < num; j++) {
					if (out[j]->performance == thisPerf) {
						result.Append(*out[j]);
					}
				}
			}
		}

		return result;
	};

public: // data
	cweeSharedPtr< OptimizationManagementTools_Details > details;

public: // data access methods
	cweeThreadedList<Curve>& performances() { return details->performances; };
	auto& upper_constraints() { return details->upper_constraints; };
	auto& lower_constraints() { return details->lower_constraints; };
	auto& iterations() { return details->iterations; };
	auto& iterations_0_primary() { return details->iterations_0_primary; };
	cweeAny& tag(void* p) { return details->tag(p); };
	cweeAny& tag() { return details->tag((void*)this); };

public: // construct / destroy
	OptimizationManagementTools() : details(make_cwee_shared<OptimizationManagementTools_Details>()) {
		static_assert(num_dimensions_d > 0, "Cannot have less than one dimension in optimization");
		static_assert(num_particles_p > 0, "Cannot have less than one particle in optimization");
	};
	OptimizationManagementTools(cweeSharedPtr< OptimizationManagementTools_Details > otherDetails) : details(otherDetails) {
		static_assert(num_dimensions_d > 0, "Cannot have less than one dimension in optimization");
		static_assert(num_particles_p > 0, "Cannot have less than one particle in optimization");
	};
	virtual ~OptimizationManagementTools() {};
};
template <int num_dimensions_d, int num_particles_p, bool minimize = true> class Random_OptimizationManagementTool final : public OptimizationManagementTools<num_dimensions_d, num_particles_p> {
public:
	typedef OptimizationManagementTools<num_dimensions_d, num_particles_p> ParentType;
	typedef typename ParentType::ParticleHistory ParticleType;
	typedef typename ParentType::IterationHistory IterationType;

	Random_OptimizationManagementTool() : ParentType() {};
	Random_OptimizationManagementTool(ParentType const& other) : ParentType(other.details) {};
	Random_OptimizationManagementTool(cweeSharedPtr< ParentType::OptimizationManagementTools_Details > other) : ParentType(other) {};
	~Random_OptimizationManagementTool() {};

	u64 percent_newtonSearch = 0.2f;

	void StartIteration(ParentType::IterationHistory& iter) override final {
		int numRecombine = cweeMath::max(0, ((u64)(num_particles_p)) * percent_newtonSearch);
		int i = 0;
		if (iter.iterationNumber > 1) {
			iter.particles[i] = BestParticle(); i++;
		}
		if (iter.iterationNumber > 2) {
			AUTO part = BestParticle();
			for (; i < numRecombine; i++) {
				AUTO p = iter.particles[i];
				for (int j = 0; j < num_dimensions_d; j++) {
					p.inputs[j] = this->GetRecommendedDimensionValue(j, part.inputs[j]);
				}
			}
		}
		for (; i < num_particles_p; i++) {
			AUTO p = iter.particles[i];
			for (int j = 0; j < num_dimensions_d; j++) {
				p.inputs[j] = this->GetRandomAtDimension(j);
			}
		}
	};
	cweeStr OptimizationType() override final {
		return cweeStr(cweeAny::TypeNameOf<decltype(this)>());
	};
	ParticleType BestParticle() override final {
		return this->BestParticleImpl(minimize);
	};
	cweeThreadedList<ParticleType> BestParticles(int num) override final {
		return this->BestParticlesImpl(num, minimize);
	};
};
template <int num_dimensions_d, int num_particles_p, bool minimize = true> class Genetic_OptimizationManagementTool final : public OptimizationManagementTools<num_dimensions_d, num_particles_p> {
public:
	typedef OptimizationManagementTools<num_dimensions_d, num_particles_p> ParentType;
	typedef typename ParentType::ParticleHistory ParticleType;
	typedef typename ParentType::IterationHistory IterationType;

	Genetic_OptimizationManagementTool() : ParentType(), num_elites(2), percent_recombination(0.4f) {};
	Genetic_OptimizationManagementTool(ParentType const& other) : ParentType(other.details), num_elites(2), percent_recombination(0.4f) {};
	Genetic_OptimizationManagementTool(Genetic_OptimizationManagementTool const& other) : ParentType(other.details), num_elites(other.num_elites), percent_recombination(other.percent_recombination) {};
	Genetic_OptimizationManagementTool(cweeSharedPtr< ParentType::OptimizationManagementTools_Details > other) : ParentType(other), num_elites(2), percent_recombination(0.4f) {};
	~Genetic_OptimizationManagementTool() {};

	enum class Genetic_modes { ELITE, RECOM, MUTANT, NO_GEN_SETTING };
	int num_elites = 3; // number of elites to use for recombination as parents. Higher values = more exploration. Lower values = more exploitation.
	u64 percent_recombination = 0.4f; // percent of population that are descendants of the selected # of elites. Higher values = more exploitation. Lower values = more exploration.
	u64 percent_newtonSearch = 0.2f;

public:
	void StartIteration(IterationType& iter) override final {
		iter.optimizationType = OptimizationType();
		if (iter.iterationNumber <= 1) {
			Random_OptimizationManagementTool<num_dimensions_d, num_particles_p, minimize>(*this).StartIteration(iter); // initialize with all random
		}
		else {
			AUTO elites = this->BestParticles(num_elites); // top # of elites from all previous generations
			int numElites = elites.Num();
			int numRecombine = cweeMath::max(0, ((u64)(num_particles_p - numElites)) * percent_recombination);
			int numNewton = cweeMath::max(0, (num_particles_p - (numElites + numRecombine)) * percent_newtonSearch);

			int i = 0;
			for (; i < numElites; i++) { // elite reinsertion
				AUTO p = iter.particles[i];
				p = elites[i];
				p.tag(this) = Genetic_modes::ELITE;
			}
			for (; i < (numElites + (numRecombine / 2.0f)); i++) { // elite recombination with structured mutation
				AUTO p = iter.particles[i];
				for (int j = 0; j < num_dimensions_d; j++) {
					auto& elite1 = elites[cweeRandomInt(0, numElites - 1)]; // select a random elite as the first "parent" for this dimension
					auto& elite2 = elites[cweeRandomInt(0, numElites - 1)]; // select a random elite as the second "parent" for this dimension
					p.inputs[j] = cweeRandomFloat(elite1.inputs[j], elite2.inputs[j]);
				}
				p.tag(this) = Genetic_modes::RECOM;
			}
			for (; i < numElites + numRecombine; i++) { // elite recombination with random mutation
				AUTO p = iter.particles[i];
				for (int j = 0; j < num_dimensions_d; j++) {
					auto& elite1 = elites[cweeRandomInt(0, numElites - 1)]; // select a random elite as the first "parent" for this dimension					
					p.inputs[j] = cweeRandomFloat(elite1.inputs[j], this->GetRandomAtDimension(j)); // second parent is random generation
				}
				p.tag(this) = Genetic_modes::RECOM;
			}
			if (iter.iterationNumber > 2) {
				for (; i < numElites + numRecombine + numNewton; i++) { // newtonian searches
					AUTO p = iter.particles[i];
					for (int j = 0; j < num_dimensions_d; j++) {
						p.inputs[j] = this->GetRecommendedDimensionValue(j, elites[cweeRandomInt(0, numElites - 1)].inputs[j]);
					}
					p.tag(this) = Genetic_modes::MUTANT;
				}
			}
			for (; i < num_particles_p; i++) { // pure mutation without elites
				AUTO p = iter.particles[i];
				for (int j = 0; j < num_dimensions_d; j++) {
					p.inputs[j] = this->GetRandomAtDimension(j);
				}
				p.tag(this) = Genetic_modes::MUTANT;
			}
		}
	};
	cweeStr OptimizationType() override final {
		return cweeStr(cweeAny::TypeNameOf<decltype(this)>());
	};
	ParticleType BestParticle() override final {
		return this->BestParticleImpl(minimize);
	};
	cweeThreadedList<ParticleType> BestParticles(int num) override final {
		return this->BestParticlesImpl(num, minimize);
	};
};
template <int num_dimensions_d, int num_particles_p, class type1, class type2> class Alternating_OptimizationManagementTool final : public OptimizationManagementTools<num_dimensions_d, num_particles_p> {
public:
	typedef OptimizationManagementTools<num_dimensions_d, num_particles_p> ParentType;
	typedef typename ParentType::ParticleHistory ParticleType;
	typedef typename ParentType::IterationHistory IterationType;

	Alternating_OptimizationManagementTool() : ParentType(), optManagementTool1(this->details), optManagementTool2(this->details) {};
	Alternating_OptimizationManagementTool(ParentType const& other) : ParentType(other.details), optManagementTool1(other.details), optManagementTool2(other.details) {};
	Alternating_OptimizationManagementTool(cweeSharedPtr< ParentType::OptimizationManagementTools_Details > other) : ParentType(other), optManagementTool1(other), optManagementTool2(other) {};
	~Alternating_OptimizationManagementTool() {};
	void StartIteration(ParentType::IterationHistory& iter) override final {
		iter.optimizationType = OptimizationType();
		if (iter.iterationNumber % 2 != 0) { // starts odd with iterationNumber == 1
			optManagementTool1.StartIteration(iter);
		}
		else
		{
			optManagementTool2.StartIteration(iter);
		}
	};
	cweeStr OptimizationType() override final {
		return cweeStr(cweeAny::TypeNameOf<decltype(this)>());
	};

	ParticleType BestParticle() override final {
		if (this->num_iterations() > 0) {
			if (this->CurrentIteration().iterationNumber % 2 != 0) { // starts odd with iterationNumber == 1
				return optManagementTool1.BestParticle();
			}
			else
			{
				return optManagementTool2.BestParticle();
			}
		}
		else {
			return ParticleType();
		}
	};
	cweeThreadedList<ParticleType> BestParticles(int num) override final {
		if (this->num_iterations() > 0) {
			if (this->CurrentIteration().iterationNumber % 2 != 0) { // starts odd with iterationNumber == 1
				return optManagementTool1.BestParticles(num);
			}
			else
			{
				return optManagementTool2.BestParticles(num);
			}
		}
		else {
			return cweeThreadedList<ParticleType>();
		}
	};

private:
	type1 optManagementTool1;
	type2 optManagementTool2;
};
#pragma endregion

#pragma region "cweeOptimizer"
/* Static class that allows user to queue an optimization asynchronously. */
class cweeOptimizer {
public:
	/* Queue and perform optimization in async based on provided parameters and functions. */
	template<template<typename> class sharedPtrType, class OptType, typename element> static cweeJob run_optimization(sharedPtrType<element> shared, OptType const& manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(element&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations = 100, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
		return run_optimization_detail(std::move(shared), manager, EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
	};
private:
	/*  */
	template<template<typename> class sharedPtrType, template<int, int> class OptType, typename element, int num_dimensions_d, int num_particles_p> static cweeJob run_optimization_detail(sharedPtrType<element> shared, OptType<num_dimensions_d, num_particles_p> const& manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(element&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
		return run_optimization_impl<num_dimensions_d, num_particles_p, element, OptType<num_dimensions_d, num_particles_p>>(std::move(shared), manager, EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
	};
	/* Random_OptimizationManagementTool && Genetic_OptimizationManagementTool */
	template<template<typename> class sharedPtrType, template<int, int, bool> class OptType, typename element, int num_dimensions_d, int num_particles_p, bool minimize> static cweeJob run_optimization_detail(sharedPtrType<element> shared, OptType<num_dimensions_d, num_particles_p, minimize> const& manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(element&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
		return run_optimization_impl<num_dimensions_d, num_particles_p, element, OptType<num_dimensions_d, num_particles_p, minimize>>(std::move(shared), manager, EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
	};
	/* Alternating_OptimizationManagementTool */
	template<template<typename> class sharedPtrType, template<int, int, class, class> class OptType, typename element, int num_dimensions_d, int num_particles_p, class type1, class type2> static cweeJob run_optimization_detail(sharedPtrType<element> shared, OptType<num_dimensions_d, num_particles_p, type1, type2> const& manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(element&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
		return run_optimization_impl<num_dimensions_d, num_particles_p, element, OptType<num_dimensions_d, num_particles_p, type1, type2>>(std::move(shared), manager, EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
	};
	/* Actual implimentation of the job queue, utilizing the multi-job scheduler. */
	template<int num_dimensions_d, int num_particles_p, typename shared_data, typename providedManagerType>	static cweeJob run_optimization_impl(cweeSharedPtr<shared_data> sharedData, cweeSharedPtr<providedManagerType> manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(shared_data&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
		static_assert(num_dimensions_d >= 1, "Optimization must use at least one dimension");
		static_assert(num_particles_p >= 1, "Optimization must use at least one particle");

		cweeJob job_actual = run_optimization_impl_detail<num_dimensions_d, num_particles_p, shared_data, providedManagerType>(sharedData, manager, EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
		return job_actual;
	};
	/* Actual implimentation of the job queue, utilizing the multi-job scheduler. */
	template<int num_dimensions_d, int num_particles_p, typename shared_data, typename providedManagerType>	static cweeJob run_optimization_impl_detail(cweeSharedPtr<shared_data> sharedData, cweeSharedPtr<providedManagerType> manager, std::function<u64(cweeThreadedList<u64>&)> EvaluatePolicy, std::function<bool(shared_data&, cweeThreadedList<u64>&, u64)> StopOptimization, int maxIterations, cweeThreadedList<float> defaultPolicy = cweeThreadedList<float>()) {
		class cweeOptimizerData {
		public:
			cweeOptimizerData() : managementTool(), sharedData(make_cwee_shared<shared_data>()) {};
			cweeOptimizerData(cweeSharedPtr<OptimizationManagementTools<num_dimensions_d, num_particles_p>> manager, cweeSharedPtr<shared_data> data) : managementTool(manager), sharedData(data) {};

			cweeSharedPtr<OptimizationManagementTools<num_dimensions_d, num_particles_p>> managementTool;
			cweeSharedPtr<shared_data> sharedData;

			u64 running_average_performance = 0;
			int num_iterations = 0;
			int num_iterations_unimproved = 0;
		};
		class policy_data_type {
		public:
			typedef typename OptimizationManagementTools<num_dimensions_d, num_particles_p>::ParticleHistory policy_type;
			policy_type* policy;
		};
		std::function func1 = [defaultPolicy, maxIterations](cweeOptimizerData& shared) -> cweeThreadedList<policy_data_type> {
			cweeThreadedList<policy_data_type> out;
			if (shared.managementTool->num_iterations() < maxIterations) {
				AUTO iteration = shared.managementTool->AllocIteration();
				shared.managementTool->StartIteration(iteration);
				for (auto& x : iteration.particles) {
					policy_data_type p; p.policy = &x;
					out.Append(p);
				}
				if (iteration.iterationNumber <= 1 && defaultPolicy.Num() == num_dimensions_d) {
					out[0].policy->inputs = defaultPolicy; // will auto-cast float to u64
				}
			}
			return out;
		};
		std::function func2 = [EvaluatePolicy](policy_data_type& policy) -> void {	policy.policy->performance = EvaluatePolicy(policy.policy->inputs);	};
		std::function func3 = [](cweeOptimizerData& shared, cweeThreadedList<policy_data_type>& policies) -> void { 
			/* Optional - do something with the collection of all policies analyzed this iteration */

			/* Tighten the contraints as the optimization proceeds. Exploitation >> Exploration as the iterations get up in age. */
			//cweeThreadedList<u64> dimension_avg;
			//{
			//	AUTO bestParticles = shared.managementTool->BestParticles(10);
			//	for (int dim_n = 0; dim_n < shared.managementTool->num_dimensions(); dim_n++) {
			//		dimension_avg.Append(0); int n = 0;
			//		for (auto& part : bestParticles) {
			//			cweeMath::rollingAverageRef<u64>(dimension_avg[dim_n], part.inputs[dim_n], n);
			//		}
			//	}
			//}
			//// for each dimension...
			//for (int dim_n = 0; dim_n < shared.managementTool->num_dimensions(); dim_n++) {
			//	// ...move the contraints towards the average
			//	shared.managementTool->lower_constraints()[dim_n] = cweeMath::Lerp<u64>(shared.managementTool->lower_constraints()[dim_n], dimension_avg[dim_n], 0.1);
			//	shared.managementTool->upper_constraints()[dim_n] = cweeMath::Lerp<u64>(shared.managementTool->upper_constraints()[dim_n], dimension_avg[dim_n], 0.1);
			//}

		};
		std::function func4 = [=](cweeOptimizerData& shared) -> bool { /* End of iteration */
			AUTO ptr = shared.managementTool->BestParticle();
			u64 p = ptr.performance;
			u64 prevA = shared.running_average_performance;
			//cweeMath::rollingAverageRef(shared.running_average_performance, p, shared.num_iterations);
			//if (cweeMath::Pow64(prevA - shared.running_average_performance, 2.0f) < 0.001f) {
			//	shared.num_iterations_unimproved++;
			//}
			//else {
			//	shared.num_iterations_unimproved = 0;
			//}
			//if (shared.num_iterations_unimproved > (maxIterations / 20)) {
			//	return true; // stop after no improvement in 5% of iteration time
			//}
			return StopOptimization(*shared.sharedData, ptr.inputs, EvaluatePolicy(ptr.inputs));
		};
		//if constexpr (run_async) {
		return cweeScheduler<cweeOptimizerData, policy_data_type>::run_async(make_cwee_shared<cweeOptimizerData>(manager.CastReference<OptimizationManagementTools<num_dimensions_d, num_particles_p>>(), std::move(sharedData)), func1, func2, func3, func4);
		//}
		//else {
			//return cweeScheduler<cweeOptimizerData, policy_data_type>::run(make_cwee_shared<cweeOptimizerData>(manager.CastReference<OptimizationManagementTools<num_dimensions_d, num_particles_p>>(), std::move(sharedData)), func1, func2, func3, func4);
		//}		
	};

#define OPT_FUNC_DIM(NUMDIM, NUMPOLICIES, _min_) case (NUMDIM) : { \
		Genetic_OptimizationManagementTool<NUMDIM, NUMPOLICIES, _min_> ramt; \
		ramt.lower_constraints() = lowerBound; \
		ramt.upper_constraints() = upperBound; \
		toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations); \
		break; \
	}
public:
	class sharedClass {
	public:
		class sharedClassInternal {
		public:
			cweeThreadedList<float> bestPolicy;
			float bestPerformance;
		};
		cweeThreadedList<sharedClassInternal> results;
	};



#if 0
	static chaiscript::small_vector<chaiscript::Boxed_Value>	Scripting_OptimizeFunction(bool minimize, const chaiscript::Boxed_Value& per_sample_Function, chaiscript::small_vector<chaiscript::Boxed_Value> const& lowBound_Vector, chaiscript::small_vector<chaiscript::Boxed_Value> const& highBound_Vector) {
		cweeThreadedList<float> lowerBound, upperBound;
		for (auto& bv : lowBound_Vector) lowerBound.push_back(chai.boxed_cast<float>(bv));
		for (auto& bv : highBound_Vector) upperBound.push_back(chai.boxed_cast<float>(bv));
		AUTO todo = [=](cweeThreadedList<float> const& x)->float {
			chaiscript::small_vector< chaiscript::Boxed_Value > bv;
			for (auto& v : x) { bv.push_back(chaiscript::var((float)v)); }
			return chai.boxed_cast<float>(chai.call_function(per_sample_Function, chaiscript::var(bv)));
		};

		int numDimensions = cweeMath::min(lowerBound.Num(), upperBound.Num());
		constexpr int maxIterations = 1000; // 1000 iterations
		constexpr float eps = 0.00001f;

		static std::function objFunc = [=](cweeThreadedList<u64>& policy) -> u64 {
			return cweeMath::Fabs(todo(policy[0]));
		};
		static std::function isFinishedFunc = [=](sharedClass& shared, cweeThreadedList<u64>& bestPolicy, u64 bestPerformance) -> bool {
			auto& i = shared.results.Alloc();
			i.bestPolicy = bestPolicy;
			i.bestPerformance = bestPerformance;
			return bestPerformance <= eps;
		};

		cweeSharedPtr<sharedClass> shared = make_cwee_shared<sharedClass>();
		cweeJob toAwait;
		if (minimize) {
			switch (numDimensions) {
				OPT_FUNC_DIM(1, true)
					OPT_FUNC_DIM(2, true)
					OPT_FUNC_DIM(3, true)
					OPT_FUNC_DIM(4, true)
					OPT_FUNC_DIM(5, true)
					OPT_FUNC_DIM(6, true)
					OPT_FUNC_DIM(7, true)
					OPT_FUNC_DIM(8, true)
					OPT_FUNC_DIM(9, true)
					OPT_FUNC_DIM(10, true)
					OPT_FUNC_DIM(11, true)
					OPT_FUNC_DIM(12, true)
					OPT_FUNC_DIM(13, true)
					OPT_FUNC_DIM(14, true)
					OPT_FUNC_DIM(15, true)
					OPT_FUNC_DIM(16, true)
					OPT_FUNC_DIM(17, true)
					OPT_FUNC_DIM(18, true)
					OPT_FUNC_DIM(19, true)
					OPT_FUNC_DIM(20, true)
			}
		}
		else {
			switch (numDimensions) {
				OPT_FUNC_DIM(1, false)
					OPT_FUNC_DIM(2, false)
					OPT_FUNC_DIM(3, false)
					OPT_FUNC_DIM(4, false)
					OPT_FUNC_DIM(5, false)
					OPT_FUNC_DIM(6, false)
					OPT_FUNC_DIM(7, false)
					OPT_FUNC_DIM(8, false)
					OPT_FUNC_DIM(9, false)
					OPT_FUNC_DIM(10, false)
					OPT_FUNC_DIM(11, false)
					OPT_FUNC_DIM(12, false)
					OPT_FUNC_DIM(13, false)
					OPT_FUNC_DIM(14, false)
					OPT_FUNC_DIM(15, false)
					OPT_FUNC_DIM(16, false)
					OPT_FUNC_DIM(17, false)
					OPT_FUNC_DIM(18, false)
					OPT_FUNC_DIM(19, false)
					OPT_FUNC_DIM(20, false)
			}
		}
		toAwait.AwaitAll();

		chaiscript::small_vector< chaiscript::Boxed_Value > bv_final;
		{
			auto& bestPolicy = shared->results[shared->results.Num() - 1].bestPolicy;
			for (auto& v_final : bestPolicy) { bv_final.push_back(chaiscript::var((float)v_final)); }
		}
		return bv_final;
	};
#undef OPT_FUNC_DIM

	static void		AppendToScriptingLanguage(chaiscript::ChaiScript& scriptingLanguage) {
		scriptingLanguage.add(chaiscript::fun<std::function< std::vector<chaiscript::Boxed_Value>(bool, const chaiscript::Boxed_Value&, std::vector<chaiscript::Boxed_Value> const&, std::vector<chaiscript::Boxed_Value> const&) >>([](
			bool minimize, const chaiscript::Boxed_Value& per_sample_Function, std::vector<chaiscript::Boxed_Value> const& lowBound_Vector, std::vector<chaiscript::Boxed_Value> const& highBound_Vector)
			{
				return Scripting_OptimizeFunction(minimize, per_sample_Function, lowBound_Vector, highBound_Vector);
			}), "Scripting_OptimizeFunction");

		scriptingLanguage.eval(R"(
			def cweeMinimize(Function per_sample, Vector lowBound, Vector highBound){
				if (lowBound.size() <= 0 || highBound.size() <= 0){ throw("Cannot optimize when constraint dimensions are 0."); }
				try{ for (j : lowBound) { `==`(j, 0.0f); } }catch{ throw("Lower constraint must only contain numbers.");  }
				try{ for (j : highBound) { `==`(j, 0.0f); } }catch{ throw("Upper constraint must only contain numbers.");  }				
				try{ per_sample(lowBound) == 0.0f; }catch{ throw("Sample Function must return a number, and must accept a vector of potential values of equal size to your lower constraints."); }
				try{ per_sample(highBound) == 0.0f; }catch{ throw("Sample Function must return a number, and must accept a vector of potential values of equal size to your upper constraints."); }
				try{ per_sample(lowBound) != null; }catch{ throw("Sample Function must return a number, and must accept a vector of potential values of equal size to your lower constraints."); }
				try{ per_sample(highBound) != null; }catch{ throw("Sample Function must return a number, and must accept a vector of potential values of equal size to your upper constraints."); }				
				
				return Scripting_OptimizeFunction(true, per_sample, lowBound, highBound);	
			};
			def cweeMaximize(Function per_sample, Vector lowBound, Vector highBound){
				if (lowBound.size() <= 0 || highBound.size() <= 0){ throw("Cannot optimize when constraint dimensions are 0."); }
				try{ for (j : lowBound) { `==`(j, 0.0f); } }catch{ throw("Lower constraint must only contain numbers.");  }
				try{ for (j : highBound) { `==`(j, 0.0f); } }catch{ throw("Upper constraint must only contain numbers.");  }				
				try{ per_sample(lowBound) == 0.0f; }catch{ throw("Sample Function must return a number, and must accept a vector of potential values of equal size to your lower constraints."); }
				try{ per_sample(highBound) == 0.0f; }catch{ throw("Sample Function must return a number, and must accept a vector of potential values of equal size to your upper constraints."); }
				try{ per_sample(lowBound) != null; }catch{ throw("Sample Function must return a number, and must accept a vector of potential values of equal size to your lower constraints."); }
				try{ per_sample(highBound) != null; }catch{ throw("Sample Function must return a number, and must accept a vector of potential values of equal size to your upper constraints."); }				
				
				return Scripting_OptimizeFunction(false, per_sample, lowBound, highBound);			
			};
		)");
	};
#endif
};

#pragma endregion