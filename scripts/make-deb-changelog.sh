#!/bin/bash

source "$(dirname "${BASH_SOURCE[0]}")/changelog.sh"
source_result=$?

# "source_result = 21 - означает, что текущая ветка не релизная
# и если changelog уже существует, то его не стоит изменять в ветке разработки,
# а если его нет, то его необходимо создать с актуальной версией программы, но
# заглушкой в списке изменений т.к. релиза ещё не было, изменения сугубо рабочие
if [[ $source_result == 21 ]]; then
    if [[ -f ${CHANGELOG_FILE} ]]; then
        echo "[II] Changelog update skipped (not a release branch)."
        exit 1
    else
        # Файл changelog не существует и для сборки пакета
        # необходима адекватная заглушка
        VERSION=${CMAKE_VERSION}
        COMMITS=$'Initial build\nProject under development'
    fi
fi

DEBIAN_REVISION="1"

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
