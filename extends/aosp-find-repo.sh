#!/usr/bin/env bash

# aosp-find-repo: Interactively find AOSP sub-repository paths using fzf.
#
# Parses the repo manifest to extract all project paths and presents them
# via fzf for quick searching and selection.
#
# Usage: aosp-find-repo <path_to_aosp_root>
# Example:./aosp-find-repo ~/aosp

# --- Configuration ---
# Exit immediately if a command exits with a non-zero status.
set -e
# Treat unset variables as an error when substituting.
set -u
# Cause pipelines to return the exit status of the last command
# that exited with a non-zero status, or zero if all commands exit successfully.
set -o pipefail

# --- Dependency Check ---

# Function to check if a command exists
command_exists() {
  command -v "$1" >/dev/null 2>&1
}

# Check for fzf
if ! command_exists fzf; then
  echo "Error: fzf is not installed. Please install fzf (https://github.com/junegunn/fzf)." >&2
  exit 1
fi

# Check for xmlstarlet
if ! command_exists xmlstarlet; then
  echo "Error: xmlstarlet is not installed. Please install xmlstarlet." >&2
  # Add package manager specific instructions if desired, e.g.:
  # echo "On Debian/Ubuntu: sudo apt-get install xmlstarlet" >&2
  # echo "On macOS (Homebrew): brew install xmlstarlet" >&2
  exit 1
fi

# --- Argument Validation ---

# Check if exactly one argument is provided
if [ "$#" -ne 1 ]; then
  echo "Usage: $(basename "$0") <aosp_root_directory>" >&2
  exit 1
fi

aosp_root_dir="$1"

# Check if the provided argument is a directory
if [ ! -d "$aosp_root_dir" ]; then
  echo "Error: Directory '$aosp_root_dir' not found or is not a directory." >&2
  exit 1
fi

# Check if the.repo directory exists within the provided path
repo_dir="${aosp_root_dir}/.repo"
if [ ! -d "$repo_dir" ]; then
  echo "Error: '.repo' directory not found in '$aosp_root_dir'." >&2
  echo "Please provide the root directory of your AOSP source tree." >&2
  exit 1
fi

# --- Manifest Location ---

manifest_wrapper="${repo_dir}/manifest.xml"
manifests_dir="${repo_dir}/manifests"

# Check if the manifest wrapper file is readable
if [ ! -r "$manifest_wrapper" ]; then
  echo "Error: Cannot read manifest wrapper file: $manifest_wrapper" >&2
  exit 1
fi

# Parse the wrapper manifest to find the name of the included manifest file
# Use a default value (e.g., empty string) in case the xpath fails, checked later
included_manifest_name=$(xmlstarlet sel -t -v '/manifest/include/@name' "$manifest_wrapper" 2>/dev/null \
  || echo "")

# Check if parsing the wrapper succeeded and yielded a name
if [ -z "$included_manifest_name" ]; then
  # Check if it's an older repo version using a symlink
  if [ -L "$manifest_wrapper" ]; then
     # Attempt to resolve symlink if it points within manifests dir
     target_path=$(readlink "$manifest_wrapper")
     if [[ "$target_path" == manifests/* ]]; then
        included_manifest_name=$(basename "$target_path")
        echo "Info: Detected symlink manifest pointing to '$included_manifest_name'." >&2
     else
        echo "Error: Could not determine the actual manifest file from '$manifest_wrapper'." >&2
        echo "It's not a valid include wrapper or a recognized symlink." >&2
        exit 1
     fi
  else
     echo "Error: Could not parse included manifest name from '$manifest_wrapper'." >&2
     echo "Ensure the file contains a valid '<include name=\"...\ />' tag." >&2
     exit 1
  fi
fi


# Construct the path to the actual manifest file
actual_manifest_path="${manifests_dir}/${included_manifest_name}"

echo "actual_manifest_path=$actual_manifest_path"

# Check if the actual manifest file is readable
if [ ! -r "$actual_manifest_path" ]; then
  echo "Error: Cannot read actual manifest file: $actual_manifest_path" >&2
  echo "(Determined from '$manifest_wrapper')" >&2
  exit 1
fi

# --- Project Path Extraction and Selection ---

# Extract project paths using xmlstarlet and pipe to fzf for selection
# Use --height to prevent fzf from taking the full screen if desired
# Use --tac to show newest additions first (might be useful if local manifests add projects)

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"


# selected_path=$(xmlstarlet sel -t -v '//project/@path' "$actual_manifest_path" | fzf \
#    --height 100% \
#    --preview-window=right:40% \
#    --preview "ls -a --color=always $aosp_root_dir/{}" \
#    --bind "alt-enter:execute($SCRIPT_DIR/fzf_find_files.sh $aosp_root_dir/{})+abort" \
#    --prompt="Enter:Select Repo,Alt-Enter: search file> " \
#    )
# fzf_exit_code=$?


xmlstarlet sel -t -v '//project/@path' "$actual_manifest_path" | fzf \
   --height 100% \
   --preview-window=right:40% \
   --preview "ls -a --color=always $aosp_root_dir/{}" \
   --bind "alt-enter:execute($SCRIPT_DIR/fzf_find_files.sh $aosp_root_dir/{})+abort" \
   --prompt="Enter:Select Repo,Alt-Enter: search file> " | \
  while read -r file; do
    # 输出选中文件的完整路径
    echo "$file"
  done

# --- Output Result ---

# Check fzf's exit code to determine action
# if [ "$fzf_exit_code" -eq 0 ]; then
#   # Success: User selected a path
#   echo "$aosp_root_dir/$selected_path"
#   exit 0
# elif [ "$fzf_exit_code" -eq 1 ]; then
#   # No match found
#   echo "No match found." >&2
#   exit 1
# elif [ "$fzf_exit_code" -eq 130 ]; then
#   # User cancelled (Ctrl-C or Esc)
#   echo "Selection cancelled by user." >&2
#   exit 130
# else
#   # fzf encountered an error
#   echo "fzf encountered an error (exit code: $fzf_exit_code)." >&2
#   exit "$fzf_exit_code"
# fi