#!/usr/bin/env bash

set -u

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/common.sh"

JSON=0
DRY_RUN=0
ALLOW_EXISTING_BRANCH=0
SHORT_NAME=""
NUMBER=""
TIMESTAMP=0
HELP=0
DESCRIPTION=()

usage() {
  cat <<'EOF'
Usage: ./create-new-feature.sh [-Json] [-DryRun] [-AllowExistingBranch] [-ShortName <name>] [-Number N] [-Timestamp] <feature description>

Options:
  -Json               Output in JSON format
  -DryRun             Compute feature name and paths without creating directories or files
  -AllowExistingBranch  Reuse an existing feature directory if it already exists
  -ShortName <name>   Provide a custom short name (2-4 words) for the feature
  -Number N           Prefer a feature number
  -Timestamp          Use timestamp prefix instead of sequential numbering
  -Help               Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -Json|-json)
      JSON=1
      ;;
    -DryRun|-dry-run)
      DRY_RUN=1
      ;;
    -AllowExistingBranch|-allow-existing-branch)
      ALLOW_EXISTING_BRANCH=1
      ;;
    -ShortName|-short-name)
      shift
      SHORT_NAME="${1:-}"
      ;;
    -Number|-number)
      shift
      NUMBER="${1:-}"
      ;;
    -Timestamp|-timestamp)
      TIMESTAMP=1
      ;;
    -Help|-help|-h)
      HELP=1
      ;;
    --)
      shift
      while [[ $# -gt 0 ]]; do DESCRIPTION+=("$1"); shift; done
      break
      ;;
    -*)
      echo "ERROR: Unknown option '$1'" >&2
      usage >&2
      exit 1
      ;;
    *)
      DESCRIPTION+=("$1")
      ;;
  esac
  shift
done

if (( HELP )); then
  usage
  exit 0
fi

if [[ ${#DESCRIPTION[@]} -eq 0 ]]; then
  echo "Usage: ./create-new-feature.sh [-Json] [-DryRun] [-AllowExistingBranch] [-ShortName <name>] [-Number N] [-Timestamp] <feature description>" >&2
  exit 1
fi

feature_desc="${DESCRIPTION[*]}"
feature_desc="${feature_desc## }"
feature_desc="${feature_desc%% }"

if [[ -z "$feature_desc" ]]; then
  echo "Error: Feature description cannot be empty or contain only whitespace" >&2
  exit 1
fi

repo_root_value="$(repo_root)" || exit 1
cd "$repo_root_value"

specs_dir="$repo_root_value/specs"
if (( ! DRY_RUN )) && [[ ! -d "$specs_dir" ]]; then
  mkdir -p "$specs_dir"
fi

clean_branch_name() {
  printf '%s' "$1" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9]/-/g; s/-\{2,\}/-/g; s/^-//; s/-$//'
}

branch_suffix_from_description() {
  local description="$1"
  local lower
  lower=$(printf '%s' "$description" | tr '[:upper:]' '[:lower:]')
  local words=()
  while IFS= read -r word; do
    [[ -n "$word" ]] && words+=("$word")
  done < <(printf '%s\n' "$lower" | sed 's/[^a-z0-9]/ /g; s/[[:space:]]\{1,\}/ /g' | tr ' ' '\n' | sed '/^$/d')

  local stop_words=('i' 'a' 'an' 'the' 'to' 'for' 'of' 'in' 'on' 'at' 'by' 'with' 'from' 'is' 'are' 'was' 'were' 'be' 'been' 'being' 'have' 'has' 'had' 'do' 'does' 'did' 'will' 'would' 'should' 'could' 'can' 'may' 'might' 'must' 'shall' 'this' 'that' 'these' 'those' 'my' 'your' 'our' 'their' 'want' 'need' 'add' 'get' 'set')
  local filtered=()
  local word
  for word in "${words[@]}"; do
    local skip=0
    for sw in "${stop_words[@]}"; do
      if [[ "$word" == "$sw" ]]; then skip=1; break; fi
    done
    if (( skip )); then continue; fi
    if (( ${#word} >= 3 )); then filtered+=("$word"); fi
  done

  if [[ ${#filtered[@]} -gt 0 ]]; then
    local max_words=3
    if [[ ${#filtered[@]} -eq 4 ]]; then max_words=4; fi
    local result
    result="$(printf '%s\n' "${filtered[@]:0:max_words}" | paste -sd '-' -)"
    printf '%s\n' "$result"
    return 0
  fi

  local fallback
  fallback=$(clean_branch_name "$description")
  fallback=$(printf '%s\n' "$fallback" | sed 's/-/\n/g' | awk 'NF' | head -n 3 | paste -sd '-' -)
  printf '%s\n' "$fallback"
}

if [[ -n "$SHORT_NAME" ]]; then
  branch_suffix=$(clean_branch_name "$SHORT_NAME")
else
  branch_suffix=$(branch_suffix_from_description "$feature_desc")
fi

if [[ -z "$branch_suffix" ]]; then
  echo "[specify] Warning: Feature name is empty after removing unsupported characters. Use -ShortName with ASCII letters or digits (for example, user-auth)." >&2
fi

has_number=0
if [[ -n "$NUMBER" ]]; then
  has_number=1
fi

if (( TIMESTAMP )) && [[ -n "$NUMBER" ]]; then
  echo "[specify] Warning: -Number is ignored when -Timestamp is used" >&2
  NUMBER=""
  has_number=0
fi

if (( TIMESTAMP )); then
  feature_num=$(date +%Y%m%d-%H%M%S)
  branch_name="${feature_num}-${branch_suffix}"
else
  resolved_number=0
  if (( ! has_number )); then
    highest=0
    if [[ -d "$specs_dir" ]]; then
      while IFS= read -r d; do
        [[ -d "$d" ]] || continue
        name=$(basename "$d")
        if [[ "$name" =~ ^([0-9]{3,})- ]] && [[ ! "$name" =~ ^[0-9]{8}-[0-9]{6}- ]]; then
          num="${BASH_REMATCH[1]}"
          if (( 10#$num > highest )); then highest=$((10#$num)); fi
        fi
      done < <(find "$specs_dir" -mindepth 1 -maxdepth 1 -print)
    fi
    resolved_number=$((highest + 1))
  elif [[ ! "$NUMBER" =~ ^[0-9]+$ ]]; then
    echo "Error: -Number must be an unsigned integer, got '$NUMBER'" >&2
    exit 1
  else
    resolved_number=$NUMBER
  fi

  feature_num=$(printf '%03d' "$resolved_number")
  requested_dir="$specs_dir/${feature_num}-${branch_suffix}"
  if (( has_number )) && [[ -d "$specs_dir" ]] && (( ! ALLOW_EXISTING_BRANCH || ! -d "$requested_dir" )); then
    conflict=0
    while IFS= read -r d; do
      [[ -d "$d" ]] || continue
      name=$(basename "$d")
      if [[ "$name" =~ ^${feature_num}- ]]; then
        conflict=1
        break
      fi
    done < <(find "$specs_dir" -mindepth 1 -maxdepth 1 -print)
    if (( conflict )); then
      highest=0
      while IFS= read -r d; do
        [[ -d "$d" ]] || continue
        name=$(basename "$d")
        if [[ "$name" =~ ^([0-9]{3,})- ]] && [[ ! "$name" =~ ^[0-9]{8}-[0-9]{6}- ]]; then
          num="${BASH_REMATCH[1]}"
          if (( 10#$num > highest )); then highest=$((10#$num)); fi
        fi
      done < <(find "$specs_dir" -mindepth 1 -maxdepth 1 -print)
      resolved_number=$highest
      while :; do
        resolved_number=$((resolved_number + 1))
        feature_num=$(printf '%03d' "$resolved_number")
        if ! find "$specs_dir" -mindepth 1 -maxdepth 1 -type d -name "${feature_num}-*" | grep -q .; then
          break
        fi
      done
      echo "[specify] Warning: -Number $NUMBER conflicts with an existing spec directory; using $feature_num instead" >&2
    fi
  fi
  branch_name="${feature_num}-${branch_suffix}"
fi

original_branch_name="${feature_num}-${branch_suffix}"
if [[ "$branch_name" != "$original_branch_name" ]]; then
  echo "[specify] Warning: Branch name exceeded GitHub's 244-byte limit" >&2
  echo "[specify] Original: $original_branch_name ($(printf '%s' "$original_branch_name" | wc -c) bytes)" >&2
  echo "[specify] Truncated to: $branch_name ($(printf '%s' "$branch_name" | wc -c) bytes)" >&2
fi

feature_dir="$specs_dir/$branch_name"
spec_file="$feature_dir/spec.md"

if (( ! DRY_RUN )); then
  if [[ -d "$feature_dir" ]] && (( ! ALLOW_EXISTING_BRANCH )); then
    if (( TIMESTAMP )); then
      echo "Error: Feature directory '$feature_dir' already exists. Rerun to get a new timestamp or use a different -ShortName." >&2
    else
      echo "Error: Feature directory '$feature_dir' already exists. Please use a different feature name or specify a different number with -Number." >&2
    fi
    exit 1
  fi

  needs_spec=0
  if [[ ! -f "$spec_file" ]]; then
    needs_spec=1
  fi

  mkdir -p "$feature_dir"

  if (( needs_spec )); then
    if template_content=$(resolve_template_content "spec-template" "$repo_root_value"); then
      printf '%s' "$template_content" > "$spec_file"
    else
      echo "Warning: Spec template not found; created empty spec file" >&2
      : > "$spec_file"
    fi
  fi

  python3 - "$repo_root_value" "$feature_dir" <<'PY'
import json, os, sys
repo_root, feature_dir = sys.argv[1], sys.argv[2]
rel = os.path.relpath(feature_dir, repo_root)
with open(os.path.join(repo_root, '.specify', 'feature.json'), 'w', encoding='utf-8') as f:
    json.dump({'feature_directory': rel}, f, separators=(',', ':'))
PY

  export SPECIFY_FEATURE="$branch_name"
  export SPECIFY_FEATURE_DIRECTORY="$feature_dir"
  echo "# To persist: \$SPECIFY_FEATURE='${branch_name}'"
  echo "#              \$SPECIFY_FEATURE_DIRECTORY='${feature_dir}'"
fi

if (( JSON )); then
  python3 - "$branch_name" "$spec_file" "$feature_num" "$DRY_RUN" <<'PY'
import json, sys
branch_name, spec_file, feature_num, dry_run = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
obj = {
    'BRANCH_NAME': branch_name,
    'SPEC_FILE': spec_file,
    'FEATURE_NUM': feature_num,
}
if dry_run == '1':
    obj['DRY_RUN'] = True
print(json.dumps(obj, separators=(',', ':')))
PY
else
  echo "BRANCH_NAME: $branch_name"
  echo "SPEC_FILE: $spec_file"
  echo "FEATURE_NUM: $feature_num"
  if (( ! DRY_RUN )); then
    echo "# To persist in your shell: \$SPECIFY_FEATURE='${branch_name}'"
    echo "#                           \$SPECIFY_FEATURE_DIRECTORY='${feature_dir}'"
  fi
fi

export FEATURE_NUM="$feature_num"
export SPEC_FILE="$spec_file"
export DRY_RUN="$DRY_RUN"
