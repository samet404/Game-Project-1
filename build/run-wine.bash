source ./.env

if [ -z "$WINE_PATH" ]; then
    echo "Error: WINE_PATH is not set"
    exit 1
fi

"$WINE_PATH" ./out/windows/game.exe
