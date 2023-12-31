//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (https://www.swig.org).
// Version 4.1.1
//
// Do not make changes to this file unless you know what you are doing - modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------


public class cweeDateTime : global::System.IDisposable {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal cweeDateTime(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(cweeDateTime obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  internal static global::System.Runtime.InteropServices.HandleRef swigRelease(cweeDateTime obj) {
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

  ~cweeDateTime() {
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
          ConvPINVOKE.delete_cweeDateTime(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
    }
  }

  public cweeDateTime() : this(ConvPINVOKE.new_cweeDateTime__SWIG_0(), true) {
  }

  public cweeDateTime(double x) : this(ConvPINVOKE.new_cweeDateTime__SWIG_1(x), true) {
  }

  public double unixTime {
    set {
      ConvPINVOKE.cweeDateTime_unixTime_set(swigCPtr, value);
    } 
    get {
      double ret = ConvPINVOKE.cweeDateTime_unixTime_get(swigCPtr);
      return ret;
    } 
  }

  public int year {
    set {
      ConvPINVOKE.cweeDateTime_year_set(swigCPtr, value);
    } 
    get {
      int ret = ConvPINVOKE.cweeDateTime_year_get(swigCPtr);
      return ret;
    } 
  }

  public int month {
    set {
      ConvPINVOKE.cweeDateTime_month_set(swigCPtr, value);
    } 
    get {
      int ret = ConvPINVOKE.cweeDateTime_month_get(swigCPtr);
      return ret;
    } 
  }

  public int day {
    set {
      ConvPINVOKE.cweeDateTime_day_set(swigCPtr, value);
    } 
    get {
      int ret = ConvPINVOKE.cweeDateTime_day_get(swigCPtr);
      return ret;
    } 
  }

  public int hour {
    set {
      ConvPINVOKE.cweeDateTime_hour_set(swigCPtr, value);
    } 
    get {
      int ret = ConvPINVOKE.cweeDateTime_hour_get(swigCPtr);
      return ret;
    } 
  }

  public int minute {
    set {
      ConvPINVOKE.cweeDateTime_minute_set(swigCPtr, value);
    } 
    get {
      int ret = ConvPINVOKE.cweeDateTime_minute_get(swigCPtr);
      return ret;
    } 
  }

  public int second {
    set {
      ConvPINVOKE.cweeDateTime_second_set(swigCPtr, value);
    } 
    get {
      int ret = ConvPINVOKE.cweeDateTime_second_get(swigCPtr);
      return ret;
    } 
  }

  public int milliseconds {
    set {
      ConvPINVOKE.cweeDateTime_milliseconds_set(swigCPtr, value);
    } 
    get {
      int ret = ConvPINVOKE.cweeDateTime_milliseconds_get(swigCPtr);
      return ret;
    } 
  }

}
