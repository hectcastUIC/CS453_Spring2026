#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 4 ]; then
  echo "Usage: $0 <graph_file> --ranks <num_ranks> --out <output_part_file>"
  exit 1
fi

GRAPH_FILE="$1"
shift

RANKS=""
OUT_FILE=""

while [ "$#" -gt 0 ]; do
  case "$1" in
    --ranks)
      RANKS="$2"
      shift 2
      ;;
    --out)
      OUT_FILE="$2"
      shift 2
      ;;
    *)
      echo "Unknown argument: $1"
      exit 1
      ;;
  esac
done

if [ ! -f "$GRAPH_FILE" ]; then
  echo "Graph file not found: $GRAPH_FILE"
  exit 1
fi

if [ -z "$RANKS" ] || [ -z "$OUT_FILE" ]; then
  echo "Missing --ranks or --out"
  exit 1
fi

mkdir -p "$(dirname "$OUT_FILE")"

mapfile -t NODES < <(cut -d: -f1 "$GRAPH_FILE" | tr -d ' ')

NODE_COUNT="${#NODES[@]}"

if [ "$RANKS" -le 0 ]; then
  echo "Ranks must be positive"
  exit 1
fi

: > "$OUT_FILE"

for ((r=0; r<RANKS; r++)); do
  printf "%d:" "$r" >> "$OUT_FILE"
  for ((i=0; i<NODE_COUNT; i++)); do
    owner=$(( i * RANKS / NODE_COUNT ))
    if [ "$owner" -eq "$r" ]; then
      printf " %s" "${NODES[$i]}" >> "$OUT_FILE"
    fi
  done
  printf "\n" >> "$OUT_FILE"
done

echo "Wrote partition to $OUT_FILE"
