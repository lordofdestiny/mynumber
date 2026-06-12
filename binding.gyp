{
  "targets": [
    {
      "target_name": "addon",
      "sources": [ "src/wrapper/Combination.cpp", "src/wrapper/Solution.cpp", "src/wrapper/addon.cpp" ],
      "cflags_cc": [
        "-std=c++23",
        "-O3",
        "-Wall"
      ],
      "xcode_settings": {
        "CLANG_CXX_LANGUAGE_STANDARD": "c++23",
        "OTHER_CFLAGS": [ "-Wall", "-O3", ]
      },
      "libraries": [
        "-L../build/lib",
        "-lmynumber"
      ],
      "include_dirs": [ "<!@(node -p \"require('node-addon-api').include\")" , "include"],
      "dependencies": [ "<!(node -p \"require('node-addon-api').targets\"):node_addon_api_except" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ],
    }
  ]
}
