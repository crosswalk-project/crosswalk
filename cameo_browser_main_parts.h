#ifndef CAMEO_CAMEO_BROWSER_MAIN_PARTS_H_
#define CAMEO_CAMEO_BROWSER_MAIN_PARTS_H_

#include "base/memory/scoped_ptr.h"
#include "content/public/browser/browser_main_parts.h"

class CameoBrowserContext;

class CameoBrowserMainParts : public content::BrowserMainParts {
 public:
  CameoBrowserMainParts() {}

  // BrowserMainParts implementation.
  virtual void PreMainMessageLoopRun();

  CameoBrowserContext* browser_context() { return browser_context_.get(); }

 private:
  scoped_ptr<CameoBrowserContext> browser_context_;
};

#endif  // CAMEO_CAMEO_BROWSER_MAIN_PARTS_H
