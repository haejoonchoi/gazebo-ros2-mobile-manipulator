#!/usr/bin/env bash

set -u

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

JSON=0
HELP=0

usage() {
  cat <<'EOF'
Usage: setup-plan.sh [-Json] [-Help]
  -Json     Output results in JSON format
  -Help     Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -Json|-json)
      JSON=1
      ;;
    -Help|-help|-h)
      HELP=1
      ;;
    *)
      echo "ERROR: Unknown option '$1'" >&2
      usage >&2
      exit 1
      ;;
  esac
  shift
done

if (( HELP )); then
  usage
  exit 0
fi

feature_paths || exit 1
mkdir -p "$FEATURE_DIR"

if [[ -f "$IMPL_PLAN" ]]; then
  if (( JSON )); then
    echo "Plan already exists at $IMPL_PLAN, skipping template copy" >&2
  else
    echo "Plan already exists at $IMPL_PLAN, skipping template copy"
  fi
else
  if template_content=$(resolve_template_content "plan-template" "$REPO_ROOT"); then
    printf '%s' "$template_content" > "$IMPL_PLAN"
    if (( JSON )); then
      echo "Copied plan template to $IMPL_PLAN" >&2
    else
      echo "Copied plan template to $IMPL_PLAN"
    fi
  else
    if (( JSON )); then
      echo "Warning: Plan template not found" >&2
    else
      echo "Warning: Plan template not found"
    fi
    : > "$IMPL_PLAN"
  fi
fi

if (( JSON )); then
  python3 - <<'PY'
import json, os
print(json.dumps({
    'FEATURE_SPEC': os.environ.get('FEATURE_SPEC', ''),
    'IMPL_PLAN': os.environ.get('IMPL_PLAN', ''),
    'FEATURE_DIR': os.environ.get('FEATURE_DIR', ''),
    'BRANCH': os.environ.get('CURRENT_BRANCH', ''),
}, separators=(',', ':')))
PY
else
  echo "FEATURE_SPEC: ${FEATURE_SPEC}"
  echo "IMPL_PLAN: ${IMPL_PLAN}"
  echo "FEATURE_DIR: ${FEATURE_DIR}"
  echo "BRANCH: ${CURRENT_BRANCH}"
fi
