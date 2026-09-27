#!/usr/bin/env bash

# Common bash helpers for Spec Kit projects.
# This mirrors the PowerShell common.ps1 behavior enough for normal workflow use
# on Linux/macOS without requiring pwsh.

set -u

specify_find_root() {
  local dir="${1:-$PWD}"
  while [[ "$dir" != "/" && "$dir" != "" ]]; do
    if [[ -d "$dir/.specify" ]]; then
      printf '%s\n' "$dir"
      return 0
    fi
    dir=$(dirname "$dir")
  done

  if [[ -d "/.specify" ]]; then
    printf '%s\n' "/"
    return 0
  fi

  return 1
}

resolve_specify_init_dir() {
  if [[ -z "${SPECIFY_INIT_DIR:-}" ]]; then
    return 1
  fi

  local init_dir="$SPECIFY_INIT_DIR"
  if [[ "$init_dir" != /* ]]; then
    init_dir="$PWD/$init_dir"
  fi

  if [[ ! -d "$init_dir" ]]; then
    echo "ERROR: SPECIFY_INIT_DIR does not point to an existing directory: ${SPECIFY_INIT_DIR}" >&2
    return 1
  fi

  local init_root="$init_dir"
  init_root="${init_root%/}"
  if [[ ! -d "$init_root/.specify" ]]; then
    echo "ERROR: SPECIFY_INIT_DIR is not a Spec Kit project (no .specify/ directory): $init_root" >&2
    return 1
  fi

  printf '%s\n' "$init_root"
}

repo_root() {
  if [[ -n "${SPECIFY_INIT_DIR:-}" ]]; then
    resolve_specify_init_dir || return 1
    return 0
  fi

  local root
  root=$(specify_find_root "$PWD") || {
    local script_dir
    script_dir="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
    printf '%s\n' "$(CDPATH= cd -- "$script_dir/../../.." && pwd)"
    return 0
  }

  printf '%s\n' "$root"
}

feature_paths() {
  local repo_root_value
  repo_root_value="$(repo_root)" || return 1

  local feature_dir=""
  if [[ -n "${SPECIFY_FEATURE_DIRECTORY:-}" ]]; then
    feature_dir="$SPECIFY_FEATURE_DIRECTORY"
    if [[ "$feature_dir" != /* ]]; then
      feature_dir="$repo_root_value/$feature_dir"
    fi
  elif [[ -f "$repo_root_value/.specify/feature.json" ]]; then
    feature_dir=$(python3 - "$repo_root_value/.specify/feature.json" <<'PY'
import json, sys
path = sys.argv[1]
try:
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    value = data.get('feature_directory')
    if value:
        print(value)
    else:
        raise SystemExit(1)
except Exception:
    raise SystemExit(1)
PY
) || {
      echo "ERROR: Feature directory not found. Set SPECIFY_FEATURE_DIRECTORY or ensure .specify/feature.json contains feature_directory." >&2
      return 1
    }
    if [[ "$feature_dir" != /* ]]; then
      feature_dir="$repo_root_value/$feature_dir"
    fi
  else
    echo "ERROR: Feature directory not found. Set SPECIFY_FEATURE_DIRECTORY or run the specify command to create .specify/feature.json." >&2
    return 1
  fi

  local current_branch="${SPECIFY_FEATURE:-}"
  if [[ -z "$current_branch" ]]; then
    current_branch=$(basename "$feature_dir")
  fi

  export REPO_ROOT="$repo_root_value"
  export CURRENT_BRANCH="$current_branch"
  export FEATURE_DIR="$feature_dir"
  export FEATURE_SPEC="$feature_dir/spec.md"
  export IMPL_PLAN="$feature_dir/plan.md"
  export TASKS="$feature_dir/tasks.md"
  export RESEARCH="$feature_dir/research.md"
  export DATA_MODEL="$feature_dir/data-model.md"
  export QUICKSTART="$feature_dir/quickstart.md"
  export CONTRACTS_DIR="$feature_dir/contracts"
}

resolve_template() {
  local template_name="$1"
  local repo_root_value="${2:-$(repo_root)}"

  local base="$repo_root_value/.specify/templates"
  local candidate=""

  for candidate in \
    "$base/overrides/${template_name}.md" \
    "$repo_root_value/.specify/presets"/*/templates/${template_name}.md \
    "$repo_root_value/.specify/presets"/*/${template_name}.md \
    "$repo_root_value/.specify/extensions"/*/templates/${template_name}.md \
    "$repo_root_value/.specify/extensions"/*/${template_name}.md \
    "$base/${template_name}.md"
  do
    if [[ -f "$candidate" ]]; then
      printf '%s\n' "$candidate"
      return 0
    fi
  done

  return 1
}

resolve_template_content() {
  local template_name="$1"
  local repo_root_value="${2:-$(repo_root)}"
  local template_file
  template_file=$(resolve_template "$template_name" "$repo_root_value") || return 1
  cat "$template_file"
}

check_dir_has_files() {
  local dir="$1"
  if [[ -d "$dir" ]] && [[ -n "$(find "$dir" -mindepth 1 -maxdepth 1 2>/dev/null)" ]]; then
    printf '%s\n' "  [OK] $2"
    return 0
  fi
  printf '%s\n' "  [FAIL] $2"
  return 1
}

check_file_exists() {
  local file="$1"
  if [[ -f "$file" ]]; then
    printf '%s\n' "  [OK] $2"
    return 0
  fi
  printf '%s\n' "  [FAIL] $2"
  return 1
}

json_property_string() {
  python3 - "$1" <<'PY'
import json, sys
value = sys.argv[1]
print(json.dumps(value))
PY
}

json_file_value() {
  python3 - "$1" "$2" <<'PY'
import json, sys
path, key = sys.argv[1], sys.argv[2]
with open(path, 'r', encoding='utf-8') as fh:
    data = json.load(fh)
value = data.get(key)
if value is None:
    raise SystemExit(1)
print(value)
PY
}
