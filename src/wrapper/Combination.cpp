#include "Combination.hpp"
#include "Solution.hpp"

Napi::FunctionReference Combination::constructor;

template <size_t N> static Napi::Array convertArray(Napi::Env env, const std::array<int, N> &arr) {
  Napi::Array jsArray = Napi::Array::New(env, arr.size());
  for (size_t i = 0; i < arr.size(); i++) {
    jsArray.Set(i, Napi::Number::New(env, arr[i]));
  }
  return jsArray;
}

template <size_t N> static std::string convertToString(const std::array<int, N> &arr) {
  std::string buffer = "[";
  auto i = 0;
  for (auto &num : arr) {
    buffer += std::to_string(num);
    if (i != 5) {
      buffer += ", ";
    }
    i++;
  }
  buffer += "]";
  return buffer;
}

Combination::Combination(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Combination>(info) {
  if (info.Length() == 0) {
    return;
  }

  Napi::Env env = info.Env();

  if (info.Length() == 1 && info[0].IsObject()) {
    Napi::Object config = info[0].As<Napi::Object>();

    if (config.Has("target") && config.Get("target").IsNumber()) {
      this->comb_.setTarget(config.Get("target").As<Napi::Number>().Int32Value());
    } else {
      Napi::TypeError::New(env, "Invalid arguments.").ThrowAsJavaScriptException();
    }

    std::array<int, 6> numbers{};
    if (config.Has("numbers") && config.Get("numbers").IsArray()) {
      Napi::Array jsArray = config.Get("numbers").As<Napi::Array>();
      uint32_t len = jsArray.Length() > 6 ? 6 : jsArray.Length();
      for (uint32_t i = 0; i < len; i++) {
        Napi::Value element = jsArray.Get(i);
        if (element.IsNumber()) {
          numbers[i] = element.As<Napi::Number>().Int32Value();
        }
      }
      this->comb_.setNumbers(numbers);
    } else {
      Napi::TypeError::New(env, "Invalid arguments.").ThrowAsJavaScriptException();
    }
    return;
  }

  // JS Constructor syntax: new Combination(target, [num1, num2, ...])
  if (info.Length() == 2) {
    if (!info[0].IsNumber() || !info[1].IsArray()) {
      Napi::TypeError::New(env, "Invalid arguments.").ThrowAsJavaScriptException();
      return;
    }

    this->comb_.setTarget(info[0].As<Napi::Number>().Int32Value());
    Napi::Array arr = info[1].As<Napi::Array>();
    std::array<int, 6> numbers{};
    uint32_t len = arr.Length() > 6 ? 6 : arr.Length();
    for (uint32_t i = 0; i < len; ++i) {
      numbers[i] = arr.Get(i).As<Napi::Number>().Int32Value();
    }
    this->comb_.setNumbers(numbers);
    return;
  }

  Napi::TypeError::New(env, "Too many arguments").ThrowAsJavaScriptException();
}

Napi::Object Combination::Init(Napi::Env env, Napi::Object exports) {
  Napi::Symbol toStringTagSymbol = Napi::Symbol::WellKnown(env, "toStringTag");
  Napi::Symbol inspectSymbol = Napi::Symbol::For(env, "nodejs.util.inspect.custom");

  Napi::Function func = DefineClass(
      env, "Combination",
      {InstanceAccessor("target", &Combination::GetTarget, &Combination::SetTarget),
       InstanceAccessor("numbers", &Combination::GetNumbers, &Combination::SetNumbers),
       InstanceMethod("toString", &Combination::ToString),
       InstanceAccessor(toStringTagSymbol, &Combination::ToStringTag, nullptr, napi_default),
       InstanceMethod(inspectSymbol, &Combination::CustomInspect), InstanceMethod("toJSON", &Combination::ToJSON),
       InstanceMethod("solve", &Combination::Solve), InstanceMethod("allSolutions", &Combination::AllSolutions),
       StaticMethod("generate", &Combination::Generate)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Combination", func);
  return exports;
}

Napi::Value Combination::CustomInspect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  // You can format this however you want the console to display it
  std::string consoleOutput = "Combination [ target: ";
  consoleOutput += std::to_string(comb_.target());
  consoleOutput += ", numbers: [";
  auto i = 0;
  for (auto num : comb_.numbers()) {
    consoleOutput += std::to_string(num);
    if (i != 5) {
      consoleOutput += ", ";
    }
    i++;
  }
  consoleOutput += "] }";

  return Napi::String::New(env, consoleOutput);
}

// Getters & Setters
Napi::Value Combination::GetTarget(const Napi::CallbackInfo &info) {
  return Napi::Number::New(info.Env(), this->comb_.target());
}
void Combination::SetTarget(const Napi::CallbackInfo &info, const Napi::Value &value) {
  this->comb_.setTarget(value.As<Napi::Number>().Int32Value());
}
Napi::Value Combination::GetNumbers(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Array arr = Napi::Array::New(env, 6);
  auto numbers = this->comb_.numbers();
  for (size_t i = 0; i < 6; ++i) {
    arr.Set(i, Napi::Number::New(env, numbers[i]));
  }
  return arr;
}
void Combination::SetNumbers(const Napi::CallbackInfo &info, const Napi::Value &value) {
  Napi::Env env = info.Env();
  if (!value.IsArray()) {
    Napi::TypeError::New(env, "Array expected").ThrowAsJavaScriptException();
    return;
  }
  std::array<int, 6> numbers{};
  Napi::Array arr = value.As<Napi::Array>();
  uint32_t len = arr.Length() > 6 ? 6 : arr.Length();
  for (uint32_t i = 0; i < len; ++i) {
    numbers[i] = arr.Get(i).As<Napi::Number>().Int32Value();
  }
  this->comb_.setNumbers(numbers);
}

// 1. Implementation for console.log() / String(obj) format
Napi::Value Combination::ToString(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  // Format your native C++ data properties into a readable string
  std::string buffer = "Combination { target: ";
  buffer += std::to_string(comb_.target()) + ", ";
  buffer += "numbers : ";
  buffer += convertToString(comb_.numbers());
  buffer += " }";

  return Napi::String::New(env, buffer);
}

Napi::Value Combination::ToStringTag(const Napi::CallbackInfo &info) {
  return Napi::String::New(info.Env(), "Combination");
}

Napi::Value Combination::ToJSON(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  Napi::Object jsonRepresentation = Napi::Object::New(env);
  jsonRepresentation.Set("target", Napi::Number::New(env, this->comb_.target()));
  jsonRepresentation.Set("numbers", convertArray(env, this->comb_.numbers()));

  return jsonRepresentation;
}

// Instance Logic mapping
Napi::Value Combination::Solve(const Napi::CallbackInfo &info) {
  mynum::Solution result = this->comb_.solve();
  return Solution::CreateNew(info.Env(), std::move(result));
}

Napi::Value Combination::AllSolutions(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<mynum::Solution> solutions = this->comb_.allSolutions();

  Napi::Array arr = Napi::Array::New(env, solutions.size());
  for (size_t i = 0; i < solutions.size(); ++i) {
    arr.Set(i, Solution::CreateNew(env, std::move(solutions[i])));
  }
  return arr;
}

Napi::Value Combination::Generate(const Napi::CallbackInfo &info) {
  Napi::Object jsInstance = constructor.New({});
  Combination *c_instance = Combination::Unwrap(jsInstance);
  c_instance->comb_ = mynum::Combination::generate();

  // 4. Return the fully seeded object
  return jsInstance;
}
