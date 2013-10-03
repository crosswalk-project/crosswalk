// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/ime/tizen-scim/scim_bridge_x11.h"

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xlibint.h>

// Get rid of a macro from Xlib.h that conflicts with Aura's RootWindow class
#undef RootWindow
// Get rid of a macro from Xlibint.h that conflicts with SCIM.
#undef max
#undef min

#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_HELPER_MODULE
#define Uses_SCIM_PANEL_CLIENT

#include <scim.h>
#include <scim_config_path.h>
#include <x11/scim_x11_utils.h>
#include <isf_imcontrol_client.h>

#include <string>
#include <vector>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/message_loop/message_loop.h"
#include "base/message_loop/message_pump_libevent.h"
#include "content/public/browser/browser_thread.h"
#include "ui/aura/root_window.h"
#include "ui/events/event.h"
#include "ui/events/event_utils.h"
#include "ui/base/ime/text_input_client.h"
#include "ui/base/x/x11_util.h"
#include "ui/gfx/x/x11_types.h"
#include "xwalk/ime/tizen-scim/input_method_scim_x11.h"

using scim::String;
using scim::IMEngineInstanceBase;
using scim::IMEngineInstancePointer;
using scim::WideString;
using scim::AttributeList;
using scim::Transaction;
using scim::LookupTable;
using scim::IMEngineFactoryPointer;
using scim::Property;
using scim::PropertyList;
using scim::ConfigPointer;
using scim::BackEndPointer;
using scim::PanelClient;
using scim::IMControlClient;
using scim::ConfigModule;
using scim::SocketClient;
using scim::SocketAddress;
using scim::DummyConfig;
using scim::CommonBackEnd;

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

static ui::CompositionText toCompositionText(const WideString &wstr) {
  ui::CompositionText composition_text;
  scim::String utf8_str = scim::utf8_wcstombs(wstr);
  composition_text.text = base::UTF8ToUTF16(utf8_str);
  composition_text.selection = gfx::Range(1, 1);
  return composition_text;
}

namespace ui {

class SCIMBridgeImpl : base::MessageLoopForIO::Watcher {
 public:
  explicit SCIMBridgeImpl(InputMethodSCIM* input_method);
  ~SCIMBridgeImpl();

  void Init();
  bool IsInitialized() {return is_initialized_;}
  void ShowInputMethodPanel();
  void HideInputMethodPanel();
  void SetFocusedWindow(XID xid);
  void SetTextInputClient(TextInputClient* client);
  void DispatchNativeKeyEvent(const base::NativeEvent& native_event);

 private:
  bool IsDaemonRunning();
  void StartDaemon();
  void InitConfigModule();
  void InitBackend();
  void InitPanelClient();
  void ConnectPanelClientSignals();
  void DisconnectPanelClient();
  void InitIMEngineFactory();
  void SetFocus(bool focused);
  void RegisterIMEFactory(const IMEngineFactoryPointer& factory);

  // SCIM PanelClient callbacks.
  void OnUpdateClientId(int context, int client_id);
  void OnProcessHelperEvent(int context,
                            const String &target_uuid,
                            const String &helper_uuid,
                            const Transaction &trans);
  void OnResetKeyboardIse(int context);
  void OnProcessKeyEvent(int context, const scim::KeyEvent& key);
  void OnReloadConfig(int context);
  void OnExit(int context);
  void OnUpdateCandidateItemLayout(int context,
                                   const std::vector<unsigned int> &items);
  void OnUpdateLookupTablePageSize(int context, int page_size);
  void OnLookupTablePageUp(int context);
  void OnLookupTablePageDown(int context);
  void OnTriggerProperty(int context, const String &property);
  void OnMovePreeditCaret(int context, int caret_pos);
  void OnUpdatePreeditCaret(int context, int caret);
  void OnSelectAux(int context, int aux_index);
  void OnSelectCandidate(int context, int candidate);
  void OnForwardKeyEvent(int context, const scim::KeyEvent& key);
  void OnRequestHelp(int context);
  void OnRequestFactoryMenu(int context);
  void OnChangeFactory(int context, const String &uuid);
  void OnUpdateKeyboardIse(int context);
  void OnCommitString(int context, const WideString &wstr);
  void OnUpdatePreeditString(int context,
                             const WideString &wstr,
                             const AttributeList &attrs);
  void OnGetSurroundingText(int context, int maxlen_before, int maxlen_after);
  void OnDeleteSurroundingText(int context, int offset, int len);

  void SendXKeyEvent(const scim::KeyEvent& key, bool synthetic);
  void SetInputMethodActiveWindow();
  void DispatchSCIMKeyEvent(const scim::KeyEvent& event);
  void DispatchKeyEventPostIME(const scim::KeyEvent& event);

  void OnFileCanReadWithoutBlocking(int fd) OVERRIDE;
  void OnFileCanWriteWithoutBlocking(int fd) OVERRIDE {}
  void WatchSCIMFd();
  void HandleScimEvent();

  // Connect Input Method Engine signals to SCIMBridgeImpl instance.
  void ConnectIMESignals();

  void OnIMEShowAuxString(IMEngineInstanceBase* ime);
  void OnIMEShowLookupTable(IMEngineInstanceBase* ime);
  void OnIMEHidePreeditString(IMEngineInstanceBase* ime);
  void OnIMEHideAuxString(IMEngineInstanceBase* ime);
  void OnIMEHideLookupTable(IMEngineInstanceBase* ime);
  void OnIMEUpdatePreeditCaret(IMEngineInstanceBase* ime, int caret);
  void OnIMEUpdatePreeditString(IMEngineInstanceBase* ime,
                                const WideString &str,
                                const AttributeList &attrs);
  void OnIMEUpdateAuxString(IMEngineInstanceBase* ime, const WideString &str,
                            const AttributeList &attrs);
  void OnIMECommitString(IMEngineInstanceBase* ime, const WideString &str);
  void OnIMEForwardKeyEvent(IMEngineInstanceBase* ime,
                            const scim::KeyEvent& key);
  void OnIMEUpdateLookupTable(IMEngineInstanceBase* ime,
                              const LookupTable &table);
  void OnIMERegisterProperties(IMEngineInstanceBase* ime,
                               const PropertyList &properties);
  void OnIMEUpdateProperty(IMEngineInstanceBase* ime, const Property &property);
  void OnIMEBeep(IMEngineInstanceBase* ime);
  void OnIMEStartHelper(IMEngineInstanceBase* ime, const String &uuid);
  void OnIMEStopHelper(IMEngineInstanceBase* ime, const String &uuid);
  void OnIMESendHelperEvent(IMEngineInstanceBase* ime, const String &uuid,
                            const Transaction &trans);
  bool OnIMEGetSurroundingText(IMEngineInstanceBase* ime,
                               WideString& text,
                               int& cursor,
                               int maxlen_before,
                               int maxlen_after);
  bool OnIMEDeleteSurroundingText(IMEngineInstanceBase* ime,
                                  int offset,
                                  int len);
  void OnIMEExpandCandidate(IMEngineInstanceBase* ime);
  void OnIMEContractCandidate(IMEngineInstanceBase* ime);
  void OnIMESetCandidateStyle(IMEngineInstanceBase* ime,
                              scim::ISF_CANDIDATE_PORTRAIT_LINE_T line,
                              scim::ISF_CANDIDATE_MODE_T mode);

 private:
  InputMethodSCIM* input_method_;
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
  IMEngineInstancePointer ime_instance_;
  scoped_ptr<IMControlClient> imcontrol_client_;
  InputSystemEngineContext ise_context_;
  int im_context_id_;
  int ime_instance_count_;

  Display* display_;
  Window window_;
  Window root_window_;

  TextInputClient* client_;

  DISALLOW_COPY_AND_ASSIGN(SCIMBridgeImpl);
};

SCIMBridge::SCIMBridge(InputMethodSCIM* input_method)
  : impl_(new SCIMBridgeImpl(input_method)) {
}

SCIMBridge::~SCIMBridge() {
}

void SCIMBridge::SetTextInputClient(TextInputClient* client) {
  impl_->SetTextInputClient(client);

  if (!client)
    return;

  if (!impl_->IsInitialized() && client->GetAttachedWindow()) {
    aura::RootWindow* rw = client->GetAttachedWindow()->GetRootWindow();

    if (!rw)
      return;

    impl_->SetFocusedWindow(rw->GetAcceleratedWidget());
    if (!content::BrowserThread::CurrentlyOn(content::BrowserThread::IO)) {
      content::BrowserThread::PostTask(content::BrowserThread::IO,
                                       FROM_HERE,
                                       base::Bind(&SCIMBridgeImpl::Init,
                                       base::Unretained(impl_.get())));
    }
  }
}

void SCIMBridge::OnTextInputChanged(ui::TextInputType type) {
  // TODO(shalamov): Set proper ise_context_ fields according to type.
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

void SCIMBridge::DispatchKeyEvent(const base::NativeEvent& native_event) {
  impl_->DispatchNativeKeyEvent(native_event);
}

SCIMBridgeImpl::SCIMBridgeImpl(InputMethodSCIM* input_method)
  : input_method_(input_method),
    is_initialized_(false),
    config_module_name_(kScimSimpleConfig),
    panel_client_id_(0),
    panel_client_fd_(-1),
    im_context_id_(getpid() % 50000),
    ime_instance_count_(0),
    display_(gfx::GetXDisplay()),
    root_window_(GetX11RootWindow()),
    client_(0) {
  // Create input system engine context data that would be sent to the daemon.
  // TODO(shalamov): Add SCIM enums.
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
  language_code_ =
      scim::scim_get_locale_language(scim::scim_get_current_locale());
  imcontrol_client_.reset(new IMControlClient());
  imcontrol_client_->open_connection();

  StartDaemon();
  InitConfigModule();
  InitBackend();
  InitPanelClient();
  InitIMEngineFactory();
  ConnectPanelClientSignals();
  is_initialized_ = true;
}

void SCIMBridgeImpl::ShowInputMethodPanel() {
  int ise_packet_length = sizeof(ise_context_);
  void* ise_packet = calloc(1, ise_packet_length);
  memcpy(ise_packet, reinterpret_cast<void*>(&ise_context_), ise_packet_length);

  SetFocus(true);

  imcontrol_client_->prepare();
  int input_panel_show = 0;
  imcontrol_client_->show_ise(panel_client_id_, im_context_id_, ise_packet,
      ise_packet_length, &input_panel_show);
  imcontrol_client_->send();

  free(ise_packet);
}

void SCIMBridgeImpl::HideInputMethodPanel() {
  SetFocus(false);
  imcontrol_client_->prepare();
  imcontrol_client_->hide_ise(panel_client_id_, im_context_id_);
  imcontrol_client_->send();
}

bool SCIMBridgeImpl::IsDaemonRunning() {
  uint32_t uid;
  SocketClient client;
  SocketAddress address;
  address.set_address(scim::scim_get_default_socket_frontend_address());

  if (!client.connect(address))
      return false;

  return scim::scim_socket_open_connection(uid, kScimConnectionTester,
                                           kScimSocketFrontend, client, 1000);
}

void SCIMBridgeImpl::StartDaemon() {
  if (!IsDaemonRunning()) {
    std::vector<String> helper_modules;
    std::vector<String> imengine_modules;

    scim::scim_get_imengine_module_list(imengine_modules);
    scim::scim_get_helper_module_list(helper_modules);

    std::vector<String>::iterator it;

    for (it = imengine_modules.begin(); it != imengine_modules.end(); it++) {
      if (*it != kScimFrontendSocketIpc)
          scim_modules_.push_back(*it);
    }

    for (it = helper_modules.begin(); it != helper_modules.end(); it++)
      scim_modules_.push_back(*it);

    String engines;

    if (scim_modules_.size() > 0)
      engines = scim::scim_combine_string_list(scim_modules_, ',');
    else
      engines = "none";

    // The --no-stay command line parameter will force server to shutdown
    // when last client disconnects from it.
    const char* argv[] = { "--no-stay", 0 };
    // Start SCIM socket frontend in "daemon" mode.
    scim::scim_launch(true, config_module_name_, engines,
                      kScimFrontendSocketIpc,
                      const_cast<char**>(argv));
  } else {
    scim_modules_.clear();
    scim_modules_.push_back(kScimFrontendSocketIpc);
    config_module_name_ = kScimFrontendSocketIpc;
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
  } else {
    LOG(ERROR) << "Cannot get panel client connection";
  }
}

void SCIMBridgeImpl::ConnectPanelClientSignals() {
  panel_client_->signal_connect_update_client_id(
      scim::slot(this, &SCIMBridgeImpl::OnUpdateClientId));
  panel_client_->signal_connect_exit(
      scim::slot(this, &SCIMBridgeImpl::OnExit));
  panel_client_->signal_connect_select_aux(
      scim::slot(this, &SCIMBridgeImpl::OnSelectAux));

  panel_client_->signal_connect_forward_key_event(
      scim::slot(this, &SCIMBridgeImpl::OnForwardKeyEvent));
  panel_client_->signal_connect_process_key_event(
      scim::slot(this, &SCIMBridgeImpl::OnProcessKeyEvent));

  panel_client_->signal_connect_reload_config(
      scim::slot(this, &SCIMBridgeImpl::OnReloadConfig));
  panel_client_->signal_connect_process_helper_event(
      scim::slot(this, &SCIMBridgeImpl::OnProcessHelperEvent));
  panel_client_->signal_connect_request_help(
      scim::slot(this, &SCIMBridgeImpl::OnRequestHelp));
  panel_client_->signal_connect_request_factory_menu(
      scim::slot(this, &SCIMBridgeImpl::OnRequestFactoryMenu));
  panel_client_->signal_connect_change_factory(
      scim::slot(this, &SCIMBridgeImpl::OnChangeFactory));

  panel_client_->signal_connect_update_candidate_item_layout(
      scim::slot(this, &SCIMBridgeImpl::OnUpdateCandidateItemLayout));
  panel_client_->signal_connect_select_candidate(
      scim::slot(this, &SCIMBridgeImpl::OnSelectCandidate));

  panel_client_->signal_connect_reset_keyboard_ise(
      scim::slot(this, &SCIMBridgeImpl::OnResetKeyboardIse));
  panel_client_->signal_connect_update_keyboard_ise(
      scim::slot(this, &SCIMBridgeImpl::OnUpdateKeyboardIse));
  panel_client_->signal_connect_update_preedit_string(
      scim::slot(this, &SCIMBridgeImpl::OnUpdatePreeditString));
  panel_client_->signal_connect_move_preedit_caret(
        scim::slot(this, &SCIMBridgeImpl::OnMovePreeditCaret));
  panel_client_->signal_connect_update_preedit_caret(
        scim::slot(this, &SCIMBridgeImpl::OnUpdatePreeditCaret));
  panel_client_->signal_connect_commit_string(
      scim::slot(this, &SCIMBridgeImpl::OnCommitString));
  panel_client_->signal_connect_get_surrounding_text(
      scim::slot(this, &SCIMBridgeImpl::OnGetSurroundingText));
  panel_client_->signal_connect_delete_surrounding_text(
      scim::slot(this, &SCIMBridgeImpl::OnDeleteSurroundingText));

  panel_client_->signal_connect_update_lookup_table_page_size(
      scim::slot(this, &SCIMBridgeImpl::OnUpdateLookupTablePageSize));
  panel_client_->signal_connect_lookup_table_page_up(
      scim::slot(this, &SCIMBridgeImpl::OnLookupTablePageUp));
  panel_client_->signal_connect_lookup_table_page_down(
      scim::slot(this, &SCIMBridgeImpl::OnLookupTablePageDown));
  panel_client_->signal_connect_trigger_property(
      scim::slot(this, &SCIMBridgeImpl::OnTriggerProperty));
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

  ime_instance_ = factory->create_instance(kScimFactoryEncoding,
      ime_instance_count_++);

  if (ime_instance_.null()) {
     LOG(ERROR) << "Cannot get default IM engine instance";
     return;
  }

  ConnectIMESignals();
  RegisterIMEFactory(factory);
}

void SCIMBridgeImpl::SetFocus(bool focused) {
  imcontrol_client_->prepare();
  if (focused) {
    imcontrol_client_->focus_in();
  } else {
    imcontrol_client_->focus_out();
  }
  imcontrol_client_->send();

  panel_client_->prepare(im_context_id_);
  ime_instance_->reset();
  if (focused) {
    panel_client_->focus_in(im_context_id_, ime_instance_->get_factory_uuid());
    ime_instance_->focus_in();
  } else {
    panel_client_->focus_out(im_context_id_);
    ime_instance_->focus_out();
  }
  panel_client_->send();
}

void SCIMBridgeImpl::RegisterIMEFactory(const IMEngineFactoryPointer& factory) {
  backend_->set_default_factory(language_code_, factory->get_uuid());
  panel_client_->prepare(im_context_id_);
  panel_client_->register_input_context(im_context_id_, factory->get_uuid());
  ime_instance_->update_client_capabilities(
      scim::SCIM_CLIENT_CAP_ALL_CAPABILITIES);
  ime_instance_->set_layout(0);
  ime_instance_->set_prediction_allow(true);
  panel_client_->send();
}

void SCIMBridgeImpl::OnUpdateClientId(int, int client_id) {
  panel_client_id_ = client_id;
}

void SCIMBridgeImpl::OnProcessHelperEvent(int context,
                                        const String &target_uuid,
                                        const String &helper_uuid,
                                        const Transaction &trans) {
  if (ime_instance_->get_factory_uuid() == target_uuid) {
    panel_client_->prepare(im_context_id_);
    ime_instance_->process_helper_event(helper_uuid, trans);
    panel_client_->send();
  }
}

void SCIMBridgeImpl::OnResetKeyboardIse(int context) {
  panel_client_->prepare(im_context_id_);
  ime_instance_->reset();
  panel_client_->send();
}

void SCIMBridgeImpl::OnProcessKeyEvent(int context, const scim::KeyEvent& key) {
  content::BrowserThread::PostTask(content::BrowserThread::UI,
                          FROM_HERE,
                          base::Bind(&SCIMBridgeImpl::SendXKeyEvent,
                                     base::Unretained(this), key, false));
}

void SCIMBridgeImpl::OnCommitString(int context, const WideString &wstr) {
  if (!client_ || im_context_id_ != context)
    return;

  if (client_->GetTextInputType() != TEXT_INPUT_TYPE_NONE) {
    scim::String utf8_str = scim::utf8_wcstombs(wstr);
    client_->ConfirmCompositionText();
    client_->InsertText(base::UTF8ToUTF16(utf8_str));
  }
}

void SCIMBridgeImpl::OnReloadConfig(int context) {
  if (context == im_context_id_)
    config_->reload();
}

void SCIMBridgeImpl::OnExit(int context) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnUpdateCandidateItemLayout(int context,
                                      const std::vector<unsigned int> &items) {
  panel_client_->prepare(im_context_id_);
  ime_instance_->update_candidate_item_layout(items);
  panel_client_->send();
}

void SCIMBridgeImpl::OnUpdateLookupTablePageSize(int context, int page_size) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnLookupTablePageUp(int context) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnLookupTablePageDown(int context) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnTriggerProperty(int context, const String &property) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnMovePreeditCaret(int context, int caret_pos) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnUpdatePreeditCaret(int context, int caret) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnSelectAux(int context, int aux_index) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnSelectCandidate(int context, int candidate) {
  panel_client_->prepare(im_context_id_);
  ime_instance_->select_candidate(candidate);
  panel_client_->send();
}

void SCIMBridgeImpl::OnForwardKeyEvent(int context, const scim::KeyEvent& key) {
  LOG(INFO) << "SCIMBridgeImpl::OnForwardKeyEvent " << key.code;
  content::BrowserThread::PostTask(content::BrowserThread::UI,
                          FROM_HERE,
                          base::Bind(&SCIMBridgeImpl::SendXKeyEvent,
                                     base::Unretained(this), key, true));
}

void SCIMBridgeImpl::OnRequestHelp(int context) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnRequestFactoryMenu(int context) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnChangeFactory(int context, const String &uuid) {
  panel_client_->prepare(im_context_id_);
  ime_instance_->reset();
  IMEngineFactoryPointer factory = backend_->get_factory(uuid);
  ime_instance_ = factory->create_instance(kScimFactoryEncoding,
                                           ime_instance_->get_id());
  ConnectIMESignals();
  RegisterIMEFactory(factory);

  if (client_->GetTextInputType() != TEXT_INPUT_TYPE_NONE) {
    SetFocus(true);
  }
}

void SCIMBridgeImpl::OnUpdateKeyboardIse(int context) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnUpdatePreeditString(int context,
                                           const WideString &wstr,
                                           const AttributeList &attrs) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnGetSurroundingText(int context,
                                          int maxlen_before,
                                          int maxlen_after) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnDeleteSurroundingText(int context, int offset, int len) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::SendXKeyEvent(const scim::KeyEvent& key, bool synthetic) {
  ::KeyCode keycode = 0;
  ::KeySym keysym = 0;
  XKeyEvent event = scim_x11_keyevent_scim_to_x11(display_, key);
  if (synthetic)
    event.time = 0;
  event.same_screen = True;
  event.window = window_;
  keysym = XStringToKeysym(key.get_key_string().c_str());
  if (keysym == NoSymbol)
    return;
  keycode = XKeysymToKeycode(display_, keysym);
  if (XkbKeycodeToKeysym(display_, keycode, 0, 1) == keysym)
    event.state |= ShiftMask;
  XSendEvent(display_, window_, True, event.type,
             reinterpret_cast<XEvent*>(&event));
}

void SCIMBridgeImpl::SetInputMethodActiveWindow() {
  SetIntProperty(root_window_, kISFActiveWindowAtom, kWindowAtomType, window_);
}

void SCIMBridgeImpl::SetFocusedWindow(XID xid) {
  window_ = xid;
  ise_context_.client_window = xid;
  SetInputMethodActiveWindow();
}

void SCIMBridgeImpl::DispatchNativeKeyEvent(
    const base::NativeEvent& native_event) {
  scim::KeyEvent event =
      scim_x11_keyevent_x11_to_scim(display_, native_event->xkey);
  char key_code = event.get_ascii_code();
  if ((native_event->xkey.time == 0 && isgraph(key_code))
      || (key_code == scim::SCIM_KEY_space
          || key_code == scim::SCIM_KEY_KP_Space)) {
    ime_instance_->reset();
    std::string string_piece;
    string_piece += key_code;
    string16 utf16_str = base::ASCIIToUTF16(string_piece);
    if (client_)
      client_->InsertText(utf16_str);
  } else {
    content::BrowserThread::PostTask(content::BrowserThread::IO,
                               FROM_HERE,
                               base::Bind(&SCIMBridgeImpl::DispatchSCIMKeyEvent,
                               base::Unretained(this), event));
  }
}

void SCIMBridgeImpl::DispatchSCIMKeyEvent(const scim::KeyEvent& event) {
  panel_client_->prepare(im_context_id_);
  if (!ime_instance_->process_key_event(event)) {
    content::BrowserThread::PostTask(content::BrowserThread::UI,
                            FROM_HERE,
                            base::Bind(&SCIMBridgeImpl::DispatchKeyEventPostIME,
                            base::Unretained(this), event));
  }
  panel_client_->send();
}

void SCIMBridgeImpl::DispatchKeyEventPostIME(const scim::KeyEvent& event) {
  // SCIM IM engine consumes keydown event, therefore, if input methed engine
  // does not need event, we have to forward full event keypress + keyrelease.
  XKeyEvent x11_key_event = scim_x11_keyevent_scim_to_x11(display_, event);
  XEvent* x11_event = reinterpret_cast<XEvent*>(&x11_key_event);
  KeyboardCode key_code = ui::KeyboardCodeFromNative(x11_event);
  int flags = ui::EventFlagsFromNative(x11_event);
  bool is_char = event.get_ascii_code() != 0;
  ui::KeyEvent key_down_event(ET_KEY_PRESSED, key_code, flags, is_char);
  ui::KeyEvent key_up_event(ET_KEY_RELEASED, key_code, flags, is_char);
  input_method_->DispatchFabricatedKeyEvent(key_down_event);
  input_method_->DispatchFabricatedKeyEvent(key_up_event);
}

void SCIMBridgeImpl::SetTextInputClient(TextInputClient* client) {
  client_ = client;
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

void SCIMBridgeImpl::ConnectIMESignals() {
  ime_instance_->signal_connect_show_aux_string(
      slot(this, &SCIMBridgeImpl::OnIMEShowAuxString));
  ime_instance_->signal_connect_show_lookup_table(
      slot(this, &SCIMBridgeImpl::OnIMEShowLookupTable));

  ime_instance_->signal_connect_hide_preedit_string(
      slot(this, &SCIMBridgeImpl::OnIMEHidePreeditString));
  ime_instance_->signal_connect_hide_aux_string(
      slot(this, &SCIMBridgeImpl::OnIMEHideAuxString));
  ime_instance_->signal_connect_hide_lookup_table(
      slot(this, &SCIMBridgeImpl::OnIMEHideLookupTable));

  ime_instance_->signal_connect_update_preedit_caret(
      slot(this, &SCIMBridgeImpl::OnIMEUpdatePreeditCaret));
  ime_instance_->signal_connect_update_preedit_string(
      slot(this, &SCIMBridgeImpl::OnIMEUpdatePreeditString));
  ime_instance_->signal_connect_update_aux_string(
      slot(this, &SCIMBridgeImpl::OnIMEUpdateAuxString));
  ime_instance_->signal_connect_update_lookup_table(
      slot(this, &SCIMBridgeImpl::OnIMEUpdateLookupTable));

  ime_instance_->signal_connect_commit_string(
      slot(this, &SCIMBridgeImpl::OnIMECommitString));

  ime_instance_->signal_connect_forward_key_event(
      slot(this, &SCIMBridgeImpl::OnIMEForwardKeyEvent));

  ime_instance_->signal_connect_register_properties(
      slot(this, &SCIMBridgeImpl::OnIMERegisterProperties));

  ime_instance_->signal_connect_update_property(
      slot(this, &SCIMBridgeImpl::OnIMEUpdateProperty));

  ime_instance_->signal_connect_beep(
      slot(this, &SCIMBridgeImpl::OnIMEBeep));

  ime_instance_->signal_connect_start_helper(
      slot(this, &SCIMBridgeImpl::OnIMEStartHelper));

  ime_instance_->signal_connect_stop_helper(
      slot(this, &SCIMBridgeImpl::OnIMEStopHelper));

  ime_instance_->signal_connect_send_helper_event(
      slot(this, &SCIMBridgeImpl::OnIMESendHelperEvent));

  ime_instance_->signal_connect_get_surrounding_text(
      slot(this, &SCIMBridgeImpl::OnIMEGetSurroundingText));

  ime_instance_->signal_connect_delete_surrounding_text(
      slot(this, &SCIMBridgeImpl::OnIMEDeleteSurroundingText));

  ime_instance_->signal_connect_expand_candidate(
      slot(this, &SCIMBridgeImpl::OnIMEExpandCandidate));
  ime_instance_->signal_connect_contract_candidate(
      slot(this, &SCIMBridgeImpl::OnIMEContractCandidate));

  ime_instance_->signal_connect_set_candidate_style(
      slot(this, &SCIMBridgeImpl::OnIMESetCandidateStyle));
}

void SCIMBridgeImpl::OnIMEShowAuxString(IMEngineInstanceBase* ime) {
  panel_client_->show_aux_string(im_context_id_);
}

void SCIMBridgeImpl::OnIMEShowLookupTable(IMEngineInstanceBase* ime) {
  panel_client_->show_lookup_table(im_context_id_);
}

void SCIMBridgeImpl::OnIMEHidePreeditString(IMEngineInstanceBase* ime) {
  if (client_)
    client_->ClearCompositionText();
}

void SCIMBridgeImpl::OnIMEHideAuxString(IMEngineInstanceBase* ime) {
  panel_client_->hide_aux_string(im_context_id_);
}

void SCIMBridgeImpl::OnIMEHideLookupTable(IMEngineInstanceBase* ime) {
  panel_client_->hide_lookup_table(im_context_id_);
}

void SCIMBridgeImpl::OnIMEUpdatePreeditCaret(IMEngineInstanceBase* ime,
                                             int caret) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnIMEUpdatePreeditString(IMEngineInstanceBase* ime,
                                              const WideString &wstr,
                                              const AttributeList &attrs) {
  if (client_)
    client_->SetCompositionText(toCompositionText(wstr));
}

void SCIMBridgeImpl::OnIMEUpdateAuxString(IMEngineInstanceBase* ime,
                                          const WideString &wstr,
                                          const AttributeList &attrs) {
  panel_client_->update_aux_string(im_context_id_, wstr, attrs);
}

void SCIMBridgeImpl::OnIMECommitString(IMEngineInstanceBase* ime,
                                       const WideString &wstr) {
  if (!client_)
    return;

  if (!client_->HasCompositionText()) {
    client_->SetCompositionText(toCompositionText(wstr));
  }

  client_->ConfirmCompositionText();
}

void SCIMBridgeImpl::OnIMEForwardKeyEvent(IMEngineInstanceBase* ime,
                                          const scim::KeyEvent &key) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnIMEUpdateLookupTable(IMEngineInstanceBase* ime,
                                            const LookupTable &table) {
  panel_client_->update_lookup_table(im_context_id_, table);
}

void SCIMBridgeImpl::OnIMERegisterProperties(IMEngineInstanceBase* ime,
                                             const PropertyList &properties) {
  panel_client_->register_properties(im_context_id_, properties);
}

void SCIMBridgeImpl::OnIMEUpdateProperty(IMEngineInstanceBase* ime,
                                         const Property &property) {
  panel_client_->update_property(im_context_id_, property);
}

void SCIMBridgeImpl::OnIMEBeep(IMEngineInstanceBase* ime) {
  // TODO(shalamov): Implement.
}

void SCIMBridgeImpl::OnIMEStartHelper(IMEngineInstanceBase* ime,
                                      const String &uuid) {
  panel_client_->start_helper(im_context_id_, uuid);
}

void SCIMBridgeImpl::OnIMEStopHelper(IMEngineInstanceBase* ime,
                                     const String &uuid) {
  panel_client_->stop_helper(im_context_id_, uuid);
}

void SCIMBridgeImpl::OnIMESendHelperEvent(IMEngineInstanceBase* ime,
                                          const String &uuid,
                                          const Transaction &trans) {
  panel_client_->send_helper_event(im_context_id_, uuid, trans);
}

bool SCIMBridgeImpl::OnIMEGetSurroundingText(IMEngineInstanceBase* ime,
                                             WideString& text,
                                             int& cursor,
                                             int maxlen_before,
                                             int maxlen_after) {
  return true;
}

bool SCIMBridgeImpl::OnIMEDeleteSurroundingText(IMEngineInstanceBase* ime,
                                                int offset, int len) {
  return true;
}

void SCIMBridgeImpl::OnIMEExpandCandidate(IMEngineInstanceBase* ime) {
  panel_client_->expand_candidate(im_context_id_);
}

void SCIMBridgeImpl::OnIMEContractCandidate(IMEngineInstanceBase* ime) {
  panel_client_->contract_candidate(im_context_id_);
}

void SCIMBridgeImpl::OnIMESetCandidateStyle(IMEngineInstanceBase* ime,
                                       scim::ISF_CANDIDATE_PORTRAIT_LINE_T line,
                                       scim::ISF_CANDIDATE_MODE_T mode) {
  panel_client_->set_candidate_style(im_context_id_, line, mode);
}

}  // namespace ui
