// Copyright (C) 2013-2016 iFunFactory Inc. All Rights Reserved.
//
// This work is confidential and proprietary to iFunFactory Inc. and
// must not be used, disclosed, copied, or distributed without the prior
// consent of iFunFactory Inc.

// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT EDIT.


#include "../pong_object.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/assign.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <funapi.h>
#include <glog/logging.h>

#include <ctime>
#include <functional>
#include <map>


namespace pong {

using fun::ApiService;
using fun::string;


template<typename ObjectType, typename AttributeType>
bool CompareAttribute(const Ptr<ObjectType> &object, const AttributeType &value, const function<AttributeType(const Ptr<ObjectType> &)> &attribute_getter, MatchCondition cond) {
  if (not object) {
    return false;
  }

  if (cond == kEqual) {
    return value == attribute_getter(object);
  } else if (cond == kLess) {
    return value > attribute_getter(object);
  } else if (cond == kGreater) {
    return value < attribute_getter(object);
  }
  BOOST_ASSERT(false);
  return false;
}


template<typename ObjectType, typename AttributeType>
bool CompareAttribute2(const Ptr<ObjectType> &object, const function<bool(const AttributeType &)> &match, const function<AttributeType(const Ptr<ObjectType> &)> &attribute_getter) {
  if (not object) {
    return false;
  }
  return match(attribute_getter(object));
}


template <>
bool ArrayRef<bool>::GetAt(size_t index) const {
  return owner_->GetArrayElementBoolean(attribute_name_, index);
}


template <>
int64_t ArrayRef<int64_t>::GetAt(size_t index) const {
  return owner_->GetArrayElementInteger(attribute_name_, index);
}


template <>
double ArrayRef<double>::GetAt(size_t index) const {
  return owner_->GetArrayElementDouble(attribute_name_, index);
}


template <>
string ArrayRef<string>::GetAt(size_t index) const {
  return owner_->GetArrayElementString(attribute_name_, index);
}


template <>
Object::Id ArrayRef<Object::Id>::GetAt(size_t index) const {
  return owner_->GetArrayElementObject(attribute_name_, index);
}


template <>
Ptr<User> ArrayRef<Ptr<User> >::GetAt(size_t index) const {
  const Object::Id &object_id = owner_->GetArrayElementObject(attribute_name_, index);
  if (object_id == Object::kNullId) {
    return User::kNullPtr;
  }

  BOOST_ASSERT(lock_type_ != kNoneLock);

  return User::Fetch(object_id, lock_type_);
}


template <>
void ArrayRef<bool>::SetAt(size_t index, const bool &value) {
  owner_->SetArrayElementBoolean(attribute_name_, index, value);
}


template <>
void ArrayRef<int64_t>::SetAt(size_t index, const int64_t &value) {
  owner_->SetArrayElementInteger(attribute_name_, index, value);
}


template <>
void ArrayRef<double>::SetAt(size_t index, const double &value) {
  owner_->SetArrayElementDouble(attribute_name_, index, value);
}


template <>
void ArrayRef<string>::SetAt(size_t index, const string &value) {
  owner_->SetArrayElementString(attribute_name_, index, value);
}


template <>
void ArrayRef<Object::Id>::SetAt(size_t index, const Object::Id &value) {
  owner_->SetArrayElementObject(attribute_name_, index, value);
}


template <>
void ArrayRef<Ptr<User> >::SetAt(size_t index, const Ptr<User> &value) {
  if (value) {
    owner_->SetArrayElementObject(attribute_name_, index, value->Id());
  } else {
    owner_->SetArrayElementObject(attribute_name_, index, Object::kNullId);
  }
}


template <>
void ArrayRef<bool>::InsertAt(size_t index, const bool &value) {
  owner_->InsertArrayElementBoolean(attribute_name_, index, value);
}


template <>
void ArrayRef<int64_t>::InsertAt(size_t index, const int64_t &value) {
  owner_->InsertArrayElementInteger(attribute_name_, index, value);
}


template <>
void ArrayRef<double>::InsertAt(size_t index, const double &value) {
  owner_->InsertArrayElementDouble(attribute_name_, index, value);
}


template <>
void ArrayRef<string>::InsertAt(size_t index, const string &value) {
  owner_->InsertArrayElementString(attribute_name_, index, value);
}


template <>
void ArrayRef<Object::Id>::InsertAt(size_t index, const Object::Id &value) {
  owner_->InsertArrayElementObject(attribute_name_, index, value);
}


template <>
void ArrayRef<Ptr<User> >::InsertAt(size_t index, const Ptr<User> &value) {
  if (value) {
    owner_->InsertArrayElementObject(attribute_name_, index, value->Id());
  } else {
    owner_->InsertArrayElementObject(attribute_name_, index, Object::kNullId);
  }
}


template <>
bool ArrayRef<bool>::Front() const {
  return GetAt(0);
}


template <>
int64_t ArrayRef<int64_t>::Front() const {
  return GetAt(0);
}


template <>
double ArrayRef<double>::Front() const {
  return GetAt(0);
}


template <>
string ArrayRef<string>::Front() const {
  return GetAt(0);
}


template <>
Object::Id ArrayRef<Object::Id>::Front() const {
  return GetAt(0);
}


template <>
Ptr<User> ArrayRef<Ptr<User> >::Front() const {
  return GetAt(0);
}


template <>
bool ArrayRef<bool>::Back() const {
  return GetAt(Size() - 1);
}


template <>
int64_t ArrayRef<int64_t>::Back() const {
  return GetAt(Size() - 1);
}


template <>
double ArrayRef<double>::Back() const {
  return GetAt(Size() - 1);
}


template <>
string ArrayRef<string>::Back() const {
  return GetAt(Size() - 1);
}


template <>
Object::Id ArrayRef<Object::Id>::Back() const {
  return GetAt(Size() - 1);
}


template <>
Ptr<User> ArrayRef<Ptr<User> >::Back() const {
  return GetAt(Size() - 1);
}


template <>
void ArrayRef<bool>::PushFront(const bool &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<int64_t>::PushFront(const int64_t &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<double>::PushFront(const double &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<string>::PushFront(const string &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<Object::Id>::PushFront(const Object::Id &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<Ptr<User> >::PushFront(const Ptr<User> &value) {
  InsertAt(0, value);
}


template <>
void ArrayRef<bool>::PushBack(const bool &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<int64_t>::PushBack(const int64_t &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<double>::PushBack(const double &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<string>::PushBack(const string &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<Object::Id>::PushBack(const Object::Id &value) {
  InsertAt(Size(), value);
}


template <>
void ArrayRef<Ptr<User> >::PushBack(const Ptr<User> &value) {
  InsertAt(Size(), value);
}


namespace {

void ConvertArrayRefToVector(const ArrayRef<Object::Id> &array, std::vector<Object::Id> *out) {
  size_t size = array.Size();
  if (size == 0) {
    return;
  }
  out->reserve(size);

  for (size_t i = 0; i < size; ++i) {
    out->push_back(array.GetAt(i));
  }
}


template <typename ObjectType>
void FetchObjectFromVector(const std::vector<Object::Id> &object_ids, bool include_null_object, LockType lock_type, std::vector<Ptr<ObjectType> > *out) {
  if (object_ids.empty()) {
    return;
  }

  std::vector<std::pair<Object::Id, Ptr<ObjectType> > > object_pairs;
  ObjectType::Fetch(object_ids, &object_pairs, lock_type);
  BOOST_ASSERT(object_ids.size() == object_pairs.size());

  if (out == NULL) {
    return;
  }

  for (size_t i = 0; i < object_pairs.size(); ++i) {
    if (include_null_object || object_pairs[i].second) {
      out->push_back(object_pairs[i].second);
    }
  }
}


template <typename ObjectType>
void FetchObjectFromArray(const ArrayRef<Object::Id> &array, bool include_null_object, LockType lock_type, std::vector<Ptr<ObjectType> > *out) {
  std::vector<Object::Id> object_ids;
  ConvertArrayRefToVector(array, &object_ids);
  FetchObjectFromVector(object_ids, include_null_object, lock_type, out);
}


template <typename KeyType>
void ConvertMapRefToVector(const MapRef<KeyType, Object::Id> &map, std::vector<Object::Id> *out) {
  std::vector<KeyType> keys = map.Keys();
  if (keys.empty()) {
    return;
  }

  out->reserve(keys.size());

  for (size_t i = 0; i < keys.size(); ++i) {
    out->push_back(map.GetAt(keys[i]));
  }
}


template <typename KeyType, typename ObjectType>
void FetchObjectFromMap(const MapRef<KeyType, Object::Id> &map, bool include_null_object, LockType lock_type, std::vector<Ptr<ObjectType> > *out) {
  std::vector<Object::Id> object_ids;
  ConvertMapRefToVector(map, &object_ids);
  FetchObjectFromVector(object_ids, include_null_object, lock_type, out);
}


}  // unnamed namespace


struct User::OpaqueData {
  OpaqueData() {
    win_count = 0;
    lose_count = 0;
  }

  int64_t win_count;
  int64_t lose_count;
};


DEFINE_CLASS_PTR(User);


function<bool(const Ptr<User> &)> User::MatchByObjectId(const Object::Id &object_id) {
  function<Object::Id(const Ptr<User> &)> obj_id_getter = bind(&User::Id, _1);
  return bind(&CompareAttribute<User, Object::Id>, _1, object_id, obj_id_getter, kEqual);
}


function<bool(const Ptr<User> &)> User::MatchById(const string &id, MatchCondition cond) {
  function<string(const Ptr<User> &)> attribute_getter = bind(&User::GetId, _1);
  return bind(&CompareAttribute<User, string>, _1, id, attribute_getter, cond);
}


function<bool(const Ptr<User> &)> User::MatchByWinCount(const int64_t &win_count, MatchCondition cond) {
  function<int64_t(const Ptr<User> &)> attribute_getter = bind(&User::GetWinCount, _1);
  return bind(&CompareAttribute<User, int64_t>, _1, win_count, attribute_getter, cond);
}


function<bool(const Ptr<User> &)> User::MatchByLoseCount(const int64_t &lose_count, MatchCondition cond) {
  function<int64_t(const Ptr<User> &)> attribute_getter = bind(&User::GetLoseCount, _1);
  return bind(&CompareAttribute<User, int64_t>, _1, lose_count, attribute_getter, cond);
};


void User::RegisterIdTrigger(const TriggerCondition &condition, const TriggerAction &action) {
  Object::RegisterAttributeTrigger("User", "Id", condition, action);
}


void User::RegisterWinCountTrigger(const TriggerCondition &condition, const TriggerAction &action) {
  Object::RegisterAttributeTrigger("User", "WinCount", condition, action);
}


void User::RegisterLoseCountTrigger(const TriggerCondition &condition, const TriggerAction &action) {
  Object::RegisterAttributeTrigger("User", "LoseCount", condition, action);
}


void User::SelectById(const Object::ConditionType &cond_type, const string &cond_value, const Object::SelectCallback &callback) {
  Object::Select("User", "Id", "", cond_type, AttributeValue(cond_value), callback);
}


void User::SelectByWinCount(const Object::ConditionType &cond_type, const int64_t &cond_value, const Object::SelectCallback &callback) {
  Object::Select("User", "WinCount", "", cond_type, AttributeValue(cond_value), callback);
}


void User::SelectByLoseCount(const Object::ConditionType &cond_type, const int64_t &cond_value, const Object::SelectCallback &callback) {
  Object::Select("User", "LoseCount", "", cond_type, AttributeValue(cond_value), callback);
}


Ptr<User::OpaqueData> User::CreateOpaqueDataFromJson(const Json &json) {
  if (not json.IsObject()) {
    LOG(ERROR) << "not object type";
    return Ptr<OpaqueData>();
  }

  Ptr<OpaqueData> data(new OpaqueData);

  if (json.HasAttribute("WinCount")) {
    if (not json["WinCount"].IsInteger()) {
      LOG(ERROR) << "wrong 'WinCount' value: type mismatch";
      return Ptr<OpaqueData>();
    }
    data->win_count = json["WinCount"].GetInteger();
  }
  if (json.HasAttribute("LoseCount")) {
    if (not json["LoseCount"].IsInteger()) {
      LOG(ERROR) << "wrong 'LoseCount' value: type mismatch";
      return Ptr<OpaqueData>();
    }
    data->lose_count = json["LoseCount"].GetInteger();
  }
  return data;
}


Ptr<User> User::Create(const string &id) {
  AttributeValueMap key_params;
  key_params["Id"].reset(new AttributeValue(id));

  const Ptr<const ObjectModel> &model(ObjectModel::FindModel("User"));
  BOOST_ASSERT(model);

  // try to fetch by same key values.
  if (FetchById(id, kReadLock)) {
    // key value is duplicated.
    return kNullPtr;
  }

  Ptr<Object> obj = Object::Create(model, key_params);
  if (not obj) {
    // key value is duplicated.
    return kNullPtr;
  }

  return Ptr<User>(new User(obj));
}


Ptr<User> User::Fetch(
    const Object::Id &id,
    LockType lock_type) {
  const Ptr<const ObjectModel> &model(ObjectModel::FindModel("User"));
  BOOST_ASSERT(model);

  Ptr<Object> obj = Object::Fetch(model, id, lock_type);
  if (not obj)
    return kNullPtr;

  return Ptr<User>(new User(obj));
}


void User::Fetch(
    const std::vector<Object::Id> &ids,
    std::vector<std::pair<Object::Id, Ptr<User> > > *result,
    LockType lock_type) {
  BOOST_ASSERT(result);
  BOOST_ASSERT(result->empty());

  const Ptr<const ObjectModel> &model(ObjectModel::FindModel("User"));
  BOOST_ASSERT(model);

  std::vector<std::pair<Object::Id, Ptr<Object> > > objs;
  objs.reserve(ids.size());
  Object::Fetch(model, ids, lock_type, &objs);
  BOOST_ASSERT(objs.size() == ids.size());

  result->reserve(ids.size());
  for (size_t i = 0; i < objs.size(); ++i) {
    Ptr<User> wrapped_obj;
    if (objs[i].second)
      wrapped_obj.reset(new User(objs[i].second));
    result->push_back(std::make_pair(objs[i].first, wrapped_obj));
  }

  BOOST_ASSERT(result->size() == ids.size());
}


Ptr<User> User::FetchById(const string &value, LockType lock_type) {
  Ptr<AttributeValue> key_value(new AttributeValue(value));
  const Ptr<const ObjectModel> &model(ObjectModel::FindModel("User"));
  BOOST_ASSERT(model);

  Ptr<Object> obj = Object::Fetch(model, "Id",  key_value, lock_type);
  if (not obj)
    return kNullPtr;

  return Ptr<User>(new User(obj));
}


void User::FetchById(
    const std::vector<string> &values,
    std::vector<std::pair<string, Ptr<User> > > *result,
    LockType lock_type) {
  BOOST_ASSERT(result);
  BOOST_ASSERT(result->empty());

  std::vector<Ptr<AttributeValue> > key_values;
  key_values.reserve(values.size());
  for (size_t i = 0; i < values.size(); ++i) {
    key_values.push_back(Ptr<AttributeValue>(new AttributeValue(values[i])));
  }

  const Ptr<const ObjectModel> &model(ObjectModel::FindModel("User"));
  BOOST_ASSERT(model);

  std::vector<std::pair<Ptr<AttributeValue>, Ptr<Object> > > objs;
  Object::Fetch(model, "Id", key_values, lock_type, &objs);
  BOOST_ASSERT(objs.size() == key_values.size());

  result->reserve(values.size());
  for (size_t i = 0; i < objs.size(); ++i) {
    Ptr<User> wrapped_obj;
    if (objs[i].second)
      wrapped_obj.reset(new User(objs[i].second));
    BOOST_ASSERT(AttributeValue(values[i]) == *objs[i].first);
    result->push_back(std::make_pair(values[i], wrapped_obj));
  }

  BOOST_ASSERT(result->size() == values.size());
}


void User::FetchRandomly(
    size_t count, std::vector<Ptr<User> > *result,
    LockType lock_type) {
  const Ptr<const ObjectModel> &model(ObjectModel::FindModel("User"));

  std::vector<Ptr<Object> > raw_objects;
  Object::FetchRandomly(model, count, lock_type, &raw_objects);
  for (size_t i = 0; i < raw_objects.size(); ++i) {
    BOOST_ASSERT(raw_objects[i]);
    result->push_back(Ptr<User>(new User(raw_objects[i])));
  }
}


bool User::IsNull() const {
  return ObjectProxy::IsNull();
}


bool User::IsFresh() const {
  return ObjectProxy::IsFresh();
}


bool User::Refresh() {
  return ObjectProxy::Refresh();
}


void User::ToJson(Json *output) const {
  if (output) {
    DumpJson(*this, output);
  }
}


const Object::Id &User::Id() const {
  BOOST_ASSERT(object());
  return object()->id();
}


void User::WriteImmediately() {
  object()->WriteImmediately();
}


void User::Delete() {
  object()->Delete();
  BOOST_ASSERT(IsNull());
}


bool User::PopulateFrom(const Ptr<OpaqueData> &opaque_data) {
  SetWinCount(opaque_data->win_count);
  SetLoseCount(opaque_data->lose_count);
  return true;
}


string User::GetId() const {
  BOOST_ASSERT(object());
  return object()->GetString("Id");
}


void User::SetId(const string &value) {
  BOOST_ASSERT(object());object()->SetString("Id", value);}


int64_t User::GetWinCount() const {
  BOOST_ASSERT(object());
  return object()->GetInteger("WinCount");
}


void User::SetWinCount(const int64_t &value) {
  BOOST_ASSERT(object());object()->SetInteger("WinCount", value);}


int64_t User::GetLoseCount() const {
  BOOST_ASSERT(object());
  return object()->GetInteger("LoseCount");
}


void User::SetLoseCount(const int64_t &value) {
  BOOST_ASSERT(object());object()->SetInteger("LoseCount", value);}


User::User(const Ptr<Object> &object)
    : ObjectProxy(object) {
}


void RegisterUserModel() {
  AttributeModelVector attrs;
  // not attribute.object
  attrs.push_back(Ptr<const AttributeModel>(
      new AttributeModel(
          // not attribute.map
          "Id", fun::AttributeModel::kString, 64, true, false, false, false, false)));
  // not attribute.object
  attrs.push_back(Ptr<const AttributeModel>(
      new AttributeModel(
          // not attribute.map
          "WinCount", fun::AttributeModel::kInteger, 0, false, false, false, false, false)));
  // not attribute.object
  attrs.push_back(Ptr<const AttributeModel>(
      new AttributeModel(
          // not attribute.map
          "LoseCount", fun::AttributeModel::kInteger, 0, false, false, false, false, false)));

  Ptr<const ObjectModel> model(new ObjectModel("User", attrs));
  ObjectModel::AddModel(model);

  // Regiters counter.
  UpdateCounter(
      "funapi_object_model",
      "User",
      "The number of active User objects in memory",
      0);
}


void ObjectModelInit() {
  RegisterUserModel();

  Object::ObjectModelInit();
}


// User dumper
void DumpJson(const User &obj, Json *dest) {
  if (not dest) {
    return;
  }

  Json &out = *dest;
  out["Id"].SetString(obj.GetId());
  out["WinCount"].SetInteger(obj.GetWinCount());
  out["LoseCount"].SetInteger(obj.GetLoseCount());
}


void DumpJson(const Ptr<const User> &obj, Json *dest) {
  if (obj) {
    DumpJson(*obj, dest);
  }
}


#ifdef ENABLE_IFUN_DEPLOY_COMPATIBILITY
namespace cs_api {

using fun::Json;
using fun::http::Request;
using fun::http::Response;

typedef fun::ApiService::MatchResult Params;


namespace {
static CsApiHandler *g_handler = NULL;
}


CsApiHandler::CsApiHandler() : schemas_(boost::assign::map_list_of
    ("User", "{\"User\": {\"propertylist\": [{\"readonly\": true, \"type\": \"string\", \"name\": \"Id\", \"key\": true}, {\"type\": \"integer\", \"name\": \"WinCount\"}, {\"type\": \"integer\", \"name\": \"LoseCount\"}], \"type\": \"object\"}}").convert_to_container<boost::unordered_map<string, string> >()),
  getters_(boost::assign::map_list_of("User", FetchUser).convert_to_container<CsApiHandler::getter_map>())
{
}


CsApiHandler::~CsApiHandler() {
}


bool CsApiHandler::GetSchemaList(std::vector<std::string> *result) {
  if (!result)
    return false;
  for (schema_map::const_iterator i = schemas_.begin(), ie = schemas_.end();
        i != ie; ++i)
    result->push_back(i->first);
  return true;
}


bool CsApiHandler::ShowSchema(const string &name, string *result) {
  if (!result)
    return false;
  schema_map::const_iterator it = schemas_.find(name);
  if (it != schemas_.end())
    result->assign(it->second);
  return true;
}


bool CsApiHandler::GetAccountTypes(std::vector<std::string>*) {
  return false;
}


bool CsApiHandler::GetAccount(const string&, const string&, Json*) {
  return false;
}


bool CsApiHandler::GetAccountCash(const string&, const string&, Json*) {
  return false;
}

bool CsApiHandler::UpdateAccountCash(
    const string&, const string&, const Json&, Json*) {
  return false;
}


bool CsApiHandler::GetAccountBillingHistory(const std::string &account_type,
      const std::string &uid, int64_t from_ts, int64_t until_ts,
      fun::Json *result) {
  return false;
}


const std::string &CsApiHandler::GetBillerUrl() const {
  static std::string _url;
  return _url;
}


bool CsApiHandler::GetHistoryFromBiller(const std::string &key, int64_t from_ts,
    int64_t until_ts, fun::Json *result) const {
  std::string url = GetBillerUrl();
  if (url.empty())
    return false;

  if (*url.rend() != '/')
    url += '/';

  url += key + "/?from=" + boost::lexical_cast<std::string>(from_ts)
      + "&until=" + boost::lexical_cast<std::string>(until_ts);

  // get response from iFunEngine Biller
  Json response;
  fun::HttpClient client;
  client.set_verbose(false);
  client.Get(url);
  const fun::http::Response &res = client.response();
  if (res.status_code != 200
      || !response.FromString(res.body)
      || !response.HasAttribute("receipts")
      || !response["receipts"].IsArray()) {
    result->SetNull();
    return true;
  }

  Json &receipts = response["receipts"];
  Json &out = (*result)["billing_history"];
  out.SetArray();
  const unsigned len = receipts.Size();
  for (unsigned i = 0; i < len; ++i) {
    out.PushBack();
    Json &dst = out[i];
    Json &src = receipts[i];

    if (src.HasAttribute("product_id"))
      dst["product_id"].SetString(src["product_id"].GetString());

    if (src.HasAttribute("quantity"))
      dst["quantity"].SetInteger(src["quantity"].GetInteger());
    else
      dst["quantity"].SetInteger(1);

    if (src.HasAttribute("purchase_timestamp"))
      dst["store_timestamp"].SetInteger(
          src["purchase_timestamp"].GetInteger());
    if (src.HasAttribute("insert_timestamp"))
      dst["server_timestamp"].SetInteger(src["insert_timestamp"].GetInteger());
  }

  return true;
}


bool CsApiHandler::GetData(const string& schema_type, const string &key,
    Json *result) {
  if (!result)
    return false;
  getter_map::const_iterator func = getters_.find(schema_type);
  BOOST_ASSERT(func != getters_.end());
  if (func == getters_.end())
    return false;

  if (!func->second(key, *result))
    result->SetNull();
  return true;
}




// User
bool FetchUser(const std::string &key, Json &out) {
  Ptr<User> obj = User::FetchById(key);

  if (!obj)
    return false;
  DumpJson(*obj, &out);
  return true;
}


void HandleSchemaList(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "GET /v1/schema/";
  std::vector<std::string> schema_list;
  if (!g_handler->GetSchemaList(&schema_list)) {
    response->status_code = fun::http::kNotImplemented;
    response->body = "not implemented";
    return;
  }

  response->status_code = fun::http::kOk;
  Json msg;
  msg["schemas"].SetArray();
  Json &schemas = msg["schemas"];
  for (std::vector<string>::const_iterator i = schema_list.begin(),
        ie = schema_list.end(); i != ie; ++i)
    schemas.PushBack(*i);
  response->body = msg.ToString();
}


void HandleSchemaGet(Ptr<Response> response, const Request &request,
    const Params &params) {
  response->status_code = fun::http::kOk;
  DLOG(INFO) << "GET /v1/schema/{" << params[1] << "}";
  if (!g_handler->ShowSchema(params[1], &(response->body))) {
    response->status_code = fun::http::kNotImplemented;
    response->body = "not implemented";
    return;
  }

  if (response->body.empty()) {
    response->status_code = fun::http::kNotFound;
    response->body = "not found";
    return;
  }
}


void HandleAccountList(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "GET /v1/account";
  response->status_code = fun::http::kOk;
  Json msg;
  Json &accounts = msg["accounts"];
  accounts.SetArray();
  std::vector<string> _accounts;
  if (!g_handler->GetAccountTypes(&_accounts)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  for (std::vector<string>::const_iterator i = _accounts.begin(),
        ie = _accounts.end(); i != ie; ++i)
    accounts.PushBack(*i);
  response->body = msg.ToString();
}


void HandleAccountGet(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "GET /v1/account/{" << params[1] << "}/{" << params[2] << "}";

  Json msg;
  Json &out = msg["account"];
  out.SetObject();
  if (!g_handler->GetAccount(params[1], params[2], &out)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }
  if (out.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }

  msg["account_type"].SetString(params[1]);
  msg["account_id"].SetString(params[2]);
  response->status_code = fun::http::kOk;
  response->body = msg.ToString();
  return;
}


void HandleAccountGetCash(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "GET /v1/account/{" << params[1] << "}/{" << params[2]
      << "}/cash";

  Json obj;
  obj.SetObject();
  if (!g_handler->GetAccountCash(params[1], params[2], &obj)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  if (obj.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }

  response->status_code = fun::http::kOk;
  response->body = obj.ToString();
  return;
}


void HandleAccountGetBillingHistory(Ptr<Response> response,
    const Request &request, const Params &params) {
  DLOG(INFO) << "GET /v1/account/{" << params[1] << "}/{" << params[2]
      << "}/billing-history";

  int64_t until_ts = std::time(NULL);
  int64_t from_ts = until_ts - (86400 * 7);

  try {
    fun::http::GetParameter::const_iterator it;
    it = request.get_parameter.find("from");
    if (it != request.get_parameter.end()) {
      DLOG(INFO) << "from_ts: " << it->second;
      from_ts = boost::lexical_cast<int64_t>(it->second);
    }

    it = request.get_parameter.find("until");
    if (it != request.get_parameter.end()) {
      DLOG(INFO) << "until_ts: " << it->second;
      until_ts = boost::lexical_cast<int64_t>(it->second);
    }
  } catch (boost::bad_lexical_cast &) {
    LOG(ERROR) << "Invalid timestamp";
    response->status_code = fun::http::kBadRequest;
    response->body = "Invalid timestamp";
    return;
  }

  Json result;
  if (!g_handler->GetAccountBillingHistory(
        params[1], params[2], from_ts, until_ts, &result)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  if (result.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }

  response->status_code = fun::http::kOk;
  response->body = result.ToString();
  return;
}


void HandleAccountUpdateCash(Ptr<Response> response, const Request &request,
    const Params &params) {
  DLOG(INFO) << "PUT /v1/account/{" << params[1] << "}/{" << params[2]
      << "}/cash";

  Json body;
  if (!body.FromString(request.body)) {
    LOG(ERROR) << "Invalid body";
    response->status_code = fun::http::kBadRequest;
    return;
  }

  Json result;
  if (!g_handler->UpdateAccountCash(params[1], params[2], body, &result)) {
    LOG(ERROR) << "Update cash handler failed (" << params[1]
        << ", " << params[2] << ')';
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  if (result.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }

  response->status_code = fun::http::kOk;
  response->body = result.ToString();
  return;
}


void HandleDataGet(Ptr<Response> response, const Request &request,
    const Params &params, const std::string &schema) {
  const string key = params["key"];
  DLOG(INFO) << "GET /v1/data/{" << schema << "}/{" << key << "}";
  Json msg;
  Json &out = msg[schema];
  out.SetObject();

  if (!g_handler->GetData(schema, key, &out)) {
    response->status_code = fun::http::kNotImplemented;
    return;
  }

  if (out.IsNull()) {
    response->status_code = fun::http::kNotFound;
    return;
  }
  response->status_code = fun::http::kOk;
  response->body = msg.ToString();
}


bool InitializeCustomerServiceAPI(CsApiHandler *handler) {
  DLOG(INFO) << "Registering CS API handlers (v1)";
  g_handler = handler;
  BOOST_ASSERT(g_handler);

  ApiService::RegisterHandler(http::kGet,
                              boost::regex("/v1/schema/"),
                              HandleSchemaList);
  ApiService::RegisterHandler(http::kGet,
                              boost::regex("/v1/schema/(?<name>[A-Za-z_]+)"),
                              HandleSchemaGet);

  ApiService::RegisterHandler(http::kGet,
                              boost::regex("/v1/account/"),
                              HandleAccountList);
  const char *pattern = "/v1/account/(?<name>[^/]+)/(?<key>[^/]+)/";
  ApiService::RegisterHandler(http::kGet,
                              boost::regex(pattern),
                              HandleAccountGet);

  const char *pattern_cash = "/v1/account/(?<name>[^/]+)/(?<key>[^/]+)/cash";
  ApiService::RegisterHandler(http::kGet,
                              boost::regex(pattern_cash),
                              HandleAccountGetCash);
  ApiService::RegisterHandler(http::kPost,
                              boost::regex(pattern_cash),
                              HandleAccountUpdateCash);

  const char *pattern_history
      = "/v1/account/(?<name>[^/]+)/(?<key>[^/]+)/billing-history";
  ApiService::RegisterHandler(http::kGet,
                              boost::regex(pattern_history),
                              HandleAccountGetBillingHistory);

  ApiService::RegisterHandler(
      http::kGet,
      boost::regex("/v1/data/User/(?<key>[^/]+)"),
      boost::bind(HandleDataGet, _1, _2, _3, std::string("User")));

  return true;
}

}  // namespace cs_api
#endif  // ENABLE_IFUN_DEPLOY_COMPATIBILITY

};  // namespace pong
