// RubyFW: a Firewire Library for Ruby
// Copyright (c) 2009 iZsh - izsh at iphone-dev.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/IOKitLib.h>
#import <IOKit/firewire/IOFireWireLib.h>

#include <ruby.h>

/* ================
    FWDevice class
   ================*/
// exported members are:
// GUID accessible through guid() => Bignum
// VendorName accessible through vendorName() => String
// VendorID accessible through vendorID() -> Fixnum
// NodeID accessible through nodeID() => Fixnum
// Speed accessible through speed() => Fixnum

typedef struct {
  io_object_t   Device;
  long long   GUID;
  char *VendorName;
  int VendorID;
  int NodeID;
  int Speed;
} FWDevice;

static VALUE cFWDevice;

static void FWDevice_free(FWDevice *Obj)
{
  free(Obj->VendorName);
  free(Obj);
}

static VALUE FWDevice_new(io_object_t Device,
                          long long GUID,
                          char *VendorName,
                          int VendorID,
                          int NodeID,
                          int Speed)
{
  VALUE fwdevice;
  FWDevice *obj;
  fwdevice = Data_Make_Struct(cFWDevice, FWDevice, 0, FWDevice_free, obj);
  rb_obj_call_init(fwdevice, 0, 0);
  
  obj->Device = Device;
  obj->GUID = GUID;
  obj->VendorName = VendorName;
  obj->VendorID = VendorID;
  obj->NodeID = NodeID;
  obj->Speed = Speed;

  return fwdevice;
}

static VALUE FWDevice_getGUID(VALUE Self)
{
  FWDevice *obj;
  Data_Get_Struct(Self, FWDevice, obj);
  
  return rb_ll2inum(obj->GUID);
}

static VALUE FWDevice_getVendorName(VALUE Self)
{
  FWDevice *obj;
  Data_Get_Struct(Self, FWDevice, obj);
  
  return rb_str_new2(obj->VendorName);
}

static VALUE FWDevice_getVendorID(VALUE Self)
{
  FWDevice *obj;
  Data_Get_Struct(Self, FWDevice, obj);
  
  return INT2FIX(obj->VendorID);
}

static VALUE FWDevice_getNodeID(VALUE Self)
{
  FWDevice *obj;
  Data_Get_Struct(Self, FWDevice, obj);
  
  return INT2FIX(obj->NodeID);
}

static VALUE FWDevice_getSpeed(VALUE Self)
{
  FWDevice *obj;
  Data_Get_Struct(Self, FWDevice, obj);
  
  return INT2FIX(obj->Speed);
}

/* =================
    Local Functions
   =================*/

// Nothing :)

/* ====================
    Exported Functions
   ====================*/

// Scan the FW bus and enumerate the devices
static VALUE scanbus()
{
  VALUE ret = rb_ary_new();
  
  mach_port_t master_port;
  io_iterator_t iterator;
  kern_return_t result = IOMasterPort(MACH_PORT_NULL, &master_port);
  CFMutableDictionaryRef matchingDictionary = IOServiceMatching("IOFireWireDevice");
  result = IOServiceGetMatchingServices(master_port, matchingDictionary, &iterator);
  
  io_object_t device;
  while((device = IOIteratorNext(iterator)))
  {
    CFNumberRef guidRef = IORegistryEntryCreateCFProperty(device, CFSTR("GUID"), NULL, 0);
    CFStringRef vendornameRef = IORegistryEntryCreateCFProperty(device, CFSTR("FireWire Vendor Name"), NULL, 0);
    CFNumberRef speedRef = IORegistryEntryCreateCFProperty(device, CFSTR("FireWire Speed"), NULL, 0);
    CFNumberRef nodeidRef = IORegistryEntryCreateCFProperty(device, CFSTR("FireWire Node ID"), NULL, 0);
    CFNumberRef vendoridRef = IORegistryEntryCreateCFProperty(device, CFSTR("Vendor_ID"), NULL, 0);

    CFIndex vendorname_len = CFStringGetLength(vendornameRef);
    char *vendorname = (char*)malloc(vendorname_len + 1);
    memset(vendorname, 0, vendorname_len);
    long long guid;
    SInt32 speed, vendorid, nodeid;
    if(guidRef
      && CFNumberGetValue(guidRef, kCFNumberLongLongType, &guid)
      && vendornameRef
      && CFStringGetCString(vendornameRef, vendorname, vendorname_len + 1, kCFStringEncodingMacRoman)
      && speedRef
      && CFNumberGetValue(speedRef, kCFNumberSInt32Type, &speed)
      && nodeidRef
      && CFNumberGetValue(nodeidRef, kCFNumberSInt32Type, &nodeid)
      && vendoridRef
      && CFNumberGetValue(vendoridRef, kCFNumberSInt32Type, &vendorid)
      )
    {
      VALUE fwdevice = FWDevice_new(device, guid, vendorname, vendorid, nodeid, speed);
      rb_ary_push(ret, fwdevice);
      CFRelease(guidRef);
      CFRelease(vendornameRef);
      CFRelease(speedRef);
      CFRelease(nodeidRef);
      CFRelease(vendoridRef);
    }
  }
  return ret;
}

// Read a 32bit value
static VALUE FWDevice_readQuadlet(VALUE Self, VALUE StartAddr)
{
  FWDevice *device;
  Data_Get_Struct(Self, FWDevice, device);
  
  IOCFPlugInInterface **cfPlugInInterface = NULL;
  SInt32 theScore;
  IOFireWireLibDeviceRef fwIntf;  

  IOReturn result = IOCreatePlugInInterfaceForService(device->Device, 
                      kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, 
                      &cfPlugInInterface, &theScore);
  (*cfPlugInInterface)->QueryInterface(cfPlugInInterface,
                          CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID), 
                          (void **) &fwIntf);
  assert((*fwIntf)->InterfaceIsInited(fwIntf));

  VALUE ret;

  if((*fwIntf)->Open(fwIntf) == 0)
  {
    // Set destination adress. Note that the upper 48 bits identify 
    // the device on the bus and the address set by the operating system.
    uint64_t startaddr = rb_num2ull(StartAddr);
    FWAddress fwaddr;

    fwaddr.nodeID = 0;
    fwaddr.addressHi =  startaddr >> 32;
    fwaddr.addressLo = startaddr & 0xffffffffL;

    // do the actual read
    UInt32 val = 0;
    result = (*fwIntf)->ReadQuadlet(fwIntf, device->Device, &fwaddr, &val, false, 0);
    ret = rb_hash_new();
    rb_hash_aset(ret, ID2SYM(rb_intern("value")), INT2FIX(val));
    rb_hash_aset(ret, ID2SYM(rb_intern("resultcode")), INT2FIX(result));
  }
  else
  {
    rb_fatal("Device->Open() failed");
    return Qnil;
  }
  
  if (fwIntf != NULL)
    (*fwIntf)->Close(fwIntf);

  if (cfPlugInInterface != NULL)
    IODestroyPlugInInterface(cfPlugInInterface);

  return ret;
}

// Read more values...
static VALUE FWDevice_read(VALUE Self, VALUE StartAddr, VALUE Size)
{
  FWDevice *device;
  Data_Get_Struct(Self, FWDevice, device);
  
  IOCFPlugInInterface **cfPlugInInterface = NULL;
  SInt32 theScore;
  IOFireWireLibDeviceRef fwIntf;  

  IOReturn result = IOCreatePlugInInterfaceForService(device->Device, 
                      kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, 
                      &cfPlugInInterface, &theScore);
  (*cfPlugInInterface)->QueryInterface(cfPlugInInterface,
                          CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID), 
                          (void **) &fwIntf);
  assert((*fwIntf)->InterfaceIsInited(fwIntf));

  VALUE ret;

  if((*fwIntf)->Open(fwIntf) == 0)
  {
    // Set destination adress. Note that the upper 48 bits identify 
    // the device on the bus and the address set by the operating system.
    uint64_t startaddr = rb_num2ull(StartAddr);
    FWAddress fwaddr;

    fwaddr.nodeID = 0;
    fwaddr.addressHi =  startaddr >> 32;
    fwaddr.addressLo = startaddr & 0xffffffffL;

    // do the actual read and store the result code in an object attribute
    UInt32 bufsize = NUM2UINT(Size);
    void *buffer = malloc(bufsize);
    memset(buffer, 0, bufsize);
    result = (*fwIntf)->Read(fwIntf, device->Device, &fwaddr, buffer, &bufsize, false, 0);
    ret = rb_hash_new();
    rb_hash_aset(ret, ID2SYM(rb_intern("buffer")), rb_str_new(buffer, bufsize));
    rb_hash_aset(ret, ID2SYM(rb_intern("resultcode")), INT2FIX(result));
    free(buffer);
  }
  else
  {
    rb_fatal("Device->Open() failed");
    return Qnil;
  }
  
  if (fwIntf != NULL)
    (*fwIntf)->Close(fwIntf);

  if (cfPlugInInterface != NULL)
    IODestroyPlugInInterface(cfPlugInInterface);

  return ret;
}

// Write a 32bit value
static VALUE FWDevice_writeQuadlet(VALUE Self, VALUE StartAddr, VALUE Val)
{
  FWDevice *device;
  Data_Get_Struct(Self, FWDevice, device);
  
  IOCFPlugInInterface **cfPlugInInterface = NULL;
  SInt32 theScore;
  IOFireWireLibDeviceRef fwIntf;  

  IOReturn result = IOCreatePlugInInterfaceForService(device->Device, 
                      kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, 
                      &cfPlugInInterface, &theScore);
  (*cfPlugInInterface)->QueryInterface(cfPlugInInterface,
                          CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID), 
                          (void **) &fwIntf);
  assert((*fwIntf)->InterfaceIsInited(fwIntf));

  VALUE ret;

  if((*fwIntf)->Open(fwIntf) == 0)
  {
    // Set destination adress. Note that the upper 48 bits identify 
    // the device on the bus and the address set by the operating system.
    uint64_t startaddr = rb_num2ull(StartAddr);
    FWAddress fwaddr;

    fwaddr.nodeID = 0;
    fwaddr.addressHi =  startaddr >> 32;
    fwaddr.addressLo = startaddr & 0xffffffffL;

    // do the actual read
    result = (*fwIntf)->WriteQuadlet(fwIntf, device->Device, &fwaddr, NUM2UINT(Val), false, 0);
    ret = INT2FIX(result);
  }
  else
  {
    rb_fatal("Device->Open() failed");
    return Qnil;
  }
  
  if (fwIntf != NULL)
    (*fwIntf)->Close(fwIntf);

  if (cfPlugInInterface != NULL)
    IODestroyPlugInInterface(cfPlugInInterface);

  return ret;
}

// Write more values
static VALUE FWDevice_write(VALUE Self, VALUE StartAddr, VALUE Buffer, VALUE Size)
{
  FWDevice *device;
  Data_Get_Struct(Self, FWDevice, device);
  
  IOCFPlugInInterface **cfPlugInInterface = NULL;
  SInt32 theScore;
  IOFireWireLibDeviceRef fwIntf;  

  IOReturn result = IOCreatePlugInInterfaceForService(device->Device, 
                      kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, 
                      &cfPlugInInterface, &theScore);
  (*cfPlugInInterface)->QueryInterface(cfPlugInInterface,
                          CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID), 
                          (void **) &fwIntf);
  assert((*fwIntf)->InterfaceIsInited(fwIntf));

  VALUE ret;

  if((*fwIntf)->Open(fwIntf) == 0)
  {
    // Set destination adress. Note that the upper 48 bits identify 
    // the device on the bus and the address set by the operating system.
    uint64_t startaddr = rb_num2ull(StartAddr);
    FWAddress fwaddr;

    fwaddr.nodeID = 0;
    fwaddr.addressHi =  startaddr >> 32;
    fwaddr.addressLo = startaddr & 0xffffffffL;

    // do the actual read
    UInt32 bufsize = NUM2UINT(Size);
    result = (*fwIntf)->Write(fwIntf, device->Device, &fwaddr, RSTRING(Buffer)->ptr, &bufsize, false, 0);
    ret = INT2FIX(result);
  }
  else
  {
    rb_fatal("Device->Open() failed");
    return Qnil;
  }
  
  if (fwIntf != NULL)
    (*fwIntf)->Close(fwIntf);

  if (cfPlugInInterface != NULL)
    IODestroyPlugInInterface(cfPlugInInterface);

  return ret;
}

/* =================
    Initializations
   =================*/

// We export our functions and classes in the module FW
void Init_fwext()
{
  VALUE fw_module = rb_define_module("FW");
  rb_define_module_function(fw_module, "scanbus", scanbus, 0);

  cFWDevice = rb_define_class_under(fw_module, "FWDevice", rb_cObject);
  rb_define_method(cFWDevice, "guid", FWDevice_getGUID, 0);
  rb_define_method(cFWDevice, "vendorName", FWDevice_getVendorName, 0);
  rb_define_method(cFWDevice, "vendorID", FWDevice_getVendorID, 0);
  rb_define_method(cFWDevice, "nodeID", FWDevice_getNodeID, 0);
  rb_define_method(cFWDevice, "speed", FWDevice_getSpeed, 0);
  rb_define_method(cFWDevice, "readQuadlet", FWDevice_readQuadlet, 1);
  rb_define_method(cFWDevice, "read", FWDevice_read, 2);
  rb_define_method(cFWDevice, "writeQuadlet", FWDevice_readQuadlet, 2);
  rb_define_method(cFWDevice, "write", FWDevice_read, 3);
}
