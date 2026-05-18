package matchmaking

import (
	"encoding/json"
	"net/http"
	"sync"
	"time"

	"github.com/mtepenner/marcos-motor-mayhem/src/network/matchmaking/internal"
)

// Config tunes the matchmaker.
type Config struct {
	MaxQueueSize      int
	PlayersPerMatch   int
	TicketTimeout     time.Duration
	MaxServersPerPoll int
}

// Matchmaker orchestrates queueing and match formation.
type Matchmaker struct {
	cfg       Config
	mu        sync.Mutex
	queue     []*internal.Ticket
	allocator *ServerAllocator
}

// NewMatchmaker creates a ready-to-use Matchmaker.
func NewMatchmaker(cfg Config) *Matchmaker {
	if cfg.PlayersPerMatch <= 0 {
		cfg.PlayersPerMatch = 4
	}
	if cfg.TicketTimeout <= 0 {
		cfg.TicketTimeout = 30 * time.Second
	}
	return &Matchmaker{
		cfg:       cfg,
		allocator: NewServerAllocator(),
	}
}

// Enqueue adds a player ticket to the queue.
// Returns the ticket ID and an error if the queue is full.
func (m *Matchmaker) Enqueue(playerID string, region string) (*internal.Ticket, error) {
	m.mu.Lock()
	defer m.mu.Unlock()

	if m.cfg.MaxQueueSize > 0 && len(m.queue) >= m.cfg.MaxQueueSize {
		return nil, ErrQueueFull
	}

	t := internal.NewTicket(playerID, region)
	m.queue = append(m.queue, t)
	return t, nil
}

// Dequeue removes a ticket by ID (player cancelled).
func (m *Matchmaker) Dequeue(ticketID string) bool {
	m.mu.Lock()
	defer m.mu.Unlock()

	for i, t := range m.queue {
		if t.ID == ticketID {
			m.queue = append(m.queue[:i], m.queue[i+1:]...)
			return true
		}
	}
	return false
}

// Poll attempts to form matches from the queue.
// Returns a slice of formed matches (each is a slice of ticket IDs).
func (m *Matchmaker) Poll() [][]string {
	m.mu.Lock()
	defer m.mu.Unlock()

	// Expire timed-out tickets
	now := time.Now()
	active := m.queue[:0]
	for _, t := range m.queue {
		if now.Sub(t.EnqueuedAt) < m.cfg.TicketTimeout {
			active = append(active, t)
		}
	}
	m.queue = active

	// Group by region and form matches
	byRegion := make(map[string][]*internal.Ticket)
	for _, t := range m.queue {
		byRegion[t.Region] = append(byRegion[t.Region], t)
	}

	var matches [][]string
	newQueue := m.queue[:0]

	for region, tickets := range byRegion {
		matched := 0
		for len(tickets)-matched >= m.cfg.PlayersPerMatch {
			group := tickets[matched : matched+m.cfg.PlayersPerMatch]
			ids := make([]string, 0, m.cfg.PlayersPerMatch)
			for _, t := range group {
				ids = append(ids, t.ID)
			}
			matches = append(matches, ids)
			m.allocator.Allocate(region, ids)
			matched += m.cfg.PlayersPerMatch
		}
		// Return unmatched tickets to queue
		for _, t := range tickets[matched:] {
			newQueue = append(newQueue, t)
		}
	}

	m.queue = newQueue
	return matches
}

// QueueDepth returns the current number of waiting tickets.
func (m *Matchmaker) QueueDepth() int {
	m.mu.Lock()
	defer m.mu.Unlock()
	return len(m.queue)
}

// RegisterRoutes mounts the matchmaking HTTP API.
func (m *Matchmaker) RegisterRoutes(mux *http.ServeMux) {
	mux.HandleFunc("/healthz", m.handleHealth)
	mux.HandleFunc("/queue/enqueue", m.handleEnqueue)
	mux.HandleFunc("/queue/dequeue", m.handleDequeue)
	mux.HandleFunc("/poll", m.handlePoll)
	mux.HandleFunc("/stats", m.handleStats)
}

func (m *Matchmaker) handleHealth(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}
	writeJSON(w, http.StatusOK, map[string]string{"status": "ok"})
}

func (m *Matchmaker) handleEnqueue(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req struct {
		PlayerID string `json:"player_id"`
		Region   string `json:"region"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "invalid JSON body", http.StatusBadRequest)
		return
	}
	if req.PlayerID == "" || req.Region == "" {
		http.Error(w, "player_id and region are required", http.StatusBadRequest)
		return
	}

	ticket, err := m.Enqueue(req.PlayerID, req.Region)
	if err != nil {
		if err == ErrQueueFull {
			http.Error(w, err.Error(), http.StatusTooManyRequests)
			return
		}
		http.Error(w, "failed to enqueue ticket", http.StatusInternalServerError)
		return
	}

	writeJSON(w, http.StatusCreated, map[string]string{
		"ticket_id": ticket.ID,
	})
}

func (m *Matchmaker) handleDequeue(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req struct {
		TicketID string `json:"ticket_id"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "invalid JSON body", http.StatusBadRequest)
		return
	}
	if req.TicketID == "" {
		http.Error(w, "ticket_id is required", http.StatusBadRequest)
		return
	}

	if !m.Dequeue(req.TicketID) {
		http.Error(w, "ticket not found", http.StatusNotFound)
		return
	}

	writeJSON(w, http.StatusOK, map[string]bool{"removed": true})
}

func (m *Matchmaker) handlePoll(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	matches := m.Poll()
	writeJSON(w, http.StatusOK, map[string]any{
		"matches": matches,
	})
}

func (m *Matchmaker) handleStats(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	writeJSON(w, http.StatusOK, map[string]any{
		"queue_depth": m.QueueDepth(),
		"allocations": m.allocator.Snapshot(),
	})
}

func writeJSON(w http.ResponseWriter, status int, value any) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(value)
}
