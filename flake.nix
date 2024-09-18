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
                python312.withPackages (ps: with ps; [ flask ])
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
