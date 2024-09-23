final: prev: {
  geop2p = prev.callPackage ./default.nix { };
  http-p2p-proxy = prev.callPackage ./scripts/http-p2p-proxy { };
}
