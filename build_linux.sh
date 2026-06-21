#!/bin/bash
# QtAutoTestFramework - Linux Build Script

set -e

echo "=========================================="
echo "QtAutoTestFramework - Linux Build"
echo "=========================================="

# 设置 Qt 路径
QT_PATH="/home/smart/Qt/6.5.3/gcc_64"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# 检查 Qt 是否存在
if [ ! -d "$QT_PATH" ]; then
    echo "Error: Qt not found at $QT_PATH"
    exit 1
fi

# 检查 libmicrohttpd
if ! pkg-config --exists libmicrohttpd 2>/dev/null; then
    echo "Installing libmicrohttpd..."
    sudo apt-get update
    sudo apt-get install -y libmicrohttpd-dev
fi

# 编译 cpp-server-lib
echo ""
echo "=== Building cpp-server-lib ==="
cd "$PROJECT_DIR/cpp-server-lib"
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="$QT_PATH"
make -j$(nproc)
echo "[OK] cpp-server-lib built successfully"

# 编译 cpp-server-example
echo ""
echo "=== Building cpp-server-example ==="
cd "$PROJECT_DIR/cpp-server-example"
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="$QT_PATH"
make -j$(nproc)
echo "[OK] cpp-server-example built successfully"

echo ""
echo "=========================================="
echo "Build completed!"
echo ""
echo "To run the demo:"
echo "  cd $PROJECT_DIR/cpp-server-example/build"
echo "  ./DemoApp --test-server --test-port=8080"
echo "=========================================="
