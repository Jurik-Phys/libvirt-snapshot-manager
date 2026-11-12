#!/bin/bash

REPO_ROOT=$(git rev-parse --show-toplevel)

"${REPO_ROOT}/scripts/make-deb-changelog.sh"

cd "$REPO_ROOT"

dpkg-buildpackage -us -uc
