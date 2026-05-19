.PHONY: build clean test help run run-matchmaker up down all

POWERSHELL := $(shell where pwsh >NUL 2>NUL && echo pwsh || echo powershell)

help:
	@echo "marcos-motor-mayhem - Kart Racing Game Engine"
	@echo ""
	@echo "Available targets:"
	@echo "  make build           - Build C++ game engine"
	@echo "  make test            - Run all tests"
	@echo "  make clean           - Clean build artifacts"
	@echo "  make run             - Run game client"
	@echo "  make run-matchmaker  - Run matchmaking service"
	@echo "  make up              - Start Redis + matchmaker + game (Windows)"
	@echo "  make down            - Stop services started by make up (Windows)"
	@echo "  make all             - Clean and build everything"
	@echo "  make help            - Show this help message"

all: clean build

build:
	@echo "Building Marcos Motor Mayhem..."
	@mkdir -p build
	@cmake -B build -DCMAKE_BUILD_TYPE=Release
	@cmake --build build --config Release
	@echo "Build complete! Binary at ./build/bin/marcos-motor-mayhem"

build-debug:
	@echo "Building debug version..."
	@mkdir -p build
	@cmake -B build -DCMAKE_BUILD_TYPE=Debug
	@cmake --build build --config Debug

test:
	@echo "Running tests..."
	@cd build && ctest --build-config Release -V

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf build cmake-build-* out dist

run: build
	@./build/bin/marcos-motor-mayhem

run-matchmaker:
	@echo "Building matchmaker..."
	@cd src/cmd/matchmaker && go build -o ../../../build/matchmaker main.go
	@echo "Starting matchmaker service on port 50051..."
	@./build/matchmaker --port 50051 --redis localhost:6379

up:
	@$(POWERSHELL) -ExecutionPolicy Bypass -File scripts/spin-up.ps1

down:
	@$(POWERSHELL) -ExecutionPolicy Bypass -File scripts/spin-down.ps1

test-backend:
	@echo "Testing Go backend services..."
	@cd src/cmd/matchmaker && go test -v -race ./...

lint:
	@echo "Running linters..."
	@cd src/cmd/matchmaker && golangci-lint run

install:
	@echo "Installing game to system..."
	@cmake --build build --target install

.DEFAULT_GOAL := help
