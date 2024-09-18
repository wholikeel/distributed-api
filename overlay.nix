final: prev:
let
  callCabal2Nix = prev.haskellPackages.callCabal2nix;
in
{
  geop2p = prev.callPackage ./default.nix { };
  http-p2p-proxy = callCabal2Nix "http-p2p-proxy" ./. { };
}
