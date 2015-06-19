// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Copyright (c) 2009 Christoph Studer <chstuder@gmail.com>
// Copyright (c) 2010 Jan Berkel <jan.berkel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package org.xwalk.core.internal.extension.api.messaging;

// Contains SMS content provider constants. These values are copied from
// com.android.provider.telephony.*

public class MessagingSmsConsts {
    public static final String ID = "_id";
    public static final String BODY = "body";
    public static final String DATE = "date";
    public static final String THREAD_ID = "thread_id";
    public static final String ADDRESS = "address";
    public static final String TYPE = "type";
    public static final String READ = "read";
    public static final String STATUS = "status";
    public static final String SERVICE_CENTER = "service_center";
    public static final String PROTOCOL = "protocol";
    public static final String PERSON = "person";
    public static final int MESSAGE_TYPE_ALL = 0;
    public static final int MESSAGE_TYPE_INBOX = 1;
    public static final int MESSAGE_TYPE_SENT = 2;
    public static final int MESSAGE_TYPE_DRAFT = 3;
    public static final int MESSAGE_TYPE_OUTBOX = 4;
    public static final int MESSAGE_TYPE_FAILED = 5; // for failed outgoing messages
    public static final int MESSAGE_TYPE_QUEUED = 6; // for messages to send later
}
