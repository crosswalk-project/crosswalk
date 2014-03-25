// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/application/common/manifest_handlers/widget_handler.h"

#include "xwalk/application/common/application_manifest_constants.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace xwalk {

namespace keys = application_widget_keys;

namespace application {


namespace {
// Below key names are readable from Javascript widget interface.
const char kAuthor[] = "author";
const char kDecription[] = "description";
const char kName[] = "name";
const char kShortName[] = "shortName";
const char kVersion[] = "version";
const char kID[] = "id";
const char kAuthorEmail[] = "authorEmail";
const char kAuthorHref[] = "authorHref";
const char kHeight[] = "height";
const char kWidth[] = "width";
const char kPreferences[] = "preferences";


// Child keys inside 'preferences' key.
const char kPreferencesName[] = "name";
const char kPreferencesValue[] = "value";
const char kPreferencesReadonly[] = "readonly";

//value to test
const char sAuthor[] = "Some one";
const char sDecription[] = "This is a test";
const char sName[] = "widget handler unittest";
const char sShortName[] = "whu";
const char sVersion[] = "0.0.0.1";
const char sID[] = "iiiiiiiiiid";
const char sAuthorEmail[] = "aaa@bbb.com";
const char sAuthorHref[] = "http://www.ssss.com";
const char sHeight[] = "800";
const char sWidth[] = "480";

const char sPreferencesName[3][10] = {"pname0","pname1","pname2"};
const char sPreferencesValue[3][10] = {"pvalue0","pvalue1","pvalue2"};
const char sPreferencesReadonly[3][10] = {"true","false","false"};

typedef std::map<std::string, std::string> SSMap;
typedef std::map<std::string, std::string>::const_iterator SSMapIterator;
typedef std::pair<std::string, std::string> SSPair;


}//Anonymous namespace 


class WidgetHandlerTest: public testing::Test {
 public:
  scoped_refptr<ApplicationData> CreateApplication(base::DictionaryValue &manifest) {
    std::string error;
    scoped_refptr<ApplicationData> application = ApplicationData::Create(
        base::FilePath(), Manifest::INVALID_TYPE, manifest, "", &error);
    return application;
  }

  /*const*/ WidgetInfo* GetWidgetInfo(
      scoped_refptr<ApplicationData> application) {
    /*const */WidgetInfo* info = static_cast<WidgetInfo*>(
        application->GetManifestData(keys::kWidgetKey));
    return info;
  }


  base::DictionaryValue* GetOnePreferences(int id,bool isFullKey){
    base::DictionaryValue* preferences = new base::DictionaryValue;
    if(isFullKey){
      preferences->SetString(keys::kPreferencesNameKey,     sPreferencesName[id]);
      preferences->SetString(keys::kPreferencesValueKey,    sPreferencesValue[id]);
      preferences->SetString(keys::kPreferencesReadonlyKey, sPreferencesReadonly[id]);
    } else {
      preferences->SetString(kPreferencesName,     sPreferencesName[id]);
      preferences->SetString(kPreferencesValue,    sPreferencesValue[id]);
      preferences->SetString(kPreferencesReadonly, sPreferencesReadonly[id]);
    }

    return preferences;
  }

  //No Preferences and full other information
  void SetAllInfoToManifest(base::DictionaryValue* manifest){  
    //Insert some key-value pairs into manifest use full key
    manifest->SetString(keys::kAuthorKey,      sAuthor);
    manifest->SetString(keys::kDescriptionKey, sDecription);
    manifest->SetString(keys::kNameKey,        sName);
    manifest->SetString(keys::kShortNameKey,   sShortName);
    manifest->SetString(keys::kVersionKey,     sVersion);
    manifest->SetString(keys::kIDKey,          sID);
    manifest->SetString(keys::kAuthorEmailKey, sAuthorEmail);
    manifest->SetString(keys::kAuthorHrefKey,  sAuthorHref);
    manifest->SetString(keys::kHeightKey,      sHeight);
    manifest->SetString(keys::kWidthKey,       sWidth);
    
  }

  //No Preferences and full other information
  void SetAllInfoToWidget(base::DictionaryValue* widget){
    //Insert some key-value pairs into widget use widget key;
    widget->SetString(kAuthor,      sAuthor);
    widget->SetString(kDecription,  sDecription);
    widget->SetString(kName,        sName);
    widget->SetString(kShortName,   sShortName);
    widget->SetString(kVersion,     sVersion);
    widget->SetString(kID,          sID);
    widget->SetString(kAuthorEmail, sAuthorEmail);
    widget->SetString(kAuthorHref,  sAuthorHref);
    widget->SetString(kHeight,      sHeight);
    widget->SetString(kWidth,       sWidth);

  }
  //Create an application use manifest,
  //and get a widget from this application,
  //then compare this new widget with 
  //the one it should be.
  void checkEquals(base::DictionaryValue* manifest,
                   base::DictionaryValue* widget){

    scoped_refptr<ApplicationData> application = CreateApplication(*manifest);
    EXPECT_TRUE(application.get());
    EXPECT_EQ(application->GetPackageType(), Manifest::TYPE_WGT);

    /*const */WidgetInfo* info = GetWidgetInfo(application);
    EXPECT_TRUE(info);
    scoped_ptr<base::DictionaryValue> copy(info->GetWidgetInfo()->DeepCopy());
  
    base::DictionaryValue* new_widget;
    copy->GetAsDictionary(&new_widget);

    EXPECT_TRUE(new_widget);
    EXPECT_TRUE(widget->Equals(new_widget));
  }

  typedef scoped_ptr<base::DictionaryValue> spDictValue;
};


TEST_F(WidgetHandlerTest, EmptyWidget) {
  base::DictionaryValue manifest;
  manifest.SetString(keys::kNameKey, "no name");
  manifest.SetString(keys::kVersionKey, "0");

  scoped_refptr<ApplicationData> application = CreateApplication(manifest);
  EXPECT_TRUE(application.get());
  
  /*const */WidgetInfo* info = GetWidgetInfo(application);
  int size = info->GetWidgetInfo()->size();

  //only name and version ,others are empty string "",but exist.
  EXPECT_EQ(size, 10);

  base::DictionaryValue* widget = info->GetWidgetInfo();
  base::DictionaryValue::Iterator it(*widget);


  std::string tmpStr;

  //check value
  while(!it.IsAtEnd()){
    it.value().GetAsString(&tmpStr);

    if( it.key() == kName ){     
      EXPECT_EQ(tmpStr,"no name");

    } else if( it.key() == kVersion ){
      EXPECT_EQ(tmpStr,"0");

    } else {
      EXPECT_EQ(tmpStr,"");

    }

    
    it.Advance();
  }

}

TEST_F(WidgetHandlerTest, OnePreferences) {

  spDictValue manifest(new base::DictionaryValue);
  spDictValue widget(new base::DictionaryValue);
  
  SetAllInfoToManifest(manifest.get());
  SetAllInfoToWidget(widget.get());

  manifest->Set(keys::kPreferencesKey,GetOnePreferences(0,true));
  widget->Set(kPreferences,GetOnePreferences(0,false));

  checkEquals(manifest.get(),widget.get());

}

TEST_F(WidgetHandlerTest, SomePreferences) {

  spDictValue manifest(new base::DictionaryValue);
  spDictValue widget(new base::DictionaryValue);

  SetAllInfoToManifest(manifest.get());
  SetAllInfoToWidget(widget.get());


    base::ListValue* manifestPreferences = new base::ListValue;
    base::ListValue* widgetPreferences   = new base::ListValue;

    for(int i=0;i<3;i++){      
      manifestPreferences->Append(GetOnePreferences(i,true));
      widgetPreferences->Append(GetOnePreferences(i,false));
    }


  manifest->Set(keys::kPreferencesKey,manifestPreferences);
  widget->Set(kPreferences,widgetPreferences);

  checkEquals(manifest.get(),widget.get());

}

}  // namespace application
}  // namespace xwalk
