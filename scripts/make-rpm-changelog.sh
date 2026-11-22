#!/bin/bash

source "$(dirname "${BASH_SOURCE[0]}")/changelog.sh"
SPEC_FILE="${REPO_ROOT}/packaging/${PROJECT_NAME}.spec"

if [[ -z "${PROJECT_NAME}" || -z "${VERSION}" ]]; then
    echo "[EE] Failed to determine project name or version. Exit." >&2
    exit 1
fi

sed -i -E "s/^Version:[[:space:]]+.*/Version:        ${VERSION}/" "${SPEC_FILE}"

RPM_REVISION="1"
RPM_DATE="$(LC_ALL=C date "+%a %b %e %Y")"

# Очистка нижней секции %changelog
sed -i '/^%changelog/,$d' "${SPEC_FILE}"

# Генерация актуальной секции %changelog
{
    echo "%changelog"

    printf '* %s %s <%s> %s-%s\n' \
        "$RPM_DATE" \
        "$MAINTAINER_NAME" \
        "$MAINTAINER_EMAIL" \
        "$VERSION" \
	"$RPM_REVISION"

    while IFS= read -r commit; do
        echo "- $commit"
    done <<< "$COMMITS"

    echo
} >> "${SPEC_FILE}"

exit 0

echo "[II] RPM changelog and version updated."

