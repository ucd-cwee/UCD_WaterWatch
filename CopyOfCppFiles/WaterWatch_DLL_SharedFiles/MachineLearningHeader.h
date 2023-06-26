#pragma once
#pragma hdrstop
#include "Precompiled.h"

#include "../dlib-19.17/source/dlib/bayes_utils.h"
#include "../dlib-19.17/source/dlib/graph_utils.h"
#include "../dlib-19.17/source/dlib/graph.h"
#include "../dlib-19.17/source/dlib/directed_graph.h"
#include "../dlib-19.17/source/dlib/svm.h"
#include "../dlib-19.17/source/dlib/mlp.h"
#include "../dlib-19.17/source/dlib/optimization.h"

enum class kernel_bit {
	RADIAL_KERNEL,
	GAUSSIAN_KERNEL
}; // create an enumerate set of definitions that contain the various different kernel types
enum class data_type_bit {
	BINARY,
	INTEGER,
	CONTINUOUS
};

class MachineLearningInput {
public:
	/* values to use to predict using (temp, precipitation, etc.) */
	cweeList<cweeList<float>> features; 
	/* values to predict in the future; (i.e. water usage) */
	cweeList<float> labels; 
	/* Gamma for the fit function */
	float gamma = 0.1f;
	/* Constant for the fit function */
	float c = 1.0f;
	/* Number of Features */
	int NumFeatures() const { return features.Num(); };
	/* Minimum number of Observations */
	int NumObs() const {
		float n = labels.Num() == 0 ? cweeMath::INF : labels.Num();
		for (auto& feat : features) {
			n = cweeMath::Fmin(feat.Num(), n);
		}
		return n >= cweeMath::INF || n < 0 ? (int)0 : (int)n;
	};
	void addFeature(const cweeThreadedList<float>& input) { features.Append(input); };
	cweeList<cweeList<float>> GetInvertedFeatures() const {
		cweeList<cweeList<float>> out(NumObs());
		for (int i = 0; i < NumObs(); i++) {
			cweeList<float> row(NumFeatures());
			for (auto& feat : features) {
				row.Append(feat[i]);
			}
			out.Append(row);
		}
		return out;
	};
	static cweeList<cweeList<float>> GetInvertedFeatures(cweeList<cweeList<float>> const& source) {
		cweeList<cweeList<float>> out;
		if (source.Num() > 0) {
			int n = source[0].Num();
			for (int i = 0; i < n; i++) {
				cweeList<float> row(source.Num());
				for (auto& feat : source) {
					row.Append(feat[i]);
				}
				out.Append(row);
			}
		}
		return out;
	};
	cweeList<float> GetLabels() const {
		cweeList <float> out = labels;
		if (NumObs() > 0) {
			out.Resize(NumObs());
		}
		return out;
	};
};
class MachineLearning_SVR_Results {
public:
	typedef dlib::matrix<float, 0, 1>								sample_type;
	typedef dlib::radial_basis_kernel<sample_type>					kernel_type;
	dlib::decision_function<kernel_type>							df2;
	cweeStr Serialize() {
		cweeStr delim = ":cweeML_parameters_SVR_regression_DELIM:";
		cweeStr out;


		{
			cweeStr listOut;
			for (auto& x : df2.alpha) {
				if (x == 0.0f) {
					if (listOut.IsEmpty()) {
						listOut = " ";
					}
					else {
						listOut.AddToDelimiter("", ",");
					}
				}
				else {
					listOut.AddToDelimiter(x, ",");
				}
			}
			if (listOut.IsEmpty()) {
				out.AddToDelimiter(" ", delim);
			}
			else {
				out.AddToDelimiter(listOut, delim);
			}
		}
		{
			// often nr = 2, nc = 1, size = 2;
			int nr = df2.basis_vectors.nr();
			int nc = df2.basis_vectors.nc();
			int nrTimesNc = nr * nc;
			int size = df2.basis_vectors.size();
			int sizeDif = size - nrTimesNc;

			cweeStr matri; cweeStr listOut;
			int sizeOfVector = 0;
			for (auto& x : df2.basis_vectors) {
				listOut.Clear();
				sizeOfVector = x.size();

				//if (sizeOfVector == nr) {
				//	int ijk = 0;
				//	// column major. i.e. 
				//	// for (auto& column : df2.basis)
				//	//		for (auto& row : column)...
				//	submitToast("Machine Learning Debug", cweeStr::printf("Column Major: nr%i nc%i size%i", nr, nc, size));
				//}
				//else if (sizeOfVector == nc) {
				//	int ijk = 0;
				//	// row major. i.e. 
				//	// for (auto& row : df2.basis)
				//	//		for (auto& column : row)...
				//	submitToast("Machine Learning Debug", cweeStr::printf("Row Major: nr%i nc%i size%i", nr, nc, size));
				//}
				//else {
				//	int ijk = 0;
				//	// no idea what is happening
				//	submitToast("Machine Learning Debug", cweeStr::printf("Unknown Major: nr%i nc%i size%i", nr, nc, size));
				//}

				// often n_subR = 192, nc = 1, size = 192

				//int n_subR = x.nr();
				//int n_subC = x.nc();
				//int subSize = x.size();
				//submitToast("Machine Learning Debug2", cweeStr::printf("Unknown Major: nr%i nc%i size%i", n_subR, n_subC, subSize));

				for (auto& y : x) {
					if (y == 0.0f) {
						if (listOut.IsEmpty()) {
							listOut = " ";
						}
						else {
							listOut.AddToDelimiter("", ",");
						}
					}
					else {
						listOut.AddToDelimiter(y, ",");
					}
				}

				if (1) {
					// reduce string size
					listOut.Replace(",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,", "_");
					listOut.Replace(",,,,,", "?");
				}

				matri.AddToDelimiter(listOut, ":SVR_basis_vector:");
			}
			if (out.IsEmpty()) out = delim + matri;
			else out.AddToDelimiter(matri, delim);
		}
		out.AddToDelimiter(df2.b, delim);
		out.AddToDelimiter(df2.kernel_function.gamma, delim);

		return out;
	};
	void Deserialize(cweeStr& in) {



		cweeParser obj(in, ":cweeML_parameters_SVR_regression_DELIM:", true);
		in.Clear();

		std::vector<float> alpha_vec;
		std::vector < std::vector <float> > basis_vector;
		float b_scalar, gamma;
		cweeParser knots, vals;
		int i, j;
		std::vector <float> row;
		{
			if (!obj.getVar(0).IsEmpty()) {
				knots.Parse(obj.getVar(0), ",", true);
				if (knots.getNumVars() >= 2) {
					alpha_vec.reserve(cweeMath::max(1, knots.getNumVars()));
					for (i = 0; i < knots.getNumVars(); i++) {
						alpha_vec.push_back((float)knots.getVar(i));
					}
				}
			}
		}
		{
			if (!obj.getVar(1).IsEmpty()) {
				cweeParser rows(obj.getVar(1), ":SVR_basis_vector:", true);
				basis_vector.reserve(cweeMath::max(1, rows.getNumVars()));
				for (i = 0; i < rows.getNumVars(); i++) {
					row.clear(); row.reserve(cweeMath::max(1, vals.getNumVars()));

					if (1) {
						// increase string size
						rows.getVar(i).Replace("?", ",,,,,");
						rows.getVar(i).Replace("_", ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,");
					}

					vals.Parse(rows.getVar(i), ",", true);
					for (j = 0; j < vals.getNumVars(); j++) {
						if (vals[j].IsEmpty())
							row.push_back(0.0f);
						else
							row.push_back((float)vals[j]);
					}
					basis_vector.push_back(row);
				}
			}
		}
		b_scalar = (float)obj.getVar(2);
		gamma = obj.getVar(3).IsEmpty() ? 0.1f : (float)obj.getVar(3);
		{
			df2 = dlib::decision_function<kernel_type>(); //  alpha_vec, b_scalar, kernel_type(), basis_vector);
			df2.basis_vectors.set_size(basis_vector.size(), 1);
			df2.alpha.set_size(alpha_vec.size(), 1);

			for (int index_i = 0; index_i < basis_vector.size(); index_i++) {
				auto& content = basis_vector[index_i];
				df2.basis_vectors(index_i).set_size(content.size(), 1);
				for (int index_j = 0; index_j < content.size(); index_j++) {
					df2.basis_vectors(index_i)(index_j) = content[index_j];
				}
			}

			for (int index_i = 0; index_i < alpha_vec.size(); index_i++) {
				df2.alpha(index_i) = alpha_vec[index_i];
			}

			df2.b = b_scalar;
			df2.kernel_function = kernel_type(gamma);
		}
	};
};
class MachineLearning_Results {
public:
	MachineLearning_SVR_Results svr_results;
	bool learned = false;
	vec4 performance = vec4(0, cweeMath::INF, 0, cweeMath::INF);
	vec4 learnPeriod = vec4(0, 0, 0, 0);
	Curve learnedAdjustment;

	cweeStr Serialize() {
		cweeStr delim = ":cweeML_learned_parameters_in_DELIM:";
		cweeStr out;

		if (learned) {
			out += " ";
			out.AddToDelimiter(svr_results.Serialize(), delim);
			out.AddToDelimiter(learned, delim);
			out.AddToDelimiter(performance.Serialize(), delim);
			out.AddToDelimiter(learnPeriod.Serialize(), delim);
			out.AddToDelimiter(learnedAdjustment.Serialize(), delim);
			return out;
		}
		else {
			return "";
		}

	};
	void Deserialize(cweeStr& in) {
		if (in.IsEmpty()) {
			*this = MachineLearning_Results(); // make new
		}
		else {
			cweeParser obj(in, ":cweeML_learned_parameters_in_DELIM:", true);
			in.Clear();

			svr_results.Deserialize(obj[1]); obj[1].Clear();
			learned = (bool)(int)obj[2]; obj[2].Clear();
			performance.Deserialize(obj[3]);
			if (obj.getNumVars() >= 5) learnPeriod.Deserialize(obj[4]);
			if (obj.getNumVars() >= 6) learnedAdjustment.Deserialize(obj[5]);
		}
	};
};
class MachineLearning_Math {
public:
	static MachineLearning_Results		Learn(const MachineLearningInput& input) {
		MachineLearning_Results output_parameters;

		typedef dlib::matrix<float, 0, 1> sample_type;
		typedef dlib::radial_basis_kernel<sample_type> kernel_type;
		AUTO labels = input.GetLabels();

		std::vector<sample_type> samples;
		{
			auto features_vec = input.GetInvertedFeatures();
			sample_type m;
			samples.reserve(features_vec.Num()); // create vector of different values
			m.set_size(features_vec[0].Num()); // now we train our object on a few samples of the sinc function.
			int fvIFN;
			for (int index_f = 0; index_f < features_vec.Num(); index_f++) {
				fvIFN = features_vec[index_f].Num();

				for (int index_element = 0; index_element < fvIFN; index_element++)
					m(index_element) = features_vec[index_f][index_element];

				samples.push_back(m);
			}
		}

		dlib::svr_trainer<kernel_type> trainer;
		{
			trainer.set_kernel(input.gamma);													//	"0.1"	//	kernal		// 1 / n features (i.e. 1/4 for auto self-regression)
			trainer.set_c(input.c);																//	"1"		//	c value		// higher = higher accuracy, lower = reduced accuracy. 
			{
				CPU_DATA this_cpu;
				Computer_Usage this_ram;

				int RAM_mb = this_ram.Ram_MB(); float maxRam;
				{			
					RAM_mb *= (100 - cweeMath::Abs(this_ram.PercentMemoryUsed())) / 100.0f;
					maxRam = cweeMath::Fmax(200.0f, cweeMath::Fmin(0.8f * (RAM_mb / this_cpu.m_numLogicalCpuCores), 1000.0f));					
				}

				// each thread may utilize UP TO:
				trainer.set_cache_size(maxRam);														//	"1000"	//	memory use	// maximum RAM usage
			}
			{
				float minLabel(std::numeric_limits<float>::max()), maxLabel(-std::numeric_limits<float>::max());
				for (auto& x : labels) {
					if (minLabel > x) minLabel = x;
					if (maxLabel < x) maxLabel = x;
				}
				float diff(1);
				if (maxLabel > minLabel) {
					diff = maxLabel - minLabel;  // i.e. 23, 0.01, 0
					// our desired accuracy is within 0.1% of this difference. If the range of this timeseries is 1, it results in an eps. of 1 / 1000 = 0.001;
				}
				trainer.set_epsilon(diff / 1000.0f); // i.e. 0.1% of the range of the dataset	
				// trainer.set_epsilon(0.001); //	"0.01"	//	epsilon		// maximum error. smaller - longer learning
			}
		}

		output_parameters.svr_results.df2 = trainer.train(samples, (std::vector<float>)labels);

		return output_parameters; // output parameters
	};
	static MachineLearning_Results		LearnFast(const MachineLearningInput& input) {
		MachineLearning_Results output_parameters;

		typedef dlib::matrix<float, 0, 1> sample_type;
		typedef dlib::radial_basis_kernel<sample_type> kernel_type;
		AUTO labels = input.GetLabels();

		std::vector<sample_type> samples;
		{
			auto features_vec = input.GetInvertedFeatures();
			sample_type m;
			samples.reserve(features_vec.Num()); // create vector of different values
			m.set_size(features_vec[0].Num()); // now we train our object on a few samples of the sinc function.
			int fvIFN;
			for (int index_f = 0; index_f < features_vec.Num(); index_f++) {
				fvIFN = features_vec[index_f].Num();

				for (int index_element = 0; index_element < fvIFN; index_element++)
					m(index_element) = features_vec[index_f][index_element];

				samples.push_back(m);
			}
		}

		dlib::svr_trainer<kernel_type> trainer;
		{
			trainer.set_kernel(input.gamma);													//	"0.1"	//	kernal		// 1 / n features (i.e. 1/4 for auto self-regression)
			trainer.set_c(input.c);																//	"1"		//	c value		// higher = higher accuracy, lower = reduced accuracy. 
			{
				CPU_DATA this_cpu;
				Computer_Usage this_ram;

				float maxRam;
				{
					int RAM_mb = this_ram.Ram_MB();
					RAM_mb *= (100 - cweeMath::Abs(this_ram.PercentMemoryUsed())) / 100.0f;
					maxRam = cweeMath::Fmax(200.0f, cweeMath::Fmin(0.8f * (RAM_mb / this_cpu.m_numLogicalCpuCores), 1000.0f));
				}
				maxRam *= 0.25f; // further restrain the ram for fast fitting

				// each thread may utilize UP TO:
				trainer.set_cache_size(maxRam);														//	"1000"	//	memory use	// maximum RAM usage
			}
			{
				float minLabel(std::numeric_limits<float>::max()), maxLabel(-std::numeric_limits<float>::max());
				for (auto& x : labels) {
					if (minLabel > x) minLabel = x;
					if (maxLabel < x) maxLabel = x;
				}
				float diff(1);
				if (maxLabel > minLabel) {
					diff = maxLabel - minLabel;  // i.e. 23, 0.01, 0
					// our desired accuracy is within 1% of this difference. If the range of this timeseries is 1, it results in an eps. of 1 / 100 = 0.01;
				}
				trainer.set_epsilon(diff / 20.0f); // i.e. 5% of the range of the dataset
			}
		}

		output_parameters.svr_results.df2 = trainer.train(samples, (std::vector<float>)labels);
		return output_parameters; // output parameters
	};
	static void CompileTimeTest() {
		MachineLearningInput ml_input;
		ml_input.labels.Append(0);
		ml_input.labels.Append(1);
		ml_input.labels.Append(2);
		ml_input.labels.Append(3);

		ml_input.addFeature(ml_input.labels); // predict using itself
		ml_input.addFeature(ml_input.labels); // predict using itself

		AUTO ml_result = MachineLearning_Math::Learn(ml_input);
		std::cout << ml_result.performance.ToString() << std::endl;
	};

};

