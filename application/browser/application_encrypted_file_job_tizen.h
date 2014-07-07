// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_APPLICATION_BROWSER_APPLICATION_ENCRYPTED_FILE_JOB_TIZEN_H_
#define XWALK_APPLICATION_BROWSER_APPLICATION_ENCRYPTED_FILE_JOB_TIZEN_H_

#include <list>
#include <string>

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "net/url_request/url_request.h"
#include "net/url_request/url_request_job.h"

#include "xwalk/application/common/application_resource.h"

namespace base {
class TaskRunner;
}

namespace net {
class FileStream;
class StringIOBuffer;
}

namespace xwalk {
namespace application {

class URLRequestAppEncryptedFileJob : public net::URLRequestJob {
 public:
  URLRequestAppEncryptedFileJob(
    net::URLRequest* request,
    net::NetworkDelegate* network_delegate,
    const scoped_refptr<base::TaskRunner>& file_task_runner,
    const std::string& application_id,
    const base::FilePath& directory_path,
    const base::FilePath& relative_path,
    const std::string& content_security_policy,
    const std::list<std::string>& locales);

  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE;
  virtual void Start() OVERRIDE;
  virtual void Kill() OVERRIDE;
  virtual bool ReadRawData(net::IOBuffer* buf,
                           int buf_size,
                           int* bytes_read) OVERRIDE;
  virtual bool IsRedirectResponse(GURL* location,
                                  int* http_status_code) OVERRIDE;
  virtual bool GetMimeType(std::string* mime_type) const OVERRIDE;

 protected:
  virtual ~URLRequestAppEncryptedFileJob();

 private:
  void ReadFilePath(const ApplicationResource& resource,
                    base::FilePath* file_path);
  void DidReadFilePath(base::FilePath* read_file_path);
  void FetchFileInfo(const base::FilePath& file_path,
                     base::File::Info* file_info);
  void DidFetchFileInfo(const base::File::Info* file_info);
  void DidOpen(int result);
  void DidReadEncryptedData(scoped_refptr<net::IOBufferWithSize> buf,
                            int result);

  base::FilePath file_path_;
  const scoped_refptr<base::TaskRunner> file_task_runner_;
  scoped_ptr<net::FileStream> stream_;
  base::FilePath relative_path_;
  std::string content_security_policy_;
  ApplicationResource resource_;
  std::list<std::string> locales_;
  scoped_refptr<net::IOBufferWithSize> cipher_buffer_;
  scoped_refptr<net::StringIOBuffer> plain_buffer_;
  int plain_buffer_offset_;
  std::string mime_type_;
  base::WeakPtrFactory<URLRequestAppEncryptedFileJob> weak_ptr_factory_;
};

}  // namespace application
}  // namespace xwalk

#endif  // XWALK_APPLICATION_BROWSER_APPLICATION_ENCRYPTED_FILE_JOB_TIZEN_H_
