cmake_minimum_required(VERSION 3.16)
project(slot_engine C)

# ============================================================
# グローバル設定
# ============================================================

set(CMAKE_C_STANDARD 99)          # C99 準拠（組み込み互換）
set(CMAKE_C_STANDARD_REQUIRED ON)

# 警告を厳しめに設定（GCC / Clang / MSVC 共通）
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ============================================================
# エンジン ライブラリ（engine/）
#
# タイトル・プラットフォームに依存しない純粋なロジック群。
# 各ターゲットがこれをリンクする。
# ============================================================

add_library(engine STATIC
    engine/rng.c
    engine/timing.c
    engine/reel_ctrl.c
    engine/input_mgr.c
    engine/effect_mgr.c
    engine/save_mgr.c
    engine/slot_engine.c
)

# エンジンはリポジトリルートからインクルードする
target_include_directories(engine PUBLIC ${CMAKE_SOURCE_DIR})

# ============================================================
# simple_fruit タイトル ライブラリ（titles/simple_fruit/）
# ============================================================

add_library(title_simple_fruit STATIC
    titles/simple_fruit/fruit_game.c
    titles/simple_fruit/fruit_judge.c
)

target_include_directories(title_simple_fruit PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/titles/simple_fruit
)

target_link_libraries(title_simple_fruit PUBLIC engine)

# ============================================================
# CLI プラットフォーム ライブラリ（platform/）
# ============================================================

add_library(platform_cli STATIC
    platform/platform_cli.c
)

target_include_directories(platform_cli PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/platform
)

target_link_libraries(platform_cli PUBLIC engine)

# POSIX のみ: 標準ライブラリのリアルタイム拡張が必要な場合に備えて
# Linux では -lm が要ることがある（今は不要だが念のため）
if (UNIX AND NOT APPLE)
    target_link_libraries(platform_cli PUBLIC m)
endif()

# ============================================================
# CLI ターゲット実行ファイル（targets/cli/）
# ============================================================

add_executable(slot_cli
    targets/cli/main_cli.c
    targets/cli/renderer_cli.c
)

target_include_directories(slot_cli PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/engine
    ${CMAKE_SOURCE_DIR}/platform
    ${CMAKE_SOURCE_DIR}/titles/simple_fruit
    ${CMAKE_SOURCE_DIR}/targets/cli
)

target_link_libraries(slot_cli PRIVATE
    engine
    title_simple_fruit
    platform_cli
)

# Windows では コンソール出力バッファを無効化しなくてよいが
# MSYS2 / MinGW では ANSI エスケープを有効にするため
# -municode を付けない（ASCII のまま動かす）

# ============================================================
# ビルド出力ディレクトリ（任意）
# ============================================================

set_target_properties(slot_cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# ============================================================
# Phase 4 以降（SDL2 ターゲット）— コメントアウト中
# ============================================================
# find_package(SDL2)
# if (SDL2_FOUND)
#     add_executable(slot_sdl ...)
#     target_link_libraries(slot_sdl PRIVATE SDL2::SDL2 engine ...)
# endif()

# ============================================================
# 簡易ビルド手順（README 相当のコメント）
# ============================================================
#
# MSYS2 MinGW64 の場合:
#   mkdir build && cd build
#   cmake .. -G "MinGW Makefiles"
#   mingw32-make
#   bin/slot_cli.exe
#
# Linux / macOS の場合:
#   mkdir build && cd build
#   cmake ..
#   make
#   ./bin/slot_cli