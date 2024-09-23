{
  description = "code-jam-DAPI";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/24.05";
    nixpkgs-unstable.url = "github:NixOS/nixpkgs/nixos-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs =
    { nixpkgs, ... }@inputs:
    inputs.utils.lib.eachDefaultSystem (
      system:
      let
        overlay = import ./overlay.nix;
        pkgs = (nixpkgs.legacyPackages.${system}.extend overlay).extend (
          _: _: { basedpyright = inputs.nixpkgs-unstable.legacyPackages.${system}.basedpyright; }
        );
      in
      {
        formatter = pkgs.nixfmt-rfc-style;
        devShells = {
          default = pkgs.mkShell {
            name = "code-jam-DAPI";
            packages = with pkgs; [
              # CMAKE add py311 into devshell?????
              cmake
              just
              # cmake-format
              # cmake-language-server
              clang-tools
              openssl_3_2
              gtest
              man-pages
              man-pages-posix
              inetutils
              #
              basedpyright
              ruff
              #
              gdb
              (python312.withPackages (
                ps: with ps; [
                  flask
                  flask-socketio
                ]
              ))
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
