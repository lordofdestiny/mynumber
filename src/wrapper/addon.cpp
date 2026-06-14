#include <napi.h>

#include "Combination.hpp"
#include "Solution.hpp"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Solution::Init(env);
  return Combination::Init(env, exports);
}

NODE_API_MODULE(mynumber, InitAll)
