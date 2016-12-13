// Copyright (c) 2015 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/dotnet/xwalk_dotnet_bridge.h"

#include <Objbase.h>
#include <vcclr.h>

#include <assert.h>
#include <iostream>

using namespace System; // NOLINT
using namespace System::IO; // NOLINT
using namespace System::Reflection; // NOLINT
using namespace System::Text; // NOLINT
using namespace Runtime::InteropServices; // NOLINT

namespace {

xwalk::extensions::XWalkDotNetBridge* g_bridge = nullptr;
XW_Extension g_xw_extension = 0;
const XW_CoreInterface* g_core = nullptr;
const XW_MessagingInterface* g_messaging = nullptr;
const XW_MessagingInterface2* g_messaging2 = nullptr;
const XW_Internal_SyncMessagingInterface* g_sync_messaging = nullptr;
const XW_Internal_EntryPointsInterface* g_entry_points = nullptr;
const XW_Internal_RuntimeInterface* g_runtime = nullptr;
const XW_Internal_PermissionsInterface* g_permission = nullptr;

typedef void* XWalkExtensionDotNet;
typedef void* XWalkExtensionAssembly;

static void MarshalString(String^ input, std::string* output) {
  const char* chars = reinterpret_cast<const char*>(
    Marshal::StringToHGlobalAnsi(input).ToPointer());
  *output = chars;
  Marshal::FreeHGlobal(IntPtr((void*)chars)); //NOLINT
}

bool InitializeInterfaces(XW_GetInterface get_interface) {
  g_core = reinterpret_cast<const XW_CoreInterface*>(
    get_interface(XW_CORE_INTERFACE));
  if (!g_core) {
    std::cerr << "Can't initialize extension: error getting Core interface.\n";
    return false;
  }

  g_messaging = reinterpret_cast<const XW_MessagingInterface*>(
      get_interface(XW_MESSAGING_INTERFACE));
  if (!g_messaging) {
    std::cerr <<
        "Can't initialize extension: error getting Messaging interface.\n";
    return false;
  }

  g_messaging2 = reinterpret_cast<const XW_MessagingInterface2*>(
    get_interface(XW_MESSAGING_INTERFACE_2));
  if (!g_messaging2) {
    std::cerr <<
        "Can't initialize extension: error getting binary Messaging interface.\n";
    return false;
  }

  g_sync_messaging =
      reinterpret_cast<const XW_Internal_SyncMessagingInterface*>(
      get_interface(XW_INTERNAL_SYNC_MESSAGING_INTERFACE));
  if (!g_sync_messaging) {
    std::cerr <<
        "Can't initialize extension: error getting SyncMessaging interface.\n";
    return false;
  }

  g_entry_points = reinterpret_cast<const XW_Internal_EntryPointsInterface*>(
      get_interface(XW_INTERNAL_ENTRY_POINTS_INTERFACE));
  if (!g_entry_points) {
    std::cerr << "NOTE: Entry points interface not available in this version "
              << "of Crosswalk, ignoring entry point data for extensions.\n";
  }

  g_runtime = reinterpret_cast<const XW_Internal_RuntimeInterface*>(
      get_interface(XW_INTERNAL_RUNTIME_INTERFACE));
  if (!g_runtime) {
    std::cerr << "NOTE: runtime interface not available in this version "
              << "of Crosswalk, ignoring runtime variables for extensions.\n";
  }

  g_permission = reinterpret_cast<const XW_Internal_PermissionsInterface*>(
      get_interface(XW_INTERNAL_PERMISSIONS_INTERFACE));
  if (!g_permission) {
    std::cerr << "NOTE: permission interface not available in this version "
              << "of Crosswalk, ignoring permission for extensions.\n";
  }

  return true;
}
}  // anonymous namespace

int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
  assert(extension);
  g_xw_extension = extension;

  if (!InitializeInterfaces(get_interface))
    return XW_ERROR;

  g_bridge = new xwalk::extensions::XWalkDotNetBridge();
  if (!g_bridge->Initialize()) {
    std::cerr << "Can't initialize .NET extension bridge \n";
    return XW_ERROR;
  }
  using namespace xwalk::extensions; // NOLINT
  g_core->RegisterShutdownCallback(
      g_xw_extension, XWalkDotNetBridge::OnShutdown);
  g_core->RegisterInstanceCallbacks(
    g_xw_extension, XWalkDotNetBridge::OnInstanceCreated,
    XWalkDotNetBridge::OnInstanceDestroyed);
  g_messaging->Register(
      g_xw_extension, XWalkDotNetBridge::HandleMessage);
  g_sync_messaging->Register(
      g_xw_extension, XWalkDotNetBridge::HandleSyncMessage);
  return XW_OK;
}

namespace xwalk {
namespace extensions {

struct PrivateBridgeData {
  PrivateBridgeData()
    : initialized_(false),
      extension_dotnet_(nullptr),
      extension_assembly_(nullptr) {
  }
  std::string library_path_;
  bool initialized_;
  XWalkExtensionDotNet extension_dotnet_;
  XWalkExtensionAssembly extension_assembly_;
};

public ref class Bridge : public Object {
public:
  explicit Bridge(XWalkDotNetBridge* bridge)
    : bridge_(bridge),
    instance_(0) {
  }
  void setNativeInstance(XW_Instance instance) {
    instance_ = instance;
  }
  void PostBinaryMessageToJS(array<unsigned char>^ message, size_t size) {
    pin_ptr<unsigned char> message_pin = &message[0];
    const unsigned char* message_ptr = message_pin;
    const char* message_to_js = (const char*)(message_ptr);

    bridge_->PostBinaryMessageToInstance(instance_, message_to_js, size);
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
  XW_Instance instance_;
};

XWalkDotNetBridge::XWalkDotNetBridge()
  : private_data_(new PrivateBridgeData) {
}

void XWalkDotNetBridge::OnShutdown(XW_Extension) {
  delete g_bridge;
  g_bridge = nullptr;
}

XWalkDotNetBridge::~XWalkDotNetBridge() {
  if (private_data_->extension_dotnet_) {
    gcroot<Object^> *extension_object_ptr =
        static_cast<gcroot<Object^>*>(private_data_->extension_dotnet_);
    delete extension_object_ptr;
    private_data_->extension_dotnet_ = nullptr;
  }
  if (private_data_->extension_assembly_) {
    gcroot<Assembly^> *extension_assembly_ptr =
        static_cast<gcroot<Assembly^>*>(private_data_->extension_assembly_);
    delete extension_assembly_ptr;
    private_data_->extension_assembly_ = nullptr;
  }
  delete private_data_;
}

bool XWalkDotNetBridge::Initialize() {
  if (private_data_->initialized_)
    return true;
  try {
    char extension_path[4096];
    g_runtime->GetRuntimeVariableString(g_xw_extension,
        "extension_path",
        extension_path,
        sizeof(extension_path));
    String^ path = gcnew String(extension_path);
    if (path->Length == 0) {
      std::cerr << "Error loading extension "
                << extension_path
                << " : path to the bridge doesn't exist";
      return false;
    }

    // Remove the quotes added by the JSON framework.
    path = path->Replace("\"", "");

    // We load the .NET extension with the same name but without the _bridge
    // suffix.
    path = path->Replace("_bridge", "");

    MarshalString(path, &private_data_->library_path_);
    Assembly^ extension_assembly = Assembly::LoadFrom(path);
    gcroot<Assembly^> *extension_assembly_ptr =
        new gcroot<Assembly^>(extension_assembly);
    private_data_->extension_assembly_ =
        static_cast<XWalkExtensionAssembly>(extension_assembly_ptr);
    // Figure out if the XWalkExtension class is present.
    Type^ extension_type =
        extension_assembly->GetType("xwalk.XWalkExtension", true);
    if (!extension_type) {
      std::cerr << "Error loading extension " << private_data_->library_path_
                << "': XWalkExtension class is not defined";
      return false;
    }
    ConstructorInfo^ extension_constructor =
        extension_type->GetConstructor(Type::EmptyTypes);
    if (!extension_constructor) {
      std::cerr << "Error loading extension " << private_data_->library_path_
                << "': XWalkExtension constructor is not defined";
      return false;
    }

    MethodInfo^ extension_name_method =
        extension_type->GetMethod("ExtensionName");
    if (!extension_name_method) {
      std::cerr << "Error loading extension " << private_data_->library_path_
                << "': ExtensionName method is not defined";
      return false;
    }

    MethodInfo^ extension_api_method =
        extension_type->GetMethod("ExtensionAPI");
    if (!extension_api_method) {
      std::cerr << "Error loading extension " << private_data_->library_path_
                << "': ExtensionAPI method is not defined";
      return false;
    }

    Object^ extension_object =
        extension_constructor->Invoke(gcnew array<Object^>(0){});
    gcroot<Object^> *extension_object_ptr =
        new gcroot<Object^>(extension_object);
    private_data_->extension_dotnet_ =
        static_cast<XWalkExtensionDotNet>(extension_object_ptr);

    Object^ extension_name_clr =
        extension_name_method->Invoke(extension_object,
        gcnew array<Object^>(0){});
    if (extension_name_clr->GetType() != String::typeid) {
      std::cerr << "Error loading extension " << private_data_->library_path_
                << "': ExtensionName return type is not valid";
      return false;
    }
    std::string extension_name;
    MarshalString(extension_name_clr->ToString(), &extension_name);
    g_core->SetExtensionName(g_xw_extension, extension_name.c_str());

    Object^ api_clr = extension_api_method->Invoke(extension_object,
        gcnew array<Object^>(0){});
    if (api_clr->GetType() != String::typeid) {
      std::cerr << "Error loading extension " << private_data_->library_path_
                << "': ExtensionAPI return type is not valid";
      return false;
    }
    std::string api;
    MarshalString(api_clr->ToString(), &api);
    g_core->SetJavaScriptAPI(g_xw_extension, api.c_str());

    private_data_->initialized_ = true;
    return true;
  }
  catch (Exception^ e) {
    std::string message;
    MarshalString(e->ToString(), &message);
    std::cerr << "Error loading extension " << private_data_->library_path_
              << "': " << message;
    return false;
  }
}

XWalkExtensionDotNetInstance XWalkDotNetBridge::CreateInstance(
    XW_Instance native_instance) {
  if (!private_data_->initialized_)
    return nullptr;
  try {
    gcroot<Assembly^> *extension_assembly_ptr =
        static_cast<gcroot<Assembly^>*>(private_data_->extension_assembly_);
    Type^ extension_instance_type =
        ((Assembly^)*extension_assembly_ptr)->GetType(
        "xwalk.XWalkExtensionInstance", true);
    if (!extension_instance_type) {
      std::cerr << "Error creating .NET extension instance "
                << private_data_->library_path_
                << "': XWalkExtensionInstance class is not defined";
      return nullptr;
    }

    array<Type^>^types = gcnew array<Type^>(1);
    types[0] = Object::typeid;
    ConstructorInfo^ extension_instance_constructor =
        extension_instance_type->GetConstructor(types);
    if (!extension_instance_constructor) {
      std::cerr << "Error loading extension instance "
                << private_data_->library_path_
                << "': XWalkExtensionInstance constructor is not defined";
      return nullptr;
    }

    array<Type^>^types_string = gcnew array<Type^>(1);
    types_string[0] = String::typeid;
    MethodInfo^ handle_sync_message_method =
        extension_instance_type->GetMethod("HandleSyncMessage", types_string);
    if (!handle_sync_message_method) {
      std::cerr << "Error loading extension instance "
                << private_data_->library_path_
                << "': HandleSyncMessage is not defined or has"
                << " an incorrect prototype";
      return nullptr;
    }

    MethodInfo^ handle_message_method =
        extension_instance_type->GetMethod("HandleMessage", types_string);
    if (!handle_message_method) {
      std::cerr << "Error loading extension instance "
                << private_data_->library_path_
                << "': HandleMessage is not defined or has"
                << " an incorrect prototype";
      return nullptr;
    }

    Bridge^ callback = gcnew Bridge(this);
    Object^ extension_instance_object =
        extension_instance_constructor->Invoke(
        gcnew array<Object^>(1){ callback });
    gcroot<Object^> *extension_instance_object_ptr =
        new gcroot<Object^>(extension_instance_object);
    XWalkExtensionDotNetInstance extension_instance_dotnet =
        static_cast<XWalkExtensionDotNet>(extension_instance_object_ptr);
    callback->setNativeInstance(native_instance);
    return extension_instance_dotnet;
  }
  catch (Exception^ e) {
    std::string message;
    MarshalString(e->ToString(), &message);
    std::cerr << "Error creating extension instance"
              << private_data_->library_path_ << "': " << message;
    return nullptr;
  }
}

void XWalkDotNetBridge::OnInstanceCreated(XW_Instance xw_instance) {
  assert(!g_core->GetInstanceData(xw_instance));
  XWalkExtensionDotNetInstance instance =
      g_bridge->CreateInstance(xw_instance);
  if (!instance)
    return;
  g_core->SetInstanceData(xw_instance, instance);
}

// static
void XWalkDotNetBridge::OnInstanceDestroyed(XW_Instance xw_instance) {
  XWalkExtensionDotNetInstance instance =
      g_core->GetInstanceData(xw_instance);
  if (!instance)
    return;
  gcroot<Object^> *instance_object_ptr =
      static_cast<gcroot<Object^>*>(instance);
  delete instance_object_ptr;
}

void XWalkDotNetBridge::HandleMessage(XW_Instance instance,
  const char* message) {
  XWalkExtensionDotNetInstance instance_dot_net =
      g_core->GetInstanceData(instance);
  String^ message_str = gcnew String(message);
  gcroot<Object^> *instance_object_ptr =
      static_cast<gcroot<Object^>*>(instance_dot_net);
  Type^ extension_instance_type = ((Object^)*instance_object_ptr)->GetType();
  MethodInfo^ handle_message_method =
      extension_instance_type->GetMethod("HandleMessage");
  if (!handle_message_method) {
    std::cerr << "Error invoking HandleMessage on extension instance "
              << "': HandleMessage method is not defined";
    return;
  }
  handle_message_method->Invoke(
      ((Object^)*instance_object_ptr), gcnew array<Object^>(1){ message_str });
}

void XWalkDotNetBridge::HandleSyncMessage(XW_Instance instance,
  const char* message) {
  XWalkExtensionDotNetInstance instance_dot_net =
      g_core->GetInstanceData(instance);
  String^ message_str = gcnew String(message);
  gcroot<Object^> *instance_object_ptr =
      static_cast<gcroot<Object^>*>(instance_dot_net);
  Type^ extension_instance_type = ((Object^)*instance_object_ptr)->GetType();
  MethodInfo^ handle_sync_message_method =
      extension_instance_type->GetMethod("HandleSyncMessage");
  if (!handle_sync_message_method) {
    std::cerr << "Error invoking HandleSyncMessage on extension instance "
              << "': HandleMessage method is not defined";
    return;
  }
  handle_sync_message_method->Invoke(
      ((Object^)*instance_object_ptr), gcnew array<Object^>(1){ message_str });
}

#undef PostMessage
void XWalkDotNetBridge::PostBinaryMessageToInstance(XW_Instance instance,
  const char* message,
  const size_t size) {
  g_messaging2->PostBinaryMessage(instance, message, size);
}

void XWalkDotNetBridge::PostMessageToInstance(XW_Instance instance,
  const std::string& message) {
  g_messaging->PostMessage(instance, message.c_str());
}

void XWalkDotNetBridge::SetSyncReply(XW_Instance instance,
  const std::string& reply) {
  g_sync_messaging->SetSyncReply(instance, reply.c_str());
}

}  // namespace extensions
}  // namespace xwalk
