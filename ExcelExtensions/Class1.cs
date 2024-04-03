using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ExcelDna.Integration;
using ExcelDna.IntelliSense;
using ExcelDna.Registration.Utils;
using ExcelDna.Registration;

public class IntelliSenseAddIn : IExcelAddIn {
    public void AutoOpen() {
        IntelliSenseServer.Install();
    }
    public void AutoClose() {
        IntelliSenseServer.Uninstall();
    }
}

namespace ExcelExtensions {
    public static class ExtensionFunctions {
        private static string CallingCellReference()
        {
            ExcelReference reference = (ExcelReference)XlCall.Excel(XlCall.xlfCaller);
            string cellReference =
            (string)XlCall.Excel(XlCall.xlfAddress, 1 + reference.RowFirst,
            1 + reference.ColumnFirst);

            string sheetName = (string)XlCall.Excel(XlCall.xlSheetNm,
            reference);
            cellReference = sheetName + "!" + cellReference;

            return cellReference;
        }
        private static Dictionary<string, object> cache = new Dictionary<string, object>();

        [ExcelFunction(Description = "Analyzes a range and caches the list of unique items in that range. Returns the ID for that cache.")]
        public static string UNIQUE(
            [ExcelArgument(Name = "Range", Description = "is the range of items to be searched.")]
            object[,] values
        )
        {
            string cellAddress = CallingCellReference();
            try
            {
                cache.Remove(cellAddress);
            }
            catch (Exception) { }

            Dictionary<Type, int> typeCount = new Dictionary<Type, int>();
            // pre-analysis
            int rows = values.GetLength(0);
            int cols = values.GetLength(1);
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++)
                    if (values[i, j] != null && !(values[i, j] is ExcelDna.Integration.ExcelEmpty))
                        typeCount[values[i, j].GetType()] = 0;

            if (typeCount.Count <= 0) return "";
            if (typeCount.Count >= 2) throw new Exception("Do not mix types when calling this function");

            Dictionary<Type, string> types = new Dictionary<Type, string>();
            types[("").GetType()] = "String";
            types[(0.0).GetType()] = "Double";
            types[(0.0f).GetType()] = "Float";
            types[(0).GetType()] = "Int";
            types[(new DateTime()).GetType()] = "DateTime";

            if (types[typeCount.Keys.First()] == "String")
            {
                Dictionary<string, int> objs = new Dictionary<string, int>();
                for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++) if (values[i, j] != null && !(values[i, j] is ExcelDna.Integration.ExcelEmpty)) objs[values[i, j] as string] = 0;
                cache[cellAddress] = objs.Keys;
            }
            else if (types[typeCount.Keys.First()] == "Double")
            {
                Dictionary<double, int> objs = new Dictionary<double, int>();
                for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++) if (values[i, j] != null && !(values[i, j] is ExcelDna.Integration.ExcelEmpty)) objs[(double)values[i, j]] = 0;
                cache[cellAddress] = objs.Keys;
            }
            else if (types[typeCount.Keys.First()] == "Float")
            {
                Dictionary<float, int> objs = new Dictionary<float, int>();
                for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++) if (values[i, j] != null && !(values[i, j] is ExcelDna.Integration.ExcelEmpty)) objs[(float)values[i, j]] = 0;
                cache[cellAddress] = objs.Keys;
            }
            else if (types[typeCount.Keys.First()] == "Int")
            {
                Dictionary<int, int> objs = new Dictionary<int, int>();
                for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++) if (values[i, j] != null && !(values[i, j] is ExcelDna.Integration.ExcelEmpty)) objs[(int)values[i, j]] = 0;
                cache[cellAddress] = objs.Keys;
            }
            else if (types[typeCount.Keys.First()] == "DateTime")
            {
                Dictionary<DateTime, int> objs = new Dictionary<DateTime, int>();
                for (int i = 0; i < rows; i++) for (int j = 0; j < cols; j++) if (values[i, j] != null && !(values[i, j] is ExcelDna.Integration.ExcelEmpty)) objs[(DateTime)values[i, j]] = 0;
                cache[cellAddress] = objs.Keys;
            }
            else
            {
                throw new Exception("Type was not handled: " + typeCount.Keys.First().ToString());
            }

            return cellAddress;
        }

        [ExcelFunction(Description = "Retrieves the i'th item from a cached unique item list.")]
        public static object GETUNIQUE(
            [ExcelArgument(Name = "cacheID", Description = "is the ID for which cache has the unique item(s) list.")]
            string cacheID,
            [ExcelArgument(Name = "index", Description = "is which (i'th) item to retrieve from the unique item(s) list.")]
            int index
        )
        {
            if (cache.ContainsKey(cacheID))
            {
                var obj = cache[cacheID];

                if (obj is ICollection<string>)
                {
                    var list = obj as ICollection<string>;
                    if (list != null) return list.ElementAt(index);
                }
                else if (obj is ICollection<double>)
                {
                    var list = obj as ICollection<double>;
                    if (list != null) return list.ElementAt(index);
                }
                else if (obj is ICollection<float>)
                {
                    var list = obj as ICollection<float>;
                    if (list != null) return list.ElementAt(index);
                }
                else if (obj is ICollection<int>)
                {
                    var list = obj as ICollection<int>;
                    if (list != null) return list.ElementAt(index);
                }
                else if (obj is ICollection<DateTime>)
                {
                    var list = obj as ICollection<DateTime>;
                    if (list != null) return list.ElementAt(index);
                }
                else
                {
                    throw new Exception("Type was not handled: " + obj.GetType().ToString());
                }
            }
            return null;
        }

        [ExcelFunction(Description = "Determines if the cell is empty or contains a blank string")]
        public static bool ISEMPTY(
            [ExcelArgument(Name = "value", Description = "is the cell (content) to check")]
            string InputString
        )
        {
            return string.IsNullOrEmpty(InputString);
        }

        [ExcelFunction(Description = "Analyzes a range and caches the list of unique items in that range. Returns the ID for that cache.")]
        public static string MAKEPATTERN(
            [ExcelArgument(Name = "xRange", Description = "is the datetime range of items to be searched.")]
            object[,] xvalues_in,
            [ExcelArgument(Name = "yRange", Description = "is the double range of items to be searched.")]
            object[,] yvalues_in
        )
        {
            string cellAddress = CallingCellReference();
            try { cache.Remove(cellAddress); } catch (Exception) { }

            // y-values or x-values are allowed to be empty. One being empty cancels the other. 
            int numX = xvalues_in.GetLength(0) * xvalues_in.GetLength(1);
            int numY = yvalues_in.GetLength(0) * yvalues_in.GetLength(1);

            object[] xvalues = new object[numX];
            object[] yvalues = new object[numY];

            int index = 0;
            for (int i = 0; i < xvalues_in.GetLength(0); i++) for (int j = 0; j < xvalues_in.GetLength(1); j++) xvalues[index++] = xvalues_in[i, j];
            index = 0;
            for (int i = 0; i < yvalues_in.GetLength(0); i++) for (int j = 0; j < yvalues_in.GetLength(1); j++) yvalues[index++] = yvalues_in[i, j];

            Dictionary<double, double> values = new Dictionary<double, double>();
            for (int i = 0; i < numX && i < numY; i++) {
                var x_val = xvalues[i];
                var y_val = yvalues[i];

                if (x_val != null && y_val != null)
                {
                    if (x_val is ExcelDna.Integration.ExcelEmpty || y_val is ExcelDna.Integration.ExcelEmpty) continue;

                    if (x_val is DateTime || x_val is double || x_val is float || x_val is int)
                    {
                        double X;
                        if (x_val is DateTime) X = ((DateTime)x_val - new DateTime(1970, 1, 1)).TotalSeconds;
                        else if (x_val is double) X = (double)x_val;
                        else if (x_val is float) X = (float)x_val;
                        else if (x_val is int) X = (int)x_val;
                        else continue;

                        if (y_val is double || y_val is float || y_val is int)
                        {
                            double Y;
                            if (y_val is double) Y = (double)y_val;
                            else if (y_val is float) Y = (float)y_val;
                            else if (y_val is int) Y = (int)y_val;
                            else continue;

                            values[X] = Y;
                        }
                    }
                }
            }


            CppContainers.cweePattern pat = new CppContainers.cweePattern();
            foreach (var x in values) {
                pat.AddValue(x.Key, x.Value);
            }

            cache[cellAddress] = pat;

            return cellAddress;
        }

        [ExcelFunction(Description = "Appends a value pair with a cached timeseries. Returns the ID for that cache.")]
        public static string APPENDPATTERN(
            [ExcelArgument(Name = "cacheID", Description = "is the cacheID used to access this pattern")]
            string cacheID,
            [ExcelArgument(Name = "xValue", Description = "is the datetime")]
            object x_val,
            [ExcelArgument(Name = "yValue", Description = "is the value")]
            object y_val
        )
        {
            CppContainers.cweePattern pat = null;
            try
            {
                if (cache.ContainsKey(cacheID))
                {
                    pat = cache[cacheID] as CppContainers.cweePattern;
                }
                else {
                    pat = new CppContainers.cweePattern();
                    cache[cacheID] = pat;
                }
            } catch (Exception) { }
            if (pat != null)
            {
                if (x_val != null && y_val != null)
                {
                    if (x_val is ExcelDna.Integration.ExcelEmpty || y_val is ExcelDna.Integration.ExcelEmpty) return cacheID;

                    if (x_val is DateTime || x_val is double || x_val is float || x_val is int)
                    {
                        double X;
                        if (x_val is DateTime) X = ((DateTime)x_val - new DateTime(1970, 1, 1)).TotalSeconds;
                        else if (x_val is double) X = (double)x_val;
                        else if (x_val is float) X = (float)x_val;
                        else if (x_val is int) X = (int)x_val;
                        else return cacheID;

                        if (y_val is double || y_val is float || y_val is int)
                        {
                            double Y;
                            if (y_val is double) Y = (double)y_val;
                            else if (y_val is float) Y = (float)y_val;
                            else if (y_val is int) Y = (int)y_val;
                            else return cacheID;

                            pat.AddUniqueValue(X, Y);
                        }
                    }
                }
            }
            return cacheID;
        }


        [ExcelFunction(Description = "Samples the item from the specified Pattern. Will be interpolated if not found directly.")]
        public static double SAMPLEPATTERN(
            [ExcelArgument(Name = "cacheID", Description = "is the ID for which cache has the timeseries pattern.")]
            string cacheID,
            [ExcelArgument(Name = "index", Description = "is which time/value to sample at. May be a number, datetime, etc.")]
            object index
        )
        {
            if (cache.ContainsKey(cacheID))
            {
                var obj = cache[cacheID];

                if (obj is CppContainers.cweePattern)
                {
                    var list = obj as CppContainers.cweePattern;
                    if (index is DateTime)
                    {
                        return list.GetCurrentValue(((DateTime)index - new DateTime(1970, 1, 1)).TotalSeconds);
                    }
                    else if (index is double)
                    {
                        return list.GetCurrentValue((double)index);
                    }
                    else if (index is float)
                    {
                        return list.GetCurrentValue((float)index);
                    }
                    else if (index is int)
                    {
                        return list.GetCurrentValue((int)index);
                    }
                    else {
                        throw new Exception("Type was not handled: " + index.GetType().ToString());
                    }
                }
            }
            return 0;
        }

        [ExcelFunction(Description = "Samples the minimum provided time from the specified Pattern.")]
        public static double PATTERN_MINTIME(
            [ExcelArgument(Name = "cacheID", Description = "is the ID for which cache has the timeseries pattern.")]
            string cacheID
        )
        {
            if (cache.ContainsKey(cacheID))
            {
                var obj = cache[cacheID];

                if (obj is CppContainers.cweePattern)
                {
                    return (obj as CppContainers.cweePattern).GetMinTime();
                }
            }
            return 0;
        }

        [ExcelFunction(Description = "Samples the maximum provided time from the specified Pattern.")]
        public static double PATTERN_MAXTIME(
            [ExcelArgument(Name = "cacheID", Description = "is the ID for which cache has the timeseries pattern.")]
            string cacheID
        )
        {
            if (cache.ContainsKey(cacheID))
            {
                var obj = cache[cacheID];

                if (obj is CppContainers.cweePattern)
                {
                    return (obj as CppContainers.cweePattern).GetMaxTime();
                }
            }
            return 0;
        }

    }
}

namespace CppContainers {
    public class cweePattern {
		private List<double> times = new List<double>();
		private List<double> values = new List<double>();
		public cweePattern() { }

		private int UnsafeIndexForTime(double time)
		{
			if (times.Count <= 0) return 0; // there is no other data...

			int len, mid, offset;
			bool res;

			// use binary search to find the index for the given time
			len = times.Count;
			mid = len;
			offset = 0;
			res = false;
			double sample;
			while (mid > 0)
			{
				mid = len >> 1;
				// OPTIMIZED ORDERING
				sample = times[offset + mid];
				if (time >= sample)
				{
					offset += mid;
					len -= mid;
					res = true;
					if (time == sample)
					{
						return offset;
					}
				}
				else
				{
					len -= mid;
					res = false;
				}
			}
			return offset + (res ? 1 : 0);
		}
        private int InsertPair(double time, double valueIN, bool unique = false)
		{
			int i = 0;
			if (unique)
			{
				double value = valueIN;

				{
					i = UnsafeIndexForTime(time);
					if (((i != 0) && (i < values.Count) && (times[i] == time)))
					{
						values[i] = value;
						return i;
					}
					double minTime;
					{
						if (times.Count == 0) minTime = 0;
						else minTime = times[0];
					}
					if (values.Count == 0 || minTime != time)
					{
						times.Insert(i, time);
						values.Insert(i, value);
					}
					else
					{
						values[0] = value;
						i = 0;
					}
				}
				return i;
			}
			else
			{
				double value = valueIN;

				{
					i = UnsafeIndexForTime(time);
					(times).Insert(i, time);
					(values).Insert(i, value);
				}

				return i;
			}
		}

        public int GetNumValues() {
			return values.Count;
		}
        public double GetMaxTime() {
			if (GetNumValues() == 0)
				return 0;
			else
				return TimeForIndex(GetNumValues() - 1);
		}
        public double GetMinTime() {
			if (GetNumValues() == 0)
				return 0;
			else
				return TimeForIndex(0);
		}
		public double GetNearestValue(double time) {
			int i = IndexForTime(time);
			if (i >= GetNumValues()) {
				return ValueForIndex(GetNumValues() - 1);
			}
			else {
				return ValueForIndex(i);
			}
		}

        public double TimeForIndex(int index) {
			double toReturn = 0;

			int n = times.Count;
			if (index >= 0 && index<n)
				toReturn = times[index];
			else if (index< 0 && n> 1)
				toReturn = times[0];
			else if (index >= n && n > 0)
				toReturn = times[n - 1];

			return toReturn;
		}
        public double ValueForIndex(int index) {
			double toReturn = 0;

			int n = values.Count;
			if (index >= 0 && index < n)
				toReturn = values[index];
			else if (index < 0 && n > 1)
				toReturn = values[0];
			else if (index >= n && n > 0)
				toReturn = values[n - 1];

			return toReturn;
		}
        public int IndexForTime(double time) {
			return UnsafeIndexForTime(time);
		}
        public int AddValue(double time, double valueIN) {
			return InsertPair(time, valueIN, false);
		}
        public int AddUniqueValue(double time, double valueIN) {
			return InsertPair(time, valueIN, true);
		}
        public void Clear() {
			values.Clear();
			times.Clear();
		}
        
        public double ClampedTime(double t) {
            double mT = GetMinTime();
            if (t <= mT)
            {
                return mT;
            }
            else
            {
                mT = GetMaxTime();
                if (t > mT)
                    return mT;
            }
            return t;
        }
        public double LoopedTime(double t) {
            return t;
	    }
        public void Basis(int index, double t, ref double[] bvals) {
            //const float x = cweeMath::min(cweeMath::max((float)((t - this->TimeForIndex(index)) / (this->TimeForIndex(index + 1) - this->TimeForIndex(index))), 0), 1);
            //const float s = 0; // where s=0 means linear, s=1 means fully smooth, s=0.5 = traditional catmull-rom. 
            //const float sx = s * x;
            //const float sx2 = sx * x;
            //const float sx3 = sx2 * x;
            //const float x2 = x * x;
            //const float x3 = x2 * x;

            //bvals[0] = 2.0f*sx2-sx-sx3;
            //bvals[1] = 1.0f-3.0f*x2+sx2+2.0f*x3-sx3;
            //bvals[2] = sx+3.0f*x2-2.0f*sx2-2.0f*x3+sx3;
            //bvals[3] = sx3 - sx2;

            double t0, t1, s;
            t0 = TimeForIndex(index);
            t1 = TimeForIndex(index + 1);           

            if (t1 <= t0) {
                bvals[0] = 0; 
                bvals[1] = 1.0f; 
                bvals[2] = 0; 
                bvals[3] = 0; 
            }
            else {
                s = (t - t0) / (t1 - t0);

                bvals[0] = ((2.0f - s) * s - 1.0f) * s * 0.5f;              // -0.5f s * s * s + s * s - 0.5f * s
                bvals[1] = (((3.0f * s - 5.0f) * s) * s + 2.0f) * 0.5f;     // 1.5f * s * s * s - 2.5f * s * s + 1.0f
                bvals[2] = ((-3.0f * s + 4.0f) * s + 1.0f) * s * 0.5f;      // -1.5f * s * s * s - 2.0f * s * s + 0.5f s
                bvals[3] = ((s - 1.0f) * s * s) * 0.5f;                     // 0.5f * s * s * s - 0.5f * s * s
            }

		}
        public double GetCurrentValue(double time) {

	        int i = 0, j = 0, k = 0;
            double[] bvals = new double[4];
	        double clampedTime;
	        double v;

	        j = GetNumValues();

	        if (j < 1) {
		        v = 0;
		        return v;
	        }
	        if (j == 1) {
		        return ValueForIndex(0);
	        }

	        clampedTime = ClampedTime(time);
	        clampedTime = LoopedTime(clampedTime);


	        i = IndexForTime(clampedTime);
	        Basis(i - 1, clampedTime, ref bvals);
	        v = 0;
	        for (j = 0; j < 4; j++) {
		        k = i + j - 2;
		        v += (ValueForIndex(k) * (float)bvals[j]);
	        }
	
	        return v;

        }
	}
}



