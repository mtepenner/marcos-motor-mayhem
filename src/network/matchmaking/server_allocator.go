package matchmaking

import (
	"fmt"
	"sync"
	"time"
)

// Allocation captures one server assignment for a formed match.
type Allocation struct {
	ServerID  string
	Region    string
	TicketIDs []string
	CreatedAt time.Time
}

// ServerAllocator is a lightweight in-memory allocator used by the matchmaker.
type ServerAllocator struct {
	mu          sync.Mutex
	nextID      int
	allocations []Allocation
}

// NewServerAllocator creates an allocator with an empty assignment history.
func NewServerAllocator() *ServerAllocator {
	return &ServerAllocator{}
}

// Allocate assigns a match to a synthetic server ID and records the allocation.
func (a *ServerAllocator) Allocate(region string, ticketIDs []string) Allocation {
	a.mu.Lock()
	defer a.mu.Unlock()

	a.nextID++
	ids := make([]string, len(ticketIDs))
	copy(ids, ticketIDs)

	allocation := Allocation{
		ServerID:  fmt.Sprintf("%s-srv-%04d", region, a.nextID),
		Region:    region,
		TicketIDs: ids,
		CreatedAt: time.Now(),
	}
	a.allocations = append(a.allocations, allocation)
	return allocation
}

// Snapshot returns a copy of all allocations for inspection.
func (a *ServerAllocator) Snapshot() []Allocation {
	a.mu.Lock()
	defer a.mu.Unlock()

	result := make([]Allocation, len(a.allocations))
	copy(result, a.allocations)
	return result
}
