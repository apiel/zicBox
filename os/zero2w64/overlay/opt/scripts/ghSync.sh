#!/bin/sh

# Usage: ./sync_repo.sh <token> <repo> <destination>
# Example: ./sync_repo.sh gho_xxx1234567890abcdef apiel/my_repo ./data/repositories

TOKEN="$1"
REPO="$2"
DEST="$3"

if [ -z "$TOKEN" ] || [ -z "$REPO" ] || [ -z "$DEST" ]; then
  echo "Usage: $0 <token> <repo> <destination>"
  exit 1
fi

REPO_NAME=$(basename "$REPO")
REPO_DIR="$DEST/$REPO_NAME"

GIT_URL="https://x-access-token:${TOKEN}@github.com/${REPO}.git"

# if REPO_NAME == current then skip because it is a reserved name
if [ "$REPO_NAME" == "current" ]; then
  echo "Skipping $REPO_NAME because it is a reserved name"
  exit 0
fi


# Ensure destination exists
mkdir -p "$DEST"

if [ ! -d "$REPO_DIR/.git" ]; then
  echo "Cloning $REPO into $REPO_DIR..."
  git clone "$GIT_URL" "$REPO_DIR" || exit 1
else
  echo "Syncing $REPO in $REPO_DIR..."
  cd "$REPO_DIR" || exit 1

  # Fetch latest changes
  git fetch origin

  # Reset local branch to remote, but keep local changes (ours strategy)
  git merge -s ours origin/main -m "Merging remote with preference to local changes" || true

  # Stage and commit local changes
  git add -A
  git commit -m "Sync local changes" || true

  # Push everything back
  git push origin main || exit 1
fi

echo "✅ Sync completed for $REPO"
