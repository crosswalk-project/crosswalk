// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scim_bridge.h"

#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_libevent.h"
#include "content/public/browser/browser_thread.h"
#include "ui/base/x/x11_util.h"
#include "ui/gfx/x/x11_types.h"

#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_HELPER_MODULE
#define Uses_SCIM_PANEL_CLIENT

#include <scim.h>
#include <x11/scim_x11_utils.h>
#include <isf_imcontrol_client.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xlibint.h>

using namespace scim;

const String kScimFrontendSocketIpc = "socket";
const String kScimDummyConfig = "dummy";
const String kScimSimpleConfig = "simple";
const String kScimConnectionTester = "ConnectionTester";
const String kScimSocketFrontend = "SocketFrontEnd";
const String kScimFactoryEncoding = "UTF-8";
const char* kISFActiveWindowAtom  = "_ISF_ACTIVE_WINDOW";
const char* kWindowAtomType = "WINDOW";

typedef struct {
    int language;
    int layout;
    int return_key_type;
    Window client_window;
    int imdata_size;
    int cursor_pos;
    bool return_key_disabled;
    bool prediction_allow;
    bool password_mode;
    bool caps_mode;
    int layout_variation;
    int reserved[248];
} InputSystemEngineContext;

namespace ui {

class SCIMBridgeImpl : base::MessageLoopForIO::Watcher {

public:

  SCIMBridgeImpl();
  ~SCIMBridgeImpl();

  void Init();
  bool IsInitialized() {return is_initialized_;}
  void ShowInputMethodPanel();
  void HideInputMethodPanel();
  void SetFocusedWindow(unsigned long xid);

private:
  bool IsDaemonRunning();
  void StartDaemon();
  void InitConfigModule();
  void InitBackend();
  void InitPanelClient();
  void ConnectPanelClientSignals();
  void DisconnectPanelClient();
  void InitIMEngineFactory();

  // SCIM PanelClient callbacks.
  void OnUpdateClientId(int context, int client_id);
  void OnProcessHelperEvent(int context,
                              const String &target_uuid,
                              const String &helper_uuid,
                              const Transaction &trans);
  void OnResetKeyboardIse(int context);
  void OnHidePreeditString(int context);
  void OnProcessKeyEvent(int context, const KeyEvent &key);
  void OnCommitString(int context, const WideString &wstr);

  void SendXKeyEvent(const KeyEvent &key);
  void SetInputMethodActiveWindow();

  void OnFileCanReadWithoutBlocking(int fd) OVERRIDE;
  void OnFileCanWriteWithoutBlocking(int fd) OVERRIDE {}
  void WatchSCIMFd();
  void HandleScimEvent();

private:
  bool is_initialized_;
  std::string language_code_;
  String config_module_name_;
  scoped_ptr<ConfigModule> config_module_;
  ConfigPointer config_;
  std::vector<String> config_list_;
  std::vector<String> scim_modules_;
  BackEndPointer backend_;
  scoped_ptr<PanelClient> panel_client_;
  int panel_client_id_;
  int panel_client_fd_;
  base::MessageLoopForIO::FileDescriptorWatcher read_fd_controller_;
  IMEngineInstancePointer im_engine_instance_;
  scoped_ptr<IMControlClient> imcontrol_client_;
  InputSystemEngineContext ise_context_;
  unsigned int im_context_id_;
  int im_engine_instance_count_;

  Display* display_;
  Window window_;
  Window root_window_;

  DISALLOW_COPY_AND_ASSIGN(SCIMBridgeImpl);
};

SCIMBridge::SCIMBridge() : impl_(new SCIMBridgeImpl()) {
}

SCIMBridge::~SCIMBridge() {
}

void SCIMBridge::Init() {
  if (impl_->IsInitialized())
    return;

  if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
    content::BrowserThread::PostTask(content::BrowserThread::IO,
                            FROM_HERE,
                            base::Bind(&SCIMBridgeImpl::Init,
                                       base::Unretained(impl_.get())));
  }
}

void SCIMBridge::SetFocusedWindow(unsigned long xid) {
  impl_->SetFocusedWindow(xid);
}

void SCIMBridge::TextInputChanged(ui::TextInputType type) {
  // TODO: set proper ise_context_ fields according to type
  if (type == ui::TEXT_INPUT_TYPE_NONE) {
    content::BrowserThread::PostTask(content::BrowserThread::IO,
                            FROM_HERE,
                            base::Bind(&SCIMBridgeImpl::HideInputMethodPanel,
                                       base::Unretained(impl_.get())));

  } else {
    content::BrowserThread::PostTask(content::BrowserThread::IO,
                            FROM_HERE,
                            base::Bind(&SCIMBridgeImpl::ShowInputMethodPanel,
                                       base::Unretained(impl_.get())));
  }
}

SCIMBridgeImpl::SCIMBridgeImpl()
  : is_initialized_(false),
    config_module_name_(kScimSimpleConfig),
    panel_client_id_(0),
    panel_client_fd_(-1),
    im_context_id_(getpid() % 50000),
    im_engine_instance_count_(0),
    display_(gfx::GetXDisplay()),
    root_window_(GetX11RootWindow())
{
  // Create input system engine context data that would be sent to the daemon.
  // TODO: Add SCIM enums.
  ise_context_.language = 0;
  ise_context_.layout = 0;
  ise_context_.return_key_type = 0;
  ise_context_.imdata_size = 0;
  ise_context_.return_key_disabled = false;
  ise_context_.prediction_allow = true;
  ise_context_.password_mode = false;
  ise_context_.caps_mode = false;
  ise_context_.layout_variation = 0;
  ise_context_.return_key_disabled = false;
}

SCIMBridgeImpl::~SCIMBridgeImpl() {
  if (panel_client_)
    DisconnectPanelClient();
}

void SCIMBridgeImpl::Init() {
  language_code_ = scim_get_locale_language(scim_get_current_locale());
  imcontrol_client_.reset(new IMControlClient());
  imcontrol_client_->open_connection();

  StartDaemon();
  InitConfigModule();
  InitBackend();
  InitIMEngineFactory();
  InitPanelClient();
  ConnectPanelClientSignals();
  is_initialized_ = true;
}

void SCIMBridgeImpl::ShowInputMethodPanel() {
  int ise_packet_length = sizeof(ise_context_);
  void* ise_packet = calloc(1, ise_packet_length);
  memcpy(ise_packet, (void *)&ise_context_, ise_packet_length);

  panel_client_->prepare(im_context_id_);
  panel_client_->focus_in(im_context_id_,
                          im_engine_instance_->get_factory_uuid());
  panel_client_->send();

  imcontrol_client_->prepare();
  int input_panel_show = 0;
  imcontrol_client_->show_ise(panel_client_id_, im_context_id_, ise_packet,
      ise_packet_length, &input_panel_show);
  imcontrol_client_->send();

  free(ise_packet);
}

void SCIMBridgeImpl::HideInputMethodPanel() {
  imcontrol_client_->prepare();
  imcontrol_client_->hide_ise(panel_client_id_, im_context_id_);
  imcontrol_client_->send();
}

bool SCIMBridgeImpl::IsDaemonRunning() {
  uint32_t uid;
  SocketClient client;
  SocketAddress address;
  address.set_address(scim_get_default_socket_frontend_address());

  if (!client.connect(address))
      return false;

  return scim_socket_open_connection(uid, kScimConnectionTester,
                                     kScimSocketFrontend, client, 1000);
}

void SCIMBridgeImpl::StartDaemon() {
  if (!IsDaemonRunning()) {
    std::vector<String> helper_modules;
    std::vector<String> imengine_modules;

    scim_get_imengine_module_list(imengine_modules);
    scim_get_helper_module_list(helper_modules);

    std::vector<String>::iterator it;

    for (it = imengine_modules.begin(); it != imengine_modules.end(); it++) {
      if (*it != kScimFrontendSocketIpc)
          scim_modules_.push_back(*it);
    }

    for (it = helper_modules.begin(); it != helper_modules.end(); it++)
      scim_modules_.push_back(*it);

    const String engines =
        (scim_modules_.size() > 0 ? scim_combine_string_list(scim_modules_, ',')
                                  : "none");
    const char *argv [] = { "--no-stay", 0 };
    scim_launch(true, config_module_name_, engines, kScimFrontendSocketIpc,
        (char **)argv);
  }
}

void SCIMBridgeImpl::InitConfigModule() {
  config_module_.reset(new ConfigModule(config_module_name_));
  if (config_module_->valid()) {
      config_ = config_module_->create_config();
  } else {
    LOG(WARNING) << "Cannot init scim config module: " << config_module_name_;
    config_module_.reset();
    config_ = new DummyConfig();
    config_module_name_ = kScimDummyConfig;
  }
}

void SCIMBridgeImpl::InitBackend() {
  backend_ = new CommonBackEnd(config_, scim_modules_);
  if (!backend_.null()) {
    backend_->initialize(config_, scim_modules_, false, false);
  } else {
    LOG(ERROR) << "Cannot create backend";
  }
}

void SCIMBridgeImpl::InitPanelClient() {
  if (!panel_client_.get())
    panel_client_.reset(new PanelClient());

  if (panel_client_->open_connection(config_->get_name(),
                                     display_->display_name) >= 0) {
      panel_client_fd_ = panel_client_->get_connection_number();
      content::BrowserThread::PostTask(content::BrowserThread::IO,
                              FROM_HERE,
                              base::Bind(&SCIMBridgeImpl::WatchSCIMFd,
                                         base::Unretained(this)));

      panel_client_->prepare(im_context_id_);
      panel_client_->register_input_context(im_context_id_,
                                      im_engine_instance_->get_factory_uuid());
      panel_client_->send();
  } else {
    LOG(ERROR) << "Cannot get panel client connection";
  }
}

void SCIMBridgeImpl::ConnectPanelClientSignals() {
  // FIXME: attach rest of the signals
  panel_client_->signal_connect_update_client_id(
      slot(this, &SCIMBridgeImpl::OnUpdateClientId));
  panel_client_->signal_connect_process_helper_event(
      slot(this, &SCIMBridgeImpl::OnProcessHelperEvent));
  panel_client_->signal_connect_reset_keyboard_ise(
      slot(this, &SCIMBridgeImpl::OnResetKeyboardIse));
  panel_client_->signal_connect_hide_preedit_string(
      slot(this, &SCIMBridgeImpl::OnHidePreeditString));
  panel_client_->signal_connect_process_key_event(
      slot(this, &SCIMBridgeImpl::OnProcessKeyEvent));
  panel_client_->signal_connect_commit_string(
      slot(this, &SCIMBridgeImpl::OnCommitString));
}

void SCIMBridgeImpl::DisconnectPanelClient() {
  panel_client_->close_connection();
  read_fd_controller_.StopWatchingFileDescriptor();
}

void SCIMBridgeImpl::InitIMEngineFactory() {
  IMEngineFactoryPointer factory =
      backend_->get_default_factory(language_code_, kScimFactoryEncoding);
  if (factory.null()) {
      LOG(ERROR) << "Cannot get default IM engine factory";
      return;
  }

  im_engine_instance_ = factory->create_instance(kScimFactoryEncoding,
      im_engine_instance_count_++);

  if (im_engine_instance_.null()) {
     LOG(ERROR) << "Cannot get default IM engine instance";
     return;
  }

  // TODO: connect im_engine_instance_ signals
}

void SCIMBridgeImpl::OnUpdateClientId(int, int client_id) {
  panel_client_id_ = client_id;
}

void SCIMBridgeImpl::OnProcessHelperEvent(int context,
                                        const String &target_uuid,
                                        const String &helper_uuid,
                                        const Transaction &trans) {
  if (im_engine_instance_->get_factory_uuid() == target_uuid) {
    panel_client_->prepare(im_context_id_);
    im_engine_instance_->process_helper_event(helper_uuid, trans);
    panel_client_->send();
  }
}

void SCIMBridgeImpl::OnResetKeyboardIse(int context) {
  // TODO: implement
  LOG(INFO) << "SCIMBridgeImpl::OnResetKeyboardIse - not implemented";
}

void SCIMBridgeImpl::OnHidePreeditString(int context) {
  // TODO: implement
  LOG(INFO) << "SCIMBridgeImpl::OnHidePreeditString - not implemented";
}

void SCIMBridgeImpl::OnProcessKeyEvent(int context, const KeyEvent &key) {
  content::BrowserThread::PostTask(content::BrowserThread::UI,
                          FROM_HERE,
                          base::Bind(&SCIMBridgeImpl::SendXKeyEvent,
                                     base::Unretained(this), key));
}

void SCIMBridgeImpl::OnCommitString(int context, const WideString &wstr) {
  // TODO: implement
  LOG(INFO) << "SCIMBridgeImpl::OnCommitString - not implemented";
}

void SCIMBridgeImpl::SendXKeyEvent(const KeyEvent &key) {
  ::KeyCode keycode = 0;
  ::KeySym keysym = 0;
  XKeyEvent event = scim_x11_keyevent_scim_to_x11(display_, key);
  event.same_screen = True;
  event.window = window_;
  keysym = XStringToKeysym(key.get_key_string().c_str());
  if (keysym == NoSymbol)
    return;
  keycode = XKeysymToKeycode(display_, keysym);
  if (XkbKeycodeToKeysym(display_, keycode, 0, 1) == keysym)
    event.state |= ShiftMask;
  XSendEvent(display_, window_, True, event.type, (XEvent *)&event);
}

void SCIMBridgeImpl::SetInputMethodActiveWindow() {
  SetIntProperty(root_window_, kISFActiveWindowAtom, kWindowAtomType, window_);
}

void SCIMBridgeImpl::SetFocusedWindow(unsigned long xid) {
  window_ = xid;
  ise_context_.client_window = xid;
  SetInputMethodActiveWindow();
}

void SCIMBridgeImpl::OnFileCanReadWithoutBlocking(int fd) {
  if (panel_client_fd_ == fd)
    HandleScimEvent();
}

void SCIMBridgeImpl::HandleScimEvent() {
  if (panel_client_->has_pending_event()
      && !panel_client_->filter_event()) {
        LOG(INFO) << "Reconnecting scim PanelClient";
        DisconnectPanelClient();
        InitPanelClient();
   }
}

void SCIMBridgeImpl::WatchSCIMFd() {
  base::MessageLoopForIO::current()->WatchFileDescriptor(
          panel_client_fd_, true, base::MessageLoopForIO::WATCH_READ,
          &read_fd_controller_, this);
}

}
