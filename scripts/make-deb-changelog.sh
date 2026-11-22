#!/bin/bash

source "$(dirname "${BASH_SOURCE[0]}")/changelog.sh"

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
