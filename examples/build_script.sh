
# If the first argument is not a file, exit
if [ ! -f "$1" ]; then
    echo "Usage: build_script.sh <file> <helper files...>"
    exit 1
fi

# Get the first argument as the path to the file to compile
FILE_PATH=$1

shift

# Get the directory of the file
FILE_DIR=$(dirname "$FILE_PATH")

# Get the name of the file without extension
FILE_NAME=$(basename "$FILE_PATH" .bs)

# Find the repo root
REPO_ROOT=$(git rev-parse --show-toplevel)
BONK_COMPILER_PATH="$REPO_ROOT/cmake-build-debug/bonk"
BONK_STDLIB_PATH="$REPO_ROOT/cmake-build-debug/bonk_stdlib/libbonk-stdlib.a"

echo "Running bonk compiler"
$BONK_COMPILER_PATH $FILE_PATH -g || exit 1

# Read $FILE_DIR/.bscache/$FILE_NAME.project.meta to PROJECT_FILES as array of lines

PROJECT_FILES=()
while IFS= read -r line; do
    PROJECT_FILES+=("$line")
done < "$FILE_DIR/.bscache/$FILE_NAME.project.meta"

# For each file in PROJECT_FILES check if built .s file is newer than .out file
# If it is not, add it to CHANGED_FILES array

CHANGED_FILES=()
for file in "${PROJECT_FILES[@]}"
do
    # If file newer than file.s or file.s does not exist
    if [ "$file" -nt "$file.s" ] || [ ! -f "$file.s" ]; then
        CHANGED_FILES+=("$file")
    fi
done

ASM_FILES=()
for file in "${PROJECT_FILES[@]}"
do
    ASM_FILES+=("$file.s")
done

echo "Running QBE"

# For each file in $CHANGED_FILES run qbe
for file in "${CHANGED_FILES[@]}"
do
    echo "Compiling $file"
    qbe -o "$file.s" "$file" || exit 1
done

# If build directory doesn't exist, create it
if [ ! -d "build" ]; then
    mkdir build
fi

# Run cc with $PROJECT_FILES and helper files
echo "Running CC"
cc -gdwarf-2 -g "${ASM_FILES[@]}" "$BONK_STDLIB_PATH" "$@" -o build/out || exit 1