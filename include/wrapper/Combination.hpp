#ifndef __COMBINATION_WRAPPER_HPP__
#define __COMBINATION_WRAPPER_HPP__

#include <impl/Combination.hpp>

#include <napi.h>

class Combination : public Napi::ObjectWrap<Combination> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Combination(const Napi::CallbackInfo &info);

private:
  // JS Getters & Setters
  Napi::Value GetTarget(const Napi::CallbackInfo &info);
  void SetTarget(const Napi::CallbackInfo &info, const Napi::Value &value);
  Napi::Value GetNumbers(const Napi::CallbackInfo &info);
  void SetNumbers(const Napi::CallbackInfo &info, const Napi::Value &value);

  // Native printing function declarations
  Napi::Value ToString(const Napi::CallbackInfo &info);
  Napi::Value ToStringTag(const Napi::CallbackInfo &info);
  Napi::Value CustomInspect(const Napi::CallbackInfo &info);
  Napi::Value ToJSON(const Napi::CallbackInfo &info);

  // Methods
  Napi::Value Solve(const Napi::CallbackInfo &info);
  Napi::Value AllSolutions(const Napi::CallbackInfo &info);

  // Static Method
  static Napi::Value Generate(const Napi::CallbackInfo &info);
  static Napi::FunctionReference constructor;

  mynum::impl::Combination comb_;
};

#endif
