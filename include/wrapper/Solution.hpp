#ifndef __STATE_VALUE_WRAPPER__
#define __STATE_VALUE_WRAPPER__

#include <impl/StateValue.hpp>

#include <napi.h>

class Solution : public Napi::ObjectWrap<Solution> {
public:
  static void Init(Napi::Env env);
  Solution(const Napi::CallbackInfo &info);

  static Napi::Object CreateNew(Napi::Env env, std::shared_ptr<mynum::impl::StateValue> state);

private:
  Napi::Value GetValue(const Napi::CallbackInfo &info);

  Napi::Value Reconstruct(const Napi::CallbackInfo &info);

  Napi::Value ToString(const Napi::CallbackInfo &info);
  Napi::Value ToStringTag(const Napi::CallbackInfo &info);

  static Napi::FunctionReference constructor;

  std::shared_ptr<mynum::impl::StateValue> state_;
};

#endif
