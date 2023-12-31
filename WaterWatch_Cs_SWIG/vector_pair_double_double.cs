//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (https://www.swig.org).
// Version 4.1.1
//
// Do not make changes to this file unless you know what you are doing - modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------


public class vector_pair_double_double : global::System.IDisposable, global::System.Collections.IEnumerable, global::System.Collections.Generic.IEnumerable<pair_double_double>
 {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal vector_pair_double_double(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(vector_pair_double_double obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  internal static global::System.Runtime.InteropServices.HandleRef swigRelease(vector_pair_double_double obj) {
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

  ~vector_pair_double_double() {
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
          ConvPINVOKE.delete_vector_pair_double_double(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
    }
  }

  public vector_pair_double_double(global::System.Collections.IEnumerable c) : this() {
    if (c == null)
      throw new global::System.ArgumentNullException("c");
    foreach (pair_double_double element in c) {
      this.Add(element);
    }
  }

  public vector_pair_double_double(global::System.Collections.Generic.IEnumerable<pair_double_double> c) : this() {
    if (c == null)
      throw new global::System.ArgumentNullException("c");
    foreach (pair_double_double element in c) {
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

  public pair_double_double this[int index]  {
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

  public void CopyTo(pair_double_double[] array)
  {
    CopyTo(0, array, 0, this.Count);
  }

  public void CopyTo(pair_double_double[] array, int arrayIndex)
  {
    CopyTo(0, array, arrayIndex, this.Count);
  }

  public void CopyTo(int index, pair_double_double[] array, int arrayIndex, int count)
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

  public pair_double_double[] ToArray() {
    pair_double_double[] array = new pair_double_double[this.Count];
    this.CopyTo(array);
    return array;
  }

  global::System.Collections.Generic.IEnumerator<pair_double_double> global::System.Collections.Generic.IEnumerable<pair_double_double>.GetEnumerator() {
    return new vector_pair_double_doubleEnumerator(this);
  }

  global::System.Collections.IEnumerator global::System.Collections.IEnumerable.GetEnumerator() {
    return new vector_pair_double_doubleEnumerator(this);
  }

  public vector_pair_double_doubleEnumerator GetEnumerator() {
    return new vector_pair_double_doubleEnumerator(this);
  }

  public static implicit operator vector_pair_double_double(global::System.Collections.Generic.List<pair_double_double> v) {
        if (v == null){
            var toReturn = new vector_pair_double_double();
            return toReturn;
        }else{
            var toReturn = new vector_pair_double_double(v.Count);
            foreach (var x in v) toReturn.Add(x);
            return toReturn;
        }
  }
  public static implicit operator global::System.Collections.Generic.List<pair_double_double>(vector_pair_double_double v) {
        if (v == null){
            var toReturn = new global::System.Collections.Generic.List<pair_double_double>();
            return toReturn;
        }else{
            var toReturn = new global::System.Collections.Generic.List<pair_double_double>(v.Count);
            foreach (var x in v) toReturn.Add(x);
            return toReturn;
        }
  }

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
  public sealed class vector_pair_double_doubleEnumerator : global::System.Collections.IEnumerator
    , global::System.Collections.Generic.IEnumerator<pair_double_double>
  {
    private vector_pair_double_double collectionRef;
    private int currentIndex;
    private object currentObject;
    private int currentSize;

    public vector_pair_double_doubleEnumerator(vector_pair_double_double collection) {
      collectionRef = collection;
      currentIndex = -1;
      currentObject = null;
      currentSize = collectionRef.Count;
    }

    // Type-safe iterator Current
    public pair_double_double Current {
      get {
        if (currentIndex == -1)
          throw new global::System.InvalidOperationException("Enumeration not started.");
        if (currentIndex > currentSize - 1)
          throw new global::System.InvalidOperationException("Enumeration finished.");
        if (currentObject == null)
          throw new global::System.InvalidOperationException("Collection modified.");
        return (pair_double_double)currentObject;
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
    ConvPINVOKE.vector_pair_double_double_Clear(swigCPtr);
  }

  public void Add(pair_double_double x) {
    ConvPINVOKE.vector_pair_double_double_Add(swigCPtr, pair_double_double.getCPtr(x));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  private uint size() {
    uint ret = ConvPINVOKE.vector_pair_double_double_size(swigCPtr);
    return ret;
  }

  private uint capacity() {
    uint ret = ConvPINVOKE.vector_pair_double_double_capacity(swigCPtr);
    return ret;
  }

  private void reserve(uint n) {
    ConvPINVOKE.vector_pair_double_double_reserve(swigCPtr, n);
  }

  public vector_pair_double_double() : this(ConvPINVOKE.new_vector_pair_double_double__SWIG_0(), true) {
  }

  public vector_pair_double_double(vector_pair_double_double other) : this(ConvPINVOKE.new_vector_pair_double_double__SWIG_1(vector_pair_double_double.getCPtr(other)), true) {
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public vector_pair_double_double(int capacity) : this(ConvPINVOKE.new_vector_pair_double_double__SWIG_2(capacity), true) {
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  private pair_double_double getitemcopy(int index) {
    pair_double_double ret = new pair_double_double(ConvPINVOKE.vector_pair_double_double_getitemcopy(swigCPtr, index), true);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private pair_double_double getitem(int index) {
    pair_double_double ret = new pair_double_double(ConvPINVOKE.vector_pair_double_double_getitem(swigCPtr, index), false);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void setitem(int index, pair_double_double val) {
    ConvPINVOKE.vector_pair_double_double_setitem(swigCPtr, index, pair_double_double.getCPtr(val));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void AddRange(vector_pair_double_double values) {
    ConvPINVOKE.vector_pair_double_double_AddRange(swigCPtr, vector_pair_double_double.getCPtr(values));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public vector_pair_double_double GetRange(int index, int count) {
    global::System.IntPtr cPtr = ConvPINVOKE.vector_pair_double_double_GetRange(swigCPtr, index, count);
    vector_pair_double_double ret = (cPtr == global::System.IntPtr.Zero) ? null : new vector_pair_double_double(cPtr, true);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Insert(int index, pair_double_double x) {
    ConvPINVOKE.vector_pair_double_double_Insert(swigCPtr, index, pair_double_double.getCPtr(x));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void InsertRange(int index, vector_pair_double_double values) {
    ConvPINVOKE.vector_pair_double_double_InsertRange(swigCPtr, index, vector_pair_double_double.getCPtr(values));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveAt(int index) {
    ConvPINVOKE.vector_pair_double_double_RemoveAt(swigCPtr, index);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveRange(int index, int count) {
    ConvPINVOKE.vector_pair_double_double_RemoveRange(swigCPtr, index, count);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public static vector_pair_double_double Repeat(pair_double_double value, int count) {
    global::System.IntPtr cPtr = ConvPINVOKE.vector_pair_double_double_Repeat(pair_double_double.getCPtr(value), count);
    vector_pair_double_double ret = (cPtr == global::System.IntPtr.Zero) ? null : new vector_pair_double_double(cPtr, true);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Reverse() {
    ConvPINVOKE.vector_pair_double_double_Reverse__SWIG_0(swigCPtr);
  }

  public void Reverse(int index, int count) {
    ConvPINVOKE.vector_pair_double_double_Reverse__SWIG_1(swigCPtr, index, count);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void SetRange(int index, vector_pair_double_double values) {
    ConvPINVOKE.vector_pair_double_double_SetRange(swigCPtr, index, vector_pair_double_double.getCPtr(values));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

}
