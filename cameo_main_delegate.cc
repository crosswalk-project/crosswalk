#include "cameo/cameo_main_delegate.h"

#include "base/path_service.h"
#include "cameo/cameo_content_browser_client.h"
#include "ui/base/resource/resource_bundle.h"

void CameoMainDelegate::PreSandboxStartup() {
  // FIXME: We are stealing content_shell.pak here, we should build our own.
  base::FilePath pak_file;
  base::FilePath pak_dir;
  PathService::Get(base::DIR_EXE, &pak_dir);
  pak_file = pak_dir.Append(FILE_PATH_LITERAL("content_shell.pak"));
  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
}

content::ContentBrowserClient* CameoMainDelegate::CreateContentBrowserClient() {
  return new CameoContentBrowserClient;
}
