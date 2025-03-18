# FTP Client in C

## Overview

This repository contains a lightweight FTP client implemented in C. The project demonstrates fundamental network programming techniques including URL parsing, socket communication, user authentication, passive mode data transfer, and file downloading. It serves as both an educational tool and a foundation for more complex FTP applications.

## Features

- **FTP URL Parsing:** Extracts server address, port, file path, and optional login credentials from an FTP URL.
- **Socket Communication:** Establishes a TCP connection using UNIX sockets.
- **User Authentication:** Supports explicit user credentials with a fallback to default "anonymous" login.
- **Passive Mode:** Implements FTP passive mode to manage data transfer connections.
- **File Download:** Retrieves and stores remote files using standard file I/O operations.

## Getting Started

### Prerequisites

- **Operating System:** Unix-like environment.
- **Compiler:** GCC or any compatible C compiler.
- **Libraries:** Standard C libraries and networking headers:
  - `<stdio.h>`
  - `<sys/socket.h>`
  - `<netinet/in.h>`
  - `<netdb.h>`
  - `<arpa/inet.h>`
  - `<stdlib.h>`
  - `<unistd.h>`
  - `<fcntl.h>`
  - `<string.h>`

### Compilation

Compile the source code with the following command:

```bash
gcc -o ftp_client download.c
