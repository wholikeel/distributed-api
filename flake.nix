{
  description = "code-jam-DAPI";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/24.05";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, ... }@inputs: inputs.utils.lib.eachDefaultSystem
    (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells = {
          default = pkgs.mkShell {
            name = "code-jam-DAPI";
            packages = with pkgs; [
              cmake
              cmake-format
              cmake-language-server
              clang-tools
              gtest

              gdb
            ];

          };
        };

        packages.default = pkgs.callPackage ./default.nix { };
      });
}

