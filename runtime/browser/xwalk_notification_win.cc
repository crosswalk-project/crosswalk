// Copyright (c) 2016 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "xwalk/runtime/browser/xwalk_notification_win.h"

#include <shlobj.h>
#include <vector>

#include "NotificationActivationCallback.h"
#include "base/files/file_util.h"
#include "base/md5.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/desktop_notification_delegate.h"
#include "content/public/common/notification_resources.h"
#include "url/gurl.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/codec/png_codec.h"
#include "xwalk/runtime/browser/xwalk_notification_manager_win.h"

namespace {

bool SaveIconToPath(const SkBitmap& bitmap, const base::FilePath& path) {
  std::vector<unsigned char> png_data;
  if (!gfx::PNGCodec::EncodeBGRASkBitmap(bitmap, false, &png_data))
    return false;

  char* data = reinterpret_cast<char*>(&png_data[0]);
  int size = static_cast<int>(png_data.size());
  return base::WriteFile(path, data, size) == size;
}

base::string16 GetAppUserModelId() {
  base::string16 app_id;
  PWSTR current_app_id;
  if (SUCCEEDED(GetCurrentProcessExplicitAppUserModelID(&current_app_id))) {
    app_id = base::string16(current_app_id);
    CoTaskMemFree(current_app_id);
  }
  return app_id;
}

HSTRING MakeHString(const base::string16& str) {
  HSTRING hstr;
  if (FAILED(::WindowsCreateString(str.c_str(), static_cast<UINT32>(str.size()),
    &hstr))) {
    PLOG(DFATAL) << "Hstring creation failed";
  }
  return hstr;
}

base::string16 MakeStdWString(HSTRING hstring) {
  const wchar_t* str;
  UINT32 size = 0;
  str = ::WindowsGetStringRawBuffer(hstring, &size);
  if (!size)
    return base::string16();
  return base::string16(str, size);
}

}  // namespace

namespace xwalk {

// static
mswr::ComPtr<winui::Notifications::IToastNotificationManagerStatics>
    XWalkNotificationWin::toast_manager_;

// static
mswr::ComPtr<winui::Notifications::IToastNotifier>
    XWalkNotificationWin::toast_notifier_;

XWalkNotificationWin::XWalkNotificationWin(
    XWalkNotificationManager* manager,
    std::unique_ptr<content::DesktopNotificationDelegate> delegate)
    : manager_(manager),
      delegate_(std::move(delegate)) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  content::BrowserThread::PostTask(
      content::BrowserThread::FILE, FROM_HERE,
      base::Bind(&XWalkNotificationWin::InitTempDir, this));
}

XWalkNotificationWin::~XWalkNotificationWin() {
}

void XWalkNotificationWin::Destroy() {
  content::BrowserThread::PostTask(
      content::BrowserThread::FILE, FROM_HERE,
      base::Bind(&XWalkNotificationWin::CleanupTempDir, this));
}

void XWalkNotificationWin::InitTempDir() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::FILE));
  temp_dir_.CreateUniqueTempDir();
}

void XWalkNotificationWin::CleanupTempDir() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::FILE));
  temp_dir_.Delete();
}


// static
bool XWalkNotificationWin::Initialize() {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  winfoundtn::Initialize(RO_INIT_MULTITHREADED);

  if (FAILED(winfoundtn::GetActivationFactory(
        MakeHString(
            RuntimeClass_Windows_UI_Notifications_ToastNotificationManager),
        &toast_manager_)))
    return false;

  base::string16 app_id = GetAppUserModelId();
  if (app_id.empty())
    return false;

  return SUCCEEDED(
      toast_manager_->CreateToastNotifierWithId(
          MakeHString(app_id), &toast_notifier_));
}

bool XWalkNotificationWin::SetImageSrc(
    const std::wstring& imagePath,
    winxml::Dom::IXmlDocument *toastXml) {
  mswr::ComPtr<winxml::Dom::IXmlNodeList> nodeList;
  if (FAILED(toastXml->GetElementsByTagName(MakeHString(L"image"), &nodeList)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> imageNode;
  if (FAILED(nodeList->Item(0, &imageNode)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNamedNodeMap> attributes;
  if (FAILED(imageNode->get_Attributes(&attributes)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> srcAttribute;
  if (FAILED(attributes->GetNamedItem(MakeHString(L"src"), &srcAttribute)))
    return false;

  std::wstringstream image_path;
  image_path << imagePath.c_str();
  mswr::ComPtr<winxml::Dom::IXmlText> inputText;
  if (FAILED(toastXml->CreateTextNode(MakeHString(image_path.str()),
                                                  &inputText)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> inputTextNode;
  if (FAILED(inputText.As(&inputTextNode)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> pAppendedChild;
  if (FAILED(srcAttribute->AppendChild(inputTextNode.Get(), &pAppendedChild)))
    return false;

  return true;
}

bool XWalkNotificationWin::SetXmlText(
    winxml::Dom::IXmlDocument* doc, const std::wstring& text) {
    mswr::ComPtr<winxml::Dom::IXmlNodeList> node_list;
  if (!GetTextNodeList(doc, &node_list, 1))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> node;
  if (FAILED(node_list->Item(0, &node)))
    return false;

  return AppendTextToXml(doc, node.Get(), text);
}

bool XWalkNotificationWin::AppendTextToXml(
    winxml::Dom::IXmlDocument* doc,
    winxml::Dom::IXmlNode* node,
    const std::wstring& text) {
  mswr::ComPtr<winxml::Dom::IXmlText> xml_text;
  if (FAILED(doc->CreateTextNode(MakeHString(text), &xml_text)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> text_node;
  if (FAILED(xml_text.As(&text_node)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> append_node;
  return SUCCEEDED(node->AppendChild(text_node.Get(), &append_node));
}


bool XWalkNotificationWin::SetXmlText(
    winxml::Dom::IXmlDocument* doc,
    const std::wstring& title,
    const std::wstring& body) {
  mswr::ComPtr<winxml::Dom::IXmlNodeList> node_list;
  if (!GetTextNodeList(doc, &node_list, 2))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> node;
  if (FAILED(node_list->Item(0, &node)))
    return false;

  if (!AppendTextToXml(doc, node.Get(), title))
    return false;

  if (FAILED(node_list->Item(1, &node)))
    return false;

  return AppendTextToXml(doc, node.Get(), body);
}

bool XWalkNotificationWin::GetTextNodeList(
    winxml::Dom::IXmlDocument* doc,
    winxml::Dom::IXmlNodeList** node_list,
    uint32_t req_length) {

  if (FAILED(doc->GetElementsByTagName(MakeHString(L"text"), node_list)))
    return false;

  uint32_t node_length;
  if (FAILED((*node_list)->get_Length(&node_length)))
    return false;

  return node_length >= req_length;
}

bool XWalkNotificationWin::CreateToastXml(
    const std::wstring& title,
    const std::wstring& body,
    const std::wstring& icon_path,
    const bool silent,
    winxml::Dom::IXmlDocument** input_xml) {
  winui::Notifications::ToastTemplateType template_type;
  if (title.empty() || body.empty()) {
    // Single line toast.
    template_type = icon_path.empty() ?
        winui::Notifications::ToastTemplateType_ToastText01 :
        winui::Notifications::ToastTemplateType_ToastImageAndText01;
    if (FAILED(toast_manager_->GetTemplateContent(template_type, input_xml)))
      return false;
    if (!SetXmlText(*input_xml,
        title.empty() ? body : title))
      return false;
  } else {
    // Title and body toast.
    template_type = icon_path.empty() ?
        winui::Notifications::ToastTemplateType_ToastText02 :
        winui::Notifications::ToastTemplateType_ToastImageAndText02;
    if (FAILED(toast_manager_->GetTemplateContent(template_type, input_xml)))
      return false;
    if (!SetXmlText(*input_xml, title, body))
      return false;
  }

  if (silent) {
    if (!SetXmlAudioSilent(*input_xml))
      return false;
  }

  if (!icon_path.empty())
    return SetImageSrc(icon_path, *input_xml);

  return true;
}

bool XWalkNotificationWin::SetXmlAudioSilent(
    winxml::Dom::IXmlDocument* doc) {

  mswr::ComPtr<winxml::Dom::IXmlNodeList> node_list;
  if (FAILED(doc->GetElementsByTagName(MakeHString(L"toast"), &node_list)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> root;
  if (FAILED(node_list->Item(0, &root)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlElement> audio_element;
  if (FAILED(doc->CreateElement(MakeHString(L"toast"), &audio_element)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> audio_node_tmp;
  if (FAILED(audio_element.As(&audio_node_tmp)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> audio_node;
  if (FAILED(root->AppendChild(audio_node_tmp.Get(), &audio_node)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNamedNodeMap> attributes;
  if (FAILED(audio_node->get_Attributes(&attributes)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlAttribute> silent_attribute;
  if (FAILED(doc->CreateAttribute(MakeHString(L"silent"), &silent_attribute)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> silent_attribute_node;
  if (FAILED(silent_attribute.As(&silent_attribute_node)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlText> silent_text;
  if (FAILED(doc->CreateTextNode(MakeHString(L"true"), &silent_text)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> silent_node;
  if (FAILED(silent_text.As(&silent_node)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> child_node;
  if (FAILED(silent_attribute_node->AppendChild(
        silent_node.Get(), &child_node)))
    return false;

  mswr::ComPtr<winxml::Dom::IXmlNode> silent_attribute_pnode;
  return SUCCEEDED(attributes.Get()->SetNamedItem(
        silent_attribute_node.Get(), &silent_attribute_pnode));
}

void XWalkNotificationWin::SaveIconToFilesystemAndProceed(
    const base::string16& title,
    const base::string16& body,
    const GURL& icon_url,
    const SkBitmap& icon,
    const bool silent) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::FILE));
  base::string16 icon_path;
  std::string filename = base::MD5String(icon_url.spec()) + ".png";
  base::FilePath path = temp_dir_.path().Append(base::UTF8ToUTF16(filename));
  if (base::PathExists(path))
    icon_path = path.value();
  else if (SaveIconToPath(icon, path))
    icon_path = path.value();
  else
    icon_path = base::UTF8ToUTF16(icon_url.spec());
  content::BrowserThread::PostTask(
      content::BrowserThread::UI, FROM_HERE,
      base::Bind(&XWalkNotificationWin::ShowNotification,
          this, title, body, icon_path, silent));
}


void XWalkNotificationWin::Show(
    const base::string16& title,
    const base::string16& body,
    const GURL& icon_url,
    const content::NotificationResources& notification_resources,
    const bool silent) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  content::BrowserThread::PostTask(
      content::BrowserThread::FILE, FROM_HERE,
      base::Bind(&XWalkNotificationWin::SaveIconToFilesystemAndProceed,
          this, title, body, icon_url,
          notification_resources.notification_icon, silent));
}

void XWalkNotificationWin::ShowNotification(
    const base::string16& title,
    const base::string16& body,
    const base::string16& icon_path,
    const bool silent) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  mswr::ComPtr<winxml::Dom::IXmlDocument> toast_xml;
  if (!CreateToastXml(title, body, icon_path, silent, &toast_xml))
    return;

  mswr::ComPtr<winui::Notifications::IToastNotificationFactory> factory;
  if (FAILED(winfoundtn::GetActivationFactory(
      MakeHString(RuntimeClass_Windows_UI_Notifications_ToastNotification),
      &factory)))
    return;

  if (FAILED(factory->CreateToastNotification(toast_xml.Get(),
                                              &toast_notification_)))
    return;

  mswr::ComPtr<NotificationEventHandlerWin> event_handler_ =
      mswr::Make<NotificationEventHandlerWin>(this);
  if (FAILED(toast_notification_->add_Activated(
      event_handler_.Get(), &activated_token_)))
    return;
  if (FAILED(toast_notification_->add_Dismissed(
      event_handler_.Get(), &dismissed_token_)))
    return;
  if (FAILED(toast_notification_->add_Failed(
      event_handler_.Get(), &failed_token_)))
    return;

  if (FAILED(toast_notifier_->Show(toast_notification_.Get())))
    return;

  delegate_->NotificationDisplayed();
}

void XWalkNotificationWin::Dismiss() {
  toast_notifier_->Hide(toast_notification_.Get());
}

void XWalkNotificationWin::NotificationClosed() {
  delegate_->NotificationClosed();
  manager_->RemoveNotification(this);
}


void XWalkNotificationWin::NotificationClicked() {
  delegate_->NotificationClick();
  manager_->RemoveNotification(this);
}

NotificationEventHandlerWin::NotificationEventHandlerWin(
    XWalkNotificationWin* notification)
    : notification_(notification) {
}

NotificationEventHandlerWin::~NotificationEventHandlerWin() {
}

IFACEMETHODIMP NotificationEventHandlerWin::Invoke(
    ABI::Windows::UI::Notifications::IToastNotification* sender,
    IInspectable* args) {
  notification_->NotificationClicked();
  return S_OK;
}

IFACEMETHODIMP NotificationEventHandlerWin::Invoke(
    ABI::Windows::UI::Notifications::IToastNotification* sender,
    ABI::Windows::UI::Notifications::IToastDismissedEventArgs* event) {
  notification_->NotificationClosed();
  return S_OK;
}

IFACEMETHODIMP NotificationEventHandlerWin::Invoke(
    ABI::Windows::UI::Notifications::IToastNotification* sender,
    ABI::Windows::UI::Notifications::IToastFailedEventArgs* event) {
  notification_->NotificationClosed();
  return S_OK;
}

}  // namespace xwalk
