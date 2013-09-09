// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/db_store_json_impl.h"

#include "base/file_util.h"
#include "base/json/json_file_value_serializer.h"
#include "base/json/json_string_value_serializer.h"
#include "content/public/browser/browser_thread.h"
#include "xwalk/application/browser/application_store.h"

namespace xwalk {
namespace application {

const base::FilePath::CharType kDBFileName[] =
    FILE_PATH_LITERAL("applications_db");

inline const std::string GetManifestPath(const std::string& application_id) {
  return application_id + "." + ApplicationStore::kManifestPath;
}

inline const std::string GetApplicationPath(const std::string& application_id) {
  return application_id + "." + ApplicationStore::kApplicationPath;
}

inline const std::string GetInstallTimePath(const std::string& application_id) {
  return application_id + "." + ApplicationStore::kInstallTime;
}

// Differentiates file loading between origin thread and passed
// (aka file) thread. It's adapted from base/prefs/json_pref_store.cc.
class FileThreadDeserializer
    : public base::RefCountedThreadSafe<FileThreadDeserializer> {
 public:
  FileThreadDeserializer(DBStoreJsonImpl* delegate,
                         base::SequencedTaskRunner* sequenced_task_runner)
      : no_dir_(false),
        error_msg_(std::string()),
        delegate_(delegate),
        sequenced_task_runner_(sequenced_task_runner),
        origin_loop_proxy_(base::MessageLoopProxy::current()) {
  }

  ~FileThreadDeserializer() {}

  void Start(const base::FilePath& path) {
    DCHECK(origin_loop_proxy_->BelongsToCurrentThread());
    sequenced_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&FileThreadDeserializer::ReadFileAndReport,
                   this, path));
  }

  // Deserializes JSON on the sequenced task runner.
  void ReadFileAndReport(const base::FilePath& path) {
    DCHECK(sequenced_task_runner_->RunsTasksOnCurrentThread());

    value_.reset(DoReading(path, &error_msg_, &no_dir_));

    origin_loop_proxy_->PostTask(
        FROM_HERE,
        base::Bind(&FileThreadDeserializer::ReportOnOriginThread, this));
  }

  // Reports deserialization result on the origin thread.
  void ReportOnOriginThread() {
    DCHECK(origin_loop_proxy_->BelongsToCurrentThread());
    delegate_->OnFileRead(value_.release(), error_msg_, no_dir_);
  }

  static base::Value* DoReading(const base::FilePath& path,
                                std::string* error_msg,
                                bool* no_dir) {
    int error_code;
    JSONFileValueSerializer serializer(path);
    base::Value* value = serializer.Deserialize(&error_code, error_msg);

    *no_dir = !file_util::PathExists(path.DirName());
    return value;
  }

 private:
  friend class base::RefCountedThreadSafe<FileThreadDeserializer>;

  bool no_dir_;
  scoped_ptr<base::Value> value_;
  std::string error_msg_;
  const scoped_ptr<DBStoreJsonImpl> delegate_;
  const scoped_refptr<base::SequencedTaskRunner> sequenced_task_runner_;
  const scoped_refptr<base::MessageLoopProxy> origin_loop_proxy_;
};

// static
scoped_refptr<base::SequencedTaskRunner>
DBStoreJsonImpl::GetTaskRunnerForFile(
    const base::FilePath& db_filename,
    base::SequencedWorkerPool* worker_pool) {
  std::string token("db_store_json-");
  token.append(db_filename.AsUTF8Unsafe());
  return worker_pool->GetSequencedTaskRunnerWithShutdownBehavior(
      worker_pool->GetNamedSequenceToken(token),
      base::SequencedWorkerPool::BLOCK_SHUTDOWN);
}

// static
const base::FilePath GetDBPath(const base::FilePath& path) {
  return path.Append(kDBFileName);
}

// The implementation is adapted from base/prefs/json_pref_store.cc.
DBStoreJsonImpl::DBStoreJsonImpl(base::FilePath path)
    : DBStore(path) {
  const base::FilePath db_path = GetDBPath(data_path_);
  task_runner_ = GetTaskRunnerForFile(
      db_path,
      content::BrowserThread::GetBlockingPool());
  writer_.reset(new base::ImportantFileWriter(
      db_path, task_runner_.get()));
}

DBStoreJsonImpl::~DBStoreJsonImpl() {
  CommitPendingWrite();
}

bool DBStoreJsonImpl::InitDB() {
  bool no_dir;
  std::string error_msg;
  base::Value* value =
      FileThreadDeserializer::DoReading(GetDBPath(data_path_),
                                        &error_msg,
                                        &no_dir);
  OnFileRead(value, error_msg, no_dir);
  return (value != NULL && error_msg.empty());
}

void DBStoreJsonImpl::OnFileRead(base::Value* value_owned,
                                    std::string error_msg,
                                    bool no_dir) {
  scoped_ptr<base::Value> value(value_owned);

  if (no_dir) {
    if (!file_util::CreateDirectory(
            GetDBPath(data_path_).DirName())) {
      FOR_EACH_OBSERVER(DBStore::Observer,
                        observers_,
                        OnInitializationCompleted(false));
      return;
    }
  }

  if (value.get()) {
    db_.reset(static_cast<base::DictionaryValue*>(value.release()));
  } else {
    // The database is invalid.
    db_.reset(new base::DictionaryValue);
    file_util::WriteFile(GetDBPath(data_path_), "{}", 2);
  }

  FOR_EACH_OBSERVER(DBStore::Observer,
                    observers_,
                    OnInitializationCompleted(true));
}

void DBStoreJsonImpl::SetValue(const std::string& key, base::Value* value) {
  DCHECK(value);
  scoped_ptr<base::Value> new_value(value);
  base::Value* old_value = NULL;
  db_->Get(key, &old_value);
  if (!old_value || !value->Equals(old_value)) {
    base::Value* changed_value = new_value.release();
    db_->Set(key, changed_value);
    ReportValueChanged(key, changed_value);
  }
}

void DBStoreJsonImpl::ReportValueChanged(const std::string& key,
                                         const base::Value* value) {
  FOR_EACH_OBSERVER(
      DBStore::Observer, observers_, OnDBValueChanged(key, value));
  writer_->ScheduleWrite(this);
#if defined(OS_TIZEN_MOBILE)
  // FIXME: Workaround for Tizen changing database file ownership during app
  // installation: Write pending commits immediately.
  CommitPendingWrite();
#endif  // OS_TIZEN_MOBILE
}

void DBStoreJsonImpl::CommitPendingWrite() {
  if (writer_->HasPendingWrite())
    writer_->DoScheduledWrite();
}

bool DBStoreJsonImpl::Insert(const Application* application,
                             const base::Time install_time) {
  std::string application_id = application->ID();
  if (!db_->HasKey(application_id)) {
    base::DictionaryValue* manifest =
        application->GetManifest()->value()->DeepCopy();
    scoped_ptr<base::DictionaryValue> value(new base::DictionaryValue);
    value->Set(ApplicationStore::kManifestPath, manifest);
    value->SetString(ApplicationStore::kApplicationPath,
                     application->Path().value());
    value->SetDouble(ApplicationStore::kInstallTime, install_time.ToDoubleT());
    SetValue(application_id, value.release());
  }
  return true;
}

bool DBStoreJsonImpl::SerializeData(std::string* output) {
  JSONStringValueSerializer serializer(output);
  serializer.set_pretty_print(true);
  scoped_ptr<base::DictionaryValue> copy(
      db_->DeepCopyWithoutEmptyChildren());

  return serializer.Serialize(*(copy.get()));
}

}  // namespace application
}  // namespace xwalk
