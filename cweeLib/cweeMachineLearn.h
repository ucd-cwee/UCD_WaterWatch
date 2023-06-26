#ifndef __MACHINE_LEARN_H__
#define __MACHINE_LEARN_H__
#include "precompiled.h"

// create an enumerate set of definitions that contain the various different kernel types
enum kernel_bit{
	RADIAL_KERNEL = BIT(0),
	GAUSSIAN_KERNEL = BIT(1)
};
// enumerate the definitions of the various learners that we have within the library
enum learner_bit {
	R_LEAST_SQAURES = BIT(0),
	NEURAL_NET = BIT(1),
	ARIMA = BIT(2),
	SVR = BIT(3)
};

enum data_type_bit {
	BINARY = BIT(0),
	INTEGER = BIT(1),
	CONTINUOUS = BIT(2)
};


class cweeML_data_input {

public:

	data_type_bit							data_type = CONTINUOUS;
	bool									has_labels = false;
	cweeThreadedList< cweeThreadedList<float> >		features_vector;
	cweeThreadedList<cweeStr>				feature_names;
	cweeThreadedList<float>					labels_vector;

	int										num_features = 0;
	int										num_obs = 0;
	float									gamma = 0.1f;
	float									c = 1.0f;

	void									addFeature(const cweeThreadedList<float>& input){
		if (num_features > 0) {
			num_obs = input.NumRef();
			//for (int i = 0; i < num_obs && i < features_vector.Num(); i++) {
			//	if (features_vector[i].GetGranularity() <= input.NumRef())
			//		features_vector[i].SetGranularity(16 + input.NumRef());
			//}
			for (int i = 0; i < num_obs && i < features_vector.Num(); i++) {
				features_vector[i].Append(input[i]); // each input has to be added to the end of each feature vector
			}
			num_features++;			
		}else{
			cweeThreadedList< cweeThreadedList<float> >	transposed_features_vector;
			transposed_features_vector.Append(input);
			num_features++;
			num_obs = input.NumRef();

			{
				features_vector.Clear();
				int numCol = 0;
				for (auto& row : transposed_features_vector) numCol = cweeMath::max(numCol, row.Num());
				cweeThreadedList< cweeThreadedList<float> > output; output.SetGranularity(numCol+16);
				cweeThreadedList<float> input_vector;
				int index_t; int index_r; int n = transposed_features_vector.Num();
				for (index_r = 0; index_r < numCol; index_r++) {// invert the for loop
					if (input_vector.Num() == n) {
						for (index_t = 0; index_t < n && index_t < input_vector.Num() && index_r < transposed_features_vector[index_t].Num(); index_t++) { // go through and rearrange the values in the vector
							input_vector[index_t] = transposed_features_vector[index_t][index_r];
						}
						features_vector.Append(input_vector);
					}
					else {
						input_vector.Clear(); input_vector.SetGranularity(n + 16);
						for (index_t = 0; index_t < n && index_r < transposed_features_vector[index_t].Num(); index_t++) { // go through and rearrange the values in the vector
							input_vector.Append(transposed_features_vector[index_t][index_r]);
						}
						features_vector.Append(input_vector);
					}
				}		
			}
		}
	} // the access functions
	int										getNumFeatures() { return num_features; }
	int										getNumObs() { return num_obs; }
	bool									getHasLabels() { return has_labels; }
	cweeThreadedList< cweeThreadedList<float> >&		getFeaturesVec() { return features_vector;}
	const cweeThreadedList< cweeThreadedList<float> >&		getFeaturesVec() const { return features_vector; }
	cweeThreadedList<float>&						getLabelsVec() { return labels_vector; }
	const cweeThreadedList<float>& getLabelsVec() const { return labels_vector; }
};

//// 
//
//
//
////
class cweeML_model_output {

public:

	data_type_bit data_type;
	std::vector <float> labels;
	std::vector <int> cat_labels;






	std::vector <float> getLabels() {	return labels;}
	std::vector <int> getCaLabels() { return cat_labels; }
};

//
//
//
//
//
class cweeML_paramaters_RecursiveLeastSquares {

public:
	float alpha;
	float b_intercept;
	std::vector<float> weights_vector; // the weights for each of the learned variables

	cweeStr Serialize() {
		cweeStr delim = ":cweeML_paramaters_RecursiveLeastSquares_DELIM:";
		cweeStr out;

		out.AddToDelimiter((float)alpha, delim);
		out.AddToDelimiter((float)b_intercept, delim);
		{
			cweeThreadedList<cweeStr> temp;
			for (auto& x : weights_vector) { temp.Append(cweeStr((float)x)); }
			
			cweeStr listOut;
			for (auto& x : temp) listOut.AddToDelimiter(x, ",");

			out.AddToDelimiter(listOut, delim);
		}

		return out;
	};
	void Deserialize(cweeStr& in) {
		cweeParser obj (in, ":cweeML_paramaters_RecursiveLeastSquares_DELIM:", true);
		
		alpha = (float)obj.getVar(0);
		b_intercept = (float)obj.getVar(1);
		if (!obj.getVar(2).IsEmpty()) {
			cweeParser knots(obj.getVar(2), ",", true);
			for (int i = 0; i < knots.getNumVars(); i++) {
				weights_vector.push_back((float)knots.getVar(i));
			}
		}
	};

};

//
//
//
//
//
class cweeML_parameters_Neural_Net {

public:
	int test = 0;
	int test2 = 0;

	cweeStr Serialize() {
		cweeStr delim = ":cweeML_parameters_Neural_Net_DELIM:";
		cweeStr out;

		out.AddToDelimiter((int)test, delim);
		out.AddToDelimiter((int)test2, delim);
		return out;
	};
	void Deserialize(cweeStr& in) {
		cweeParser obj (in, ":cweeML_parameters_Neural_Net_DELIM:", true);

		test = (int)obj.getVar(0);
		test = (int)obj.getVar(1);
	};

};

//
//
//
//
//
class cweeML_parameters_ARIMA {

public:
	int test = 0;
	int test2 = 0;

	cweeStr Serialize() {
		cweeStr delim = ":cweeML_parameters_Neural_Net_DELIM:";
		cweeStr out;

		out.AddToDelimiter((int)test, delim);
		out.AddToDelimiter((int)test2, delim);
		return out;
	};
	void Deserialize(cweeStr& in) {
		cweeParser obj (in, ":cweeML_parameters_Neural_Net_DELIM:", true);

		test = (int)obj.getVar(0);
		test = (int)obj.getVar(1);
	};
};

//
//
//
//
//



class cweeML_parameters_SVR_regression {

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
			else{
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

#if 0
		// maintain compatability with older system
		if (in.Find(":cweeML_parameters_SVR_regression_DELIM:") < 0) {
			// dlib system
			if (in.IsEmpty()) {
				df2 = dlib::decision_function<kernel_type>();
			}
			else {
				std::string t = in.c_str();
				std::istringstream inStr(t);
				dlib::deserialize(df2, inStr);
			}
		}
		else {
			// cwee system
			if (1) {
				cweeParser obj(in, ":cweeML_parameters_SVR_regression_DELIM:", true);
				in.Clear();

				std::vector<float> alpha_vec;
				std::vector < std::vector <float> > basis_vector;
				float b_scalar;
				cweeParser knots, vals;
				int i, j;
				std::vector <float> row;
				{
					if (!obj.getVar(0).IsEmpty()) {
						knots.Parse(obj.getVar(0), ",", true);
						alpha_vec.reserve(cweeMath::max(1, knots.getNumVars()));
						for (i = 0; i < knots.getNumVars(); i++) {
							alpha_vec.push_back((float)knots.getVar(i));
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

				{
					df2 = dlib::decision_function<kernel_type>();
					df2.basis_vectors.set_size(basis_vector.size());
					for (int index_i = 0; index_i < basis_vector.size(); index_i++) df2.basis_vectors(index_i) = dlib::mat(basis_vector[index_i]);
					df2.alpha = dlib::mat(alpha_vec);
					df2.b = b_scalar;
				}
			}
		}

#else

		cweeParser obj (in, ":cweeML_parameters_SVR_regression_DELIM:", true);
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


#endif

	};
};

//
//
//
//
//

#define SVR_Only

class cweeML_learned_parameters {

public:
	std::string method_name;
#ifdef SVR_Only
	cweeML_parameters_SVR_regression svr_param;
#else
	cweeML_paramaters_RecursiveLeastSquares rls_param;
	cweeML_parameters_SVR_regression svr_param;
	cweeML_parameters_ARIMA ARIMA_param;
	cweeML_parameters_Neural_Net NN_param;
#endif
	//cweeThreadedList<bool> includeFeature;
	bool learned = false;
	vec4 performance = vec4(0, cweeMath::INF, 0, cweeMath::INF);
	vec4 learnPeriod = vec4(0, 0, 0, 0);
	Curve learnedAdjustment;

	cweeStr Serialize() {
		cweeStr delim = ":cweeML_learned_parameters_in_DELIM:";
		cweeStr out;

		if (learned) {
			if (cweeStr(method_name.c_str()).IsEmpty())
				out += " ";
			else
				out.AddToDelimiter(method_name.c_str(), delim);
#ifdef SVR_Only
			out.AddToDelimiter(svr_param.Serialize(), delim);
#else
			out.AddToDelimiter(rls_param.Serialize(), delim);
			out.AddToDelimiter(svr_param.Serialize(), delim);
			out.AddToDelimiter(ARIMA_param.Serialize(), delim);
			out.AddToDelimiter(NN_param.Serialize(), delim);
#endif
			out.AddToDelimiter(learned, delim);
			out.AddToDelimiter(performance.Serialize(), delim);
			//cweeStr t; for (auto& x : includeFeature) t.AddToDelimiter(x, ",");
			//out.AddToDelimiter(t, delim);
			out.AddToDelimiter(learnPeriod.Serialize(), delim);
			out.AddToDelimiter(learnedAdjustment.Serialize(), delim);
			return out;
		}
		else {
			// not learned, therefore record nothing. 
			return "";
		}

	};
	void Deserialize(cweeStr& in) {		
		if (in.IsEmpty()) {
			*this = cweeML_learned_parameters(); // make new
		}
		else {
			cweeParser obj(in, ":cweeML_learned_parameters_in_DELIM:", true);
			in.Clear();

			method_name = obj[0].c_str();
#ifdef SVR_Only
			svr_param.Deserialize(obj[1]); obj[1].Clear();
			learned = (bool)(int)obj[2]; obj[2].Clear();

#else
			rls_param.Deserialize(obj.getVar(1)); obj[1].Clear();
			svr_param.Deserialize(obj.getVar(2)); obj[2].Clear();
			ARIMA_param.Deserialize(obj.getVar(3)); obj[3].Clear();
			NN_param.Deserialize(obj.getVar(4)); obj[4].Clear();
			learned = (bool)(int)obj.getVar(5); obj[5].Clear();
			//includeFeature.Clear();
			//cweeParser t(obj[6], ",", true);
			//includeFeature.SetGranularity(t.getNumVars() + 16);
			//for (auto& x : t) includeFeature.Append((bool)(int)x);
#endif
			performance.Deserialize(obj[3]);
			if (obj.getNumVars() >= 5) learnPeriod.Deserialize(obj[4]);
			if (obj.getNumVars() >= 6) learnedAdjustment.Deserialize(obj[5]);
		}
	};
};




//
//
// cweeML_Math contains wrappers that outline the machine learning functions 
//
//
class cweeML_Math {

public:
#ifndef SVR_Only
	// the RLS methods 
	static cweeML_learned_parameters		LearnRecursiveLeastSquares(cweeML_data_input input);
	static cweeML_model_output				OutputRecursiveLeastSquares(cweeML_learned_parameters input_1, cweeML_data_input input_2);
	static cweeML_model_output				OutputRecursiveLeastSquaresNoDot(cweeML_learned_parameters input_1, cweeML_data_input input_2);
	
	// 
	static cweeML_learned_parameters		LearnNeuralNet(cweeML_data_input input);
	static cweeML_model_output				OutputNeuralNet(cweeML_learned_parameters input, cweeML_data_input input2);

	// 
	static cweeML_learned_parameters		LearnARIMASquares(cweeML_data_input input);
	static cweeML_model_output				OutputARIMASquares(cweeML_learned_parameters input, cweeML_data_input input2);
#endif

	static cweeML_learned_parameters		LearnSVRmodel(const cweeML_data_input& input);
	static void								LearnSVRmodel_debug(const cweeML_data_input& input, cweeML_learned_parameters& output_parameters);
	static cweeML_model_output				OutputSVRmodel(const cweeML_learned_parameters& input_1, const cweeML_data_input& input_2);

	// 
	static float sinc_x(std::vector<float> x_vec, std::vector<float> parameter_vector);
	static cweeML_data_input generateTestData(int dimensions, int row_in, std::vector<float> generator);
	static cweeML_data_input generateTestDataNoGen(int dimensions, int row);
	static cweeML_data_input generateTestDataCategorical(int dimensions, int row_in, std::vector<float> generator, std::vector<float> threshold);

private:
	
	static dlib::matrix<float> ConvertTwoVectorToMatrix(std::vector< std::vector <float > > vector_in); // no private variables for now, this class will house only static methods 
	static dlib::matrix<float> ConvertOneVectorToMatrix(std::vector <float> vector_in);
	static std::vector< std::vector <float > > ConvertMatrixToTwoVector(dlib::matrix<float> mat_in);
	static std::vector<float> ConvertMatrixToOneVector(dlib::matrix<float> mat_in);	// The converstion functions

};

INLINE dlib::matrix<float> cweeML_Math::ConvertTwoVectorToMatrix(std::vector< std::vector <float > > vector_in) {
	dlib::matrix<float>  matrix_out;
	return matrix_out;
}

INLINE dlib::matrix<float> cweeML_Math::ConvertOneVectorToMatrix(std::vector <float> vector_in) {
	dlib::matrix<float> matrix_out;
	return matrix_out;
}
INLINE std::vector< std::vector <float > > cweeML_Math::ConvertMatrixToTwoVector(dlib::matrix<float> mat_in) {

	std::vector< std::vector <float > > vector_out;
	return vector_out;
}
INLINE std::vector<float> cweeML_Math::ConvertMatrixToOneVector(dlib::matrix<float> mat_in) {

	std::vector<float> vector_out;
	return vector_out;
}

INLINE float cweeML_Math::sinc_x(std::vector<float> x_vec, std::vector<float> parameter_vector) {

	float solution = 0;
	float alpha_offset;
	float x_compute_value;
	std::vector<float> parameter_vector_update;

	if (parameter_vector.size() != x_vec.size()) {
		parameter_vector_update.insert(parameter_vector_update.end(), x_vec.size(), 1);
	}
	else {
		parameter_vector_update = parameter_vector;
	}

	for (int index = 1; index < x_vec.size(); index++) {// create a for loop 
		x_compute_value = x_vec.at(index); // 
		alpha_offset = parameter_vector_update.at(index);
		solution = solution + (sin(x_compute_value) / (alpha_offset * x_compute_value));
	}

	return solution;
}

// generateTestData 
INLINE cweeML_data_input cweeML_Math::generateTestData(int dimensions, int row_in, std::vector<float> generator) {

	cweeML_data_input output;
	cweeThreadedList<cweeThreadedList<float>> row_inputs;
	std::vector<float> label_out;
	float label;

	// generate a random vector of parameters equal to the dimension
	for (int index_r = 1; index_r < row_in; index_r++) {
		// generate a new random feature vector 
		// calculate the SinC function with the random generator
		std::vector<float> x_vec = cweeMath::GenerateRandomVector(dimensions, 3, 0);

		label = sinc_x(x_vec, generator);
		row_inputs.Append(x_vec);
		label_out.push_back(label); // push back the value of the label;
	}
	
	// set the output data
	output.features_vector = row_inputs;
	output.labels_vector = label_out;
	output.has_labels = TRUE; 
	output.num_obs = row_in;

	return output; 
}

INLINE cweeML_data_input cweeML_Math::generateTestDataCategorical(int dimensions, int row_in, std::vector<float> generator, std::vector<float> threshold) {

	cweeML_data_input output;
	std::vector < std::vector <float> > row_inputs;
	std::vector<float> label_out;
	int label;
	
	for (int index_r = 1; index_r < row_in; index_r++) { // generate a random vector of parameters equal to the dimension
		std::vector<float> x_vec = cweeMath::GenerateRandomVector(dimensions, 3, 0);
		label = cweeMath::categorizeThreshold(sinc_x(x_vec, generator), threshold);
		row_inputs.push_back(x_vec);
		label_out.push_back(label); // push back the value of the label;
	}

	return output;
}

INLINE cweeML_data_input cweeML_Math::generateTestDataNoGen(int dimensions, int row) {
	cweeML_data_input output;

	std::vector<float> generator_int = cweeMath::GenerateRandomVector(dimensions, 0, 3); // generate a random vector
	output = cweeML_Math::generateTestData(dimensions, row, generator_int);  // generate a random vector here 

	return output; // return the output of the generation
}


#ifndef SVR_Only

// This paramaters_RecursiveLeastSquares will learn the machine learning
INLINE cweeML_learned_parameters cweeML_Math::LearnRecursiveLeastSquares(cweeML_data_input input_1) {

	cweeML_learned_parameters output_parameters;
	//cweeML_paramaters_RecursiveLeastSquares rls_param_out;

	//typedef dlib::matrix<float, 0, 1> linear_kernel_type;
	//typedef dlib::linear_kernel<linear_kernel_type> linear_basis_kernel;
	//typedef dlib::matrix<float, 0, 1> sample_type;

	//dlib::rls learner;  // create a test 3 function, 
	////dlib::decision_function<linear_basis_kernel> rls_decision;

	//std::vector < std::vector <float> > features_vec = input_1.getFeaturesVec();
	//std::vector <float> labels_vec = input_1.getLabelsVec();

	//sample_type m;   
	//
	//std::vector<float> test_vector;
	//test_vector = features_vec.at(0);
	//m.set_size(test_vector.size()); // now we train our object on a few samples of the sinc function.

	//for (int index_f = 0; index_f < features_vec.size(); index_f++) {
	//	test_vector = features_vec.at(index_f);
	//	for (int index_element = 0; index_element < test_vector.size(); index_element++) {
	//		m(index_element) = test_vector.at(index_element);
	//	}

	//	learner.train(m, labels_vec.at(index_f));
	//}

	//auto rls_decision = learner.get_decision_function(); // copy the decision function to the storage object for our machine learning 
	//rls_param_out.alpha = rls_decision.alpha;
	//rls_param_out.b_intercept = rls_decision.b; 

	//auto weight_matrix = learner.get_w();
	//
	//for (int index_mat = 0; index_mat < weight_matrix.size(); index_mat++) {
	//	rls_param_out.weights_vector.push_back (weight_matrix(index_mat));
	//}

	//output_parameters.rls_param = rls_param_out;
	return output_parameters;
}

INLINE cweeML_model_output	cweeML_Math::OutputRecursiveLeastSquares(cweeML_learned_parameters input_1, cweeML_data_input input_2) {
	std::vector < std::vector <float>> data_vector = input_2.getFeaturesVec();
	std::vector <float> output_labels;
	cweeML_model_output model_output;

	float found_label;
	float b_intercept = input_1.rls_param.b_intercept;
	std::vector<float> A_weight_vec = input_1.rls_param.weights_vector;

	for (int index_v = 0; index_v < data_vector.size(); index_v++) {
		found_label = cweeMath::VectorInnerProductD((cweeThreadedList<float>)A_weight_vec, (cweeThreadedList<float>)data_vector.at(index_v)) + b_intercept;
		output_labels.push_back(found_label);
	}

	model_output.labels = output_labels;// set up the labels
	
	return model_output;
}

INLINE cweeML_model_output	cweeML_Math::OutputRecursiveLeastSquaresNoDot(cweeML_learned_parameters input_1, cweeML_data_input input_2) {

	typedef dlib::matrix<float, 0, 1> linear_kernel_type;
	typedef dlib::linear_kernel<linear_kernel_type> linear_basis_kernel;
	typedef dlib::matrix<float, 0, 1> sample_type;
	float found_label =0;
	dlib::matrix<float,0,1> found_label_mat;

	// set up the vector
	std::vector < std::vector <float>> data_vector = input_2.getFeaturesVec();
	std::vector <float> output_labels;
	cweeML_model_output model_output;
	linear_basis_kernel used_kernel;
	sample_type alpha_vec;
	alpha_vec = dlib::mat(input_1.rls_param.alpha);

	// set up the decision function
	dlib::decision_function<linear_basis_kernel> rls_decision;
	rls_decision.alpha = alpha_vec;
	rls_decision.kernel_function = used_kernel;
	dlib::matrix<float,0,1> test_mat = dlib::mat(input_1.rls_param.weights_vector);
	rls_decision.b = input_1.rls_param.b_intercept;
	rls_decision.basis_vectors.set_size(1);
	rls_decision.basis_vectors(0).set_size(input_1.rls_param.weights_vector.size());
	rls_decision.basis_vectors(0) = test_mat;

	for (int index_v = 0; index_v < data_vector.size(); index_v++) {
		found_label = rls_decision(dlib::mat(data_vector.at(index_v))) ;
		output_labels.push_back(found_label);
	}

	model_output.labels = output_labels; // set up the labels
	return model_output;
}

INLINE cweeML_learned_parameters	cweeML_Math::LearnNeuralNet(cweeML_data_input input_1) {

	cweeML_learned_parameters output_parameters;
//	cweeML_paramaters_RecursiveLeastSquares rls_param_out;
//	typedef dlib::matrix<float, 0, 1> linear_kernel_type;
//	typedef dlib::linear_kernel<linear_kernel_type> linear_basis_kernel;
//	typedef dlib::matrix<float, 0, 1> sample_type;
//
//	dlib::rls learner;  // create a test 3 function, 
//	dlib::decision_function<linear_basis_kernel> rls_decision;
//
//	std::vector < std::vector <float> > features_vec = input_1.getFeaturesVec();
//	std::vector <float> labels_vec = input_1.getLabelsVec();
//	std::vector <float> test_vector = features_vec[0];
//
//	dlib::mlp::kernel_1a_c net(2, 5);
//
//	sample_type m;
///*
//	m.set_size(test_vector.size()); // now we train our object on a few samples of the sinc function.
//
//	// go through and 
//	for (int index_i = 0; index_i < labels_vec.size(); index_i++) {
//		test_vector = features_vec.at(index_i);
//		for (int index_j = 0; index_j < m.size(); index_j++) {
//			m(index_j) = test_vector.at(index_j);
//		}
//		net.train(m, labels_vec.at(index_i));
//
//	}
//	*/

	return output_parameters;
}

INLINE cweeML_model_output						cweeML_Math::OutputNeuralNet(cweeML_learned_parameters input_1, cweeML_data_input input_2) {
	//cout << "Hello World" << endl;
}
INLINE cweeML_learned_parameters		cweeML_Math::LearnARIMASquares(cweeML_data_input input_1) {
	//cout << "Hello World" << endl;
}
INLINE cweeML_model_output						cweeML_Math::OutputARIMASquares(cweeML_learned_parameters input_1, cweeML_data_input input_2) {
	//cout << "Hello World" << endl;
}

#endif

// 
// 
INLINE cweeML_model_output cweeML_Math::OutputSVRmodel(const cweeML_learned_parameters& learned_Parameters, const cweeML_data_input& featureObservations) {
	// instantiate

	cweeThreadedList<float> alpha_vec;
	cweeThreadedList<cweeThreadedList<float>> basis_vector;
	float b_scalar;
	{
		for (auto& x : learned_Parameters.svr_param.df2.alpha) {
			alpha_vec.Append((float)x); 
		}
	}
	{
		cweeStr matri; cweeThreadedList<float> temp;
		for (auto& x : learned_Parameters.svr_param.df2.basis_vectors) {
			temp.Clear();

			for (auto& y : x) { 
				temp.Append(y);
			}

			basis_vector.Append(temp);
		}

		b_scalar = learned_Parameters.svr_param.df2.b;
	}

	cweeThreadedList<cweeThreadedList<float>>		featuresMatrix			(featureObservations.features_vector);
	cweeThreadedList<cweeThreadedList<float>>		learnedMatrixParam		(basis_vector);
	float									b_vec = b_scalar;
	float									found_label;

	typedef dlib::matrix<float, 0, 1>								linear_kernel_type;
	typedef dlib::linear_kernel<linear_kernel_type>					linear_basis_kernel;
	typedef dlib::matrix<float, 0, 1>								sample_type;
	typedef dlib::radial_basis_kernel<sample_type>					kernel_type;
	dlib::svr_trainer<kernel_type>									trainer;
	dlib::decision_function<kernel_type>							df2;

	cweeML_model_output												output_object;

	// compile the decision_function
	df2.basis_vectors.set_size(learnedMatrixParam.NumRef());
	std::vector<float> t;
	for (int index_i = 0; index_i < learnedMatrixParam.NumRef(); index_i++) {
		t = learnedMatrixParam[index_i];
		df2.basis_vectors(index_i) = dlib::mat(t);
	}
	t = alpha_vec;
	df2.alpha = dlib::mat(t);
	df2.b = b_vec;

	// perform the forecast
	for (int index_v = 0; index_v < featuresMatrix.NumRef(); index_v++) {  // go through and find the labels for each of the points
		t = featuresMatrix[index_v];
		found_label = df2(dlib::mat(t));
		output_object.labels.push_back(found_label);
	}

	return output_object;
}

#endif 