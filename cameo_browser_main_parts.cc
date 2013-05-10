#include "cameo/cameo_browser_main_parts.h"

#include "cameo/cameo_browser_context.h"
#include "cameo/cameo_views_delegate.h"
#include "cameo/cameo_web_contents_delegate.h"
#include "cameo/cameo_widget_delegate.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "ui/gfx/screen.h"
#include "ui/views/controls/webview/webview.h"
#include "ui/views/widget/desktop_aura/desktop_screen.h"
#include <string>

void CameoBrowserMainParts::PreMainMessageLoopRun() {
  // FIXME: Fix ownership of all objects created in this function.
  // FIXME: Move this code to a more descriptive function or object that
  // will handle widget and webcontents creation.

  new CameoViewsDelegate;

  browser_context_.reset(new CameoBrowserContext);

  gfx::Screen::SetScreenInstance(
     gfx::SCREEN_TYPE_NATIVE, views::CreateDesktopScreen());

  CameoWidgetDelegate* delegate = new CameoWidgetDelegate();

  const int width = 600;
  const int height = 600;

  views::Widget* widget = views::Widget::CreateWindowWithBounds(
      delegate, gfx::Rect(0, 0, width, height));

  content::WebContents::CreateParams create_params(browser_context_.get(), NULL);
  create_params.initial_size = gfx::Size(width, height);
  content::WebContents* web_contents = content::WebContents::Create(create_params);
  web_contents->SetDelegate(new CameoWebContentsDelegate);

  views::WebView* web_view = new views::WebView(web_contents->GetBrowserContext());
  web_view->SetWebContents(web_contents);
  delegate->AddChildView(web_view);
  delegate->Layout();

  content::NavigationController::LoadURLParams params(GURL("http://www.chromium.org"));
  params.transition_type = content::PageTransitionFromInt(
      content::PAGE_TRANSITION_TYPED | content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  params.frame_name = std::string();
  web_contents->GetController().LoadURLWithParams(params);
  web_contents->GetView()->Focus();

  widget->Show();
}
