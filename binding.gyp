{
  "targets": [
    {
      "target_name": "addon",
      "sources": [
        "src/wrapper/addon.cpp",
        "src/wrapper/Combination.cpp",
        "src/wrapper/Solution.cpp"
      ],
      "cflags_cc": [
        "-std=c++20",
        "-O3",
        "-Wall",
        "-fvisibility=hidden"
      ],
      "xcode_settings": {
        "CLANG_CXX_LANGUAGE_STANDARD": "c++20",
        "OTHER_CFLAGS": [ "-Wall", "-O3", "-fvisibility=hidden" ]
      },
      "msvs_settings": {
        "VCCLCompilerTool": {
          "AdditionalOptions": [ "/std:c++20", "/O2", "/W4" ]
        }
      },
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "include"
      ],
      "libraries": [
        "<(module_root_dir)/out/lib/libmynumber.a"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').targets\"):node_addon_api_except"
      ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
  ]
}
