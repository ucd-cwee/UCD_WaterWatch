#pragma once
#include "Precompiled.h"

// #define useLearnedAdjustment

/*!
Generalized interface for machine learning.
Use Learn(...) to generate the machine learned parameters. Use Forecast(...) to utilize the parameters.
*/
class cweeMachineLearning {
private:
	static float Average(cweeThreadedList<float> const& values) {
		float avg = 0; 
		int n = 0; 
		for (auto& x : values) cweeMath::rollingAverageRef(avg, x, n);
		return avg;
	};
public:
	class SummarizeFeature {
	public:
		SummarizeFeature() {};
		SummarizeFeature(cweeThreadedList<float> const& in) { Eval(in); };
		float Normalized(float in) const { return (in - min) / range; };
		cweeThreadedList<float> Normalized(cweeThreadedList<float> in)const {
			for (auto& x : in) {
				x = Normalized(x);
			}
			return in;			
		};
		float Unnormalized(float in) const {
			return (in * range) + min;
		};
		cweeThreadedList<float> Unnormalized(cweeThreadedList<float> in) const {
			for (auto& x : in) {
				x = Unnormalized(x);
			}
			return in;
		};
		float avg;
		float min;
		float max;
		float range;
		int num;
	private:		
		void Eval(cweeThreadedList<float> const& d) {
			avg = Average(d);
			min = cweeMath::INF; for (auto& x : d) min = x < min ? x : min;
			max = -cweeMath::INF; for (auto& x : d) max = x > max ? x : max;
			range = max - min;
			num = d.Num();
		};
	};
	class cweeLearnedParams {
	public:
		cweeThreadedList< SummarizeFeature > featureSummaries;
		SummarizeFeature labelSummary;
		cweeThreadedList<float> fit;
	};

	class OptimizationObj : public cweeOptimizer::sharedClass {
	public:
		int numIterationsFailedImprovement = 0;
	};

	static cweeLearnedParams Learn_Opt(cweeSharedPtr<cweeThreadedList<float>> trueLabels, cweeSharedPtr<cweeThreadedList<cweeThreadedList<float>>> trueFeatures) {
		cweeSharedPtr<cweeLearnedParams> out = new cweeLearnedParams();
		{
			// the constant is the average value
			out->labelSummary = SummarizeFeature(*trueLabels);
			for (auto& x : *trueFeatures) out->featureSummaries.Append(SummarizeFeature(x));
			
			// solve for the multiplier per feature.

			// predict the normalized labels using the normalized features
			{
				cweeThreadedList<float> lowerBound, upperBound;
				for (auto& bv : *trueFeatures) { lowerBound.push_back(-10000); }
				for (auto& bv : *trueFeatures) { upperBound.push_back(10000); }

				// MINIMIZE THE ERROR
				AUTO todo = [=](cweeThreadedList<float> const& x) -> float {
					cweeThreadedList<float> result;
					float prediction = out->labelSummary.min;
					int x_n;
					for (int obs_i = 0; obs_i < trueLabels->Num(); obs_i++) {
						prediction = out->labelSummary.min;
						x_n = 0;
						for (int feature_n = 0; feature_n < trueFeatures->Num(); feature_n++) {
							prediction += (
								x[x_n++] * (
									trueFeatures->at(feature_n)[obs_i] - (out->featureSummaries[feature_n].min)
								)
							);
						}
						result.push_back(prediction);
					}
					float err = 0;
					for (int i = 0; i < result.Num(); i++) { err += cweeMath::Pow(trueLabels->at(i) - result[i], 2.0); }
					err = cweeMath::Pow(err, 0.5);
					return err;
				};
				int numDimensions = cweeMath::min(lowerBound.Num(), upperBound.Num());

				constexpr int maxIterations = 1000;
				constexpr float eps = 0.000001f;
				static std::function objFunc = [=](cweeThreadedList<u64>& policy) -> u64 {
					return todo(policy);
				};
				static std::function isFinishedFunc = [=](OptimizationObj& shared, cweeThreadedList<u64>& bestPolicy, u64 bestPerformance) -> bool {
					auto& i = shared.results.Alloc();
					i.bestPolicy = bestPolicy;
					i.bestPerformance = bestPerformance;
					if (shared.results.Num() <= 10) return false; // guarrantee at least 10 iterations
					else {
						// ensure the most recent 5 iterations are better than the 5 before them.
						int i = shared.results.Num() - 1;
						float perf_new = 0;
						float perf_old = 0;
						int count = 0;
						for (; i >= (shared.results.Num() - 5); i--) {
							cweeMath::rollingAverageRef(perf_new, shared.results[i].bestPerformance, count);
						}
						count = 0;
						for (; i >= 0 && i >= (shared.results.Num() - 10); i--) {
							cweeMath::rollingAverageRef(perf_old, shared.results[i].bestPerformance, count);
						}

						// ensure that we are improving. 
						if (cweeMath::Fabs(perf_new - perf_old) < cweeMath::Fabs(bestPerformance * eps)) {
							shared.numIterationsFailedImprovement++;
							if (shared.numIterationsFailedImprovement > (maxIterations / 10)) { 
								return true;
							}
							else {
								return false;
							}
						}
						shared.numIterationsFailedImprovement = 0;
						return false; // we have not flat-lined. continue.
					}
				};
				cweeSharedPtr<OptimizationObj> shared = make_cwee_shared<OptimizationObj>();

				cweeJob toAwait;
				switch (numDimensions) {
					OPT_FUNC_DIM(1, 10, true)
					OPT_FUNC_DIM(2, 10, true)
					OPT_FUNC_DIM(3, 10, true)
					OPT_FUNC_DIM(4, 10, true)
					OPT_FUNC_DIM(5, 10, true)
					OPT_FUNC_DIM(6, 10, true)
					OPT_FUNC_DIM(7, 10, true)
					OPT_FUNC_DIM(8, 10, true)
					OPT_FUNC_DIM(9, 10, true)
					OPT_FUNC_DIM(10, 10, true)
					OPT_FUNC_DIM(11, 10, true)
					OPT_FUNC_DIM(12, 10, true)
					OPT_FUNC_DIM(13, 10, true)
					OPT_FUNC_DIM(14, 10, true)
					OPT_FUNC_DIM(15, 10, true)
					OPT_FUNC_DIM(16, 10, true)
					OPT_FUNC_DIM(17, 10, true)
					OPT_FUNC_DIM(18, 10, true)
					OPT_FUNC_DIM(19, 10, true)
					OPT_FUNC_DIM(20, 10, true)
				}
				toAwait.AwaitAll();

				auto& bestPolicy = shared->results[shared->results.Num() - 1].bestPolicy;
				out->fit = bestPolicy;
			}
		}
		return *out;
	};
	static float Forecast_Opt(const cweeLearnedParams& out, const cweeThreadedList<float>& features) {
		float prediction = 0;
		int x_n;

		prediction = out.labelSummary.min;
		x_n = 0;
		for (int feature_n = 0; feature_n < features.Num(); feature_n++) {
			prediction += (
				out.fit[x_n++] * (
					features.at(feature_n) - (out.featureSummaries[feature_n].min)
				)
			);
		}
		return prediction;
	};
	static cweeThreadedList<float> Forecast_Opt(const cweeLearnedParams& out, const cweeThreadedList<cweeThreadedList<float>>& features) {
		cweeThreadedList<float> result;
		float prediction = out.labelSummary.min;
		int x_n;
		if (features.Num() > 0 && features[0].Num() > 0) {
			for (int obs_i = 0; obs_i < features[0].Num(); obs_i++) {
				prediction = out.labelSummary.min;
				x_n = 0;
				for (int feature_n = 0; feature_n < features.Num(); feature_n++) {
					prediction += (
						out.fit[x_n++] * (
							features.at(feature_n)[obs_i] - (out.featureSummaries[feature_n].min)
						)
					);
				}
				result.push_back(prediction);
			}
		}
		return result;
	};

public:
	class dlibLearnedParams {
	public:
		cweeThreadedList< SummarizeFeature > featureSummaries;
		SummarizeFeature labelSummary;
		MachineLearning_SVR_Results fit;
	};
	static dlibLearnedParams Learn_Dlib(cweeThreadedList<float> const& trueLabels, cweeThreadedList<cweeThreadedList<float>> const& trueFeatures) {
		dlibLearnedParams out; 
		/* Summarize the inputs */
		{
			out.labelSummary = SummarizeFeature(trueLabels);
			for (auto& x : trueFeatures) out.featureSummaries.Append(SummarizeFeature(x));
		}
		/* Meta-optimization of the machine-learning */
		{
			MachineLearningInput input;
			input.labels = trueLabels;
			for (auto& feat : trueFeatures) { input.addFeature(feat); }
			if (input.NumObs() > 0 && input.NumFeatures() > 0) {
				/* Discover optimum selection for gamma and C */
				float gamma = 1.0f, C = 1.0f; {
					cweeThreadedList<float> lowerBound({0.1f, 0.1f}), upperBound({ 1250.0f, 1250.0f });

					// MINIMIZE THE ERROR
					AUTO todo = [=](cweeThreadedList<float> const& x) -> float {
						MachineLearningInput temp_input = input;
						temp_input.gamma = x[0];
						temp_input.c = x[1];
						AUTO result = MachineLearning_Math::LearnFast(temp_input);
						return result.performance.x;
					};
					int numDimensions = cweeMath::min(lowerBound.Num(), upperBound.Num());

					constexpr int maxIterations = 1000;
					constexpr float eps = 0.000001f;
					static std::function objFunc = [=](cweeThreadedList<u64>& policy) -> u64 {
						return todo(policy);
					};
					static std::function isFinishedFunc = [=](OptimizationObj& shared, cweeThreadedList<u64>& bestPolicy, u64 bestPerformance) -> bool {
						auto& i = shared.results.Alloc();
						i.bestPolicy = bestPolicy;
						i.bestPerformance = bestPerformance;
						if (shared.results.Num() <= 10) return false; // guarrantee at least 10 iterations
						else {
							// ensure the most recent 5 iterations are better than the 5 before them.
							int i = shared.results.Num() - 1;
							float perf_new = 0;
							float perf_old = 0;
							int count = 0;
							for (; i >= (shared.results.Num() - 5); i--) {
								cweeMath::rollingAverageRef(perf_new, shared.results[i].bestPerformance, count);
							}
							count = 0;
							for (; i >= 0 && i >= (shared.results.Num() - 10); i--) {
								cweeMath::rollingAverageRef(perf_old, shared.results[i].bestPerformance, count);
							}

							// ensure that we are improving. 
							if (cweeMath::Fabs(perf_new - perf_old) < cweeMath::Fabs(bestPerformance * eps)) {
								shared.numIterationsFailedImprovement++;
								if (shared.numIterationsFailedImprovement > (maxIterations / 10)) { // 10% processed and we saw no further improvement. Unlikely to see improvement with another 10%.
									return true;
								}
								else {
									return false;
								}
							}
							shared.numIterationsFailedImprovement = 0;
							return false; // we have not flat-lined. continue.
						}
					};
					cweeSharedPtr<OptimizationObj> shared = make_cwee_shared<OptimizationObj>();

					Genetic_OptimizationManagementTool<2, 10, true> ramt; // 2 dimensions, 10 policies per iteration, minimize
					ramt.lower_constraints() = lowerBound; ramt.upper_constraints() = upperBound;
					cweeOptimizer::run_optimization(shared, ramt, objFunc, isFinishedFunc, maxIterations, cweeThreadedList<float>({gamma, C})).AwaitAll();

					auto& bestPolicy = shared->results[shared->results.Num() - 1].bestPolicy;
					gamma = bestPolicy[0];
					C = bestPolicy[1];
				}
				input.gamma = gamma;
				input.c = C;
				AUTO result = MachineLearning_Math::Learn(input); // using the 'best' version, do the best learn we can do. 
				out.fit = result.svr_results;
			}
		}
		return out;
	};
	static float Forecast_Dlib(const dlibLearnedParams& out, const cweeThreadedList<float>& features) {
		auto mat = dlib::mat(std::vector<float>(features));
		return out.fit.df2(mat);
	};
	static cweeThreadedList<float> Forecast_Dlib(const dlibLearnedParams& out, const cweeThreadedList<cweeThreadedList<float>>& features) {
		cweeThreadedList<float> results;
		if (features.Num() > 0) {
			results.SetGranularity(features[0].Num() + 16);
			for (auto& x : MachineLearningInput::GetInvertedFeatures(features)) results.Append(out.fit.df2(dlib::mat(std::vector<float>(x))));
		}
		return results;
	};

public: // Public Interface for Generalized Machine Learning
	/*!
	Generate the machine learned parameters. Optionally, automatically perform a random split of the dataset and generate fit performance utilizing it.
	std::pair<vec2, vec2>  -->  [ [ train_R^2 , train_MSE ] , [ test_R^2 , test_MSE ] ]
	*/
	template< class type >
	static MachineLearning_Results Learn(const cweeThreadedList<type>& trueLabels, const cweeThreadedList<cweeThreadedList<type>>& features, std::pair<vec2, vec2>* fit = nullptr, const float percentSplit = 10) {
		cweeThreadedList<cweeThreadedList<float>> Features;
		MachineLearning_Results out;
		Features.SetGranularity(cweeMath::max(100, features.Num()));

		if (fit == nullptr) {
			//Features = features;
			for (auto& x : features) {
				cweeThreadedList<float> saver;
				saver.SetGranularity(cweeMath::max(100, x.Num()));
				for (auto& y : x) {
					saver.Append((float)y);
				}
				Features.Append(saver);
			}
			cweeThreadedList<float> TrueLabels;
			TrueLabels = trueLabels;

			out = LearnSVR(TrueLabels, Features); // to-do, switch statement.
			auto ML_result = Forecast(out, Features);
			vec2 t1 = CalculateError(TrueLabels, ML_result);
			out.performance.x = t1.x;
			out.performance.y = t1.y;

			for (int i = 0; i < ML_result.Num() && i < TrueLabels.Num(); i++) {
				out.learnedAdjustment.AddValue((float)ML_result[i], (float)TrueLabels[i]);
			}
			// for each 'x' value, aggregate the 'y' values to their average. 
			{
				float vSample = 0; int numSamples = 0; u64 prevT = 0; cweeThreadedList<int> indexesToDelete(out.learnedAdjustment.GetNumValues() + 16);
				for (int i = 0; i < out.learnedAdjustment.GetNumValues(); i++) {
					u64 t = out.learnedAdjustment.TimeForIndex(i);
					if (t == prevT) {
						if ((i - 1) >= 0) indexesToDelete.Append(i - 1); // remove previous value


						cweeMath::rollingAverageRef(vSample, out.learnedAdjustment.ValueForIndex(i), numSamples);
					}
					else {
						if ((i - 1) >= 0) out.learnedAdjustment.SetValue(i - 1, vSample);
						vSample = out.learnedAdjustment.ValueForIndex(i); numSamples = 1;
					}
					prevT = t;
				}
				out.learnedAdjustment.knots.RemoveIndexes(indexesToDelete); 
				out.learnedAdjustment.RemoveUnnecessaryKnots();
				out.learnedAdjustment.SetBoundaryType(boundary_t::BT_CLAMPED);
			}

#ifdef useLearnedAdjustment
			for (auto& x : ML_result) x = out.learnedAdjustment.GetCurrentValue(x);

			vec2 t2 = CalculateError(TrueLabels, ML_result);

			if (t2.y >= t1.y) out.learnedAdjustment.Clear(); // didn't help, remove it. 
#endif

		}
		else {
			cweeThreadedList<cweeThreadedList<float>> featuresLearn = features;
			cweeThreadedList<cweeThreadedList<float>> featuresTest(featuresLearn.Num());
			cweeThreadedList<float> labelsLearn = trueLabels;
			cweeThreadedList<float> labelsTest(labelsLearn.Num());

			SplitRandomly(featuresLearn, labelsLearn, featuresTest, labelsTest, percentSplit); // i.e. 10% split.

			//Features = featuresLearn;
			for (auto& x : featuresLearn) {
				cweeThreadedList<float> saver;
				saver.SetGranularity(cweeMath::max(100, x.Num()));
				for (auto& y : x) {
					saver.Append((float)y);
				}
				Features.Append(saver);
			}
			out = LearnSVR(labelsLearn, Features); // to-do, switch statement.

			auto ML_result = Forecast(out, featuresLearn);
			fit->first = CalculateError(labelsLearn, ML_result);

			for (int i = 0; i < ML_result.Num() && i < labelsLearn.Num(); i++) {
				out.learnedAdjustment.AddValue((float)ML_result[i], (float)labelsLearn[i]);
			}

			ML_result = Forecast(out, featuresTest);
			fit->second = CalculateError(labelsTest, ML_result);

			for (int i = 0; i < ML_result.Num() && i < labelsTest.Num(); i++) {
				out.learnedAdjustment.AddValue((float)ML_result[i], (float)labelsTest[i]);
			}
			// for each 'x' value, aggregate the 'y' values to their average. 
			{
				float vSample = 0; int numSamples = 0; u64 prevT = 0; cweeThreadedList<int> indexesToDelete(out.learnedAdjustment.GetNumValues() + 16);
				for (int i = 0; i < out.learnedAdjustment.Num(); i++) {
					u64 t = out.learnedAdjustment.TimeForIndex(i);
					if (t == prevT) {
						if ((i - 1) >= 0) indexesToDelete.Append(i - 1); // remove previous value
						cweeMath::rollingAverageRef(vSample, out.learnedAdjustment.ValueForIndex(i), numSamples);
					}
					else {
						if ((i - 1) >= 0) out.learnedAdjustment.SetValue(i - 1, vSample);
						vSample = out.learnedAdjustment.ValueForIndex(i); numSamples = 1;
					}
					prevT = t;
				}
				out.learnedAdjustment.knots.RemoveIndexes(indexesToDelete);
				out.learnedAdjustment.RemoveUnnecessaryKnots();
				out.learnedAdjustment.SetBoundaryType(boundary_t::BT_CLAMPED);
			}
#ifdef useLearnedAdjustment
			ML_result = Forecast(out, featuresLearn);
			for (auto& x : ML_result) x = out.learnedAdjustment.GetCurrentValue(x);
			vec2 t2 = CalculateError(labelsLearn, ML_result);
			if (t2.y >= fit->first.y) out.learnedAdjustment.Clear(); // didn't help, remove it. 
#endif

		}
		out.learned = true;
		return out;
	};

	/*!
	Generate the machine learned parameters. Optionally, automatically perform a random split of the dataset and generate fit performance utilizing it.
	std::pair<vec2, vec2>  -->  [ [ train_R^2 , train_MSE ] , [ test_R^2 , test_MSE ] ]
	*/
	template< class type >
	static MachineLearning_Results Learn(const cweeThreadedList<std::pair<u64, type>>& trueLabels, const cweeThreadedList<cweeThreadedList<std::pair<u64, type>>>& features, std::pair<vec2, vec2>* fit = nullptr, const float percentSplit = 10) {
		cweeThreadedList<cweeThreadedList<float>> Features;
		cweeThreadedList<float> Labels;

		cweeThreadedList<cweeThreadedList<float>> TestFeatures;
		cweeThreadedList<float> TestLabels;

		Features.SetGranularity(cweeMath::max(100, features.Num() + 16));
		Labels.SetGranularity(cweeMath::max(100, trueLabels.Num() + 16));
		TestFeatures.SetGranularity(cweeMath::max(100, features.Num() + 16));
		TestLabels.SetGranularity(cweeMath::max(100, trueLabels.Num() + 16));

		MachineLearning_Results out;

		if (fit == nullptr) {
			for (auto& x : features) {
				cweeThreadedList<float> saver;
				saver.SetGranularity(cweeMath::max(100, x.Num()));
				for (auto& y : x) {
					saver.Append((float)y.second);
				}
				Features.Append(saver);
			}

			for (auto& x : trueLabels) {
				Labels.Append((float)x.second);
			}
			out = LearnSVR(Labels, Features); // to-do, switch statement.
			auto ML_result = Forecast(out, Features);
			vec2 t1 = CalculateError(Labels, ML_result);
			out.performance.x = t1.x;
			out.performance.y = t1.y;

			for (int i = 0; i < ML_result.Num() && i < Labels.Num(); i++) {
				out.learnedAdjustment.AddValue((float)ML_result[i], (float)Labels[i]);
			}
			// for each 'x' value, aggregate the 'y' values to their average. 
			{
				float vSample = 0; int numSamples = 0; u64 prevT = 0; cweeThreadedList<int> indexesToDelete(out.learnedAdjustment.GetNumValues() + 16);
				for (int i = 0; i < out.learnedAdjustment.Num(); i++) {
					u64 t = out.learnedAdjustment.TimeForIndex(i);
					if (t == prevT) {
						if ((i - 1) >= 0) indexesToDelete.Append(i - 1); // remove previous value
						cweeMath::rollingAverageRef(vSample, out.learnedAdjustment.ValueForIndex(i), numSamples);
					}
					else {
						if ((i - 1) >= 0) out.learnedAdjustment.SetValue(i-1, vSample);
						vSample = out.learnedAdjustment.ValueForIndex(i); numSamples = 1;
					}
					prevT = t;
				}
				out.learnedAdjustment.knots.RemoveIndexes(indexesToDelete); 
				out.learnedAdjustment.RemoveUnnecessaryKnots();
				out.learnedAdjustment.SetBoundaryType(boundary_t::BT_CLAMPED);
			}
#ifdef useLearnedAdjustment
			for (auto& x : ML_result) x = out.learnedAdjustment.GetCurrentValue(x);
			vec2 t2 = CalculateError(Labels, ML_result);
			if (t2.y >= t1.y) out.learnedAdjustment.Clear(); // didn't help, remove it. 
#endif
		}
		else {
			cweeThreadedList<cweeThreadedList<std::pair<u64, type>>> featuresLearn = features;
			cweeThreadedList<cweeThreadedList<std::pair<u64, type>>> featuresTest(featuresLearn.Num());
			cweeThreadedList<std::pair<u64, type>> labelsLearn = trueLabels;
			cweeThreadedList<std::pair<u64, type>> labelsTest(labelsLearn.Num());

			SplitRandomly(featuresLearn, labelsLearn, featuresTest, labelsTest, percentSplit); // i.e. 10% split.

			for (auto& x : featuresLearn) {
				cweeThreadedList<float> saver;
				saver.SetGranularity(cweeMath::max(100, x.Num()));
				for (auto& y : x) {
					saver.Append((float)y.second);
				}
				Features.Append(saver);
			}

			for (auto& x : labelsLearn) {
				Labels.Append((float)x.second);
			}

			for (auto& x : featuresTest) {
				cweeThreadedList<float> saver(cweeMath::max(100, x.Num()));
				for (auto& y : x) {
					saver.Append((float)y.second);
				}
				TestFeatures.Append(saver);
			}

			for (auto& x : labelsTest) {
				TestLabels.Append((float)x.second);
			}

			out = LearnSVR(Labels, Features, TestLabels, TestFeatures); // to-do, switch statement.
			auto ML_result = Forecast(out, featuresLearn);
			fit->first = CalculateError(labelsLearn, ML_result);

			for (int i = 0; i < ML_result.Num() && i < labelsLearn.Num(); i++) {
				out.learnedAdjustment.AddValue((float)ML_result[i].second, (float)labelsLearn[i].second);
			}

			ML_result = Forecast(out, featuresTest);
			fit->second = CalculateError(labelsTest, ML_result);

			for (int i = 0; i < ML_result.Num() && i < labelsTest.Num(); i++) {
				out.learnedAdjustment.AddValue((float)ML_result[i].second, (float)labelsTest[i].second);
			}

			// for each 'x' value, aggregate the 'y' values to their average. 
			{
				float vSample = 0; int numSamples = 0; u64 prevT = 0; cweeThreadedList<int> indexesToDelete(out.learnedAdjustment.GetNumValues() + 16);
				for (int i = 0; i < out.learnedAdjustment.Num(); i++) {
					u64 t = out.learnedAdjustment.TimeForIndex(i);
					if (t == prevT) {
						if ((i - 1) >= 0) indexesToDelete.Append(i - 1); // remove previous value
						cweeMath::rollingAverageRef(vSample, out.learnedAdjustment.ValueForIndex(i), numSamples);
					}
					else {
						if ((i - 1) >= 0) out.learnedAdjustment.SetValue(i-1, vSample);
						vSample = out.learnedAdjustment.ValueForIndex(i); numSamples = 1;
					}
					prevT = t;
				}
				out.learnedAdjustment.knots.RemoveIndexes(indexesToDelete); 
				out.learnedAdjustment.RemoveUnnecessaryKnots();
				out.learnedAdjustment.SetBoundaryType(boundary_t::BT_CLAMPED);
			}
#ifdef useLearnedAdjustment
			ML_result = Forecast(out, featuresLearn);
			for (auto& x : ML_result) x.second = out.learnedAdjustment.GetCurrentValue(x.second);
			vec2 t2 = CalculateError(labelsLearn, ML_result);
			if (t2.y >= fit->first.y) out.learnedAdjustment.Clear(); // didn't help, remove it. 
#endif
		}
		out.learned = true;
		return out;
	};

	/*!
	Generate a machine learned forecast using input features. Note, must match the feature arrangement used during training.
	*/
	static float Forecast(const MachineLearning_Results& controller, const cweeThreadedList<float>& features, const float roundNearest = 0.001f) {
		std::vector<float> Features = features;
		auto mat = dlib::mat(Features);
		float out = controller.svr_results.df2(mat);
#ifdef useLearnedAdjustment
		if (controller.learned && controller.learnedAdjustment.GetNumValues() > 0) {
			out = controller.learnedAdjustment.GetCurrentValue(out);
		}
#endif
		return cweeMath::roundNearest(out, roundNearest);
	};

	/*!
	Generate a machine learned forecast using input features. Note, must match the feature arrangement used during training.
	*/
	template< class type >
	static cweeThreadedList<float> Forecast(const MachineLearning_Results& controller, const cweeThreadedList<cweeThreadedList<type>>& features, const float roundNearest = 0.001f) {
		cweeThreadedList<float> out;
		if (features.Num() > 0) out.SetGranularity(features[0].Num() + 16);
#if 1
		MachineLearningInput featureObservations;
		{
			for (int i = 0; i < features.Num(); i++) {
				featureObservations.addFeature(features[i]);
			}
		}

		// perform the forecast
		float temp; std::vector<float> t;
		AUTO features_flipped = featureObservations.GetInvertedFeatures();

		for (int index_v = 0; index_v < features_flipped.Num(); index_v++) {
			t = features_flipped[index_v];
			temp = controller.svr_results.df2(dlib::mat(t));
#ifdef useLearnedAdjustment
			if (controller.learned && controller.learnedAdjustment.GetNumValues() > 0) {
				temp = controller.learnedAdjustment.GetCurrentValue(temp);
			}
#endif
			out.Append(
				cweeMath::roundNearest(
					temp,
					roundNearest
				)
			);
		}
#else
		if (features.Num() > 0) {
			int n = features[0].Num();
			std::vector<float> featuresAtTime; featuresAtTime.reserve(features[0].Num() + 16);
			float temp; int i;
			for (int index_v = 0; index_v < n; index_v++) {
				for (i = 0; i < features.Num(); i++) {
					if (i <= featuresAtTime.size()) {
						featuresAtTime.push_back(features[i][index_v]);
					}
					else {
						featuresAtTime[i] = features[i][index_v];
					}
				}

				temp = controller.svr_param.df2(dlib::mat(featuresAtTime));
#ifdef useLearnedAdjustment
				if (controller.learned && controller.learnedAdjustment.GetNumValues() > 0) {
					temp = controller.learnedAdjustment.GetCurrentValue(temp);
				}
#endif
				out.Append(
					cweeMath::roundNearest(
						temp,
						roundNearest
					)
				);
			}
		}
#endif
		return out;
	};

	/*!
	Generate a machine learned forecast using input features. Note, must match the feature arrangement used during training.
	*/
	template< class type >
	static cweeThreadedList<std::pair<u64, float>> Forecast(const MachineLearning_Results& controller, const cweeThreadedList<cweeThreadedList<std::pair<u64, type>>>& Features, const float roundNearest = 0.001f) {
		cweeThreadedList<std::pair<u64, float>> out;
		if (Features.Num() > 0) out.SetGranularity(Features[0].NumRef() + 16);
#if 1
		cweeThreadedList<u64> times;
		cweeThreadedList<cweeThreadedList<float>> features(Features.NumRef() + 16); {
			// fill the time vector			
			for (auto& x : Features) {
				times.SetGranularity(x.NumRef() + 16);
				for (auto& y : x)
					times.Append(y.first);
				break;
			}
			// fill the features vectors
			for (int i = 0; i < Features.Num(); i++) {
				cweeThreadedList<float> saver(Features[i].Num() + 16);
				for (auto& y : Features[i])
					saver.Append((float)y.second);
				features.Append(saver);
			}
		}
		MachineLearningInput featureObservations; {
			for (auto& x : features)
				featureObservations.addFeature(x);
		}

		// perform the forecast
		float temp; std::vector<float> t;
		AUTO features_flipped = featureObservations.GetInvertedFeatures();

		for (int index_v = 0; index_v < features_flipped.NumRef(); index_v++) {  // go through and find the labels for each of the points
			t = features_flipped[index_v];
			temp = controller.svr_results.df2(dlib::mat(t));
#ifdef useLearnedAdjustment
			if (controller.learned && controller.learnedAdjustment.GetNumValues() > 0) {
				temp = controller.learnedAdjustment.GetCurrentValue(temp);
			}
#endif
			out.Append(
				std::make_pair(
					times[index_v],
					cweeMath::roundNearest(
						temp,
						roundNearest
					)
				)
			);
		}

#else

		if (Features.Num() > 0) {
			int n = Features[0].Num();
			std::vector<float> featuresAtTime; featuresAtTime.reserve(Features[0].Num() + 16);
			float temp; int i; std::pair<u64, float> p;
			for (int index_v = 0; index_v < n; index_v++) {
				for (i = 0; i < Features.Num(); i++) {
					if (i <= featuresAtTime.size()) {
						featuresAtTime.push_back(Features[i][index_v].second);
					}
					else {
						featuresAtTime[i] = Features[i][index_v].second;
					}
				}

				temp = controller.svr_param.df2(dlib::mat(featuresAtTime));
#ifdef useLearnedAdjustment
				if (controller.learned && controller.learnedAdjustment.GetNumValues() > 0) {
					temp = controller.learnedAdjustment.GetCurrentValue(temp);
				}
#endif
				p.first = Features[0][index_v].first;
				p.second = cweeMath::roundNearest(
					temp,
					roundNearest
				);
				out.Append(p);
			}
		}

#endif

		return out;
	};

	/*!
	Calculate the error between the observed and modeled data sets.
	Vec2 = [ R^2, MSE ]
	*/
	static vec2 CalculateError(const cweeThreadedList<float>& observed, const cweeThreadedList<float>& modeled) {
		vec2 out;

		float y_Bar(0);
		float sum_Square_Total(0);
		float sum_Square_Residuals(0);
		float R_squared(0);
		float Mean_Squared_Error(0);

		// y_Bar
		{
			int i(0);
			for (auto& x : observed) {

				cweeMath::rollingAverageRef(y_Bar, x, i);
			}
		}

		// sum_Square_Total
		{
			for (auto& x : observed) {
				sum_Square_Total += (x - y_Bar) * (x - y_Bar);
			}
		}

		// sum_Square_Residuals
		{
			int j(0);
			for (int i = 0; i < observed.Num() && i < modeled.Num(); i++) {
				sum_Square_Residuals += (observed[i] - modeled[i]) * (observed[i] - modeled[i]);
				cweeMath::rollingAverageRef(Mean_Squared_Error, (observed[i] - modeled[i]) * (observed[i] - modeled[i]), j);
			}
		}

		// R_squared
		R_squared = 1 - (sum_Square_Total == 0.0f ? 0.0f : sum_Square_Residuals / (sum_Square_Total + cweeMath::EPSILON));

		out = vec2(R_squared, Mean_Squared_Error);

		return out;
	};

	/*!
	Calculate the error between the observed and modeled data sets.
	Vec2 = [ R^2, MSE ]
	*/
	static float CalculateLikelihood(const cweeThreadedList<float>& observed, const cweeThreadedList<float>& modeled) {
		// x=observed, y=predicted
		Curve organizer; for (int i = 0; i < observed.Num() && i < modeled.Num(); i++)  organizer.AddValue(observed[i], modeled[i]);
		// list of unique observed values
		cweeThreadedList<float> uniqueObserved; for (auto& x : observed) uniqueObserved.AddUnique(x);
		// list of list of predicted values for the indexed observed value
		cweeThreadedList<cweeThreadedList<float>> predictedValuesForObservedIndex; {
			for (int i = 0; i < uniqueObserved.Num(); i++)  predictedValuesForObservedIndex.Append(cweeThreadedList<float>()); // set size
			int currentIndex = -1; float currentObserved = -1; // optimized search
			for (auto& knot : organizer.GetKnotSeries()) { // append values
				if (currentObserved != knot.first) {
					currentIndex = uniqueObserved.FindIndex(knot.first);
					currentObserved = knot.first;
				}
				if (currentIndex >= 0 && currentIndex < predictedValuesForObservedIndex.Num()) {
					predictedValuesForObservedIndex[currentIndex].Append(knot.second);
				}
			}
		}





		return 1.0f;
	};

	/*!
	Calculate the error between the observed and modeled data sets.
	Vec2 = [ R^2, MSE ]
	*/
	static vec2 CalculateError(const cweeThreadedList<std::pair<u64, float>>& observed, const cweeThreadedList<std::pair<u64, float>>& modeled) {
		vec2 out;

		cweeThreadedList<float> trueFloats(observed.Num() + 16);
		cweeThreadedList<float> estimateFloats(modeled.Num() + 16);

		for (auto& x : observed) trueFloats.Append(x.second);
		for (auto& x : modeled) estimateFloats.Append(x.second);

		out = CalculateError(trueFloats, estimateFloats);

		return out;
	};

private: // Private Interface for Machine Learning
	/*!
	Use a support vector to calculate the machine learning parameters.
	*/
	static MachineLearning_Results LearnSVR(const cweeThreadedList<float>& trueLabels, const cweeThreadedList<cweeThreadedList<float>>& features, const cweeThreadedList<float>& testLabels = cweeThreadedList<float>(), const cweeThreadedList<cweeThreadedList<float>>& testfeatures = cweeThreadedList<cweeThreadedList<float>>()) {
		MachineLearning_Results out;
		int numFeatures = features.Num();

		MachineLearningInput input;

		input.labels = trueLabels;
		for (int i = 0; i < features.Num(); i++) {
			input.addFeature(features[i]);
		}

		if (input.NumObs() > 0 && input.NumFeatures() > 0) {
			// temp. Stupid hard-coded grid search - single elite from several attempts.  
			constexpr int max_i = 1;
			constexpr int max_j = 1;

			float minGamma = 0.1f;
			float maxGamma = 1250.0f;

			float minC = 0.1f;
			float maxC = 1250.0f;

			float previousPerformance = cweeMath::INF;

			float bestGamma;
			float bestC;
			cweeThreadedList<float> forecasted;
			MachineLearning_Results temp;
			float Gamma; float C; vec2 fit1; vec2 fit2; float perf;
			for (int k = 0; k < 2; k++) { // Gamma and C refinement over time
				for (int i = 0; i < (max_i + 1); i++) { // Gamma
					for (int j = 0; j < (max_j + 1); j++) { // C
						Gamma = minGamma + ((maxGamma - minGamma) / max_i) * (((float)i));
						C = minC + ((maxC - minC) / max_j) * (((float)j));

						input.gamma = Gamma;
						input.c = C;

						temp = MachineLearning_Math::Learn(input);

						forecasted = Forecast(temp, features);
						fit1 = CalculateError(trueLabels, forecasted);

						if (testLabels.Num() > 0) {
							forecasted = Forecast(temp, testfeatures);
							fit2 = CalculateError(testLabels, forecasted);
						}
						else {
							fit2 = vec2(1, 0);
						}

						perf = cweeMath::Fmax(fit1.y, fit2.y);

						if (perf < previousPerformance) {
							previousPerformance = perf;
							out = temp;
							bestGamma = Gamma;
							bestC = C;

							if (perf <= 0) return out; // early exit because... it's good enough. 
						}
					}
				}

				if (bestC >= ((maxC + minC) / 2.0f)) {
					minC += ((maxC + minC) / 2.0f);
					maxC *= 2.0f;
				}
				else {
					maxC -= ((maxC + minC) / 2.0f);
					minC *= 0.5f;
				}

				if (bestGamma >= ((maxGamma + minGamma) / 2.0f)) {
					minGamma += ((maxGamma + minGamma) / 2.0f);
					maxGamma *= 2.0f;
				}
				else {
					maxGamma -= ((maxGamma + minGamma) / 2.0f);
					minGamma *= 0.5f;
				}
			}

			input.gamma = bestGamma;
			input.c = bestC;
			out = MachineLearning_Math::Learn(input); // using the 'best' version, do the best learn we can do. 
		}

		return out;
	};

	/*!
	Split the incoming features/labels randomly into outgoing features/labels.
	*/
	template< class type >
	static void SplitRandomly(cweeThreadedList<cweeThreadedList<type>>& featuresIn, cweeThreadedList<type>& labelsIn, cweeThreadedList<cweeThreadedList<type>>& featuresOut, cweeThreadedList<type>& labelsOut, float percentSplit) {
		featuresOut.Clear();
		labelsOut.Clear();

		int numArrays(featuresIn.Num());
		int numPoints(labelsIn.Num());

		for (auto& x : featuresIn) {
			cweeThreadedList < type > Array;
			featuresOut.Append(Array);
		}

		for (int i = numPoints - 1; i >= 0; i--) {
			if (cweeRandomInt(0, 100) < percentSplit) {
				labelsOut.Insert(labelsIn[i]);
				labelsIn.RemoveIndexFast(i);
				for (int j = 0; j < numArrays; j++) {
					featuresOut[j].Insert(featuresIn[j][i]);
					featuresIn[j].RemoveIndexFast(i);
				}
			}
		}
	};

	/*!
	Split the incoming features/labels randomly into outgoing features/labels.
	*/
	template< class type >
	static void SplitRandomly(cweeThreadedList<cweeThreadedList<std::pair<u64, type>>>& featuresIn, cweeThreadedList<std::pair<u64, type>>& labelsIn, cweeThreadedList<cweeThreadedList<std::pair<u64, type>>>& featuresOut, cweeThreadedList<std::pair<u64, type>>& labelsOut, float percentSplit) {
		featuresOut.Clear();
		labelsOut.Clear();

		int numArrays(featuresIn.Num());
		int numPoints(labelsIn.Num());

		for (int i = 0; i < numArrays; i++) {
			featuresOut.Append(cweeThreadedList<std::pair<u64, type>>());
		}
		for (auto& x : featuresOut) x.SetGranularity((numPoints * (percentSplit + 1) / 100.0f) + 16);
		labelsOut.SetGranularity((numPoints * (percentSplit + 1) / 100.0f) + 16);

		for (int i = numPoints - 1; i >= 0; i--) {
			if (cweeRandomInt(0, 100) < percentSplit) {
				labelsOut.Append(labelsIn[i]);
				labelsIn.RemoveIndexFast(i);
				for (int j = 0; j < numArrays; j++) {
					featuresOut[j].Append(featuresIn[j][i]);
					featuresIn[j].RemoveIndexFast(i);
				}
			}
		}
	};

private:
	/*!
	Example method using the cweeMachineLearning class.
	*/
	void Example(void) {
		std::vector<float> labels_source = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f };
		std::vector<float> feature1_source = { 0.4f, 1.3f, 2.1f, 3.3f, 4.4f, 5.4f, 6.3f, 7.4f, 8.4f, 9.2f, 10.2f, 11.4f, 12.2f, 13.4f, 14.4f };
		std::vector<float> feature2_source = { 0.3f, 1.2f, 2.1f, 3.2f, 4.1f, 5.2f, 6.2f, 7.2f, 8.2f, 9.4f, 10.4f, 11.2f, 12.4f, 13.2f, 14.2f };

		cweeThreadedList<float>							labels(labels_source);
		cweeThreadedList < cweeThreadedList<float> >	features;
		{
			cweeThreadedList<float> data1(feature1_source);
			cweeThreadedList<float> data2(feature2_source);
			features.Append(data1);
			features.Append(data2);
		}

		std::pair<vec2, vec2> fit;

		auto ML_param = cweeMachineLearning::Learn(labels, features, &fit);

		float TRAINING_R_squared = fit.first.x;
		float TRAINING_MSE = fit.first.y;

		float TESTING_R_squared = fit.second.x;
		float TESTING_MSE = fit.second.y;

		std::vector<float> feature1_forecast = { 0.1f, 1.21f, 2.2f, 3.4f, 4.0f, 5.3f, 6.6f, 7.7f, 8.3f, 9.5f, 10.5f, 11.3f, 12.5f, 13.3f, 14.3f };
		std::vector<float> feature2_forecast = { 0.1f, 1.21f, 2.2f, 3.4f, 4.0f, 5.3f, 6.6f, 7.7f, 8.3f, 9.5f, 10.5f, 11.3f, 12.5f, 13.3f, 14.3f };
		features.Clear();
		{
			cweeThreadedList<float> data1(feature1_forecast);
			cweeThreadedList<float> data2(feature2_forecast);
			features.Append(data1);
			features.Append(data2);
		}

		cweeThreadedList<float> forecast = cweeMachineLearning::Forecast(ML_param, features);
	};
};
