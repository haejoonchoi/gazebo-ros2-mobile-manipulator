#!/usr/bin/env bash

set -u

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

JSON=0
REQUIRE_SPEC=0
REQUIRE_TASKS=0
INCLUDE_TASKS=0
PATHS_ONLY=0
TEMPLATE_NAME=""
HELP=0

usage() {
  cat <<'EOF'
Usage: check-prerequisites.sh [OPTIONS]

Consolidated prerequisite checking for Spec-Driven Development workflow.

OPTIONS:
  -Json               Output in JSON format
  -RequireSpec        Require spec.md to exist
  -RequireTasks       Require tasks.md to exist
  -IncludeTasks       Include tasks.md in AVAILABLE_DOCS
  -PathsOnly          Only output path variables
  -Template NAME      Include composed template content in JSON output
  -Help, -h           Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -Json|-json)
      JSON=1
      ;;
    -RequireSpec|-require-spec)
      REQUIRE_SPEC=1
      ;;
    -RequireTasks|-require-tasks)
      REQUIRE_TASKS=1
      ;;
    -IncludeTasks|-include-tasks)
      INCLUDE_TASKS=1
      ;;
    -PathsOnly|-paths-only)
      PATHS_ONLY=1
      ;;
    -Template|-template)
      shift
      TEMPLATE_NAME="${1:-}"
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

if (( PATHS_ONLY )); then
  if (( JSON )); then
    python3 - <<'PY'
import json, os
print(json.dumps({
    'REPO_ROOT': os.environ.get('REPO_ROOT', ''),
    'BRANCH': os.environ.get('CURRENT_BRANCH', ''),
    'FEATURE_DIR': os.environ.get('FEATURE_DIR', ''),
    'FEATURE_SPEC': os.environ.get('FEATURE_SPEC', ''),
    'IMPL_PLAN': os.environ.get('IMPL_PLAN', ''),
    'TASKS': os.environ.get('TASKS', ''),
}, separators=(',', ':')))
PY
  else
    echo "REPO_ROOT: ${REPO_ROOT}"
    echo "BRANCH: ${CURRENT_BRANCH}"
    echo "FEATURE_DIR: ${FEATURE_DIR}"
    echo "FEATURE_SPEC: ${FEATURE_SPEC}"
    echo "IMPL_PLAN: ${IMPL_PLAN}"
    echo "TASKS: ${TASKS}"
  fi
  exit 0
fi

if [[ ! -d "${FEATURE_DIR}" ]]; then
  echo "ERROR: Feature directory not found: ${FEATURE_DIR}" >&2
  echo "Run /speckit-specify first to create the feature structure." >&2
  exit 1
fi

if [[ ! -f "${IMPL_PLAN}" ]]; then
  echo "ERROR: plan.md not found in ${FEATURE_DIR}" >&2
  echo "Run /speckit-plan first to create the implementation plan." >&2
  exit 1
fi

if (( REQUIRE_SPEC )) && [[ ! -f "${FEATURE_SPEC}" ]]; then
  echo "ERROR: spec.md not found in ${FEATURE_DIR}" >&2
  echo "Run /speckit-specify first to create the feature specification." >&2
  exit 1
fi

if (( REQUIRE_TASKS )) && [[ ! -f "${TASKS}" ]]; then
  echo "ERROR: tasks.md not found in ${FEATURE_DIR}" >&2
  echo "Run /speckit-tasks first to create the task list." >&2
  exit 1
fi

docs=()
[[ -f "${RESEARCH}" ]] && docs+=("research.md")
[[ -f "${DATA_MODEL}" ]] && docs+=("data-model.md")
if [[ -d "${CONTRACTS_DIR}" ]] && [[ -n "$(find "${CONTRACTS_DIR}" -mindepth 1 -maxdepth 1 2>/dev/null)" ]]; then
  docs+=("contracts/")
fi
[[ -f "${QUICKSTART}" ]] && docs+=("quickstart.md")
if (( INCLUDE_TASKS )) && [[ -f "${TASKS}" ]]; then
  docs+=("tasks.md")
fi

if [[ -n "${TEMPLATE_NAME}" ]]; then
  template_content=$(resolve_template_content "$TEMPLATE_NAME" "$REPO_ROOT") || {
    echo "ERROR: Could not resolve required ${TEMPLATE_NAME} from the template override stack for ${REPO_ROOT}" >&2
    exit 1
  }
  export TEMPLATE_CONTENT="$template_content"
fi

export DOCS="$(printf '%s\n' "${docs[@]}" 2>/dev/null | sed '/^$/d' | paste -sd '\n' -)"

if (( JSON )); then
  python3 - <<'PY'
import json, os
feature_dir = os.environ.get('FEATURE_DIR', '')
docs = [d for d in os.environ.get('DOCS', '').split('\n') if d]
result = {'FEATURE_DIR': feature_dir, 'AVAILABLE_DOCS': docs}
if os.environ.get('TEMPLATE_CONTENT') is not None and os.environ.get('TEMPLATE_CONTENT') != '':
    result['TEMPLATE_CONTENT'] = os.environ.get('TEMPLATE_CONTENT', '')
print(json.dumps(result, separators=(',', ':')))
PY
else
  echo "FEATURE_DIR:${FEATURE_DIR}"
  echo "AVAILABLE_DOCS:"
  for doc in "${docs[@]}"; do
    if [[ "$doc" == "research.md" ]]; then
      check_file_exists "$RESEARCH" "research.md" >/dev/null || true
    elif [[ "$doc" == "data-model.md" ]]; then
      check_file_exists "$DATA_MODEL" "data-model.md" >/dev/null || true
    elif [[ "$doc" == "contracts/" ]]; then
      check_dir_has_files "$CONTRACTS_DIR" "contracts/" >/dev/null || true
    elif [[ "$doc" == "quickstart.md" ]]; then
      check_file_exists "$QUICKSTART" "quickstart.md" >/dev/null || true
    elif [[ "$doc" == "tasks.md" ]]; then
      check_file_exists "$TASKS" "tasks.md" >/dev/null || true
    fi
  done
fi
