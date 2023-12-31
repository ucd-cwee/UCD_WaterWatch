//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (https://www.swig.org).
// Version 4.1.1
//
// Do not make changes to this file unless you know what you are doing - modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------


public class Awaiter : global::System.IDisposable {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal Awaiter(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(Awaiter obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  internal static global::System.Runtime.InteropServices.HandleRef swigRelease(Awaiter obj) {
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

  ~Awaiter() {
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
          ConvPINVOKE.delete_Awaiter(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
    }
  }

  public Awaiter() : this(ConvPINVOKE.new_Awaiter(), true) {
  }

  public bool IsFinished() {
    bool ret = ConvPINVOKE.Awaiter_IsFinished(swigCPtr);
    return ret;
  }

  public string Result() {
    string ret = ConvPINVOKE.Awaiter_Result(swigCPtr);
    return ret;
  }

  public SWIGTYPE_p_std__shared_ptrT_std__string_t data_m {
    set {
      ConvPINVOKE.Awaiter_data_m_set(swigCPtr, SWIGTYPE_p_std__shared_ptrT_std__string_t.getCPtr(value));
    } 
    get {
      global::System.IntPtr cPtr = ConvPINVOKE.Awaiter_data_m_get(swigCPtr);
      SWIGTYPE_p_std__shared_ptrT_std__string_t ret = (cPtr == global::System.IntPtr.Zero) ? null : new SWIGTYPE_p_std__shared_ptrT_std__string_t(cPtr, false);
      return ret;
    } 
  }

  public SWIGTYPE_p_std__shared_ptrT_bool_t isFinished_m {
    set {
      ConvPINVOKE.Awaiter_isFinished_m_set(swigCPtr, SWIGTYPE_p_std__shared_ptrT_bool_t.getCPtr(value));
    } 
    get {
      global::System.IntPtr cPtr = ConvPINVOKE.Awaiter_isFinished_m_get(swigCPtr);
      SWIGTYPE_p_std__shared_ptrT_bool_t ret = (cPtr == global::System.IntPtr.Zero) ? null : new SWIGTYPE_p_std__shared_ptrT_bool_t(cPtr, false);
      return ret;
    } 
  }

}
