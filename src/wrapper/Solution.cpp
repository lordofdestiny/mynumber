#include "napi.h"
#include <wrapper/Solution.hpp>

Napi::FunctionReference Solution::constructor;

void Solution::Init(Napi::Env env) {
  Napi::Symbol toStringTagSymbol = Napi::Symbol::WellKnown(env, "toStringTag");

  Napi::Function func =
      DefineClass(env, "Solution",
                  {
                      InstanceAccessor("value", &Solution::GetValue, nullptr),
                      InstanceMethod("expression", &Solution::Reconstruct),
                      InstanceMethod("toString", &Solution::ToString),
                      InstanceAccessor(toStringTagSymbol, &Solution::ToStringTag, nullptr, napi_default),
                  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
}

Solution::Solution(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Solution>(info) {
  if (info.Length() != 1 || !info[0].IsExternal()) {
    Napi::TypeError::New(info.Env(), "Illegal constructor: Solution objects cannot be created directly.")
        .ThrowAsJavaScriptException();
    return;
  }
}

Napi::Value Solution::GetValue(const Napi::CallbackInfo &info) { return Napi::Number::New(info.Env(), state_->value); }

Napi::Value Solution::Reconstruct(const Napi::CallbackInfo &info) {
  return Napi::String::New(info.Env(), state_->reconstruct());
}

Napi::Value Solution::ToString(const Napi::CallbackInfo &info) {
  std::string buffer;
  buffer += "Solution: ";
  buffer += state_->reconstruct();
  buffer += " = ";
  buffer += std::to_string(state_->value);

  return Napi::String::New(info.Env(), buffer);
}

Napi::Value Solution::ToStringTag(const Napi::CallbackInfo &info) { return Napi::String::New(info.Env(), "Solution"); }

// Add this implementation to Solution.cpp
Napi::Object Solution::CreateNew(Napi::Env env, std::shared_ptr<mynum::impl::StateValue> state) {
  Napi::External<void> secretToken = Napi::External<void>::New(env, nullptr);
  Napi::Object obj = constructor.New({secretToken});
  Solution *instance = Solution::Unwrap(obj);
  instance->state_ = state;
  return obj;
}
