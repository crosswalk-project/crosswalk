#include "cameo/cameo_main_delegate.h"
#include "content/public/app/content_main.h"

int main(int argc, const char* argv[]) {
  CameoMainDelegate delegate;
  return content::ContentMain(argc, argv, &delegate);
}
