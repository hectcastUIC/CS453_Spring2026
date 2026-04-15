#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <config_file> <output_graph_file>"
  exit 1
fi

CONFIG_FILE="$1"
OUTPUT_GRAPH="$2"

if [ ! -f "$CONFIG_FILE" ]; then
  echo "Config file not found: $CONFIG_FILE"
  exit 1
fi

mkdir -p "$(dirname "$OUTPUT_GRAPH")"
mkdir -p outputs/netgamesim

SEED=$(grep 'seed' "$CONFIG_FILE" | head -n1 | sed 's/[^0-9]//g')
if [ -z "$SEED" ]; then
  SEED=123
fi

if [ ! -f "netgamesim/target/scala-3.2.2/netmodelsim.jar" ]; then
  echo "Building NetGameSim..."
  (
    cd netgamesim
    sbt clean compile assembly
  )
fi

STAMP=$(date +%Y%m%d_%H%M%S)
BASE="outputs/netgamesim/Graph_${STAMP}.ngs"

echo "Running NetGameSim..."

# Allow NetGameSim to fail late, because we only need the DOT file.
set +e
java -Xms1G -Xmx2G \
  -Dconfig.file="$(pwd)/$CONFIG_FILE" \
  -jar netgamesim/target/scala-3.2.2/netmodelsim.jar \
  "$BASE"
JAVA_EXIT=$?
set -e

if [ "$JAVA_EXIT" -ne 0 ]; then
  echo "NetGameSim exited with code $JAVA_EXIT, checking whether DOT output was still created..."
fi

DOT_FILE=$(find outputs -type f -name "*.dot" | sort | tail -n 1)

if [ -z "${DOT_FILE:-}" ]; then
  echo "Could not find NetGameSim DOT output"
  exit 1
fi

echo "Importing NetGameSim DOT: $DOT_FILE"
python3 tools/graph_export/convert_dot_to_weighted.py "$DOT_FILE" "$OUTPUT_GRAPH" "$SEED"

echo "seed=$SEED" > "${OUTPUT_GRAPH}.seed"
cat > "${OUTPUT_GRAPH}.meta" <<EOF
source=NetGameSim
config_file=$CONFIG_FILE
seed=$SEED
dot_file=$DOT_FILE
EOF

echo "Wrote weighted graph to $OUTPUT_GRAPH"
echo "Wrote seed to ${OUTPUT_GRAPH}.seed"
echo "Wrote metadata to ${OUTPUT_GRAPH}.meta"
