{
  "targets": [
    {
      "target_name": "mynumber",
      "sources": [
        "src/wrapper/addon.cpp",
        "src/wrapper/Combination.cpp",
        "src/wrapper/Solution.cpp"
      ],
      "cflags!": [
        "-fvisibility=default"
      ],
      "cflags_cc": [
        "-std=c++20",
        "-O3",
        "-Wall",
        "-Wextra",
        "-fvisibility=hidden",
        "-fvisibility-inlines-hidden"
      ],
      "xcode_settings": {
        "CLANG_CXX_LANGUAGE_STANDARD": "c++20",
        "GCC_SYMBOLS_PRIVATE_EXTERN": "YES",
        "OTHER_CFLAGS": [
          "-Wall",
          "-O3",
          "-fvisibility=hidden",
          "-fvisibility-inlines-hidden"
        ]
      },
      "conditions": [
        [
          "OS=='linux'",
          {
            "ldflags": [
              "-Wl,--exclude-libs,ALL"
            ]
          }
        ],
        [
          "OS=='win'",
          {
            "libraries": [
              "<(module_root_dir)/native-lib/mynumber.lib"
            ]
          }
        ],
        [
          "OS!='win'",
          {
            "libraries": [
              "<(module_root_dir)/native-lib/libmynumber.a"
            ]
          }
        ]
      ],
      "msvs_settings": {
        "VCCLCompilerTool": {
          "AdditionalOptions!": [ "/W3" ],
          "AdditionalOptions": [ "/std:c++20", "/O2", "/W4" ]
        }
      },
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "include"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').targets\"):node_addon_api_except"
      ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
  ]
}
