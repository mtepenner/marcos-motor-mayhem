package internal

import (
	"crypto/rand"
	"encoding/hex"
	"time"
)

// Ticket represents a player waiting to be matched.
type Ticket struct {
	ID         string
	PlayerID   string
	Region     string
	EnqueuedAt time.Time
}

// NewTicket constructs a ticket with a fresh random ID.
func NewTicket(playerID, region string) *Ticket {
	return &Ticket{
		ID:         newID(),
		PlayerID:   playerID,
		Region:     region,
		EnqueuedAt: time.Now(),
	}
}

func newID() string {
	b := make([]byte, 16)
	_, _ = rand.Read(b)
	return hex.EncodeToString(b)
}
