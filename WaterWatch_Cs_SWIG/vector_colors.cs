//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (https://www.swig.org).
// Version 4.1.1
//
// Do not make changes to this file unless you know what you are doing - modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------


public class vector_colors : global::System.IDisposable, global::System.Collections.IEnumerable, global::System.Collections.Generic.IEnumerable<Color_Interop>
 {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal vector_colors(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(vector_colors obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  internal static global::System.Runtime.InteropServices.HandleRef swigRelease(vector_colors obj) {
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

  ~vector_colors() {
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
          ConvPINVOKE.delete_vector_colors(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
    }
  }

  public vector_colors(global::System.Collections.IEnumerable c) : this() {
    if (c == null)
      throw new global::System.ArgumentNullException("c");
    foreach (Color_Interop element in c) {
      this.Add(element);
    }
  }

  public vector_colors(global::System.Collections.Generic.IEnumerable<Color_Interop> c) : this() {
    if (c == null)
      throw new global::System.ArgumentNullException("c");
    foreach (Color_Interop element in c) {
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

  public Color_Interop this[int index]  {
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

  public void CopyTo(Color_Interop[] array)
  {
    CopyTo(0, array, 0, this.Count);
  }

  public void CopyTo(Color_Interop[] array, int arrayIndex)
  {
    CopyTo(0, array, arrayIndex, this.Count);
  }

  public void CopyTo(int index, Color_Interop[] array, int arrayIndex, int count)
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

  public Color_Interop[] ToArray() {
    Color_Interop[] array = new Color_Interop[this.Count];
    this.CopyTo(array);
    return array;
  }

  global::System.Collections.Generic.IEnumerator<Color_Interop> global::System.Collections.Generic.IEnumerable<Color_Interop>.GetEnumerator() {
    return new vector_colorsEnumerator(this);
  }

  global::System.Collections.IEnumerator global::System.Collections.IEnumerable.GetEnumerator() {
    return new vector_colorsEnumerator(this);
  }

  public vector_colorsEnumerator GetEnumerator() {
    return new vector_colorsEnumerator(this);
  }

  public static implicit operator vector_colors(global::System.Collections.Generic.List<Color_Interop> v) {
        if (v == null){
            var toReturn = new vector_colors();
            return toReturn;
        }else{
            var toReturn = new vector_colors(v.Count);
            foreach (var x in v) toReturn.Add(x);
            return toReturn;
        }
  }
  public static implicit operator global::System.Collections.Generic.List<Color_Interop>(vector_colors v) {
        if (v == null){
            var toReturn = new global::System.Collections.Generic.List<Color_Interop>();
            return toReturn;
        }else{
            var toReturn = new global::System.Collections.Generic.List<Color_Interop>(v.Count);
            foreach (var x in v) toReturn.Add(x);
            return toReturn;
        }
  }

  // Type-safe enumerator
  /// Note that the IEnumerator documentation requires an InvalidOperationException to be thrown
  /// whenever the collection is modified. This has been done for changes in the size of the
  /// collection but not when one of the elements of the collection is modified as it is a bit
  /// tricky to detect unmanaged code that modifies the collection under our feet.
  public sealed class vector_colorsEnumerator : global::System.Collections.IEnumerator
    , global::System.Collections.Generic.IEnumerator<Color_Interop>
  {
    private vector_colors collectionRef;
    private int currentIndex;
    private object currentObject;
    private int currentSize;

    public vector_colorsEnumerator(vector_colors collection) {
      collectionRef = collection;
      currentIndex = -1;
      currentObject = null;
      currentSize = collectionRef.Count;
    }

    // Type-safe iterator Current
    public Color_Interop Current {
      get {
        if (currentIndex == -1)
          throw new global::System.InvalidOperationException("Enumeration not started.");
        if (currentIndex > currentSize - 1)
          throw new global::System.InvalidOperationException("Enumeration finished.");
        if (currentObject == null)
          throw new global::System.InvalidOperationException("Collection modified.");
        return (Color_Interop)currentObject;
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
    ConvPINVOKE.vector_colors_Clear(swigCPtr);
  }

  public void Add(Color_Interop x) {
    ConvPINVOKE.vector_colors_Add(swigCPtr, Color_Interop.getCPtr(x));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  private uint size() {
    uint ret = ConvPINVOKE.vector_colors_size(swigCPtr);
    return ret;
  }

  private uint capacity() {
    uint ret = ConvPINVOKE.vector_colors_capacity(swigCPtr);
    return ret;
  }

  private void reserve(uint n) {
    ConvPINVOKE.vector_colors_reserve(swigCPtr, n);
  }

  public vector_colors() : this(ConvPINVOKE.new_vector_colors__SWIG_0(), true) {
  }

  public vector_colors(vector_colors other) : this(ConvPINVOKE.new_vector_colors__SWIG_1(vector_colors.getCPtr(other)), true) {
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public vector_colors(int capacity) : this(ConvPINVOKE.new_vector_colors__SWIG_2(capacity), true) {
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  private Color_Interop getitemcopy(int index) {
    Color_Interop ret = new Color_Interop(ConvPINVOKE.vector_colors_getitemcopy(swigCPtr, index), true);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private Color_Interop getitem(int index) {
    Color_Interop ret = new Color_Interop(ConvPINVOKE.vector_colors_getitem(swigCPtr, index), false);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  private void setitem(int index, Color_Interop val) {
    ConvPINVOKE.vector_colors_setitem(swigCPtr, index, Color_Interop.getCPtr(val));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void AddRange(vector_colors values) {
    ConvPINVOKE.vector_colors_AddRange(swigCPtr, vector_colors.getCPtr(values));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public vector_colors GetRange(int index, int count) {
    global::System.IntPtr cPtr = ConvPINVOKE.vector_colors_GetRange(swigCPtr, index, count);
    vector_colors ret = (cPtr == global::System.IntPtr.Zero) ? null : new vector_colors(cPtr, true);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Insert(int index, Color_Interop x) {
    ConvPINVOKE.vector_colors_Insert(swigCPtr, index, Color_Interop.getCPtr(x));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void InsertRange(int index, vector_colors values) {
    ConvPINVOKE.vector_colors_InsertRange(swigCPtr, index, vector_colors.getCPtr(values));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveAt(int index) {
    ConvPINVOKE.vector_colors_RemoveAt(swigCPtr, index);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void RemoveRange(int index, int count) {
    ConvPINVOKE.vector_colors_RemoveRange(swigCPtr, index, count);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public static vector_colors Repeat(Color_Interop value, int count) {
    global::System.IntPtr cPtr = ConvPINVOKE.vector_colors_Repeat(Color_Interop.getCPtr(value), count);
    vector_colors ret = (cPtr == global::System.IntPtr.Zero) ? null : new vector_colors(cPtr, true);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public void Reverse() {
    ConvPINVOKE.vector_colors_Reverse__SWIG_0(swigCPtr);
  }

  public void Reverse(int index, int count) {
    ConvPINVOKE.vector_colors_Reverse__SWIG_1(swigCPtr, index, count);
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void SetRange(int index, vector_colors values) {
    ConvPINVOKE.vector_colors_SetRange(swigCPtr, index, vector_colors.getCPtr(values));
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

}