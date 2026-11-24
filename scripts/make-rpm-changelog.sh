#!/bin/bash

check_changelog() {
    local in_section=0
    local line
    while IFS= read -r line; do
        # Поиск секции
        if [[ $line =~ ^%changelog([[:space:]]|$) ]]; then
            in_section=1
            continue
        fi
        # Секция найдена, поиск непустых строк
        if (( in_section )) && [[ -n "${line//[[:space:]]/}" ]]; then
            return 0
        fi
    done < "$1"
    # Непустые строки не найдены
    return 1
}

source "$(dirname "${BASH_SOURCE[0]}")/changelog.sh"
source_result=$?

SPEC_FILE="${REPO_ROOT}/packaging/${PROJECT_NAME}.spec"

# Обработка нерелизной ветки (ветка с разработкой)
if [[ $source_result == 21 ]]; then
    if check_changelog "${SPEC_FILE}"; then
        echo "[II] Changelog update skipped (not a release branch)."
        exit 1
    else
        # Секция %changelog пустая, вставка адекватной заглушки
        VERSION=${CMAKE_VERSION}
        COMMITS=$'Initial build\nProject under development'
        echo "[II] Init changelog section."
    fi
fi

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

echo "[II] RPM changelog and version updated."
