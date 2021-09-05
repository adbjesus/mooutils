{
  description = "moco";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs { inherit system; };
      in rec {
        packages.moco = pkgs.stdenv.mkDerivation {
          pname = "moco";
          version = "0.1.0";
          src = self;
          nativeBuildInputs = with pkgs; [ cmake ninja ];
          buildInputs = with pkgs; [ glpk fmt_8 catch2 nodejs ];
        };
        defaultPackage = self.packages.${system}.moco;
        # devShell = pkgs.mkShell { buildInputs = with pkgs; [ gcc meson ]; };
      });
}
