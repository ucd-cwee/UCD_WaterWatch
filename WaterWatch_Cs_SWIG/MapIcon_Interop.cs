//------------------------------------------------------------------------------
// <auto-generated />
//
// This file was automatically generated by SWIG (https://www.swig.org).
// Version 4.1.1
//
// Do not make changes to this file unless you know what you are doing - modify
// the SWIG interface file instead.
//------------------------------------------------------------------------------


public class MapIcon_Interop : global::System.IDisposable {
  private global::System.Runtime.InteropServices.HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal MapIcon_Interop(global::System.IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new global::System.Runtime.InteropServices.HandleRef(this, cPtr);
  }

  internal static global::System.Runtime.InteropServices.HandleRef getCPtr(MapIcon_Interop obj) {
    return (obj == null) ? new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero) : obj.swigCPtr;
  }

  internal static global::System.Runtime.InteropServices.HandleRef swigRelease(MapIcon_Interop obj) {
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

  ~MapIcon_Interop() {
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
          ConvPINVOKE.delete_MapIcon_Interop(swigCPtr);
        }
        swigCPtr = new global::System.Runtime.InteropServices.HandleRef(null, global::System.IntPtr.Zero);
      }
    }
  }

  public Color_Interop color {
    set {
      ConvPINVOKE.MapIcon_Interop_color_set(swigCPtr, Color_Interop.getCPtr(value));
    } 
    get {
      global::System.IntPtr cPtr = ConvPINVOKE.MapIcon_Interop_color_get(swigCPtr);
      Color_Interop ret = (cPtr == global::System.IntPtr.Zero) ? null : new Color_Interop(cPtr, false);
      return ret;
    } 
  }

  public double size {
    set {
      ConvPINVOKE.MapIcon_Interop_size_set(swigCPtr, value);
    } 
    get {
      double ret = ConvPINVOKE.MapIcon_Interop_size_get(swigCPtr);
      return ret;
    } 
  }

  public double longitude {
    set {
      ConvPINVOKE.MapIcon_Interop_longitude_set(swigCPtr, value);
    } 
    get {
      double ret = ConvPINVOKE.MapIcon_Interop_longitude_get(swigCPtr);
      return ret;
    } 
  }

  public double latitude {
    set {
      ConvPINVOKE.MapIcon_Interop_latitude_set(swigCPtr, value);
    } 
    get {
      double ret = ConvPINVOKE.MapIcon_Interop_latitude_get(swigCPtr);
      return ret;
    } 
  }

  public bool HideOnCollision {
    set {
      ConvPINVOKE.MapIcon_Interop_HideOnCollision_set(swigCPtr, value);
    } 
    get {
      bool ret = ConvPINVOKE.MapIcon_Interop_HideOnCollision_get(swigCPtr);
      return ret;
    } 
  }

  public string IconPathGeometry {
    set {
      ConvPINVOKE.MapIcon_Interop_IconPathGeometry_set(swigCPtr, value);
      if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      string ret = ConvPINVOKE.MapIcon_Interop_IconPathGeometry_get(swigCPtr);
      if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public string Label {
    set {
      ConvPINVOKE.MapIcon_Interop_Label_set(swigCPtr, value);
      if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
    } 
    get {
      string ret = ConvPINVOKE.MapIcon_Interop_Label_get(swigCPtr);
      if (ConvPINVOKE.SWIGPendingException.Pending) throw ConvPINVOKE.SWIGPendingException.Retrieve();
      return ret;
    } 
  }

  public MapIcon_Interop() : this(ConvPINVOKE.new_MapIcon_Interop(), true) {
  }

}