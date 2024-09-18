#!/usr/bin/env python312

import threading
from flask import Flask, render_template
from flask_socketio import SocketIO

import asyncio
import argparse
import sys
import os
import socket
from abc import ABC, abstractmethod
from typing import NoReturn, cast, Annotated, override

from dataclasses import dataclass
from enum import Enum

app = Flask(__name__)
socketio = SocketIO(app)


class MessageType(Enum):
    STATUS_INIT = 0


@dataclass
class Args:
    port: Annotated[int, "Listening port number"]
    host: Annotated[str, "Listening host address"]


@app.route('/')
def index() -> str:
    return render_template("index.html")

def main() -> int:
    parser = argparse.ArgumentParser(
            prog="http-p2p-proxy",
            description="Proxy for accessing P2P network via HTTP.",
            epilog="http-p2p-proxy - mail@michaellepera.xyz"
            )

    _ = parser.add_argument(
            "-p", "--port",
            type=int,
            required=True,
            )
    _ = parser.add_argument(
            "-h", "--host",
            required=True,
            )

    args = cast(Args, cast(object, parser.parse_args()))


    app.run(host=args.host, port=args.port)
    return 0


class PeerNetwork(ABC):

    @abstractmethod
    async def get_all_nodes(self) -> None: ...




class UnixSockPeerNetwork(PeerNetwork):

    def __init__(self, sock_path: str):
        self._connected = False
        self._thread = None

        self._sock_path = sock_path

        self._sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    def start(self):

        self._sock.connect(sock_path)
        self._thread = threading.Thread(target=self.listen)
        self._thread.daemon = True
        self._thread.start()
        print("Started daemon listener thread")
    

    def listen(self) -> None:
        while True:
            try:
                data = self._sock.recv(1024)
                if not data:
                    break
                socketio.emit("p2p-update", {"data": data.decode()}) # pyright: ignore[reportUnknownMemberType]
                print(f"Received: {data.decode()}")
            except Exception:
                print("err")

    async def write(self, message: str):
        self._sock.sendall(message.encode())

    @override
    async def get_all_nodes(self) -> None:
        ...


if __name__ == "__main__":
    # sys.exit(main())
    runtime_dir = os.getenv("XDG_RUNTIME_DIR") 
    if runtime_dir is None:
        print("runtime dir not set")
        sys.exit(1)

    sock_path = os.path.join(runtime_dir, ".test.sock")

    sock_proxy = UnixSockPeerNetwork(sock_path)

    sock_proxy.start()

    socketio.run(app) # pyright: ignore[reportUnknownMemberType]




