#!/bin/bash

REPO_ROOT=$(git rev-parse --show-toplevel)
RPMBUILD_DIR="${REPO_ROOT}/packaging/rpmbuild"
CMAKE_FILE="${REPO_ROOT}/CMakeLists.txt"
PROJECT_NAME=$(grep -m1 -oE 'project\([A-Za-z0-9_.-]+' ${CMAKE_FILE} | cut -d'(' -f2)
VERSION=$(sed -nE \
    "s/^project\(${PROJECT_NAME}[[:space:]]+VERSION[[:space:]]+([0-9]+\.[0-9]+\.[0-9]+).*/\1/p" \
    "$CMAKE_FILE")

if [[ -z "${PROJECT_NAME}" || -z "${VERSION}" ]]; then
    echo "[EE] Failed to determine project name or version. Exit" >&2
    exit 1
fi

mkdir -p "${RPMBUILD_DIR}"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
echo "[II] Build working directories created"

tar \
    --exclude="./.git" \
    --exclude="./.github" \
    --exclude="./build" \
    --exclude="./debian" \
    --exclude="./packaging/*.tar.gz" \
    --exclude="./packaging/AppDir" \
    --exclude="./packaging/*.spec" \
    --exclude="./scripts" \
    -czf "${REPO_ROOT}/packaging/rpmbuild/SOURCES/${PROJECT_NAME}-${VERSION}.tar.gz" \
    -C "${REPO_ROOT}" \
    --transform="s,^\\.,${PROJECT_NAME}-${VERSION}," \
    .

echo "[II] Source archive created: packaging/${PROJECT_NAME}-${VERSION}.tar.gz"

cp "${REPO_ROOT}"/packaging/${PROJECT_NAME}.spec "${REPO_ROOT}"/packaging/rpmbuild/SPECS/
echo "[II] RPM spec-file was copied"

rpmbuild -ba "${RPMBUILD_DIR}/SPECS/${PROJECT_NAME}.spec" --define "_topdir ${RPMBUILD_DIR}"
echo "[II] Done"

