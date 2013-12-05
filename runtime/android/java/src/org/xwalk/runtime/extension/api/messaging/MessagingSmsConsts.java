/* Copyright (c) 2009 Christoph Studer <chstuder@gmail.com>
 * Copyright (c) 2010 Jan Berkel <jan.berkel@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.xwalk.runtime.extension.api.messaging;

/**
 * Contains SMS content provider constants. These values are copied from
 * com.android.provider.telephony.*
 */
public interface MessagingSmsConsts {

    String ID = "_id";

    String BODY = "body";

    String DATE = "date";

    String THREAD_ID = "thread_id";

    String ADDRESS = "address";

    String TYPE = "type";

    String READ = "read";

    String STATUS = "status";

    String SERVICE_CENTER = "service_center";

    String PROTOCOL = "protocol";

    String PERSON = "person";

    @SuppressWarnings("UnusedDeclaration")
    int MESSAGE_TYPE_ALL = 0;

    int MESSAGE_TYPE_INBOX = 1;

    int MESSAGE_TYPE_SENT = 2;

    int MESSAGE_TYPE_DRAFT = 3;

    @SuppressWarnings("UnusedDeclaration")
    int MESSAGE_TYPE_OUTBOX = 4;

    @SuppressWarnings("UnusedDeclaration")
    int MESSAGE_TYPE_FAILED = 5; // for failed outgoing messages

    @SuppressWarnings("UnusedDeclaration")
    int MESSAGE_TYPE_QUEUED = 6; // for messages to send later
}