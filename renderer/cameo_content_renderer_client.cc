#include "cameo/renderer/cameo_content_renderer_client.h"

#include "content/public/renderer/render_thread.h"

#include "v8/include/v8.h"
#include <cstdlib>
#include <string>

#include <iostream>

CameoContentRendererClient::CameoContentRendererClient() {
}

CameoContentRendererClient::~CameoContentRendererClient() {
}

class NotifyExtension : public v8::Extension {
 public:
  NotifyExtension();

  virtual v8::Handle<v8::FunctionTemplate> GetNativeFunction(v8::Handle<v8::String> name);

 private:
  static v8::Handle<v8::Value> Notify(const v8::Arguments& args);
};

static const char* const kNotifyExtensionName = "cameo/notify";
static const char* const kNotifyExtensionAPI =
    "var cameo;"
    "if (!cameo)"
    "  cameo = {};"
    "cameo.notify = function(msg) {"
    "  native function Notify();"
    "  Notify(msg);"
    "};";

NotifyExtension::NotifyExtension()
    : v8::Extension(kNotifyExtensionName, kNotifyExtensionAPI)
{
}

v8::Handle<v8::FunctionTemplate> NotifyExtension::GetNativeFunction(
    v8::Handle<v8::String> name) {
  if (name->Equals(v8::String::New("Notify")))
    return v8::FunctionTemplate::New(Notify);
  return v8::Handle<v8::FunctionTemplate>();
}

v8::Handle<v8::Value> NotifyExtension::Notify(const v8::Arguments& args) {
  if (args.Length() < 1)
    return v8::Undefined();

  const std::string msg = *v8::String::Utf8Value(args[0]->ToString());
  const std::string cmd = "notify-send \"Message from Cameo\" \"" + msg + "\"";
  std::cout << cmd << "\n";
  system(cmd.c_str());

  return v8::Undefined();
}

void CameoContentRendererClient::RenderThreadStarted() {
  content::RenderThread* thread = content::RenderThread::Get();
  thread->RegisterExtension(new NotifyExtension);
}
