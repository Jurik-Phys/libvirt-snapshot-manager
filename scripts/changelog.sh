#!/bin/bash

MAINTAINER_NAME="Yury Ovsyannikov"
MAINTAINER_EMAIL="jurik.phys@gmail.com"

CURRENT_BRANCH=$(git branch --show-current)
REPO_ROOT=$(git rev-parse --show-toplevel)
CMAKE_FILE="${REPO_ROOT}/CMakeLists.txt"
PROJECT_NAME=$(grep -m1 -oE 'project\([A-Za-z0-9_.-]+' ${CMAKE_FILE} | cut -d'(' -f2)
CHANGELOG_FILE="${REPO_ROOT}/debian/changelog"

# Получение версии проекта из CMakeList.txt
CMAKE_VERSION=$(sed -nE \
    "s/^project\(${PROJECT_NAME}[[:space:]]+VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/p" \
    "$CMAKE_FILE")

if [[ -z "$CMAKE_VERSION" ]]; then
    echo "[EE] Project version not found in $CMAKE_FILE. Exit." >&2
    exit 1
fi

# Проверка текущей ветки (для дальнейшей работы необходима release/x.y.z)
if [[ ! "$CURRENT_BRANCH" =~ ^release/[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    # 21 означает, что текущая ветка проекта не релизная
    return 21
fi

# Получение версии проекта из названия ветки (release/x.y.z => x.y.z)
GIT_VERSION=${CURRENT_BRANCH#release/}

if [[ "$GIT_VERSION" != "$CMAKE_VERSION" ]]; then
    echo "[EE] Release version on git $GIT_VERSION does not match CMake version $CMAKE_VERSION." >&2
    exit 1
else
    VERSION=${CMAKE_VERSION}
fi

echo "[II] Project version: $VERSION."

# Поиск последнего релизного тега
PREVIOUS_TAG=$(git tag --list 'v[0-9]*.[0-9]*.[0-9]*' --sort=-version:refname | head -n 1)

# Получение коммитов
if [[ -n "$PREVIOUS_TAG" ]]; then
    COMMITS=$(git log "${PREVIOUS_TAG}..HEAD" --no-merges --pretty=format:'%s' --reverse)
else
    # Предыдущих тегов нет, следовательно, данный релиз будет первым
    COMMITS="Initial public release"
fi

if [[ -z "$COMMITS" ]]; then
    COMMITS="[II] No changes since the previous release."
fi
