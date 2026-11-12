#!/bin/bash

set -euo pipefail

PROJECT_NAME="libvirt-snapshot-manager"
DEBIAN_REVISION="1"

REPO_ROOT=$(git rev-parse --show-toplevel)
CMAKE_FILE="${REPO_ROOT}/CMakeLists.txt"
CHANGELOG_FILE="${REPO_ROOT}/debian/changelog"
CURRENT_BRANCH=$(git branch --show-current)
MAINTAINER_NAME=$(git config user.name)
MAINTAINER_EMAIL=$(git config user.email)

# Проверка текущей ветки (для дальнейшей работы необходима release/x.y.z)
if [[ ! "$CURRENT_BRANCH" =~ ^release/[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "Debian changelog update skipped (not a release branch)." >&2
    exit 0
fi

# Проверка наличия контактов мейнтейнера программы
if [[ -z "$MAINTAINER_NAME" || -z "$MAINTAINER_EMAIL" ]]; then
    echo "Error: Git user.name or user.email is not configured." >&2
    exit 1
fi

# Получение версии проекта из названия ветки (release/x.y.z => x.y.z)
GIT_VERSION=${CURRENT_BRANCH#release/}

# Получение версии проекта из CMakeList.txt
CMAKE_VERSION=$(sed -nE \
    "s/^project\(${PROJECT_NAME}[[:space:]]+VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/p" \
    "$CMAKE_FILE")

if [[ -z "$CMAKE_VERSION" ]]; then
    echo "Error: project version not found in $CMAKE_FILE." >&2
    exit 1
fi

if [[ "$GIT_VERSION" != "$CMAKE_VERSION" ]]; then
    echo "Error: release version on git $GIT_VERSION does not match CMake version $CMAKE_VERSION." >&2
    exit 1
else
    VERSION=${CMAKE_VERSION}
fi

echo "Project version: $VERSION"

# Поиск последнего релизного тега
PREVIOUS_TAG=$(git tag --list 'v[0-9]*.[0-9]*.[0-9]*' --sort=-version:refname | head -n 1)

# Получение коммитов
if [[ -n "$PREVIOUS_TAG" ]]; then
    COMMITS=$(git log "${PREVIOUS_TAG}..HEAD" --no-merges --pretty=format:'%s' --reverse)
else
    COMMITS=$(git log HEAD --no-merges --pretty=format:'%s' --reverse)
fi

if [[ -z "$COMMITS" ]]; then
    COMMITS="No changes since the previous release."
fi

# Генерация актуального changelog'а
{
    echo "$PROJECT_NAME (${VERSION}-${DEBIAN_REVISION}) unstable; urgency=medium"
    echo

    while IFS= read -r commit; do
        echo "  * $commit"
    done <<< "$COMMITS"

    echo
    printf ' -- %s <%s>  %s\n' \
        "$MAINTAINER_NAME" \
        "$MAINTAINER_EMAIL" \
        "$(date -R)"
} > "$CHANGELOG_FILE"

echo "Generated $CHANGELOG_FILE:"
echo
cat "$CHANGELOG_FILE"
