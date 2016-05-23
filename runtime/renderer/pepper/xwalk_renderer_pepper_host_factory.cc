// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/runtime/renderer/pepper/xwalk_renderer_pepper_host_factory.h"

#include "chrome/renderer/pepper/pepper_flash_drm_renderer_host.h"
#include "chrome/renderer/pepper/pepper_flash_font_file_host.h"
#include "chrome/renderer/pepper/pepper_flash_fullscreen_host.h"
#include "chrome/renderer/pepper/pepper_flash_menu_host.h"
#include "chrome/renderer/pepper/pepper_flash_renderer_host.h"
#include "content/public/renderer/renderer_ppapi_host.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "xwalk/runtime/renderer/pepper/pepper_uma_host.h"

using ppapi::host::ResourceHost;

XWalkRendererPepperHostFactory::XWalkRendererPepperHostFactory(
    content::RendererPpapiHost* host)
    : host_(host) {
}

XWalkRendererPepperHostFactory::~XWalkRendererPepperHostFactory() {
}

std::unique_ptr<ResourceHost>
XWalkRendererPepperHostFactory::CreateResourceHost(
    ppapi::host::PpapiHost* host,
    PP_Resource resource,
    PP_Instance instance,
    const IPC::Message& message) {
  DCHECK(host == host_->GetPpapiHost());

  // Make sure the plugin is giving us a valid instance for this resource.
  if (!host_->IsValidInstance(instance))
    return std::unique_ptr<ResourceHost>();

  if (host_->GetPpapiHost()->permissions().HasPermission(
          ppapi::PERMISSION_FLASH)) {
    switch (message.type()) {
      case PpapiHostMsg_Flash_Create::ID: {
        return std::unique_ptr<ResourceHost>(
            new PepperFlashRendererHost(host_, instance, resource));
      }
      case PpapiHostMsg_FlashFullscreen_Create::ID: {
        return std::unique_ptr<ResourceHost>(
            new PepperFlashFullscreenHost(host_, instance, resource));
      }
      case PpapiHostMsg_FlashMenu_Create::ID: {
        ppapi::proxy::SerializedFlashMenu serialized_menu;
        if (ppapi::UnpackMessage<PpapiHostMsg_FlashMenu_Create>(
                message, &serialized_menu)) {
          return std::unique_ptr<ResourceHost>(new PepperFlashMenuHost(
              host_, instance, resource, serialized_menu));
        }
        break;
      }
    }
  }

  if (host_->GetPpapiHost()->permissions().HasPermission(
          ppapi::PERMISSION_FLASH) ||
      host_->GetPpapiHost()->permissions().HasPermission(
          ppapi::PERMISSION_PRIVATE)) {
    switch (message.type()) {
      case PpapiHostMsg_FlashFontFile_Create::ID: {
        ppapi::proxy::SerializedFontDescription description;
        PP_PrivateFontCharset charset;
        if (ppapi::UnpackMessage<PpapiHostMsg_FlashFontFile_Create>(
                message, &description, &charset)) {
          return std::unique_ptr<ResourceHost>(new PepperFlashFontFileHost(
              host_, instance, resource, description, charset));
        }
        break;
      }
      case PpapiHostMsg_FlashDRM_Create::ID:
        return std::unique_ptr<ResourceHost>(
            new PepperFlashDRMRendererHost(host_, instance, resource));
    }
  }

  // Permissions for the following interfaces will be checked at the
  // time of the corresponding instance's method calls.  Currently these
  // interfaces are available only for whitelisted apps which may not have
  // access to the other private interfaces.
  if (message.type() == PpapiHostMsg_UMA_Create::ID) {
    return std::unique_ptr<ResourceHost>(new PepperUMAHost(
        host_, instance, resource));
  }

  return std::unique_ptr<ResourceHost>();
}
