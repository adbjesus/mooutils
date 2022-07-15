{
  description = "mooutils";

  inputs = {
    nixpkgs = {
      url = "github:nixos/nixpkgs/nixos-unstable";
    };

    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs { inherit system; };
      in rec {
        packages.mooutils = pkgs.stdenv.mkDerivation {
          pname = "mooutils";
          version = "0.1.0";
          src = self;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            doxygen
            catch2
          ];
        };

        defaultPackage = self.packages.${system}.mooutils;
      });
}
