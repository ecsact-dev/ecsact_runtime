# Changelog
All notable changes to this project will be documented in this file. See [conventional commits](https://www.conventionalcommits.org/) for commit guidelines.

- - -
## 0.8.2 - 2025-01-31
#### Features
- wasm system implementation headers (#275) - (6791ff1) - Ezekiel Warren
- add async session event callback to c++ wrapper - (6a0b119) - Ezekiel Warren
#### Miscellaneous Chores
- formatting - (c92655e) - Ezekiel Warren

- - -

## 0.8.1 - 2025-01-30
#### Bug Fixes
- added missing session IDs to async C++ wrapper fns - (847ef2f) - Ezekiel Warren

- - -

## 0.8.0 - 2025-01-22
#### Features
- async session api (#273) - (6582788) - Ezekiel Warren
- Remove request id from ecsact_async_stream (#266) - (22010eb) - Austin Kelway

- - -

## 0.7.1 - 2024-12-06
#### Features
- std::format for ecsact IDs (#271) - (0b9d377) - Ezekiel Warren
- registry hash method (#270) - (6e7f0f9) - Ezekiel Warren
- new clone registry core function (#269) - (732177c) - Ezekiel Warren

- - -

## 0.7.0 - 2024-10-14
#### Bug Fixes
- use correct macro in async header (#265) - (dbce850) - Ezekiel Warren
#### Features
- remove use of variadic arguments in api due to wasm limitation (#264) - (b034c85) - Ezekiel Warren
- API for streaming components (#262) - (bf7b0d9) - Austin Kelway
- allow export load at runtime (#263) - (e811be1) - Ezekiel Warren

- - -

## 0.6.9 - 2024-08-26
#### Features
- reserve error async codes for custom implementations (#260) - (83c5955) - Ezekiel Warren
#### Miscellaneous Chores
- sync with ecsact_common (#257) - (61846c4) - seaubot

- - -

## 0.6.8 - 2024-08-14
#### Bug Fixes
- add missing 'async to FOR_EACH_ECSACT_API_FN macro (#259) - (5df4132) - Ezekiel Warren
#### Miscellaneous Chores
- update readme logo - (877ed85) - Ezekiel Warren

- - -

## 0.6.7 - 2024-07-02
#### Bug Fixes
- **(meta)** c++ wrapper bad container access (#252) - (2677d39) - Ezekiel Warren
#### Features
- get system caps as a list (#256) - (7122196) - Ezekiel Warren
- indexed fields multi-add api (#255) - (64b81d7) - Ezekiel Warren
#### Miscellaneous Chores
- sync with ecsact_common (#253) - (5d9f5e6) - seaubot
- sync with ecsact_common (#240) - (3de9dc1) - seaubot

- - -

## 0.6.6 - 2024-06-19
#### Bug Fixes
- **(meta)** c++ wrapper using wrong assoc cap count (#251) - (9f29536) - Ezekiel Warren

- - -

## 0.6.5 - 2024-05-31
#### Features
- more parallel execution options (#249) - (675c334) - Ezekiel Warren
- using assoc id in 'other' context api (#247) - (2f27be4) - Ezekiel Warren

- - -

## 0.6.4 - 2024-05-27
#### Features
- improve assoc api + C++ wrappers (#246) - (9288773) - Ezekiel Warren
- invalid id macro (#245) - (d97a63b) - Ezekiel Warren
- more multi assoc api adjustments (#244) - (82e37e2) - Ezekiel Warren

- - -

## 0.6.3 - 2024-05-23
#### Features
- multi field association API (#243) - (e3682e8) - Ezekiel Warren
#### Miscellaneous Chores
- sync with ecsact_common (#239) - (95ffc34) - seaubot

- - -

## 0.6.2 - 2024-05-17
#### Features
- add 'any' component callback to cpp wrapper (#238) - (e8eadfd) - Ezekiel Warren
- better invalid cast error (#234) - (b3979f6) - Ezekiel Warren
#### Miscellaneous Chores
- **(deps)** update dependency platforms to v0.0.10 (#236) - (aa8c7d2) - renovate[bot]
- **(deps)** update dependency googletest to v1.14.0.bcr.1 (#220) - (11be759) - renovate[bot]
- **(deps)** update dependency bazel_skylib to v1.6.1 (#235) - (51a8bab) - renovate[bot]
- ignore bazel lock files - (fe198c7) - Ezekiel Warren
- fixed typos - (af4c774) - Ezekiel Warren
- sync with ecsact_common (#228) - (606630e) - seaubot

- - -

## 0.6.1 - 2024-04-04
#### Bug Fixes
- always inline with inline keyword (#233) - (392b5df) - Ezekiel Warren

- - -

## 0.6.0 - 2024-04-04
#### Bug Fixes
- force inline for c++ wrappers (#231) - (996d407) - Ezekiel Warren
#### Features
- **(meta)** add main package C++ method (#206) - (24f5963) - Ezekiel Warren
#### Miscellaneous Chores
- **(deps)** update hedron_compile_commands digest to eca42c6 (#218) - (7e86b7e) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to ade23e0 (#217) - (f3c3590) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to 7500623 (#213) - (6c1a752) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to 1e5f3c6 (#211) - (01866b8) - renovate[bot]
- **(deps)** update hedron_compile_commands digest to ac6411f (#210) - (a1b1335) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to 0a9feb7 (#209) - (a66efba) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to 42fa12b (#208) - (c9df3b6) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to a76d197 (#207) - (c4d7aa8) - renovate[bot]
- formatting (#232) - (d568662) - Ezekiel Warren
- enable github merge queue - (77c4df8) - Ezekiel Warren
- update bazel version - (7789f64) - Ezekiel Warren
- update clang-format version - (db044b8) - Ezekiel Warren
- update build deps (#230) - (7ac2a3b) - Ezekiel Warren
- fix typos (#229) - (5365dda) - Ezekiel Warren
- fix line endings - (c306068) - Ezekiel Warren
- bzlmod updates - (be0647a) - Ezekiel Warren
- sync with ecsact_common (#224) - (745d7e3) - seaubot
- sync with ecsact_common (#203) - (91ce6b8) - seaubot

- - -

## 0.5.4 - 2023-10-01
#### Features
- new field index type (#204) - (39e4b62) - Ezekiel Warren
#### Miscellaneous Chores
- **(deps)** update com_grail_bazel_toolchain digest to 91abcad (#205) - (755034e) - renovate[bot]

- - -

## 0.5.3 - 2023-09-29
#### Bug Fixes
- typo in system notify settings cpp wrapper - (25397aa) - Ezekiel Warren

- - -

## 0.5.2 - 2023-09-29
#### Features
- system notify settings (#202) - (5bb8737) - Ezekiel Warren
#### Miscellaneous Chores
- **(deps)** update com_grail_bazel_toolchain digest to 2733561 (#201) - (f5e7bef) - renovate[bot]

- - -

## 0.5.1 - 2023-09-20
#### Miscellaneous Chores
- **(deps)** update com_grail_bazel_toolchain digest to 885e692 (#199) - (c5afee1) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to 3e6d4d9 (#198) - (2ba2d08) - renovate[bot]
- sync with ecsact_common (#200) - (0c54f3b) - seaubot

- - -

## 0.5.0 - 2023-09-18
#### Features
- removed codegen references (#197) - (ace86cb) - Ezekiel Warren
#### Miscellaneous Chores
- **(deps)** update com_grail_bazel_toolchain digest to edc058a (#196) - (b48188e) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to e4fad4e (#195) - (9433edb) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to df8ee5a (#194) - (c3be074) - renovate[bot]

- - -

## 0.4.1 - 2023-09-13
#### Bug Fixes
- using ecasct_codegen from registry (#193) - (6d4fab2) - Ezekiel Warren
#### Miscellaneous Chores
- prevent failure on buildozer compatibility_level set - (0258ea6) - Ezekiel Warren
- prevent failure on buildozer compatibility_level set - (1f3f342) - Ezekiel Warren
- sync with ecsact_common (#191) - (0702247) - seaubot
- sync with ecsact_common (#190) - (686d9f0) - seaubot

- - -

## 0.4.0 - 2023-09-13
#### Features
- bzlmodify ecsact_runtime (#185) - (30a2b42) - seaubot
#### Miscellaneous Chores
- **(deps)** update com_grail_bazel_toolchain digest to c217c03 (#188) - (b86cb1b) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to 9d8cc8a (#186) - (7ab9342) - renovate[bot]
- **(deps)** update hedron_compile_commands digest to e160627 (#181) - (6395667) - renovate[bot]
- **(deps)** update hedron_compile_commands digest to ecafb96 (#180) - (a919143) - renovate[bot]
- **(deps)** update hedron_compile_commands digest to aaa2fda (#179) - (51ed25d) - renovate[bot]
- **(deps)** update bazel c++ tooling repositories (#177) - (da32f87) - renovate[bot]
- cog version.minor - (3205c7a) - Ezekiel Warren
- bzlmod archive automatic - (38ce46b) - Ezekiel Warren
- buildozer in cog.toml - (61d0b23) - Ezekiel Warren
- add version to module file - (8d54764) - Ezekiel Warren
- codegen header cleanup (#178) - (f32d5f3) - Ezekiel Warren

- - -

## 0.3.0 - 2023-06-07
#### Features
- add parallel execution hint option (#176) - (03412eb) - Ezekiel Warren
- serialize C++ fns now accept C function as parameters (#171) - (bc89804) - Ezekiel Warren
#### Miscellaneous Chores
- **(deps)** update com_grail_bazel_toolchain digest to ceeedcc (#174) - (f296082) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to b397ad2 (#173) - (00853be) - renovate[bot]
- **(deps)** update com_grail_bazel_toolchain digest to 41ff2a0 (#172) - (cf7eac3) - renovate[bot]
- **(deps)** update hedron_compile_commands digest to 3dddf20 (#170) - (70b4ebb) - renovate[bot]
- **(deps)** update hedron_compile_commands digest to 80ac7ef (#169) - (d412656) - renovate[bot]
- restore components length test (#175) - (cae3239) - Ezekiel Warren
#### Refactoring
- deprecated codegen_plugin.bzl (#168) - (982d9ff) - Ezekiel Warren

- - -

## 0.2.0 - 2023-04-30
#### Features
- new entity execution status core + dynamic API (#167) - (89bf5d9) - Ezekiel Warren
- new lazy system API (#165) - (54779b9) - Ezekiel Warren
#### Miscellaneous Chores
- add cog.toml - (46ee48e) - Ezekiel Warren
- report errors in check module header script (#166) - (7acad1b) - Ezekiel Warren

- - -

Changelog generated by [cocogitto](https://github.com/cocogitto/cocogitto).
