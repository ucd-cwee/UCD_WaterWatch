//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (https://www.swig.org).
// Version 4.1.1
//
// Do not make changes to this file unless you know what you are doing - modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------


public class SharedMatrix : global::System.IDisposable {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal SharedMatrix(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(SharedMatrix obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  internal static global::System.Runtime.InteropServices.HandleRef swigRelease(SharedMatrix obj) {
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

  ~SharedMatrix() {
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
          ConvPINVOKE.delete_SharedMatrix(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
    }
  }

  public SharedMatrix() : this(ConvPINVOKE.new_SharedMatrix__SWIG_0(), true) {
  }

  public SharedMatrix(int index, bool deleteDataWhenScopeEnds) : this(ConvPINVOKE.new_SharedMatrix__SWIG_1(index, deleteDataWhenScopeEnds), true) {
  }

  public SharedMatrix(int index) : this(ConvPINVOKE.new_SharedMatrix__SWIG_2(index), true) {
  }

  public SharedMatrix(SharedMatrix other) : this(ConvPINVOKE.new_SharedMatrix__SWIG_3(SharedMatrix.getCPtr(other)), true) {
    if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
  }

  public void Clear() {
    ConvPINVOKE.SharedMatrix_Clear(swigCPtr);
  }

  public void AppendData(double X, double Y, float value) {
    ConvPINVOKE.SharedMatrix_AppendData(swigCPtr, X, Y, value);
  }

  public double GetValue(double X, double Y) {
    double ret = ConvPINVOKE.SharedMatrix_GetValue(swigCPtr, X, Y);
    return ret;
  }

  public vector_double GetKnotSeries(double Left, double Top, double Right, double Bottom, int numColumns, int numRows) {
    vector_double ret = new vector_double(ConvPINVOKE.SharedMatrix_GetKnotSeries(swigCPtr, Left, Top, Right, Bottom, numColumns, numRows), true);
    return ret;
  }

  public vector_double GetTimeSeries(double Left, double Top, double Right, double Bottom, int numColumns, int numRows) {
    vector_double ret = new vector_double(ConvPINVOKE.SharedMatrix_GetTimeSeries(swigCPtr, Left, Top, Right, Bottom, numColumns, numRows), true);
    return ret;
  }

  public double GetMinX() {
    double ret = ConvPINVOKE.SharedMatrix_GetMinX(swigCPtr);
    return ret;
  }

  public double GetMaxX() {
    double ret = ConvPINVOKE.SharedMatrix_GetMaxX(swigCPtr);
    return ret;
  }

  public double GetMinY() {
    double ret = ConvPINVOKE.SharedMatrix_GetMinY(swigCPtr);
    return ret;
  }

  public double GetMaxY() {
    double ret = ConvPINVOKE.SharedMatrix_GetMaxY(swigCPtr);
    return ret;
  }

  public double GetMinValue() {
    double ret = ConvPINVOKE.SharedMatrix_GetMinValue(swigCPtr);
    return ret;
  }

  public double GetMaxValue() {
    double ret = ConvPINVOKE.SharedMatrix_GetMaxValue(swigCPtr);
    return ret;
  }

  public int GetNumValues() {
    int ret = ConvPINVOKE.SharedMatrix_GetNumValues(swigCPtr);
    return ret;
  }

  public int Index() {
    int ret = ConvPINVOKE.SharedMatrix_Index(swigCPtr);
    return ret;
  }

}