#include <napi.h>

#include <wrapper/Combination.hpp>
#include <wrapper/Solution.hpp>

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Solution::Init(env);
  return Combination::Init(env, exports);
}

NODE_API_MODULE(addon, InitAll)
