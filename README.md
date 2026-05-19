# Marcos Motor Mayhem 🏎️

A high-performance, cross-platform kart racing game engine built with C++ and Go. Inspired by classic kart racing games, Marco Motor Mayhem delivers fast-paced competitive racing with physics-based gameplay, network multiplayer, and deep customization.

## Features

### Game Engine (C++)
- **High-Performance Rendering**: Vulkan-based graphics pipeline with advanced shader management
- **Physics System**: Custom kart dynamics with realistic friction, drifting, and momentum mechanics
- **Collision Detection**: Optimized AABB and sphere-based collision system
- **Input Handling**: Hardware-agnostic input mapping supporting multiple controller types
- **Memory Management**: Custom memory allocators for optimal performance and cache locality

### Gameplay
- **Multiple Game Modes**:
  - Grand Prix: 4-race cup progression with increasing difficulty
  - Time Trial: Precise timing with ghost comparison
  - Race Mode: Head-to-head competitive racing
- **Item System**: Dynamic item distribution based on race placement with homing and bouncing projectiles
- **Track Hazards**: Obstacle-laden courses with unique environmental challenges
- **Character Roster**: Diverse kart drivers with unique stats and handling characteristics

### Multiplayer & Backend (Go)
- **Matchmaking Service**: Fast queue-based player pairing with dynamic server allocation
- **State Synchronization**: Low-latency UDP network protocol with dead reckoning for smooth client-side prediction
- **Leaderboards**: Persistent ranking system backed by Redis/PostgreSQL
- **Scalable Infrastructure**: Microservices architecture supporting concurrent game sessions

## Architecture

```
marcos-motor-mayhem/
├── src/
│   ├── engine/          # Core game engine (C++)
│   │   ├── core/        # Game loop, entry point, memory management
│   │   ├── physics/     # Collision, rigidbody, kart dynamics
│   │   ├── render/      # Vulkan renderer, camera, shader management
│   │   └── input/       # Input handling and controller mapping
│   ├── game/            # Game logic (C++)
│   │   ├── entities/    # Base entities, karts, hazards
│   │   ├── items/       # Item system and projectiles
│   │   └── modes/       # Race modes and game state
│   ├── cmd/             # Command-line tools
│   │   └── matchmaker/  # Matchmaking service entry point
│   └── network/         # Backend services (Go)
│       └── matchmaking/ # Player queueing and server allocation
├── tests/               # Unit and integration tests
├── tools/               # Build and asset tools
├── docs/                # Documentation and design specs
├── assets/              # Game assets (models, audio, UI)
└── build/               # Build output directory
```

## Building

### Requirements
- CMake 3.20+
- C++17 compiler (GCC 10+, Clang 12+, MSVC 2019+)
- Vulkan SDK 1.3+
- Go 1.22+ (for backend services)

### Build Steps

#### C++ Game Engine
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

#### Go Backend Services
```bash
cd src/cmd/matchmaker
go build -o ../../build/matchmaker main.go
```

## Running

### Game Client
```bash
./build/marcos-motor-mayhem
```

Current local build behavior:
- the game launches as a playable terminal UI,
- `W` accelerates,
- `S` brakes,
- `A` and `D` steer,
- `Space` drifts,
- `Q` or `Esc` quits.

The terminal view shows a top-down track, your kart (`P`), AI racers (`1`-`3`), lap count, speed, and standings.

### Matchmaking Service
```bash
./build/matchmaker --port 50051 --redis localhost:6379
```

### Spin Up Everything At Once (Windows)
```powershell
make up
```

This command:
- starts a local Redis container (`mmm-redis`) if Docker is available,
- builds the C++ game and Go matchmaker,
- starts the matchmaker service,
- launches the game client.

Stop services with:
```powershell
make down
```

## Network Protocol

The state synchronization service uses UDP for low-latency updates:
- **Packet Format**: Binary protocol with kart position, rotation, input state
- **Update Frequency**: 60 Hz client updates, server broadcasts at 30 Hz
- **Dead Reckoning**: Clients extrapolate kart positions between server updates
- **Latency Compensation**: Input prediction with server-side rollback

## Development

### Running Tests
```bash
# C++ tests
ctest --build-config Release -V

# Go tests
go test ./... -v
```

### Code Structure
- Headers files (.h) define interfaces and data structures
- Implementation files (.cpp/.go) contain logic
- Each module has a clear separation of concerns
- Physics and rendering are decoupled through abstract interfaces

## Performance Targets
- Render: 60 FPS at 1440p on mid-range hardware
- Network latency: < 100ms for optimal gameplay
- Memory footprint: < 500 MB game client
- Matchmaking response: < 2 seconds average queue time

## Contributing
Contributions are welcome! Please ensure:
- Code follows the project's C++ style guide (comments in code)
- Tests are included for new features
- Physics changes are validated with unit tests
- Network protocols maintain backward compatibility

## License
MIT License - See LICENSE file for details

## Contact & Support
For issues, feature requests, or contributions, please open an issue on the GitHub repository.

---

**Game On!** 🏁
