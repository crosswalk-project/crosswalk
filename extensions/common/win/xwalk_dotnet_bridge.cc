// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/extensions/common/win/xwalk_dotnet_bridge.h"

#include <Objbase.h>
#include <vcclr.h>

#include <iostream>
#include <string>

using namespace System; // NOLINT
using namespace System::Reflection; // NOLINT
using namespace Runtime::InteropServices; // NOLINT

namespace xwalk {
namespace extensions {

static void MarshalString(String^ input, std::string* output) {
  const char* chars = reinterpret_cast<const char*>(
      Marshal::StringToHGlobalAnsi(input).ToPointer());
  *output = chars;
  Marshal::FreeHGlobal(IntPtr((void*)chars)); //NOLINT
}

public ref class Bridge : public Object {
 public:
  Bridge(XWalkDotNetBridge* bridge)
    : bridge_(bridge),
      instance_(0) {}
  void setNativeInstance(void* instance) {
    instance_ = instance;
  }
  void PostMessageToJS(String^ message) {
    std::string message_to_js;
    MarshalString(message, &message_to_js);
    bridge_->PostMessageToInstance(instance_, message_to_js);
  }
  void SendSyncReply(String^ message) {
    std::string message_to_js;
    MarshalString(message, &message_to_js);
    bridge_->SetSyncReply(instance_, message_to_js);
  }
 private:
  XWalkDotNetBridge* bridge_;
  void* instance_;
};

XWalkDotNetBridge::XWalkDotNetBridge(
    void* extension, const std::wstring& library_path)
    : extension_(extension),
      library_path_(library_path),
      initialized_(false),
      extension_dotnet_(0),
      set_name_callback_(0),
      set_javacript_api_callback_(0) {}

XWalkDotNetBridge::~XWalkDotNetBridge() {
  if (extension_dotnet_) {
    gcroot<Object^> *extension_object_ptr =
        static_cast<gcroot<Object^>*>(extension_dotnet_);
    delete extension_object_ptr;
    extension_dotnet_ = 0;
  }
  if (extension_assembly_) {
    gcroot<Assembly^> *extension_assembly_ptr =
        static_cast<gcroot<Assembly^>*>(extension_assembly_);
    delete extension_assembly_ptr;
    extension_assembly_ = 0;
  }
}

bool XWalkDotNetBridge::Initialize() {
  if (initialized_)
    return true;
  try {
    String^ path = gcnew String(library_path_.c_str());
    Assembly^ extension_assembly = Assembly::LoadFrom(path);
    gcroot<Assembly^> *extension_assembly_ptr =
        new gcroot<Assembly^>(extension_assembly);
    extension_assembly_ = static_cast<void*>(extension_assembly_ptr);
    // Figure out if the XWalkExtension class is present.
    Type^ extension_type =
        extension_assembly->GetType("xwalk.XWalkExtension", true);
    if (!extension_type) {
      std::cerr << "Error loading extension "
                << library_path_.c_str()
                << "': XWalkExtension class is not defined";
      return false;
    }
    ConstructorInfo^ extension_constructor =
        extension_type->GetConstructor(Type::EmptyTypes);
    if (!extension_constructor) {
      std::cerr << "Error loading extension " << library_path_.c_str()
                << "': XWalkExtension constructor is not defined";
      return false;
    }

    MethodInfo^ extension_name_method =
        extension_type->GetMethod("ExtensionName");
    if (!extension_name_method) {
      std::cerr << "Error loading extension " << library_path_.c_str()
                << "': ExtensionName method is not defined";
      return false;
    }

    MethodInfo^ extension_api_method =
        extension_type->GetMethod("ExtensionAPI");
    if (!extension_api_method) {
      std::cerr << "Error loading extension " << library_path_.c_str()
                << "': ExtensionAPI method is not defined";
      return false;
    }

    Object^ extension_object =
        extension_constructor->Invoke(gcnew array<Object^>(0){});
    gcroot<Object^> *extension_object_ptr =
        new gcroot<Object^>(extension_object);
    extension_dotnet_ = static_cast<void*>(extension_object_ptr);

    Object^ extension_name_clr =
        extension_name_method->Invoke(extension_object,
                                      gcnew array<Object^>(0){});
    if (extension_name_clr->GetType() != String::typeid) {
      std::cerr << "Error loading extension " << library_path_.c_str()
                << "': ExtensionName return type is not valid";
      return false;
    }
    std::string extension_name;
    MarshalString(extension_name_clr->ToString(), &extension_name);
    set_name_callback_(extension_, extension_name);

    Object^ api_clr = extension_api_method->Invoke(extension_object,
                                                   gcnew array<Object^>(0){});
    if (api_clr->GetType() != String::typeid) {
      std::cerr << "Error loading extension " << library_path_.c_str()
                << "': ExtensionAPI return type is not valid";
      return false;
    }
    std::string api;
    MarshalString(api_clr->ToString(), &api);
    set_javacript_api_callback_(extension_, api);

    initialized_ = true;
    return true;
  }
  catch (Exception^ e) {
    std::string message;
    MarshalString(e->ToString(), &message);
    std::cerr << "Error loading extension " << library_path_.c_str()
              << "': " << message;
    return false;
  }
}

void XWalkDotNetBridge::set_name_callback(set_name_callback_func callback) {
  set_name_callback_ = callback;
}

void XWalkDotNetBridge::set_javascript_api_callback(
    set_javacript_api_callback_func callback) {
  set_javacript_api_callback_ = callback;
}

void XWalkDotNetBridge::set_post_message_callback(
    post_message_callback_func callback) {
  post_message_callback_ = callback;
}

void XWalkDotNetBridge::set_set_sync_reply_callback(
  set_sync_reply_callback_func callback) {
  set_sync_reply_callback_ = callback;
}

void* XWalkDotNetBridge::CreateInstance(void* native_instance) {
  if (!initialized_)
    return 0;
  try {
    gcroot<Assembly^> *extension_assembly_ptr =
        static_cast<gcroot<Assembly^>*>(extension_assembly_);
    Type^ extension_instance_type =
        ((Assembly^)*extension_assembly_ptr)->GetType(
            "xwalk.XWalkExtensionInstance", true);
    if (!extension_instance_type) {
      std::cerr << "Error creating .NET extension instance "
                << library_path_.c_str()
                << "': XWalkExtensionInstance class is not defined";
      return 0;
    }

    array<Type^>^types = gcnew array<Type^>(1);
    types[0] = Object::typeid;
    ConstructorInfo^ extension_instance_constructor =
        extension_instance_type->GetConstructor(types);
    if (!extension_instance_constructor) {
      std::cerr << "Error loading extension instance " << library_path_.c_str()
                << "': XWalkExtensionInstance constructor is not defined";
      return 0;
    }

    array<Type^>^types_string = gcnew array<Type^>(1);
    types_string[0] = String::typeid;
    MethodInfo^ handle_sync_message_method =
        extension_instance_type->GetMethod("HandleSyncMessage", types_string);
    if (!handle_sync_message_method) {
      std::cerr << "Error loading extension instance " << library_path_.c_str()
                << "': HandleSyncMessage is not defined or has"
                << " an incorrect prototype";
      return 0;
    }

    MethodInfo^ handle_message_method =
        extension_instance_type->GetMethod("HandleMessage", types_string);
    if (!handle_message_method) {
      std::cerr << "Error loading extension instance " << library_path_.c_str()
                << "': HandleMessage is not defined or has"
                << " an incorrect prototype";
      return 0;
    }

    Bridge^ callback = gcnew Bridge(this);
    Object^ extension_instance_object =
        extension_instance_constructor->Invoke(
            gcnew array<Object^>(1){ callback });
    gcroot<Object^> *extension_instance_object_ptr =
        new gcroot<Object^>(extension_instance_object);
    void* extension_instance_dotnet =
        static_cast<void*>(extension_instance_object_ptr);
    callback->setNativeInstance(native_instance);
    return extension_instance_dotnet;
  }
  catch (Exception^ e) {
    std::string message;
    MarshalString(e->ToString(), &message);
    std::cerr << "Error creating extension instance"
              << library_path_.c_str() << "': " << message;
    return 0;
  }
}

void XWalkDotNetBridge::HandleMessage(void* instance,
                                      const std::string& message) {
  String^ message_str = gcnew String(message.c_str());
  gcroot<Object^> *instance_object_ptr =
      static_cast<gcroot<Object^>*>(instance);
  Type^ extension_instance_type = ((Object^)*instance_object_ptr)->GetType();
  MethodInfo^ handle_message_method =
      extension_instance_type->GetMethod("HandleMessage");
  if (!handle_message_method) {
    std::cerr << "Error invoking HandleMessage on extension instance "
              << library_path_.c_str()
              << "': HandleMessage method is not defined";
    return;
  }
  Object^ returnValue = handle_message_method->Invoke(
      ((Object^)*instance_object_ptr), gcnew array<Object^>(1){ message_str });
}

void XWalkDotNetBridge::HandleSyncMessage(void* instance,
                                          const std::string& message) {
  String^ message_str = gcnew String(message.c_str());
  gcroot<Object^> *instance_object_ptr =
      static_cast<gcroot<Object^>*>(instance);
  Type^ extension_instance_type = ((Object^)*instance_object_ptr)->GetType();
  MethodInfo^ handle_sync_message_method =
      extension_instance_type->GetMethod("HandleSyncMessage");
  if (!handle_sync_message_method) {
    std::cerr << "Error invoking HandleSyncMessage on extension instance "
              << library_path_.c_str()
              << "': HandleMessage method is not defined";
    return;
  }
  Object^ returnValue = handle_sync_message_method->Invoke(
      ((Object^)*instance_object_ptr), gcnew array<Object^>(1){ message_str });
}

void XWalkDotNetBridge::PostMessageToInstance(void* instance,
                                              const std::string& message) {
  post_message_callback_(instance, message);
}

void XWalkDotNetBridge::SetSyncReply(void* instance,
                                     const std::string& message) {
  set_sync_reply_callback_(instance, message);
}

}  // namespace extensions
}  // namespace xwalk
