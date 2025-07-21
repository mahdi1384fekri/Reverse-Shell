# Reverse Shell Implementation - Computer Networks Project

## Overview
This project implements a TCP-based reverse shell in C, consisting of:
- A **server** that listens for incoming connections and sends commands
- A **client** that connects to the server and executes received commands

## Features
- TCP socket communication
- Command execution on client machine
- Multi-client support (server-side)
- Basic terminal interface for server management
- Connection status monitoring

## File Structure

client: revshell_client.c
server: revshell_server_v3.c
Makefile: Includes build and clean commands

## Build Instructions

You can build the project using the provided `Makefile`:

```bash
make
```
To clean up the compiled binaries:
```bash
make clean
```
Run this on the attacker's machine (the one waiting for incoming reverse shell connections):
```bash
./server
```
Run this on the victim's machine (the one connecting back):
```bash
./client
```
