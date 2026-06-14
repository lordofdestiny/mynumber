#ifndef __SOLUTION_WRAPPER__
#define __SOLUTION_WRAPPER__

#include <mynumber/Solution.hpp>

#include <napi.h>

class Solution : public Napi::ObjectWrap<Solution> {
public:
  static void Init(Napi::Env env);
  Solution(const Napi::CallbackInfo &info);

  static Napi::Object CreateNew(Napi::Env env, mynum::Solution state);

private:
  Napi::Value GetValue(const Napi::CallbackInfo &info);

  Napi::Value Reconstruct(const Napi::CallbackInfo &info);

  Napi::Value ToString(const Napi::CallbackInfo &info);
  Napi::Value ToStringTag(const Napi::CallbackInfo &info);

  static Napi::FunctionReference constructor;

  mynum::Solution state_;
};

#endif
