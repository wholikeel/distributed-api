#!/usr/bin/env python312

import argparse
import asyncio
import logging
import os
import signal
import socket
import sys
import threading
import types

from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import Enum
from flask import Flask, render_template
from flask_socketio import SocketIO
from logging.handlers import SysLogHandler
from typing import NoReturn, cast, Annotated, override


app = Flask(__name__)
socketio = SocketIO(app)


class MessageType(Enum):
    STATUS_INIT = 0


@dataclass
class Args:
    port: Annotated[int, "Listening port number"]
    host: Annotated[str, "Listening host address"]


def get_logger(level: int | str) -> logging.Logger:
    handler = SysLogHandler()
    formatter = logging.Formatter("client: %(message)s")
    logger = logging.getLogger(__name__)
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    logger.setLevel(level)
    return logger


@app.route("/")
def index() -> str:
    return render_template("index.html")


class RuntimeHandler:
    def __init__(self, logger: logging.Logger) -> None:
        self._logger = logger
        self._stopping = False
        self._lock = threading.Lock()

    def __call__(
        self, sig: signal.Signals | int, frame: types.FrameType | None
    ) -> None:
        with self._lock:
            self._stopping = True

    @property
    def is_running(self) -> bool:
        with self._lock:
            return not self._stopping


class PeerNetwork(ABC):
    @abstractmethod
    async def get_all_nodes(self) -> None: ...


class UnixSockPeerNetwork(PeerNetwork):
    def __init__(
        self,
        sock_path: str,
        logger: logging.Logger,
        runtime_handler: RuntimeHandler,
    ):
        self._logger = logger
        self._runtime_handler = runtime_handler
        self._connected = False
        self._thread = None

        self._sock_path = sock_path

        self._sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

    def start(self):
        self._sock.connect(self._sock_path)
        self._thread = threading.Thread(target=self.listen)
        self._thread.daemon = True
        self._thread.start()
        logging.info("started daemon listener thread.")

    def listen(self) -> None:
        while self._runtime_handler.is_running:
            try:
                message = self._read_msg()
                if not message:
                    break
                socketio.emit("p2p-update", {"data": message.decode()})  # pyright: ignore[reportUnknownMemberType]
            except Exception:
                logging.warning("could not recieve data from socket.")
        self._logger.info("listener shutting down.")

    def _read_msg(
        self, max_chunk_count: int = 5, chunk_size: int = 512
    ) -> bytearray:
        message = bytearray()
        count = max_chunk_count
        while not message.endswith(b"\n") and count > 0:
            if not self._runtime_handler.is_running:
                break
            count -= 1
            data = self._sock.recv(chunk_size)
            message += data
        self._logger.info(f"read: {message}")
        return message

    async def write(self, message: str):
        self._sock.sendall(message.encode())

    @override
    async def get_all_nodes(self) -> None: ...


def main() -> int:

    parser = argparse.ArgumentParser(
        prog="http-p2p-proxy",
        description="Proxy for accessing P2P network via HTTP.",
        epilog="http-p2p-proxy - mail@michaellepera.xyz",
    )

    _ = parser.add_argument(
        "--port",
        type=int,
        required=True,
    )
    _ = parser.add_argument(
        "--host",
        required=True,
    )

    args = cast(Args, cast(object, parser.parse_args()))

    logger = get_logger(logging.ERROR)

    signal_handler = RuntimeHandler(logger)
    _ = signal.signal(signal.SIGTERM, signal_handler)
    _ = signal.signal(signal.SIGINT, signal_handler)

    logger.info("Clients starting")

    runtime_dir = os.getenv("XDG_RUNTIME_DIR")
    if runtime_dir is None:
        logger.error("XDG_RUNTIME_DIR not set.")
        return 1

    sock_path = os.path.join(runtime_dir, ".test.sock")

    if not os.path.exists(sock_path):
        logger.error("Local P2P peer is not running with an active unix-socket server")
        return 1

    sock_client = UnixSockPeerNetwork(sock_path, logger, signal_handler)
    sock_client.start()

    socketio.run(app, host=args.host, port=args.port)  # pyright: ignore[reportUnknownMemberType]
    return 0


if __name__ == "__main__":
    sys.exit(main())
