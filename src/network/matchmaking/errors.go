package matchmaking

import "errors"

// ErrQueueFull is returned when the ticket queue has reached its maximum size.
var ErrQueueFull = errors.New("matchmaking queue is full")
