//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (https://www.swig.org).
// Version 4.1.1
//
// Do not make changes to this file unless you know what you are doing - modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------


public class vector_pair_timeseries : global::System.IDisposable, global::System.Collections.IEnumerable, global::System.Collections.Generic.IEnumerable<pair_timeseries>
 {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal vector_pair_timeseries(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(vector_pair_timeseries obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  internal static global::System.Runtime.InteropServices.HandleRef swigRelease(vector_pair_timeseries obj) {
    if (obj != null) {
      if (!obj.swigCMemOwn)
        throw new global::System.ApplicationException("Cannot release ownership as memory is not owned");
      global::System.Runtime.InteropServices.HandleRef ptr = obj.swigCPtr;
      obj.swigCMemOwn = false;
      obj.Dispose();
      return ptr;
    } else {
      return new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
    }
  }

  ~vector_pair_timeseries() {
    Dispose(false);
  }

  public void Dispose() {
    Dispose(true);
    global::System.GC.SuppressFinalize(this);
  }

  protected virtual void Dispose(bool disposing) {
    lock(this) {
      if (swigCPtr.Handle != global::System.IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          ConvPINVOKE.delete_vector_pair_timeseries(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
    }
  }

  public vector_pair_timeseries(global::System.Collections.IEnumerable c) : this() {
    if (c == null)
      throw new global::System.ArgumentNullException("c");
    foreach (pair_timeseries element in c) {
      this.Add(element);
    }
  }

  public vector_pair_timeseries(global::System.Collections.Generic.IEnumerable<pair_timeseries> c) : this() {
    if (c == null)
      throw new global::System.ArgumentNullException("c");
    foreach (pair_timeseries element in c) {
      this.Add(element);
    }
  }

  public bool IsFixedSize {
    get {
      return false;
    }
  }

  public bool IsReadOnly {
    get {
      return false;
    }
  }

  public pair_timeseries this[int index]  {
    get {
      return getitem(index);
    }
    set {
      setitem(index, value);
    }
  }

  public int Capacity {
    get {
      return (int)capacity();
    }
    set {
      if (value < 0 || (uint)value < size())
        throw new global::System.ArgumentOutOfRangeException("Capacity");
      reserve((uint)value);
    }
  }

  public int Count {
    get {
      return (int)size();
    }
  }

  public bool IsSynchronized {
    get {
      return false;
    }
  }

  public void CopyTo(pair_timeseries[] array)
  {
    CopyTo(0, array, 0, this.Count);
  }

  public void CopyTo(pair_timeseries[] array, int arrayIndex)
  {
    CopyTo(0, array, arrayIndex, this.Count);
  }

  public void CopyTo(int index, pair_timeseries[] array, int arrayIndex, int count)
  {
    if (array == null)
      throw new global::System.ArgumentNullException("array");
    if (index < 0)
      throw new global::System.ArgumentOutOfRangeException("index", "Value is less than zero");
    if (arrayIndex < 0)
      throw new global::System.ArgumentOutOfRangeException("arrayIndex", "Value is less than zero");
    if (count < 0)
      throw new global::System.ArgumentOutOfRangeException("count", "Value is less than zero");
    if (array.Rank > 1)
      throw new global::System.ArgumentException("Multi dimensional array.", "array");
    if (index+count > this.Count || arrayIndex+count > array.Length)
      throw new global::System.ArgumentException("Number of elements to copy is too large.");
    for (int i=0; i<count; i++)
      array.SetValue(getitemcopy(index+i), arrayIndex+i);
  }

  public pair_timeseries[] ToArray() {
    pair_timeseries[] array = new pair_timeseries[this.Count];
    this.CopyTo(array);
    return array;
  }

  global::System.Collections.Generic.IEnumerator<pair_timeseries> global::System.Collections.Generic.IEnumerable<pair_timeseries>.GetEnumerator() {
    return new vector_pair_timeseriesEnumerator(this);
  }

  global::System.Collections.IEnumerator global::System.Collections.IEnumerable.GetEnumerator() {
    return new vector_pair_timeseriesEnumerator(this);
  }

  public vector_pair_timeseriesEnumerator GetEnumerator() {
    return new vector_pair_timeseriesEnumerator(this);
  }

  public static implicit operator vector_pair_timeseries(global::System.Collections.Generic.List<pair_timeseries> v) {
        if (v == null){
            var toReturn = new vector_pair_timeseries();
            return toReturn;
        }else{
            var toReturn = new vector_pair_timeseries(v.Count);
            foreach (var x in v) toReturn.Add(x);
            return toReturn;
        }
  }
  public static implicit operator global::System.Collections.Generic.List<pair_timeseries>(vector_pair_timeseries v) {
        if (v == null){
            var toReturn = new global::System.Collections.Generic.List<pair_timeseries>();
            return toReturn;
        }else{
            var toReturn = new global::System.Collections.Generic.List<pair_timeseries>(v.Count);
            foreach (var x in v) toReturn.Add(x);
            return toReturn;
        }
  }

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
  public sealed class vector_pair_timeseriesEnumerator : global::System.Collections.IEnumerator
    , global::System.Collections.Generic.IEnumerator<pair_timeseries>
  {
    private vector_pair_timeseries collectionRef;
    private int currentIndex;
    private object currentObject;
    private int currentSize;

    public vector_pair_timeseriesEnumerator(vector_pair_timeseries collection) {
      collectionRef = collection;
      currentIndex = -1;
      currentObject = null;
      currentSize = collectionRef.Count;
    }

    // Type-safe iterator Current
    public pair_timeseries Current {
      get {
        if (currentIndex == -1)
          throw new global::System.InvalidOperationException("Enumeration not started.");
        if (currentIndex > currentSize - 1)
          throw new global::System.InvalidOperationException("Enumeration finished.");
        if (currentObject == null)
          throw new global::System.InvalidOperationException("Collection modified.");
        return (pair_timeseries)currentObject;
      }
    }

    // Type-unsafe IEnumerator.Current
    object global::System.Collections.IEnumerator.Current {
      get {
        return Current;
      }
    }

    public bool MoveNext() {
      int size = collectionRef.Count;
      bool moveOkay = (currentIndex+1 < size) && (size == currentSize);
      if (moveOkay) {
        currentIndex++;
        currentObject = collectionRef[currentIndex];
      } else {
        currentObject = null;
      }
      return moveOkay;
    }

    public void Reset() {
      currentIndex = -1;
      currentObject = null;
      if (collectionRef.Count != currentSize) {
        throw new global::System.InvalidOperationException("Collection modified.");
      }
    }

    public void Dispose() {
        currentIndex = -1;
        currentObject = null;
    }
  }

  public void Clear() {
    ConvPINVOKE.vector_pair_timeseries_Clear(swigCPtr);
  }

  public void Add(pair_timeseries x) {
    ConvPINVOKE.vector_pair_timeseries_Add(swigCPtr, pair_timeseries.getCPtr(x));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  private uint size() {
    uint ret = ConvPINVOKE.vector_pair_timeseries_size(swigCPtr);
    return ret;
  }

  private uint capacity() {
    uint ret = ConvPINVOKE.vector_pair_timeseries_capacity(swigCPtr);
    return ret;
  }

  private void reserve(uint n) {
    ConvPINVOKE.vector_pair_timeseries_reserve(swigCPtr, n);
  }

  public vector_pair_timeseries() : this(ConvPINVOKE.new_vector_pair_timeseries__SWIG_0(), true) {
  }

  public vector_pair_timeseries(vector_pair_timeseries other) : this(ConvPINVOKE.new_vector_pair_timeseries__SWIG_1(vector_pair_timeseries.getCPtr(other)), true) {
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public vector_pair_timeseries(int capacity) : this(ConvPINVOKE.new_vector_pair_timeseries__SWIG_2(capacity), true) {
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  private pair_timeseries getitemcopy(int index) {
    pair_timeseries ret = new pair_timeseries(ConvPINVOKE.vector_pair_timeseries_getitemcopy(swigCPtr, index), true);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private pair_timeseries getitem(int index) {
    pair_timeseries ret = new pair_timeseries(ConvPINVOKE.vector_pair_timeseries_getitem(swigCPtr, index), false);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void setitem(int index, pair_timeseries val) {
    ConvPINVOKE.vector_pair_timeseries_setitem(swigCPtr, index, pair_timeseries.getCPtr(val));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void AddRange(vector_pair_timeseries values) {
    ConvPINVOKE.vector_pair_timeseries_AddRange(swigCPtr, vector_pair_timeseries.getCPtr(values));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public vector_pair_timeseries GetRange(int index, int count) {
    global::System.IntPtr cPtr = ConvPINVOKE.vector_pair_timeseries_GetRange(swigCPtr, index, count);
    vector_pair_timeseries ret = (cPtr == global::System.IntPtr.Zero) ? null : new vector_pair_timeseries(cPtr, true);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Insert(int index, pair_timeseries x) {
    ConvPINVOKE.vector_pair_timeseries_Insert(swigCPtr, index, pair_timeseries.getCPtr(x));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void InsertRange(int index, vector_pair_timeseries values) {
    ConvPINVOKE.vector_pair_timeseries_InsertRange(swigCPtr, index, vector_pair_timeseries.getCPtr(values));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveAt(int index) {
    ConvPINVOKE.vector_pair_timeseries_RemoveAt(swigCPtr, index);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveRange(int index, int count) {
    ConvPINVOKE.vector_pair_timeseries_RemoveRange(swigCPtr, index, count);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public static vector_pair_timeseries Repeat(pair_timeseries value, int count) {
    global::System.IntPtr cPtr = ConvPINVOKE.vector_pair_timeseries_Repeat(pair_timeseries.getCPtr(value), count);
    vector_pair_timeseries ret = (cPtr == global::System.IntPtr.Zero) ? null : new vector_pair_timeseries(cPtr, true);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Reverse() {
    ConvPINVOKE.vector_pair_timeseries_Reverse__SWIG_0(swigCPtr);
  }

  public void Reverse(int index, int count) {
    ConvPINVOKE.vector_pair_timeseries_Reverse__SWIG_1(swigCPtr, index, count);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void SetRange(int index, vector_pair_timeseries values) {
    ConvPINVOKE.vector_pair_timeseries_SetRange(swigCPtr, index, vector_pair_timeseries.getCPtr(values));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

}
