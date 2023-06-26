#pragma hdrstop
#include "precompiled.h"
#include "cweeMachineLearn.h"
extern void	Sys_CPUCount(int& logicalNum, int& coreNum, int& packageNum);

cweeML_learned_parameters	cweeML_Math::LearnSVRmodel(const cweeML_data_input& input) {
	cweeML_learned_parameters output_parameters;

	typedef dlib::matrix<float, 0, 1> sample_type;
	typedef dlib::radial_basis_kernel<sample_type> kernel_type;

	std::vector<sample_type> samples; 
	{
		auto& features_vec = input.getFeaturesVec();
		sample_type m;

		samples.reserve(features_vec.NumRef()); // create vector of different values
		const std::vector<float>& targets = input.getLabelsVec();
		m.set_size(features_vec[0].NumRef()); // now we train our object on a few samples of the sinc function.
		int fvIFN; 
		for (int index_f = 0; index_f < features_vec.NumRef(); index_f++) {
			fvIFN = features_vec[index_f].NumRef();

//#pragma loop(hint_parallel(8))
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
			int numPhysicalCpuCores, numLogicalCpuCores, numCpuPackages;
			Sys_CPUCount(numLogicalCpuCores, numPhysicalCpuCores, numCpuPackages);
			int RAM_mb; float maxRam;
			{
				MEMORYSTATUSEX statex;
				statex.dwLength = sizeof(statex);
				GlobalMemoryStatusEx(&statex);
				int physRam = statex.ullTotalPhys / (1024 * 1024);
				physRam = (physRam + 8) & ~15; // HACK: For some reason, ullTotalPhys is sometimes off by a meg or two, so we round up to the nearest 16 megs
				RAM_mb = physRam;

				// modify by current statisticis, not just hardware max
				{
					sysMemoryStats_t in = Sys_GetMemoryStats();
					RAM_mb *= (100 - cweeMath::Abs(in.percentMemoryUsed)) / 100.0f;
					// i.e. each successive, simultaneous ML operation may use less and less RAM.

					maxRam = cweeMath::Fmax(200.0f, cweeMath::Fmin(0.8f * (RAM_mb / numLogicalCpuCores), 1000.0f));
#if 0
					if (cweeRandomInt(0, 100) < 1)
						fileSystem->submitToast("Finished Machine Learning", cweeStr::printf("Completed training. PhysRAM: %i, Remaining RAM: %i, Utilized RAM: %i", (int)physRam, (int)RAM_mb, (int)maxRam));
#endif
				}
			}

			// each thread may utilize UP TO:

			trainer.set_cache_size(maxRam);														//	"1000"	//	memory use	// maximum RAM usage
		}
		{
			float minLabel(std::numeric_limits<float>::max()), maxLabel(-std::numeric_limits<float>::max());
			for (auto& x : input.labels_vector) {
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

	output_parameters.svr_param.df2 = trainer.train(samples, (std::vector<float>)input.getLabelsVec());

	return output_parameters; // output parameters
};

void	cweeML_Math::LearnSVRmodel_debug(const cweeML_data_input& input, cweeML_learned_parameters& output_parameters) {
	typedef dlib::matrix<float, 0, 1> sample_type;
	typedef dlib::radial_basis_kernel<sample_type> kernel_type;

	std::vector<sample_type> samples; 
	{
		auto& features_vec = input.getFeaturesVec();
		sample_type m;

		samples.reserve(features_vec.NumRef()); // create vector of different values
		const std::vector<float>& targets = input.getLabelsVec();
		if (features_vec.NumRef() > 0) {
			int index_element, index_f, vecL;
			m.set_size(features_vec[0].NumRef());
			vecL = features_vec.NumRef();
			int fvIFN;
			for (index_f = 0; index_f < vecL; index_f++) {
				fvIFN = features_vec[index_f].NumRef() - 1;

//#pragma loop(hint_parallel(8))
				for (index_element = fvIFN; index_element >= 0; --index_element) 
					m(index_element) = features_vec[index_f][index_element];
				
				samples.push_back(m);
			}
		}
	}

	dlib::svr_trainer<kernel_type> trainer;		
	{
		trainer.set_kernel(input.gamma);													//	"0.1"	//	kernal		// 1 / n features (i.e. 1/4 for auto self-regression)
		trainer.set_c(input.c);																//	"1"		//	c value		// higher = higher accuracy, lower = reduced accuracy. 
		{
			int numPhysicalCpuCores, numLogicalCpuCores, numCpuPackages;
			Sys_CPUCount(numLogicalCpuCores, numPhysicalCpuCores, numCpuPackages);
			int RAM_mb; float maxRam;
			{
				MEMORYSTATUSEX statex;
				statex.dwLength = sizeof(statex);
				GlobalMemoryStatusEx(&statex);
				int physRam = statex.ullTotalPhys / (1024 * 1024);
				physRam = (physRam + 8) & ~15; // HACK: For some reason, ullTotalPhys is sometimes off by a meg or two, so we round up to the nearest 16 megs
				RAM_mb = physRam;


				// modify by current statisticis, not just hardware max
				{
					sysMemoryStats_t in = Sys_GetMemoryStats();
					RAM_mb *= (100 - cweeMath::Abs(in.percentMemoryUsed)) / 100.0f;
					// i.e. each successive, simultaneous ML operation may use less and less RAM.

					maxRam = cweeMath::Fmax(200.0f, cweeMath::Fmin(0.8f * (RAM_mb / numLogicalCpuCores), 1000.0f)); // i.e. 200 MB? 
#if 0
					if (cweeRandomInt(0, 100) < 1)
						fileSystem->submitToast("Finished Machine Learning", cweeStr::printf("Completed training. PhysRAM: %i, Remaining RAM: %i, Utilized RAM: %i", (int)physRam, (int)RAM_mb, (int)maxRam));
#endif
				}
			}

			// each thread may utilize UP TO:
			trainer.set_cache_size(maxRam);														//	"1000"	//	memory use	// maximum RAM usage
		}
		{
			float minLabel(std::numeric_limits<float>::max()), maxLabel(-std::numeric_limits<float>::max());
			for (auto& x : input.labels_vector) {
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

	output_parameters.svr_param.df2 = trainer.train_debug(samples, (std::vector<float>)input.getLabelsVec());
};
