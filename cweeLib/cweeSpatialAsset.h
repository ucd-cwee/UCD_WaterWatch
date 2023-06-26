
#ifndef __CWEESPATIALASSET_H__
#define __CWEESPATIALASSET_H__

template<typename A, typename B>
class cweePair {
public:
	cweePair<A, B>() {};
	cweePair<A, B>(const A& in1, const B& in2) {
		first = in1;
		second = in2;
	};

	cweePair<A, B>& operator=(const cweePair<A, B>& other) {
		first = other.first;
		second = other.second;

		return *this;
	};
	friend bool			operator==(const cweePair& a, const cweePair& b) {
		if ((b.first == a.first) && (b.second == a.second)) return true;
		return false;
	};
	friend bool			operator!=(const cweePair& a, const cweePair& b) {
		return !operator==(a);
	};

	template< typename _otherA_, typename _otherB_ >
	operator cweePair<_otherA_, _otherB_>() const {
		cweePair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};
	template< typename _otherA_, typename _otherB_ >
	operator cweePair<_otherA_, _otherB_>() {
		cweePair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};

	template< typename _otherA_, typename _otherB_ >
	operator std::pair<_otherA_, _otherB_>() const {
		std::pair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};
	template< typename _otherA_, typename _otherB_ >
	operator std::pair<_otherA_, _otherB_>() {
		std::pair<_otherA_, _otherB_> out;

		out.first = (_otherA_)first;
		out.second = (_otherB_)second;

		return out;
	};

	mutable A first;
	mutable B second;
};


/*! Generic object intended to host constant and time-varying strings as well as measurements associated to a real-world site.  */
class cweeSpatialAsset {
public:
	enum class SpatialAssetParam
	{
		missingParam = -1,
		constantParam = 0,
		timeseriesParam = 1,
		measurement = 2
	};
	typedef cweePair<SpatialAssetParam, int> assetParameter;

	cweeSpatialAsset& operator=(const cweeSpatialAsset& in) {
		Name = in.Name;
		prev_index = in.prev_index;
		prev_search = in.prev_search;
		parameters_names = in.parameters_names;
		parameters = in.parameters;
		timeseries_measurements = in.timeseries_measurements;
		timeseries_parameters = in.timeseries_parameters;
		constants = in.constants;

		return *this;
	};
	void Clear() {
		prev_index = assetParameter();
		prev_search = "";
		parameters_names.Clear();
		parameters.Clear();
		timeseries_measurements.Clear();
		timeseries_parameters.Clear();
		constants.Clear();
	};
	cweeSpatialAsset() {
		timeseries_measurements.SetGranularity(1); // set small so as not to suddenly require 2GB of memory on use of 1 pattern
		timeseries_parameters.SetGranularity(2); 
		constants.SetGranularity(4); 
	};

	/*! Add a parameter to the object of the specified category with the provided string key */
	void						AddParameter(const SpatialAssetParam& type, const cweeStr& name)	{ 
		_addParameter(type, name); };
	/*! Add a constant string parameter to the object with the specified name, initialized with the provided object */
	void						AddParameter(const cweeStr& name, const cweeStr& param) {
		auto p = _addParameter(SpatialAssetParam::constantParam, name);
		constants[p.second] = cweePair<int, cweeStr>(parameters_names.FindIndex(name), param);
	};
	/*! Add a time-verying string parameter to the object with the specified name, initialized with the provided object */
	void						AddParameter(const cweeStr& name, const cweeCurve<cweeStr>& param) {
		auto p = _addParameter(SpatialAssetParam::timeseriesParam, name);
		timeseries_parameters[p.second] = cweePair<int, cweeCurve<cweeStr>>(parameters_names.FindIndex(name), param);
	};
	/*! Add a time-varying measurement to the object with the specified name, initialized with the provided object */
	void						AddParameter(const cweeStr& name, const Pattern& param) {
		auto p = _addParameter(SpatialAssetParam::measurement, name);
		timeseries_measurements[p.second] = cweePair<int, Pattern>(parameters_names.FindIndex(name), param);
	};
	/*! Get the number of parameters in the object */
	int							Num() const {
		return constants.Num() + timeseries_parameters.Num() + timeseries_measurements.Num();
	};
	/*! Get the nuber of parameters in the object of the requested type  */
	int							Num(const SpatialAssetParam& which) const {
		switch (which) {
		case SpatialAssetParam::constantParam:
			return constants.Num();
			break;
		case SpatialAssetParam::timeseriesParam:
			return timeseries_parameters.Num();
			break;
		case SpatialAssetParam::measurement:
			return timeseries_measurements.Num();
			break;
		}
		return -1;
	};
	/*! Get a list of all parameter names in the object  */
	cweeThreadedList<cweeStr>	GetParameters() const {
		return parameters_names;
	};
	/*! Get a list of all parameter names in the object of the specified type */
	cweeThreadedList<cweeStr>	GetParameters(const SpatialAssetParam& which) const {
		switch (which) {
		case SpatialAssetParam::constantParam:
			return GetConstParameters();
			break;
		case SpatialAssetParam::timeseriesParam:
			return GetTimeSeriesParameters();
			break;
		case SpatialAssetParam::measurement:
			return GetMeasurementParameters();
			break;
		}
		return cweeThreadedList<cweeStr>();
	};
	/*! Get the type of the parameter key name */
	SpatialAssetParam			GetParameter(const cweeStr& name) {
		assetParameter out;
		_getParameter(name, out); return out.first;
	};
	/*! Replace the parameter with the provided name with the input object data */
	void						Append(const cweeStr& name, const cweeStr& in) {				
		GetConstParameter(name) = in;
	};
	/*! Append the parameter with the provided name with the input object data */
	void						Append(const cweeStr& name, const u64& time, const cweeStr& in) {
		GetTimeSeriesParameter(name).AddUniqueValue(time, in);
	};
	/*! Append the parameter with the provided name with the input object data */
	void						Append(const cweeStr& name, const u64& time, float in) {
		GetMeasurement(name).AddUniqueValue(time, in);
	};
	/*! Directly access the parameter with the provided name */
	cweeStr&					GetConstParameter(const cweeStr& name) {
		assetParameter index;
		if (!_getParameter(name, index)) {
			_addParameter(cweeSpatialAsset::SpatialAssetParam::constantParam, name);
			_getParameter(name, index);
		}
		return GetConstParameter(index);
	};
	/*! Directly access the parameter with the provided name */
	cweeCurve<cweeStr>&			GetTimeSeriesParameter(const cweeStr& name) {
		assetParameter index;
		if (!_getParameter(name, index)) {
			_addParameter(cweeSpatialAsset::SpatialAssetParam::timeseriesParam, name);
			_getParameter(name, index);
		}
		return GetTimeSeriesParameter(index);
	};
	/*! Directly access the parameter with the provided name */
	Pattern&					GetMeasurement(const cweeStr& name) {
		assetParameter index;
		if (!_getParameter(name, index)) {
			_addParameter(cweeSpatialAsset::SpatialAssetParam::measurement, name);
			_getParameter(name, index);
		}
		return GetMeasurement(index);
	};
	/*! Save the object to a cweeStr */
	cweeStr						Serialize(int option = -1) {
		cweeStr delim = ":CUSTOM_in_DELIM:";
		cweeStr out; cweeStr temp; cweeStr temp2;

		if (1){
			temp = ""; for (auto& x : constants) {
				temp2 = cweeStr(x.first) + "," + x.second; // 1,STRING
				temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:"); // 1,STRING:CUSTOM_in_DELIM_in:2,STRING:CUSTOM_in_DELIM_in:3,STRING
			}
			temp2 = "";
			out.AddToDelimiter(temp, delim); if (out.IsEmpty()) out = " ";
		}
		if (1) {
			temp = ""; for (auto& x : timeseries_parameters) {
				temp2 = cweeStr(x.first) + "," + x.second.Serialize(); // 1,STRING
				temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:"); // 1,STRING:CUSTOM_in_DELIM_in:2,STRING:CUSTOM_in_DELIM_in:3,STRING
			}
			temp2 = "";
			out.AddToDelimiter(temp, delim);
		}
		if (1) {
			temp = ""; for (auto& x : timeseries_measurements) {
				temp2 = cweeStr(x.first) + "," + x.second.Serialize(option); // 1,STRING
				temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:"); // 1,STRING:CUSTOM_in_DELIM_in:2,STRING:CUSTOM_in_DELIM_in:3,STRING
			}
			temp2 = "";
			out.AddToDelimiter(temp, delim);
		}

		temp = "";
		for (auto& x : parameters) { 
			cweeStr temp2 = cweeStr(static_cast<int>(x.first)) + "," + cweeStr(x.second);
			temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:");
		}
		out.AddToDelimiter(temp, delim);

		temp = "";
		for (auto& x : parameters_names) { 
			temp.AddToDelimiter(x, ":CUSTOM_in_DELIM_in:"); 
		}
		out.AddToDelimiter(temp, delim);

		temp = "";
		out.AddToDelimiter(Name, delim);
		
		return out;
	};
	/*! Load the object from a cweeStr */
	void						Deserialize(cweeStr& in) {
		if (in.IsEmpty()) {
			Clear(); 
			return;
		}

		cweeParser obj(in, ":CUSTOM_in_DELIM:", true); 
		in.Clear(); cweeParser s, r;
		{
			cweeStr& t = obj[0];			
			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = constants;
			cweePair<int, cweeStr> q;
			c.Clear(); c.SetGranularity((int)(s.getNumVars() + 16));
			for (auto& x : s)
			{
				if (x.IsEmpty() || x == " ") continue;
				r.ParseFirstDelimiterOnly(x, ",");
				q = cweePair<int, cweeStr>((int)r[0], r[1]);
				c.Append(q);
			}
		}
		{
			cweeStr& t = obj[1];
			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = timeseries_parameters;
			c.Clear(); c.SetGranularity((int)(s.getNumVars() + 16)); cweePair<int, cweeCurve<cweeStr>> q; cweeCurve<cweeStr> o;
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;

				r.ParseFirstDelimiterOnly(x, ",");
				cweeStr& r2 = r[1];
				cweeStr& r1 = r[0];
				o.Deserialize(r[1]);
				q = cweePair<int, cweeCurve<cweeStr>>((int)r[0], o);
				c.Append(q);
			}
		}
		{
			cweeStr& t = obj[2];
			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = timeseries_measurements;
			c.Clear(); c.SetGranularity((int)(s.getNumVars() + 16)); cweePair<int, Pattern> q; Pattern o;
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;

				r.ParseFirstDelimiterOnly(x, ",");
				o.Deserialize(r[1]);
				q = cweePair<int, Pattern>((int)r[0], o);
				c.Append(q);
			}
		}
		{
			cweeStr& t = obj[3];
			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = parameters;
			c.Clear(); c.SetGranularity((int)(s.getNumVars() + 16)); cweePair<SpatialAssetParam, int> q;
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;

				r.Parse(x, ",", true);
				q = cweePair<SpatialAssetParam, int>(static_cast<SpatialAssetParam>((int)r[0]), (int)r[1]);
				c.Append(q);
			}

		}
		{
			cweeStr& t = obj[4];
			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = parameters_names;
			c.Clear(); c.SetGranularity((int)(s.getNumVars() + 16));
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;
				c.Append(x);
			}

		}
		Name = obj[5];

		prev_search.Clear();
	};

	bool						ParameterExists(const cweeStr& name) {		
		if (GetParameter(name) == SpatialAssetParam::missingParam) return false;
		return true;
	};
	bool						RemoveParameter(const cweeStr& name) {
		if (GetParameter(name) == SpatialAssetParam::missingParam) return false;
#if 0
		{
			prev_index = assetParameter();
			prev_search = "";

			int removeAt = parameters_names.FindIndex(name);
			parameters_names[removeAt] = "";
			assetParameter toDelete = parameters[removeAt];
			parameters[removeAt].first = SpatialAssetParam::missingParam;
			parameters[removeAt].second = -1;

			switch (toDelete.first) {
			case SpatialAssetParam::constantParam: {
				constants[toDelete.second].first = -1;
				constants[toDelete.second].second.Clear();
				break;
			}
			case SpatialAssetParam::timeseriesParam: {
				constants[toDelete.second].first = -1;
				constants[toDelete.second].second.Clear();
				break;
			}
			case SpatialAssetParam::measurement: {
				constants[toDelete.second].first = -1;
				constants[toDelete.second].second.Clear();
				break;
			}
			}
		}
		
#else
		cweeSpatialAsset other;
		other = *this; // copy over
		
		Clear();

		Name = other.Name;
		for (auto& P : other.GetParameters(cweeSpatialAsset::SpatialAssetParam::constantParam)) {
			if (P != name)
				GetConstParameter(P) = other.GetConstParameter(P);
		}
		for (auto& P : other.GetParameters(cweeSpatialAsset::SpatialAssetParam::timeseriesParam)) {
			if (P != name)
				GetTimeSeriesParameter(P) = other.GetTimeSeriesParameter(P);
		}
		for (auto& P : other.GetParameters(cweeSpatialAsset::SpatialAssetParam::measurement)) {
			if (P != name)
				GetMeasurement(P) = other.GetMeasurement(P);			
		}

#endif
		return true;
	};
protected:
	cweeThreadedList<cweeStr>	GetConstParameters() const {
		cweeThreadedList<cweeStr> out(constants.Num() + 1);
		for (auto& x : constants) out.Append(parameters_names[x.first]);
		return out;
	};
	cweeThreadedList<cweeStr>	GetTimeSeriesParameters() const {
		cweeThreadedList<cweeStr> out(constants.Num() + 1);
		for (auto& x : timeseries_parameters) out.Append(parameters_names[x.first]);
		return out;
	};
	cweeThreadedList<cweeStr>	GetMeasurementParameters() const {
		cweeThreadedList<cweeStr> out(constants.Num() + 1);
		for (auto& x : timeseries_measurements) out.Append(parameters_names[x.first]);
		return out;
	};

	cweeStr&					GetConstParameter(const assetParameter& index) { return GetConstParameter(index.second); };
	cweeCurve<cweeStr>&			GetTimeSeriesParameter(const assetParameter& index) { return GetTimeSeriesParameter(index.second); };
	Pattern&					GetMeasurement(const assetParameter& index) { return GetMeasurement(index.second); };

	cweeStr&					GetConstParameter(int index) { return constants[index].second; };
	cweeCurve<cweeStr>&			GetTimeSeriesParameter(int index) { return timeseries_parameters[index].second; };
	Pattern&					GetMeasurement(int index) { return timeseries_measurements[index].second; };

	cweeThreadedList< cweePair<int, cweeStr>>				constants; // site name, site address, etc. 
	cweeThreadedList< cweePair<int, cweeCurve<cweeStr>>>	timeseries_parameters; // customer name, customer account number, etc. 
	cweeThreadedList< cweePair<int, Pattern>>				timeseries_measurements; // water usage, temperature, etc. 

private:
	assetParameter				_newParameter(const SpatialAssetParam& type) {
		assetParameter out;
		out.first = type;
		switch (type) {
		case SpatialAssetParam::constantParam:
			constants.Alloc().first = parameters.Num();
			out.second = constants.Num() - 1;
			break;
		case SpatialAssetParam::timeseriesParam:
			timeseries_parameters.Alloc().first = parameters.Num();
			out.second = timeseries_parameters.Num() - 1;
			break;
		case SpatialAssetParam::measurement:
			timeseries_measurements.Alloc().first = parameters.Num();
			out.second = timeseries_measurements.Num() - 1;
			break;
		}
		return out;
	};
	assetParameter				_addParameter(const SpatialAssetParam& type, const cweeStr& name) {
		int index = parameters_names.FindIndex(name);
		if (index < 0) {
			// new object;
			auto newP = _newParameter(type);
			int i = parameters.Num();
			parameters_names.Append(name.c_str());
			parameters.Append(newP);
			return newP;
		}
		else {
			// existing object;
			return parameters[index];
		}
	};
	bool						_getParameter(const cweeStr& name, assetParameter& out) const  {	
		if (name == prev_search) out = prev_index;
		
		int index = parameters_names.FindIndex(name);
		if (index < 0) {
			out.first = SpatialAssetParam::missingParam;
			return false;
		}
		else {
			out = parameters[index];
			prev_search = name;
			prev_index = out;
			return true;
		}
	}
			
	cweeThreadedList<assetParameter>						parameters;
	cweeThreadedList<cweeStr>								parameters_names; // the position is the index for 'parameters'.

	mutable cweeStr											prev_search; // optimizaiton for searching the same key over and over
	mutable assetParameter									prev_index; // optimizaiton for searching the same key over and over

public:
	cweeStr													Name = "No Name";

	/*!    */
	static void					Instantiate(
		const nanodbc::connection con,
		cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& QueryNames,
		const cweeThreadedList<std::pair<cweeSpatialAsset::SpatialAssetParam, cweeStr>> initParams = cweeThreadedList<std::pair<cweeSpatialAsset::SpatialAssetParam, cweeStr>>()
	) {
		sites.Clear();

		for (auto& site : odbc->FirstColumnOnly(odbc->GetResults(con, QueryNames))) {
			int i = sites.Append();
			{
				sites.Lock();
				auto ptr = sites.UnsafeRead(i);
				if (ptr) {
					ptr->Name = site;
					ptr->AddParameter("Unique ID", site); // const param
				}
				sites.Unlock();
			}
		}

		// instantiate parameters - in case anyone queries these they need to at least "exist" to start.
		for (auto& i : sites.GetList()) {
			sites.Lock();
			cweeSpatialAsset* ptr = sites.UnsafeRead(i);
			if (ptr) {
				for (auto& x : initParams) {
					switch (x.first) {
					case cweeSpatialAsset::SpatialAssetParam::constantParam:
						ptr->AddParameter(x.first, x.second);
						break;
					case cweeSpatialAsset::SpatialAssetParam::timeseriesParam:
						ptr->AddParameter(x.first, x.second);
						break;
					case cweeSpatialAsset::SpatialAssetParam::measurement:
						ptr->AddParameter(x.first, x.second);
						{
							ptr->GetMeasurement(x.second).SetName(ptr->Name);
							ptr->GetMeasurement(x.second).SetSpecifier(i);
							ptr->GetMeasurement(x.second).SetBoundaryType(boundary_t::BT_LOOP);
							ptr->GetMeasurement(x.second).SetInterpolationType(interpolation_t::IT_LEFT_CLAMP);
						}
						break;
					}
				}
			}
			sites.Unlock();
		}

	};

	static void					Instantiate(
		const nanodbc::connection con,
		cweeSpatialAsset& site,
		const cweeThreadedList<std::pair<cweeSpatialAsset::SpatialAssetParam, cweeStr>> initParams = cweeThreadedList<std::pair<cweeSpatialAsset::SpatialAssetParam, cweeStr>>()
	) {
		for (auto& x : initParams) {
			switch (x.first) {
			case cweeSpatialAsset::SpatialAssetParam::constantParam:
				site.AddParameter(x.first, x.second);
				break;
			case cweeSpatialAsset::SpatialAssetParam::timeseriesParam:
				site.AddParameter(x.first, x.second);
				break;
			case cweeSpatialAsset::SpatialAssetParam::measurement:
				site.AddParameter(x.first, x.second);
				{
					site.GetMeasurement(x.second).SetName(site.Name);
					site.GetMeasurement(x.second).SetSpecifier(-1);
					site.GetMeasurement(x.second).SetBoundaryType(boundary_t::BT_LOOP);
					site.GetMeasurement(x.second).SetInterpolationType(interpolation_t::IT_LEFT_CLAMP);
				}
				break;
			}
		}
	};

	static cweeThreadedList<cweeStr>	Instantiate(
		const nanodbc::connection con,
		cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& sql_select,
		const cweeStr& sql_table,
		const cweeThreadedList<std::pair<cweeSpatialAsset::SpatialAssetParam, cweeStr>> initParams = cweeThreadedList<std::pair<cweeSpatialAsset::SpatialAssetParam, cweeStr>>()
	) {
		cweeThreadedList<cweeStr> out;
		cweeStr QueryNames = cweeStr::printf("SELECT DISTINCT %s FROM %s;", odbc->SafeString(sql_select).c_str(), odbc->SafeString(sql_table).c_str());

		sites.Clear();

		for (auto& site : odbc->FirstColumnOnly(odbc->GetResults(con, QueryNames))) {
			int i = sites.Append();
			{
				sites.Lock();
				auto ptr = sites.UnsafeRead(i);
				if (ptr) {
					ptr->Name = site;
					ptr->AddParameter("Unique ID", site); // const param
				}
				sites.Unlock();
			}

			if (i == 1) {
				out.Append(site);
			}
		}

		// instantiate parameters - in case anyone queries these they need to at least "exist" to start.
		for (auto& i : sites.GetList()) {
			sites.Lock();
			cweeSpatialAsset* ptr = sites.UnsafeRead(i);
			if (ptr) {
				for (auto& x : initParams) {
					switch (x.first) {
					case cweeSpatialAsset::SpatialAssetParam::constantParam:
						ptr->AddParameter(x.first, x.second);
						break;
					case cweeSpatialAsset::SpatialAssetParam::timeseriesParam:
						ptr->AddParameter(x.first, x.second);
						break;
					case cweeSpatialAsset::SpatialAssetParam::measurement:
						ptr->AddParameter(x.first, x.second);
						{
							ptr->GetMeasurement(x.second).SetName(ptr->Name);
							ptr->GetMeasurement(x.second).SetSpecifier(i);
							ptr->GetMeasurement(x.second).SetBoundaryType(boundary_t::BT_LOOP);
							ptr->GetMeasurement(x.second).SetInterpolationType(interpolation_t::IT_LEFT_CLAMP);
						}
						break;
					}
				}
			}
			sites.Unlock();
		}

		return out;
	};

	/*! 
	Set measurement units of Pattern with the "measurementParameterKey" throughout the sites 
	*/
	static void					SetMeasurementUnits(
		cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& measurementParameterKey,
		const value_t& v,
		const measurement_t& m
	) {
		// instantiate parameters - in case anyone queries these they need to at least "exist" to start.
		for (auto& i : sites.GetList()) {
			sites.Lock();
			cweeSpatialAsset* ptr = sites.UnsafeRead(i);
			if (ptr) {
				ptr->GetMeasurement(measurementParameterKey).SetCharacteristic(v);
				ptr->GetMeasurement(measurementParameterKey).SetMeasurement(m);
			}
			sites.Unlock();
		}
	};

	/*!
	Set measurement units of Pattern (AND add default const parameters!) with the "measurementParameterKey" throughout the sites based on that site's Name
	*/
	static void					SetDefaultMeasurementUnits(
		cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& measurementParameterKey,
		const cweeStr& const_key_where = "Unique ID"
	) {
		// instantiate parameters - in case anyone queries these they need to at least "exist" to start.
		cweeStr name;
		for (auto& i : sites.GetList()) {
			sites.Lock();
			cweeSpatialAsset* ptr = sites.UnsafeRead(i);
			if (ptr) {
				if (ptr->ParameterExists(const_key_where)) {
					name = ptr->GetConstParameter(const_key_where);
				}
				else {
					name = const_key_where;
				}				
			}
			sites.Unlock();

			auto guess = GuessScadaTagPathCharacteristics(name);
			{
				sites.Lock();
				cweeSpatialAsset* ptr = sites.UnsafeRead(i);
				if (ptr) {
					ptr->AddParameter("asset_t", GetString(std::get<0>(guess)));
					ptr->AddParameter("value_t", GetString(std::get<1>(guess)));
					ptr->AddParameter("measurement_t", GetString(std::get<2>(guess)));

					if (!measurementParameterKey.IsEmpty()) {
						ptr->GetMeasurement(measurementParameterKey).SetCharacteristic(std::get<1>(guess));
						ptr->GetMeasurement(measurementParameterKey).SetMeasurement(std::get<2>(guess));
					}
				}
				sites.Unlock();
			}

		}
	};

	/*!
	Set measurement units of Pattern with the "measurementParameterKey" throughout the sites
	*/
	static void					SetMeasurementUnits(
		cweeSpatialAsset& site,
		const cweeStr& measurementParameterKey,
		const value_t& v,
		const measurement_t& m
	) {
		// instantiate parameters - in case anyone queries these they need to at least "exist" to start.
		site.GetMeasurement(measurementParameterKey).SetCharacteristic(v);
		site.GetMeasurement(measurementParameterKey).SetMeasurement(m);				
	};

	/*!
	Set measurement units of Pattern with the "measurementParameterKey" throughout the sites
	*/
	static void					SetMeasurementUnits(
		cweeSpatialAsset& site,
		const cweeStr& measurementParameterKey,
		const cweeStr& ValueMeasurementString
	) {
		auto guess = GuessScadaTagPathCharacteristics(ValueMeasurementString);
		site.GetMeasurement(measurementParameterKey).SetCharacteristic(std::get<1>(guess));
		site.GetMeasurement(measurementParameterKey).SetMeasurement(std::get<2>(guess));
	};

	/*! 
	* SELECT DISTINCT <CONST PARAM 2 VAR> FROM <TABLE> WHERE <CONST PARAM 1 VAR> IN (<CONST PARAM 1>);
	
	SELECT DISTINCT 
		<sql_select> // internally saved as <const_key_select>
	FROM 
		<sql_table>
	WHERE
		<sql_where>
	IN
		(
			<const_key_where>
		)
	;
	*/
	static cweeThreadedList<cweeStr>					ExecuteQueryAcrossList_Method1(
		const nanodbc::connection con,
		const cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& sql_select,
		const cweeStr& const_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& const_key_where = "Unique ID",
		int batchSize = 500
	) {
		cweeStr searchList;
		std::map<std::string, int> matchWhereToSites;
		int numBatches = cweeMath::Ceil(((float)sites.Num()) / (float)cweeMath::max(1, batchSize));
		int step = cweeMath::Ceil((float)sites.Num() / (float)numBatches);
		int start = 0;
		int end = start + step;
		int i = start;
		int n = sites.Num() + 1;
		cweeThreadedList<cweeStr> row;
		cweeStr query;
		cweeSpatialAsset* ptr = nullptr;
		int j;
		while (i < n) {
			matchWhereToSites.clear();
			searchList.Clear();
			auto a = clock_ns() * 0.000000001;

			if (sql_where.IsEmpty()) {
				auto noWhere = sites.GetList();

				{ // DO WORK
					// how to match results to the place to put them? Additional key.
					query = cweeStr::printf(
						"SELECT \n"
						"%s \n"
						"FROM \n"
						"%s; \n"
						,
						odbc->SafeString(sql_select).c_str(),
						odbc->SafeString(sql_table).c_str()
					);

					int replySize = 0;
					{
						auto result = odbc->Query(con, query, 100);
						while (odbc->GetNextRow(result, row)) {
							for (int k : noWhere) {
								sites.Lock();
								ptr = sites.UnsafeRead(k);
								if (ptr) {
									ptr->Append(const_key_select, row[0]);
								}
								sites.Unlock();
							}

							replySize++;
						}
					}
					if (replySize == 0) {
						auto result = odbc->Query(con, query, 100);
						while (odbc->GetNextRow(result, row)) {
							for (int k : noWhere) {
								sites.Lock();
								ptr = sites.UnsafeRead(k);
								if (ptr) {
									ptr->Append(const_key_select, row[0]);
								}
								sites.Unlock();
							}

						}
					}
				}

				i = end;
			}
			else {

				{ // DO WORK
					for (; i < end && i < n; i++) {
						sites.Lock();
						ptr = sites.UnsafeRead(i);
						if (ptr) {
							if (!ptr->GetConstParameter(const_key_select).IsEmpty()) {
								cweeStr& WhereVal = ptr->GetConstParameter(const_key_where);
								matchWhereToSites[std::string(WhereVal.c_str())] = i;
								searchList.AddToDelimiter("'" + WhereVal + "'", ",");
							}
						}
						sites.Unlock();
					}

					// how to match results to the place to put them? Additional key.
					query = cweeStr::printf(
						"SELECT \n"
						"%s,%s \n"
						"FROM \n"
						"%s \n"
						"WHERE \n"
						"%s \n"
						"IN \n"
						"(%s); "
						,
						odbc->SafeString(sql_where).c_str(), odbc->SafeString(sql_select).c_str(),
						odbc->SafeString(sql_table).c_str(),
						odbc->SafeString(sql_where).c_str(),
						searchList.c_str()
					);

					int replySize = 0;
					{
						auto result = odbc->Query(con, query, 100);
						while (odbc->GetNextRow(result, row)) {
							// Expect exactly two columns in the reply per row.
							j = matchWhereToSites[std::string(row[0].c_str())]; if (j==0) break;
							sites.Lock();
							ptr = sites.UnsafeRead(j);
							if (ptr) {
								ptr->Append(const_key_select, row[1]);
							}
							sites.Unlock();
							replySize++;
						}
					}
					if (replySize == 0) {
						auto result = odbc->Query(con, query, 100);
						while (odbc->GetNextRow(result, row)) {
							// Expect exactly two columns in the reply per row.
							j = matchWhereToSites[std::string(row[0].c_str())]; if (j==0) break;
							sites.Lock();
							ptr = sites.UnsafeRead(j);
							if (ptr) {
								ptr->Append(const_key_select, row[1]);
							}
							sites.Unlock();
						}
					}
				}

			}


			end = end + step;
			auto b = clock_ns() * 0.000000001;

#if 1
			std::cout << i << " / " << n << ". Est. Completion Time: " << cweeStr((time_t)(fileSystem->getCurrentTime() + (b - a) * (n - i))) << std::endl;
#endif
		}

		return row;
	};

	/*! 
	* SELECT DISTINCT <UNIX TIME PARAM 1 VAR><TIMESERIES PARAM 1 VAR> FROM <TABLE> WHERE <CONST PARAM 1 VAR> IN (<CONST PARAM 1>);
	
	SELECT DISTINCT
		<sql_select_unixTime>,<sql_select> // internally saved as <ts_key_select>
	FROM
		<sql_table>
	WHERE
		<sql_where>
	IN
		(
			<const_key_where>
		)
	;
	*/
	static cweeThreadedList<cweeStr>					ExecuteQueryAcrossList_Method2(
		const nanodbc::connection con,
		const cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& sql_select_unixTime, 
		const cweeStr& sql_select_value,
		const cweeStr& ts_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& const_key_where = "Unique ID",
		int batchSize = 500
	) {
		cweeStr searchList;
		std::map<std::string, cweeThreadedList<int>> matchWhereToSites;

		int numBatches = cweeMath::Ceil(((float)sites.Num()) / (float)cweeMath::max(1, batchSize));
		int step = cweeMath::Ceil((float)sites.Num() / (float)numBatches);
		int start = 0;
		int end = start + step;
		int i = start;
		int n = sites.Num() + 1;
		cweeThreadedList<cweeStr> row;
		cweeStr query;
		cweeSpatialAsset* ptr = nullptr;
		int j;
		while (i < n) {
			matchWhereToSites.clear();
			searchList.Clear();
			auto a = clock_ns() * 0.000000001;

			if (sql_where.IsEmpty()) {
				auto noWhere = sites.GetList();

				{ // DO WORK
					// how to match results to the place to put them? Additional key.
					query = cweeStr::printf(
						"SELECT "
						"%s,%s "
						"FROM "
						"%s; "
						,
						odbc->SafeString(sql_select_unixTime).c_str(), odbc->SafeString(sql_select_value).c_str(),
						odbc->SafeString(sql_table).c_str()
					);

					int replySize = 0; {
						auto result = odbc->Query(con, query, 100);
						cweeThreadedList<int> list;

						while (odbc->GetNextRow(result, row)) {
							for (auto& k : noWhere) {
								sites.Lock();
								ptr = sites.UnsafeRead(k);
								if (ptr) {
									ptr->Append(ts_key_select, (u64)row[0], row[1]);
								}
								sites.Unlock();
							}
							replySize++;
						}
					}
					if (replySize == 0) { // try again.
						auto result = odbc->Query(con, query, 100);
						replySize = 0;
						while (odbc->GetNextRow(result, row)) {
							for (int k : noWhere) {
								sites.Lock();
								ptr = sites.UnsafeRead(k);
								if (ptr) {
									ptr->Append(ts_key_select, (u64)row[0], row[1]);
								}
								sites.Unlock();
							}
							replySize++;
						}

					}
				}
				i = end;
			} 
			else {

				{ // DO WORK
					for (; i < end && i < n; i++) {
						sites.Lock();
						ptr = sites.UnsafeRead(i);
						if (ptr) {
							cweeStr& WhereVal = ptr->GetConstParameter(const_key_where);

							if (matchWhereToSites.find(std::string(WhereVal.c_str())) == matchWhereToSites.end())
								matchWhereToSites[std::string(WhereVal.c_str())] = cweeThreadedList<int>();
							matchWhereToSites[std::string(WhereVal.c_str())].Append(i);

							searchList.AddToDelimiter("'" + WhereVal + "'", ",");
							
						}
						sites.Unlock();
					}

					// how to match results to the place to put them? Additional key.
					query = cweeStr::printf(
						"SELECT "
						"%s,%s,%s "
						"FROM "
						"%s "
						"WHERE "
						"%s "
						"IN "
						"(%s); "
						,
						odbc->SafeString(sql_where).c_str(), odbc->SafeString(sql_select_unixTime).c_str(), odbc->SafeString(sql_select_value).c_str(),
						odbc->SafeString(sql_table).c_str(),
						odbc->SafeString(sql_where).c_str(),
						searchList.c_str()
					);
					int replySize = 0;
					{
						auto result = odbc->Query(con, query, 100);
						cweeThreadedList<int> list;

						while (odbc->GetNextRow(result, row)) {
							// Expect exactly three columns in the reply per row.
							for (auto& j : matchWhereToSites[std::string(row[0].c_str())]) {
								sites.Lock();
								ptr = sites.UnsafeRead(j);
								if (ptr) {
									ptr->Append(ts_key_select, (u64)row[1], row[2]);
								}
								sites.Unlock();
							}
							replySize++;
						}
					}
					if (replySize == 0) { // try again.
						auto result = odbc->Query(con, query, 100);
						replySize = 0;
						while (odbc->GetNextRow(result, row)) {
							// Expect exactly three columns in the reply per row.
							for (auto& j : matchWhereToSites[std::string(row[0].c_str())]) {
								sites.Lock();
								ptr = sites.UnsafeRead(j);
								if (ptr) {
									ptr->Append(ts_key_select, (u64)row[1], row[2]);
								}
								else {
									int jkj = 0;
									jkj++;
									jkj = NULL;
								}
								sites.Unlock();
							}
							replySize++;
						}

					}
				}

			}

			end = end + step;
			auto b = clock_ns() * 0.000000001;

#if 1
			std::cout << i << " / " << n << ". Est. Completion Time: " << cweeStr((time_t)(fileSystem->getCurrentTime() + (b - a) * (n - i))) << std::endl;
#endif
		}

		return row;
	};

	/*!
	* SELECT DISTINCT <TIMESERIES PARAM 2 VAR> FROM <TABLE> WHERE <TIMESERIES PARAM 1 VAR> IN (<TIMESERIES PARAM 1>);

	SELECT DISTINCT
		<sql_select_value> // internally saved as <ts_key_select>
	FROM
		<sql_table>
	WHERE
		<sql_where>
	IN
		(
			<ts_key_where>
		)
	;
	*/
	static cweeThreadedList<cweeStr>					ExecuteQueryAcrossList_Method3(
		const nanodbc::connection con,
		const cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& sql_select_value,
		const cweeStr& ts_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& ts_key_where,
		int batchSize = 500
	) {
		cweeStr searchList;
		std::map<std::string, cweeThreadedList<int>> matchWhereToSites;
		std::map<std::string, cweeThreadedList<u64>> matchWhereToTimes;

		int numBatches = cweeMath::Ceil(((float)sites.Num()) / (float)cweeMath::max(1, batchSize));
		int step = cweeMath::Ceil((float)sites.Num() / (float)numBatches);
		int start = 0;
		int end = start + step;
		int i = start;
		int n = sites.Num() + 1;
		cweeThreadedList<cweeStr> row;
		cweeStr query;
		cweeSpatialAsset* ptr = nullptr;
		int j; u64 k;
		while (i < n) {
			matchWhereToSites.clear();
			matchWhereToTimes.clear();
			searchList.Clear();
			auto a = clock_ns() * 0.000000001;

			{ // DO WORK
				for (; i < end && i < n; i++) {
					sites.Lock();
					ptr = sites.UnsafeRead(i);
					if (ptr) {
						auto curve = ptr->GetTimeSeriesParameter(ts_key_where);
						for (auto& x : curve.GetKnotSeries()) {
							if (matchWhereToSites.find(std::string(x.second.c_str())) == matchWhereToSites.end()) {
								matchWhereToSites[std::string(x.second.c_str())] = cweeThreadedList<int>();
								matchWhereToTimes[std::string(x.second.c_str())] = cweeThreadedList<u64>();
							}
							matchWhereToSites[std::string(x.second.c_str())].Append(i);
							matchWhereToTimes[std::string(x.second.c_str())].Append((u64)x.first);
								
							searchList.AddToDelimiter("'" + x.second + "'", ",");
						}
					}
					sites.Unlock();
				}

				// how to match results to the place to put them? Additional key.
				query = cweeStr::printf(
					"SELECT \n"
					"%s,%s \n"
					"FROM \n"
					"%s \n"
					"WHERE \n"
					"%s \n"
					"IN \n"
					"(%s); "
					,
					odbc->SafeString(sql_where).c_str(), odbc->SafeString(sql_select_value).c_str(),
					odbc->SafeString(sql_table).c_str(),
					odbc->SafeString(sql_where).c_str(),
					searchList.c_str()
				);

				int replySize = 0;
				{
					auto result = odbc->Query(con, query, 100);

					while (odbc->GetNextRow(result, row)) {
						// Expect exactly two columns in the reply per row.
						auto& site_indexs = matchWhereToSites[std::string(row[0].c_str())];
						auto& index_times = matchWhereToTimes[std::string(row[0].c_str())];

						for (j = 0; j < site_indexs.Num(); j++) {
							sites.Lock();
							ptr = sites.UnsafeRead(site_indexs[j]);
							if (ptr) {
								ptr->Append(ts_key_select, index_times[j], row[1]);
							}
							sites.Unlock();
						}
						replySize++;
					}
				}
				if (replySize == 0) {
					auto result = odbc->Query(con, query, 100);
					while (odbc->GetNextRow(result, row)) {
						// Expect exactly two columns in the reply per row.
						auto& site_indexs = matchWhereToSites[std::string(row[0].c_str())];
						auto& index_times = matchWhereToTimes[std::string(row[0].c_str())];

						for (j = 0; j < site_indexs.Num(); j++) {
							sites.Lock();
							ptr = sites.UnsafeRead(site_indexs[j]);
							if (ptr) {
								ptr->Append(ts_key_select, index_times[j], row[1]);
							}
							sites.Unlock();
						}
					}
				}

			}
		
			end = end + step;
			auto b = clock_ns() * 0.000000001;

#if 1
			std::cout << i << " / " << n << ". Est. Completion Time: " << cweeStr((time_t)(fileSystem->getCurrentTime() + (b - a) * (n - i))) << std::endl;
#endif
		}

		return row;
	};

	/*!
	* SELECT DISTINCT <MEASUREMENT PARAM 1 VAR> FROM <TABLE> WHERE <TIMESERIES PARAM 1 VAR> IN (<TIMESERIES PARAM 1>);

	SELECT DISTINCT
		<sql_select_value> // internally saved as <meas_key_select>
	FROM
		<sql_table>
	WHERE
		<sql_where>
	IN
		(
			<ts_key_where>
		)
	;
	*/
	static cweeThreadedList<cweeStr>					ExecuteQueryAcrossList_Method4(
		const nanodbc::connection con,
		const cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& sql_select_value,
		const cweeStr& meas_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& ts_key_where,
		int batchSize = 500
	) {
		cweeStr searchList;
		std::map<std::string, cweeThreadedList<int>> matchWhereToSites;
		std::map<std::string, cweeThreadedList<u64>> matchWhereToTimes;

		int numBatches = cweeMath::Ceil(((float)sites.Num()) / (float)cweeMath::max(1, batchSize));
		int step = cweeMath::Ceil((float)sites.Num() / (float)numBatches);
		int start = 0;
		int end = start + step;
		int i = start;
		int n = sites.Num() + 1;
		cweeThreadedList<cweeStr> row;
		cweeStr query;
		cweeSpatialAsset* ptr = nullptr;
		int j; u64 k;
		while (i < n) {
			matchWhereToSites.clear();
			matchWhereToTimes.clear();
			searchList.Clear();
			auto a = clock_ns() * 0.000000001;
			{ // DO WORK
				for (; i < end && i < n; i++) {
					sites.Lock();
					ptr = sites.UnsafeRead(i);
					if (ptr) {
						auto curve = ptr->GetTimeSeriesParameter(ts_key_where);
						for (auto& x : curve.GetKnotSeries()) {
							if (matchWhereToSites.find(std::string(x.second.c_str())) == matchWhereToSites.end()) {
								matchWhereToSites[std::string(x.second.c_str())] = cweeThreadedList<int>();
								matchWhereToTimes[std::string(x.second.c_str())] = cweeThreadedList<u64>();
							}
							matchWhereToSites[std::string(x.second.c_str())].Append(i);
							matchWhereToTimes[std::string(x.second.c_str())].Append((u64)x.first);

							searchList.AddToDelimiter("'" + x.second + "'", ",");
						}
					}
					sites.Unlock();
				}

				// how to match results to the place to put them? Additional key.
				query = cweeStr::printf(
					"SELECT \n"
					"%s,%s \n"
					"FROM \n"
					"%s \n"
					"WHERE \n"
					"%s \n"
					"IN \n"
					"(%s); "
					,
					odbc->SafeString(sql_where).c_str(), odbc->SafeString(sql_select_value).c_str(),
					odbc->SafeString(sql_table).c_str(),
					odbc->SafeString(sql_where).c_str(),
					searchList.c_str()
				);
				int replySize = 0;
				{
					auto result = odbc->Query(con, query, 100);

					while (odbc->GetNextRow(result, row)) {
						// Expect exactly two columns in the reply per row.
						auto& site_indexs = matchWhereToSites[std::string(row[0].c_str())];
						auto& index_times = matchWhereToTimes[std::string(row[0].c_str())];

						for (j = 0; j < site_indexs.Num(); j++) {
							sites.Lock();
							ptr = sites.UnsafeRead(site_indexs[j]);
							if (ptr) {
								ptr->Append(meas_key_select, index_times[j], (float)row[1]);
							}
							sites.Unlock();
						}
						replySize++;
					}
				}
				if (replySize == 0) {
					auto result = odbc->Query(con, query, 100);
					while (odbc->GetNextRow(result, row)) {
						// Expect exactly two columns in the reply per row.
						auto& site_indexs = matchWhereToSites[std::string(row[0].c_str())];
						auto& index_times = matchWhereToTimes[std::string(row[0].c_str())];

						for (j = 0; j < site_indexs.Num(); j++) {
							sites.Lock();
							ptr = sites.UnsafeRead(site_indexs[j]);
							if (ptr) {
								ptr->Append(meas_key_select, index_times[j], (float)row[1]);
							}
							sites.Unlock();
						}
					}
				}

			}
			end = end + step;
			auto b = clock_ns() * 0.000000001;

#if 1
			std::cout << i << " / " << n << ". Est. Completion Time: " << cweeStr((time_t)(fileSystem->getCurrentTime() + (b - a) * (n - i))) << std::endl;
#endif
		}

		for (auto& ind : sites.GetList()) {
			sites.Lock();
			ptr = sites.UnsafeRead(ind);
			if (ptr) {
				ptr->GetMeasurement(meas_key_select).RemoveUnnecessaryKnots();
			}
			sites.Unlock();
		}

		return row;
	};

	/*!
	* SELECT DISTINCT <UNIX TIME PARAM 1 VAR><MEASUREMENT PARAM 1 VAR> FROM <TABLE> WHERE <CONST PARAM 1 VAR> IN (<CONST PARAM 1>);

	SELECT DISTINCT
		<sql_select_unixTime>,<sql_select> // internally saved as <meas_key_select>
	FROM
		<sql_table>
	WHERE
		<sql_where>
	IN
		(
			<const_key_where>
		)
	;
	*/
	static cweeThreadedList<cweeStr>					ExecuteQueryAcrossList_Method5(
		const nanodbc::connection con,
		const cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& sql_select_unixTime,
		const cweeStr& sql_select_value,
		const cweeStr& meas_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& const_key_where = "Unique ID",
		int batchSize = 500
	) {
		cweeStr searchList;
		std::map<std::string, cweeThreadedList<int>> matchWhereToSites;

		int numBatches = cweeMath::Ceil(((float)sites.Num()) / (float)cweeMath::max(1, batchSize));
		int step = cweeMath::Ceil((float)sites.Num() / (float)numBatches);
		int start = 0;
		int end = start + step;
		int i = start;
		int n = sites.Num() + 1;
		cweeThreadedList<cweeStr> row;
		cweeStr query;
		cweeSpatialAsset* ptr = nullptr;
		int j;
		while (i < n) {
			matchWhereToSites.clear();
			searchList.Clear();
			auto a = clock_ns() * 0.000000001;
			{ // DO WORK
				for (; i < end && i < n; i++) {
					sites.Lock();
					ptr = sites.UnsafeRead(i);
					if (ptr) {
						cweeStr& WhereVal = ptr->GetConstParameter(const_key_where);

						if (matchWhereToSites.find(std::string(WhereVal.c_str())) == matchWhereToSites.end())
							matchWhereToSites[std::string(WhereVal.c_str())] = cweeThreadedList<int>();
						matchWhereToSites[std::string(WhereVal.c_str())].Append(i);

						searchList.AddToDelimiter("'" + WhereVal + "'", ",");
					}
					sites.Unlock();
				}

				// how to match results to the place to put them? Additional key.
				query = cweeStr::printf(
					"SELECT "
					"%s,%s,%s "
					"FROM "
					"%s "
					"WHERE "
					"%s "
					"IN "
					"(%s)"
					"ORDER BY "
					"%s,%s,%s; "
					,
					odbc->SafeString(sql_where).c_str(), odbc->SafeString(sql_select_unixTime).c_str(), odbc->SafeString(sql_select_value).c_str(),
					odbc->SafeString(sql_table).c_str(),
					odbc->SafeString(sql_where).c_str(),
					searchList.c_str(),
					odbc->SafeString(sql_where).c_str(), odbc->SafeString(sql_select_unixTime).c_str(), odbc->SafeString(sql_select_value).c_str()
				);

				int replySize = 0;
				{
					auto result = odbc->Query(con, query, 100);
					cweeThreadedList<int> list;

					while (odbc->GetNextRow(result, row)) {
						// Expect exactly three columns in the reply per row.
						for (auto& j : matchWhereToSites[std::string(row[0].c_str())]) {
							sites.Lock();
							ptr = sites.UnsafeRead(j);
							if (ptr) {
								ptr->Append(meas_key_select, (u64)row[1], (float)row[2]);
							}
							sites.Unlock();
						}
						replySize++;
					}
				}
				if (replySize == 0) { // try again.
					auto result = odbc->Query(con, query, 100);
					replySize = 0;
					while (odbc->GetNextRow(result, row)) {
						// Expect exactly three columns in the reply per row.
						for (auto& j : matchWhereToSites[std::string(row[0].c_str())]) {
							sites.Lock();
							ptr = sites.UnsafeRead(j);
							if (ptr) {
								ptr->Append(meas_key_select, (u64)row[1], (float)row[2]);
							}
							else {
								int jkj = 0;
								jkj++;
								jkj = NULL;
							}
							sites.Unlock();
						}
						replySize++;
					}

				}
			}
			end = end + step;
			auto b = clock_ns() * 0.000000001;

#if 1
			std::cout << i << " / " << n << ". Est. Completion Time: " << cweeStr((time_t)(fileSystem->getCurrentTime() + (b - a) * (n - i))) << std::endl;
#endif
		}

		for (auto& ind : sites.GetList()) {
			sites.Lock();
			ptr = sites.UnsafeRead(ind);
			if (ptr) {
				ptr->GetMeasurement(meas_key_select).RemoveUnnecessaryKnots();
			}
			sites.Unlock();
		}

		return row;
	};

	/*!
	* SELECT DISTINCT <UNIX TIME PARAM 1 VAR><MEASUREMENT PARAM 1 VAR> FROM <TABLE> WHERE <CONST PARAM 1 VAR> IN (<CONST PARAM 1>);

	SELECT DISTINCT
		<sql_select_unixTime>,<sql_select> // internally saved as <meas_key_select>
	FROM
		<sql_table>
	WHERE
		<sql_where>
	IN
		(
			<const_key_where>
		)
	;
	*/
	static cweeThreadedList<cweeStr>					ExecuteQueryAcrossList_Method6(
		const nanodbc::connection con,
		const cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& sql_select_unixTime,
		const cweeStr& sql_select_value,
		const cweeStr& meas_key_select,
		cweeSpatialAsset& globalObj,
		const cweeStr& global_ts_table_key,
		const cweeStr& sql_where,
		const cweeStr& const_key_where = "Unique ID",
		int batchSize = 500
	) {
		cweeStr searchList;
		std::map<std::string, cweeThreadedList<int>> matchWhereToSites;

		int numBatches = cweeMath::Ceil(((float)sites.Num()) / (float)cweeMath::max(1, batchSize));
		int step = cweeMath::Ceil((float)sites.Num() / (float)numBatches);
		int start = 0;
		int end = start + step;
		int i = start;
		int n = sites.Num() + 1;
		cweeThreadedList<cweeStr> row;
		cweeStr query;
		cweeSpatialAsset* ptr = nullptr;
		int j;
		while (i < n) {
			u64 earliest = std::numeric_limits<u64>::max();
			u64 tempT = std::numeric_limits<u64>::max();
			for (int k = i; k < end && k < n; k++) {
				sites.Lock();
				ptr = sites.UnsafeRead(k);
				if (ptr) {
					tempT = ptr->GetMeasurement(meas_key_select).GetMaxTime();					
				}
				sites.Unlock();
				if (tempT < earliest) earliest = tempT;
				if (earliest <= 0) break;
			}

			matchWhereToSites.clear();
			searchList.Clear();
			auto a = clock_ns() * 0.000000001;

			{ // DO WORK
				for (; i < end && i < n; i++) {
					sites.Lock();
					ptr = sites.UnsafeRead(i);
					if (ptr) {
						cweeStr& WhereVal = ptr->GetConstParameter(const_key_where);

						if (matchWhereToSites.find(std::string(WhereVal.c_str())) == matchWhereToSites.end())
							matchWhereToSites[std::string(WhereVal.c_str())] = cweeThreadedList<int>();
						matchWhereToSites[std::string(WhereVal.c_str())].Append(i);

						searchList.AddToDelimiter("'" + WhereVal + "'", ",");
					}
					sites.Unlock();
				}


				for (auto& t_pair : globalObj.GetTimeSeriesParameter(global_ts_table_key).GetKnotSeries()) {
					u64 d = fileSystem->getLastDayOfSameMonth(t_pair.first);
					if (d < earliest) continue;
					auto& t = t_pair.second;

					// how to match results to the place to put them? Additional key.
					query = cweeStr::printf(
						"SELECT "
						"%s,%s,%s "
						"FROM "
						"%s "
						"WHERE "
						"%s "
						"IN "
						"(%s); "
						,
						odbc->SafeString(sql_where).c_str(), odbc->SafeString(sql_select_unixTime).c_str(), odbc->SafeString(sql_select_value).c_str(),
						odbc->SafeString(t).c_str(),
						odbc->SafeString(sql_where).c_str(),
						searchList.c_str()
					);
					int replySize = 0;
					{
						auto result = odbc->Query(con, query, 100);
						cweeThreadedList<int> list;
						while (odbc->GetNextRow(result, row)) {
							// Expect exactly three columns in the reply per row.
							list = matchWhereToSites[std::string(row[0].c_str())];
							for (auto& j : list) {
								sites.Lock();
								ptr = sites.UnsafeRead(j);
								if (ptr) {
									ptr->Append(meas_key_select, (u64)row[1], (float)row[2]);
								}
								sites.Unlock();
							}
							replySize++;
						}
					}
					if (replySize == 0) { // try again.
						auto result = odbc->Query(con, query, 100);
						replySize = 0;
						while (odbc->GetNextRow(result, row)) {
							// Expect exactly three columns in the reply per row.
							for (auto& j : matchWhereToSites[std::string(row[0].c_str())]) {
								sites.Lock();
								ptr = sites.UnsafeRead(j);
								if (ptr) {
									ptr->Append(meas_key_select, (u64)row[1], (float)row[2]);
								}
								else {
									int jkj = 0;
									jkj++;
									jkj = NULL;
								}
								sites.Unlock();
							}
							replySize++;
						}
					}

					for (auto& ind : sites.GetList()) {
						sites.Lock();
						ptr = sites.UnsafeRead(ind);
						if (ptr) {
							ptr->GetMeasurement(meas_key_select).RemoveUnnecessaryKnots();
						}
						sites.Unlock();
					}
				}
			}



			end = end + step;
			auto b = clock_ns() * 0.000000001;

#if 1
			std::cout << i << " / " << n << ". Est. Completion Time: " << cweeStr((time_t)(fileSystem->getCurrentTime() + (b - a) * (n - i))) << std::endl;
#endif
		}

		for (auto& ind : sites.GetList()) {
			sites.Lock();
			ptr = sites.UnsafeRead(ind);
			if (ptr) {
				ptr->GetMeasurement(meas_key_select).RemoveUnnecessaryKnots();
			}
			sites.Unlock();
		}

		return row;
	};


	/*!
	* SELECT DISTINCT <MEASUREMENT PARAM 1 VAR> FROM <TABLE> WHERE <TIMESERIES PARAM 1 VAR> IN (<TIMESERIES PARAM 1>);

	SELECT DISTINCT
		<sql_select_value> // internally saved as <meas_key_select>
	FROM
		<sql_table>
	WHERE
		<sql_where>
	IN
		(
			<ts_key_where>
		)
	;
	*/
	static cweeThreadedList<cweeStr>					ExecuteQueryAcrossList_Method7(
		const nanodbc::connection con,
		const cweeUnorderedList<cweeSpatialAsset>& sites,
		const cweeStr& sql_select_unixTime,
		const cweeStr& sql_select_value,
		const cweeStr& meas_key_select,
		cweeSpatialAsset& globalObj,
		const cweeStr& global_ts_table_key,
		const cweeStr& sql_where,
		const cweeStr& ts_key_where,
		int batchSize = 500
	) {
		cweeStr searchList;
		std::map<std::string, cweeThreadedList<int>> matchWhereToSites;

		int numBatches = cweeMath::Ceil(((float)sites.Num()) / (float)cweeMath::max(1, batchSize));
		int step = cweeMath::Ceil((float)sites.Num() / (float)numBatches);
		int start = 0;
		int end = start + step;
		int i = start;
		int n = sites.Num() + 1;
		cweeThreadedList<cweeStr> row;
		cweeStr query;
		cweeSpatialAsset* ptr = nullptr;
		int j; u64 k;		

		while (i < n) {
			u64 earliest = std::numeric_limits<u64>::max();
			u64 tempT = std::numeric_limits<u64>::max();
			for (int k = i; k < end && k < n; k++) {
				sites.Lock();
				ptr = sites.UnsafeRead(k);
				if (ptr) {
					tempT = ptr->GetMeasurement(meas_key_select).GetMaxTime();
					ptr->GetMeasurement(meas_key_select).SetGranularity(10000); // assume 10000 measurements
				}
				sites.Unlock();
				if (tempT < earliest) earliest = tempT;
				if (earliest <= 0) break;
			}

			matchWhereToSites.clear();
			searchList.Clear();
			auto a = clock_ns() * 0.000000001;

				{ // DO WORK
					for (; i < end && i < n; i++) {
						sites.Lock();
						ptr = sites.UnsafeRead(i);
						if (ptr) {
							cweeCurve<cweeStr>& curve = ptr->GetTimeSeriesParameter(ts_key_where);
							for (auto& x : curve.GetKnotSeries()) {
								if (matchWhereToSites.find(std::string(x.second.c_str())) == matchWhereToSites.end()) {
									matchWhereToSites[std::string(x.second.c_str())] = cweeThreadedList<int>(curve.GetNumValues() + 1);
								}
								matchWhereToSites[std::string(x.second.c_str())].Append(i);

								searchList.AddToDelimiter("'" + x.second + "'", ",");
							}
						}
						sites.Unlock();
					}
					
					u64 d; cweeStr t; int replySize; auto list = sites.GetList(); cweeThreadedList<int>* tagIds = nullptr; cweeStr prevTagPath;
					for (auto& t_pair : globalObj.GetTimeSeriesParameter(global_ts_table_key).GetKnotSeries()) {
						d = fileSystem->getLastDayOfSameMonth(t_pair.first);
						if (d < earliest) continue;
						t = t_pair.second;

						// how to match results to the place to put them? Additional key.
						query = cweeStr::printf(
							"SELECT "
							"%s,%s,%s "
							"FROM "
							"%s "
							"WHERE "
							"%s "
							"IN "
							"(%s)"
							"ORDER BY "
							"%s,%s,%s; "
							,
							odbc->SafeString(sql_where).c_str(), odbc->SafeString(sql_select_unixTime).c_str(), odbc->SafeString(sql_select_value).c_str(),
							odbc->SafeString(t).c_str(),
							odbc->SafeString(sql_where).c_str(),
							searchList.c_str(),
							odbc->SafeString(sql_where).c_str(), odbc->SafeString(sql_select_unixTime).c_str(), odbc->SafeString(sql_select_value).c_str()
						);
						replySize = 0;
						{
							auto result = odbc->Query(con, query, 100);							
							while (odbc->GetNextRow(result, row)) {
								// Expect exactly three columns in the reply per row.
								if (prevTagPath != row[0]) {
									tagIds = &matchWhereToSites[std::string(row[0].c_str())];
								}
								if (tagIds) {
									for (auto& j : *tagIds) {
										sites.Lock();
										ptr = sites.UnsafeRead(j);
										if (ptr) {
											ptr->Append(meas_key_select, (u64)row[1], (float)row[2]);
										}
										sites.Unlock();
									}
								}
								replySize++;
							}
						}
						{
							if (replySize == 0) { // try again.
								auto result = odbc->Query(con, query, 100);
								replySize = 0;

								while (odbc->GetNextRow(result, row)) {
									// Expect exactly three columns in the reply per row.
									if (prevTagPath != row[0]) {
										tagIds = &matchWhereToSites[std::string(row[0].c_str())];
									}
									if (tagIds) {
										for (auto& j : *tagIds) {
											sites.Lock();
											ptr = sites.UnsafeRead(j);
											if (ptr) {
												ptr->Append(meas_key_select, (u64)row[1], (float)row[2]);
											}
											sites.Unlock();
										}
									}
									replySize++;
								}
							}
						}

						for (auto& ind : list) {
							sites.Lock();
							ptr = sites.UnsafeRead(ind);
							if (ptr) {
								ptr->GetMeasurement(meas_key_select).ReduceMemory(1);
							}
							sites.Unlock();
						}
					}
				}

			

			end = end + step;
			auto b = clock_ns() * 0.000000001;

#if 1
			std::cout << i << " / " << n << ". Est. Completion Time: " << cweeStr((time_t)(fileSystem->getCurrentTime() + (b - a) * (n - i))) << std::endl;
#endif
		}

		return row;
	};

	static cweeThreadedList<cweeStr>					ExecuteQuery_Method1(
		const nanodbc::connection con,
		cweeSpatialAsset& site,
		const cweeStr& sql_select,
		const cweeStr& const_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& const_key_where = "Unique ID"
	) {
		cweeUnorderedList<cweeSpatialAsset> sites;
		auto i = sites.Append();
		sites.Swap(i, site);

		auto out = ExecuteQueryAcrossList_Method1(con, sites, sql_select, const_key_select, sql_table, sql_where, const_key_where);

		site = sites[i];

		return out;
	};

	static cweeThreadedList<cweeStr>					ExecuteQuery_Method2(
		const nanodbc::connection con,
		cweeSpatialAsset& site,
		const cweeStr& sql_select_unixTime,
		const cweeStr& sql_select_value,
		const cweeStr& ts_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& const_key_where = "Unique ID"
	) {
		cweeUnorderedList<cweeSpatialAsset> sites;
		auto i = sites.Append();
		sites.Swap(i,site);

		auto out = ExecuteQueryAcrossList_Method2(con, sites, sql_select_unixTime, sql_select_value, ts_key_select, sql_table, sql_where, const_key_where);

		site = sites[i];

		return out;
	};

	static cweeThreadedList<cweeStr>					ExecuteQuery_Method3(
		const nanodbc::connection con,
		cweeSpatialAsset& site,
		const cweeStr& sql_select_value,
		const cweeStr& ts_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& ts_key_where = "Unique ID"
	) {
		cweeUnorderedList<cweeSpatialAsset> sites;
		auto i = sites.Append();
		sites.Swap(i, site);

		auto out = ExecuteQueryAcrossList_Method3(con, sites, sql_select_value, ts_key_select, sql_table, sql_where, ts_key_where);

		site = sites[i];

		return out;
	};

	static cweeThreadedList<cweeStr>					ExecuteQuery_Method4(
		const nanodbc::connection con,
		cweeSpatialAsset& site,
		const cweeStr& sql_select_value,
		const cweeStr& meas_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& ts_key_where = "Unique ID"
	) {
		cweeUnorderedList<cweeSpatialAsset> sites;
		auto i = sites.Append();
		sites.Swap(i, site);

		auto out = ExecuteQueryAcrossList_Method4(con, sites, sql_select_value, meas_key_select, sql_table, sql_where, ts_key_where);

		site = sites[i];

		return out;
	};

	static cweeThreadedList<cweeStr>					ExecuteQuery_Method5(
		const nanodbc::connection con,
		cweeSpatialAsset& site,
		const cweeStr& sql_select_unixTime,
		const cweeStr& sql_select_value,
		const cweeStr& meas_key_select,
		const cweeStr& sql_table,
		const cweeStr& sql_where,
		const cweeStr& const_key_where = "Unique ID"
	) {
		cweeUnorderedList<cweeSpatialAsset> sites;
		auto i = sites.Append();
		sites.Swap(i, site);

		auto out = ExecuteQueryAcrossList_Method5(con, sites, sql_select_unixTime, sql_select_value, meas_key_select, sql_table, sql_where, const_key_where);

		site = sites[i];

		return out;
	};
};


class cweeThreadedSpatialAsset {
public: // ENUMS AND TYPEDEFS
	enum class SpatialAssetParam
	{
		missingParam = -1,
		constantParam = 0,
		timeseriesParam = 1,
		measurement = 2
	};
	typedef cweePair<SpatialAssetParam, int> assetParameter;

public: // PUBLIC FUNCTIONS

	cweeThreadedSpatialAsset& operator=(const cweeThreadedSpatialAsset& in) {
		SetName(in.GetName());
		
		Lock();
		in.Lock();
		{		
			prev_index = in.prev_index;
			prev_search = in.prev_search;
			parameters = in.parameters;
		}
		in.Unlock();
		Unlock();

		timeseries->Copy(*in.timeseries.get());
		measurements->Copy(*in.measurements.get());
		constants->Copy(*in.constants.get());

		return *this;
	};
	void Clear() {
		Lock();
			prev_index = assetParameter();
			prev_search = "";
			parameters.clear();
		Unlock();

		measurements->Clear();
		timeseries->Clear();
		constants->Clear();
	};
	cweeThreadedSpatialAsset() {
		//measurements->SetGranularity(1); // set small so as not to suddenly require 2GB of memory on use of 1 pattern
		//timeseries->SetGranularity(2);
		//constants->SetGranularity(4);
	};

	/*! Add a parameter to the object of the specified category with the provided string key */
	void						AddParameter(const SpatialAssetParam& type, const cweeStr& name) {
		_addParameter(type, name);
	};
	/*! Add a constant string parameter to the object with the specified name, initialized with the provided object */
	void						AddParameter(const cweeStr& name, const cweeStr& param) {
		auto p = _addParameter(SpatialAssetParam::constantParam, name);
		auto x = GetConstParameter(p);
		if (x) {
			x->Lock();
			*x->UnsafeRead() = param;
			x->Unlock();
		}
	};
	/*! Add a time-verying string parameter to the object with the specified name, initialized with the provided object */
	void						AddParameter(const cweeStr& name, const cweeCurve<cweeStr>& param) {
		auto p = _addParameter(SpatialAssetParam::timeseriesParam, name);
		auto x = GetTimeSeriesParameter(p);
		if (x) {
			x->Lock();
			*x->UnsafeRead() = param;
			x->Unlock();
		}
	};
	/*! Add a time-varying measurement to the object with the specified name, initialized with the provided object */
	void						AddParameter(const cweeStr& name, const Pattern& param) {
		auto p = _addParameter(SpatialAssetParam::measurement, name);
		auto x = GetMeasurement(p);
		if (x) {
			x = param;
		}
	};
	/*! Get the number of parameters in the object */
	int							Num() const {
		return constants->Num() + timeseries->Num() + measurements->Num();
	};
	/*! Get the nuber of parameters in the object of the requested type  */
	int							Num(const SpatialAssetParam& which) const {
		switch (which) {
		case SpatialAssetParam::constantParam:
			return constants->Num();
			break;
		case SpatialAssetParam::timeseriesParam:
			return timeseries->Num();
			break;
		case SpatialAssetParam::measurement:
			return measurements->Num();
			break;
		}
		return -1;
	};
	/*! Get a list of all parameter names in the object  */
	cweeThreadedList<cweeStr>	GetParameters() const {
		cweeThreadedList<cweeStr> out;
		
		Lock();
		out.SetGranularity(UnsafeGetParameters().size() + 16);
		for (auto& x : UnsafeGetParameters()) out.Append(x.first.c_str());
		Unlock();

		return out;
	};
	/*! Get a list of all parameter names in the object of the specified type */
	cweeThreadedList<cweeStr>	GetParameters(const SpatialAssetParam& which) const {
		switch (which) {
		case SpatialAssetParam::constantParam:
			return GetConstParameters();
			break;
		case SpatialAssetParam::timeseriesParam:
			return GetTimeSeriesParameters();
			break;
		case SpatialAssetParam::measurement:
			return GetMeasurementParameters();
			break;
		}
		return cweeThreadedList<cweeStr>();
	};
	/*! Get the type of the parameter key name */
	SpatialAssetParam			GetParameter(const cweeStr& name) {
		assetParameter out;
		_getParameter(name, out); 
		return out.first;
	};
	/*! Replace the parameter with the provided name with the input object data */
	void						Append(const cweeStr& name, const cweeStr& in) {
		GetConstParameter(name) = in;
	};
	/*! Append the parameter with the provided name with the input object data */
	void						Append(const cweeStr& name, const u64& time, const cweeStr& in) {
		auto x = GetTimeSeriesParameter(name);
		x->Lock();
		auto ptr = x->UnsafeRead();
		if (ptr) {
			ptr->AddUniqueValue(time, in);
		}
		x->Unlock();
	};
	/*! Append the parameter with the provided name with the input object data */
	void						Append(const cweeStr& name, const u64& time, float in) {
		GetMeasurement(name)->AddUniqueValue(time, in);
	};
	/*! Directly access the parameter with the provided name */
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>> GetConstParameter(const cweeStr& name) {
		assetParameter index;
		if (!_getParameter(name, index)) {
			_addParameter(cweeThreadedSpatialAsset::SpatialAssetParam::constantParam, name);
			_getParameter(name, index);
		}
		return GetConstParameter(index);
	};
	/*! Directly access the parameter with the provided name */
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>> GetTimeSeriesParameter(const cweeStr& name) {
		assetParameter index;
		if (!_getParameter(name, index)) {
			_addParameter(cweeThreadedSpatialAsset::SpatialAssetParam::timeseriesParam, name);
			_getParameter(name, index);
		}
		return GetTimeSeriesParameter(index);
	};
	/*! Directly access the parameter with the provided name */
	cweeUnorderedListReferenceObject<Pattern> GetMeasurement(const cweeStr& name) {
		assetParameter index;
		if (!_getParameter(name, index)) {
			_addParameter(cweeThreadedSpatialAsset::SpatialAssetParam::measurement, name);
			_getParameter(name, index);
		}
		return GetMeasurement(index);
	};
	/*! Save the object to a cweeStr */
	cweeStr						Serialize(int option = -1) {
		cweeStr delim = ":CUSTOM_in_DELIM:";
		cweeStr out; cweeStr temp; cweeStr temp2;

		if (1) {
			auto list = constants->GetList();
			list.Sort<int, TAG_LIST>();
			temp = ""; for (auto& x : list) {
				constants->PreventDeletion(x);
				constants->Lock();
				auto ptr = constants->UnsafeRead(x);
				constants->Unlock();
				if (ptr) {
					ptr->Lock();
					temp2 = cweeStr(x) + "," + *ptr->UnsafeRead(); // 1,STRING
					ptr->Unlock();
				}
				constants->AllowDeletion(x);

				// temp2 = cweeStr(x) + "," + (cweeStr)constants->Read(x); // 1,STRING
				temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:"); // 1,STRING:CUSTOM_in_DELIM_in:2,STRING:CUSTOM_in_DELIM_in:3,STRING
			}
			temp2 = "";
			out.AddToDelimiter(temp, delim); if (out.IsEmpty()) out = " ";
		}
		if (1) {
			auto list = timeseries->GetList();
			list.Sort<int, TAG_LIST>();
			temp = ""; for (auto& x : list) {
				timeseries->PreventDeletion(x);
				timeseries->Lock();
				auto ptr = timeseries->UnsafeRead(x);
				timeseries->Unlock();
				if (ptr) {
					ptr->Lock();
					temp2 = cweeStr(x) + "," + ptr->UnsafeRead()->Serialize(); // 1,STRING
					ptr->Unlock();
				}
				timeseries->AllowDeletion(x);

				// temp2 = cweeStr(x) + "," + timeseries->Read(x)->Serialize(); 
				temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:"); // 1,STRING:CUSTOM_in_DELIM_in:2,STRING:CUSTOM_in_DELIM_in:3,STRING
			}
			temp2 = "";
			out.AddToDelimiter(temp, delim);
		}
		if (1) {
			auto list = measurements->GetList();
			list.Sort<int, TAG_LIST>();
			temp = ""; for (auto& x : list) {
				measurements->PreventDeletion(x);
				measurements->Lock();
				auto ptr = measurements->UnsafeRead(x);
				measurements->Unlock();
				if (ptr) {
					temp2 = cweeStr(x) + "," + ptr->Serialize(); // 1,STRING
				}
				measurements->AllowDeletion(x);
				// temp2 = cweeStr(x) + "," + measurements->Read(x).Serialize(option); // 1,STRING
				temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:"); // 1,STRING:CUSTOM_in_DELIM_in:2,STRING:CUSTOM_in_DELIM_in:3,STRING
			}
			temp2 = "";
			out.AddToDelimiter(temp, delim);
		}

		temp = "";
		Lock();
		for (auto& x : UnsafeGetParameters()) {
			cweeStr temp2 = cweeStr(static_cast<int>(x.second.first)) + "," + cweeStr(x.second.second);
			temp.AddToDelimiter(temp2, ":CUSTOM_in_DELIM_in:");
		}
		Unlock();
		out.AddToDelimiter(temp, delim);

		temp = "";
		Lock();
		for (auto& x : UnsafeGetParameters()) {
			temp.AddToDelimiter(x.first.c_str(), ":CUSTOM_in_DELIM_in:");
		}
		Unlock();
		out.AddToDelimiter(temp, delim);

		temp = "";
		out.AddToDelimiter(GetName(), delim);

		return out;
	};
	/*! Load the object from a cweeStr */
	void						Deserialize(cweeStr& in) {
		if (in.IsEmpty()) {
			Clear();
			return;
		}

		cweeParser obj(in, ":CUSTOM_in_DELIM:", true);
		in.Clear(); cweeParser s, r;

		cweeThreadedList<cweeStr> names;
		{
			cweeStr& t = obj[4];
			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);

			Lock();
			names.SetGranularity((int)(s.getNumVars() + 16));
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;
				names.Append(x);
			}
			Unlock();
		}
		{
			cweeStr& t = obj[3];

			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);

			Lock();
			auto& c = UnsafeGetParameters();
			c.clear(); c.reserve((int)(s.getNumVars() + 16)); cweePair<SpatialAssetParam, int> q;
			int namesIndex = 0;
				
			for (auto& x : s)
			{
				if (x.IsEmpty() || names.Num() <= namesIndex) continue;
				r.Parse(x, ",", true);

				int newIndex = UnsafeGetNumParameters(static_cast<SpatialAssetParam>((int)r[0])) + 1;

				q = cweePair<SpatialAssetParam, int>(static_cast<SpatialAssetParam>((int)r[0]), newIndex);  //  (int)r[1]);
				c[names[namesIndex].c_str()] = q;
				namesIndex++;
			}
			Unlock();
		}

		{
			cweeStr& t = obj[0];

			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = constants;
			auto params = UnsafeGetParameters(SpatialAssetParam::constantParam);  int progress = 0;
			cweePair<int, cweeStr> q;
			for (auto& x : s)
			{
				if (x.IsEmpty() || x == " ") continue;
				r.ParseFirstDelimiterOnly(x, ",");
											
				if (params.Num() > progress) {
					c->Emplace(params[progress].second, r[1]);
				}
				progress++;
			}
		}
		{
			cweeStr& t = obj[1];

			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = timeseries;
			auto params = UnsafeGetParameters(SpatialAssetParam::timeseriesParam);  int progress = 0;
			cweePair<int, cweeCurve<cweeStr>> q; cweeCurve<cweeStr> o;
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;

				r.ParseFirstDelimiterOnly(x, ",");
				cweeStr& r2 = r[1];
				cweeStr& r1 = r[0];
				o.Deserialize(r[1]);

				if (params.Num() > progress) {
					c->Emplace(params[progress].second, o);
				}
				progress++;				
			}
		}
		{
			cweeStr& t = obj[2];

			s.Parse(t, ":CUSTOM_in_DELIM_in:", true);
			auto& c = measurements;
			auto params = UnsafeGetParameters(SpatialAssetParam::measurement);  int progress = 0;
			cweePair<int, Pattern> q; Pattern o;
			for (auto& x : s)
			{
				if (x.IsEmpty()) continue;

				r.ParseFirstDelimiterOnly(x, ",");
				o.Deserialize(r[1]);
			
				if (params.Num() > progress) {
					c->Emplace(params[progress].second, o);
				}
				progress++;
			}
			
		}

		SetName(obj[5]);

		prev_search.Clear();
	};

	bool						ParameterExists(const cweeStr& name) {
		if (GetParameter(name) == SpatialAssetParam::missingParam) return false;
		return true;
	};
	bool						RemoveParameter(const cweeStr& name) {
		return _removeParameter(GetParameter(name), name);		
	};

	cweeStr						GetName(void) const {
		cweeStr out;

		Lock();
		out = _name;
		Unlock();

		return out;
	};
	void						SetName(const cweeStr& newName) {
		Lock();
		_name = newName;
		Unlock();
	};





















































private: // PRIVATE FUNCTIONS
	assetParameter												_newParameter(const SpatialAssetParam& type) {
		assetParameter out;
		out.first = type;
		switch (type) {
		case SpatialAssetParam::constantParam:
			out.second = constants->Append();
			break;
		case SpatialAssetParam::timeseriesParam:
			out.second = timeseries->Append();
			break;
		case SpatialAssetParam::measurement:
			out.second = measurements->Append();
			break;
		}
		return out;
	};
	assetParameter												_addParameter(const SpatialAssetParam& type, const cweeStr& name) {
		Lock();
		auto found = UnsafeGetParameters().find(name.c_str());
		bool success = (found != UnsafeGetParameters().end());
		Unlock(); 

		if (!success) {
			// new object;
			auto newP = _newParameter(type); // first is type, second is unorderedMap index

			Lock();
			UnsafeGetParameters()[name.c_str()] = newP;
			Unlock();			
			
			return newP;
		}
		else {
			// existing object;
			assetParameter out;

			Lock();
			if (found != UnsafeGetParameters().end()) {
				out = found->second;
			}
			Unlock();

			return out;
		}
	};
	bool														_getParameter(const cweeStr& name, assetParameter& out) const {
		bool earlyExit = false;
		Lock();		
		if (name == UnsafeGetPrevSearch()) {
			earlyExit = true;
			out = UnsafeGetPrevIndex();
		}
		Unlock();
		if (earlyExit) return true;

		Lock();
		auto found = UnsafeGetParameters().find(name.c_str());
		bool success = (found != UnsafeGetParameters().end());
		Unlock();

		if (!success) {
			out.first = SpatialAssetParam::missingParam;
			return false;
		}
		else {
			Lock();
			if (found != UnsafeGetParameters().end()) {
				out = found->second;
				UnsafeGetPrevSearch() = name;
				UnsafeGetPrevIndex() = out;
			}		
			Unlock();

			return true;
		}
	}
	bool														_removeParameter(const SpatialAssetParam& type, const cweeStr& name) {
		bool success = false;
		Lock();
		
		auto found = UnsafeGetParameters().find(name.c_str());
		if (found != UnsafeGetParameters().end()) {
			UnsafeGetPrevSearch() = "";

			switch (found->second.first) {
			case SpatialAssetParam::constantParam:
				constants->Erase(found->second.second);
				break;
			case SpatialAssetParam::timeseriesParam:
				timeseries->Erase(found->second.second);
				break;
			case SpatialAssetParam::measurement:
				measurements->Erase(found->second.second);
				break;
			}

			success = true;
		}

		Unlock();

		return success;
	}
protected: // PROTECTED FUNCTIONS
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>>					GetConstParameter(const assetParameter& index) {
		cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>>				temp(constants, index.second);
		return temp;
	};
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>>		GetTimeSeriesParameter(const assetParameter& index) {
		cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>>	temp(timeseries, index.second);
		return temp;
	};
	cweeUnorderedListReferenceObject<Pattern>					GetMeasurement(const assetParameter& index) {
		cweeUnorderedListReferenceObject<Pattern>				temp(measurements, index.second);
		return temp;
	};

	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>>					GetConstParameter(int index) {
		cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeStr>>				temp(constants, index);
		return temp;
	};
	cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>>		GetTimeSeriesParameter(int index) {
		cweeUnorderedListReferenceObject<cweeUnpooledInterlocked<cweeCurve<cweeStr>>>	temp(timeseries, index);
		return temp;
	};
	cweeUnorderedListReferenceObject<Pattern>					GetMeasurement(int index) {
		cweeUnorderedListReferenceObject<Pattern>				temp(measurements, index);
		return temp;
	};

	cweeThreadedList<cweeStr>									GetConstParameters() const {
		cweeThreadedList<cweeStr> out(constants->Num() + 1);

		Lock();
		int n = UnsafeGetParameters().size();	
		for (auto& x : UnsafeGetParameters()) {
			if (x.second.first == SpatialAssetParam::constantParam) {
				out.Append(x.first.c_str());
			}
		}
		Unlock();

		return out;
	};
	cweeThreadedList<cweeStr>									GetTimeSeriesParameters() const {
		cweeThreadedList<cweeStr> out(timeseries->Num() + 1);

		Lock();
		int n = UnsafeGetParameters().size();
		for (auto& x : UnsafeGetParameters()) {
			if (x.second.first == SpatialAssetParam::timeseriesParam) {
				out.Append(x.first.c_str());
			}
		}
		Unlock();

		return out;
	};
	cweeThreadedList<cweeStr>									GetMeasurementParameters() const {
		cweeThreadedList<cweeStr> out(measurements->Num() + 1);

		Lock();
		int n = UnsafeGetParameters().size();
		for (auto& x : UnsafeGetParameters()) {
			if (x.second.first == SpatialAssetParam::measurement) {
				out.Append(x.first.c_str());
			}
		}
		Unlock();

		return out;
	};
	tsl::robin_map<std::string, assetParameter, robin_hood::hash<std::string>, std::equal_to<std::string>, std::allocator<std::pair<std::string, assetParameter>>, true>&							UnsafeGetParameters() {
		return parameters;
	};
	const tsl::robin_map<std::string, assetParameter, robin_hood::hash<std::string>, std::equal_to<std::string>, std::allocator<std::pair<std::string, assetParameter>>, true>&						UnsafeGetParameters() const {
		return parameters;
	};
	cweeThreadedList< assetParameter >							UnsafeGetParameters(const SpatialAssetParam& type) const {
		cweeThreadedList< assetParameter > out;
		out.SetGranularity(parameters.size() + 1);
		for (auto& x : parameters) {
			if (x.second.first == type) {
				out.Append(x.second);
			}
		}
		return out;
	};
	int															UnsafeGetNumParameters(const SpatialAssetParam& type) const {
		int out = 0;
		for (auto& x : parameters) {
			if (x.second.first == type) {
				out++;
			}
		}
		return out;
	};
	cweeStr&													UnsafeGetPrevSearch() const {
		return prev_search;
	};
	assetParameter&												UnsafeGetPrevIndex() const {
		return prev_index;
	};

	cweeSharedPtr < cweeUnorderedList< cweeUnpooledInterlocked<cweeStr> > >				constants = make_cwee_shared < cweeUnorderedList< cweeUnpooledInterlocked<cweeStr> > >(); // site name, site address, etc. 
	cweeSharedPtr < cweeUnorderedList< cweeUnpooledInterlocked<cweeCurve<cweeStr>> > >	timeseries = make_cwee_shared < cweeUnorderedList< cweeUnpooledInterlocked<cweeCurve<cweeStr>> > >(); // customer name, customer account number, etc. 
	cweeSharedPtr < cweeUnorderedList< Pattern > >				measurements = make_cwee_shared < cweeUnorderedList< Pattern > >(); // water usage, temperature, etc.

private: // PRIVATE DATA
	mutable cweeSysMutex										lock;
	tsl::robin_map<std::string, assetParameter, robin_hood::hash<std::string>, std::equal_to<std::string>, std::allocator<std::pair<std::string, assetParameter>>, true> parameters;
	// std::unordered_map<std::string, assetParameter>				parameters;

	mutable cweeStr												prev_search; // optimizaiton for searching the same key over and over
	mutable assetParameter										prev_index; // optimizaiton for searching the same key over and over

	void														Lock() const {  
		lock.Lock();
	};
	void														Unlock() const {
		lock.Unlock();
	};

public: // PUBLIC DATA
	cweeStr														_name = "No Name";
};






/*! Thread-Safe System for managing a cweeUnorderedList<cweeSpatialAsset> for use as custom object layers with real-world connections to SQL servers */
class cweeSpatialAssetCollection {
public:
	cweeSpatialAssetCollection() {};
	~cweeSpatialAssetCollection() { Clear(); };

	cweeUnorderedList<cweeSpatialAsset>& GetCollection() const {
		return list;
	};
	cweeThreadedList<int> GetIndexesWhere(const cweeStr& ConstOrTimeSeriesParameter, const cweeStr& EqualsOrContainsThisValue) const  {
		cweeThreadedList<int> out(GetCollection().Num() + 1);

		cweeSpatialAsset::SpatialAssetParam t = cweeSpatialAsset::SpatialAssetParam::missingParam;

		// is the requested parameter a const or TS or neither
		for (auto& x : object_parameters.Read()) {
			if (x.second == ConstOrTimeSeriesParameter) {
				t = x.first;
				break;
			}
		}

		if (t != cweeSpatialAsset::SpatialAssetParam::missingParam) {
			switch (t) {
			case cweeSpatialAsset::SpatialAssetParam::constantParam: 
				{
					cweeStr test;
					for (auto& i : GetCollection().GetList()) {
						GetCollection().Lock();
						auto ptr = GetCollection().UnsafeRead(i);
						if (ptr) test = ptr->GetConstParameter(ConstOrTimeSeriesParameter);						
						GetCollection().Unlock();
						if (test == EqualsOrContainsThisValue) out.Append(i);
					}
				}
				break;
			case cweeSpatialAsset::SpatialAssetParam::timeseriesParam: 
				{
					int test = -1;
					for (auto& i : GetCollection().GetList()) {
						GetCollection().Lock();
						auto ptr = GetCollection().UnsafeRead(i);
						if (ptr) test = ptr->GetTimeSeriesParameter(ConstOrTimeSeriesParameter).FindExactY(EqualsOrContainsThisValue);						
						GetCollection().Unlock();
						if (test >= 0) out.Append(i);
					}
				}
				break;
			}
		}
		return out;
	};
	cweeThreadedList<cweeStr> GetUniqueConstValues(const cweeStr& ConstParameter) const {
		cweeThreadedList<cweeStr> out(GetCollection().Num() + 1);
		cweeStr test;
		for (auto& i : GetCollection().GetList()) {
			GetCollection().Lock();
			auto ptr = GetCollection().UnsafeRead(i);
			if (ptr) test = ptr->GetConstParameter(ConstParameter);
			GetCollection().Unlock();
			out.AddUnique(test);		
		}
		return out;
	};
	cweeThreadedList<cweeStr> GetParameters(const cweeSpatialAsset::SpatialAssetParam& type = cweeSpatialAsset::SpatialAssetParam::missingParam) {
		cweeThreadedList<cweeStr> out;
		
		switch (type) {
		case cweeSpatialAsset::SpatialAssetParam::missingParam:
		{
			for (auto& x : object_parameters.Read()) {
				out.Append(x.second);
			}
		}
		break;
		default:
		{
			for (auto& x : object_parameters.Read()) {
				if (x.first == type)
					out.Append(x.second);
			}
		}
		break;
		}
		
		return out;
	};
	void Clear() {
		list.Clear();
		db.Clear();
		address.Clear();
		username.Clear();
		db_parameters.Clear();
		object_parameters.Clear();
		instructions.Clear();
		{
			con.Lock();
			odbc->EndConnection(*con.UnsafeRead());
			con.Unlock();
		}
		con.Clear();
		password.Clear();
		instructionOutputs.Clear();
	};

public:
	bool IsConnected() const {
		return odbc->IsConnected(con);
	};
	void Connect(const cweeStr& Server, const cweeStr& Username, const cweeStr& Password) {
		if (address.Read() == Server && username.Read() == Username && password.Read() == Password && IsConnected()) {
			// do nothing
		}
		else {
			address = Server;
			username = Username;
			password = Password;
			ConnectAsync();
		}
	};
	
	void AppendInstruction(int method, const cweeThreadedList<cweeStr>& Instructions) {
		cweePair<int, cweeThreadedList<cweeStr>> inst(method, Instructions);
		instructions.Lock();
		instructions.UnsafeRead()->Append(inst);
		instructions.Unlock();
	};
	void AppendDbParameter(const cweeSpatialAsset::SpatialAssetParam& type, const cweeStr& name) {
		db_parameters.Lock();
		db_parameters.UnsafeRead()->Append(cweePair<cweeSpatialAsset::SpatialAssetParam, cweeStr>(type, name));
		db_parameters.Unlock();
	};
	void AppendObjectParameter(const cweeSpatialAsset::SpatialAssetParam& type, const cweeStr& name) {
		object_parameters.Lock();
		object_parameters.UnsafeRead()->Append(cweePair<cweeSpatialAsset::SpatialAssetParam, cweeStr>(type, name));
		object_parameters.Unlock();
	};

	int  GetNumInstructions() const  {
		int i = 0;
		instructions.Lock();
		i = instructions.UnsafeRead()->Num();
		instructions.Unlock();
		return i;
	};
	auto GetInstructionMethod(int which) const {
		int i = 0;
		instructions.Lock();
		i = instructions.UnsafeRead()->operator[](which).first;
		instructions.Unlock();
		return i;
	};
	auto GetInstructionDetails(int which) const {
		cweeThreadedList<cweeStr> i;
		instructions.Lock();
		i = instructions.UnsafeRead()->operator[](which).second;
		instructions.Unlock();
		return i;
	};
	auto GetInstructionOutputSample(int which) const {
		cweeThreadedList<cweeStr> i;
		instructionOutputs.Lock();
		auto ptr = instructionOutputs.UnsafeRead();
		if (ptr->Num() > which) 
			i = ptr->operator[](which);
		instructionOutputs.Unlock();
		return i;
	};
	bool GetInstructionOutputSampleExists(int which) const {		
		bool i = false;
		instructionOutputs.Lock();
		auto ptr = instructionOutputs.UnsafeRead();
		if (ptr->Num() > which)
			i = true;
		instructionOutputs.Unlock();
		return i;
	};
	void SetInstruction(int which, int method, const cweeThreadedList<cweeStr>& Instructions) {
		instructions.Lock();
		instructions.UnsafeRead()->operator[](which).first = method;
		instructions.UnsafeRead()->operator[](which).second = Instructions;
		instructions.Unlock();
	};

	int  GetNumObjectParameters() const {
		int i = 0;
		object_parameters.Lock();
		i = object_parameters.UnsafeRead()->Num();
		object_parameters.Unlock();
		return i;
	};
	auto GetObjectParameterType(int which) const {
		cweeSpatialAsset::SpatialAssetParam i;
		object_parameters.Lock();
		i = object_parameters.UnsafeRead()->operator[](which).first;
		object_parameters.Unlock();
		return i;
	};
	auto GetObjectParameterName(int which) const {
		cweeStr i;
		object_parameters.Lock();
		i = object_parameters.UnsafeRead()->operator[](which).second;
		object_parameters.Unlock();
		return i;
	};
	void SetObjectParameter(int which, const cweeSpatialAsset::SpatialAssetParam& type, const cweeStr& name) {
		object_parameters.Lock();
		object_parameters.UnsafeRead()->operator[](which).first = type;
		object_parameters.UnsafeRead()->operator[](which).second = name;
		object_parameters.Unlock();
	};

	int  GetNumDbParameters() const {
		int i = 0;
		db_parameters.Lock();
		i = db_parameters.UnsafeRead()->Num();
		db_parameters.Unlock();
		return i;
	};
	auto GetDbParameterType(int which) const {
		cweeSpatialAsset::SpatialAssetParam i;
		db_parameters.Lock();
		i = db_parameters.UnsafeRead()->operator[](which).first;
		db_parameters.Unlock();
		return i;	
	};
	auto GetDbParameterName(int which) const {
		cweeStr i;
		db_parameters.Lock();
		i = db_parameters.UnsafeRead()->operator[](which).second;
		db_parameters.Unlock();
		return i;
	};
	void SetDbParameter(int which, const cweeSpatialAsset::SpatialAssetParam& type, const cweeStr& name) {
		db_parameters.Lock();
		db_parameters.UnsafeRead()->operator[](which).first = type;
		db_parameters.UnsafeRead()->operator[](which).second = name;
		db_parameters.Unlock();
	};

	auto GetServer() const {
		return address.Read(); 
	};
	auto GetUsername() const {
		return username.Read(); 
	};

public:
	struct AsyncInstructionPackage {
		cweeSpatialAssetCollection* srce = nullptr;
		cweePair<int, cweeThreadedList<cweeStr>> instruction;
		cweeStr saveLoadParam;
	};
	static void RunInstructionAsync_Actual(AsyncInstructionPackage* io) {
		if (io->srce) {
			io->srce->RunInstruction(io->instruction);
		}
		if (io) delete io;
	};
	static void ConnectAsync_Actual(AsyncInstructionPackage* io) {
		if (io->srce) {
			io->srce->Connect();
		}
		if (io) delete io;
	};
	static void SaveAsync_Actual(AsyncInstructionPackage* io) {
		if (io->srce) {
			io->srce->Save();
		}
		if (io) delete io;
	}; 
	static void LoadAsync_Actual(AsyncInstructionPackage* io) {
		if (io->srce) {
			io->srce->Load(io->saveLoadParam);
		}
		if (io) delete io;
	};

public:	
	void RunUpdate() {
		for (auto& instruction : instructions.Read()) {
			RunInstruction(instruction);
		}
	};
	void RunUpdateAsync() {
		for (auto& instruction : instructions.Read()) {
			AsyncInstructionPackage* io = new AsyncInstructionPackage; {
				io->srce = this;
				io->instruction = instruction;
			}
			cweeMultithreading::ADD_JOB(cweeSpatialAssetCollection::RunInstructionAsync_Actual, io, jobType::IO_thread);
		}
	};

	void Connect() {
		con = odbc->CreateConnection(address, username, password);
	};
	void ConnectAsync() {
		AsyncInstructionPackage* io = new AsyncInstructionPackage; {
			io->srce = this;
		}
		cweeMultithreading::ADD_JOB(cweeSpatialAssetCollection::ConnectAsync_Actual, io, jobType::IO_thread);
	};

	cweeStr Serialize() const {
		cweeStr out;
		cweeStr delim = "::cweeSpatialAssetCollection_delim::";

		out.AddToDelimiter(db->Serialize(), delim);
		out.AddToDelimiter(list.Serialize("::cweeSpatialAssetCollection_list_delim::"), delim);		
		out.AddToDelimiter(address.Read(), delim);
		out.AddToDelimiter(username.Read(), delim);
		cweeStr S_db_parameters; for (auto& x : db_parameters.Read()) {
			S_db_parameters.AddToDelimiter(cweeStr::printf("%i,%s", enumClassToInt(x.first), x.second.c_str()),"::S_db_parameters::");
		}
		out.AddToDelimiter(S_db_parameters, delim);
		cweeStr S_object_parameters; for (auto& x : object_parameters.Read()) {
			S_object_parameters.AddToDelimiter(cweeStr::printf("%i,%s", enumClassToInt(x.first), x.second.c_str()), "::S_object_parameters::");
		}
		out.AddToDelimiter(S_object_parameters, delim);
		cweeStr S; for (auto& x : instructions.Read()) {
			cweeStr R; for (auto& y : x.second) {
				R.AddToDelimiter(y, "::S_R_Delim::");
			}
			S.AddToDelimiter(cweeStr::printf("%i,%s", (int)x.first, R.c_str()), "::S_instructions::");
		}
		out.AddToDelimiter(S, delim);

		return out;
	}
	void Deserialize(const cweeStr& in) {
		cweeStr delim = "::cweeSpatialAssetCollection_delim::";

		cweeParser a(in, delim, true);
		
		db.Lock();
		db.UnsafeRead()->Deserialize(a[0]);
		db.Unlock();
		list.Deserialize(a[1], "::cweeSpatialAssetCollection_list_delim::");
		
		address = a[2];
		username = a[3];

		db_parameters.Clear();
		cweeParser t(a[4], "::S_db_parameters::", true); for (auto& x : t) {
			cweeParser b; b.ParseFirstDelimiterOnly(x, ",");
			cweeSpatialAsset::SpatialAssetParam o = static_cast<cweeSpatialAsset::SpatialAssetParam>((int)b[0]);
			db_parameters.Lock();
			db_parameters.UnsafeRead()->Append(cweePair<cweeSpatialAsset::SpatialAssetParam, cweeStr>(o, b[1]));
			db_parameters.Unlock();
		}

		object_parameters.Clear();
		cweeParser q(a[5], "::S_object_parameters::", true); for (auto& x : q) {
			cweeParser b; b.ParseFirstDelimiterOnly(x, ",");
			cweeSpatialAsset::SpatialAssetParam o = static_cast<cweeSpatialAsset::SpatialAssetParam>((int)b[0]);
			object_parameters.Lock();
			object_parameters.UnsafeRead()->Append(cweePair<cweeSpatialAsset::SpatialAssetParam, cweeStr>(o, b[1]));
			object_parameters.Unlock();
		}

		instructions.Clear();
		cweeParser s(a[6], "::S_instructions::", true); for (auto& x : s) {
			cweeParser b; b.ParseFirstDelimiterOnly(x, ",");

			cweeThreadedList<cweeStr> c;
			cweeParser d(b[1], "::S_R_Delim::", true);
			for (auto& y : d) c.Append(y);

			instructions.Lock();
			instructions.UnsafeRead()->Append(cweePair<int, cweeThreadedList<cweeStr>>((int)b[0], c));
			instructions.Unlock();
		}

		instructionOutputs.Clear(); // will need to be re-set once the system runs again.
		// con wan't release, nor was password. 
	}

	void Save() const {
		cweeStr serialized = Serialize();
		{
			cweeStr filePath = fileSystem->createFilePath(fileSystem->getDataFolder(), address, fileType_t::sqlDB);
			
			// Always start fresh
			if (fileSystem->checkFileExists(filePath)) {
				fileSystem->removeFile(filePath.c_str());
			}

			// Remake the file
			auto thisCon = odbc->CreateConnection(filePath, "", "");
			odbc->GetResults(thisCon, "DROP TABLE IF EXISTS Save;");
			odbc->GetResults(thisCon, "CREATE TABLE Save ( serialized TEXT PRIMARY KEY );");
			//odbc->GetResults(thisCon, "BEGIN TRANSACTION;");
			// Add the content to the file
#if 0
			cweeStr saveContent = "INSERT INTO Save (serialized) VALUES (\"" + serialized + "\");";
			odbc->GetResults(thisCon, saveContent);

#else
			u64 n = serialized.Length();
			u64 step = cweeMath::min(10000000,cweeMath::Rint((n / 100.0f))); // break incoming into 100 or more rows
			u64 i = 0;
			cweeStr row;
			for (; i < serialized.Length(); i += step) {
				cweeStr saveContent = "INSERT INTO Save (serialized) VALUES (\"" + serialized.Mid(i, step) + "\");";
				odbc->GetResults(thisCon, saveContent);
			}
#endif

			//odbc->GetResults(thisCon, "END TRANSACTION;");

			thisCon.disconnect();
		}
	};
	bool Load(const cweeStr& Address) {
		cweeStr filePath = fileSystem->createFilePath(fileSystem->getDataFolder(), Address, fileType_t::sqlDB);
		if (fileSystem->checkFileExists(filePath)) {

			cweeStr serialized;
			{
				auto thisCon = odbc->CreateConnection(filePath, "", "");
				auto res = odbc->Query(thisCon, "SELECT * FROM Save;");			
				for (auto& x : odbc->FirstColumnOnly(odbc->GetResults(res))) {
					serialized.Append(x);
				}					
				thisCon.disconnect();
			}
			Deserialize(serialized);
			return true;
		}
		return false;
	};
	void SaveAsync() {
		AsyncInstructionPackage* io = new AsyncInstructionPackage; {
			io->srce = this;
		}
		cweeMultithreading::ADD_JOB(cweeSpatialAssetCollection::SaveAsync_Actual, io);
	};
	void LoadAsync(const cweeStr& Address) {
		AsyncInstructionPackage* io = new AsyncInstructionPackage; {
			io->srce = this;
			io->saveLoadParam = Address;
		}
		cweeMultithreading::ADD_JOB(cweeSpatialAssetCollection::LoadAsync_Actual, io);
	};

protected:	
	void RunInstruction(const cweePair<int, cweeThreadedList<cweeStr>>& instruction) {
		if (!IsConnected()) Connect();
		if (!IsConnected()) return;

		auto Instructions = instructions.Read();

		int instructionSlot = 0;

		for (; instructionSlot < Instructions.Num(); instructionSlot++)
			if (Instructions[instructionSlot] == instruction)
				break;

		cweeThreadedList<cweeStr> sample;
		// perform job
		{
			constexpr int div = 6;
			cweeSpatialAsset temp = db.Read();
			switch (instruction.first) {
			case 0:
				sample = cweeSpatialAsset::Instantiate(con, list, instruction.second[0], instruction.second[1], object_parameters.Read()); // set up the initial list and parameters													
				break;
			case 1:
				sample = cweeSpatialAsset::ExecuteQueryAcrossList_Method1(con, list, instruction.second[0], instruction.second[1], instruction.second[2], instruction.second[3], instruction.second[4], cweeMath::max(100, list.Num() / div));
				break;
			case 2:
				sample = cweeSpatialAsset::ExecuteQueryAcrossList_Method2(con, list, instruction.second[0], instruction.second[1], instruction.second[2], instruction.second[3], instruction.second[4], instruction.second[5], cweeMath::max(100, list.Num() / div));
				break;
			case 3:
				sample = cweeSpatialAsset::ExecuteQueryAcrossList_Method3(con, list, instruction.second[0], instruction.second[1], instruction.second[2], instruction.second[3], instruction.second[4], cweeMath::max(100, list.Num() / div));
				break;
			case 4:
				// cweeSpatialAsset::ExecuteQueryAcrossList_Method4(...)
				break;
			case 5:
				sample = cweeSpatialAsset::ExecuteQueryAcrossList_Method5(con, list, instruction.second[0], instruction.second[1], instruction.second[2], instruction.second[3], instruction.second[4], instruction.second[5], cweeMath::max(100, list.Num() / div));
				break;
			case 6:
				// cweeSpatialAsset::ExecuteQueryAcrossList_Method6(...)
				break;
			case 7:
				sample = cweeSpatialAsset::ExecuteQueryAcrossList_Method7(con, list,
					instruction.second[0], instruction.second[1], instruction.second[2],
					temp, instruction.second[3], instruction.second[4], instruction.second[5]
				);
				break;
			case 8:
				// cweeSpatialAsset::ExecuteQuery_Method1(...)
				break;
			case 9:
				if (instruction.second.Num() == 6)
					sample = cweeSpatialAsset::ExecuteQuery_Method2(con, temp, instruction.second[0], instruction.second[1], instruction.second[2], instruction.second[3], instruction.second[4], instruction.second[5]);
				else
					sample = cweeSpatialAsset::ExecuteQuery_Method2(con, temp, instruction.second[0], instruction.second[1], instruction.second[2], instruction.second[3], instruction.second[4]);
				break;
			case 10:
				// cweeSpatialAsset::ExecuteQuery_Method3(...)
				break;
			case 11:
				// cweeSpatialAsset::ExecuteQuery_Method4(...)
				break;
			case 12:
				// cweeSpatialAsset::ExecuteQuery_Method5(...)
				break;
			case 13:
				cweeSpatialAsset::SetDefaultMeasurementUnits(list, instruction.second[0], instruction.second[1].IsEmpty() ? cweeStr("Unique ID") : instruction.second[1]);
				sample.Append("Successful");				
				break;
			}
			db = temp;
		}

		// record a recent Query sample for viewing and comparison later
		{
			instructionOutputs.Lock();
			auto ptr = instructionOutputs.UnsafeRead();
			ptr->Resize(cweeMath::max(Instructions.Num(), ptr->Num()));
			ptr->Insert(sample, instructionSlot);
			instructionOutputs.Unlock();
		}
	};
	
private:
	// All Data is Thread-Safe, thought the const characteristic of it has been pierced.
	mutable cweeUnorderedList < cweeSpatialAsset  > list;		
	mutable cweeUnpooledInterlocked < cweeSpatialAsset > db;
	mutable cweeUnpooledInterlocked < cweeStr > address;
	mutable cweeUnpooledInterlocked < cweeStr > username;	
	mutable cweeUnpooledInterlocked < cweeThreadedList<cweePair<cweeSpatialAsset::SpatialAssetParam, cweeStr>> > db_parameters;
	mutable cweeUnpooledInterlocked < cweeThreadedList<cweePair<cweeSpatialAsset::SpatialAssetParam, cweeStr>> > object_parameters;
	mutable cweeUnpooledInterlocked < cweeThreadedList<cweePair<int, cweeThreadedList<cweeStr>>> > instructions;
	
	mutable cweeUnpooledInterlocked < nanodbc::connection > con;
	mutable cweeUnpooledInterlocked < cweeStr > password;
	mutable cweeUnpooledInterlocked < cweeThreadedList<cweeThreadedList<cweeStr>> > instructionOutputs;
};

class WaterSystemData {
public:
	cweeSpatialAssetCollection scada;
	cweeSpatialAssetCollection service;
	void Save() const {
		scada.Save();
		service.Save();
	};
	void SaveAsync() {
		scada.SaveAsync();
		service.SaveAsync();
	};
	void Load(const cweeStr& scadaServer, const cweeStr& serviceServer) {
		scada.Load(scadaServer);
		service.Load(serviceServer);
	};
	void LoadAsync(const cweeStr& scadaServer, const cweeStr& serviceServer) {
		scada.LoadAsync(scadaServer);
		service.LoadAsync(serviceServer);
	};

public:
	cweeStr Serialize() {
		cweeStr out;
		cweeStr delim = "::WaterSystemData_delim::";

		out = scada.GetServer() + delim + service.GetServer();

		Save();

		return out;
	};
	void Deserialize(const cweeStr& in) {
		cweeStr delim = "::WaterSystemData_delim::";

		cweeParser a(in, delim, true);

		cweeStr scadaServer = a[0]; cweeStr serviceServer = a[1];

		Load(scadaServer, serviceServer);
	};

};

#endif