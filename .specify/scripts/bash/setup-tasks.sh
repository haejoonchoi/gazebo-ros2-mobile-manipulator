#!/usr/bin/env bash

set -u

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

JSON=0
HELP=0

usage() {
  cat <<'EOF'
Usage: setup-tasks.sh [-Json] [-Help]
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

if [[ ! -f "${IMPL_PLAN}" ]]; then
  echo "ERROR: plan.md not found in ${FEATURE_DIR}" >&2
  echo "Run /speckit-plan first to create the implementation plan." >&2
  exit 1
fi

if [[ ! -f "${FEATURE_SPEC}" ]]; then
  echo "ERROR: spec.md not found in ${FEATURE_DIR}" >&2
  echo "Run /speckit-specify first to create the feature structure." >&2
  exit 1
fi

docs=()
[[ -f "${RESEARCH}" ]] && docs+=("research.md")
[[ -f "${DATA_MODEL}" ]] && docs+=("data-model.md")
if [[ -d "${CONTRACTS_DIR}" ]] && [[ -n "$(find "${CONTRACTS_DIR}" -mindepth 1 -maxdepth 1 2>/dev/null)" ]]; then
  docs+=("contracts/")
fi
[[ -f "${QUICKSTART}" ]] && docs+=("quickstart.md")

if ! tasks_template=$(resolve_template "tasks-template" "$REPO_ROOT"); then
  echo "ERROR: Could not resolve required tasks-template from the template override stack for ${REPO_ROOT}" >&2
  exit 1
fi

export DOCS="$(printf '%s\n' "${docs[@]}" 2>/dev/null | sed '/^$/d' | paste -sd '\n' -)"

if (( JSON )); then
  python3 - "$tasks_template" <<'PY'
import json, os, sys
with open(sys.argv[1], 'r', encoding='utf-8') as fh:
    template_content = fh.read()
print(json.dumps({
    'FEATURE_DIR': os.environ.get('FEATURE_DIR', ''),
    'AVAILABLE_DOCS': [d for d in os.environ.get('DOCS', '').split('\n') if d],
    'TASKS_TEMPLATE': sys.argv[1],
    'TASKS_TEMPLATE_CONTENT': template_content,
}, separators=(',', ':')))
PY
else
  echo "FEATURE_DIR: ${FEATURE_DIR}"
  echo "TASKS_TEMPLATE: ${tasks_template}"
  echo "AVAILABLE_DOCS:"
  for doc in "${docs[@]}"; do
    case "$doc" in
      research.md) check_file_exists "$RESEARCH" "research.md" >/dev/null || true ;;
      data-model.md) check_file_exists "$DATA_MODEL" "data-model.md" >/dev/null || true ;;
      contracts/) check_dir_has_files "$CONTRACTS_DIR" "contracts/" >/dev/null || true ;;
      quickstart.md) check_file_exists "$QUICKSTART" "quickstart.md" >/dev/null || true ;;
    esac
  done
fi
