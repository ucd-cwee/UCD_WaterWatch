/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#include "Precompiled.h"
#include "SharedPtr.h"
#include "List.h"
#include "cweeJob.h"
#include "cweeAny.h"
#include "Curve.h"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>


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
			else {
				for (int i = 0; i < schedule.Num(); i++) {
					AsyncInvoke(cweeJob([](cweeThreadedList<policy_data>& policies, int which, cweeSharedPtr<cweeSchedulerData> in3, cweeSysInterlockedInteger& count, cweeJob& scheduler, cweeJob& finalJob3) {
						in3->m_EvaluatePolicy(policies[which]); // evaluate 1 of n policies...
						if (count.Increment() >= policies.Num()) { // after we have finished all policies..
							in3->m_FinishIteration(*(in3->m_sharedData.Get()), policies); // first, evaluate policies, then...
							if (!in3->m_ReviewStopConditions(*(in3->m_sharedData.Get()))) AsyncInvoke(scheduler); // loop again, or...
							else AsyncInvoke(finalJob3); // end optimization.
						}
						}, schedulerResult, i, in2, counter, schedulerResults, finalJob2));
				}
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
		j.ForceInvoke(); // Async
		return j;
	};
	static cweeJob& Invoke(cweeJob&& j) {
		j.ForceInvoke(); // Async
		return j;
	};

};
#pragma endregion

#pragma region "OptimizationManagementTools"
#define useOptimizedMatrixForCweeOptimization
template <typename returnType = u64> class OptimizationManagementTools {
public:
	u64 default_constraint() const noexcept { return cweeMath::Pow64(cweeMath::INF, 0.25f) / num_dimensions(); };
	static constexpr u64 default_constraint(int num_dimensions) { return cweeMath::Pow64(cweeMath::INF, 0.25f) / num_dimensions; };
	static constexpr u64 default_performance() { return cweeMath::INF; };

	int num_dimensions_d, num_particles_p;

	class ParticleHistory {
	private:
		int num_dimensions_d, num_particles_p;
	public:
		int num_dimensions()  const noexcept { return num_dimensions_d; };
		int num_particles()  const noexcept { return num_particles_p; };
		cweeAny& tag(void* p) { return *tags[p]; };
		cweeAny& tag() { return *tags[(void*)this]; };
		ParticleHistory(int numDimensions = 1, int numParticles = 1) : num_dimensions_d(numDimensions), num_particles_p(numParticles) { inputs.AssureSize(num_dimensions(), 0); };
		bool ParticleLikelyProcessed() { return performance != OptimizationManagementTools<>::default_performance(); };

	public:
		cweeThreadedList<u64> inputs;
		u64 performance = OptimizationManagementTools<>::default_performance();
	protected: // data
		cweeThreadedMap<void*, cweeAny> tags;
		friend class OptimizationManagementTools;
		friend class IterationHistory;
		friend class OptimizationManagementTools_Details;
	};
	class IterationHistory {
	private:
		int num_dimensions_d, num_particles_p;

	public:
		int num_dimensions()  const noexcept { return num_dimensions_d; };
		int num_particles()  const noexcept { return num_particles_p; };
		ParticleHistory BestParticle(bool FindMinimum = true) {
			ParticleHistory out;
			u64 performance = FindMinimum ? OptimizationManagementTools<>::default_performance() : -OptimizationManagementTools<>::default_performance();
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
		IterationHistory(int numDimensions = 1, int numParticles = 1) : num_dimensions_d(numDimensions), num_particles_p(numParticles) { 
			particles.AssureSize(num_particles_p, ParticleHistory(num_dimensions_d, num_particles_p));
		};
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
	private:
		int num_dimensions_d, num_particles_p;

	public:
		int num_dimensions()  const noexcept { return num_dimensions_d; };
		int num_particles()  const noexcept { return num_particles_p; };
		OptimizationManagementTools_Details(int numDimensions = 1, int numParticles = 1) : num_dimensions_d(numDimensions), num_particles_p(numParticles) {
			// assert(num_dimensions_d >= 1, "OptimizationManagementTools_Details requires the number of dimensions in optimization problem to be equal to or greater than 1. Otherwise there is no need for an optimization as nothing can be tested.");
			upper_constraints.AssureSize(num_dimensions_d, OptimizationManagementTools<>::default_constraint(num_dimensions_d));
			lower_constraints.AssureSize(num_dimensions_d, -OptimizationManagementTools<>::default_constraint(num_dimensions_d));
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

public: // data access methods
	cweeThreadedList<Curve>& performances() { return details->performances; };
	auto& upper_constraints() { return details->upper_constraints; };
	auto& lower_constraints() { return details->lower_constraints; };
	auto& iterations() { return details->iterations; };
	auto& iterations_0_primary() { return details->iterations_0_primary; };
	cweeAny& tag(void* p) { return details->tag(p); };
	cweeAny& tag() { return details->tag((void*)this); };

public:
	int num_dimensions()  const noexcept { return num_dimensions_d; };
	int num_particles()  const noexcept { return num_particles_p; };
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
			out = &iterations().Alloc(IterationHistory(num_dimensions(), num_particles()));
			out->iterationNumber = iterations().Num();
		}
		else if (iterations().Num() == 1) {
			iterations_0_primary() = false;
			out = &iterations().Alloc(IterationHistory(num_dimensions(), num_particles()));
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
	virtual void StartIteration(OptimizationManagementTools::IterationHistory& iter) = 0;
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
		u64 performance = FindMinimum ? OptimizationManagementTools<>::default_performance() : -OptimizationManagementTools<>::default_performance();
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

public: // construct / destroy
	OptimizationManagementTools(int numDimensions = 1, int numParticles = 1) : num_dimensions_d(numDimensions), num_particles_p(numParticles), details(make_cwee_shared<OptimizationManagementTools_Details>(numDimensions, numParticles)) {
		//assert(num_dimensions_d > 0, "Cannot have less than one dimension in optimization");
		//assert(num_particles_p > 0, "Cannot have less than one particle in optimization");
	};
	OptimizationManagementTools(cweeSharedPtr< OptimizationManagementTools_Details > otherDetails, int numDimensions = 1, int numParticles = 1) : num_dimensions_d(numDimensions), num_particles_p(numParticles), details(otherDetails) {
		//assert(num_dimensions_d > 0, "Cannot have less than one dimension in optimization");
		//assert(num_particles_p > 0, "Cannot have less than one particle in optimization");
	};
	virtual ~OptimizationManagementTools() {};
};

template <bool minimize = true> class Random_OptimizationManagementTool final : public OptimizationManagementTools<> {
public:
	typedef OptimizationManagementTools<> ParentType;
	typedef typename ParentType::ParticleHistory ParticleType;
	typedef typename ParentType::IterationHistory IterationType;

	Random_OptimizationManagementTool(int numDimensions = 1, int numParticles = 1) : ParentType(numDimensions, numParticles) {};
	Random_OptimizationManagementTool(ParentType const& other, int numDimensions = 1, int numParticles = 1) : ParentType(other.details, numDimensions, numParticles) {};
	Random_OptimizationManagementTool(cweeSharedPtr< ParentType::OptimizationManagementTools_Details > other, int numDimensions = 1, int numParticles = 1) : ParentType(other, numDimensions, numParticles) {};
	~Random_OptimizationManagementTool() {};

	u64 percent_newtonSearch = 0.2f;

	void StartIteration(ParentType::IterationHistory& iter) override final {
		int numRecombine = cweeMath::max(0, ((u64)(this->num_particles())) * percent_newtonSearch);
		int i = 0;
		if (iter.iterationNumber > 1) {
			iter.particles[i] = BestParticle(); i++;
		}
		if (iter.iterationNumber > 2) {
			AUTO part = BestParticle();
			for (; i < numRecombine; i++) {
				AUTO p = iter.particles[i];
				for (int j = 0; j < this->num_dimensions(); j++) {
					p.inputs[j] = this->GetRecommendedDimensionValue(j, part.inputs[j]);
				}
			}
		}
		for (; i < this->num_particles(); i++) {
			AUTO p = iter.particles[i];
			for (int j = 0; j < this->num_dimensions(); j++) {
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

template <bool minimize = true> class Genetic_OptimizationManagementTool final : public OptimizationManagementTools<> {
public:
	typedef OptimizationManagementTools<> ParentType;
	typedef typename ParentType::ParticleHistory ParticleType;
	typedef typename ParentType::IterationHistory IterationType;

	Genetic_OptimizationManagementTool(int numDimensions = 1, int numParticles = 1) : ParentType(numDimensions, numParticles), num_elites(std::max(2, numParticles / 10)), percent_recombination(0.4f), percent_newtonSearch(0.2f), percent_mutation(0.2f) {};
	Genetic_OptimizationManagementTool(ParentType const& other, int numDimensions = 1, int numParticles = 1) : ParentType(other.details, numDimensions, numParticles), num_elites(std::max(2, numParticles / 10)), percent_recombination(0.4f), percent_newtonSearch(0.2f), percent_mutation(0.2f) {};
	Genetic_OptimizationManagementTool(Genetic_OptimizationManagementTool const& other, int numDimensions = 1, int numParticles = 1) : ParentType(other.details, numDimensions, numParticles), num_elites(other.num_elites), percent_recombination(other.percent_recombination), percent_newtonSearch(other.percent_newtonSearch), percent_mutation(other.percent_mutation) {};
	Genetic_OptimizationManagementTool(cweeSharedPtr< ParentType::OptimizationManagementTools_Details > other, int numDimensions = 1, int numParticles = 1) : ParentType(other, numDimensions, numParticles), num_elites(std::max(2, numParticles / 10)), percent_recombination(0.4f), percent_newtonSearch(0.2f), percent_mutation(0.2f) {};
	~Genetic_OptimizationManagementTool() {};

	enum class Genetic_modes { ELITE, RECOM, MUTANT, NO_GEN_SETTING };
	int num_elites; // number of elites to use for recombination as parents. Higher values = more exploration. Lower values = more exploitation.
	u64 percent_recombination; // percent of population that are descendants of the selected # of elites. Higher values = more exploitation. Lower values = more exploration.
	u64 percent_newtonSearch;
	u64 percent_mutation;
public:
	void StartIteration(IterationType& iter) override final {
		iter.optimizationType = OptimizationType();
		if (iter.iterationNumber <= 1) {
			Random_OptimizationManagementTool<minimize>(*this, this->num_dimensions(), this->num_particles()).StartIteration(iter); // initialize with all random
		}
		else {
			AUTO elites = this->BestParticles(num_elites); // top # of elites from all previous generations
			int numElites = elites.Num();
			int numRecombine = cweeMath::max(0, ((u64)(num_particles() - numElites)) * percent_recombination);
			int numNewton = cweeMath::max(0, (num_particles() - (numElites + numRecombine)) * percent_newtonSearch);

			int i = 0;
			for (; i < numElites; i++) { // elite reinsertion
				auto& p = iter.particles[i];
				p = elites[i];
				p.tag(this) = Genetic_modes::ELITE;
			}
			//for (; i < (numElites + (numRecombine / 2.0f)); i++) { // elite recombination with structured mutation
			//	AUTO p = iter.particles[i];
			//	// for each recombination...
			//	ParticleHistory* elite1, elite2;
			//	elite1 = &elites[cweeRandomInt(0, numElites - 1)]; // select a random elite as the first "parent" for this dimension
			//	elite2 = elite1;
			//	// make sure we get different parents
			//	while (elite2 == elite1) {
			//		elite2 = &elites[cweeRandomInt(0, numElites - 1)]; // select a random elite as the second "parent" for this dimension
			//	}
			//	int length = 0; int progress1 = 0; int progress2 = 0;
			//	while (length < this->num_dimensions()) {
			//		// choose a random length between 1 and what remains...
			//		auto len = cweeRandomFloat(1, this->num_dimensions() - length);
			//		// for a random parent...
			//		if (cweeRandomFloat(0.0, 1.0) >= 0.5) {
			//			for (int i = progress1; i < len + progress1; i++) {
			//				p.inputs[length] = elite1->inputs[i];
			//				length++;
			//			}
			//			progress1 += len;
			//		}
			//		else {
			//			for (int i = progress2; i < len + progress2; i++) {
			//				p.inputs[length] = elite2->inputs[i];
			//				length++;
			//			}
			//			progress2 += len;
			//		}
			//	}
			//	for (int j = 0; j < this->num_dimensions(); j++) {
			//		if (cweeRandomFloat(0.0, 1.0) < percent_mutation) {
			//			p.inputs[length] = this->GetRandomAtDimension(j)
			//		}
			//	}
			//	p.tag(this) = Genetic_modes::RECOM;
			//}
			for (; i < numElites + numRecombine; i++) { // elite recombination with random mutation
				auto& p = iter.particles[i];
				// for each recombination...
				ParticleHistory* elite1;
				ParticleHistory* elite2;
				elite1 = &elites[cweeRandomInt(0, numElites - 1)]; // select a random elite as the first "parent" for this dimension
				elite2 = elite1;
				// make sure we get different parents
				while (elite2 == elite1) {
					elite2 = &elites[cweeRandomInt(0, numElites - 1)]; // select a random elite as the second "parent" for this dimension
				}

				int length = 0; int progress1 = 0; int progress2 = 0;
				while (length < this->num_dimensions()) {
					// choose a random length between 1 and what remains...
					auto len = cweeRandomInt(1, this->num_dimensions() - length);
					// for a random parent...
					if (cweeRandomFloat(0.0, 1.0) >= 0.5) {
						for (int k = progress1; (k < len + progress1) && length < this->num_dimensions(); k++) {
							p.inputs[length] = elite1->inputs[k];
							length++;
						}
						progress1 += len;
					}
					else {
						for (int k = progress2; (k < len + progress2) && length < this->num_dimensions(); k++) {
							p.inputs[length] = elite2->inputs[k];
							length++;
						}
						progress2 += len;
					}
				}

				for (int j = 0; j < this->num_dimensions(); j++) {
					if (cweeRandomFloat(0.0, 1.0) < percent_mutation) {
						p.inputs[j] = this->GetRandomAtDimension(j);
					}
				}

				p.tag(this) = Genetic_modes::RECOM;
			}
			if (iter.iterationNumber > 2) {
				for (; i < numElites + numRecombine + numNewton; i++) { // newtonian searches
					auto& p = iter.particles[i];
					for (int j = 0; j < this->num_dimensions(); j++) {
						p.inputs[j] = this->GetRecommendedDimensionValue(j, elites[cweeRandomInt(0, numElites - 1)].inputs[j]);
					}
					p.tag(this) = Genetic_modes::MUTANT;
				}
			}
			for (; i < num_particles(); i++) { // pure mutation without elites
				auto& p = iter.particles[i];
				for (int j = 0; j < this->num_dimensions(); j++) {
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

template <bool minimize = true> class ParticleSwarm_OptimizationManagementTool final : public OptimizationManagementTools<> {
public:
	typedef OptimizationManagementTools<> ParentType;
	typedef typename ParentType::ParticleHistory ParticleType;
	typedef typename ParentType::IterationHistory IterationType;

	ParticleSwarm_OptimizationManagementTool(int numDimensions = 1, int numParticles = 1) : ParentType(numDimensions, numParticles) {};
	ParticleSwarm_OptimizationManagementTool(ParentType const& other, int numDimensions = 1, int numParticles = 1) : ParentType(other.details, numDimensions, numParticles) {};
	ParticleSwarm_OptimizationManagementTool(ParticleSwarm_OptimizationManagementTool const& other, int numDimensions = 1, int numParticles = 1) : ParentType(other.details, numDimensions, numParticles) {};
	ParticleSwarm_OptimizationManagementTool(cweeSharedPtr< ParentType::OptimizationManagementTools_Details > other, int numDimensions = 1, int numParticles = 1) : ParentType(other, numDimensions, numParticles) {};
	~ParticleSwarm_OptimizationManagementTool() {};

	/*!
	This code uses the k-means clustering algorithm to generate
	k clusters of n-dimensional points.
	The code generates random points in the space and then clusters them using the k-means clustering algorithm.
	Suggested to "expand" the points by 20% to reach the edges.
	*/
	INLINE static std::vector<std::vector<double>> kmeans(int k, int n) {
		int m = n * k * 10;
		std::vector<std::vector<double>> centers(k, std::vector<double>(n));
		std::vector<std::vector<double>> data(m, std::vector<double>(n));
		std::vector<int> labels(m);
		//random_device rd;
		//mt19937 gen(rd());
		//uniform_real_distribution<> dis(0.0, 1.0);
		for (int i = 0; i < m; ++i) {
			for (int j = 0; j < n; ++j) {
				data[i][j] = cweeRandomFloat(0, 1); // dis(gen);
			}
		}
		for (int i = 0; i < k; ++i) {
			centers[i] = data[i];
		}
		while (true) {
			std::vector<std::vector<double>> new_centers(k, std::vector<double>(n));
			std::vector<int> counts(k);
			for (int i = 0; i < m; ++i) {
				double min_dist = std::numeric_limits<double>::max();
				int label = -1;
				for (int j = 0; j < k; ++j) {
					double dist = 0;
					for (int l = 0; l < n; ++l) {
						dist += std::pow(data[i][l] - centers[j][l], 2);
					}
					if (dist < min_dist) {
						min_dist = dist;
						label = j;
					}
				}
				labels[i] = label;
				counts[label]++;
				for (int l = 0; l < n; ++l) {
					new_centers[label][l] += data[i][l];
				}
			}
			bool converged = true;
			for (int i = 0; i < k; ++i) {
				if (counts[i] == 0) {
					continue;
				}
				for (int l = 0; l < n; ++l) {
					new_centers[i][l] /= counts[i];
					if (new_centers[i][l] != centers[i][l]) {
						converged = false;
					}
					centers[i][l] = new_centers[i][l];
				}
			}
			if (converged) {
				break;
			}
		}
		return centers;
	};



	class IslandMemory {
	public:
		u64 performance;
		cweeList< u64 > position;
	};
	class Bird {
	public:
		IslandMemory BestIsland;

		IslandMemory CurrentIsland;
		cweeList< u64 > current_velocity;
	};

	cweeList< Bird > Birds;
	IslandMemory Best_Island;
	int NumBirds;
	float g_cor = 0.5f, 
		  l_cor = 0.5f, 
		  inertia = 0.5f, 
		  devianceChance = 0.1f,
		  percent_length_initial_velocity = 0.25f;
	bool use_random = true;

	void UpdateBirdVelocity(Bird& bird, IslandMemory& bestIsland) {
		cweeList< u64 >& previousLocation = bird.CurrentIsland.position;
		cweeList< u64 >& previous_velocity = bird.current_velocity;
		cweeList< u64 >& bestLocalLocation = bird.BestIsland.position;
		cweeList< u64 >& bestGlobalLocation = bestIsland.position;

		cweeList< u64 > new_velocity;
		for (int dim = 0; dim < this->num_dimensions(); dim++) {
			float random_loc = cweeRandomFloat(0, 1), random_global = cweeRandomFloat(0, 1);

			float vel_comp = 
				inertia * previous_velocity[dim]
				+ random_loc * l_cor * (bestLocalLocation[dim] - previousLocation[dim])
				+ random_global * g_cor * (bestGlobalLocation[dim] - previousLocation[dim]);

			if (cweeRandomFloat(0, 1) < devianceChance) {
				// this bird is a bit of a rebel and is willing to deviate it's path somewhat in the hopes of discovery of a nearby but better island
				vel_comp *= cweeRandomFloat(cweeRandomFloat(0.8f, 1.0f), cweeRandomFloat(1.0f, 1.2f));
			}

			new_velocity.Append(vel_comp);
		}
		
		bird.current_velocity = new_velocity;
	};
	void UpdateBirdPosition(Bird& bird) {
		for (int dim = 0; dim < this->num_dimensions(); dim++) {
			bird.CurrentIsland.position[dim] += bird.current_velocity[dim];
		}
		// ensure the birds do not exceed the bounds of the studying
		auto& upper = this->upper_constraints();
		auto& lower = this->lower_constraints();
		for (int dim = 0; dim < this->num_dimensions(); dim++) {
			if (bird.CurrentIsland.position[dim] > upper[dim]) {
				bird.CurrentIsland.position[dim] = upper[dim];
			}
			if (bird.CurrentIsland.position[dim] < lower[dim]) {
				bird.CurrentIsland.position[dim] = lower[dim];
			}
		}
	};

public:
	void StartIteration(IterationType& iter) override final {
		iter.optimizationType = OptimizationType();

		if (Birds.Num() < this->num_particles() - 1) {
			NumBirds = this->num_particles() - 1;

			auto centers = kmeans(NumBirds, this->num_dimensions());

			// establish our birds!
			int i = Birds.Num();
			Birds.AssureSize(NumBirds, Bird());

			cweeList< u64 > defaultVelocity; defaultVelocity.AssureSize(this->num_dimensions(), 0);
			auto& upper = this->upper_constraints();
			auto& lower = this->lower_constraints();
			for (; i < NumBirds; i++) {
				cweeList< u64 > defaultPosition; 
				for (int p = 0; p < this->num_dimensions(); p++) {
					defaultPosition.Append(cweeMath::Lerp<u64>(lower[p], upper[p], centers[i][p]));
				}				
				Bird& bird = Birds[i]; {
					bird.current_velocity = defaultVelocity;
					bird.CurrentIsland.position = defaultPosition;
					if (minimize) {
						bird.CurrentIsland.performance = this->default_performance();
					}
					else {
						bird.CurrentIsland.performance = -this->default_performance();
					}
					bird.BestIsland = bird.CurrentIsland;
				}
			}

			Best_Island.position = defaultVelocity;
			if (minimize) {
				Best_Island.performance = this->default_performance();
			}
			else {
				Best_Island.performance = -this->default_performance();
			}
		}

		auto& iters = this->iterations();
		if (iter.iterationNumber <= 1 || iters.Num() <= 1) {
			for (int i = 0; i < NumBirds; i++) {
				auto& bird = Birds[i];
				for (int n = 0; n < this->num_dimensions(); n++) {
					iter.particles[i].inputs[n] = bird.CurrentIsland.position[n];
				}
			}
			// the final particle holds the "best" particle to allow convergence -- make it random for now.
			for (int n = 0; n < this->num_dimensions(); n++) {
				iter.particles[NumBirds].inputs[n] = this->GetRandomAtDimension(n);
			}
		}
		else {
			// get the previous iteration(s)
			IterationType& prevIter = iters[iters.Num() - 2];
			auto& prevIterParticles = prevIter.particles;

			// update the bird's best known and current islands
			for (int i = 0; i < NumBirds; i++) {
				ParticleType& prevParticle = prevIterParticles[i];
				auto& bird = Birds[i];

				bird.CurrentIsland.performance = prevParticle.performance;

				// if the previous iteration produced the best island this bird ever saw, record it.
				if (minimize) {
					if (bird.CurrentIsland.performance < bird.BestIsland.performance) {
						bird.BestIsland = bird.CurrentIsland;
					}
				}
				else {
					if (bird.CurrentIsland.performance > bird.BestIsland.performance) {
						bird.BestIsland = bird.CurrentIsland;
					}
				}
			}

			// update the bird's collectively best known island
			for (int i = 0; i < NumBirds; i++) {
				auto& bird = Birds[i];
				if (minimize) {
					if (bird.BestIsland.performance < Best_Island.performance) {
						Best_Island = bird.BestIsland;
					}
				}
				else {
					if (bird.BestIsland.performance > Best_Island.performance) {
						Best_Island = bird.BestIsland;
					}
				}
			}

			// update the bird's velocities using their personal best islands and the best island
			for (int i = 0; i < NumBirds; i++) {
				auto& bird = Birds[i];
				UpdateBirdVelocity(bird, Best_Island);
			}

			// update the bird's positions using their velocities, and use them to set the next iteration's policies.
			for (int i = 0; i < NumBirds; i++) {
				auto& bird = Birds[i];
				UpdateBirdPosition(bird);
				for (int n = 0; n < this->num_dimensions(); n++) {
					iter.particles[i].inputs[n] = bird.CurrentIsland.position[n];
				}				
			}

			// the final particle holds the "best" particle to allow convergence
			iter.particles[NumBirds] = BestParticle();
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


template <class type1, class type2> class Alternating_OptimizationManagementTool final : public OptimizationManagementTools<> {
public:
	typedef OptimizationManagementTools<> ParentType;
	typedef typename ParentType::ParticleHistory ParticleType;
	typedef typename ParentType::IterationHistory IterationType;

	Alternating_OptimizationManagementTool(int numDimensions = 1, int numParticles = 1) : ParentType(numDimensions, numParticles), optManagementTool1(this->details, numDimensions, numParticles), optManagementTool2(this->details, numDimensions, numParticles) {};
	Alternating_OptimizationManagementTool(ParentType const& other, int numDimensions = 1, int numParticles = 1) : ParentType(other.details, numDimensions, numParticles), optManagementTool1(other.details, numDimensions, numParticles), optManagementTool2(other.details, numDimensions, numParticles) {};
	Alternating_OptimizationManagementTool(cweeSharedPtr< ParentType::OptimizationManagementTools_Details > other, int numDimensions = 1, int numParticles = 1) : ParentType(other, numDimensions, numParticles), optManagementTool1(other, numDimensions, numParticles), optManagementTool2(other, numDimensions, numParticles) {};
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
			return ParticleType(this->num_dimensions(), this->num_particles());
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
namespace cweeOptimizer {
	class sharedClass {
	public:
		class sharedClassInternal {
		public:			
			cweeThreadedList<float> bestPolicy;
			float bestPerformance;
		};
		cweeThreadedList<sharedClassInternal> results;
		
		cweeList<cweeList<float>> policies;
	};

	namespace { // effectively a private namespace
		/* Actual implimentation of the job queue, utilizing the multi-job scheduler. */
		template<typename shared_data, typename providedManagerType>
		cweeJob run_optimization_impl_detail(cweeSharedPtr<shared_data> sharedData, cweeSharedPtr<providedManagerType> manager, std::function<u64(cweeThreadedList<u64>&)> EvaluatePolicy, std::function<bool(shared_data&, cweeThreadedList<u64>&, u64)> StopOptimization, int maxIterations, cweeThreadedList<float> defaultPolicy = cweeThreadedList<float>()) {
			class cweeOptimizerData {
			public:
				cweeOptimizerData() : managementTool(), sharedData(make_cwee_shared<shared_data>()) {};
				cweeOptimizerData(cweeSharedPtr<OptimizationManagementTools<>> manager, cweeSharedPtr<shared_data> data) : managementTool(manager), sharedData(data) {};

				cweeSharedPtr<OptimizationManagementTools<>> managementTool;
				cweeSharedPtr<shared_data> sharedData;

				u64 running_average_performance = 0;
				int num_iterations = 0;
				int num_iterations_unimproved = 0;
			};
			class policy_data_type {
			public:
				typedef typename OptimizationManagementTools<>::ParticleHistory policy_type;
				policy_type* policy;
			};
			std::function func1 = [manager, defaultPolicy, maxIterations](cweeOptimizerData& shared) -> cweeThreadedList<policy_data_type> {
				cweeThreadedList<policy_data_type> out;
				try {
				if (shared.managementTool->num_iterations() < maxIterations) {
					AUTO iteration = shared.managementTool->AllocIteration();
					shared.managementTool->StartIteration(iteration);
					for (auto& x : iteration.particles) {
						policy_data_type p; p.policy = &x;
						out.Append(p);
					}
					if (iteration.iterationNumber <= 1 && defaultPolicy.Num() == manager->num_dimensions()) {
						out[0].policy->inputs = defaultPolicy; // will auto-cast float to u64
					}
				}
				}
				catch (...) {}
				return out;
			};
			std::function func2 = [EvaluatePolicy](policy_data_type& policy) -> void {
				try {
					AUTO p = EvaluatePolicy(policy.policy->inputs);
					policy.policy->performance = p;
				}catch(...){}				
			};
			std::function func3 = [](cweeOptimizerData& shared, cweeThreadedList<policy_data_type>& policies) -> void {
				try {
				for (policy_data_type& policy : policies) {
					cweeList<float> conv; conv = policy.policy->inputs;
					shared.sharedData->policies.Append(conv);
				}
				}
				catch (...) {}

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
				//	share.managementTool->upper_constraints()[dim_n] = cweeMath::Lerp<u64>(shared.managementTool->upper_constraints()[dim_n], dimension_avg[dim_n], 0.1);
				//}
			};
			std::function func4 = [=](cweeOptimizerData& shared) -> bool { /* End of iteration */
				try {
					AUTO ptr = shared.managementTool->BestParticle();
					return StopOptimization(*shared.sharedData, ptr.inputs, EvaluatePolicy(ptr.inputs));
				}catch(...){}
				return true;
			};
			return cweeScheduler<cweeOptimizerData, policy_data_type>::run_async(make_cwee_shared<cweeOptimizerData>(manager.template CastReference<OptimizationManagementTools<>>(), std::move(sharedData)), func1, func2, func3, func4);
		};

		/* Actual implimentation of the job queue, utilizing the multi-job scheduler. */
		template<typename shared_data, typename providedManagerType>
		cweeJob run_optimization_impl(cweeSharedPtr<shared_data> sharedData, cweeSharedPtr<providedManagerType> manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(shared_data&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
			//static_assert(num_dimensions_d >= 1, "Optimization must use at least one dimension");
			//static_assert(num_particles_p >= 1, "Optimization must use at least one particle");

			cweeJob job_actual = run_optimization_impl_detail<shared_data, providedManagerType>(sharedData, manager, EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
			return job_actual;
		};

		/* Alternating_OptimizationManagementTool */
		template<template<typename> class sharedPtrType, template<class, class> class OptType, typename element, class type1, class type2>
		cweeJob run_optimization_detail(sharedPtrType<element> shared, OptType<type1, type2> const& manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(element&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
			return run_optimization_impl<element, OptType<type1, type2>>(std::move(shared), make_cwee_shared<OptType<type1, type2>>(manager, manager.num_dimensions(), manager.num_particles()), EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
		};

		/* Random_OptimizationManagementTool && Genetic_OptimizationManagementTool */
		template<template<typename> class sharedPtrType, template<bool> class OptType, typename element, bool minimize>
		cweeJob run_optimization_detail(sharedPtrType<element> shared, OptType<minimize> const& manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(element&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
			return run_optimization_impl<element, OptType<minimize>>(std::move(shared), make_cwee_shared<OptType<minimize>>(manager, manager.num_dimensions(), manager.num_particles()), EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
		};

		/*  */
		template<template<typename> class sharedPtrType, class OptType, typename element>
		cweeJob run_optimization_detail(sharedPtrType<element> shared, OptType const& manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(element&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
			return run_optimization_impl<element, OptType>(std::move(shared), make_cwee_shared<OptType>(manager, manager.num_dimensions(), manager.num_particles()), EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
		};
	};

	/* Queue and perform optimization in async based on provided parameters and functions. */
	template<template<typename> class sharedPtrType, class OptType, typename element> 
	cweeJob run_optimization(sharedPtrType<element> shared, OptType const& manager, std::function<u64(cweeThreadedList<u64>&)> const& EvaluatePolicy, std::function<bool(element&, cweeThreadedList<u64>&, u64)> const& StopOptimization, int maxIterations = 100, cweeThreadedList<float> const& defaultPolicy = cweeThreadedList<float>()) {
		return run_optimization_detail(std::move(shared), manager, EvaluatePolicy, StopOptimization, maxIterations, defaultPolicy);
	};

#define OPT_FUNC_DIM(NUMDIM, NUMPOLICIES, _min_) case (NUMDIM) : { \
		Genetic_OptimizationManagementTool<_min_> ramt(NUMDIM, NUMPOLICIES); \
		ramt.lower_constraints() = lowerBound; \
		ramt.upper_constraints() = upperBound; \
		toAwait = cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations); \
		break; \
	}
};

#pragma endregion