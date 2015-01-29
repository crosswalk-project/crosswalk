// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.internal.extension.api.contacts;

import android.content.ContentResolver;
import android.database.Cursor;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Event;
import android.provider.ContactsContract.CommonDataKinds.GroupMembership;
import android.provider.ContactsContract.CommonDataKinds.Im;
import android.provider.ContactsContract.CommonDataKinds.Nickname;
import android.provider.ContactsContract.CommonDataKinds.Note;
import android.provider.ContactsContract.CommonDataKinds.Organization;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.Photo;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.CommonDataKinds.StructuredPostal;
import android.provider.ContactsContract.CommonDataKinds.Website;
import android.util.Log;
import android.util.Pair;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * This class searches contacts by given options.
 */
public class ContactFinder {
    private ContactUtils mUtils;
    private static final String TAG = "ContactFinder";

    public ContactFinder(ContentResolver resolver) {
        mUtils = new ContactUtils(resolver);
    }

    public static class FindOption {
        public String mWhere;
        public String[] mWhereArgs;
        public String mSortOrder;

        public FindOption(String where, String[] whereArgs, String sortOrder) {
            mWhere = where;
            mWhereArgs = whereArgs;
            mSortOrder = sortOrder;
        }
    }

    private class ContactData {
        public String lastUpdated;
        public JSONObject oName;
        public JSONArray aEmails;
        public JSONArray aPhotos;
        public JSONArray aUrls;
        public JSONArray aCategories;
        public JSONArray aAddresses;
        public JSONArray aNumbers;
        public JSONArray aOrganizations;
        public JSONArray aJobTitles;
        public String birthday;
        public JSONArray aNotes;
        public JSONArray aImpp;
        public String anniversary;
        public String gender;
        public JSONObject ensurePut(long id) {
            JSONObject o = new JSONObject();
            try {
                o.put("id", id);
            } catch (JSONException e) {
                Log.e(TAG, "ensurePut - Failed to build json data: " + e.toString());
            }
            ensurePut(o, "name", oName);
            ensurePut(o, "lastUpdated", lastUpdated);
            ensurePut(o, "emails", aEmails);
            ensurePut(o, "photos", aPhotos);
            ensurePut(o, "urls", aUrls);
            ensurePut(o, "categories", aCategories);
            ensurePut(o, "addresses", aAddresses);
            ensurePut(o, "phoneNumbers", aNumbers);
            ensurePut(o, "organizations", aOrganizations);
            ensurePut(o, "jobTitles", aJobTitles);
            ensurePut(o, "birthday", birthday);
            ensurePut(o, "notes", aNotes);
            ensurePut(o, "impp", aImpp);
            ensurePut(o, "anniversary", anniversary);
            ensurePut(o, "gender", gender);
            return o;
        }
        private <T> void ensurePut(JSONObject o, String jsonName, T t) {
            try {
                if (t != null) o.put(jsonName, t);
            } catch (JSONException e) {
                Log.e(TAG, "ensurePut - Failed to add json data: " + e.toString());
            }
        }
    }

    private JSONObject addString(JSONObject o, Cursor c, String jsonName, String dataName) {
        try {
            final String value = c.getString(c.getColumnIndex(dataName));
            if (o == null) o = new JSONObject();
            if (value != null) o.put(jsonName, value);
        } catch (JSONException e) {
            Log.e(TAG, "addString - Failed to build json data: " + e.toString());
        }
        return o;
    }

    private JSONArray addString(JSONArray array, Cursor c, String dataName) {
        if (array == null) array = new JSONArray();
        final String value = c.getString(c.getColumnIndex(dataName));
        if (value != null) array.put(value);
        return array;
    }

    private JSONObject addArrayTop(JSONObject o, Cursor c,
            String jsonName, String dataName, Map<String, Integer> typeValuesMap) {
        final String nameString = ContactUtils.getKeyFromValue(
                typeValuesMap, Integer.valueOf(c.getString(c.getColumnIndex(dataName))));
        return ensureAddArrayTop(o, c, jsonName, nameString);
    }

    private JSONObject addArrayTop(JSONObject o, Cursor c,
            String jsonName, String dataName) {
        final String nameString = c.getString(c.getColumnIndex(dataName));
        return ensureAddArrayTop(o, c, jsonName, nameString);
    }

    private JSONObject ensureAddArrayTop(JSONObject o, Cursor c,
            String jsonName, String nameString) {
        try {
            if (o == null) o = new JSONObject();
            if (nameString != null) {
                JSONArray nameArray = new JSONArray();
                nameArray.put(nameString);
                o.put(jsonName, nameArray);
            }
        } catch (JSONException e) {
            Log.e(TAG, "ensureAddArrayTop - Failed to build json data: " + e.toString());
        }
        return o;
    }

    private JSONArray addTypeArray(JSONArray array, Cursor c, String data,
            Map<String, String> typeMap, Map<String, Integer> typeValuesMap) {
        try {
            if (array == null) array = new JSONArray();
            final String primary = typeMap.get("isSuperPrimary");
            final String preferred =
                    (c.getString(c.getColumnIndex(primary)).equals("1")) ? "true" : "false";
            JSONObject o = new JSONObject();
            o.put("preferred", preferred);
            addArrayTop(o, c, "types", typeMap.get("type"), typeValuesMap);
            String value = c.getString(c.getColumnIndex(data));
            if (c.getString(c.getColumnIndex(Data.MIMETYPE)).equals(Im.CONTENT_ITEM_TYPE)) {
                int protocol = c.getInt(c.getColumnIndex(Im.PROTOCOL));
                String prefix = ContactUtils.getKeyFromValue(ContactConstants.imProtocolMap, protocol);
                value = prefix + ':' + value;
            }
            o.put("value", value);
            array.put(o);
        } catch (JSONException e) {
            Log.e(TAG, "addTypeArray - Failed to build json data: " + e.toString());
        }
        return array;
    }

    private Set<String> getContactIds(FindOption findOption) {
        Set<String> ids = null;
        Cursor c = null;
        try {
            c = mUtils.mResolver.query(Data.CONTENT_URI, null, findOption.mWhere,
                                       findOption.mWhereArgs, findOption.mSortOrder);
            ids = new HashSet<String>();
            while (c.moveToNext()) {
                ids.add(String.valueOf(c.getLong(c.getColumnIndex(Data.CONTACT_ID))));
            }
            return ids;
        } catch (SecurityException e) {
            Log.e(TAG, "getContactIds: " + e.toString());
            return null;
        } finally {
            if (c != null) c.close();
        }
    }

    /**
     * Generate a string to be used as sort order by SQL clause
     * @param sortBy e.g. "givenNames", "phoneNumbers"
     * @param sortOrder e.g. "descending"
     * @return Data name with SQL clause: "StructuredName.GIVEN_NAME DESC, Phone.NUMBER DESC"
     */
    private String getSortOrder(List<String> sortBy, String sortOrder) {
        if (sortOrder == null) return null;

        String suffix = "";
        if (sortOrder.equals("ascending")) {
            suffix = " ASC";
        } else if (sortOrder.equals("descending")) {
            suffix = " DESC";
        }

        String order = "";
        for (String s : sortBy) {
            Pair<String, String> fields = ContactConstants.contactDataMap.get(s);
            if (fields == null) continue;
            order += fields.first + suffix + ",";
        }
        return (order != "") ? order.substring(0, order.length()-1) : null;
    }

    //TODO(hdq): Currently this function doesn't support multi-column sorting.
    private JSONArray getContacts(
            Set<String> contactIds, String sortOrder, String sortByMimeType, Long resultsLimit) {
        // Get all records of given contactIds.
        // For example, sort by ascending:
        // -----------------------------
        // id  data           mimetype
        // -----------------------------
        // 60  +34600000000   phone_v2
        // 59  +34698765432   phone_v2
        // 59  David1 Smith   name
        // 60  David2 Smith   name
        String where = null;
        String[] whereArgs = null;
        if (contactIds.size() != 0) {
            where = Data.CONTACT_ID + " in (" + ContactUtils.makeQuestionMarkList(contactIds) + ")";
            whereArgs = contactIds.toArray(new String[contactIds.size()]);
        }
        Cursor c = null;
        Map<Long, ContactData> dataMap = null;
        try {
            c = mUtils.mResolver.query(Data.CONTENT_URI, null, where, whereArgs, sortOrder);
            dataMap = new LinkedHashMap<Long, ContactData>();

            // Read contact IDs to build the array by sortOrder.
            if (sortOrder != null) {
                while (c.moveToNext()) {
                    // We should only check for mimetype of sorting field.
                    // As e.g. above, sort by phone number: [60, 59], by name: [59, 60]
                    String mime = c.getString(c.getColumnIndex(Data.MIMETYPE));
                    if (!mime.equals(sortByMimeType)) continue;
                    long id = c.getLong(c.getColumnIndex(Data.CONTACT_ID));
                    if (!dataMap.containsKey(id)) dataMap.put(id, new ContactData());
                }
                c.moveToFirst();
            }

            // Read details of each contacts
            while (c.moveToNext()) {
                long id = c.getLong(c.getColumnIndex(Data.CONTACT_ID));
                if (!dataMap.containsKey(id)) dataMap.put(id, new ContactData());
                ContactData d = dataMap.get(id);
                if (d.lastUpdated == null && VERSION.SDK_INT >= VERSION_CODES.JELLY_BEAN_MR2) {
                    d.lastUpdated = mUtils.getLastUpdated(id);
                }
                String mime = c.getString(c.getColumnIndex(Data.MIMETYPE));
                if (mime.equals(StructuredName.CONTENT_ITEM_TYPE)) {
                    d.oName = addString(d.oName, c, "displayName", StructuredName.DISPLAY_NAME);
                    d.oName = addArrayTop(d.oName, c, "honorificPrefixes", StructuredName.PREFIX);
                    d.oName = addArrayTop(d.oName, c, "givenNames", StructuredName.GIVEN_NAME);
                    d.oName = addArrayTop(d.oName, c, "additionalNames", StructuredName.MIDDLE_NAME);
                    d.oName = addArrayTop(d.oName, c, "familyNames", StructuredName.FAMILY_NAME);
                    d.oName = addArrayTop(d.oName, c, "honorificSuffixes", StructuredName.SUFFIX);
                } else if (mime.equals(Nickname.CONTENT_ITEM_TYPE)) {
                    d.oName = addArrayTop(d.oName, c, "nicknames", Nickname.NAME);
                } else if (mime.equals(Email.CONTENT_ITEM_TYPE)) {
                    d.aEmails = addTypeArray(d.aEmails, c, Email.DATA,
                                             ContactConstants.emailTypeMap,
                                             ContactConstants.emailTypeValuesMap);
                } else if (mime.equals(Photo.CONTENT_ITEM_TYPE)) {
                    d.aPhotos = addString(d.aPhotos, c, Photo.PHOTO);
                } else if (mime.equals(Website.CONTENT_ITEM_TYPE)) {
                    d.aUrls = addTypeArray(d.aUrls, c, Website.DATA,
                                           ContactConstants.websiteTypeMap,
                                           ContactConstants.websiteTypeValuesMap);
                } else if (mime.equals(GroupMembership.CONTENT_ITEM_TYPE)) {
                    String title = mUtils.getGroupTitle(
                            c.getString(c.getColumnIndex(GroupMembership.GROUP_ROW_ID)));
                    if (title != null) {
                        if (d.aCategories == null) d.aCategories = new JSONArray();
                        d.aCategories.put(title);
                    }
                } else if (mime.equals(StructuredPostal.CONTENT_ITEM_TYPE)) {
                    d.aAddresses = addTypeArray(d.aAddresses, c, StructuredPostal.DATA,
                                                ContactConstants.addressTypeMap,
                                                ContactConstants.addressTypeValuesMap);
                } else if (mime.equals(Phone.CONTENT_ITEM_TYPE)) {
                    d.aNumbers = addTypeArray(d.aNumbers, c, Phone.DATA,
                                              ContactConstants.phoneTypeMap,
                                              ContactConstants.phoneTypeValuesMap);
                } else if (mime.equals(Organization.CONTENT_ITEM_TYPE)) {
                    d.aOrganizations = addString(d.aOrganizations, c, Organization.COMPANY);
                } else if (mime.equals(Organization.CONTENT_ITEM_TYPE)) {
                    d.aJobTitles = addString(d.aJobTitles, c, Organization.TITLE);
                } else if (mime.equals(Event.CONTENT_ITEM_TYPE)) {
                    int type = Integer.valueOf(c.getString(c.getColumnIndex(Event.TYPE)));
                    if (type == Event.TYPE_BIRTHDAY) {
                        d.birthday = c.getString(c.getColumnIndex(Event.START_DATE));
                    } else if (type == Event.TYPE_ANNIVERSARY) {
                        d.anniversary = c.getString(c.getColumnIndex(Event.START_DATE));
                    }
                } else if (mime.equals(Note.CONTENT_ITEM_TYPE)) {
                    d.aNotes = addString(d.aNotes, c, Note.NOTE);
                } else if (mime.equals(Im.CONTENT_ITEM_TYPE)) {
                    d.aImpp = addTypeArray(d.aImpp, c, Im.DATA, ContactConstants.imTypeMap,
                                           ContactConstants.imTypeValuesMap);
                } else if (mime.equals(ContactConstants.CUSTOM_MIMETYPE_GENDER)) {
                    d.gender = c.getString(c.getColumnIndex(Data.DATA1));
                }
            }
        } catch (Exception e) {
            if (e instanceof NumberFormatException || e instanceof SecurityException) {
                Log.e(TAG, "getContacts: " + e.toString());
                return new JSONArray();
            } else {
                throw new RuntimeException(e);
            }
        } finally {
            if (c != null) c.close();
        }

        int i = 0;
        JSONArray returnArray = new JSONArray();
        for (Map.Entry<Long, ContactData> entry : dataMap.entrySet()) {
            if (resultsLimit != null && ++i > resultsLimit) break;
            JSONObject o = entry.getValue().ensurePut(entry.getKey());
            returnArray.put(o);
        }
        return returnArray;
    }

    private FindOption createFindIDOption(String findString) {
        ContactJson findJson = new ContactJson(findString);
        String value = (findString != null) ? findJson.getString("value") : null;
        if (value == null || value.equals("") || findString.equals("")) {
            return new FindOption(null, null, null);
        } else {
            List<String> args = new ArrayList<String>();
            List<String> fields = findJson.getStringArray("fields");
            String operator = findJson.getString("operator");
            if (operator == null) {
                return new FindOption(null, null, null);
            } else if (operator.equals("is")) {
                operator = " = ";
            } else if (operator.equals("contains")) {
                operator = " LIKE ";
                value = "%" + value + "%";
            } else {
                Log.e(TAG, "find - Wrong Operator: ["+operator+"], should be 'is' or 'contains'");
                return null;
            }
            String where = "";
            for (String field : fields) {
                // E.g. for "givenName" should check column of "givenNames".
                String column = ContactConstants.findFieldMap.get(field);
                // Skip invalid fields
                if (column == null) continue;
                android.util.Pair<String, String> name = ContactConstants.contactDataMap.get(column);
                // E.g. first is GIVEN_NAME, second is StructuredName.MIMETYPE
                where += name.first + operator + " ? AND " + Data.MIMETYPE + " = ? or ";
                args.add(value);
                args.add(name.second);
            }
            if (where == "") return new FindOption(null, null, null);
            // Remove the "or " which appended in the loop above.
            where = where.substring(0, where.length()-3);
            String[] whereArgs = args.toArray(new String[args.size()]);
            return new FindOption(where, whereArgs, null);
        }
    }

    public JSONArray find(String findString) {
        Set<String> ids = getContactIds(createFindIDOption(findString));
        if (ids == null) return new JSONArray();
        ContactJson findJson = new ContactJson(findString);
        List<String> sortBy = findJson.getStringArray("sortBy");
        String order = getSortOrder(sortBy, findJson.getString("sortOrder"));
        String orderMimeType = (order == null) ? null :
                ContactConstants.contactDataMap.get(sortBy.get(0)).second;
        String resultsLimit = findJson.getString("resultsLimit");
        Long resultsLimitLong = (resultsLimit == null) ? null : Long.valueOf(resultsLimit);
        return getContacts(ids, order, orderMimeType, resultsLimitLong);
    }
}
