package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/mtepenner/marcos-motor-mayhem/src/network/matchmaking"
)

func main() {
	addr := envOrDefault("MATCHMAKER_ADDR", ":8090")

	mm := matchmaking.NewMatchmaker(matchmaking.Config{
		MaxQueueSize:      256,
		PlayersPerMatch:   4,
		TicketTimeout:     30 * time.Second,
		MaxServersPerPoll: 4,
	})

	mux := http.NewServeMux()
	mm.RegisterRoutes(mux)

	srv := &http.Server{Addr: addr, Handler: mux}

	go func() {
		log.Printf("[matchmaker] listening on %s\n", addr)
		if err := srv.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			fmt.Fprintf(os.Stderr, "[matchmaker] fatal: %v\n", err)
			os.Exit(1)
		}
	}()

	sig := make(chan os.Signal, 1)
	signal.Notify(sig, syscall.SIGINT, syscall.SIGTERM)
	<-sig
	log.Println("[matchmaker] shutting down")
}

func envOrDefault(key, def string) string {
	if v := os.Getenv(key); v != "" {
		return v
	}
	return def
}
