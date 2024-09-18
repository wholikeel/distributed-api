{
  description = "code-jam-DAPI";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/24.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { nixpkgs, ... }@inputs:
    inputs.utils.lib.eachDefaultSystem (
      system:
      let
        overlay = import ./overlay.nix;
        pkgs = nixpkgs.legacyPackages.${system}.extend overlay;
      in
      {
        formatter = pkgs.nixfmt-rfc-style;
        devShells =
          let
            hp = pkgs.haskellPackages;
          in
          {
            default = pkgs.mkShell {
              name = "code-jam-DAPI";
              packages = with pkgs; [
                cmake
                cmake-format
                cmake-language-server
                clang-tools
                openssl_3_2
                gtest

                gdb
              ];
            };
            hask = hp.shellFor {
              packages = p: [ ];
              withHoogle = true;
              buildInputs = with hp; [
                haskell-language-server
                cabal-install
              ];
            };
          };

        packages = {
          default = pkgs.geop2p;
          http-p2p-proxy = pkgs.http-p2p-proxy;
        };
      }
    );
}
